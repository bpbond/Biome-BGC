/* 
planting_init.c
read planting information for pointbgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Copyright 2020, D. Hidy [dori.hidy@gmail.com]
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


int planting_init(file init, control_struct* ctrl, planting_struct* PLT, epconst_struct* epc)
{
	
	char header[STRINGSIZE];
	char PLT_filename[STRINGSIZE];
	file PLT_file;

	int maxlen=STRINGSIZE;

	int errorCode=0;
	int okFILE = 1;

	

	int mgmread;
	int nmgm = 0;

	int p1,p2,p3;
	double p4,p5,p6,p7;
	char tempvar;
	char cropfile[STRINGSIZE];

	int n_PLTparam, maxPLT_num;

	int* PLTyear_array;					
	int* PLTmonth_array;				
    int* PLTday_array;							
	double* germ_depth_array;			      
	double* n_seedlings_array;				    
	double* weight_1000seed_array;				
	double* seed_carbon_array;				
	char** filename_array;						

	maxPLT_num=1000;


	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** PLANTING                                                       ** 
	**                                                                 **
	********************************************************************/
	
	/* header reading */
	if (!errorCode && scan_value(init, header, 's'))
	{
		printf("ERROR reading header, planting_init()\n");
		errorCode=1;
	}
	

	/* keyword control */
	if (!errorCode && scan_value(init, header, 's'))
	{
		printf("ERROR reading header, planting_init()\n");
		errorCode=1;
	}
	
	/* number of management action */
	if (!errorCode && scan_value(init, &PLT->PLT_num, 'i'))
	{
		printf("ERROR reading number of planting in PLANTING section\n");
		errorCode=1;
	}


	/* if PLT_num = 1 -> planting */
	if (!errorCode && PLT->PLT_num)
	{
		/* allocate space for the MGM array */
		PLTyear_array         = (int*) malloc(maxPLT_num*sizeof(int));  
		PLTmonth_array        = (int*) malloc(maxPLT_num*sizeof(int)); 
		PLTday_array          = (int*) malloc(maxPLT_num*sizeof(int)); 
		germ_depth_array      = (double*) malloc(maxPLT_num*sizeof(double)); 
		n_seedlings_array     = (double*) malloc(maxPLT_num*sizeof(double)); 
		weight_1000seed_array = (double*) malloc(maxPLT_num*sizeof(double)); 
		seed_carbon_array     = (double*) malloc(maxPLT_num*sizeof(double)); 
		filename_array        = (char**) malloc(maxPLT_num * sizeof(char*));
		
		if (!errorCode && scan_value(init, PLT_filename, 's'))
		{
			printf("ERROR reading planting calculating file\n");
			errorCode=1;
		}
		
		strcpy(PLT_file.name, PLT_filename);
		
		/* open the main init file for ascii read and check for errors */
		if (file_open(&PLT_file,'i',1))
		{
			printf("ERROR opening PLT_file, planting_int.c\n");
			errorCode=1;
			okFILE=0;
		}

		if (!errorCode && scan_value(PLT_file, header, 's'))
		{
			printf("ERROR reading header for PLANTING section in MANAGMENET file\n");
			errorCode=1;
		}

	
		while (!errorCode && !(mgmread = scan_array (PLT_file, &p1, 'i', 0, 0)))
		{
			filename_array[nmgm] = (char*) malloc(STRINGSIZE * sizeof(char));

			n_PLTparam = 9;

			mgmread = fscanf(PLT_file.ptr, "%c%d%c%d%lf%lf%lf%lf%s%*[^\n]",&tempvar,&p2,&tempvar,&p3,&p4,&p5,&p6,&p7,(char*)&cropfile);

			if (mgmread != n_PLTparam)
			{
				printf("ERROR reading PLANTING parameters from PLANTING file\n");
				errorCode=1;
			}
			if ((int) sizeof(cropfile) > maxlen)
			{
				printf("ERROR reading length of filename in planting file (lenght must be in the range 1-100)\n");
				errorCode=1;
			}

			if (p1 >= ctrl->simstartyear && p1 < ctrl->simstartyear + ctrl->simyears)
			{
				PLTyear_array[nmgm]         = p1;
				PLTmonth_array[nmgm]        = p2;
				PLTday_array[nmgm]          = p3;
				germ_depth_array[nmgm]      = p4;
				n_seedlings_array[nmgm]     = p5;
				weight_1000seed_array[nmgm] = p6;
				seed_carbon_array[nmgm]     = p7;
				strcpy(filename_array[nmgm], cropfile);

				nmgm += 1;
			}
		}

		PLT->PLT_num = nmgm;
		nmgm = 0;
			

		/* allocate space for the temporary MGM array */
		PLT->PLTyear_array         = (int*) malloc(PLT->PLT_num*sizeof(int));  
		PLT->PLTmonth_array        = (int*) malloc(PLT->PLT_num*sizeof(int)); 
		PLT->PLTday_array          = (int*) malloc(PLT->PLT_num*sizeof(int)); 
		PLT->germ_depth_array      = (double*) malloc(PLT->PLT_num*sizeof(double)); 
		PLT->n_seedlings_array     = (double*) malloc(PLT->PLT_num*sizeof(double)); 
		PLT->weight_1000seed_array = (double*) malloc(PLT->PLT_num*sizeof(double)); 
		PLT->seed_carbon_array     = (double*) malloc(PLT->PLT_num*sizeof(double)); 
		PLT->filename_array        = (char**) malloc(PLT->PLT_num * sizeof(char*));

		for (nmgm = 0; nmgm < PLT->PLT_num; nmgm++)
		{

			PLT->PLTyear_array[nmgm]         = PLTyear_array[nmgm];
			PLT->PLTmonth_array[nmgm]        = PLTmonth_array[nmgm];
			PLT->PLTday_array[nmgm]          = PLTday_array[nmgm];
			PLT->germ_depth_array[nmgm]      = germ_depth_array[nmgm];
			PLT->n_seedlings_array[nmgm]     = n_seedlings_array[nmgm] ;
			PLT->weight_1000seed_array[nmgm] = weight_1000seed_array[nmgm];
			PLT->seed_carbon_array[nmgm]     = seed_carbon_array[nmgm];

			PLT->filename_array[nmgm] = (char*) malloc(STRINGSIZE * sizeof(char));
			strcpy(PLT->filename_array[nmgm], filename_array[nmgm]);
	
		}

		/* close PLANTING file  and free temporary memory*/
		if (okFILE) fclose (PLT_file.ptr);

		free(PLTyear_array);
		free(PLTmonth_array);
		free(PLTday_array);
		free(germ_depth_array);
		free(n_seedlings_array);
		free(weight_1000seed_array);
		free(seed_carbon_array);
		free(filename_array);
	}
	else
	{
		/* reading the line of management file into a temporary variable */
		if (!errorCode && scan_value(init, header, 's'))
		{
			printf("ERROR reading line of management file (in case of no management)\n");
			errorCode=1;
		}
	}


	/* conrtol */
	if (!errorCode && PLT->PLT_num && epc->n_germ_phenophase == 0)
	{
		printf("ERROR in phenophase parametrization: if PLANTING is defined -> n_germ_phenophase must be specified in EPC file\n");
		errorCode=1;
	}

	
	PLT->mgmdPLT = 0;

	
	return (errorCode);
 }
