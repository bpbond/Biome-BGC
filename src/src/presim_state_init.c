/* 
presim_state_init.c
Initialize water, carbon, and nitrogen state variables to 0.0 before
each simulation.

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.1
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2017, D. Hidy [dori.hidy@gmail.com]
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

int presim_state_init(wstate_struct* ws, cstate_struct* cs, nstate_struct* ns, cinit_struct* cinit)
{
	int ok=1;
	int layer;
	
	/* Hidy 2010 - multilayer soil */
	for (layer = 0; layer < N_SOILLAYERS; layer ++)
	{
		ws->soilw[layer] = 0.0;
	}
	ws->soilw_SUM = 0.0;
	ws->pond_water = 0.0;
	ws->snoww=0;
	ws->canopyw=0;
	ws->prcp_src=0;
	ws->soilevap_snk=0;
	ws->snowsubl_snk=0;
	ws->canopyevap_snk=0;
	ws->pondwevap_snk=0;
	ws->trans_snk=0;
	ws->runoff_snk=0;
	ws->deeppercolation_snk=0;
	ws->deepdiffusion_snk=0;
	ws->deeptrans_src=0;
	ws->groundwater_src=0;
	ws->pondwater_src=0;
	ws->canopyw_THNsnk=0;
	ws->canopyw_MOWsnk=0;
	ws->canopyw_HRVsnk=0;
	ws->canopyw_PLGsnk=0;
	ws->canopyw_GRZsnk=0;
	ws->IRGsrc=0;
	ws->balanceERR = 0;

	cinit->max_leafc = 0.0;
	cinit->max_stemc = 0.0;
	
	cs->leafc = 0.0;
	cs->leafc_storage = 0.0;
	cs->leafc_transfer = 0.0;
	cs->frootc = 0.0;
	cs->frootc_storage = 0.0;
	cs->frootc_transfer = 0.0;
	cs->livestemc = 0.0;
	cs->livestemc_storage = 0.0;
	cs->livestemc_transfer = 0.0;
	cs->deadstemc = 0.0;
	cs->deadstemc_storage = 0.0;
	cs->deadstemc_transfer = 0.0;
	cs->livecrootc = 0.0;
	cs->livecrootc_storage = 0.0;
	cs->livecrootc_transfer = 0.0;
	cs->deadcrootc = 0.0;
	cs->deadcrootc_storage = 0.0;
	cs->deadcrootc_transfer = 0.0;
	cs->gresp_storage = 0.0;
	cs->gresp_transfer = 0.0;
	cs->cwdc = 0.0;
	cs->litr1c = 0.0;
	cs->litr2c = 0.0;
	cs->litr3c = 0.0;
	cs->litr4c = 0.0;
	cs->litr_belowground = 0.0;
	cs->litr_aboveground = 0.0;
	/* Hidy 2013 - senescence */
	cs->STDB_litr1c = 0.0;
	cs->STDB_litr2c = 0.0;
	cs->STDB_litr3c = 0.0;
	cs->STDB_litr4c = 0.0;
	cs->STDBc = 0.0;
	cs->CTDB_litr1c = 0.0;
	cs->CTDB_litr2c = 0.0;
	cs->CTDB_litr3c = 0.0;
	cs->CTDB_litr4c = 0.0;
	cs->CTDB_cwdc  = 0.0;
	cs->CTDBc = 0.0;
	cs->soil1c = 0.0;
	cs->soil2c = 0.0;
	cs->soil3c = 0.0;
	cs->soil4c = 0.0;
	cs->cpool = 0.0;
	cs->psnsun_src = 0.0;
	cs->psnshade_src = 0.0;
	cs->leaf_mr_snk = 0.0;
	cs->leaf_gr_snk = 0.0;
	cs->froot_mr_snk = 0.0;
	cs->froot_gr_snk = 0.0;
	cs->livestem_mr_snk = 0.0;
	cs->livestem_gr_snk = 0.0;
	cs->deadstem_gr_snk = 0.0;
	cs->livecroot_mr_snk = 0.0;
	cs->livecroot_gr_snk = 0.0;
	cs->deadcroot_gr_snk = 0.0;
	cs->litr1_hr_snk = 0.0;
	cs->litr2_hr_snk = 0.0;
	cs->litr4_hr_snk = 0.0;
	cs->soil1_hr_snk = 0.0;
	cs->soil2_hr_snk = 0.0;
	cs->soil3_hr_snk = 0.0;
	cs->soil4_hr_snk = 0.0;
	cs->fire_snk = 0.0;
    /* planting - Hidy 2012.*/
	cs->PLTsrc = 0.0; 
	/* thinning - Hidy 2012.*/
	cs->THNsnk = 0.0; 
	cs->THNsrc = 0.0;
	cs->THN_transportC  = 0.0;
	/* mowing - Hidy 2008.*/
	cs->MOWsnk = 0.0; 
	cs->MOWsrc = 0.0;
	cs->MOW_transportC = 0.0;
	/* grazing - Hidy 2008. */
	cs->GRZsnk = 0.0;  
	cs->GRZsrc = 0.0;
	/* harvesting - Hidy 2012. */
	cs->HRVsnk = 0.0;  
	cs->HRVsrc = 0.0;
	cs->HRV_transportC = 0.0;
	/* ploughing - Hidy 2012. */
	cs->PLGsnk = 0.0;	
	cs->PLGsrc = 0.0;
	cs->PLG_cpool = 0.0;
	/* fertilizing - Hidy 2009.*/
	cs->FRZsrc = 0.0;
	/* senescence - Hidy 2012.*/
	cs->SNSCsnk= 0.0;
	cs->SNSCsrc= 0.0;
	/* fruit simulation - Hidy 2013.*/
	cs->fruitc = 0.0;
	cs->fruitc_storage = 0.0;
	cs->fruitc_transfer = 0.0;
	cs->fruit_gr_snk = 0.0;
	cs->fruit_mr_snk = 0.0;
	/* softstem simulation - Hidy 2013.*/
	cs->softstemc = 0.0;
	cs->softstemc_storage = 0.0;
	cs->softstemc_transfer = 0.0;
	cs->softstem_gr_snk = 0.0;
	cs->softstem_mr_snk = 0.0;
	cs->balanceERR = 0;
	
	ns->leafn = 0.0;
	ns->leafn_storage = 0.0;
	ns->leafn_transfer = 0.0;
	ns->frootn = 0.0;
	ns->frootn_storage = 0.0;
	ns->frootn_transfer = 0.0;
	ns->livestemn = 0.0;
	ns->livestemn_storage = 0.0;
	ns->livestemn_transfer = 0.0;
	ns->deadstemn = 0.0;
	ns->deadstemn_storage = 0.0;
	ns->deadstemn_transfer = 0.0;
	ns->livecrootn = 0.0;
	ns->livecrootn_storage = 0.0;
	ns->livecrootn_transfer = 0.0;
	ns->deadcrootn = 0.0;
	ns->deadcrootn_storage = 0.0;
	ns->deadcrootn_transfer = 0.0;
	ns->cwdn = 0.0;
	ns->litr1n = 0.0;
	ns->litr2n = 0.0;
	ns->litr3n = 0.0;
	ns->litr4n = 0.0;
	/* Hidy 2013 - senescence */
	ns->STDB_litr1n = 0.0;
	ns->STDB_litr2n = 0.0;
	ns->STDB_litr3n = 0.0;
	ns->STDB_litr4n = 0.0;
	ns->STDBn = 0.0;
	ns->CTDB_litr1n = 0.0;
	ns->CTDB_litr2n = 0.0;
	ns->CTDB_litr3n = 0.0;
	ns->CTDB_litr4n = 0.0;
	ns->CTDB_cwdn = 0.0;
	cs->CTDBc = 0.0;
	ns->soil1n = 0.0;
	ns->soil2n = 0.0;
	ns->soil3n = 0.0;
	ns->soil4n = 0.0;
	/* Hidy 2011 - multilayer soil */
	for (layer = 0; layer < N_SOILLAYERS; layer ++)
	{
		ns->sminn[layer] = 0.0;
	}
	ns->sminn_RZ = 0.0;
	ns->retransn = 0.0;
	ns->npool = 0.0;
	ns->nfix_src = 0.0;
	ns->ndep_src = 0.0;
	ns->nleached_snk = 0.0;
	ns->ndiffused_snk = 0.0;
	ns->nvol_snk = 0.0;
	ns->fire_snk = 0.0;
	/* effect of boundary layer with constant N-content - Hidy 2015 */
	ns->BNDRYsrc = 0.0;
	ns->sum_ndemand = 0.0;
	/* planting - Hidy 2012. */
	ns->PLTsrc  = 0.0;  
	/* thining - Hidy 2012. */
	ns->THNsnk = 0.0;  
	ns->THNsrc = 0.0;
	ns->THN_transportN  = 0.0;
	/* mowing - Hidy 2008. */
	ns->MOWsnk = 0.0;  
	ns->MOWsrc = 0.0;
	ns->MOW_transportN = 0.0;
	 /* grazing - Hidy 2008. */
	ns->GRZsnk = 0.0; 
	ns->GRZsrc = 0.0;
	/* harvesting - Hidy 2012. */
	ns->HRVsnk = 0.0;  
	ns->HRVsrc = 0.0;
	ns->HRV_transportN = 0.0;
	/* ploughing - Hidy 2012. */
	ns->PLGsnk = 0.0;	
	ns->PLGsrc = 0.0;
	ns->PLG_npool = 0.0;
    /* fertilization - Hidy 2008. */
	ns->FRZsrc = 0.0;  
	/* senescence - Hidy 2012.*/
	ns->SNSCsnk= 0.0;
	ns->SNSCsrc= 0.0;
	/* fruit simulation - Hidy 2013.*/
	ns->fruitn = 0.0;
	ns->fruitn_storage = 0.0;
	ns->fruitn_transfer = 0.0;
	/* softstem simulation - Hidy 2013.*/
	ns->softstemn = 0.0;
	ns->softstemn_storage = 0.0;
	ns->softstemn_transfer = 0.0;
	
	ns->balanceERR = 0;
	return(!ok);
}
	
