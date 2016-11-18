/* 
groundwater_init.c
read ground water depth information if it is available

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.1
Copyright 2016, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
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
#include "bgc_constants.h"



int groundwater_init(siteconst_struct* sitec, control_struct* ctrl)
{
	int ok = 1;
	int i;
	int n_data=0;
	file gwd_file;	


	/* ------------------------------------------------------ */
	/* Hidy 2013 - using varying whole plant mortality values */


	
	/* gwd flag: constans or varying gwd from file */
	ctrl->GWD_flag = 0;
	if (ok)
	{
		if (ctrl->spinup == 0)   /* normal run */
		{
			strcpy(gwd_file.name, "groundwater_normal.txt");
			if (!file_open(&gwd_file,'j'))
			{
				ctrl->GWD_flag = 1;
			}
			n_data=ctrl->simyears* NDAY_OF_YEAR;
		}
		else                     /* spinup and tranzient run */        
		{ 
			
			strcpy(gwd_file.name, "groundwater_spinup.txt");
		
			if (!file_open(&gwd_file,'j'))
			{
				ctrl->GWD_flag = 1;
			}
			n_data=NDAY_OF_YEAR;

		
		}
	}


	if (ok && ctrl->GWD_flag) 
	{		
		/* allocate space for the annual gwd array */
		sitec->gwd_array = (double*) malloc(ctrl->simyears * NDAY_OF_YEAR * sizeof(double));
		if (!sitec->gwd_array)
		{
			printf("Error allocating for gwd array, groundwater_init()\n");
			ok=0;
		}

		/* read year and gwd for each simday in each simyear */
		for (i=0 ; ok && i < n_data; i++)
		{
			if (fscanf(gwd_file.ptr,"%*i%lf", &(sitec->gwd_array[i]))==EOF)
			{
				printf("Error reading annual gwd array, groundwater_init()\n");
				printf("Note: file must contain a pair of values for each simday\n");
				printf("simyear: year and gwd.\n");
				ok=0;
			}
			if (sitec->gwd_array[i] < 0.0)
			{
				printf("Error in groundwater_init(): gound water depth must be positive\n");
				ok=0;
			}
		}
		fclose(gwd_file.ptr);
	}	
	else /* if no changing data constant sitec parameter are used */
	{
		sitec->gwd_array = 0;
	}	
	

	/* -------------------------------------------*/

		
	return (!ok);
}
