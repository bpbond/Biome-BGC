/* 
mowing_init.c
read mowing information for pointbgc simulation

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


int mowing_init(file init, control_struct* ctrl, mowing_struct* MOW)
{

	char key1[] = "MOWING";
	char keyword[80];
	char bin[100];

	char MOW_filename[100];
	file MOW_file;

	int i;
	int ok = 1;
	int ny=1;

	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** MOWING                                                       ** 
	**                                                                 **
	********************************************************************/
	
	/* scan for the MOWING file keyword, exit if not next */
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
	
	if (ok && scan_value(init, &MOW->MOW_flag, 'i'))
	{
		if (ok && scan_value(init, MOW_filename, 's'))
		{
			printf("Error reading mowing calculating file\n");
			ok=0;
		}
		else
		{
			
			ok=1;
			if (ctrl->onscreen) printf("INFORMATION: mowing information from file\n");
			MOW->MOW_flag = 2;
			strcpy(MOW_file.name, MOW_filename);
		}
	}

	/* the first three mowing parameters are contant (can not be varied year to year */	
	if (ok && scan_value(init, &MOW->fixday_or_fixLAI_flag, 'i'))
	{
		printf("Error reading fixday_or_fixLAI_flag\n");
		ok=0;
	}

	if (ok && scan_value(init, &MOW->fixLAI_befMOW, 'd'))
	{
		printf("Error reading fixLAI_befMOW\n");
		ok=0;
	}

	if (ok && scan_value(init, &MOW->fixLAI_aftMOW, 'd'))
	{
		printf("Error reading fixLAI_aftMOW\n");
		ok=0;
	}

	/* the other mowing parameters are can be varied year to year */	
	/* yeary varied mowing parameters (MOW_flag=2); else: constant mowing parameters (MOW_flag=1) */
	if (MOW->MOW_flag == 2)
	{
		ny = ctrl->simyears; 
	
		/* open the main init file for ascii read and check for errors */
		if (file_open(&MOW_file,'i'))
		{
			printf("Error opening MOW_file, mowing_int.c\n");
			exit(1);
		}

		/* step forward in init file */
		for (i=0; i < n_MOWparam; i++) scan_value(init, bin, 'd');

	}
	else MOW_file=init;

	
	if (ok && read_mgmarray(ny, MOW->MOW_flag, MOW_file, &(MOW->MOWdays_array)))
	{
		printf("Error reading MOWdays_array\n");
		ok=0;
	}

	if (ok && read_mgmarray(ny, MOW->MOW_flag, MOW_file, &(MOW->LAI_limit_array)))
	{
		printf("Error reading LAI_limit_array\n");
		ok=0;
	}

	if (ok && read_mgmarray(ny, MOW->MOW_flag, MOW_file, &(MOW->transport_coeff_array)))
	{
		printf("Error reading transport_coeff_array\n");
		ok=0;
	}


	if (MOW->MOW_flag == 2)
	{
		fclose (MOW_file.ptr);
	}
		
	MOW->mgmd = -1;	

	return (!ok);
}
