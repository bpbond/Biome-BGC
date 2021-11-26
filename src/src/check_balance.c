/* 
check_balance.c
daily test of mass balance (water, carbon, and nitrogen state variables)

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2020, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "ini.h"
#include "bgc_struct.h"
#include "bgc_func.h"
#include "bgc_constants.h"

int check_water_balance(wstate_struct* ws, int first_balance)
{
	int errorCode=0;
	static double old_balance;
	
	int layer; 
	double balance, soilw_SUM, soilw_2m;
	balance=soilw_SUM=soilw_2m=0;
	
	/* DAILY CHECK ON WATER BALANCE */

	/* control to avoid negative storage */
	for (layer=0; layer < N_SOILLAYERS; layer++)
	{
		if (ws->soilw[layer] < 0.0)
		{
			printf("\n");
			printf("ERROR: negative soil water content\n");
			errorCode=1;
		}
		soilw_SUM += ws->soilw[layer];
		if (layer < 7) soilw_2m += ws->soilw[layer];

	}
	ws->soilw_SUM = soilw_SUM;
	ws->soilw_2m = soilw_2m;

	if (ws->snoww < 0.0 || ws->canopyw < 0  || ws->soilw_SUM < 0 || ws->pondw < 0)
	{
		printf("\n");
		printf("ERROR: negative water storage\n");
		errorCode=1;
	}
	
	/* sum of sources */
	ws->inW = ws->prcp_src + ws->groundwater_src + ws->IRGsrc_W +  ws->FRZsrc_W;
	
	/* sum of sinks */
	ws->outW = ws->soilEvap_snk + ws->snowsubl_snk + ws->groundwater_snk +
		ws->canopyevap_snk + ws->trans_snk +
		ws->canopyw_THNsnk +			/* thinning */
		ws->canopyw_MOWsnk +			/* mowing */
		ws->canopyw_HRVsnk +			/* harvesting */
		ws->canopyw_PLGsnk +			/* ploughing */
		ws->canopyw_GRZsnk +			/* grazing */
		ws->runoff_snk     +			/* soil-water submodel */
		ws->deeppercolation_snk +		/* soil-water submodel .*/
		ws->pondEvap_snk;

	/* sum of current storage */
	ws->storeW = ws->soilw_SUM + ws->pondw + ws->snoww + ws->canopyw;
	
	/* calculate current balance */
	balance = ws->inW - ws->outW - ws->storeW;
	 
	/* calculate actual maximum balance error */
	if (!first_balance && (fabs(old_balance - balance) > ws->WbalanceERR))
	{
		ws->WbalanceERR = fabs(old_balance - balance);
	}
	old_balance = balance;
	
	return (errorCode);
}

int check_carbon_balance(cstate_struct* cs, int first_balance)
{
	int errorCode=0;
	int layer=0;
	static double old_balance;
	double balance;
	
	/* control avoiding negative pools */
	if (cs->leafc < 0.0 ||  cs->leafc_storage < 0.0 || cs->leafc_transfer < 0.0 || 
		cs->frootc < 0.0 || cs->frootc_storage < 0.0 || cs->frootc_transfer < 0.0 || 
		cs->fruitc < 0.0 || cs->fruitc_storage < 0.0 || cs->fruitc_transfer < 0.0 || 
		cs->softstemc < 0.0 || cs->softstemc_storage < 0.0 || cs->softstemc_transfer < 0.0 || 
		cs->livestemc < 0.0 || cs->livestemc_storage < 0.0 || cs->livestemc_transfer < 0.0 || 
		cs->deadstemc < 0.0 || cs->deadstemc_storage < 0.0 || cs->deadstemc_transfer < 0.0 || 
		cs->livecrootc < 0.0 ||  cs->livecrootc_storage < 0.0 || cs->livecrootc_transfer < 0.0 || 
		cs->deadcrootc < 0.0 || cs->deadcrootc_storage < 0.0 || cs->deadcrootc_transfer < 0.0 || 
		cs->gresp_storage < 0.0 || cs->gresp_transfer < 0.0 ||
		cs->STDBc_leaf < 0 || cs->STDBc_froot < 0 || cs->STDBc_fruit < 0|| cs->STDBc_softstem < 0 || cs->STDBc_nsc < 0 || 
		cs->CTDBc_leaf < 0 || cs->CTDBc_froot < 0 || cs->CTDBc_fruit < 0|| cs->CTDBc_softstem < 0 || cs->CTDBc_nsc < 0 || cs->CTDBc_cstem < 0  || cs->CTDBc_croot < 0)
		{
			printf("\n");
			printf("ERROR: negative carbon stock\n");
			errorCode=1;
		}

	
	/* control avoiding negative pools in SOILC array and calculate soil1c_total */
	cs->soil1c_total	= 0;
	cs->soil2c_total	= 0;
	cs->soil3c_total	= 0;
	cs->soil4c_total	= 0;
	cs->litr1c_total	= 0;
	cs->litr2c_total	= 0;
	cs->litr3c_total	= 0;
	cs->litr4c_total	= 0;
	cs->cwdc_total		= 0;

	/* summarizing soil and litter pools  */
	for (layer=0; layer < N_SOILLAYERS; layer++)
	{
		if (cs->soil1c[layer] < 0.0 || cs->soil2c[layer] < 0.0 || cs->soil3c[layer] < 0.0 || cs-> soil4c[layer] < 0.0)
		{
		 	printf("\n");
			printf("ERROR: negative soil carbon stock\n");
			errorCode=1;
		}
		cs->soil1c_total	+= cs->soil1c[layer];
		cs->soil2c_total	+= cs->soil2c[layer];
		cs->soil3c_total	+= cs->soil3c[layer];
		cs->soil4c_total	+= cs->soil4c[layer];
		cs->soilC[layer]     = cs->soil1c[layer] + cs->soil2c[layer] + cs->soil3c[layer] + cs->soil4c[layer];

		if (cs->soil1_DOC[layer] < 0.0  || cs->soil2_DOC[layer] < 0.0 || cs->soil3_DOC[layer] < 0.0 || cs->soil4_DOC[layer] < 0.0)
		{			
			printf("\n");
			printf("ERROR: negative dissolved soil nitrogen pool\n");
			errorCode=1;
		}
		cs->soil_DOC[layer]     = cs->soil1_DOC[layer] + cs->soil2_DOC[layer] + cs->soil3_DOC[layer] + cs->soil4_DOC[layer];
	
		if (cs->litr1c[layer] < 0.0 || cs->litr2c[layer] < 0.0 || cs->litr3c[layer] < 0.0 || cs-> litr4c[layer] < 0.0  || 
			cs->cwdc[layer] < 0.0)
		{	
			printf("ERROR: negative litter carbon stock\n");
			errorCode=1;
		}
		cs->litr1c_total += cs->litr1c[layer];
		cs->litr2c_total += cs->litr2c[layer];
		cs->litr3c_total += cs->litr3c[layer];
		cs->litr4c_total += cs->litr4c[layer];
		cs->litrC[layer] = cs->litr1c[layer] + cs->litr2c[layer] + cs->litr3c[layer] + cs->litr4c[layer];
		cs->cwdc_total   += cs->cwdc[layer];
	}


	/* summarizing cut-down and standing dead biomass */
	cs->CTDBc_above = cs->CTDBc_leaf  + cs->CTDBc_fruit    + cs->CTDBc_softstem + cs->CTDBc_cstem + cs->CTDBc_nsc;
	cs->CTDBc_below = cs->CTDBc_froot + cs->CTDBc_croot;
	
	cs->STDBc_above = cs->STDBc_leaf  + cs->STDBc_fruit + cs->STDBc_softstem + cs->STDBc_nsc;
	cs->STDBc_below = cs->STDBc_froot;



	/* DAILY CHECK ON CARBON BALANCE */
	
	/* sum of sources: photosynthesis and managenet */
	cs->inC = cs->psnsun_src + cs->psnshade_src + cs->PLTsrc_C + cs->GRZsrc_C  + cs->FRZsrc_C;
	
	/* sum of sinks: respiration, fire and management */
	cs->outC = cs->leaf_mr_snk      + cs->leaf_gr_snk      + cs->froot_mr_snk     + cs->froot_gr_snk + 
			  cs->fruit_mr_snk     + cs->fruit_gr_snk     + cs->softstem_mr_snk  + cs->softstem_gr_snk + 
			  cs->livestem_mr_snk  + cs->livestem_gr_snk  + cs->deadstem_gr_snk  + 
			  cs->livecroot_mr_snk + cs->livecroot_gr_snk + cs->deadcroot_gr_snk + 
			  cs->litr1_hr_snk + cs->litr2_hr_snk + cs->litr4_hr_snk + 
			  cs->soil1_hr_snk + cs->soil2_hr_snk + cs->soil3_hr_snk + cs->soil4_hr_snk + 
			  cs->FIREsnk_C +  cs->Cdeepleach_snk + 
			  cs->GRZsnk_C + cs->THN_transportC + cs->MOW_transportC + cs->HRV_transportC; 
		
		     
		
	/* sum of current storage */
	cs->storeC = cs->leafc      + cs->leafc_storage      + cs->leafc_transfer +
				cs->frootc     + cs->frootc_storage     + cs->frootc_transfer + 
				cs->fruitc     + cs->fruitc_storage     + cs->fruitc_transfer +
				cs->softstemc  + cs->softstemc_storage  + cs->softstemc_transfer +
				cs->livestemc  + cs->livestemc_storage  + cs->livestemc_transfer + 
				cs->deadstemc  + cs->deadstemc_storage  + cs->deadstemc_transfer +
				cs->livecrootc + cs->livecrootc_storage + cs->livecrootc_transfer + 
				cs->deadcrootc + cs->deadcrootc_storage + cs->deadcrootc_transfer + 
				cs->gresp_storage + cs->gresp_transfer  + 
				cs->cwdc_total + 
				cs->litr1c_total + cs->litr2c_total + cs->litr3c_total + cs->litr4c_total + 
				cs->soil1c_total + cs->soil2c_total + cs->soil3c_total + cs->soil4c_total + 
				cs->cpool +
				cs->CTDBc_above + cs->STDBc_above + cs->CTDBc_below + cs->STDBc_below;   
	
	
	/* calculate current balance */
	balance = cs->inC - cs->outC - cs->storeC;
	 
	/* calculate actual maximum balance error */
	if (!first_balance && (fabs(old_balance - balance) > cs->CbalanceERR))
	{
	 	cs->CbalanceERR = fabs(old_balance - balance);
	}
	old_balance = balance;


	return (errorCode);
}		

int check_nitrogen_balance(nstate_struct* ns, int first_balance)
{
	int errorCode=0;
	int layer=0;
	double balance;
	static double old_balance = 0.0;
	
	/* CONTROL AVOIDING NITROGEN POOLS */
	if (ns->leafn < 0.0 || ns->leafn < 0.0 ||  ns->leafn_storage < 0.0 || ns->leafn_transfer < 0.0 || 
		ns->frootn < 0.0 || ns->frootn_storage < 0.0 || ns->frootn_transfer < 0.0 || 
		ns->fruitn < 0.0 || ns->fruitn_storage < 0.0 || ns->fruitn_transfer < 0.0 || 
		ns->softstemn < 0.0 || ns->softstemn_storage < 0.0 || ns->softstemn_transfer < 0.0 || 
		ns->livestemn < 0.0 || ns->livestemn_storage < 0.0 || ns->livestemn_transfer < 0.0 || 
		ns->deadstemn < 0.0 || ns->deadstemn_storage < 0.0 || ns->deadstemn_transfer < 0.0 || 
		ns->livecrootn < 0.0 ||  ns->livecrootn_storage < 0.0 || ns->livecrootn_transfer < 0.0 || 
		ns->deadcrootn < 0.0 || ns->deadcrootn_storage < 0.0 || ns->deadcrootn_transfer < 0.0 ||
		ns->retransn < 0.0 ||
		ns->STDBn_leaf < 0 || ns->STDBn_froot < 0 || ns->STDBn_fruit < 0|| ns->STDBn_softstem < 0 || ns->STDBn_nsc < 0 ||  
		ns->CTDBn_leaf < 0 || ns->CTDBn_froot < 0 || ns->CTDBn_fruit < 0|| ns->CTDBn_softstem < 0 || ns->CTDBn_nsc < 0 || ns->CTDBn_cstem < 0  || ns->CTDBn_croot < 0)
	{	
		printf("\n");
		printf("ERROR: negative plant nitrogen pool\n");
		errorCode=1;
	}

	/* control avoiding negative pools in SOILC array and calculate soil1c_total */
	ns->soil1n_total	= 0;
	ns->soil2n_total	= 0;
	ns->soil3n_total	= 0;
	ns->soil4n_total	= 0;
	ns->sminNH4_total	= 0;
	ns->sminNO3_total	= 0;
	ns->litr1n_total	= 0;
	ns->litr2n_total	= 0;
	ns->litr3n_total	= 0;
	ns->litr4n_total	= 0;
	ns->cwdn_total		= 0;

	/* summarizing soil and litter pools  */
	for (layer=0; layer < N_SOILLAYERS; layer++)
	{
		if (ns->soil1n[layer] < 0.0  || ns->soil2n[layer] < 0.0 || ns->soil3n[layer] < 0.0 || ns->soil4n[layer] < 0.0)
		{			
			printf("\n");
			printf("ERROR: negative soil nitrogen pool\n");
			errorCode=1;
		}
		ns->soil1n_total	+= ns->soil1n[layer];
		ns->soil2n_total	+= ns->soil2n[layer];
		ns->soil3n_total	+= ns->soil3n[layer];
		ns->soil4n_total	+= ns->soil4n[layer];
		ns->soilN[layer]     = ns->soil1n[layer] + ns->soil2n[layer] + ns->soil3n[layer] + ns->soil4n[layer];

		if (ns->soil1_DON[layer] < 0.0  || ns->soil2_DON[layer] < 0.0 || ns->soil3_DON[layer] < 0.0 || ns->soil4_DON[layer] < 0.0)
		{			
			printf("\n");
			printf("ERROR: negative dissolved soil nitrogen pool\n");
			errorCode=1;
		}
		ns->soil_DON[layer]     = ns->soil1_DON[layer] + ns->soil2_DON[layer] + ns->soil3_DON[layer] + ns->soil4_DON[layer];
		

		if (ns->litr1n[layer] < 0.0 || ns->litr2n[layer] < 0.0 || ns->litr3n[layer] < 0.0 || ns-> litr4n[layer] < 0.0 || ns->cwdn[layer] < 0.0)
		{	
			printf("\n");
			printf("ERROR: negative litter nitrogen pool\n");
			errorCode=1;
		}
		ns->litr1n_total	+= ns->litr1n[layer];
		ns->litr2n_total	+= ns->litr2n[layer];
		ns->litr3n_total	+= ns->litr3n[layer];
		ns->litr4n_total	+= ns->litr4n[layer];
		ns->litrN[layer]     = ns->litr1n[layer] + ns->litr2n[layer] + ns->litr3n[layer] + ns->litr4n[layer];
		ns->cwdn_total		+= ns->cwdn[layer];

		if (ns->sminNH4[layer] < 0 || ns->sminNO3[layer] < 0 )
		{
			printf("\n");
			printf("ERROR: negative mineralized nitrogen pool\n");
			errorCode=1;
		}
	
		ns->sminNH4_total	+= ns->sminNH4[layer];
		ns->sminNO3_total	+= ns->sminNO3[layer];
	}
	
	/* summarizing cut-down and standing dead biomass */
	ns->CTDBn_above = ns->CTDBn_leaf  + ns->CTDBn_fruit + ns->CTDBn_softstem + ns->CTDBn_cstem + ns->CTDBn_nsc;
	ns->CTDBn_below = ns->CTDBn_froot + ns->CTDBn_croot;

	ns->STDBn_above = ns->STDBn_leaf + ns->STDBn_fruit + ns->STDBn_softstem  + ns->STDBn_nsc;
	ns->STDBn_below = ns->STDBn_froot;



	/* DAILY CHECK ON NITROGEN BALANCE */
	
	/* sum of sources: fixation, deposition, spinup add, management */
	ns->inN = ns->Nfix_src + ns->Ndep_src + ns->SPINUPsrc +
			  ns->PLTsrc_N +  ns->GRZsrc_N +  ns->FRZsrc_N;
	
	
	/* sum of sinks: volatilization, fire, deep leach, management */
	ns->outN = ns->Nvol_snk + ns->Nprec_snk + ns->FIREsnk_N + ns->Ndeepleach_snk + 
			   ns->GRZsnk_N + ns->THN_transportN + ns->MOW_transportN+  + ns->HRV_transportN;

		
	/* sum of current storage */
	ns->storeN = ns->leafn      + ns->leafn_storage      + ns->leafn_transfer +
				 ns->frootn     + ns->frootn_storage     + ns->frootn_transfer + 			
				ns->fruitn     + ns->fruitn_storage     + ns->fruitn_transfer + 
				ns->softstemn  + ns->softstemn_storage  + ns->softstemn_transfer + 
				ns->livestemn  + ns->livestemn_storage  + ns->livestemn_transfer + 
				ns->deadstemn  + ns->deadstemn_storage  + ns->deadstemn_transfer + 
				ns->livecrootn + ns->livecrootn_storage + ns->livecrootn_transfer + 
				ns->deadcrootn + ns->deadcrootn_storage + ns->deadcrootn_transfer +
				ns->retransn + 
				ns->litr1n_total + ns->litr2n_total + ns->litr3n_total + ns->litr4n_total +
				ns->soil1n_total + ns->soil2n_total + ns->soil3n_total + ns->soil4n_total +
				ns->sminNH4_total  + ns->sminNO3_total  + ns->cwdn_total + 
				ns->npool        + 
				ns->CTDBn_above + ns->STDBn_above + ns->CTDBn_below + ns->STDBn_below;

	
	/* calculate current balance */
	balance = ns->inN - ns->outN - ns->storeN;
	 
	/* calculate actual maximum balance error */
	if (!first_balance && (fabs(old_balance - balance) > ns->NbalanceERR))
	{
		ns->NbalanceERR = fabs(old_balance - balance);

	}
	old_balance = balance;

	
	
	return (errorCode);
}

