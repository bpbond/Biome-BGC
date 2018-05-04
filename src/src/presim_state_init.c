/* 
presim_state_init.c
Initialize water, carbon, and nitrogen state variables to 0.0 before
each simulation.

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2018 D. Hidy [dori.hidy@gmail.com]
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
	

	cinit->max_leafc = 0.0;
	cinit->max_frootc = 0.0;
	cinit->max_fruitc = 0.0;
	cinit->max_softstemc = 0.0;
	cinit->max_livestemc = 0.0;
	cinit->max_livecrootc = 0.0;
	
	ws->soilw_SUM = 0;
	ws->pond_water = 0;
	ws->snoww = 0;
	ws->canopyw = 0;
	ws->prcp_src = 0;
	ws->soilevap_snk = 0;
	ws->snowsubl_snk = 0;
	ws->canopyevap_snk = 0;
	ws->pondwevap_snk = 0;
	ws->trans_snk = 0;
	ws->runoff_snk = 0;
	ws->deeppercolation_snk = 0;
	ws->groundwater_src = 0;
	ws->canopyw_THNsnk = 0;
	ws->canopyw_MOWsnk = 0;
	ws->canopyw_HRVsnk = 0;
	ws->canopyw_PLGsnk = 0;
	ws->canopyw_GRZsnk = 0;
	ws->IRGsrc = 0;
	ws->FRZsrc = 0;
	ws->balanceERR = 0;
	ws->in = 0;
	ws->out = 0;
	ws->store = 0;
	cs->leafc = 0;
	cs->leafc_storage = 0;
	cs->leafc_transfer = 0;
	cs->frootc = 0;
	cs->frootc_storage = 0;
	cs->frootc_transfer = 0;
	cs->fruitc = 0;
	cs->fruitc_storage = 0;
	cs->fruitc_transfer = 0;
	cs->softstemc = 0;
	cs->softstemc_storage = 0;
	cs->softstemc_transfer = 0;
	cs->livestemc = 0;
	cs->livestemc_storage = 0;
	cs->livestemc_transfer = 0;
	cs->deadstemc = 0;
	cs->deadstemc_storage = 0;
	cs->deadstemc_transfer = 0;
	cs->livecrootc = 0;
	cs->livecrootc_storage = 0;
	cs->livecrootc_transfer = 0;
	cs->deadcrootc = 0;
	cs->deadcrootc_storage = 0;
	cs->deadcrootc_transfer = 0;
	cs->gresp_storage = 0;
	cs->gresp_transfer = 0;
	cs->litr1c_total = 0;
	cs->litr2c_total = 0;
	cs->litr3c_total = 0;
	cs->litr4c_total = 0;
	cs->cwdc_total = 0;
	cs->STDBc_leaf = 0;
	cs->STDBc_froot = 0;
	cs->STDBc_fruit = 0;
	cs->STDBc_softstem = 0;
	cs->STDBc_transfer = 0;
	cs->STDBc_above = 0;
	cs->STDBc_below = 0;
	cs->CTDBc_leaf = 0;
	cs->CTDBc_froot = 0;
	cs->CTDBc_fruit = 0;
	cs->CTDBc_softstem = 0;
	cs->CTDBc_transfer = 0;
	cs->CTDBc_cstem = 0;
	cs->CTDBc_croot = 0;
	cs->CTDBc_above = 0;
	cs->CTDBc_below = 0;
	cs->soil1c_total = 0;
	cs->soil2c_total = 0;
	cs->soil3c_total = 0;
	cs->soil4c_total = 0;
	cs->cpool = 0;
	cs->psnsun_src = 0;
	cs->psnshade_src = 0;
	cs->NSC_mr_snk = 0;
	cs->actC_mr_snk = 0;
	cs->leaf_mr_snk = 0;
	cs->leaf_gr_snk = 0;
	cs->froot_mr_snk = 0;
	cs->froot_gr_snk = 0;
	cs->fruit_gr_snk = 0;
	cs->fruit_mr_snk = 0;
	cs->softstem_gr_snk = 0;
	cs->softstem_mr_snk = 0;
	cs->livestem_mr_snk = 0;
	cs->livestem_gr_snk = 0;
	cs->deadstem_gr_snk = 0;
	cs->livecroot_mr_snk = 0;
	cs->livecroot_gr_snk = 0;
	cs->deadcroot_gr_snk = 0;
	cs->litr1_hr_snk = 0;
	cs->litr2_hr_snk = 0;
	cs->litr4_hr_snk = 0;
	cs->soil1_hr_snk = 0;
	cs->soil2_hr_snk = 0;
	cs->soil3_hr_snk = 0;
	cs->soil4_hr_snk = 0;
	cs->fire_snk = 0;
	cs->SNSCsnk = 0;
	cs->PLTsrc = 0;
	cs->THN_transportC = 0;
	cs->HRV_transportC = 0;
	cs->MOW_transportC = 0;
	cs->GRZsnk = 0;
	cs->GRZsrc = 0;
	cs->FRZsrc = 0;
	cs->balanceERR = 0;
	cs->in = 0;
	cs->out = 0;
	cs->store = 0;
	ns->leafn = 0;
	ns->leafn_storage = 0;
	ns->leafn_transfer = 0;
	ns->frootn = 0;
	ns->frootn_storage = 0;
	ns->frootn_transfer = 0;
	ns->fruitn = 0;
	ns->fruitn_storage = 0;
	ns->fruitn_transfer = 0;
	ns->softstemn = 0;
	ns->softstemn_storage = 0;
	ns->softstemn_transfer = 0;
	ns->livestemn = 0;
	ns->livestemn_storage = 0;
	ns->livestemn_transfer = 0;
	ns->deadstemn = 0;
	ns->deadstemn_storage = 0;
	ns->deadstemn_transfer = 0;
	ns->livecrootn = 0;
	ns->livecrootn_storage = 0;
	ns->livecrootn_transfer = 0;
	ns->deadcrootn = 0;
	ns->deadcrootn_storage = 0;
	ns->deadcrootn_transfer = 0;
	ns->npool = 0;
	ns->litr1n_total = 0;
	ns->litr2n_total = 0;
	ns->litr3n_total = 0;
	ns->litr4n_total = 0;
	ns->cwdn_total = 0;
	ns->STDBn_leaf = 0;
	ns->STDBn_froot = 0;
	ns->STDBn_fruit = 0;
	ns->STDBn_softstem = 0;
	ns->STDBn_transfer = 0;
	ns->STDBn_above = 0;
	ns->STDBn_below = 0;
	ns->CTDBn_leaf = 0;
	ns->CTDBn_froot = 0;
	ns->CTDBn_fruit = 0;
	ns->CTDBn_softstem = 0;
	ns->CTDBn_transfer = 0;
	ns->CTDBn_cstem = 0;
	ns->CTDBn_croot = 0;
	ns->CTDBn_above = 0;
	ns->CTDBn_below = 0;
	ns->soil1n_total = 0;
	ns->soil2n_total = 0;
	ns->soil3n_total = 0;
	ns->soil4n_total = 0;
	ns->retransn = 0;
	ns->sminNH4_total = 0;
	ns->sminNO3_total = 0;
	ns->nfix_src = 0;
	ns->ndep_src = 0;
	ns->nleached_snk = 0;
	ns->nvol_snk = 0;
	ns->fire_snk = 0;
	ns->ndiffused_snk = 0;
	ns->SNSCsnk = 0;
	ns->FRZsrc = 0;
	ns->PLTsrc = 0;
	ns->THN_transportN = 0;
	ns->MOW_transportN = 0;
	ns->HRV_transportN = 0;
	ns->GRZsnk = 0;
	ns->GRZsrc = 0;
	ns->BNDRYsrc = 0;
	ns->SPINUPsrc = 0;
	ns->sum_ndemand = 0;
	ns->balanceERR = 0;
	ns->in = 0;
	ns->out = 0;
	ns->store = 0;


	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		ws->soilw[layer] = 0;
		cs->cwdc[layer] = 0;
		cs->litr1c[layer] = 0;
		cs->litr2c[layer] = 0;
		cs->litr3c[layer] = 0;
		cs->litr4c[layer] = 0;
		cs->litrC[layer] = 0;
		cs->soil1c[layer] = 0;
		cs->soil2c[layer] = 0;
		cs->soil3c[layer] = 0;
		cs->soil4c[layer] = 0;
		cs->soilC[layer] = 0;
		cs->soil1_DOC[layer] = 0;
		cs->soil2_DOC[layer] = 0;
		cs->soil3_DOC[layer] = 0;
		cs->soil4_DOC[layer] = 0;
		cs->soil_DOC[layer] = 0;
		ns->cwdn[layer] = 0;
		ns->litr1n[layer] = 0;
		ns->litr2n[layer] = 0;
		ns->litr3n[layer] = 0;
		ns->litr4n[layer] = 0;
		ns->litrN[layer] = 0;
		ns->soil1n[layer] = 0;
		ns->soil2n[layer] = 0;
		ns->soil3n[layer] = 0;
		ns->soil4n[layer] = 0;
		ns->soilN[layer] = 0;
		ns->soil1_DON[layer] = 0;
		ns->soil2_DON[layer] = 0;
		ns->soil3_DON[layer] = 0;
		ns->soil4_DON[layer] = 0;
		ns->soil_DON[layer] = 0;
		ns->sminNH4[layer] = 0;
		ns->sminNO3[layer] = 0;
	}

	return(!ok);
}
	
