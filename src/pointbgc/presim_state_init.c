/* 
presim_state_init.c
Initialize water, carbon, and nitrogen state variables to 0.0 before
each simulation.

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "pointbgc.h"

int presim_state_init(wstate_struct* ws, cstate_struct* cs, nstate_struct* ns,
cinit_struct* cinit)
{
	int ok=1;
	
	ws->soilw = 0.0;
	ws->snoww = 0.0;
	ws->canopyw = 0.0;
	ws->prcp_src = 0.0;
	ws->outflow_snk = 0.0;
	ws->soilevap_snk = 0.0;
	ws->snowsubl_snk = 0.0;
	ws->canopyevap_snk = 0.0;
	ws->trans_snk = 0.0;
	
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
	ns->soil1n = 0.0;
	ns->soil2n = 0.0;
	ns->soil3n = 0.0;
	ns->soil4n = 0.0;
	ns->sminn = 0.0;
	ns->retransn = 0.0;
	ns->npool = 0.0;
	ns->nfix_src = 0.0;
	ns->ndep_src = 0.0;
	ns->nleached_snk = 0.0;
	ns->nvol_snk = 0.0;
	ns->fire_snk = 0.0;
	
	return(!ok);
}
	
