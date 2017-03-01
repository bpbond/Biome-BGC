/* 
ndep_init.c
Initialize the varied N deposition parameters for bgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.4
Copyright 2017, D. Hidy [dori.hidy@gmail.com]
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

int ndep_init(file init, ndep_control_struct* ndep, int simyears)
{
	int ok = 1;
	int i;
	char key1[] = "NDEP_CONTROL";
	char keyword[80];
	char junk[80];
	file temp;
	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** ndep                                                     ** 
	**                                                                 **
	********************************************************************/

	/* scan for the climate change block keyword, exit if not next */
	if (ok && scan_value(init, keyword, 's'))
	{
		printf("Error reading keyword, ndep_init()\n");
		ok=0;
	}
	if (ok && strcmp(keyword,key1))
	{
		printf("Expecting keyword --> %s in %s\n",key1,init.name);
		ok=0;
	}

	/* begin reading ndep information */
	if (ok && scan_value(init, &ndep->varndep, 'i'))
	{
		printf("Error reading varied Ndep flag: ndep_init()\n");
		ok=0;
	}

 
	if (ok && scan_value(init, &ndep->ndep, 'd'))
	{
		printf("Error reading N deposition, ndep_init()\n");
		ok=0;
	}

	
	/* if using variable Ndep file, open it, otherwise
	discard the next line of the ini file */
	if (ok && ndep->varndep == 1)
	{
    	if (scan_open(init,&temp,'r')) 
		{
			printf("Error opening annual Ndep file\n");
			ok=0;
		}
		
		/* allocate space for the annual Ndep array */
		if (ok)
		{
			ndep->ndep_array = (double*) malloc(simyears * sizeof(double));
			if (!ndep->ndep_array)
			{
				printf("Error allocating for annual Ndep array, ndep_init()\n");
				ok=0;
			}
		}
		/* read year and Ndep for each simyear */
		for (i=0 ; ok && i<simyears ; i++)
		{
			if (fscanf(temp.ptr,"%*i%lf",&(ndep->ndep_array[i]))==EOF)
			{
				printf("Error reading annual Ndep array, ctrl_init()\n");
				printf("Note: file must contain a pair of values for each\n");
				printf("simyear: year and Ndep.\n");
				ok=0;
			}
			if (ndep->ndep_array[i] < 0.0)
			{
				printf("Error in ndep_init(): Ndep must be positive\n");
				ok=0;
			}
		}
		fclose(temp.ptr);
	}
	else
	{
		if (scan_value(init, junk, 's'))
		{
			printf("Error scanning annual Ndep filename\n");
			ok=0;
		}
	}	
	return (!ok);
}
