/* 
co2_init.c
Initialize the annual co2 concentration parameters for bgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.2
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
#include "pointbgc_struct.h"
#include "pointbgc_func.h"

int co2_init(file init, co2control_struct* co2, int simyears)
{
	int ok = 1;
	int i;
	char key1[] = "CO2_CONTROL";
	char keyword[80];
	char junk[80];
	file temp;

	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** CO2_CONTROL                                                     ** 
	**                                                                 **
	********************************************************************/

	/* scan for the climate change block keyword, exit if not next */
	if (ok && scan_value(init, keyword, 's'))
	{
		printf("Error reading keyword, co2_init()\n");
		ok=0;
	}
	if (ok && strcmp(keyword,key1))
	{
		printf("Expecting keyword --> %s in %s\n",key1,init.name);
		ok=0;
	}

	/* begin reading co2 control information */
	if (ok && scan_value(init, &co2->varco2, 'i'))
	{
		printf("Error reading variable CO2 flag: co2_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &co2->co2ppm, 'd'))
	{
		printf("Error reading constant CO2 value: co2_init()\n");
		ok=0;
	}
	/* if using variable CO2 file, open it, otherwise
	discard the next line of the ini file */
	if (ok && co2->varco2)
	{
    	if (scan_open(init,&temp,'r')) 
		{
			printf("Error opening annual CO2 file\n");
			ok=0;
		}
		
		/* allocate space for the annual CO2 array */
		if (ok) 
		{
			co2->co2ppm_array = (double*) malloc(simyears * sizeof(double));
			if (!co2->co2ppm_array)
			{
				printf("Error allocating for annual CO2 array, co2_init()\n");
				ok=0;
			}
		}
		/* read year and co2 concentration for each simyear */
		for (i=0 ; ok && i<simyears ; i++)
		{
			if (fscanf(temp.ptr,"%*i%lf",&(co2->co2ppm_array[i]))==EOF)
			{
				printf("Error reading annual CO2 array, ctrl_init()\n");
				printf("Note: file must contain a pair of values for each\n");
				printf("simyear: year and CO2.\n");
				ok=0;
			}
			if (co2->co2ppm_array[i] < 0.0)
			{
				printf("Error in co2_init(): co2 (ppm) must be positive\n");
				ok=0;
			}
		}
		fclose(temp.ptr);
	}
	else
	{
		if (scan_value(init, junk, 's'))
		{
			printf("Error scanning annual co2 filename\n");
			ok=0;
		}
	}
	
	if (co2->co2ppm < 0.0)
	{
		printf("Error in co2_init(): co2 (ppm) must be positive\n");
		ok=0;
	}

	return (!ok);
}
