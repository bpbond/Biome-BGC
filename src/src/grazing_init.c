/* 
grazing_init.c
read grazinz information for pointbgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.6
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


int grazing_init(file init, control_struct* ctrl, grazing_struct* GRZ)
{

	char key1[] = "GRAZING";
	char keyword[80];
	char bin[100];

	char GRZ_filename[100];
	file GRZ_file;

	int i;
	int ok = 1;
	int ny=1;



	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** GRAZING                                                       ** 
	**                                                                 **
	********************************************************************/
	
	/* scan for the GRAZING file keyword, exit if not next */
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
	
	if (ok && scan_value(init, &GRZ->GRZ_flag, 'i'))
	{
		if (ok && scan_value(init, GRZ_filename, 's'))
		{
			printf("Error reading grazing calculating file\n");
			ok=0;
		}
		else
		{
			
			ok=1;
			if (ctrl->onscreen) printf("But it is not a problem (it is only due to the reading of grazing file)\n");
			if (ctrl->onscreen) printf("INFORMATION: grazing information from file\n");
			GRZ->GRZ_flag = 2;
			strcpy(GRZ_file.name, GRZ_filename);
		}
	}

	/* yeary varied garzing parameters (GRZ_flag=2); else: constant garzing parameters (GRZ_flag=1) */
	if (GRZ->GRZ_flag == 2)
	{
		ny = ctrl->simyears; 
	
		/* open the main init file for ascii read and check for errors */
		if (file_open(&GRZ_file,'i'))
		{
			printf("Error opening GRZ_file, grazing_int.c\n");
			exit(1);
		}

		/* step forward in init file */
		for (i=0; i < n_GRZparam; i++) scan_value(init, bin, 'd');

	}
	else GRZ_file=init;

	
	if (ok && read_mgmarray(ny, GRZ->GRZ_flag, GRZ_file, &(GRZ->GRZ_start_array)))
	{
		printf("Error reading first day of grazing\n");
		ok=0;
	}

	if (ok && read_mgmarray(ny, GRZ->GRZ_flag, GRZ_file, &(GRZ->GRZ_end_array)))
	{
		printf("Error reading first day of grazing\n");
		ok=0;
	}

	if (ok && read_mgmarray(ny, GRZ->GRZ_flag, GRZ_file, &(GRZ->weight_LSU)))
	{
		printf("Error reading livestock unit\n");
		ok=0;
	}

	if (ok && read_mgmarray(ny, GRZ->GRZ_flag, GRZ_file, &(GRZ->stocking_rate_array)))
	{
		printf("Error reading animal stocking rate\n");
		ok=0;
	}

	if (ok && read_mgmarray(ny, GRZ->GRZ_flag, GRZ_file, &(GRZ->DMintake_array)))
	{
		printf("Error reading last day of grazing\n");
		ok=0;
	}


	/* trampling effect */
	if (ok && read_mgmarray(ny, GRZ->GRZ_flag, GRZ_file, &(GRZ->trampling_effect)))
	{
		printf("Error reading trampling_effect\n");
		ok=0;
	}

	if (ok && read_mgmarray(ny, GRZ->GRZ_flag, GRZ_file, &(GRZ->prop_DMintake2excr_array)))
	{
		printf("Error reading prop. of the dry matter intake formed excrement\n");
		ok=0;
	}

	if (ok && read_mgmarray(ny, GRZ->GRZ_flag, GRZ_file, &(GRZ->prop_excr2litter_array)))
	{
		printf("Error reading prop. of excrement return to litter\n");
		ok=0;
	}

	if (ok && read_mgmarray(ny, GRZ->GRZ_flag, GRZ_file, &(GRZ->DM_Ccontent_array)))
	{
		printf("Error reading carbon content of dry matter\n");
		ok=0;
	}
		
	if (ok && read_mgmarray(ny, GRZ->GRZ_flag, GRZ_file, &(GRZ->EXCR_Ncontent_array)))
	{
		printf("Error reading EXCR_Ncontent_array\n");
		ok=0;
	}
			
	if (ok && read_mgmarray(ny, GRZ->GRZ_flag, GRZ_file, &(GRZ->EXCR_Ccontent_array)))
	{
		printf("Error reading EXCR_Ccontent_array\n");
		ok=0;
	}

	if (ok && read_mgmarray(ny, GRZ->GRZ_flag, GRZ_file, &(GRZ->EFman_N2O)))
	{
		printf("Error reading manure emission factor of N2O\n");
		ok=0;
	}

	if (ok && read_mgmarray(ny, GRZ->GRZ_flag, GRZ_file, &(GRZ->Nexrate)))
	{
		printf("Error reading manure emission factor of N2O\n");
		ok=0;
	}

	if (ok && read_mgmarray(ny, GRZ->GRZ_flag, GRZ_file, &(GRZ->EFman_CH4)))
	{
		printf("Error reading manure emission factor of CH4\n");
		ok=0;
	}

	if (ok && read_mgmarray(ny, GRZ->GRZ_flag, GRZ_file, &(GRZ->EFfer_CH4)))
	{
		printf("Error reading fermentation emission factor of CH4\n");
		ok=0;
	}


	if (GRZ->GRZ_flag == 2)
	{
		fclose (GRZ_file.ptr);
	}
		
	GRZ->mgmd = -1;

	return (!ok);	

}
