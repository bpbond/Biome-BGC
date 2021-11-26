/* 
thinning_init.c
read thinning information for pointbgc simulation

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


int thinning_init(file init, const control_struct* ctrl, thinning_struct* THN)
{

	char header[STRINGSIZE];
	char THN_filename[STRINGSIZE];
	file THN_file;

	int errorCode=0;
	int okFILE = 1;

	int mgmread;
	int nmgm = 0;

	int p1,p2,p3;
	double p4,p5,p6,p7;
	char tempvar;

	int n_THNparam, maxTHN_num;
	
	int* THNyear_array;				
	int* THNmonth_array;					
	int* THNday_array;						
	double* thinningrate_w_array;				
	double* thinningrate_nw_array;				
	double* transpcoeff_w_array;			
	double* transpcoeff_nw_array;			
	
	maxTHN_num=1000;

	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** THINNING                                                       ** 
	**                                                                 **
	********************************************************************/
	
	/* header reading */
	if (!errorCode && scan_value(init, header, 's'))
	{
		printf("ERROR reading keyword, thinning_init()\n");
		errorCode=1;
	}

	/* keyword control */
	if (!errorCode && scan_value(init, header, 's'))
	{
		printf("ERROR reading keyword, thinning_init()\n");
		errorCode=1;
	}

	
	/* number of management action */
	if (!errorCode && scan_value(init, &THN->THN_num, 'i'))
	{
		printf("ERROR reading number of thinning in THINNING section\n");
		errorCode=1;
	}


	/* if THN_num > 0 -> thinning */
	if (!errorCode && THN->THN_num)
	{
		/* allocate space for the temporary MGM array */
		THNyear_array         = (int*) malloc(maxTHN_num*sizeof(double));  
		THNmonth_array        = (int*) malloc(maxTHN_num*sizeof(double)); 
		THNday_array          = (int*) malloc(maxTHN_num*sizeof(double)); 
		thinningrate_w_array  = (double*) malloc(maxTHN_num*sizeof(double)); 
		thinningrate_nw_array = (double*) malloc(maxTHN_num*sizeof(double)); 
		transpcoeff_w_array   = (double*) malloc(maxTHN_num*sizeof(double)); 
		transpcoeff_nw_array  = (double*) malloc(maxTHN_num*sizeof(double)); 
		
		if (!errorCode && scan_value(init, THN_filename, 's'))
		{
			printf("ERROR reading thinning calculating file\n");
			errorCode=1;
		}
		
		strcpy(THN_file.name, THN_filename);
		
		/* open the main init file for ascii read and check for errors */
		if (file_open(&THN_file,'i',1))
		{
			printf("ERROR opening THN_file, thinning_int.c\n");
			errorCode=1;
			okFILE=0;
		}

		if (!errorCode && scan_value(THN_file, header, 's'))
		{
			printf("ERROR reading header for THINNING section in MANAGMENET file\n");
			errorCode=1;
		}

	
		while (!errorCode && !(mgmread = scan_array (THN_file, &p1, 'i', 0, 0)))
		{
			n_THNparam = 8;
 			mgmread = fscanf(THN_file.ptr, "%c%d%c%d%lf%lf%lf%lf%*[^\n]",&tempvar,&p2,&tempvar,&p3,&p4,&p5,&p6,&p7);
			if (mgmread != n_THNparam)
			{
				printf("ERROR reading THINNING parameters from THINNING file  file\n");
				errorCode=1;
			}

			if (p1 >= ctrl->simstartyear && p1 < ctrl->simstartyear + ctrl->simyears)
			{
				THNyear_array[nmgm]         = p1;
				THNmonth_array[nmgm]        = p2;
				THNday_array[nmgm]          = p3;
				thinningrate_w_array[nmgm]  = p4;
				thinningrate_nw_array[nmgm] = p5;
				transpcoeff_w_array[nmgm]   = p6;
				transpcoeff_nw_array[nmgm]  = p7;

				nmgm += 1;
			}
		}
		
		THN->THN_num = nmgm;
		nmgm = 0;
		
		THN->THNyear_array         = (int*) malloc(THN->THN_num*sizeof(double));  
		THN->THNmonth_array        = (int*) malloc(THN->THN_num*sizeof(double)); 
		THN->THNday_array          = (int*) malloc(THN->THN_num*sizeof(double)); 
		THN->thinningrate_w_array  = (double*) malloc(THN->THN_num*sizeof(double)); 
		THN->thinningrate_nw_array = (double*) malloc(THN->THN_num*sizeof(double)); 
		THN->transpcoeff_w_array   = (double*) malloc(THN->THN_num*sizeof(double)); 
		THN->transpcoeff_nw_array  = (double*) malloc(THN->THN_num*sizeof(double)); 

		for (nmgm = 0; nmgm < THN->THN_num; nmgm++)
		{
			THN->THNyear_array[nmgm]         = THNyear_array[nmgm];
			THN->THNmonth_array[nmgm]        = THNmonth_array[nmgm] ;
			THN->THNday_array[nmgm]          = THNday_array[nmgm];
			THN->thinningrate_w_array[nmgm]  = thinningrate_w_array[nmgm];
			THN->thinningrate_nw_array[nmgm] = thinningrate_nw_array[nmgm];
			THN->transpcoeff_w_array[nmgm]   = transpcoeff_w_array[nmgm] ;
			THN->transpcoeff_nw_array[nmgm]  = transpcoeff_nw_array[nmgm];
		}


		/* close THINNING file and free temporary memory*/
		if (okFILE) fclose (THN_file.ptr);

		free(THNyear_array);				
		free(THNmonth_array);					
		free(THNday_array);						
		free(thinningrate_w_array);				
		free(thinningrate_nw_array);				
		free(transpcoeff_w_array);			
		free(transpcoeff_nw_array);	
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

	
	
	THN->mgmdTHN = 0;


	
	return (errorCode);
}
