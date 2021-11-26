/* 
harvesting_init.c
read harvesting information for pointbgc simulation

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


int harvesting_init(file init, const control_struct* ctrl, planting_struct* PLT, harvesting_struct* HRV)
{
	char header[STRINGSIZE];
	char HRV_filename[STRINGSIZE];
	file HRV_file;

	int errorCode=0;
	int okFILE = 1;

	int mgmread;
	int nmgm = 0;

	int p1,p2,p3;
	double p4,p5;
	char tempvar;

	int n_HRVparam, maxHRV_num, PLTyday, HRVyday;

	int* HRVyear_array;						
	int* HRVmonth_array;						
	int* HRVday_array;							
	double* snagprop_array;					
	double* transportHRV_array;		

	maxHRV_num=1000;

	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** HARVESTING                                                       ** 
	**                                                                 **
	********************************************************************/
	
	/* header reading */
	if (!errorCode && scan_value(init, header, 's'))
	{
		printf("ERROR reading keyword, harvesting_init()\n");
		errorCode=1;
	}

	/* keyword control */
	if (!errorCode && scan_value(init, header, 's'))
	{
		printf("ERROR reading keyword for HARVESTING section\n");
		errorCode=1;
	}
	
	/* number of management action */
	if (!errorCode && scan_value(init, &HRV->HRV_num, 'i'))
	{
		printf("ERROR reading number of harvesting in HARVESTING section\n");
		errorCode=1;
	}


	/* if HRV_num > 0 -> harvesting */
	if (!errorCode && HRV->HRV_num)
	{
		/* allocate space for the temporary MGM array */
		HRVyear_array         = (int*) malloc(maxHRV_num*sizeof(double));  
		HRVmonth_array        = (int*) malloc(maxHRV_num*sizeof(double)); 
		HRVday_array          = (int*) malloc(maxHRV_num*sizeof(double)); 
		snagprop_array        = (double*) malloc(maxHRV_num*sizeof(double)); 
		transportHRV_array    = (double*) malloc(maxHRV_num*sizeof(double)); 
		
		if (!errorCode && scan_value(init, HRV_filename, 's'))
		{
			printf("ERROR reading harvesting calculating file\n");
			errorCode=1;
		}
		
		strcpy(HRV_file.name, HRV_filename);
		
		/* open the main init file for ascii read and check for errors */
		if (file_open(&HRV_file,'i',1))
		{
			printf("ERROR opening HRV_file, harvesting_int.c\n");
			errorCode=1;
			okFILE=0;
		}

		if (!errorCode && scan_value(HRV_file, header, 's'))
		{
			printf("ERROR reading header for HARVESTING section in MANAGMENET file\n");
			errorCode=1;
		}

	
		while (!errorCode && !(mgmread = scan_array (HRV_file, &p1, 'i', 0, 0)))
		{
			n_HRVparam = 6;
			mgmread = fscanf(HRV_file.ptr, "%c%d%c%d%lf%lf%*[^\n]",&tempvar,&p2,&tempvar,&p3,&p4,&p5);
			if (mgmread != n_HRVparam)
			{
				printf("ERROR reading HARVESTING parameters from HARVESTING file  file\n");
				errorCode=1;
			}

			if (p1 >= ctrl->simstartyear && p1 < ctrl->simstartyear + ctrl->simyears)
			{
				PLTyday = PLT->PLTyear_array[0] * nDAYS_OF_YEAR + date_to_doy(PLT->PLTmonth_array[0], PLT->PLTday_array[0]);
				HRVyday = p1 * nDAYS_OF_YEAR + date_to_doy(p2, p3);
				if (HRVyday > PLTyday)
				{
					HRVyear_array[nmgm]         = p1;
					HRVmonth_array[nmgm]        = p2;
					HRVday_array[nmgm]          = p3;
					snagprop_array[nmgm]        = p4;
					transportHRV_array[nmgm]    = p5;

					nmgm += 1;
				}
			}
		}

		HRV->HRV_num = nmgm;
		nmgm = 0;
			
		
		HRV->HRVyear_array         = (int*) malloc(HRV->HRV_num*sizeof(double));  
		HRV->HRVmonth_array        = (int*) malloc(HRV->HRV_num*sizeof(double)); 
		HRV->HRVday_array          = (int*) malloc(HRV->HRV_num*sizeof(double)); 
		HRV->snagprop_array        = (double*) malloc(HRV->HRV_num*sizeof(double)); 
		HRV->transportHRV_array    = (double*) malloc(HRV->HRV_num*sizeof(double)); 

		for (nmgm = 0; nmgm < HRV->HRV_num; nmgm++)
		{		
			HRV->HRVyear_array[nmgm]         = HRVyear_array[nmgm];
			HRV->HRVmonth_array[nmgm]        = HRVmonth_array[nmgm] ;
			HRV->HRVday_array[nmgm]          = HRVday_array[nmgm];
			HRV->snagprop_array[nmgm]        = snagprop_array[nmgm] ;
			HRV->transportHRV_array[nmgm]    = transportHRV_array[nmgm];
		}

		/* close HARVESTING file and free temporary memory*/
		if (okFILE) fclose (HRV_file.ptr);

		free(HRVyear_array);						
		free(HRVmonth_array);						
		free(HRVday_array);							
		free(snagprop_array);					
		free(transportHRV_array);		
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

	
	
	HRV->mgmdHRV = 0;

	

	return (errorCode);
}
