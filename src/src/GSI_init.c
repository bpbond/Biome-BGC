/* 
GSI_init.c
read GSI file for pointbgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.4
Copyright 2017, D. Hidy [dori.hidy@gmail.com]
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


int GSI_init(file init, GSI_struct* GSI)
{
	int ok = 1;
	char key1[] = "GROWING_SEASON";
	char keyword[80];


	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** GSI_FILE                                                       ** 
	**                                                                 **
	********************************************************************/
	
	/* scan for the input GSI_FILE  keyword, exit if not next */
	if (ok && scan_value(init, keyword, 's'))
	{
		printf("Error reading keyword for GSI file: GSI_init()\n");
		ok=0;
	}
	if (ok && strcmp(keyword, key1))
	{
		printf("Expecting keyword --> %s in file %s\n",key1,init.name);
		ok=0;
	}

	
	/* snowcover_limit for calculation heatsum */
	if (ok && scan_value(init, &GSI->snowcover_limit, 'd'))
	{
		printf("Error reading snowcover_limit parameter: GSI_init()\n");
		ok=0;
	}
	
	/* get flag of GSI flag */
	if (ok && scan_value(init, &GSI->GSI_flag, 'i'))
	{
		printf("Error reading flag indicating usage of GSI file: GSI_init()\n");
		ok=0;
	}


	/* heatsum_limit1 for calculation heatsum index */
	if (ok && scan_value(init, &GSI->heatsum_limit1, 'd'))
	{
		printf("Error reading heatsum_limit1 parameter: GSI_init()\n");
		ok=0;
	}

	/* heatsum_limit2 for calculation heatsum index */
	if (ok && scan_value(init, &GSI->heatsum_limit2, 'd'))
	{
		printf("Error reading heatsum_limit2 parameter: GSI_init()\n");
		ok=0;
	}

	/* tmin_limit1 for calculation tmin index */
	if (ok && scan_value(init, &GSI->tmin_limit1, 'd'))
	{
		printf("Error reading tmin_limit1 parameter: GSI_init()\n");
		ok=0;
	}

	/* tmin_limit2 for calculation tmin index */
	if (ok && scan_value(init, &GSI->tmin_limit2, 'd'))
	{
		printf("Error reading tmin_limit2 parameter: GSI_init()\n");
		ok=0;
	}

	/* vpd_limit1 for calculation vpd index */
	if (ok && scan_value(init, &GSI->vpd_limit1, 'd'))
	{
		printf("Error reading vpd_limit1 parameter: GSI_init()\n");
		ok=0;
	}

	/* vpd for calculation vpd index */
	if (ok && scan_value(init, &GSI->vpd_limit2, 'd'))
	{
		printf("Error reading vpd_limit2 parameter: GSI_init()\n");
		ok=0;
	}

	/* dayl_limit1 for calculation dayl index */
	if (ok && scan_value(init, &GSI->dayl_limit1, 'd'))
	{
		printf("Error reading dayl_limit1 parameter: GSI_init()\n");
		ok=0;
	}

	/* dayl_limit2 for calculation dayl index */
	if (ok && scan_value(init, &GSI->dayl_limit2, 'd'))
	{
		printf("Error reading dayl_limit2 parameter: GSI_init()\n");
		ok=0;
	}

	/* n_moving_avg for calculation moving average from indexes */
	if (ok && scan_value(init, &GSI->n_moving_avg, 'i'))
	{
		printf("Error reading n_moving_avg parameter: GSI_init()\n");
		ok=0;
	}

	/* GSI_limit for calculation yday of start growing season  */
	if (ok && scan_value(init, &GSI->GSI_limit_SGS, 'd'))
	{
		printf("Error reading GSI_limit_SGS parameter: GSI_init()\n");
		ok=0;
	}

	/* GSI_limit for calculation yday of end growing season  */
	if (ok && scan_value(init, &GSI->GSI_limit_EGS, 'd'))
	{
		printf("Error reading GSI_limit_EGS parameter: GSI_init()\n");
		ok=0;
	}

	/* get the estimated_vegper filename */
	if (ok && scan_value(init, &GSI->GSI_file,'s')) 
	{
		printf("Error reading GSI filename: GSI_init()\n");
		ok=0;
	}
	


	return (!ok);
}

