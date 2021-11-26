/* 
grazing_init.c
read grazinz information for pointbgc simulation

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


int grazing_init(file init, const control_struct* ctrl, grazing_struct* GRZ)
{
	char header[STRINGSIZE];
	char GRZ_filename[STRINGSIZE];
	file GRZ_file;

	int errorCode=0;
	int okFILE = 1;

	int mgmread;
	int nmgm = 0;

	int p1,p2,p3, p4,p5,p6;
	double p7,p8,p9,p10,p11,p12,p13,p14,p15,p16,p17,p18,p19;
	char tempvar;

	int n_GRZparam, maxGRZ_num;

	int* GRZstart_year_array;				
	int* GRZstart_month_array;					
	int* GRZstart_day_array;					
	int* GRZend_year_array;						
	int* GRZend_month_array;					
	int* GRZend_day_array;				
	double* trampling_effect;					
	double* weight_LSU;							
	double* stocking_rate_array;				
	double* DMintake_array;						
	double* DMintake2excr_array;			
	double* excr2litter_array;			
	double* DM_Ccontent_array;				
	double* EXCR_Ncontent_array;				
	double* EXCR_Ccontent_array;				
	double* Nexrate;                          
	double* EFman_N2O;					
	double* EFman_CH4;						
	double* EFfer_CH4;			

	maxGRZ_num=1000;

	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** GRAZING                                                       ** 
	**                                                                 **
	********************************************************************/
	
	/* header reading */
	if (!errorCode && scan_value(init, header, 's'))
	{
		printf("ERROR reading keyword, grazing_init()\n");
		errorCode=1;
	}

	/* keyword control */
	if (!errorCode && scan_value(init, header, 's'))
	{
		printf("ERROR reading keyword, grazing_init()\n");
		errorCode=1;
	}
	
	/* number of management action */
	if (!errorCode && scan_value(init, &GRZ->GRZ_num, 'i'))
	{
		printf("ERROR reading number of grazing in GRAZING section\n");
		errorCode=1;
	}


	/* if GRZ_num > 0 -> grazing */
	if (!errorCode && GRZ->GRZ_num)
	{
		/* allocate space for the temporary MGM array */ 
		GRZstart_year_array       = (int*) malloc(maxGRZ_num*sizeof(double));  
		GRZstart_month_array      = (int*) malloc(maxGRZ_num*sizeof(double)); 
		GRZstart_day_array        = (int*) malloc(maxGRZ_num*sizeof(double)); 
		GRZend_year_array         = (int*) malloc(maxGRZ_num*sizeof(double));  
		GRZend_month_array        = (int*) malloc(maxGRZ_num*sizeof(double)); 
		GRZend_day_array          = (int*) malloc(maxGRZ_num*sizeof(double)); 
		trampling_effect          = (double*) malloc(maxGRZ_num*sizeof(double)); 
		weight_LSU                = (double*) malloc(maxGRZ_num*sizeof(double)); 
		stocking_rate_array       = (double*) malloc(maxGRZ_num*sizeof(double)); 
		DMintake_array            = (double*) malloc(maxGRZ_num*sizeof(double));
		DMintake2excr_array       = (double*) malloc(maxGRZ_num*sizeof(double)); 
		excr2litter_array         = (double*) malloc(maxGRZ_num*sizeof(double)); 
		DM_Ccontent_array         = (double*) malloc(maxGRZ_num*sizeof(double)); 
		EXCR_Ncontent_array       = (double*) malloc(maxGRZ_num*sizeof(double));
		EXCR_Ccontent_array       = (double*) malloc(maxGRZ_num*sizeof(double)); 
		Nexrate                   = (double*) malloc(maxGRZ_num*sizeof(double)); 
		EFman_N2O                 = (double*) malloc(maxGRZ_num*sizeof(double)); 
		EFman_CH4                 = (double*) malloc(maxGRZ_num*sizeof(double));
		EFfer_CH4                 = (double*) malloc(maxGRZ_num*sizeof(double));
		

		if (!errorCode && scan_value(init, GRZ_filename, 's'))
		{
			printf("ERROR reading grazing calculating file\n");
			errorCode=1;
		}
		
		strcpy(GRZ_file.name, GRZ_filename);
		
		/* open the main init file for ascii read and check for errors */
		if (file_open(&GRZ_file,'i',1))
		{
			printf("ERROR opening GRZ_file, grazing_int.c\n");
			errorCode=1;
			okFILE=0;
		}

		if (!errorCode && scan_value(GRZ_file, header, 's'))
		{
			printf("ERROR reading header for GRAZING section in MANAGMENET file\n");
			errorCode=1;
		}

	
		while (!errorCode && !(mgmread = scan_array (GRZ_file, &p1, 'i', 0, 0)))
		{
			n_GRZparam = 22;
			mgmread = fscanf(GRZ_file.ptr, "%c%d%c%d%d%c%d%c%d%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf*[^\n]",&tempvar,&p2,&tempvar,&p3,&p4,&tempvar,&p5,&tempvar,&p6, 
				                                                      &p7,&p8,&p9,&p10,&p11,&p12,&p13,&p14,&p15,&p16,&p17,&p18,&p19);
			if (mgmread != n_GRZparam)
			{
				printf("ERROR reading GRAZING parameters from GRAZING file  file\n");
				errorCode=1;
			}

			if (p1 >= ctrl->simstartyear && p1 < ctrl->simstartyear + ctrl->simyears)
			{
						
				GRZstart_year_array[nmgm]       = p1;  
				GRZstart_month_array[nmgm]      = p2; 
				GRZstart_day_array[nmgm]        = p3; 
				GRZend_year_array[nmgm]         = p4;  
				GRZend_month_array[nmgm]        = p5; 
				GRZend_day_array[nmgm]          = p6; 
				weight_LSU[nmgm]                = p7; 
				stocking_rate_array[nmgm]       = p8; 
				DMintake_array[nmgm]            = p9;
				trampling_effect[nmgm]          = p10; 
				DMintake2excr_array[nmgm]       = p11; 
				excr2litter_array[nmgm]         = p12; 
				DM_Ccontent_array[nmgm]         = p13; 
				EXCR_Ncontent_array[nmgm]       = p14; 
				EXCR_Ccontent_array[nmgm]       = p15; 
				Nexrate[nmgm]                   = p16; 
				EFman_N2O[nmgm]                 = p17; 
				EFman_CH4[nmgm]                 = p18; 
				EFfer_CH4[nmgm]                 = p19; 
				
	
				nmgm += 1;
			}

		}
			
		GRZ->GRZ_num = nmgm;
		nmgm = 0;

		
		GRZ->GRZstart_year_array       = (int*) malloc(GRZ->GRZ_num*sizeof(double));  
		GRZ->GRZstart_month_array      = (int*) malloc(GRZ->GRZ_num*sizeof(double)); 
		GRZ->GRZstart_day_array        = (int*) malloc(GRZ->GRZ_num*sizeof(double)); 
		GRZ->GRZend_year_array         = (int*) malloc(GRZ->GRZ_num*sizeof(double));  
		GRZ->GRZend_month_array        = (int*) malloc(GRZ->GRZ_num*sizeof(double)); 
		GRZ->GRZend_day_array          = (int*) malloc(GRZ->GRZ_num*sizeof(double)); 
		GRZ->trampling_effect          = (double*) malloc(GRZ->GRZ_num*sizeof(double)); 
		GRZ->weight_LSU                = (double*) malloc(GRZ->GRZ_num*sizeof(double)); 
		GRZ->stocking_rate_array       = (double*) malloc(GRZ->GRZ_num*sizeof(double)); 
		GRZ->DMintake_array            = (double*) malloc(GRZ->GRZ_num*sizeof(double));
		GRZ->DMintake2excr_array       = (double*) malloc(GRZ->GRZ_num*sizeof(double)); 
		GRZ->excr2litter_array         = (double*) malloc(GRZ->GRZ_num*sizeof(double)); 
		GRZ->DM_Ccontent_array         = (double*) malloc(GRZ->GRZ_num*sizeof(double)); 
		GRZ->EXCR_Ncontent_array       = (double*) malloc(GRZ->GRZ_num*sizeof(double));
		GRZ->EXCR_Ccontent_array       = (double*) malloc(GRZ->GRZ_num*sizeof(double)); 
		GRZ->Nexrate                   = (double*) malloc(GRZ->GRZ_num*sizeof(double)); 
		GRZ->EFman_N2O                 = (double*) malloc(GRZ->GRZ_num*sizeof(double)); 
		GRZ->EFman_CH4                 = (double*) malloc(GRZ->GRZ_num*sizeof(double));
		GRZ->EFfer_CH4                 = (double*) malloc(GRZ->GRZ_num*sizeof(double));

		for (nmgm = 0; nmgm < GRZ->GRZ_num; nmgm++)
		{
			GRZ->GRZstart_year_array[nmgm]       = GRZstart_year_array[nmgm];  
			GRZ->GRZstart_month_array[nmgm]      = GRZstart_month_array[nmgm]; 
			GRZ->GRZstart_day_array[nmgm]        = GRZstart_day_array[nmgm]; 
			GRZ->GRZend_year_array[nmgm]         = GRZend_year_array[nmgm] ;  
			GRZ->GRZend_month_array[nmgm]        = GRZend_month_array[nmgm] ; 
			GRZ->GRZend_day_array[nmgm]          = GRZend_day_array[nmgm]; 
			GRZ->trampling_effect[nmgm]          = trampling_effect[nmgm]; 
			GRZ->weight_LSU[nmgm]                = weight_LSU[nmgm]  ; 
			GRZ->stocking_rate_array[nmgm]       = stocking_rate_array[nmgm] ; 
			GRZ->DMintake_array[nmgm]            = DMintake_array[nmgm] ;
			GRZ->DMintake2excr_array[nmgm]       = DMintake2excr_array[nmgm] ; 
			GRZ->excr2litter_array[nmgm]         = excr2litter_array[nmgm]; 
			GRZ->DM_Ccontent_array[nmgm]         = DM_Ccontent_array[nmgm]  ; 
			GRZ->EXCR_Ncontent_array[nmgm]       = EXCR_Ncontent_array[nmgm] ; 
			GRZ->EXCR_Ccontent_array[nmgm]       = EXCR_Ccontent_array[nmgm] ; 
			GRZ->Nexrate[nmgm]                   = Nexrate[nmgm]; 
			GRZ->EFman_N2O[nmgm]                 = EFman_N2O[nmgm]  ; 
			GRZ->EFman_CH4[nmgm]                 = EFman_CH4[nmgm] ; 
			GRZ->EFfer_CH4[nmgm]                 = EFfer_CH4[nmgm] ; 
		}

		/* close GRAZING file and free temporary memory */
		if (okFILE) fclose (GRZ_file.ptr);

		free(GRZstart_year_array);				
		free(GRZstart_month_array);					
		free(GRZstart_day_array);					
		free(GRZend_year_array);						
		free(GRZend_month_array);					
		free(GRZend_day_array);				
		free(trampling_effect);					
		free(weight_LSU);							
		free(stocking_rate_array);				
		free(DMintake_array);						
		free(DMintake2excr_array);			
		free(excr2litter_array);			
		free(DM_Ccontent_array);				
		free(EXCR_Ncontent_array);				
		free(EXCR_Ccontent_array);				
		free(Nexrate);                          
		free(EFman_N2O);					
		free(EFman_CH4);						
		free(EFfer_CH4);			

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

	
	
	GRZ->mgmdGRZ = 0;

	
	return (errorCode);	

}
