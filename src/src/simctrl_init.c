/* 
simctrl_init.c
read simulation control flags for pointbgc simulation

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



int simctrl_init(file init, epconst_struct* epc, control_struct* ctrl, planting_struct* PLT)
{
	int errorCode=0;
	char key[] = "SIMULATION_CONTROL";
	char keyword[STRINGSIZE];


	
	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** SOI_FILE                                                      ** 
	**                                                                 **
	********************************************************************/
	
	
	/* scan for the SOIL file keyword, exit if not next */
	if (!errorCode && scan_value(init, keyword, 's'))
	{
		printf("ERROR reading keyword for control data\n");
		errorCode=211;
	}
	if (!errorCode && strcmp(keyword, key))
	{
		printf("Expecting keyword --> %s in file %s\n",key,init.name);
		errorCode=211;
	}

	if (!errorCode && scan_value(init, &epc->phenology_flag, 'i'))
	{
		printf("ERROR reading phenology flag, epc_init()\n");
		errorCode=21101;
	}

	/* get flag of GSI flag */
	if (!errorCode && scan_value(init, &epc->GSI_flag, 'i'))
	{
		printf("ERROR reading flag indicating usage of GSI file: epc_init()\n");
		errorCode=21102;
	}
	
	if (!errorCode && scan_value(init, &epc->transferGDD_flag, 'i'))
	{
		printf("ERROR reading transferGDD_flag, epc_init()\n");
		errorCode=21103;
	}

	/* control of phenophase number */
	if (!errorCode)
	{
		if (epc->transferGDD_flag && epc->n_emerg_phenophase < 1)
		{
			printf("ERROR in phenophase parametrization: if transferGDD_flag = 1 -> n_emerg_phenophase must be specified in EPC file()\n");
			errorCode=2110301;
		}
	}

	/* temperature dependent q10 value */
	if (!errorCode && scan_value(init, &epc->q10depend_flag, 'i'))
	{
		printf("ERROR reading q10depend_flag, epc_init()\n");
		errorCode=21104;
	}
	
	/* acclimation */
	if (!errorCode && scan_value(init, &epc->phtsyn_acclim_flag, 'i'))
	{
		printf("ERROR reading acclimation flag of photosynthesis, epc_init()\n");
		errorCode=21105;
	}
	if (!errorCode && scan_value(init, &epc->resp_acclim_flag, 'i'))
	{
		printf("ERROR reading acclimation flag of respiration, epc_init()\n");
		errorCode=21106;
	}
	
	/* get flag of CO2 conductance reduction */
	if (!errorCode && scan_value(init, &epc->CO2conduct_flag, 'i'))
	{
		printf("ERROR reading CO2conduct_flag, epc_init()\n");
		errorCode=21107;
	}


	/* soil temperature calculation flag */
	if (!errorCode && scan_value(init, &epc->STCM_flag, 'i'))
	{
		printf("ERROR reading soil temperature calculation flag: epc_init()\n");
		errorCode=21108;
	}
	
	/* soil water calculation flag */
	if (!errorCode && scan_value(init, &epc->SHCM_flag, 'i'))
	{
		printf("ERROR reading soil hydrological calculation method flag: epc_init()\n");
		errorCode=21109;
	}

	/*  discretitaion level of VWC calculation simulation */
	if (!errorCode && scan_value(init, &epc->discretlevel_Richards, 'i'))
	{
		printf("ERROR reading discretitaion level of VWC calculation: epc_init.c\n");
		errorCode=21110;
	}

	/* control of discretlevel_Richards */
	if (!errorCode && (epc->SHCM_flag == 0 || epc->SHCM_flag == 2)  && epc->discretlevel_Richards > 0)
	{
		if (ctrl->onscreen) printf("WARNING: discretization level of soil hydr.calc. is used only with Richards-method, epc_init()\n");
	}


	/*  photosynthesis calculation method flag */
	if (!errorCode && scan_value(init, &epc->photosynt_flag, 'i'))
	{
		printf("ERROR reading photosynthesis calculation method flag: epc_init.c\n");
		errorCode=21111;
	}


	/*  evapotranspiration calculation method flag */
	if (!errorCode && scan_value(init, &epc->evapotransp_flag, 'i'))
	{
		printf("ERROR reading evapotranspiration calculation method flag: epc_init.c\n");
		errorCode=21112;
	}

	/* control: evapotransp_flag */
	if (!errorCode && epc->evapotransp_flag) 
	{
		printf("ERROR: Priestley-Taylor evaportanspiration method is not implemented yet in the model.\n");
		printf("Please use the Penman-Monteith Scheme [see INI file]\n");
		errorCode=21112;
	}


	/*  radiation calculation method flag */
	if (!errorCode && scan_value(init, &epc->radiation_flag, 'i'))
	{
		printf("ERROR reading radiation calculation method flag: epc_init.c\n");
		errorCode=21113;
	}

	/*  soilstress calculation method flag */
	if (!errorCode && scan_value(init, &epc->soilstress_flag, 'i'))
	{
		printf("ERROR reading soilstress calculation method flag: epc_init.c\n");
		errorCode=21114;
	}

	/* control: in case of planting/harvesting, model-defined phenology is not possible: first day - planting day, last day - harvesting day */
	if (epc->phenology_flag == 1 && PLT->PLT_num) 
	{
		ctrl->prephen1_flag = 1;
		epc->phenology_flag = 0;
	}

	/* control: in case of user-defined phenology, GSI-method is not possible */
	if (epc->phenology_flag == 0 && epc->GSI_flag) 
	{
		ctrl->prephen2_flag = 1;
		epc->GSI_flag = 0;
	}
	



  return (errorCode);
  
}