/* 
fertilizing_init.c
read fertilizing information for pointbgc simulation

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


int fertilizing_init(file init, const control_struct* ctrl, fertilizing_struct* FRZ)
{
	char header[STRINGSIZE];
	char FRZ_filename[STRINGSIZE];
	file FRZ_file;

	int maxlen=STRINGSIZE;

	int errorCode=0;
	int okFILE = 1;

	int mgmread;
	int nmgm = 0;

	int p1,p2,p3;
	double p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14;
	char tempvar;
	char ferttype[STRINGSIZE];

	int n_FRZparam, maxFRZ_num;

	int* FRZyear_array;			
	int* FRZmonth_array;						
	int* FRZday_array;							
	double* FRZdepth_array;					
	double* fertilizer_array;				
	double* DM_array;				          
	double* NO3content_array;					
 	double* NH4content_array;					
	double* UREAcontent_array;					
	double* orgCcontent_array;					
	double* orgNcontent_array;					
	double* litr_flab_array;					
	double* litr_fcel_array;								
	double* EFfert_N2O;							
	char** ferttype_array;						

	maxFRZ_num=1000;

	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with header:  **
	** FERTILIZING                                                       ** 
	**                                                                 **
	********************************************************************/
	
	/* header reading */
	if (!errorCode && scan_value(init, header, 's'))
	{
		printf("ERROR reading header, fertilizing_init()\n");
		errorCode=1;
	}

	/* header control */
	if (!errorCode && scan_value(init, header, 's'))
	{
		printf("ERROR reading header, fertilizing_init()\n");
		errorCode=1;
	}
	
	
	/* number of management action */
	if (!errorCode && scan_value(init, &FRZ->FRZ_num, 'i'))
	{
		printf("ERROR reading number of fertilizing in FERTILIZING section\n");
		errorCode=1;
	}


	/* if FRZ_num > 0 -> fertilizing */
	if (!errorCode && FRZ->FRZ_num)
	{
		/* allocate space for the temporary MGM array */
		FRZyear_array         = (int*) malloc(maxFRZ_num*sizeof(int));  
		FRZmonth_array        = (int*) malloc(maxFRZ_num*sizeof(int)); 
		FRZday_array          = (int*) malloc(maxFRZ_num*sizeof(int)); 
		FRZdepth_array        = (double*) malloc(maxFRZ_num*sizeof(double)); 
		fertilizer_array      = (double*) malloc(maxFRZ_num*sizeof(double)); 
		DM_array              = (double*) malloc(maxFRZ_num*sizeof(double)); 
		NO3content_array      = (double*) malloc(maxFRZ_num*sizeof(double)); 
		NH4content_array      = (double*) malloc(maxFRZ_num*sizeof(double)); 
		UREAcontent_array     = (double*) malloc(maxFRZ_num*sizeof(double)); 
		orgNcontent_array     = (double*) malloc(maxFRZ_num*sizeof(double));
		orgCcontent_array     = (double*) malloc(maxFRZ_num*sizeof(double)); 
		litr_flab_array       = (double*) malloc(maxFRZ_num*sizeof(double)); 
		litr_fcel_array       = (double*) malloc(maxFRZ_num*sizeof(double)); 
		EFfert_N2O            = (double*) malloc(maxFRZ_num*sizeof(double));
		
		ferttype_array        = (char**) malloc(maxFRZ_num * sizeof(char*));
		
		if (!errorCode && scan_value(init, FRZ_filename, 's'))
		{
			printf("ERROR reading fertilizing calculating file\n");
			errorCode=1;
		}
		
		strcpy(FRZ_file.name, FRZ_filename);
		
		/* open the main init file for ascii read and check for errors */
		if (file_open(&FRZ_file,'i',1))
		{
			printf("ERROR opening FRZ_file, fertilizing_int.c\n");
			errorCode=1;
			okFILE=0;
		}

		if (!errorCode && scan_value(FRZ_file, header, 's'))
		{
			printf("ERROR reading header for FERTILIZING section in MANAGMENET file\n");
			errorCode=1;
		}

	
		while (!errorCode && !(mgmread = scan_array (FRZ_file, &p1, 'i', 0, 0)))
		{
			ferttype_array[nmgm] = (char*) malloc(STRINGSIZE * sizeof(char));

			n_FRZparam = 16;

			mgmread = fscanf(FRZ_file.ptr, "%c%d%c%d%s%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%*[^\n]",
				             &tempvar,&p2,&tempvar,&p3,(char*)&ferttype,&p4,&p5,&p6,&p7,&p8,&p9,&p10,&p11,&p12,&p13,&p14);
			
			if (mgmread != n_FRZparam)
			{
				printf("ERROR reading FERTILIZING parameters from FERTILIZING file  file\n");
				errorCode=1;
			}

			if (sizeof(ferttype) > maxlen)
			{
				printf("ERROR reading ferttype in fertilizing file (lenght must be in the range 1-200)\n");
				errorCode=1;
			}
			
			if (p1 >= ctrl->simstartyear && p1 < ctrl->simstartyear + ctrl->simyears)
			{
				FRZyear_array[nmgm]     = p1;
				FRZmonth_array[nmgm]    = p2;
				FRZday_array[nmgm]      = p3;
				fertilizer_array[nmgm]  = p4;
				FRZdepth_array[nmgm]    = p5;
				DM_array[nmgm]          = p6;
				NO3content_array[nmgm]  = p7;
				NH4content_array[nmgm]  = p8;
				UREAcontent_array[nmgm] = p9;
				orgNcontent_array[nmgm] = p10;
				orgCcontent_array[nmgm] = p11;
				litr_flab_array[nmgm]   = p12;
				litr_fcel_array[nmgm]   = p13;
				EFfert_N2O[nmgm]        = p14;

				strcpy(ferttype_array[nmgm],  ferttype);

				nmgm += 1;
			}
		}
		
			
		FRZ->FRZ_num = nmgm;
		nmgm = 0;

		
		FRZ->FRZyear_array         = (int*) malloc(FRZ->FRZ_num*sizeof(int));  
		FRZ->FRZmonth_array        = (int*) malloc(FRZ->FRZ_num*sizeof(int)); 
		FRZ->FRZday_array          = (int*) malloc(FRZ->FRZ_num*sizeof(int)); 
		FRZ->FRZdepth_array        = (double*) malloc(FRZ->FRZ_num*sizeof(double)); 
		FRZ->fertilizer_array      = (double*) malloc(FRZ->FRZ_num*sizeof(double)); 
		FRZ->DM_array              = (double*) malloc(FRZ->FRZ_num*sizeof(double)); 
		FRZ->NO3content_array      = (double*) malloc(FRZ->FRZ_num*sizeof(double)); 
		FRZ->NH4content_array      = (double*) malloc(FRZ->FRZ_num*sizeof(double)); 
		FRZ->UREAcontent_array     = (double*) malloc(FRZ->FRZ_num*sizeof(double)); 
		FRZ->orgNcontent_array     = (double*) malloc(FRZ->FRZ_num*sizeof(double));
		FRZ->orgCcontent_array     = (double*) malloc(FRZ->FRZ_num*sizeof(double)); 
		FRZ->litr_flab_array       = (double*) malloc(FRZ->FRZ_num*sizeof(double)); 
		FRZ->litr_fcel_array       = (double*) malloc(FRZ->FRZ_num*sizeof(double)); 
		FRZ->EFfert_N2O            = (double*) malloc(FRZ->FRZ_num*sizeof(double));
		
		FRZ->ferttype_array        = (char**) malloc(FRZ->FRZ_num * sizeof(char*));

		for (nmgm = 0; nmgm < FRZ->FRZ_num; nmgm++)
		{
			FRZ->FRZyear_array[nmgm]     = FRZyear_array[nmgm];
			FRZ->FRZmonth_array[nmgm]    = FRZmonth_array[nmgm] ;
			FRZ->FRZday_array[nmgm]      = FRZday_array[nmgm] ;
			FRZ->fertilizer_array[nmgm]  = fertilizer_array[nmgm];
			FRZ->FRZdepth_array[nmgm]    = FRZdepth_array[nmgm] ;
			FRZ->DM_array[nmgm]          = DM_array[nmgm] ;
			FRZ->NO3content_array[nmgm]  = NO3content_array[nmgm] ;
			FRZ->NH4content_array[nmgm]  = NH4content_array[nmgm];
			FRZ->UREAcontent_array[nmgm] = UREAcontent_array[nmgm];
			FRZ->orgNcontent_array[nmgm] = orgNcontent_array[nmgm];
			FRZ->orgCcontent_array[nmgm] = orgCcontent_array[nmgm];
			FRZ->litr_flab_array[nmgm]   = litr_flab_array[nmgm] ;
			FRZ->litr_fcel_array[nmgm]   = litr_fcel_array[nmgm] ;
			FRZ->EFfert_N2O[nmgm]        = EFfert_N2O[nmgm];

			FRZ->ferttype_array[nmgm] = (char*) malloc(STRINGSIZE * sizeof(char));
			strcpy(FRZ->ferttype_array[nmgm], ferttype_array[nmgm]);
		}

		/* close FERTILIZING file  and free temporary memory */
		if (okFILE) fclose (FRZ_file.ptr);

		free(FRZyear_array);	
		free(FRZmonth_array);	
		free(FRZday_array);	
		free(FRZdepth_array);					
		free(fertilizer_array);				
		free(DM_array);				          
		free(NO3content_array);					
 		free(NH4content_array);					
		free(UREAcontent_array);					
		free(orgNcontent_array);	
		free(orgCcontent_array);									
		free(litr_flab_array);					
		free(litr_fcel_array);							
		free(EFfert_N2O);							
		free(ferttype_array);	
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


	
	FRZ->mgmdFRZ = 0;


	return (errorCode);
}
