/* 
harvesting_init.c
read harvesting information for pointbgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.3
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


int harvesting_init(file init, control_struct* ctrl, harvesting_struct* HRV)
{
	char key1[] = "HARVESTING";
	char keyword[80];
	char bin[100];

	char HRV_filename[100];
	file HRV_file;

	int i;
	int ok = 1;
	int ny=1;


	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** HARVESTING                                                       ** 
	**                                                                 **
	********************************************************************/
	
	/* scan for the HARVESTING file keyword, exit if not next */
	if (ok && scan_value(init, keyword, 's'))
	{
		printf("Error reading keyword for control data\n");
		ok=0;
	}
	if (ok && strcmp(keyword, key1))
	{
		printf("Expecting keyword --> %s in file %s\n",key1,init.name);
		ok=0;
	}

	if (ok && scan_value(init, &HRV->HRV_flag, 'i'))
	{
		if (ok && scan_value(init, HRV_filename, 's'))
		{
			printf("Error reading harvesting calculating file\n");
			ok=0;
		}
		else
		{
			
			ok=1;
			if (ctrl->onscreen) printf("INFORMATION: harvesting information from file\n");
			HRV->HRV_flag = 2;
			strcpy(HRV_file.name, HRV_filename);
		}
	}

	/* yeary varied garzing parameters (HRV_flag=2); else: constant garzing parameters (HRV_flag=1) */
	if (HRV->HRV_flag == 2)
	{
		ny = ctrl->simyears; 
	
		/* open the main init file for ascii read and check for errors */
		if (file_open(&HRV_file,'i'))
		{
			printf("Error opening HRV_file  (harvesting_init.c)\n");
			exit(1);
		}

		/* step forward in init file */
		for (i=0; i < n_HRVparam; i++) scan_value(init, bin, 'd');

	}
	else HRV_file=init;
	

	if (ok && read_mgmarray(ny, HRV->HRV_flag, HRV_file, &(HRV->HRVdays_array)))
	{
		printf("Error reading first day of harvesting\n");
		ok=0;
	}

	if (ok && read_mgmarray(ny, HRV->HRV_flag, HRV_file, &(HRV->snag_array)))
	{
		printf("Error reading LAI_snag\n");
		ok=0;
	}

	if (ok && read_mgmarray(ny, HRV->HRV_flag, HRV_file, &(HRV->transport_coeff_array)))
	{
		printf("Error reading transport_coef\n");
		ok=0;
	}


	if (HRV->HRV_flag == 2)
	{
		fclose (HRV_file.ptr);
	}

	HRV->mgmd = -1;
	HRV->afterHRV = 0;


	return (!ok);
}
