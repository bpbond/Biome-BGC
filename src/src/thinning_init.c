/* 
thinning_init.c
read thinning information for pointbgc simulation

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
#include "bgc_constants.h"


int thinning_init(file init, control_struct* ctrl, thinning_struct* THN)
{

	char key1[] = "THINNING";
	char keyword[80];
	char bin[100];

	char THN_filename[100];
	file THN_file;

	int i;
	int ok = 1;
	int ny=1;


	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** thinning                                                       ** 
	**                                                                 **
	********************************************************************/
	
	/* scan for the thinning file keyword, exit if not next */
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
	
	if (ok && scan_value(init, &THN->THN_flag, 'i'))
	{
		if (ok && scan_value(init, THN_filename, 's'))
		{
			printf("Error reading thinning calculating file\n");
			ok=0;
		}
		else
		{
			
			ok=1;
			if (ctrl->onscreen) printf("But it is not a problem (it is only due to the reading of thinning file)\n");
			if (ctrl->onscreen) printf("INFORMATION: thinning information from file\n");
			THN->THN_flag = 2;
			strcpy(THN_file.name, THN_filename);
		}
	}

	/* yeary varied thinning parameters (THN_flag=2); else: constant thinning parameters (THN_flag=1) */
	if (THN->THN_flag == 2)
	{
		ny = ctrl->simyears; 
	
		/* open the main init file for ascii read and check for errors */
		if (file_open(&THN_file,'i'))
		{
			printf("Error opening THN_file, thinning_int.c\n");
			exit(1);
		}

		/* step forward in init file */
		for (i=0; i < n_THNparam; i++) scan_value(init, bin, 'd');

	}
	else THN_file=init;
	

	if (ok && read_mgmarray(ny, THN->THN_flag, THN_file, &(THN->THNdays_array)))
	{
		printf("Error reading first day of thinning\n");
		ok=0;
	}

	if (ok && read_mgmarray(ny, THN->THN_flag, THN_file, &(THN->thinning_rate_array)))
	{
		printf("Error reading thinninng rate\n");
		ok=0;
	}

	if (ok && read_mgmarray(ny, THN->THN_flag, THN_file, &(THN->transpcoeff_woody_array)))
	{
		printf("Error reading transpcoeff_woody\n");
		ok=0;
	}

	if (ok && read_mgmarray(ny, THN->THN_flag, THN_file, &(THN->transpcoeff_nwoody_array)))
	{
		printf("Error reading transpcoeff_nwoody\n");
		ok=0;
	}

	if (THN->THN_flag == 2)
	{
		fclose (THN_file.ptr);
	}

	THN->mgmd = -1;
	
	return (!ok);
}
