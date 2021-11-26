/* 
mowing_init.c
read mowing information for pointbgc simulation

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


int mowing_init(file init, const control_struct* ctrl, mowing_struct* MOW)
{

	char header[STRINGSIZE];
	char MOW_filename[STRINGSIZE];
	file MOW_file;

	int errorCode=0;
	int okFILE = 1;

	int mgmread;
	int nmgm = 0;

	int p1,p2,p3;
	double p4,p5;
	char tempvar;

	int n_MOWparam, maxMOW_num;

	int* MOWyear_array;							
	int* MOWmonth_array;					
	int* MOWday_array;						
	double* LAI_limit_array;					
	double* transportMOW_array;					

	maxMOW_num=1000;

	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** MOWING                                                       ** 
	**                                                                 **
	********************************************************************/
	
	/* header reading */
	if (!errorCode && scan_value(init, header, 's'))
	{
		printf("ERROR reading keyword, mowing_init()\n");
		errorCode=1;
	}

	/* keyword control */
	if (!errorCode && scan_value(init, header, 's'))
	{
		printf("ERROR reading keyword for MOWING section\n");
		errorCode=1;
	}
	
	/* number of management action */
	if (!errorCode && scan_value(init, &MOW->MOW_num, 'i'))
	{
		printf("ERROR reading number of mowing in MOWING section\n");
		errorCode=1;
	}


	/* if MOW_num > 0 -> mowing */
	if (!errorCode && MOW->MOW_num)
	{
		/* allocate space for the temporary MGM array */
		MOWyear_array         = (int*) malloc(maxMOW_num*sizeof(double));  
		MOWmonth_array        = (int*) malloc(maxMOW_num*sizeof(double)); 
		MOWday_array          = (int*) malloc(maxMOW_num*sizeof(double)); 
		LAI_limit_array	      = (double*) malloc(maxMOW_num*sizeof(double)); 
		transportMOW_array    = (double*) malloc(maxMOW_num*sizeof(double)); 

		
		if (!errorCode && scan_value(init, MOW_filename, 's'))
		{
			printf("ERROR reading mowing calculating file\n");
			errorCode=1;
		}
		
		strcpy(MOW_file.name, MOW_filename);
		
		/* open the main init file for ascii read and check for errors */
		if (file_open(&MOW_file,'i',1))
		{
			printf("ERROR opening MOW_file, mowing_int.c\n");
			errorCode=1;
			okFILE=0;
		}

		if (!errorCode && scan_value(MOW_file, header, 's'))
		{
			printf("ERROR reading header for MOWING section in MANAGMENET file\n");
			errorCode=1;
		}

	
		while (!errorCode && !(mgmread = scan_array (MOW_file, &p1, 'i', 0, 0)))
		{
			n_MOWparam = 6;
			mgmread = fscanf(MOW_file.ptr, "%c%d%c%d%lf%lf%*[^\n]",&tempvar,&p2,&tempvar,&p3,&p4,&p5);
			if (mgmread != n_MOWparam)
			{
				printf("ERROR reading MOWING parameters from MOWING file\n");
				errorCode=1;
			}
			
			if (p1 >= ctrl->simstartyear && p1 < ctrl->simstartyear + ctrl->simyears)
			{
				MOWyear_array[nmgm]         = p1;
				MOWmonth_array[nmgm]        = p2;
				MOWday_array[nmgm]          = p3;
				LAI_limit_array[nmgm] 	     = p4; 
				transportMOW_array[nmgm]    = p5; 

				nmgm += 1;
			}
		}
			
		MOW->MOW_num = nmgm;
		nmgm = 0;
		
		MOW->MOWyear_array         = (int*) malloc(MOW->MOW_num*sizeof(double));  
		MOW->MOWmonth_array        = (int*) malloc(MOW->MOW_num*sizeof(double)); 
		MOW->MOWday_array          = (int*) malloc(MOW->MOW_num*sizeof(double)); 
		MOW->LAI_limit_array	   = (double*) malloc(MOW->MOW_num*sizeof(double)); 
		MOW->transportMOW_array    = (double*) malloc(MOW->MOW_num*sizeof(double)); 
			
		for (nmgm = 0; nmgm < MOW->MOW_num; nmgm++)
		{
			MOW->MOWyear_array[nmgm]         = MOWyear_array[nmgm];
			MOW->MOWmonth_array[nmgm]        = MOWmonth_array[nmgm];
			MOW->MOWday_array[nmgm]          = MOWday_array[nmgm];
			MOW->LAI_limit_array[nmgm] 	     = LAI_limit_array[nmgm]; 
			MOW->transportMOW_array[nmgm]    = transportMOW_array[nmgm]; 
		}
		
		/* close MOWING file and free temporary memory*/
		if (okFILE) fclose (MOW_file.ptr);

		free(MOWyear_array);							
		free(MOWmonth_array);					
		free(MOWday_array);						
		free(LAI_limit_array);					
		free(transportMOW_array);		
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

	
	
	MOW->mgmdMOW = 0;

	
	
	return (errorCode);
}
