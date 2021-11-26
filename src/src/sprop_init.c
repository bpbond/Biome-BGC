/* 
sprop_init.c
read sprop file for pointbgc simulation

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
#include "bgc_func.h"


int sprop_init(file init, siteconst_struct* sitec, soilprop_struct* sprop, control_struct* ctrl)
{
	int errorCode=0;
	int layer, scanflag;
	int dofilecloseSOILPROP = 1;
	file sprop_file;
	char key[] = "SOIL_FILE";
	char keyword[STRINGSIZE];
	char header[STRINGSIZE];

	/* original soil C:N values */
	double soilCN12_orig = 12;
	double soilCN34_orig = 10;

	file eSp_file;
	
	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** SOIL_FILE                                                       ** 
	**                                                                 **
	********************************************************************/
	
	
	/* scan for the SOIL file keyword, exit if not next */
	if (!errorCode && scan_value(init, keyword, 's'))
	{
		printf("ERROR reading keyword for control data\n");
		errorCode=208;
	}
	if (!errorCode && strcmp(keyword, key))
	{
		printf("Expecting keyword --> %s in file %s\n",key,init.name);
		errorCode=208;
	}
	/* open simple SOIL file  */
	if (!errorCode && scan_open(init,&sprop_file,'r',1)) 
	{
		printf("ERROR opening sproponst file, sprop_init()\n");
		dofilecloseSOILPROP = 0;
		errorCode=20800;
	}
	
	/* first scan sprop keyword to ensure proper *.sprop format */
	if (!errorCode && scan_value(sprop_file, header, 's'))
	{
		printf("ERROR reading header, sprop_init()\n");
		errorCode=20800;
	}

	/***********************************************************/
	/* dividing line from file */ 
	if (!errorCode && scan_value(sprop_file, header, 's'))
	{
		printf("ERROR reading 1. dividing line, sprop_init()\n");
		errorCode=20801;
	}
	if (!errorCode && scan_value(sprop_file, header, 's'))
	{
		printf("ERROR reading 1. dividing line, sprop_init()\n");
		errorCode=20801;
	}

	/* using varying extra SOI parameters
		from extraSOIparameters.txt file if it exists OR from SOI file if extraSOIparameters.txt file if doesnt exists  */
	strcpy(eSp_file.name, "extraSOIparameters.txt");
	

	/* new SOI file with 64 lines - with extraSOIparameters.txt from file */
	if (!errorCode && !file_open(&eSp_file,'j',1)) 
	{
		ctrl->oldSOIfile_flag = 1;

		/*********************************************/
		/* reading from extraSOIparameters.txt */
		/*********************************************/

		if (!errorCode && scan_value(eSp_file, &sprop->pHp1_nitrif, 'd'))
		{
			printf("ERROR pHp1_nitrif, sprop_init()\n");
			errorCode=20833;
		}
		if (!errorCode && scan_value(eSp_file, &sprop->pHp2_nitrif, 'd'))
		{
			printf("ERROR pHp2_nitrif, sprop_init()\n");
			errorCode=20834;
		}
		if (!errorCode && scan_value(eSp_file, &sprop->pHp3_nitrif, 'd'))
		{
			printf("ERROR pHp3_nitrif, sprop_init()\n");
			errorCode=20835;
		}
		if (!errorCode && scan_value(eSp_file, &sprop->pHp4_nitrif, 'd'))
		{
			printf("ERROR pHp4_nitrif, sprop_init()\n");
			errorCode=20836;
		}
		if (!errorCode && scan_value(eSp_file, &sprop->Tp1_nitrif, 'd'))
		{
			printf("ERROR Tp1_nitrif, sprop_init()\n");
			errorCode=20837;
		}
		if (!errorCode && scan_value(eSp_file, &sprop->Tp2_nitrif, 'd'))
		{
			printf("ERROR Tp2_nitrif, sprop_init()\n");
			errorCode=20838;
		}
		if (!errorCode && scan_value(eSp_file, &sprop->Tp3_nitrif, 'd'))
		{
			printf("ERROR Tp3_nitrif, sprop_init()\n");
			errorCode=20839;
		}
		if (!errorCode && scan_value(eSp_file, &sprop->Tp4_nitrif, 'd'))
		{
			printf("ERROR Tp4_nitrif, sprop_init()\n");
			errorCode=20840;
		}
		if (!errorCode && scan_value(eSp_file, &sprop->Tp1_decomp, 'd'))
		{
			printf("ERROR Tp1_decomp, sprop_init()\n");
			errorCode=20841;
		}
		if (!errorCode && scan_value(eSp_file, &sprop->Tp2_decomp, 'd'))
		{
			printf("ERROR Tp2_decomp, sprop_init()\n");
			errorCode=20842;
		}
		if (!errorCode && scan_value(eSp_file, &sprop->Tp3_decomp, 'd'))
		{
			printf("ERROR Tp3_decomp, sprop_init()\n");
			errorCode=20843;
		}
		if (!errorCode && scan_value(eSp_file, &sprop->Tp4_decomp, 'd'))
		{
			printf("ERROR Tp4_decomp, sprop_init()\n");
			errorCode=20844;
		}
		if (!errorCode && scan_value(eSp_file, &sprop->Tmin_decomp, 'd'))
		{
			printf("ERROR Tp0_decomp, sprop_init()\n");
			errorCode=20845;
		}
		if (!errorCode && scan_value(eSp_file, &sprop->pLAYER_mulch, 'd'))
		{
			printf("ERROR pLAYER_mulch, sprop_init()\n");
			errorCode=20846;
		}
		if (!errorCode && scan_value(eSp_file, &sprop->p1_mulch, 'd'))
		{
			printf("ERROR p1_mulch, sprop_init()\n");
			errorCode=20847;
		}
		if (!errorCode && scan_value(eSp_file, &sprop->p2_mulch, 'd'))
		{
			printf("ERROR p2_mulch, sprop_init()\n");
			errorCode=20848;
		}
		if (!errorCode && scan_value(eSp_file, &sprop->p3_mulch, 'd'))
		{
			printf("ERROR p3_mulch, sprop_init()\n");
			errorCode=20849;
		}
		if (!errorCode && scan_value(eSp_file, &sprop->pRED_mulch, 'd'))
		{
			printf("ERROR pRED_mulch, sprop_init()\n");
			errorCode=20850;
		}
		if (!errorCode && scan_value(eSp_file, &sprop->p1diffus_tipping, 'd'))
		{
			printf("ERROR p1diffus_tipping, sprop_init()\n");
			errorCode=20852;
		}
		if (!errorCode && scan_value(eSp_file, &sprop->p2diffus_tipping, 'd'))
		{
			printf("ERROR p2diffus_tipping, sprop_init()\n");
			errorCode=20853;
		}
		if (!errorCode && scan_value(eSp_file, &sprop->p3diffus_tipping, 'd'))
		{
			printf("ERROR p3diffus_tipping, sprop_init()\n");
			errorCode=20854;
		}
		if (!errorCode && scan_value(eSp_file, &ctrl->GW_flag, 'i'))
		{
			printf("ERROR GW_flag, sprop_init()\n");
			errorCode=20855;
		}

		if (!errorCode && scan_value(eSp_file, &sprop->CapillFringe_act, 'd'))
		{
			printf("ERROR CapillFringe_act, sprop_init()\n");
			errorCode=20856;
		}

		if (sprop->CapillFringe_act != DATA_GAP) sprop->CapillFringe = sprop->CapillFringe_act;
		
		fclose(eSp_file.ptr);

		/********************************************/
		/* reading from old SOI file */
		/*********************************************/

		/* NITROGEN AND DECOMPOSITION PARAMETERS  */
	
		/*  denetirification proportion and mobilen proportion
			original: BBGC constant - new version: can be set in SOIL file*/
		if (!errorCode && scan_value(sprop_file, &sprop->denitr_coeff, 'd'))
		{
			printf("ERROR reading denitr_coeff: sprop_init()\n");
			errorCode=20802;
		}

		if (!errorCode && scan_value(sprop_file, &sprop->netMiner_to_nitrif, 'd'))
		{
			printf("ERROR reading netMiner_to_nitrif: sprop_init()\n");
			errorCode=20803;
		}

		if (!errorCode && scan_value(sprop_file, &sprop->maxNitrif_rate, 'd'))
		{
			printf("ERROR reading maxNitrif_rate: sprop_init()\n");
			errorCode=20803;
		}
	
		if (!errorCode && scan_value(sprop_file, &sprop->N2Ocoeff_nitrif, 'd'))
		{
			printf("ERROR reading N2Ocoeff_nitrif: sprop_init()\n");
			errorCode=20804;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->NH4_mobilen_prop, 'd'))
		{
			printf("ERROR reading NH4_mobilen_prop: sprop_init()\n");
			errorCode=20805;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->N2Oratio_denitr, 'd'))
		{
			printf("ERROR reading N2Oratio_denitr: sprop_init()\n");
			errorCode=20806;
		}

		sprop->critWFPS_denitr = 0.55;

		if (!errorCode && scan_value(sprop_file, &sprop->efolding_depth, 'd'))
		{
			printf("ERROR reading e-folding depth: sprop_init()\n");
			errorCode=20807;
		}

		if (!errorCode && scan_value(sprop_file, &sprop->SOIL1_dissolv_prop, 'd'))
		{
			printf("ERROR reading SOIL1_dissolv_prop: sprop_init()\n");
			errorCode=20808;
		}
	
		if (!errorCode && scan_value(sprop_file, &sprop->SOIL2_dissolv_prop, 'd'))
		{
			printf("ERROR reading SOIL2_dissolv_prop: sprop_init()\n");
			errorCode=20808;
		}

		if (!errorCode && scan_value(sprop_file, &sprop->SOIL3_dissolv_prop, 'd'))
		{
			printf("ERROR reading SOIL3_dissolv_prop: sprop_init()\n");
			errorCode=20808;
		}

		if (!errorCode && scan_value(sprop_file, &sprop->SOIL4_dissolv_prop, 'd'))
		{
			printf("ERROR reading SOIL4_dissolv_prop: sprop_init()\n");
			errorCode=20808;
		}

		if (!errorCode && scan_value(sprop_file, &sprop->minWFPS_nitrif, 'd'))
		{
			printf("ERROR reading WFPS_min: sprop_init()\n");
			errorCode=20809;
		}

		if (!errorCode && scan_value(sprop_file, &sprop->opt1WFPS_nitrif, 'd'))
		{
			printf("ERROR reading WFPS_opt1: sprop_init()\n");
			errorCode=20809;
		}


		if (!errorCode && scan_value(sprop_file, &sprop->opt2WFPS_nitrif, 'd'))
		{
			printf("ERROR reading WFPS_opt2: sprop_init()\n");
			errorCode=20809;
		}

		if (!errorCode && scan_value(sprop_file, &sprop->scalarWFPSmin_nitrif, 'd'))
		{
			printf("ERROR reading scalarWFPSmin_nitrif: sprop_init()\n");
			errorCode=20809;
		}

		if (!errorCode && scan_value(sprop_file, &sprop->soil4_CN, 'd'))
		{
			printf("ERROR reading SOIL4_CN: sprop_init()\n");
			errorCode=20809;
		}
		sprop->soil1_CN = sprop->soil4_CN * (soilCN12_orig/soilCN34_orig);
		sprop->soil2_CN = sprop->soil4_CN * (soilCN12_orig/soilCN34_orig);
		sprop->soil3_CN = sprop->soil4_CN;

	
		/*****************************/
		/* dividing line from file */ 
		if (!errorCode && scan_value(sprop_file, header, 's'))
		{
			printf("ERROR reading 2. dividing line, sprop_init()\n");
			errorCode=20810;
		}
		if (!errorCode && scan_value(sprop_file, header, 's'))
		{
			printf("ERROR reading 2. dividing line, sprop_init()\n");
			errorCode=20810;
		}
	
		/******************************/
		/* RATE SCALARS */
	
		/* respiration fractions for fluxes between compartments  */
		if (!errorCode && scan_value(sprop_file, &sprop->rfl1s1, 'd'))
		{
			printf("ERROR reading rfl1s1: sprop_init()\n");
			errorCode=20811;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->rfl2s2, 'd'))
		{
			printf("ERROR reading rfl2s2: sprop_init()\n");
			errorCode=20811;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->rfl4s3, 'd'))
		{
			printf("ERROR reading rfl4s3: sprop_init()\n");
			errorCode=20811;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->rfs1s2, 'd'))
		{
			printf("ERROR reading rfs1s2: sprop_init()\n");
			errorCode=20811;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->rfs2s3, 'd'))
		{
			printf("ERROR reading rfs2s3: sprop_init()\n");
			errorCode=20811;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->rfs3s4, 'd'))
		{
			printf("ERROR reading rfs3s4: sprop_init()\n");
			errorCode=20811;
		}

		/* 	base values of rate constants are (1/day)   */
		if (!errorCode && scan_value(sprop_file, &sprop->kl1_base, 'd'))
		{
			printf("ERROR reading kl1_base: sprop_init()\n");
			errorCode=20812;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->kl2_base, 'd'))
		{
			printf("ERROR reading kl2_base: sprop_init()\n");
			errorCode=20812;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->kl4_base, 'd'))
		{
			printf("ERROR reading kl4_base: sprop_init()\n");
			errorCode=20812;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->ks1_base, 'd'))
		{
			printf("ERROR reading ks1_base: sprop_init()\n");
			errorCode=20812;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->ks2_base, 'd'))
		{
			printf("ERROR reading ks2_base: sprop_init()\n");
			errorCode=20812;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->ks3_base, 'd'))
		{
			printf("ERROR reading ks3_base: sprop_init()\n");
			errorCode=20811;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->ks4_base, 'd'))
		{
			printf("ERROR reading ks4_base: sprop_init()\n");
			errorCode=20812;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->kfrag_base, 'd'))
		{
			printf("ERROR reading kfrag_base: sprop_init()\n");
			errorCode=20812;
		}

		/****************************/
		/* dividing line from file */ 

		if (!errorCode && scan_value(sprop_file, header, 's'))
		{
			printf("ERROR reading 3. dividing line, sprop_init()\n");
			errorCode=20813;
		}
		if (!errorCode && scan_value(sprop_file, header, 's'))
		{
			printf("ERROR reading 3. dividing line, sprop_init()\n");
			errorCode=20813;
		}
		/********************/
		/* CH4 PARAMETERS */

		if (!errorCode && scan_value(sprop_file, &sprop->pBD1_CH4, 'd'))
		{
			printf("ERROR reading pBD1_CH4: sprop_init()\n");
			errorCode=20814;
		}

		if (!errorCode && scan_value(sprop_file, &sprop->pBD2_CH4, 'd'))
		{
			printf("ERROR reading pBD2_CH4: sprop_init()\n");
			errorCode=20814;
		}

		if (!errorCode && scan_value(sprop_file, &sprop->pVWC1_CH4, 'd'))
		{
			printf("ERROR reading pVWC1_CH4: sprop_init()\n");
			errorCode=20814;
		}

		if (!errorCode && scan_value(sprop_file, &sprop->pVWC2_CH4, 'd'))
		{
			printf("ERROR reading pVWC2_CH4: sprop_init()\n");
			errorCode=20814;
		}

		if (!errorCode && scan_value(sprop_file, &sprop->pVWC3_CH4, 'd'))
		{
			printf("ERROR reading pVWC3_CH4: sprop_init()\n");
			errorCode=20814;
		}

		if (!errorCode && scan_value(sprop_file, &sprop->C_pVWC4, 'd'))
		{
			printf("ERROR reading C_pVWC4: sprop_init()\n");
			errorCode=20814;
		}

		if (!errorCode && scan_value(sprop_file, &sprop->pTS_CH4, 'd'))
		{
			printf("ERROR reading pTS_CH4: sprop_init()\n");
			errorCode=20814;
		}
	
		
		/***************************/
		/* dividing line from file */ 
		if (!errorCode && scan_value(sprop_file, header, 's'))
		{
			printf("ERROR reading 4. dividing line, sprop_init()\n");
			errorCode=20815;
		}
		if (!errorCode && scan_value(sprop_file, header, 's'))
		{
			printf("ERROR reading 4. dividing line, sprop_init()\n");
			errorCode=20815;
		}
		/********************/
		/* SOIL PARAMETERS */

		/*  soil depth */
		if (!errorCode && scan_value(sprop_file, &sprop->soildepth, 'd'))
		{
			printf("ERROR reading soildepth: sprop_init()\n");
			errorCode=20816;
		}

		if (!errorCode && sprop->soildepth <= 0)
		{
			printf("ERROR in sprop data in SOIL file: negative or zero soildepth, sprop_init()\n");
			errorCode=2081601;
		}

		/*  ratio of bare soil evaporation and pot.evaporation */
		if (!errorCode && scan_value(sprop_file, &sprop->soilEvapLIM, 'd'))
		{
			printf("ERROR reading soilEvapLIM: sprop_init()\n");
			errorCode=20817;
		}

		/*  maximum height of pond water */
		if (!errorCode && scan_value(sprop_file, &sprop->pondmax, 'd'))
		{
			printf("ERROR reading maximum height of pond water: sprop_init()\n");
			errorCode=20818;
		}

		/*  maximum height of pond water */
		if (!errorCode && scan_value(sprop_file, &sprop->q_soilstress, 'd'))
		{
			printf("ERROR reading q_soilstress: sprop_init()\n");
			errorCode=20818;
		}
	
		/* runoff parameter (Campbell and Diaz)  */
		if (!errorCode && scan_value(sprop_file, &sprop->RCN_mes, 'd'))
		{
			printf("ERROR reading measured runoff curve number: sprop_init()\n");
			errorCode=20819;
		}

		/* aerodynamic paramter (Wallace and Holwill, 1997) */
		if (!errorCode && scan_value(sprop_file, &sprop->aerodyn_resist, 'd'))
		{
			printf("ERROR reading aerodyn_resist: sprop_init()\n");
			errorCode=20819;
		}
	
		/*****************************/
		/* dividing line from file */ 
		if (!errorCode && scan_value(sprop_file, header, 's'))
		{
			printf("ERROR reading 5. dividing line, sprop_init()\n");
			errorCode=20820;
		}
		if (!errorCode && scan_value(sprop_file, header, 's'))
		{
			printf("ERROR reading 5. dividing line, sprop_init()\n");
			errorCode=20820;
		}
		
		/************************************/
		/* SOIL COMPOSITION AND CHARACTERISTIC VALUES (-9999: no measured data) */
	
		/* SAND array - mulilayer soil  */
		scanflag=0; 
		for (layer=0; layer<N_SOILLAYERS; layer++)
		{
			if (layer==N_SOILLAYERS-1) scanflag=1;
			if (!errorCode && scan_array(sprop_file, &(sprop->sand[layer]), 'd', scanflag, 1))
			{
				printf("ERROR reading percent sand in layer %i, sprop_init()\n", layer);
				errorCode=20821;
			}
		}

		/* SILT array - mulilayer soil   */
		scanflag=0; 
		for (layer=0; layer<N_SOILLAYERS; layer++)
		{
			if (layer==N_SOILLAYERS-1) scanflag=1;
			if (!errorCode && scan_array(sprop_file, &(sprop->silt[layer]), 'd', scanflag, 1))
			{
				printf("ERROR reading percent silt in layer %i, sprop_init()\n", layer);
				errorCode=20822;
			}
		}

		/* pH array - mulilayer soil */
		scanflag=0; 
		for (layer=0; layer<N_SOILLAYERS; layer++)
		{
			if (layer==N_SOILLAYERS-1) scanflag=1;
			if (!errorCode && scan_array(sprop_file, &(sprop->pH[layer]), 'd', scanflag, 1))
			{
				printf("ERROR reading soil pH in layer %i, sprop_init()\n", layer);
				errorCode=20823;
			}
		}
	

		/* measured bulk density    */
		scanflag=0; 
		for (layer=0; layer<N_SOILLAYERS; layer++)
		{
			if (layer==N_SOILLAYERS-1) scanflag=1;
			if (!errorCode && scan_array(sprop_file, &(sprop->BD_mes[layer]), 'd', scanflag, 1))
			{
				printf("ERROR reading BD_mes in layer %i, sprop_init()\n", layer);
				errorCode=20824;
			}
		}

	
		/* measured critical VWC values - saturation    */
		scanflag=0; 
		for (layer=0; layer<N_SOILLAYERS; layer++)
		{
			if (layer==N_SOILLAYERS-1) scanflag=1;
			if (!errorCode && scan_array(sprop_file, &(sprop->VWCsat_mes[layer]), 'd', scanflag, 1))
			{
				printf("ERROR reading VWCsat_mes in layer %i, sprop_init()\n", layer);
				errorCode=20825;
			}
		}
	
		/* measured critical VWC values - field capacity     */
		scanflag=0; 
		for (layer=0; layer<N_SOILLAYERS; layer++)
		{
			if (layer==N_SOILLAYERS-1) scanflag=1;
			if (!errorCode && scan_array(sprop_file, &(sprop->VWCfc_mes[layer]), 'd', scanflag, 1))
			{
				printf("ERROR reading VWCfc_mes in layer %i, sprop_init()\n", layer);
				errorCode=20826;
			}
		}
	
		/* measured critical VWC values - wilting point    */
		scanflag=0; 
		for (layer=0; layer<N_SOILLAYERS; layer++)
		{
			if (layer==N_SOILLAYERS-1) scanflag=1;
			if (!errorCode && scan_array(sprop_file, &(sprop->VWCwp_mes[layer]), 'd', scanflag, 1))
			{
				printf("ERROR reading VWCwp_mes in layer %i, sprop_init()\n", layer);
				errorCode=20827;
			}
		}

		/* measured critical VWC values - hygr. water    */
		scanflag=0; 
		for (layer=0; layer<N_SOILLAYERS; layer++)
		{
			if (layer==N_SOILLAYERS-1) scanflag=1;
			if (!errorCode && scan_array(sprop_file, &(sprop->VWChw_mes[layer]), 'd', scanflag, 1))
			{
				printf("ERROR reading VWChw_mes in layer %i, sprop_init()\n", layer);
				errorCode=20828;
			}
		}

		/* measured drainage coeff */
		scanflag=0; 
		for (layer=0; layer<N_SOILLAYERS; layer++)
		{
			if (layer==N_SOILLAYERS-1) scanflag=1;
			if (!errorCode && scan_array(sprop_file, &(sprop->drain_coeff_mes[layer]), 'd', scanflag, 1))
			{
				printf("ERROR reading drain_coeff_mes in layer %i, sprop_init()\n", layer);
				errorCode=20829;
			}
		}

		/* measured hydraulic conductivity  */
		scanflag=0; 
		for (layer=0; layer<N_SOILLAYERS; layer++)
		{
			if (layer==N_SOILLAYERS-1) scanflag=1;
			if (!errorCode && scan_array(sprop_file, &(sprop->conduct_sat_mes[layer]), 'd', scanflag, 1))
			{
				printf("ERROR reading conduct_sat_mes in layer %i, sprop_init()\n", layer);
				errorCode=20830;
			}
		}

	}
	/* new SOI file with 91 lines - no extraSOIparameters.txt */
	else
	{
		/****************************/
		/* SOIL GENERIC PARAMETERS */

		/*  soil depth */
		if (!errorCode && scan_value(sprop_file, &sprop->soildepth, 'd'))
		{
			printf("ERROR reading soildepth: sprop_init()\n");
			errorCode=20816;
		}
		if (!errorCode && sprop->soildepth <= 0)
		{
			printf("ERROR in sprop data in SOIL file: negative or zero soildepth, sprop_init()\n");
			errorCode=2081601;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->soil4_CN, 'd'))
		{
			printf("ERROR reading SOIL4_CN: sprop_init()\n");
			errorCode=20809;
		}
		sprop->soil1_CN = sprop->soil4_CN * (soilCN12_orig/soilCN34_orig);
		sprop->soil2_CN = sprop->soil4_CN * (soilCN12_orig/soilCN34_orig);
		sprop->soil3_CN = sprop->soil4_CN;

		if (!errorCode && scan_value(sprop_file, &sprop->NH4_mobilen_prop, 'd'))
		{
			printf("ERROR reading NH4_mobilen_prop: sprop_init()\n");
			errorCode=20805;
		}
		/* aerodynamic paramter (Wallace and Holwill, 1997) */
		if (!errorCode && scan_value(sprop_file, &sprop->aerodyn_resist, 'd'))
		{
			printf("ERROR reading aerodyn_resist: sprop_init()\n");
			errorCode=20819;
		}

		/****************************/
		/* dividing line from file */ 

		if (!errorCode && scan_value(sprop_file, header, 's'))
		{
			printf("ERROR reading 1. dividing line, sprop_init()\n");
			errorCode=20801;
		}
		if (!errorCode && scan_value(sprop_file, header, 's'))
		{
			printf("ERROR reading 1. dividing line, sprop_init()\n");
			errorCode=20801;
		}

		/****************************/
		/*  DECOMPOSITION, NITRIFICATION AND DENITRIFICATION PARAMETERS  */
		
		if (!errorCode && scan_value(sprop_file, &sprop->Tp1_decomp, 'd'))
		{
			printf("ERROR Tp1_decomp, sprop_init()\n");
			errorCode=20841;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->Tp2_decomp, 'd'))
		{
			printf("ERROR Tp2_decomp, sprop_init()\n");
			errorCode=20842;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->Tp3_decomp, 'd'))
		{
			printf("ERROR Tp3_decomp, sprop_init()\n");
			errorCode=20843;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->Tp4_decomp, 'd'))
		{
			printf("ERROR Tp4_decomp, sprop_init()\n");
			errorCode=20844;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->Tmin_decomp, 'd'))
		{
			printf("ERROR Tp0_decomp, sprop_init()\n");
			errorCode=20845;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->efolding_depth, 'd'))
		{
			printf("ERROR reading e-folding depth: sprop_init()\n");
			errorCode=20807;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->netMiner_to_nitrif, 'd'))
		{
			printf("ERROR reading netMiner_to_nitrif: sprop_init()\n");
			errorCode=20803;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->maxNitrif_rate, 'd'))
		{
			printf("ERROR reading maxNitrif_rate: sprop_init()\n");
			errorCode=20803;
		}	
		if (!errorCode && scan_value(sprop_file, &sprop->N2Ocoeff_nitrif, 'd'))
		{
			printf("ERROR reading N2Ocoeff_nitrif: sprop_init()\n");
			errorCode=20804;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->pHp1_nitrif, 'd'))
		{
			printf("ERROR pHp1_nitrif, sprop_init()\n");
			errorCode=20833;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->pHp2_nitrif, 'd'))
		{
			printf("ERROR pHp2_nitrif, sprop_init()\n");
			errorCode=20834;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->pHp3_nitrif, 'd'))
		{
			printf("ERROR pHp3_nitrif, sprop_init()\n");
			errorCode=20835;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->pHp4_nitrif, 'd'))
		{
			printf("ERROR pHp4_nitrif, sprop_init()\n");
			errorCode=20836;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->Tp1_nitrif, 'd'))
		{
			printf("ERROR Tp1_nitrif, sprop_init()\n");
			errorCode=20837;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->Tp2_nitrif, 'd'))
		{
			printf("ERROR Tp2_nitrif, sprop_init()\n");
			errorCode=20838;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->Tp3_nitrif, 'd'))
		{
			printf("ERROR Tp3_nitrif, sprop_init()\n");
			errorCode=20839;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->Tp4_nitrif, 'd'))
		{
			printf("ERROR Tp4_nitrif, sprop_init()\n");
			errorCode=20840;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->minWFPS_nitrif, 'd'))
		{
			printf("ERROR reading WFPS_min: sprop_init()\n");
			errorCode=20809;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->opt1WFPS_nitrif, 'd'))
		{
			printf("ERROR reading WFPS_opt1: sprop_init()\n");
			errorCode=20809;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->opt2WFPS_nitrif, 'd'))
		{
			printf("ERROR reading WFPS_opt2: sprop_init()\n");
			errorCode=20809;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->scalarWFPSmin_nitrif, 'd'))
		{
			printf("ERROR reading scalarWFPSmin_nitrif: sprop_init()\n");
			errorCode=20809;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->denitr_coeff, 'd'))
		{
			printf("ERROR reading denitr_coeff: sprop_init()\n");
			errorCode=20802;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->N2Oratio_denitr, 'd'))
		{
			printf("ERROR reading N2Oratio_denitr: sprop_init()\n");
			errorCode=20806;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->critWFPS_denitr, 'd'))
		{
			printf("ERROR reading critWFPS_denitr: sprop_init()\n");
			errorCode=20806;
		}
		
		/****************************/
		/* dividing line from file */ 

		if (!errorCode && scan_value(sprop_file, header, 's'))
		{
			printf("ERROR reading 2. dividing line, sprop_init()\n");
			errorCode=20810;
		}
		if (!errorCode && scan_value(sprop_file, header, 's'))
		{
			printf("ERROR reading 2. dividing line, sprop_init()\n");
			errorCode=20810;
		}

		/****************************/
		/* RATE SCALARS */

		/* respiration fractions for fluxes between compartments  */
		if (!errorCode && scan_value(sprop_file, &sprop->rfl1s1, 'd'))
		{
			printf("ERROR reading rfl1s1: sprop_init()\n");
			errorCode=20811;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->rfl2s2, 'd'))
		{
			printf("ERROR reading rfl2s2: sprop_init()\n");
			errorCode=20811;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->rfl4s3, 'd'))
		{
			printf("ERROR reading rfl4s3: sprop_init()\n");
			errorCode=20811;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->rfs1s2, 'd'))
		{
			printf("ERROR reading rfs1s2: sprop_init()\n");
			errorCode=20811;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->rfs2s3, 'd'))
		{
			printf("ERROR reading rfs2s3: sprop_init()\n");
			errorCode=20811;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->rfs3s4, 'd'))
		{
			printf("ERROR reading rfs3s4: sprop_init()\n");
			errorCode=20811;
		}

		/* 	base values of rate constants are (1/day)   */
		if (!errorCode && scan_value(sprop_file, &sprop->kl1_base, 'd'))
		{
			printf("ERROR reading kl1_base: sprop_init()\n");
			errorCode=20812;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->kl2_base, 'd'))
		{
			printf("ERROR reading kl2_base: sprop_init()\n");
			errorCode=20812;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->kl4_base, 'd'))
		{
			printf("ERROR reading kl4_base: sprop_init()\n");
			errorCode=20812;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->ks1_base, 'd'))
		{
			printf("ERROR reading ks1_base: sprop_init()\n");
			errorCode=20812;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->ks2_base, 'd'))
		{
			printf("ERROR reading ks2_base: sprop_init()\n");
			errorCode=20812;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->ks3_base, 'd'))
		{
			printf("ERROR reading ks3_base: sprop_init()\n");
			errorCode=20811;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->ks4_base, 'd'))
		{
			printf("ERROR reading ks4_base: sprop_init()\n");
			errorCode=20812;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->kfrag_base, 'd'))
		{
			printf("ERROR reading kfrag_base: sprop_init()\n");
			errorCode=20812;
		}

		/****************************/
		/* dividing line from file */ 

		if (!errorCode && scan_value(sprop_file, header, 's'))
		{
			printf("ERROR reading 3. dividing line, sprop_init()\n");
			errorCode=20813;
		}
		if (!errorCode && scan_value(sprop_file, header, 's'))
		{
			printf("ERROR reading 3. dividing line, sprop_init()\n");
			errorCode=20813;
		}
	
		/****************************/
		/* SOIL MOISTURE PARAMETERS */

		/*  ratio of bare soil evaporation and pot.evaporation */
		if (!errorCode && scan_value(sprop_file, &sprop->soilEvapLIM, 'd'))
		{
			printf("ERROR reading soilEvapLIM: sprop_init()\n");
			errorCode=20817;
		}
		/*  maximum height of pond water */
		if (!errorCode && scan_value(sprop_file, &sprop->pondmax, 'd'))
		{
			printf("ERROR reading maximum height of pond water: sprop_init()\n");
			errorCode=20818;
		}
		/*  maximum height of pond water */
		if (!errorCode && scan_value(sprop_file, &sprop->q_soilstress, 'd'))
		{
			printf("ERROR reading q_soilstress: sprop_init()\n");
			errorCode=20818;
		}
		/* runoff parameter (Campbell and Diaz)  */
		if (!errorCode && scan_value(sprop_file, &sprop->RCN_mes, 'd'))
		{
			printf("ERROR reading measured runoff curve number: sprop_init()\n");
			errorCode=20819;
		}
		/* fraction of dissolving coefficients  */
		if (!errorCode && scan_value(sprop_file, &sprop->SOIL1_dissolv_prop, 'd'))
		{
			printf("ERROR reading SOIL1_dissolv_prop: sprop_init()\n");
			errorCode=20808;
		}	
		if (!errorCode && scan_value(sprop_file, &sprop->SOIL2_dissolv_prop, 'd'))
		{
			printf("ERROR reading SOIL2_dissolv_prop: sprop_init()\n");
			errorCode=20808;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->SOIL3_dissolv_prop, 'd'))
		{
			printf("ERROR reading SOIL3_dissolv_prop: sprop_init()\n");
			errorCode=20808;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->SOIL4_dissolv_prop, 'd'))
		{
			printf("ERROR reading SOIL4_dissolv_prop: sprop_init()\n");
			errorCode=20808;
		}
		/* mulch parameters  */
		if (!errorCode && scan_value(sprop_file, &sprop->pLAYER_mulch, 'd'))
		{
			printf("ERROR pLAYER_mulch, sprop_init()\n");
			errorCode=20846;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->pCRIT_mulch, 'd'))
		{
			printf("ERROR pCRIT_mulch, sprop_init()\n");
			errorCode=20851;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->p1_mulch, 'd'))
		{
			printf("ERROR p1_mulch, sprop_init()\n");
			errorCode=20847;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->p2_mulch, 'd'))
		{
			printf("ERROR p2_mulch, sprop_init()\n");
			errorCode=20848;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->p3_mulch, 'd'))
		{
			printf("ERROR p3_mulch, sprop_init()\n");
			errorCode=20849;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->pRED_mulch, 'd'))
		{
			printf("ERROR pRED_mulch, sprop_init()\n");
			errorCode=20850;
		}
	
		/* tipping diffusion parameters  */
		if (!errorCode && scan_value(sprop_file, &sprop->p1diffus_tipping, 'd'))
		{
			printf("ERROR p1diffus_tipping, sprop_init()\n");
			errorCode=20852;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->p2diffus_tipping, 'd'))
		{
			printf("ERROR p2diffus_tipping, sprop_init()\n");
			errorCode=20853;
		}
		if (!errorCode && scan_value(sprop_file, &sprop->p3diffus_tipping, 'd'))
		{
			printf("ERROR p3diffus_tipping, sprop_init()\n");
			errorCode=20854;
		}
		/* flag of groundwater calculation method */
		if (!errorCode && scan_value(sprop_file, &ctrl->GW_flag, 'i'))
		{
			printf("ERROR GW_flag, sprop_init()\n");
			errorCode=20855;
		}
		/* capillary fringe */
		if (!errorCode && scan_value(sprop_file, &sprop->CapillFringe_act, 'd'))
		{
			printf("ERROR CapillFringe_act, sprop_init()\n");
			errorCode=20856;
		}

		if (sprop->CapillFringe_act != DATA_GAP) sprop->CapillFringe = sprop->CapillFringe_act;

		/****************************/
		/* dividing line from file */ 
		if (!errorCode && scan_value(sprop_file, header, 's'))
		{
			printf("ERROR reading 4. dividing line, sprop_init()\n");
			errorCode=20820;
		}
		if (!errorCode && scan_value(sprop_file, header, 's'))
		{
			printf("ERROR reading 4. dividing line, sprop_init()\n");
			errorCode=20820;
		}
		
		/****************************/
		/* CH4 PARAMETERS */

		if (!errorCode && scan_value(sprop_file, &sprop->pBD1_CH4, 'd'))
		{
			printf("ERROR reading pBD1_CH4: sprop_init()\n");
			errorCode=20814;
		}

		if (!errorCode && scan_value(sprop_file, &sprop->pBD2_CH4, 'd'))
		{
			printf("ERROR reading pBD2_CH4: sprop_init()\n");
			errorCode=20814;
		}

		if (!errorCode && scan_value(sprop_file, &sprop->pVWC1_CH4, 'd'))
		{
			printf("ERROR reading pVWC1_CH4: sprop_init()\n");
			errorCode=20814;
		}

		if (!errorCode && scan_value(sprop_file, &sprop->pVWC2_CH4, 'd'))
		{
			printf("ERROR reading pVWC2_CH4: sprop_init()\n");
			errorCode=20814;
		}

		if (!errorCode && scan_value(sprop_file, &sprop->pVWC3_CH4, 'd'))
		{
			printf("ERROR reading pVWC3_CH4: sprop_init()\n");
			errorCode=20814;
		}

		if (!errorCode && scan_value(sprop_file, &sprop->C_pVWC4, 'd'))
		{
			printf("ERROR reading C_pVWC4: sprop_init()\n");
			errorCode=20814;
		}

		if (!errorCode && scan_value(sprop_file, &sprop->pTS_CH4, 'd'))
		{
			printf("ERROR reading pTS_CH4: sprop_init()\n");
			errorCode=20814;
		}
	
		/****************************/
		/* dividing line from file */ 
		if (!errorCode && scan_value(sprop_file, header, 's'))
		{
			printf("ERROR reading 5. dividing line, sprop_init()\n");
			errorCode=20820;
		}
		if (!errorCode && scan_value(sprop_file, header, 's'))
		{
			printf("ERROR reading 5. dividing line, sprop_init()\n");
			errorCode=20820;
		}
	
		/****************************/
		/* SOIL COMPOSITION AND CHARACTERISTIC VALUES (-9999: no measured data) */
	
		/* SAND array - mulilayer soil  */
		scanflag=0; 
		for (layer=0; layer<N_SOILLAYERS; layer++)
		{
			if (layer==N_SOILLAYERS-1) scanflag=1;
			if (!errorCode && scan_array(sprop_file, &(sprop->sand[layer]), 'd', scanflag, 1))
			{
				printf("ERROR reading percent sand in layer %i, sprop_init()\n", layer);
				errorCode=20821;
			}
		}

		/* SILT array - mulilayer soil   */
		scanflag=0; 
		for (layer=0; layer<N_SOILLAYERS; layer++)
		{
			if (layer==N_SOILLAYERS-1) scanflag=1;
			if (!errorCode && scan_array(sprop_file, &(sprop->silt[layer]), 'd', scanflag, 1))
			{
				printf("ERROR reading percent silt in layer %i, sprop_init()\n", layer);
				errorCode=20822;
			}
		}

		/* pH array - mulilayer soil */
		scanflag=0; 
		for (layer=0; layer<N_SOILLAYERS; layer++)
		{
			if (layer==N_SOILLAYERS-1) scanflag=1;
			if (!errorCode && scan_array(sprop_file, &(sprop->pH[layer]), 'd', scanflag, 1))
			{
				printf("ERROR reading soil pH in layer %i, sprop_init()\n", layer);
				errorCode=20823;
			}
		}

		/* soilB parameter - mulilayer soil */
		scanflag=0; 
		for (layer=0; layer<N_SOILLAYERS; layer++)
		{
			if (layer==N_SOILLAYERS-1) scanflag=1;
			if (!errorCode && scan_array(sprop_file, &(sprop->soilB[layer]), 'd', scanflag, 1))
			{
				printf("ERROR reading soilB in layer %i, sprop_init()\n", layer);
				errorCode=20823;
			}
		}

		/* measured bulk density    */
		scanflag=0; 
		for (layer=0; layer<N_SOILLAYERS; layer++)
		{
			if (layer==N_SOILLAYERS-1) scanflag=1;
			if (!errorCode && scan_array(sprop_file, &(sprop->BD_mes[layer]), 'd', scanflag, 1))
			{
				printf("ERROR reading BD_mes in layer %i, sprop_init()\n", layer);
				errorCode=20824;
			}
		}

		/* measured critical VWC values - saturation    */
		scanflag=0; 
		for (layer=0; layer<N_SOILLAYERS; layer++)
		{
			if (layer==N_SOILLAYERS-1) scanflag=1;
			if (!errorCode && scan_array(sprop_file, &(sprop->VWCsat_mes[layer]), 'd', scanflag, 1))
			{
				printf("ERROR reading VWCsat_mes in layer %i, sprop_init()\n", layer);
				errorCode=20825;
			}
		}
	
		/* measured critical VWC values - field capacity     */
		scanflag=0; 
		for (layer=0; layer<N_SOILLAYERS; layer++)
		{
			if (layer==N_SOILLAYERS-1) scanflag=1;
			if (!errorCode && scan_array(sprop_file, &(sprop->VWCfc_mes[layer]), 'd', scanflag, 1))
			{
				printf("ERROR reading VWCfc_mes in layer %i, sprop_init()\n", layer);
				errorCode=20826;
			}
		}
	
		/* measured critical VWC values - wilting point    */
		scanflag=0; 
		for (layer=0; layer<N_SOILLAYERS; layer++)
		{
			if (layer==N_SOILLAYERS-1) scanflag=1;
			if (!errorCode && scan_array(sprop_file, &(sprop->VWCwp_mes[layer]), 'd', scanflag, 1))
			{
				printf("ERROR reading VWCwp_mes in layer %i, sprop_init()\n", layer);
				errorCode=20827;
			}
		}

		/* measured critical VWC values - hygr. water    */
		scanflag=0; 
		for (layer=0; layer<N_SOILLAYERS; layer++)
		{
			if (layer==N_SOILLAYERS-1) scanflag=1;
			if (!errorCode && scan_array(sprop_file, &(sprop->VWChw_mes[layer]), 'd', scanflag, 1))
			{
				printf("ERROR reading VWChw_mes in layer %i, sprop_init()\n", layer);
				errorCode=20828;
			}
		}

		/* measured drainage coeff */
		scanflag=0; 
		for (layer=0; layer<N_SOILLAYERS; layer++)
		{
			if (layer==N_SOILLAYERS-1) scanflag=1;
			if (!errorCode && scan_array(sprop_file, &(sprop->drain_coeff_mes[layer]), 'd', scanflag, 1))
			{
				printf("ERROR reading drain_coeff_mes in layer %i, sprop_init()\n", layer);
				errorCode=20829;
			}
		}

		/* measured hydraulic conductivity  */
		scanflag=0; 
		for (layer=0; layer<N_SOILLAYERS; layer++)
		{
			if (layer==N_SOILLAYERS-1) scanflag=1;
			if (!errorCode && scan_array(sprop_file, &(sprop->conduct_sat_mes[layer]), 'd', scanflag, 1))
			{
				printf("ERROR reading conduct_sat_mes in layer %i, sprop_init()\n", layer);
				errorCode=20830;
			}
		}

	}
	

	/* CONTROL - measured VWC values (all or none should to be set) */
	if (sprop->VWCsat_mes[0] == DATA_GAP || sprop->VWCfc_mes[0] == DATA_GAP || sprop->VWCwp_mes[0] == DATA_GAP  || sprop->VWChw_mes[0] == DATA_GAP)
	{	
		if (sprop->VWCsat_mes[0] == DATA_GAP && sprop->VWCfc_mes[0] == DATA_GAP && sprop->VWCwp_mes[0] == DATA_GAP  && sprop->VWChw_mes[0] == DATA_GAP)
		{	
			for (layer=0; layer < N_SOILLAYERS; layer++)
			{
				sprop->VWCsat_mes[layer] = DATA_GAP;
				sprop->VWCfc_mes[layer]  = DATA_GAP;
				sprop->VWCwp_mes[layer]  = DATA_GAP;
				sprop->VWChw_mes[layer]  = DATA_GAP;
			}
		}
		else
		{
			printf("ERROR in measured soil data in SOI file\n");
			printf("All or none critical VWC data should to be set by the user\n");
			errorCode=20831;
		}
	}

	/* MULTILAYER SOIL CALCULATION: calculation of soil properties layer by layer (soilb, BD, PSI and VWC critical values),hydr.conduct and hydr.diffus  */
	if (!errorCode && multilayer_soilcalc(ctrl, sitec, sprop))
	{
		printf("\n");
		printf("ERROR in multilayer_soilcalc() in sprop_init.c\n");
		errorCode=20832; 
	} 

	/* groundwater data initalization */
	sprop->GWD=DATA_GAP;
	sprop->GWlayer=DATA_GAP;

	
	/* extra parameters for nitrification and decompostion from independent file - preparation of MuSo7 */

	




	/* -------------------------------------------*/
	if (dofilecloseSOILPROP) fclose(sprop_file.ptr);

		
	return (errorCode);

}

int soilb_estimation(double sand, double silt, double* soilB, double* VWCsat,double* VWCfc, double* VWCwp,  
	                 double* BD, double* RCN, double* CapillFringe, int* soiltype)
{

	int st=-1;
	int errorCode=0;
	
	double soilb_array[12]			= {3.45,  4.11,  5.26,  6.12,  5.39,  4.02,		7.63,  7.71,	8.56,	9.22,	10.45,	12.46};
	double VWCsat_array[12]			= {0.4,   0.42,  0.44,  0.46,  0.48,  0.49,		0.5,   0.505,	0.51,	0.515,	0.52,	0.525};
	double VWCfc_array[12]			= {0.155, 0.190, 0.250, 0.310, 0.360, 0.380,	0.390, 0.405,	0.420,	0.435,	0.445,	0.460};
	double VWCwp_array[12]			= {0.030, 0.050, 0.090, 0.130, 0.170, 0.190,	0.205, 0.220,	0.240,	0.260,	0.275,	0.290};
	double BD_array[12]				= {1.6,   1.58,  1.56,  1.54,  1.52,  1.5,		1.48,  1.46,	1.44,	1.42,	1.4,	1.38};
	double RCN_array[12]			= {50,    52,    54,    56,    58,    60,		62,    64,		66,		68,		70,		72};
	double CapillFringe_array[12]	= {0.17,  0.19,  0.25,  0.37,  0.68,  1.63,		0.26,  0.47,	1.34,	0.3,	1.92,   0.81};
	                                 //sand l.sand sa.loam  loam  si.loam silt  sa.c.loam  c.loam si.c.loam sa.clay si.clay clay  
	double clay = 100-sand-silt;



	if (silt+1.5*clay < 15)													//sand 
		st=0;	

	if ((silt+1.5*clay >= 15) && (silt+2*clay < 30)) 						//loamy_sand
		st=1;	

	if ((clay >= 7 && clay < 20) && (sand > 52) && (silt+2*clay >= 30))		//sandy_loam
		st=2;  

	if (clay < 7 && silt < 50 && silt+2*clay >= 30)   						//sandy_loam
		st=2;

	if (clay >= 7 && clay < 27 && silt >= 28 && silt < 50 && sand <= 52)	//loam
		st=3;

	if (silt >= 50 && clay >= 12 && clay < 27)								//silt loam
		st=4;

	if (silt >= 50 && silt < 80 && clay < 12)								//silt loam
		st=4;

	if (silt >= 80 && clay < 12)											//silt
		st=5;

	if (clay >= 20 && clay < 35 && silt < 28 && sand > 45)					//sandy_clay_loam
		st=6;

	if (clay >= 27 && clay < 40 && sand > 20 && sand <= 45) 				//clay_loam	 
		st=7;

	if (clay >= 27 && clay < 40 && sand <= 20)  							//silty clay_loam				 
		st=8;

	if (clay >= 35 && sand > 45)  											//sandy_clay					 
		st=9;

	if (clay >= 40 && silt >= 40) 											//silty_clay						 
		st=10;

	if (clay >= 40 && sand <= 45 && silt < 40) 								//clay					 
		st=11;

	*soiltype = st;

	if (st >= 0)
	{
		*soilB			= soilb_array[st];
		*VWCsat			= VWCsat_array[st];
		*VWCfc			= VWCfc_array[st];
		*VWCwp			= VWCwp_array[st];
		*BD				= BD_array[st];
		*RCN			= RCN_array[st];
		*CapillFringe  = CapillFringe_array[st];
	}
	else
	{	
		printf("ERROR in soiltype founding\n");
		errorCode=1;
	}

  return (errorCode);
  
}

int multilayer_soilcalc(control_struct* ctrl,  siteconst_struct* sitec, soilprop_struct* sprop)
{
	int layer;
	double m_to_cm, conduct_sat;


	double sand, silt, clay, PSIfc, PSIwp, PSIsat;
	double soilB, BD, RCN, CapillFringe, VWCsat, VWCfc, VWCwp, VWChw, hydrCONDUCTsat, hydrDIFFUSsat, hydrCONDUCTfc,hydrDIFFUSfc; 
	int errorCode = 0;

	double CapillFringe_layer[N_SOILLAYERS];

	soilB = BD = RCN = CapillFringe = PSIsat = VWCsat =VWCwp = VWChw = hydrCONDUCTsat = hydrDIFFUSsat = hydrCONDUCTfc =hydrDIFFUSfc = 0;
	m_to_cm   = 100;

	
	/* -------------------------------------------------------------------------------------------------------------------------------*/

	/* initialization */
	if (sprop->CapillFringe_act == DATA_GAP) sprop->CapillFringe = 0;

	/* 1. estimated soil water potential at hygroscopic water in MPa (1MPa = 10000cm)  (fc: pF = 2.5; wp: pF = 4.2) */
	sprop->PSIhw  = pow(10,pF_hygroscopw) / (-10000);

	
	/* 2. Calculate the soil pressure-volume coefficients from texture data (soil water content, soil water potential and Clapp-Hornberger parameter) 
		    - different estimation methods 4: modell estimtaion, talajharomszog, measured data )*/

	for (layer=0; layer < N_SOILLAYERS; layer++)
	{

		sand		= sprop->sand[layer];
		silt		= sprop->silt[layer];
		clay		= 100-sand-silt;

		sprop->clay[layer] = clay;

	
		/*  2.1 CONTROL to avoid negative data  */
 		if (sand < 0 || silt < 0 || clay < 0)
		{
			if (!errorCode) printf("ERROR in site data in SOI file: negative sand/silt/clay data, sprop_init()\n");
			if (!errorCode) printf("Check values in initialization file.\n");
			errorCode=1;
		}

		if (sand == 0 && silt == 0 && sprop->pH[layer] == 0)
		{
			if (!errorCode) printf("ERROR in site data in SOI file: user-defined sand and silt and pH data is zero in layer %i\n", layer);
			if (!errorCode) printf("Check values in initialization file.\n");
			errorCode=1;
		}
	

	
		/* 2.2 saturation value of soil water potential */
		PSIsat = -(exp((1.54 - 0.0095*sand + 0.0063*silt)*log(10.0))*9.8e-5);


		if (soilb_estimation(sand, silt, &soilB, &VWCsat, &VWCfc, &VWCwp,&BD, &RCN, &CapillFringe, &ctrl->soiltype))
		{
			if (!errorCode) 
			{
				printf("\n");
				printf("ERROR: soilb_estimation() for sprop_init \n");
			}
			errorCode=1;
		}	

		if (sprop->BD_mes[layer] != (double) DATA_GAP) BD      = sprop->BD_mes[layer];

		/* using the model soil properties */
		if (sprop->VWCsat_mes[layer] == (double) DATA_GAP)
		{
	        /* VWCfc, VWCwp VWChw are the functions of VWCsat and soilB (new HW function from Fodor Nándor */
			VWChw= (0.01 * (100 - sand) + 0.06 * clay)/100.;
			//VWChw = VWCsat * (log(soilB) / log(sprop->PSIhw/PSIsat));
		    PSIfc = exp(VWCsat/VWCfc*log(soilB))*PSIsat;
			PSIwp = exp(VWCsat/VWCwp*log(soilB))*PSIsat;


			/* control for soil type with high clay content  */
			if (VWCsat - VWCfc < 0.001)  VWCfc = VWCsat - 0.001;
			if (VWCfc  - VWCwp < 0.001)  VWCwp = VWCfc  - 0.001;
			if (VWCwp  - VWChw < 0.001)  VWChw = VWCwp  - 0.001;
	
		}
		/* measured soil water content at wilting point, field capacity and saturation in m3/m3 */
		else 	
		{

			VWCsat = sprop->VWCsat_mes[layer];
			VWCfc  = sprop->VWCfc_mes[layer];
			VWCwp  = sprop->VWCwp_mes[layer];
			VWChw  = sprop->VWChw_mes[layer];

			PSIfc = exp(VWCsat/VWCfc*log(soilB))*PSIsat;
			PSIwp = exp(VWCsat/VWCwp*log(soilB))*PSIsat;
			
		}
		
	
		/* 2.4 CONTROL - measured VWC values: SAT>FC>WP>HW */
		if ((VWCsat - VWCfc) < 0.001 || (VWCfc - VWCwp) < 0.001  || (VWCwp - VWChw) < 0.001 || VWChw < 0.001 || VWCsat > 1.0) 
		{
			if (!errorCode) printf("ERROR in measured VWC data in SOI file\n");
			if (!errorCode) printf("rules: VWCsat > VWCfc; VWCfc > VWCwp; VWCwp > VWChw; VWCsat <1.0, VWChw>0.01\n");
			errorCode=1;
		}



		/* 2.5 hydr. conduct and diffusivity at saturation and field capacity(Cosby et al. 1984)*/

		/* if measured data is availabe (in cm/day) */
		if (sprop->conduct_sat_mes[layer] != (double) DATA_GAP)
			hydrCONDUCTsat = sprop->conduct_sat_mes[layer] / 100 / nSEC_IN_DAY;
		else
		{
			hydrCONDUCTsat = 7.05556 * 1e-6 * pow(10, (-0.6+0.0126*sand-0.0064*clay));
			hydrCONDUCTsat = (50*exp(-0.075*clay) + 200*exp(-0.075*(100-sand))) / 100 / nSEC_IN_DAY;
		}

		hydrDIFFUSsat = (soilB * hydrCONDUCTsat * (-100*PSIsat))/VWCsat;
		hydrCONDUCTfc = hydrCONDUCTsat * pow(VWCfc/VWCsat, 2*soilB+3);
		hydrDIFFUSfc = (((soilB * hydrCONDUCTsat * (-100*PSIsat))) / VWCsat) * 
							pow(VWCfc/VWCsat, soilB+2);
			

		if (ctrl->oldSOIfile_flag || sprop->soilB[layer] == DATA_GAP) sprop->soilB[layer] = soilB;
		sprop->BD[layer]                = BD;
		sprop->PSIsat[layer]			= PSIsat;
		sprop->PSIfc[layer]				= PSIfc;
		sprop->PSIwp[layer]				= PSIwp;
		sprop->VWCsat[layer]			= VWCsat;
		sprop->VWCfc[layer]				= VWCfc;
		sprop->VWCfc_base[layer]		= VWCfc;
		sprop->VWCwp[layer]				= VWCwp;
		sprop->VWChw[layer]				= VWChw;
		sprop->hydrCONDUCTsat[layer]	= hydrCONDUCTsat;
		sprop->hydrDIFFUSsat[layer]		= hydrDIFFUSsat;
		sprop->hydrCONDUCTfc[layer]		= hydrCONDUCTfc;
		sprop->hydrDIFFUSfc[layer]		= hydrDIFFUSfc;
		sprop->GWeff[layer]		        = 0;

		CapillFringe_layer[layer]	    = CapillFringe;

		if (sprop->CapillFringe_act == DATA_GAP)
			sprop->CapillFringe += CapillFringe_layer[layer] * sitec->soillayer_thickness[layer] / sitec->soillayer_depth[N_SOILLAYERS-1];

		if (layer == 0) 
		{
			if (sprop->RCN_mes == DATA_GAP)
				sprop->RCN  = RCN;
			else
				sprop->RCN  = sprop->RCN_mes;
		}

		/* soil water conductitvity constans: ratio of the drained of water of a given day [1/day] */

		conduct_sat = sprop->hydrCONDUCTsat[layer] * m_to_cm * nSEC_IN_DAY; // saturated hydraulic conductivity (cm/day = m/s * 100 * sec/day)
		if (sprop->drain_coeff_mes[layer] != DATA_GAP)
			sprop->drain_coeff[layer] = sprop->drain_coeff_mes[layer];
		else
			sprop->drain_coeff[layer] = 0.1122 * pow(conduct_sat,0.339);

	}
	return (errorCode);

}