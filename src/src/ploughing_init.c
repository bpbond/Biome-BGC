/* 
ploughing_init.c
read ploughing information for pointbgc simulation

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


int ploughing_init(file init, const control_struct* ctrl, ploughing_struct* PLG)
{
	char header[STRINGSIZE];
	char PLG_filename[STRINGSIZE];
	file PLG_file;

	int errorCode=0;
	int okFILE = 1;

	int mgmread;
	int nmgm = 0;

	int p1,p2,p3;
	double p4;
	char tempvar;

	int n_PLGparam, maxPLG_num;

	int* PLGyear_array;			
	int* PLGmonth_array;						
	int* PLGday_array;							
	double* PLGdepths_array;				

	maxPLG_num=1000;


	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** PLOUGHING                                                       ** 
	**                                                                 **
	********************************************************************/
	
	/* header reading */
	if (!errorCode && scan_value(init, header, 's'))
	{
		printf("ERROR reading keyword, ploughing_init()\n");
		errorCode=1;
	}

	/* header control */
	if (!errorCode && scan_value(init, header, 's'))
	{
		printf("ERROR reading keyword, ploughing_init()\n");
		errorCode=1;
	}

	
	/* number of management action */
	if (!errorCode && scan_value(init, &PLG->PLG_num, 'i'))
	{
		printf("ERROR reading number of ploughing in PLOUGHING section\n");
		errorCode=1;
	}


	/* if PLG_num > 0 -> ploughing */
	if (!errorCode && PLG->PLG_num)
	{
		/* allocate space for the temporary MGM array */
		PLGyear_array         = (int*) malloc(maxPLG_num*sizeof(double));  
		PLGmonth_array        = (int*) malloc(maxPLG_num*sizeof(double)); 
		PLGday_array          = (int*) malloc(maxPLG_num*sizeof(double)); 
		PLGdepths_array       = (double*) malloc(maxPLG_num*sizeof(double)); 
		
		if (!errorCode && scan_value(init, PLG_filename, 's'))
		{
			printf("ERROR reading ploughing calculating file\n");
			errorCode=1;
		}
		
		strcpy(PLG_file.name, PLG_filename);
		
		/* open the main init file for ascii read and check for errors */
		if (file_open(&PLG_file,'i',1))
		{
			printf("ERROR opening PLG_file, ploughing_int.c\n");
			errorCode=1;
			okFILE=0;
		}

		if (!errorCode && scan_value(PLG_file, header, 's'))
		{
			printf("ERROR reading header for PLOUGHING section in MANAGMENET file\n");
			errorCode=1;
		}

	
		while (!errorCode && !(mgmread = scan_array (PLG_file, &p1, 'i', 0, 0)))
		{
			n_PLGparam = 5;

			mgmread = fscanf(PLG_file.ptr, "%c%d%c%d%lf%*[^\n]",&tempvar,&p2,&tempvar,&p3,&p4);
			if (mgmread != n_PLGparam)
			{
				printf("ERROR reading PLOUGHING parameters from PLOUGHING file  file\n");
				errorCode=1;
			}

			if (p1 >= ctrl->simstartyear && p1 < ctrl->simstartyear + ctrl->simyears)
			{
				PLGyear_array[nmgm]         = p1;
				PLGmonth_array[nmgm]        = p2;
				PLGday_array[nmgm]          = p3;
				PLGdepths_array[nmgm]       = p4;

				nmgm += 1;
			}
		}
			
		PLG->PLG_num = nmgm;
		nmgm = 0;

		
		PLG->PLGyear_array         = (int*) malloc(PLG->PLG_num*sizeof(double));  
		PLG->PLGmonth_array        = (int*) malloc(PLG->PLG_num*sizeof(double)); 
		PLG->PLGday_array          = (int*) malloc(PLG->PLG_num*sizeof(double)); 
		PLG->PLGdepths_array       = (double*) malloc(PLG->PLG_num*sizeof(double)); 
			
		for (nmgm = 0; nmgm < PLG->PLG_num; nmgm++)
		{
			PLG->PLGyear_array[nmgm]         = PLGyear_array[nmgm];
			PLG->PLGmonth_array[nmgm]        = PLGmonth_array[nmgm] ;
			PLG->PLGday_array[nmgm]          = PLGday_array[nmgm]  ;
			PLG->PLGdepths_array[nmgm]       = PLGdepths_array[nmgm] ;
		}

		/* close PLOUGHING file and free temporary memory*/
		if (okFILE) fclose (PLG_file.ptr);

		free(PLGyear_array);			
		free(PLGmonth_array);						
		free(PLGday_array);							
		free(PLGdepths_array);	
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

	
	PLG->mgmdPLG = 0;

	
	
	return (errorCode);
}
