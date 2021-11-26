/* 
co2_init.c
Initialize the annual co2 concentration parameters for bgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group (NTSG)
School of Forestry, University of Montana
Missoula, MT 59812
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "ini.h"
#include "bgc_struct.h"
#include "bgc_constants.h"
#include "pointbgc_struct.h"
#include "pointbgc_func.h"

int co2_init(file init, co2control_struct* co2, control_struct *ctrl)
{
	int errorCode, ny;
	char key1[] = "CO2_CONTROL";
	char keyword[STRINGSIZE];
	char junk[STRINGSIZE];
	file temp;
	int yr;
	double data;

	ny=0;
	errorCode=0;

	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** CO2_CONTROL                                                     ** 
	**                                                                 **
	********************************************************************/

	/* scan for the climate change block keyword, exit if not next */
	if (!errorCode && scan_value(init, keyword, 's'))
	{
		printf("ERROR reading keyword, co2_init()\n");
		errorCode=205;
	}
	if (!errorCode && strcmp(keyword,key1))
	{
		printf("Expecting keyword --> %s in %s\n",key1,init.name);
		errorCode=205;
	}

	/* begin reading co2 control information */
	if (!errorCode && scan_value(init, &co2->varco2, 'i'))
	{
		printf("ERROR reading variable CO2 flag: co2_init()\n");
		errorCode=20501;
	}
	if (!errorCode && scan_value(init, &co2->co2ppm, 'd'))
	{
		printf("ERROR reading constant CO2 value: co2_init()\n");
		errorCode=20502;
	}
	/* if using variable CO2 file, open it, otherwise
	discard the next line of the ini file */
	if (!errorCode && co2->varco2)
	{
    	if (scan_open(init,&temp,'r',1)) 
		{
			printf("ERROR opening annual CO2 file\n");
			errorCode=20503;
		}
		
		/* allocate space for the annual CO2 array */
		if (!errorCode) 
		{
			co2->co2ppm_array = (double*) malloc(ctrl->simyears * sizeof(double));
			co2->co2yrs_array = (int*) malloc(ctrl->simyears * sizeof(int));
			if (!co2->co2ppm_array)
			{
				printf("ERROR allocating for annual CO2 array, co2_init()\n");
				errorCode=20504;
			}
		}
		/* read year and co2 concentration for each simyear */
		while (!errorCode && !(fscanf(temp.ptr,"%i%lf",&yr,&data)==EOF) && ny < ctrl->simyears)
		{
			if (!ny && yr > ctrl->simstartyear)
			{
				printf("ERROR reading annual CO2 array, co2_init()\n");
				printf("Note: file must contain a pair of values for each simulation years\n");
				printf("simyear: year and CO2.\n");
				errorCode=20505;
			} 
			if (data < 0.0)
			{
				printf("ERROR in CO2 data: CO2 must be positive\n");
				errorCode=20506;
			}

			if (!errorCode && yr >= ctrl->simstartyear)
			{
				co2->co2yrs_array[ny] = yr;
				co2->co2ppm_array[ny] = data;
				ny+=1;
			}
			
		}
		if (errorCode != 20503) fclose(temp.ptr);
	}
	else
	{
		if (scan_value(init, junk, 's'))
		{
			printf("ERROR scanning annual co2 filename\n");
			errorCode=20507;
		}
	}

	/* control */
	if (!errorCode && co2->varco2 && ny != ctrl->simyears)
	{
		printf("ERROR reading annual CO2 array, co2_init()\n");
		printf("Note: file must contain a pair of values for each simulation years\n");
		printf("simyear: year and CO2.\n");
		errorCode=20508;
	} 
	


	return (errorCode);
}
