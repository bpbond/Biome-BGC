/* 
groundwater_init.c
read groundwater depth information if it is available

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



int groundwater_init(groundwater_struct* gws, control_struct* ctrl)
{
	int errorCode=0;
	file GWD_file;	

	int dataread;
	int ndata = 0;

	int p1,p2,p3, maxGWD_num, nmgm;
	double p4;
	char tempvar;

	int* GWyear_array;			
	int* GWmonth_array;						
	int* GWday_array;
	double* GWdepth_array;

	nmgm=0;
	maxGWD_num=ctrl->simyears* nDAYS_OF_YEAR;
	gws->GWD_num = 0;
	/* ------------------------------------------------------ */
	/* using varying whole plant mortality values */
	
	/* GWD flag: constans or varying GWD from file */

	if (!errorCode)
	{
		if (ctrl->spinup == 0)   /* normal run */
		{
			strcpy(GWD_file.name, "groundwater_normal.txt");
			if (!file_open(&GWD_file,'j',1)) gws->GWD_num = 1;
		}
		else                     /* spinup and transient run */        
		{ 	
			strcpy(GWD_file.name, "groundwater_spinup.txt");
			if (!file_open(&GWD_file,'j',1)) gws->GWD_num = 1;	
		}
	}


	if (!errorCode && gws->GWD_num > 0) 
	{		
		

		/* allocate space for the temporary MGM array */
		GWyear_array    = (int*) malloc(maxGWD_num*sizeof(int));  
		GWmonth_array   = (int*) malloc(maxGWD_num*sizeof(int)); 
		GWday_array     = (int*) malloc(maxGWD_num*sizeof(int)); 
        GWdepth_array   = (double*) malloc(maxGWD_num*sizeof(double)); 

		
		ndata=0;
		while (!errorCode && !(dataread = scan_array (GWD_file, &p1, 'i', 0, 0)))
		{
			dataread = fscanf(GWD_file.ptr, "%c%d%c%d%lf%*[^\n]", &tempvar,&p2,&tempvar,&p3,&p4);
				
			if (p1 >= ctrl->simstartyear && p1 < ctrl->simstartyear + ctrl->simyears)
			{
				GWyear_array[ndata]     = p1;
				GWmonth_array[ndata]    = p2;
				GWday_array[ndata]      = p3;
				GWdepth_array[ndata]    = p4;

                nmgm += 1;
				ndata += 1;
			}
		}

		gws->GWD_num = nmgm;
		nmgm = 0;
	
		gws->GWyear_array      = (int*) malloc(gws->GWD_num*sizeof(double));  
		gws->GWmonth_array     = (int*) malloc(gws->GWD_num*sizeof(double)); 
		gws->GWday_array       = (int*) malloc(gws->GWD_num*sizeof(double)); 
		gws->GWdepth_array  = (double*) malloc(gws->GWD_num*sizeof(double)); 

		for (nmgm = 0; nmgm < gws->GWD_num; nmgm++)
		{
			gws->GWyear_array[nmgm]      = GWyear_array[nmgm];
			gws->GWmonth_array[nmgm]     = GWmonth_array[nmgm];
			gws->GWday_array[nmgm]       = GWday_array[nmgm];

			gws->GWdepth_array[nmgm]  = GWdepth_array[nmgm];
		}

		/* read year and GWD for each simday in each simyear */
		
		free(GWyear_array);	
		free(GWmonth_array);	
		free(GWday_array);	
        free(GWdepth_array);	

		fclose(GWD_file.ptr);
	}	

	

	gws->mgmdGWD = 0;

	/* -------------------------------------------*/

		
	return (errorCode);
}
