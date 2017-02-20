/* 
irrigation_init.c
read irrigation information for pointbgc simulation

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


int irrigation_init(file init, control_struct* ctrl, irrigation_struct* IRG)
{
	
	char key1[] = "IRRIGATION";
	char keyword[80];
	char bin[100];

	char IRG_filename[100];
	file IRG_file;

	int i;
	int ok = 1;
	int ny=1;


	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** irrigation                                                       ** 
	**                                                                 **
	********************************************************************/
	
	/* scan for the FERTILIZING file keyword, exit if not next */

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
	
	if (ok && scan_value(init, &IRG->IRG_flag, 'i'))
	{
		if (ok && scan_value(init, IRG_filename, 's'))
		{
			printf("Error reading irrigation calculating file\n");
			ok=0;
		}
		else
		{
			
			ok=1;
			if (ctrl->onscreen) printf("INFORMATION: irrigation information from file\n");
			IRG->IRG_flag = 2;
			strcpy(IRG_file.name, IRG_filename);
		}
	}
	else IRG_file=init;

	/* yeary varied garzing parameters (IRG_flag=2); else: constant garzing parameters (IRG_flag=1) */
	if (IRG->IRG_flag == 2)
	{
		ny = ctrl->simyears; 
	
		/* open the main init file for ascii read and check for errors */
		if (file_open(&IRG_file,'i'))
		{
			printf("Error opening IRG_file, irrigation_int.c\n");
			exit(1);
		}

		/* step forward in init file */
		for (i=0; i < n_IRGparam; i++) scan_value(init, bin, 'd');

	}
	else IRG_file=init;

	if (ok && read_mgmarray(ny, IRG->IRG_flag, IRG_file, &(IRG->IRGdays_array)))
	{
		printf("Error reading IRGdays_array\n");
		ok=0;
	}

	if (ok && read_mgmarray(ny, IRG->IRG_flag, IRG_file, &(IRG->IRGquantity_array)))
	{
		printf("Error reading H2Oquantity_array\n");
		ok=0;
	}


	if (IRG->IRG_flag == 2)
	{
		fclose (IRG_file.ptr);
	}
	
	
	IRG->mgmd = -1;
	
	return (!ok);
}
