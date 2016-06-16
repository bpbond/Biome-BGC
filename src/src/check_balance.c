/* 
check_balance.c
daily test of mass balance (water, carbon, and nitrogen state variables)

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
BBGC MuSo v4
Copyright 2000, Peter E. Thornton
Numerical Terradynamics Siulation Group
Copyright 2014, D. Hidy (dori.hidy@gmail.com)
Hungarian Academy of Sciences
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "bgc_struct.h"
#include "bgc_func.h"
#include "bgc_constants.h"

int check_water_balance(wstate_struct* ws, int first_balance)
{
	int ok=1;
	static double old_balance;
	double in, out, store, balance;
	
	/* DAILY CHECK ON WATER BALANCE */
	
	/* sum of sources */
	in = ws->prcp_src + ws->deeptrans_src + ws->groundwater_src + ws->IRGsrc;
	
	/* sum of sinks */
	out = ws->soilevap_snk + ws->snowsubl_snk + 
		ws->canopyevap_snk + ws->trans_snk + ws->pondwevap_snk +
		ws->canopyw_THNsnk +		/* thinning - Hidy 2012.*/
		ws->canopyw_MOWsnk +		/* mowing - Hidy 2008.*/
		ws->canopyw_HRVsnk +		/* harvesting - Hidy 2008.*/
		ws->canopyw_PLGsnk +			/* ploughing - Hidy 2008.*/
		ws->canopyw_GRZsnk +			/* grazing - Hidy 2008.*/
		ws->runoff_snk	  +			/* soil-water submodel - Hidy 2010.*/
		ws->deeppercolation_snk +   /* soil-water submodel - Hidy 2010.*/
		ws->deepdiffusion_snk;		/* soil-water submodel - Hidy 2010.*/

	/* sum of current storage */
	store = ws->soilw_SUM + ws->pond_water + ws->snoww + ws->canopyw;

	if (ws->snoww < 0.0 || ws->canopyw < 0  || ws->soilw_SUM < 0 || ws->pond_water < 0)
	{	
		printf("ERROR: negative water storage\n");
		ok=0;
	}
	
	/* calculate current balance */
	balance = in - out - store;
	 
	if (!first_balance)
	{
		if (fabs(old_balance - balance) > 1e-4)
		{
			printf("WARNING: Water balance error:\n");
			printf("Difference (previous - current) = %lf\n",old_balance-balance);
		}
	}
	old_balance = balance;
	
	return (!ok);
}

int check_carbon_balance(cstate_struct* cs, int first_balance)
{
	int ok=1;
	static double old_balance;
	double in, out, store, balance;
	
	/* Hidy 2010 - control avoiding negative pools */
	if (cs->leafc < 0.0 ||  cs->leafc_storage < 0.0 || cs->leafc_transfer < 0.0 || 
	cs->frootc < 0.0 || cs->frootc_storage < 0.0 || cs->frootc_transfer < 0.0 || 
	cs->fruitc < 0.0 || cs->fruitc_storage < 0.0 || cs->fruitc_transfer < 0.0 || 
	cs->softstemc < 0.0 || cs->softstemc_storage < 0.0 || cs->softstemc_transfer < 0.0 || 
	cs->livestemc < 0.0 || cs->livestemc_storage < 0.0 || cs->livestemc_transfer < 0.0 || 
	cs->deadstemc < 0.0 || cs->deadstemc_storage < 0.0 || cs->deadstemc_transfer < 0.0 || 
	cs->livecrootc < 0.0 ||  cs->livecrootc_storage < 0.0 || cs->livecrootc_transfer < 0.0 || 
	cs->deadcrootc < 0.0 || cs->deadcrootc_storage < 0.0 || cs->deadcrootc_transfer < 0.0 || 
	cs->gresp_storage < 0.0 || cs->gresp_transfer < 0.0 || cs->cwdc < 0.0 || 
	cs->litr1c < 0.0 || cs->litr2c < 0.0 || cs->litr3c < 0.0 || cs->litr4c < 0.0 || 
	cs->soil1c < 0.0 || cs->soil2c < 0.0 || cs->soil3c < 0.0 || cs-> soil4c < 0.0 || cs->litr_aboveground < 0.0)
	{	
		printf("ERROR: negative carbon stock\n");
		ok=0;
	}


	/* DAILY CHECK ON CARBON BALANCE */
	
	/* sum of sources */
	in = cs->psnsun_src + cs->psnshade_src + 
		/* senescence - Hidy 2012 */
		 cs->SNSCsrc +
		/* management - Hidy 2012. */
		cs->PLTsrc + cs->THNsrc +  cs->MOWsrc + cs->GRZsrc + cs->HRVsrc + cs->PLGsrc + cs->FRZsrc;
	
	/* sum of sinks */
	out = cs->leaf_mr_snk + cs->leaf_gr_snk + cs->froot_mr_snk + 
		cs->froot_gr_snk + cs->livestem_mr_snk + cs->livestem_gr_snk + 
		cs->deadstem_gr_snk + cs->livecroot_mr_snk + cs->livecroot_gr_snk + 
		cs->deadcroot_gr_snk + cs->litr1_hr_snk + cs->litr2_hr_snk + 
		cs->litr4_hr_snk + cs->soil1_hr_snk + cs->soil2_hr_snk + 
		cs->soil3_hr_snk + cs->soil4_hr_snk + cs->fire_snk + 
		/* management and senescence - Hidy 2012. */
		cs->SNSCsnk + cs->THNsnk + cs->MOWsnk + cs->GRZsnk + cs->HRVsnk + cs->PLGsnk + 
		/* fruit simulation - Hidy 2013. */
		cs->fruit_gr_snk + cs->fruit_mr_snk +
		/* softstem simulation - Hidy 2013. */
		cs->softstem_gr_snk + cs->softstem_mr_snk; 
		     
		
	/* sum of current storage */
	store = cs->leafc + cs->leafc_storage + cs->leafc_transfer +
		cs->frootc + cs->frootc_storage + cs->frootc_transfer + 
		cs->livestemc + cs->livestemc_storage + cs->livestemc_transfer + 
		cs->deadstemc + cs->deadstemc_storage + cs->deadstemc_transfer +
		cs->livecrootc + cs->livecrootc_storage + cs->livecrootc_transfer + 
		cs->deadcrootc + cs->deadcrootc_storage + cs->deadcrootc_transfer + 
		cs->gresp_storage + cs->gresp_transfer + cs->cwdc + cs->litr1c +
		cs->litr2c + cs->litr3c + cs->litr4c + cs->soil1c + cs->soil2c +
		cs->soil3c + cs->soil4c + cs->cpool +
		/* fruit simulation */
		cs->fruitc + cs->fruitc_storage + cs->fruitc_transfer +
		/* softstem simulation */
		cs->softstemc + cs->softstemc_storage + cs->softstemc_transfer;   
	
	/* calculate current balance */
	balance = in - out - store;
	 
	if (!first_balance)
	{
		if (fabs(old_balance - balance) > 1e-4)
		{
	 		printf("WARNING: carbon balance error:\n");
			printf("Difference (previous - current) = %lf\n",old_balance-balance);
		}
	}
	old_balance = balance;


	return (!ok);
}		

int check_nitrogen_balance(nstate_struct* ns, int first_balance)
{
	int ok=1;
	double in,out,store,balance;
	static double old_balance = 0.0;
	
	/* Hidy 2010 -	CONTROL AVOIDING NITROGEN POOLS */
	if (ns->leafn < 0.0 || ns->leafn < 0.0 ||  ns->leafn_storage < 0.0 || ns->leafn_transfer < 0.0 || 
		ns->frootn < 0.0 || ns->frootn_storage < 0.0 || ns->frootn_transfer < 0.0 || 
		ns->fruitn < 0.0 || ns->fruitn_storage < 0.0 || ns->fruitn_transfer < 0.0 || 
		ns->softstemn < 0.0 || ns->softstemn_storage < 0.0 || ns->softstemn_transfer < 0.0 || 
		ns->livestemn < 0.0 || ns->livestemn_storage < 0.0 || ns->livestemn_transfer < 0.0 || 
		ns->deadstemn < 0.0 || ns->deadstemn_storage < 0.0 || ns->deadstemn_transfer < 0.0 || 
		ns->livecrootn < 0.0 ||  ns->livecrootn_storage < 0.0 || ns->livecrootn_transfer < 0.0 || 
		ns->deadcrootn < 0.0 || ns->deadcrootn_storage < 0.0 || ns->deadcrootn_transfer < 0.0 || 
		ns->cwdn < 0.0 || 
		ns->sminn[0] < 0.0 || ns->sminn[1] < 0.0 || ns->sminn[2] < 0.0 || ns->sminn[3] < 0.0 || 
		ns->sminn[4] < 0.0 || ns->sminn[5] < 0.0 || ns->sminn[6] < 0.0 || 
		ns->litr1n < 0.0  || ns->litr2n < 0.0  || ns->litr3n < 0.0  || ns->litr4n < 0.0 || 
		ns->soil1n < 0.0  || ns->soil2n < 0.0  || ns->soil3n < 0.0  || ns->soil4n < 0.0)
	{	
		printf("ERROR: negative nitrogen stock\n");
		ok=0;
	}


	/* DAILY CHECK ON NITROGEN BALANCE */
	
	/* sum of sources */
	in = ns->nfix_src + ns->ndep_src + 
		/* effect of boundary layer with constant N-content - Hidy 2015 */
		ns->BNDRYsrc +
		/*  senescence */
		ns->SNSCsrc +	/*  senescence */
		/* management */
		ns->PLTsrc + ns->THNsrc +  ns->MOWsrc + ns->GRZsrc + ns->HRVsrc + ns->PLGsrc + ns->FRZsrc;
	
	
	/* sum of sinks */
	out = ns->nvol_snk + ns->fire_snk + //ns->nleached_snk + ns->ndiffused_snk + 
		/*  senescence */
		ns->SNSCsnk + 
		/* management */
		ns->THNsnk + ns->MOWsnk + ns->GRZsnk + ns->HRVsnk + ns->PLGsnk;

		
	/* sum of current storage */
	store = ns->leafn + ns->leafn_storage + ns->leafn_transfer +
		ns->frootn + ns->frootn_storage + ns->frootn_transfer + 
		ns->livestemn + ns->livestemn_storage + ns->livestemn_transfer + 
		ns->deadstemn + ns->deadstemn_storage + ns->deadstemn_transfer + 
		ns->livecrootn + ns->livecrootn_storage + ns->livecrootn_transfer + 
		ns->deadcrootn + ns->deadcrootn_storage + ns->deadcrootn_transfer + 
		ns->cwdn + ns->litr1n + ns->litr2n + ns->litr3n + ns->litr4n +
		ns->soil1n + ns->soil2n + ns->soil3n + ns->soil4n +
		ns->sminn[0] + ns->sminn[1] + ns->sminn[2] + ns->sminn[3] + ns->sminn[4] + ns->sminn[5] + ns->sminn[6] +
		ns->npool + ns->retransn +
		/* fruit simulation */
		ns->fruitn + ns->fruitn_storage + ns->fruitn_transfer +
		/* softstem simulation */
		ns->softstemn + ns->softstemn_storage + ns->softstemn_transfer;
	
	/* calculate current balance */
	balance = in - out - store;
	 
	if (!first_balance)
	{
		if (fabs(old_balance - balance) > 1e-4)
		{
			printf("WARNING: nitrogen balance error:\n");
			printf("Difference (previous - current) = %lf\n",old_balance-balance);
		}
	}
	old_balance = balance;

	
	
	return (!ok);
}

