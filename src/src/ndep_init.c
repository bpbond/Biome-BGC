/* 
ndep_init.c
Initialize the varied N deposition parameters for bgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2020, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Modified:
13/07/2000: Added input of Ndep from file. Changes are made by Galina Churkina.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "ini.h"
#include "bgc_struct.h"
#include "pointbgc_struct.h"
#include "pointbgc_func.h"

int ndep_init(file init, ndep_control_struct* ndep, control_struct *ctrl)
{
	int errorCode, ny;
	char key1[] = "NDEP_CONTROL";
	char keyword[STRINGSIZE];
	char junk[STRINGSIZE];
	int yr;
	double data;
	file temp;

	ny=0;
	errorCode=0;

	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** ndep                                                     ** 
	**                                                                 **
	********************************************************************/

	/* scan for the climate change block keyword, exit if not next */
	if (!errorCode && scan_value(init, keyword, 's'))
	{
		printf("ERROR reading keyword, ndep_init()\n");
		errorCode=206;
	}
	if (!errorCode && strcmp(keyword,key1))
	{
		printf("Expecting keyword --> %s in %s\n",key1,init.name);
		errorCode=206;
	}

	/* begin reading ndep information */
	if (!errorCode && scan_value(init, &ndep->varndep, 'i'))
	{
		printf("ERROR reading varied Ndep flag: ndep_init()\n");
		errorCode=20601;
	}

 
	if (!errorCode && scan_value(init, &ndep->ndep, 'd'))
	{
		printf("ERROR reading N deposition, ndep_init()\n");
		errorCode=20602;
	}

	/* flag for using artificial N-addition during spinup phase (sign of Ndep input data) */
	if (ndep->ndep < 0 && ctrl->spinup)
	{
		ctrl->NaddSPINUP_flag = 1;
		ndep->ndep = -1 * ndep->ndep;
	}
	else
	{
		ctrl->NaddSPINUP_flag = 0;
		if (ctrl->spinup == 0 && ndep->ndep < 0)
		{
			printf("ERROR in Ndep data: negative Ndep value is not possible (in spinup phase negative Ndep = artificial N-adding)\n");
			errorCode=20609;
		}
	}
	
	/* if using variable Ndep file, open it, otherwise
	discard the next line of the ini file */
	if (!errorCode && ndep->varndep == 1)
	{
    	if (scan_open(init,&temp,'r',1)) 
		{
			printf("ERROR opening annual Ndep file\n");
			errorCode=20603;
		}
		
		/* allocate space for the annual Ndep array */
		if (!errorCode)
		{
			ndep->Ndep_array = (double*) malloc(ctrl->simyears * sizeof(double));
			ndep->Nyrs_array = (int*) malloc(ctrl->simyears * sizeof(int));
			if (!ndep->Ndep_array)
			{
				printf("ERROR allocating for annual Ndep array, ndep_init()\n");
				errorCode=20604;
			}
		}
		/* read year and Ndep for each simyear */
		while (!errorCode && !(fscanf(temp.ptr,"%i%lf",&yr,&data)==EOF) && ny < ctrl->simyears)
		{
			if (!ny && yr > ctrl->simstartyear)
			{
				printf("ERROR reading annual ndep array, ndep_init()\n");
				printf("Note: file must contain a pair of values for each simulation years\n");
				printf("simyear: year and ndep.\n");
				errorCode=20605;
			} 
			if (data < 0.0)
			{
				printf("ERROR in Ndep data: Ndep must be positive\n");
				errorCode=20606;
			}

			if (!errorCode && yr >= ctrl->simstartyear)
			{
				ndep->Nyrs_array[ny] = yr;
				ndep->Ndep_array[ny] = data;
				ny+=1;
			}
			
		}
		if (errorCode != 20603) fclose(temp.ptr);
	}
	else
	{
		if (scan_value(init, junk, 's'))
		{
			printf("ERROR scanning annual Ndep filename\n");
			errorCode=20607;
		}
	}	

	/* control */
	if (!errorCode && ndep->varndep && ny != ctrl->simyears)
	{
		printf("ERROR reading annual ndep array, ndep_init()\n");
		printf("Note: file must contain a pair of values for each simulation years\n");
		printf("simyear: year and ndep.\n");
		errorCode=20608;
	} 
	return (errorCode);
}
