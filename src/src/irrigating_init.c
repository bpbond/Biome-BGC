/* 
irrigating_init.c
read irrigating information for pointbgc simulation

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


int irrigating_init(file init, const control_struct* ctrl, irrigating_struct* IRG)
{
	
	char header[STRINGSIZE];
	char IRG_filename[STRINGSIZE];
	file IRG_file;

	int errorCode=0;
	int okFILE = 1;

	int mgmread;
	int nmgm = 0;

	int p1,p2,p3;
	double p4;
	char tempvar;


	int n_IRGparam, maxIRG_num;

    int* IRGyear_array;	
	int* IRGmonth_array;	
	int* IRGday_array;							
	double* IRGquantity_array;					

	maxIRG_num=1000;



	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** IRRIGATING                                                       ** 
	**                                                                 **
	********************************************************************/
	
	/* header reading */
	if (!errorCode && scan_value(init, header, 's'))
	{
		printf("ERROR reading keyword, irrigating_init()\n");
		errorCode=1;
	}

	/* keyword control */
	if (!errorCode && scan_value(init, header, 's'))
	{
		printf("ERROR reading keyword, irrigating_init()\n");
		errorCode=1;
	}
	
	/* number of management action */
	if (!errorCode && scan_value(init, &IRG->IRG_num, 'i'))
	{
		printf("ERROR reading number of irrigating in IRRIGATING section\n");
		errorCode=1;
	}


	/* if IRG_num > 0 -> irrigating */
	if (IRG->IRG_num)
	{
		/* allocate space for the temporary MGM array */
		IRGyear_array      = (int*) malloc(maxIRG_num*sizeof(double));  
		IRGmonth_array     = (int*) malloc(maxIRG_num*sizeof(double)); 
		IRGday_array       = (int*) malloc(maxIRG_num*sizeof(double)); 
		IRGquantity_array  = (double*) malloc(maxIRG_num*sizeof(double)); 
	
		
		if (!errorCode && scan_value(init, IRG_filename, 's'))
		{
			printf("ERROR reading irrigating calculating file\n");
			errorCode=1;
		}
		
		strcpy(IRG_file.name, IRG_filename);
		
		/* open the main init file for ascii read and check for errors */
		if (file_open(&IRG_file,'i',1))
		{
			printf("ERROR opening IRG_file, irrigating_int.c\n");
			errorCode=1;
			okFILE=0;
		}

		if (!errorCode && scan_value(IRG_file, header, 's'))
		{
			printf("ERROR reading header for IRRIGATING section in MANAGMENET file\n");
			errorCode=1;
		}

	
		while (!errorCode && !(mgmread = scan_array (IRG_file, &p1, 'i', 0, 0)))
		{
			n_IRGparam = 5;

			mgmread = fscanf(IRG_file.ptr, "%c%d%c%d%lf%*[^\n]",&tempvar,&p2,&tempvar,&p3,&p4);
			if (mgmread != n_IRGparam)
			{
				printf("ERROR reading IRRIGATING parameters from IRRIGATING file  file\n");
				errorCode=1;
			}
			
			if (p1 >= ctrl->simstartyear && p1 < ctrl->simstartyear + ctrl->simyears)
			{
				IRGyear_array[nmgm]      = p1;
				IRGmonth_array[nmgm]     = p2;
				IRGday_array[nmgm]       = p3;
				IRGquantity_array[nmgm]  = p4;

				nmgm += 1;
			}

		}	
	
		IRG->IRG_num = nmgm;
		nmgm = 0;
	
		IRG->IRGyear_array      = (int*) malloc(IRG->IRG_num*sizeof(double));  
		IRG->IRGmonth_array     = (int*) malloc(IRG->IRG_num*sizeof(double)); 
		IRG->IRGday_array       = (int*) malloc(IRG->IRG_num*sizeof(double)); 
		IRG->IRGquantity_array  = (double*) malloc(IRG->IRG_num*sizeof(double)); 

		for (nmgm = 0; nmgm < IRG->IRG_num; nmgm++)
		{
			IRG->IRGyear_array[nmgm]      = IRGyear_array[nmgm];
			IRG->IRGmonth_array[nmgm]     = IRGmonth_array[nmgm];
			IRG->IRGday_array[nmgm]       = IRGday_array[nmgm];
			IRG->IRGquantity_array[nmgm]  = IRGquantity_array[nmgm];
		}

		/* close IRRIGATING file and free temporary memory*/
		if (okFILE) fclose (IRG_file.ptr);

		free(IRGyear_array);	
		free(IRGmonth_array);	
		free(IRGday_array);							
		free(IRGquantity_array);		
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

	
	
	IRG->mgmdIRG = 0;
	
	return (errorCode);
}
