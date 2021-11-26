/* 
mgm_init.c
read mgm file for pointbgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2020, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
20.03.01 Galina Churkina added variable "sum" substituting  t1+t2+t3 in IF statement,
which gave an error.
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



int mgm_init(file init, control_struct *ctrl,  epconst_struct* epc, fertilizing_struct* FRZ, grazing_struct* GRZ, harvesting_struct* HRV, mowing_struct* MOW, 
	         planting_struct* PLT, ploughing_struct* PLG, thinning_struct* THN, irrigating_struct* IRG)
{
	int errorCode=0;
	int dofilecloseMANAGEMENT = 1;
	file mgm_file;
	char key1[] = "MANAGEMENT_FILE";
	char key2[] = "none";
	char keyword[STRINGSIZE];
	char header[STRINGSIZE];

	int mgm, PLTyday, HRVyday, GRZstart_yday,GRZend_yday;

	
	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** MANAGEMENT_FILE                                                       ** 
	**                                                                 **
	********************************************************************/
	
	/* initialization */
	PLT->mgmdPLT = -1;
	THN->THN_num = 0;
	MOW->MOW_num = 0;
	HRV->HRV_num = 0;
	PLG->PLG_num = 0;
	GRZ->GRZ_num = 0;
	FRZ->FRZ_num = 0;
	IRG->IRG_num = 0;
	GRZ->trampleff_act	= DATA_GAP;

	
	/* scan for the MANAGEMENT file keyword, exit if not next */
	if (!errorCode && scan_value(init, keyword, 's'))
	{
		printf("ERROR reading keyword for control data\n");
		errorCode=210;
	}
	if (!errorCode && strcmp(keyword, key1))
	{
		printf("Expecting keyword --> %s in file %s\n",key1,init.name);
		errorCode=210;
	}
	/* open simple MANAGEMENT file  */
	if (!errorCode && scan_open(init,&mgm_file,'r',0)) 
	{
		
			if (!errorCode && strcmp(mgm_file.name, key2))
			{
				printf("ERROR opening mgm_file file from INI file, mgm_init()\n");
				errorCode=210;
			}
			dofilecloseMANAGEMENT = 0;

	}
	
	/* if management data file is open: read management data */
	if (dofilecloseMANAGEMENT)
	{
		/* first scan sprop header to ensure proper *.sprop format */
		if (!errorCode && scan_value(mgm_file, header, 's'))
		{
			printf("ERROR reading header, mgm_init()\n");
			errorCode=210;
		}
		
		/* -------------------------------------------------------------------------*/
		/* MANAGEMENT SECTION  */
	
		/* read the planting information */
		if (!errorCode && planting_init(mgm_file, ctrl, PLT, epc))
		{
			printf("ERROR in call to planting_init() from mgm_init.c... Exiting\n");
			errorCode=2101;
		}

		/* read the thinning information */
		if (!errorCode && thinning_init(mgm_file, ctrl, THN))
		{
			printf("ERROR in call to thinning_init() from mgm_init.c... Exiting\n");
			errorCode=2102;
		}

		/* read the mowing  information */
		if (!errorCode && mowing_init(mgm_file, ctrl, MOW))
		{
			printf("ERROR in call to mowing_init() from mgm_init.c... Exiting\n");
			errorCode=2103;
		}

		/* read the grazing information */
		if (!errorCode && grazing_init(mgm_file, ctrl, GRZ))
		{
			printf("ERROR in call to grazing_init() from mgm_init.c... Exiting\n");
			errorCode=2104;
		}

		/* read the harvesting information */
		if (!errorCode && harvesting_init(mgm_file, ctrl, PLT, HRV))
		{
			printf("ERROR in call to harvesting_init() from mgm_init.c... Exiting\n");
			errorCode=2105;
		}

		/* read the harvesting information */
		if (!errorCode && ploughing_init(mgm_file, ctrl, PLG))
		{
			printf("ERROR in call to ploughing_init() from mgm_init.c... Exiting\n");
			errorCode=2106;
		}

		/* read the fertilizing  information */
		if (!errorCode && fertilizing_init(mgm_file, ctrl, FRZ))
		{
			printf("ERROR in call to fertilizing_init() from mgm_init.c... Exiting\n");
			errorCode=2107;
		}

		/* read the irrigating information */
		if (!errorCode && irrigating_init(mgm_file, ctrl, IRG))
		{
			printf("ERROR in call to irrigating_init() from mgm_init.c... Exiting\n");
			errorCode=2108;
		}


		fclose(mgm_file.ptr);

		/* CONTROL OF MANAGEMENT DATA */
		
		/* planting and harvest setting*/
		if (!errorCode && (PLT->PLT_num != HRV->HRV_num))
		{
			if (PLT->PLT_num == HRV->HRV_num + 1)
			{
				if (ctrl->onscreen) printf("WARNING: last planting is inactive (because of the lack of harvest)\n");
				PLT->PLT_num -= 1;
			}
			else
			{
				printf("ERROR in management data: number planting and harvest action should be equal\n");
				errorCode=2100001;
			}
		}

		/* planting and harvest date */
		if (!errorCode && PLT->PLT_num)
		{
			for (mgm = 0; mgm < PLT->PLT_num; mgm++)
			{
				PLTyday = PLT->PLTyear_array[mgm] * nDAYS_OF_YEAR + date_to_doy(PLT->PLTmonth_array[mgm], PLT->PLTday_array[mgm]);
				HRVyday = HRV->HRVyear_array[mgm] * nDAYS_OF_YEAR + date_to_doy(HRV->HRVmonth_array[mgm], HRV->HRVday_array[mgm]);
				if (HRVyday <= PLTyday)
				{
					printf("ERROR in management data: PLANTING must be before HARVESTING date\n");
					errorCode=2100002;
				}
			}
		}


		/* planting and harvest date */
		if (!errorCode && GRZ->GRZ_num)
		{
			for (mgm = 0; mgm < PLT->PLT_num; mgm++)
			{
				GRZstart_yday = GRZ->GRZstart_year_array[mgm] * nDAYS_OF_YEAR + date_to_doy(GRZ->GRZstart_month_array[mgm], GRZ->GRZstart_day_array[mgm]);
				GRZend_yday   = GRZ->GRZend_year_array[mgm] * nDAYS_OF_YEAR + date_to_doy(GRZ->GRZend_month_array[mgm], GRZ->GRZend_day_array[mgm]);
				if (GRZend_yday < GRZstart_yday)
				{
					printf("ERROR in management date: start of GRAZING must be before end of GRAZING\n");
					errorCode=2100003;
				}
			}
		}

	

	}
	/* if no management */
	else
	{
		PLT->PLT_num = 0;
		THN->THN_num = 0;
		MOW->MOW_num = 0;
		GRZ->GRZ_num = 0;
		HRV->HRV_num = 0;
		PLG->PLG_num = 0;
		FRZ->FRZ_num = 0;
		IRG->IRG_num = 0;

	}
	
		
	return (errorCode);

}
