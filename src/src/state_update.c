/*
state_update.c
Resolve the fluxes in bgc() daily loop to update state variables

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2018, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Modified:
4/17/2000 (PET): Included the new denitrification flux. See daily_allocation.c
for complete description of this change.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "bgc_struct.h"
#include "bgc_func.h"
#include "bgc_constants.h"


int daily_water_state_update(const wflux_struct* wf, wstate_struct* ws)
{
	/* daily update of the water state variables */
	 
	int ok=1;
	int layer;
	
	/* precipitation fluxes */
	ws->canopyw        += wf->prcp_to_canopyw;
	ws->prcp_src       += wf->prcp_to_canopyw;
	
	ws->prcp_src       += wf->prcp_to_soilw;
	
	ws->snoww          += wf->prcp_to_snoww;
	ws->prcp_src       += wf->prcp_to_snoww;
	
	/* canopy intercepted water fluxes */
	ws->canopyevap_snk += wf->canopyw_evap;
	ws->canopyw        -= wf->canopyw_evap;
	ws->canopyw        -= wf->canopyw_to_soilw;

	/* pond water evaporation */
	ws->pondwevap_snk  += wf->pondw_evap;
	ws->pond_water     -= wf->pondw_evap;

	/* snowmelt fluxes */
	ws->snoww          -= wf->snoww_to_soilw;
	ws->snowsubl_snk   += wf->snoww_subl;
	ws->snoww          -= wf->snoww_subl;
	
	/* bare soil evaporation */
	ws->soilevap_snk   += wf->soilw_evap;
	
	/* transpiration */
	ws->trans_snk      += wf->soilw_trans_SUM;
	
	/* runoff - from the top soil layer (net loss) */
	ws->runoff_snk	   += wf->prcp_to_runoff;

	/* deep percolation: percolation of the bottom layer is net loss for the sytem*/
	ws->deeppercolation_snk += wf->soilw_percolated[N_SOILLAYERS-1];
	
	/* groundwater */	
	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		ws->groundwater_src += wf->soilw_from_GW[layer];
	}


	/* irrigation */
	ws->IRGsrc += wf->IRG_to_prcp;
	
	return (!ok);
}

int daily_CN_state_update(const epconst_struct* epc, control_struct* ctrl, epvar_struct* epv, cflux_struct* cf, nflux_struct* nf, cstate_struct* cs, nstate_struct* ns, 
	                      int alloc, int woody, int evergreen)
{
	/* daily update of the carbon state variables */
	
	int ok=1;
	int layer, pp;
	double leafc_to_litr, leafn_to_litr, frootc_to_litr, frootn_to_litr, fruitc_to_litr, fruitn_to_litr, softstemc_to_litr, softstemn_to_litr;

	/* C state variables are updated below in the order of the relevant fluxes in the daily model loop */
	
	/* NOTE: Mortality fluxes are all accounted for in a separate routine, which is to be called after this routine.  
	This is a special case where the updating of state variables is order-sensitive, since otherwise the complications of possibly having 
	mortality fluxes drive the pools negative would create big, unnecessary headaches. */
	

	/* 1. Phenology fluxes */
	if (ok && epc->leaf_cn && CNratio_control(ctrl, epc->leaf_cn, cs->leafc, ns->leafn, cf->leafc_transfer_to_leafc, nf->leafn_transfer_to_leafn, 0)) 
	{
		cs->leafc               += cf->leafc_transfer_to_leafc;
		cs->leafc_transfer      -= cf->leafc_transfer_to_leafc;
		ns->leafn               = cs->leafc / epc->leaf_cn;
		ns->leafn_transfer      = cs->leafc_transfer / epc->leaf_cn;
	}
	else if (epc->leaf_cn)		
	{
		ok = 0;
		printf("ERROR in leaf CN calculation in state_update.c\n");
	}

	if (ok && epc->froot_cn && CNratio_control(ctrl, epc->froot_cn, cs->frootc, ns->frootn, cf->frootc_transfer_to_frootc, nf->frootn_transfer_to_frootn, 0)) 
	{
		cs->frootc               += cf->frootc_transfer_to_frootc;
		cs->frootc_transfer      -= cf->frootc_transfer_to_frootc;
		ns->frootn               = cs->frootc / epc->froot_cn;
		ns->frootn_transfer      = cs->frootc_transfer / epc->froot_cn;
	}
	else if (epc->froot_cn)		
	{
		ok = 0;
		printf("ERROR in froot CN calculation in state_update.c\n");
	}

	if (ok && epc->fruit_cn && CNratio_control(ctrl, epc->fruit_cn, cs->fruitc, ns->fruitn, cf->fruitc_transfer_to_fruitc, nf->fruitn_transfer_to_fruitn, 0)) 
	{
		cs->fruitc               += cf->fruitc_transfer_to_fruitc;
		cs->fruitc_transfer      -= cf->fruitc_transfer_to_fruitc;
		ns->fruitn               = cs->fruitc / epc->fruit_cn;
		ns->fruitn_transfer      = cs->fruitc_transfer / epc->fruit_cn;
	}
	else if (epc->fruit_cn)	
	{	
		ok = 0;
		printf("ERROR in fruit CN calculation in state_update.c\n");
	}

	if (ok && epc->softstem_cn && CNratio_control(ctrl, epc->softstem_cn, cs->softstemc, ns->softstemn, cf->softstemc_transfer_to_softstemc, nf->softstemn_transfer_to_softstemn, 0)) 
	{
		cs->softstemc               += cf->softstemc_transfer_to_softstemc;
		cs->softstemc_transfer      -= cf->softstemc_transfer_to_softstemc;
		ns->softstemn               = cs->softstemc / epc->softstem_cn;
		ns->softstemn_transfer      = cs->softstemc_transfer / epc->softstem_cn;
	}
	else if (epc->softstem_cn)		
	{
		ok = 0;
		printf("ERROR in softstem CN calculation in state_update.c\n");
	}
	
	if (ok && epc->livewood_cn && CNratio_control(ctrl, epc->livewood_cn, cs->livestemc, ns->livestemn, cf->livestemc_transfer_to_livestemc, nf->livestemn_transfer_to_livestemn, 0)) 
	{
		cs->livestemc               += cf->livestemc_transfer_to_livestemc;
		cs->livestemc_transfer      -= cf->livestemc_transfer_to_livestemc;
		ns->livestemn               = cs->livestemc / epc->livewood_cn;
		ns->livestemn_transfer      = cs->livestemc_transfer / epc->livewood_cn;
	}
	else if (epc->livewood_cn)		
	{
		ok = 0;
		printf("ERROR in livestem CN calculation in state_update.c\n");
	}

	if (ok && epc->livewood_cn && CNratio_control(ctrl, epc->livewood_cn, cs->livecrootc, ns->livecrootn, cf->livecrootc_transfer_to_livecrootc, nf->livecrootn_transfer_to_livecrootn, 0)) 
	{
		cs->livecrootc               += cf->livecrootc_transfer_to_livecrootc;
		cs->livecrootc_transfer      -= cf->livecrootc_transfer_to_livecrootc;
		ns->livecrootn               = cs->livecrootc / epc->livewood_cn;
		ns->livecrootn_transfer      = cs->livecrootc_transfer / epc->livewood_cn;
	}
	else if (epc->livewood_cn)		
	{
		ok = 0;
		printf("ERROR in livecroot CN calculation in state_update.c\n");
	}

	if (ok && epc->deadwood_cn && CNratio_control(ctrl, epc->deadwood_cn, cs->deadstemc, ns->deadstemn, cf->deadstemc_transfer_to_deadstemc, nf->deadstemn_transfer_to_deadstemn, 0)) 
	{
		cs->deadstemc               += cf->deadstemc_transfer_to_deadstemc;
		cs->deadstemc_transfer      -= cf->deadstemc_transfer_to_deadstemc;
		ns->deadstemn               = cs->deadstemc / epc->deadwood_cn;
		ns->deadstemn_transfer      = cs->deadstemc_transfer / epc->deadwood_cn;
	}
	else if (epc->deadwood_cn)		
	{
		ok = 0;
		printf("ERROR in deadstem CN calculation in state_update.c\n");
	}

	if (ok && epc->deadwood_cn && CNratio_control(ctrl, epc->deadwood_cn, cs->deadcrootc, ns->deadcrootn, cf->deadcrootc_transfer_to_deadcrootc, nf->deadcrootn_transfer_to_deadcrootn, 0)) 
	{
		cs->deadcrootc               += cf->deadcrootc_transfer_to_deadcrootc;
		cs->deadcrootc_transfer      -= cf->deadcrootc_transfer_to_deadcrootc;
		ns->deadcrootn               = cs->deadcrootc / epc->deadwood_cn;
		ns->deadcrootn_transfer      = cs->deadcrootc_transfer / epc->deadwood_cn;
	}
	else if (epc->deadwood_cn)		
	{
		ok = 0;
		printf("ERROR in deadcroot CN calculation in state_update.c\n");
	}

	
	/* 2. Aboveground pool litterfall and retranslocation to the first soil layer */
	leafc_to_litr = cf->leafc_to_litr1c + cf->leafc_to_litr2c + cf->leafc_to_litr3c + cf->leafc_to_litr4c;
	leafn_to_litr = nf->leafn_to_litr1n + nf->leafn_to_litr2n + nf->leafn_to_litr3n + nf->leafn_to_litr4n;
	if (ok && epc->leaf_cn && CNratio_control(ctrl, epc->leaf_cn, cs->leafc, ns->leafn, leafc_to_litr, leafn_to_litr, epc->leaflitr_cn)) 
	{
		cs->litr1c[0]  += (cf->leafc_to_litr1c);
		cs->litr2c[0]  += (cf->leafc_to_litr2c);
		cs->litr3c[0]  += (cf->leafc_to_litr3c);
		cs->litr4c[0]  += (cf->leafc_to_litr4c);
		cs->leafc      -= (cf->leafc_to_litr1c + cf->leafc_to_litr2c + cf->leafc_to_litr3c + cf->leafc_to_litr4c);

		ns->litr1n[0]  += (nf->leafn_to_litr1n);
		ns->litr2n[0]  += (nf->leafn_to_litr2n);
		ns->litr3n[0]  += (nf->leafn_to_litr3n);
		ns->litr4n[0]  += (nf->leafn_to_litr4n);
		ns->leafn       = cs->leafc / epc->leaf_cn;

		ns->retransn   += nf->leafn_to_retransn;   

		
	}
	else if (epc->leaflitr_cn || epc->leaf_cn)		
	{
		ok = 0;
		printf("ERROR in leaf_to_litr CN calculation in state_update.c\n");
	}
	
	fruitc_to_litr = cf->fruitc_to_litr1c + cf->fruitc_to_litr2c + cf->fruitc_to_litr3c + cf->fruitc_to_litr4c;
	fruitn_to_litr = nf->fruitn_to_litr1n + nf->fruitn_to_litr2n + nf->fruitn_to_litr3n + nf->fruitn_to_litr4n;
	if (ok && epc->fruit_cn && CNratio_control(ctrl, epc->fruit_cn, cs->fruitc, ns->fruitn, fruitc_to_litr, fruitn_to_litr, 0)) 
	{
		cs->litr1c[0]  += (cf->fruitc_to_litr1c);
		cs->litr2c[0]  += (cf->fruitc_to_litr2c);
		cs->litr3c[0]  += (cf->fruitc_to_litr3c);
		cs->litr4c[0]  += (cf->fruitc_to_litr4c);
		cs->fruitc      -= (cf->fruitc_to_litr1c + cf->fruitc_to_litr2c + cf->fruitc_to_litr3c + cf->fruitc_to_litr4c);

		ns->litr1n[0]  += (nf->fruitn_to_litr1n);
		ns->litr2n[0]  += (nf->fruitn_to_litr2n);
		ns->litr3n[0]  += (nf->fruitn_to_litr3n);
		ns->litr4n[0]  += (nf->fruitn_to_litr4n);
		ns->fruitn      = cs->fruitc / epc->fruit_cn;
	}
	else if (epc->fruit_cn)		
	{
		ok = 0;
		printf("ERROR in fruit_to_litr CN calculation in state_update.c\n");
	}

	softstemc_to_litr = cf->softstemc_to_litr1c + cf->softstemc_to_litr2c + cf->softstemc_to_litr3c + cf->softstemc_to_litr4c;
	softstemn_to_litr = nf->softstemn_to_litr1n + nf->softstemn_to_litr2n + nf->softstemn_to_litr3n + nf->softstemn_to_litr4n;
	if (ok && epc->softstem_cn && CNratio_control(ctrl, epc->softstem_cn, cs->softstemc, ns->softstemn, softstemc_to_litr, softstemn_to_litr, 0)) 
	{
		cs->litr1c[0]  += (cf->softstemc_to_litr1c);
		cs->litr2c[0]  += (cf->softstemc_to_litr2c);
		cs->litr3c[0]  += (cf->softstemc_to_litr3c);
		cs->litr4c[0]  += (cf->softstemc_to_litr4c);
		cs->softstemc      -= (cf->softstemc_to_litr1c + cf->softstemc_to_litr2c + cf->softstemc_to_litr3c + cf->softstemc_to_litr4c);

		ns->litr1n[0]  += (nf->softstemn_to_litr1n);
		ns->litr2n[0]  += (nf->softstemn_to_litr2n);
		ns->litr3n[0]  += (nf->softstemn_to_litr3n);
		ns->litr4n[0]  += (nf->softstemn_to_litr4n);
		ns->softstemn   = cs->softstemc / epc->softstem_cn;
	}
	else if (epc->softstem_cn)		
	{
		ok = 0;
		printf("ERROR in softstem_to_litr CN calculation in state_update.c\n");
	}
	
	/* 3. Belowground litterfall is distributed between the different soil layers */
	
	frootc_to_litr = cf->frootc_to_litr1c + cf->frootc_to_litr2c + cf->frootc_to_litr3c + cf->frootc_to_litr4c;
	frootn_to_litr = nf->frootn_to_litr1n + nf->frootn_to_litr2n + nf->frootn_to_litr3n + nf->frootn_to_litr4n;
	if (ok && epc->froot_cn && CNratio_control(ctrl, epc->froot_cn, cs->frootc, ns->frootn, frootc_to_litr, frootn_to_litr, 0)) 
	{
		for (layer = 0; layer < N_SOILLAYERS; layer++)
		{
			cs->litr1c[layer] += cf->frootc_to_litr1c * epv->rootlength_prop[layer];
			cs->litr2c[layer] += cf->frootc_to_litr2c * epv->rootlength_prop[layer];
			cs->litr3c[layer] += cf->frootc_to_litr3c * epv->rootlength_prop[layer];
			cs->litr4c[layer] += cf->frootc_to_litr4c * epv->rootlength_prop[layer];

			ns->litr1n[layer] += nf->frootn_to_litr1n * epv->rootlength_prop[layer];
			ns->litr2n[layer] += nf->frootn_to_litr2n * epv->rootlength_prop[layer];
			ns->litr3n[layer] += nf->frootn_to_litr3n * epv->rootlength_prop[layer];
			ns->litr4n[layer] += nf->frootn_to_litr4n * epv->rootlength_prop[layer];
		}
		cs->frootc       -= frootc_to_litr;
		ns->frootn	      = cs->frootc / epc->froot_cn;
	}
	else if (epc->froot_cn)		
	{
		ok = 0;
		printf("ERROR in froot_to_litr CN calculation in state_update.c\n");
	}

	 /* 4. Livewood turnover fluxes */
	cs->deadstemc  += cf->livestemc_to_deadstemc;
	cs->livestemc  -= cf->livestemc_to_deadstemc;
	cs->deadcrootc += cf->livecrootc_to_deadcrootc;
	cs->livecrootc -= cf->livecrootc_to_deadcrootc;
	ns->retransn      += nf->livestemn_to_retransn;  
	ns->livestemn     -= nf->livestemn_to_retransn;
	ns->retransn      += nf->livecrootn_to_retransn; 
	ns->livecrootn    -= nf->livecrootn_to_retransn;
	

	ns->deadstemn  += nf->livestemn_to_deadstemn;
	ns->livestemn  -= nf->livestemn_to_deadstemn;
	ns->deadcrootn += nf->livecrootn_to_deadcrootn;
	ns->livecrootn -= nf->livecrootn_to_deadcrootn;
	

	/* 6. Photosynthesis fluxes */
	cs->psnsun_src   += cf->psnsun_to_cpool;
	cs->psnshade_src += cf->psnshade_to_cpool;
	cs->cpool        += (cf->psnsun_to_cpool + cf->psnshade_to_cpool);

	/* 7. Plant allocation flux, from N retrans pool */
	ns->npool		    += nf->retransn_to_npool;
	ns->retransn        -= nf->retransn_to_npool;

	
	/* 8. Litter decomposition fluxes - MULTILAYER SOIL */
	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		/* Fluxes out of coarse woody debris into litter pools */
		cs->litr2c[layer]       += cf->cwdc_to_litr2c[layer];
		cs->litr3c[layer]       += cf->cwdc_to_litr3c[layer];
		cs->litr4c[layer]       += cf->cwdc_to_litr4c[layer];
		cs->cwdc[layer]         -= (cf->cwdc_to_litr2c[layer] + cf->cwdc_to_litr3c[layer] + cf->cwdc_to_litr4c[layer]);
		/* Fluxes out of labile litter pool */
		cs->litr1_hr_snk		+= cf->litr1_hr[layer];
		cs->litr1c[layer]       -= cf->litr1_hr[layer];
		cs->soil1c[layer]       += cf->litr1c_to_soil1c[layer];
		cs->litr1c[layer]       -= cf->litr1c_to_soil1c[layer];
		/* Fluxes out of cellulose litter pool */
		cs->litr2_hr_snk		+= cf->litr2_hr[layer];
		cs->litr2c[layer]       -= cf->litr2_hr[layer];
		cs->soil2c[layer]       += cf->litr2c_to_soil2c[layer];
		cs->litr2c[layer]       -= cf->litr2c_to_soil2c[layer];
		/* Fluxes from shielded to unshielded cellulose pools */
		cs->litr2c[layer]       += cf->litr3c_to_litr2c[layer];
		cs->litr3c[layer]       -= cf->litr3c_to_litr2c[layer];
		/* Fluxes out of lignin litter pool */
		cs->litr4_hr_snk		+= cf->litr4_hr[layer];
		cs->litr4c[layer]       -= cf->litr4_hr[layer];
		cs->soil3c[layer]       += cf->litr4c_to_soil3c[layer];
		cs->litr4c[layer]       -= cf->litr4c_to_soil3c[layer];
		/* Fluxes out of fast soil pool */
		cs->soil1_hr_snk		+= cf->soil1_hr[layer];
		cs->soil1c[layer]       -= cf->soil1_hr[layer];
		cs->soil2c[layer]       += cf->soil1c_to_soil2c[layer];
		cs->soil1c[layer]       -= cf->soil1c_to_soil2c[layer];
		/* Fluxes out of medium soil pool */
		cs->soil2_hr_snk		+= cf->soil2_hr[layer];
		cs->soil2c[layer]       -= cf->soil2_hr[layer];
		cs->soil3c[layer]       += cf->soil2c_to_soil3c[layer];
		cs->soil2c[layer]       -= cf->soil2c_to_soil3c[layer];
		/* Fluxes out of slow soil pool */
		cs->soil3_hr_snk		+= cf->soil3_hr[layer];
		cs->soil3c[layer]       -= cf->soil3_hr[layer];
		cs->soil4c[layer]       += cf->soil3c_to_soil4c[layer];
		cs->soil3c[layer]       -= cf->soil3c_to_soil4c[layer];
		/* Fluxes out of recalcitrant SOM pool */
		cs->soil4_hr_snk		+= cf->soil4_hr[layer];
		cs->soil4c[layer]       -= cf->soil4_hr[layer];

	
		/* Fluxes out of coarse woody debris into litter pools */
		ns->litr2n[layer]       += nf->cwdn_to_litr2n[layer];
		ns->litr3n[layer]       += nf->cwdn_to_litr3n[layer];
		ns->litr4n[layer]       += nf->cwdn_to_litr4n[layer];
		ns->cwdn[layer]         -= (nf->cwdn_to_litr2n[layer] + nf->cwdn_to_litr3n[layer] + nf->cwdn_to_litr4n[layer]);
		/* Fluxes out of labile litter pool */
		ns->soil1n[layer]       += nf->litr1n_to_soil1n[layer];
		ns->litr1n[layer]       -= nf->litr1n_to_soil1n[layer];
		/* Fluxes out of cellulose litter pool */
		ns->soil2n[layer]       += nf->litr2n_to_soil2n[layer];
		ns->litr2n[layer]       -= nf->litr2n_to_soil2n[layer];
		/* Fluxes from shielded to unshielded cellulose pools */
		ns->litr2n[layer]       += nf->litr3n_to_litr2n[layer];
		ns->litr3n[layer]       -= nf->litr3n_to_litr2n[layer];
		/* Fluxes out of lignin litter pool */
		ns->soil3n[layer]       += nf->litr4n_to_soil3n[layer];
		ns->litr4n[layer]       -= nf->litr4n_to_soil3n[layer];
		/* Fluxes out of fast soil pool */
		ns->soil2n[layer]       += nf->soil1n_to_soil2n[layer];
		ns->soil1n[layer]       -= nf->soil1n_to_soil2n[layer];
		/* Fluxes out of medium soil pool */
		ns->soil3n[layer]       += nf->soil2n_to_soil3n[layer];
		ns->soil2n[layer]       -= nf->soil2n_to_soil3n[layer];
		/* Fluxes out of slow soil pool */
		ns->soil4n[layer]       += nf->soil3n_to_soil4n[layer];
		ns->soil3n[layer]       -= nf->soil3n_to_soil4n[layer];
	}


	/* 9. Daily allocation fluxes */
	/* daily leaf allocation fluxes */
	
	if (ok && epc->leaf_cn && CNratio_control(ctrl, epc->leaf_cn, cs->leafc, ns->leafn, cf->cpool_to_leafc, nf->npool_to_leafn, 0)) 
	{
		cs->cpool          -= cf->cpool_to_leafc;
		cs->leafc          += cf->cpool_to_leafc;
		
		ns->npool          -= nf->npool_to_leafn;
		ns->leafn           = cs->leafc / epc->leaf_cn;
	}
	else if (epc->leaf_cn)		
	{
		ok = 0;
		printf("ERROR in cpool_to_leafc CN calculation in state_update.c\n");
	}

	if (ok && epc->leaf_cn && CNratio_control(ctrl, epc->leaf_cn, cs->leafc_storage, ns->leafn_storage, cf->cpool_to_leafc_storage, nf->npool_to_leafn_storage, 0)) 
	{
		cs->cpool          -= cf->cpool_to_leafc_storage;
		cs->leafc_storage  += cf->cpool_to_leafc_storage;
		
		ns->npool          -= nf->npool_to_leafn_storage;
		ns->leafn_storage   = cs->leafc_storage / epc->leaf_cn;
	}
	else if (epc->leaf_cn)		
	{
		ok = 0;
		printf("ERROR in cpool_to_leafc_storage CN calculation in state_update.c\n");
	}

	/* Daily fine root allocation fluxes */
	if (ok && epc->froot_cn && CNratio_control(ctrl, epc->froot_cn, cs->frootc, ns->frootn, cf->cpool_to_frootc, nf->npool_to_frootn, 0)) 
	{
		cs->cpool          -= cf->cpool_to_frootc;
		cs->frootc         += cf->cpool_to_frootc;
		
		ns->npool          -= nf->npool_to_frootn;
		ns->frootn          = cs->frootc / epc->froot_cn;
	}
	else if (epc->froot_cn)		
	{
		ok = 0;
		printf("ERROR in cpool_to_frootc CN calculation in state_update.c\n");
	}

	if (ok && epc->froot_cn && CNratio_control(ctrl, epc->froot_cn, cs->frootc_storage, ns->frootn_storage, cf->cpool_to_frootc_storage, nf->npool_to_frootn_storage, 0)) 
	{
		cs->cpool          -= cf->cpool_to_frootc_storage;
		cs->frootc_storage += cf->cpool_to_frootc_storage;
		
		ns->npool          -= nf->npool_to_frootn_storage;
		ns->frootn_storage  = cs->frootc_storage / epc->froot_cn;
	}
	else if (epc->froot_cn)		
	{
		ok = 0;
		printf("ERROR in cpool_to_frootc_storage CN calculation in state_update.c\n");
	}

	/* Daily fruit allocation fluxes + EXTRA: effect of heat stress during flowering in grain filling phenophase */
	if (ok && epc->fruit_cn && CNratio_control(ctrl, epc->fruit_cn, cs->fruitc, ns->fruitn, cf->cpool_to_fruitc, nf->npool_to_fruitn, 0)) 
	{
		cs->cpool          -= cf->cpool_to_fruitc;
		cs->fruitc         += cf->cpool_to_fruitc;
		
		ns->npool          -= nf->npool_to_fruitn;
		ns->fruitn          = cs->fruitc / epc->fruit_cn;

		cs->fruitc         -= cf->fruitc_to_flowHS;
		cs->STDBc_fruit    += cf->fruitc_to_flowHS;

		ns->fruitn         -= nf->fruitn_to_flowHS;
		ns->STDBn_fruit    += nf->fruitn_to_flowHS;

		/* control */
		if ((cf->fruitc_to_flowHS > 0 && nf->fruitn_to_flowHS > 0 && epv->n_actphen != epc->n_flowHS_phenophase + 1) || 
			(cs->fruitc < 0 && fabs(cs->fruitc) > CRIT_PREC)  || (ns->fruitn < 0 && fabs(ns->fruitn) > CRIT_PREC)  )
		{
			ok = 0;
			printf("ERROR in flowering heat stress calculation in state_update.c\n");
		}
	}
	else if (epc->fruit_cn)		
	{
		ok = 0;
		printf("ERROR in cpool_to_fruitc CN calculation in state_update.c\n");
	}

	if (ok && epc->fruit_cn && CNratio_control(ctrl, epc->fruit_cn, cs->fruitc_storage, ns->fruitn_storage, cf->cpool_to_fruitc_storage, nf->npool_to_fruitn_storage, 0)) 
	{
		cs->cpool          -= cf->cpool_to_fruitc_storage;
		cs->fruitc_storage += cf->cpool_to_fruitc_storage;
		
		ns->npool          -= nf->npool_to_fruitn_storage;
		ns->fruitn_storage  = cs->fruitc_storage / epc->fruit_cn;
	}
	else if (epc->fruit_cn)		
	{
		ok = 0;
		printf("ERROR in cpool_to_fruitc_storage CN calculation in state_update.c\n");
	}
	
	/* Daily sofstem allocation fluxes */
	if (ok && epc->softstem_cn && CNratio_control(ctrl, epc->softstem_cn, cs->softstemc, ns->softstemn, cf->cpool_to_softstemc, nf->npool_to_softstemn, 0)) 
	{
		cs->cpool          -= cf->cpool_to_softstemc;
		cs->softstemc       += cf->cpool_to_softstemc;
		
		ns->npool          -= nf->npool_to_softstemn;
		ns->softstemn       = cs->softstemc / epc->softstem_cn;
	}
	else if (epc->softstem_cn)		
	{
		ok = 0;
		printf("ERROR in cpool_to_softstemc CN calculation in state_update.c\n");
	}

	if (ok && epc->softstem_cn && CNratio_control(ctrl, epc->softstem_cn, cs->softstemc_storage, ns->softstemn_storage, cf->cpool_to_softstemc_storage, nf->npool_to_softstemn_storage, 0)) 
	{
		cs->cpool              -= cf->cpool_to_softstemc_storage;
		cs->softstemc_storage  += cf->cpool_to_softstemc_storage;
		
		ns->npool              -= nf->npool_to_softstemn_storage;
		ns->softstemn_storage   = cs->softstemc_storage / epc->softstem_cn;
	}
	else if (epc->softstem_cn)		
	{
		ok = 0;
		printf("ERROR in cpool_to_softstemc_storage CN calculation in state_update.c\n");
	}			

	/* Daily live stem wood allocation fluxes */
	if (ok && epc->livewood_cn && CNratio_control(ctrl, epc->livewood_cn, cs->livestemc, ns->livestemn, cf->cpool_to_livestemc, nf->npool_to_livestemn, 0)) 
	{
		cs->cpool      -= cf->cpool_to_livestemc;
		cs->livestemc  += cf->cpool_to_livestemc;
		
		ns->npool      -= nf->npool_to_livestemn;
		ns->livestemn   = cs->livestemc / epc->livewood_cn;
	}
	else if (epc->livewood_cn)		
	{
		ok = 0;
		printf("ERROR in cpool_to_livestemc CN calculation in state_update.c\n");
	}

	if (ok && epc->livewood_cn && CNratio_control(ctrl, epc->livewood_cn, cs->livestemc_storage, ns->livestemn_storage, cf->cpool_to_livestemc_storage, nf->npool_to_livestemn_storage, 0)) 
	{
		cs->cpool              -= cf->cpool_to_livestemc_storage;
		cs->livestemc_storage  += cf->cpool_to_livestemc_storage;
		
		ns->npool              -= nf->npool_to_livestemn_storage;
		ns->livestemn_storage   = cs->livestemc_storage / epc->livewood_cn;
	}
	else if (epc->livewood_cn)		
	{
		ok = 0;
		printf("ERROR in cpool_to_livestemc_storage CN calculation in state_update.c\n");
	}
	
	/* Daily dead stem wood allocation fluxes */
	if (ok && epc->deadwood_cn && CNratio_control(ctrl, epc->deadwood_cn, cs->deadstemc, ns->deadstemn, cf->cpool_to_deadstemc, nf->npool_to_deadstemn, 0)) 
	{
		cs->cpool          -= cf->cpool_to_deadstemc;
		cs->deadstemc          += cf->cpool_to_deadstemc;
		
		ns->npool          -= nf->npool_to_deadstemn;
		ns->deadstemn           = cs->deadstemc / epc->deadwood_cn;
	}
	else if (epc->deadwood_cn)		
	{
		ok = 0;
		printf("ERROR in cpool_to_deadstemc CN calculation in state_update.c\n");
	}

	if (ok && epc->deadwood_cn && CNratio_control(ctrl, epc->deadwood_cn, cs->deadstemc_storage, ns->deadstemn_storage, cf->cpool_to_deadstemc_storage, nf->npool_to_deadstemn_storage, 0)) 
	{
		cs->cpool          -= cf->cpool_to_deadstemc_storage;
		cs->deadstemc_storage  += cf->cpool_to_deadstemc_storage;
		
		ns->npool          -= nf->npool_to_deadstemn_storage;
		ns->deadstemn_storage   = cs->deadstemc_storage / epc->deadwood_cn;
	}
	else if (epc->deadwood_cn)		
	{
		ok = 0;
		printf("ERROR in cpool_to_deadstemc_storage CN calculation in state_update.c\n");
	}
	
	/* Daily live coarse root wood allocation fluxes */
	if (ok && epc->livewood_cn && CNratio_control(ctrl, epc->livewood_cn, cs->livecrootc, ns->livecrootn, cf->cpool_to_livecrootc, nf->npool_to_livecrootn, 0)) 
	{
		cs->cpool          -= cf->cpool_to_livecrootc;
		cs->livecrootc          += cf->cpool_to_livecrootc;
		
		ns->npool          -= nf->npool_to_livecrootn;
		ns->livecrootn           = cs->livecrootc / epc->livewood_cn;
	}
	else if (epc->livewood_cn)		
	{
		ok = 0;
		printf("ERROR in cpool_to_livecrootc CN calculation in state_update.c\n");
	}

	if (ok && epc->livewood_cn && CNratio_control(ctrl, epc->livewood_cn, cs->livecrootc_storage, ns->livecrootn_storage, cf->cpool_to_livecrootc_storage, nf->npool_to_livecrootn_storage, 0)) 
	{
		cs->cpool          -= cf->cpool_to_livecrootc_storage;
		cs->livecrootc_storage  += cf->cpool_to_livecrootc_storage;
		
		ns->npool          -= nf->npool_to_livecrootn_storage;
		ns->livecrootn_storage   = cs->livecrootc_storage / epc->livewood_cn;
	}
	else if (epc->livewood_cn)		
	{
		ok = 0;
		printf("ERROR in cpool_to_livecrootc_storage CN calculation in state_update.c\n");
	}

	/* Daily dead coarse root wood allocation fluxes */
	if (ok && epc->deadwood_cn && CNratio_control(ctrl, epc->deadwood_cn, cs->deadcrootc, ns->deadcrootn, cf->cpool_to_deadcrootc, nf->npool_to_deadcrootn, 0)) 
	{
		cs->cpool          -= cf->cpool_to_deadcrootc;
		cs->deadcrootc          += cf->cpool_to_deadcrootc;
		
		ns->npool          -= nf->npool_to_deadcrootn;
		ns->deadcrootn           = cs->deadcrootc / epc->deadwood_cn;
	}
	else if (epc->deadwood_cn)		
	{
		ok = 0;
		printf("ERROR in cpool_to_deadcrootc CN calculation in state_update.c\n");
	}

	if (ok && epc->deadwood_cn && CNratio_control(ctrl, epc->deadwood_cn, cs->deadcrootc_storage, ns->deadcrootn_storage, cf->cpool_to_deadcrootc_storage, nf->npool_to_deadcrootn_storage, 0)) 
	{
		cs->cpool          -= cf->cpool_to_deadcrootc_storage;
		cs->deadcrootc_storage  += cf->cpool_to_deadcrootc_storage;
		
		ns->npool          -= nf->npool_to_deadcrootn_storage;
		ns->deadcrootn_storage   = cs->deadcrootc_storage / epc->deadwood_cn;
	}
	else if (epc->deadwood_cn)		
	{
		ok = 0;
		printf("ERROR in cpool_to_deadcrootc_storage CN calculation in state_update.c\n");
	}
	

	/* Daily allocation for transfer growth respiration */
	cs->gresp_storage  += cf->cpool_to_gresp_storage;
	cs->cpool          -= cf->cpool_to_gresp_storage;
	 
	/* Calculate sum of leafC in a given phenophase */
	pp = (int) epv->n_actphen - 1;
	cs->leafcSUM_phenphase[pp] += cf->cpool_to_leafc + cf->leafc_transfer_to_leafc;

	
	/* 10. Daily growth respiration fluxes */
	/* Leaf growth respiration */
	cs->leaf_gr_snk     += cf->cpool_leaf_gr;
	cs->cpool           -= cf->cpool_leaf_gr;
	cs->leaf_gr_snk     += cf->cpool_leaf_storage_gr;
	cs->cpool           -= cf->cpool_leaf_storage_gr;
	cs->leaf_gr_snk     += cf->transfer_leaf_gr;
	cs->gresp_transfer  -= cf->transfer_leaf_gr;
	/* Fine root growth respiration */
	cs->froot_gr_snk    += cf->cpool_froot_gr;
	cs->cpool           -= cf->cpool_froot_gr;
	cs->froot_gr_snk    += cf->cpool_froot_storage_gr;
	cs->cpool           -= cf->cpool_froot_storage_gr;
	cs->froot_gr_snk    += cf->transfer_froot_gr;
	cs->gresp_transfer  -= cf->transfer_froot_gr;
	/* Fruit growth respiration */
	cs->fruit_gr_snk     += cf->cpool_fruit_gr;
	cs->cpool            -= cf->cpool_fruit_gr;
	cs->fruit_gr_snk     += cf->cpool_fruit_storage_gr;
	cs->cpool            -= cf->cpool_fruit_storage_gr;
	cs->fruit_gr_snk     += cf->transfer_fruit_gr;
	cs->gresp_transfer   -= cf->transfer_fruit_gr;
	/* Fruit growth respiration. */
	cs->softstem_gr_snk  += cf->cpool_softstem_gr;
	cs->cpool            -= cf->cpool_softstem_gr;
	cs->softstem_gr_snk  += cf->cpool_softstem_storage_gr;
	cs->cpool            -= cf->cpool_softstem_storage_gr;
	cs->softstem_gr_snk  += cf->transfer_softstem_gr;
	cs->gresp_transfer   -= cf->transfer_softstem_gr;
	/* Live stem growth respiration */ 
	cs->livestem_gr_snk  += cf->cpool_livestem_gr;
	cs->cpool            -= cf->cpool_livestem_gr;
	cs->livestem_gr_snk  += cf->cpool_livestem_storage_gr;
	cs->cpool            -= cf->cpool_livestem_storage_gr;
	cs->livestem_gr_snk  += cf->transfer_livestem_gr;
	cs->gresp_transfer   -= cf->transfer_livestem_gr;
	/* Dead stem growth respiration */ 
	cs->deadstem_gr_snk  += cf->cpool_deadstem_gr;
	cs->cpool            -= cf->cpool_deadstem_gr;
	cs->deadstem_gr_snk  += cf->cpool_deadstem_storage_gr;
	cs->cpool            -= cf->cpool_deadstem_storage_gr;
	cs->deadstem_gr_snk  += cf->transfer_deadstem_gr;
	cs->gresp_transfer   -= cf->transfer_deadstem_gr;
	/* Live coarse root growth respiration */ 
	cs->livecroot_gr_snk += cf->cpool_livecroot_gr;
	cs->cpool            -= cf->cpool_livecroot_gr;
	cs->livecroot_gr_snk += cf->cpool_livecroot_storage_gr;
	cs->cpool            -= cf->cpool_livecroot_storage_gr;
	cs->livecroot_gr_snk += cf->transfer_livecroot_gr;
	cs->gresp_transfer   -= cf->transfer_livecroot_gr;
	/* Dead coarse root growth respiration */ 
	cs->deadcroot_gr_snk += cf->cpool_deadcroot_gr;
	cs->cpool            -= cf->cpool_deadcroot_gr;
	cs->deadcroot_gr_snk += cf->cpool_deadcroot_storage_gr;
	cs->cpool            -= cf->cpool_deadcroot_storage_gr;
	cs->deadcroot_gr_snk += cf->transfer_deadcroot_gr;
	cs->gresp_transfer   -= cf->transfer_deadcroot_gr;

	

	/* 11. Maintanance respiration fluxes
	       covering of maintananance respiration fluxes from NSC (non-structural carbohydrate), namely storage and transfer pools*/
	if (ok && nsc_maintresp(epc, epv, cf, nf, cs, ns))
	{
		printf("Error in nsc_maintresp() from bgc()\n");
		ok=0;
	}
	
	
	
	
	/* 12. Annual allocation fluxes, one day per year */
	if (alloc)
	{
		/* Move storage material into transfer compartments on the annual
		allocation day. This is a special case, where a flux is assessed in
		the state_variable update routine.  This is required to have the
		allocation of excess C and N show up as new growth in the next growing
		season, instead of two growing seasons from now. */
		cf->leafc_storage_to_leafc_transfer				= cs->leafc_storage;
		cf->frootc_storage_to_frootc_transfer			= cs->frootc_storage;
		cf->fruitc_storage_to_fruitc_transfer			= cs->fruitc_storage;
		cf->softstemc_storage_to_softstemc_transfer		= cs->softstemc_storage;
		cf->gresp_storage_to_gresp_transfer				= cs->gresp_storage;
		cf->livestemc_storage_to_livestemc_transfer		= cs->livestemc_storage;
		cf->deadstemc_storage_to_deadstemc_transfer		= cs->deadstemc_storage;
		cf->livecrootc_storage_to_livecrootc_transfer	= cs->livecrootc_storage;
		cf->deadcrootc_storage_to_deadcrootc_transfer	= cs->deadcrootc_storage;

		nf->leafn_storage_to_leafn_transfer				= ns->leafn_storage;
		nf->frootn_storage_to_frootn_transfer			= ns->frootn_storage;
		nf->fruitn_storage_to_fruitn_transfer			= ns->fruitn_storage;
		nf->softstemn_storage_to_softstemn_transfer		= ns->softstemn_storage;
		nf->livestemn_storage_to_livestemn_transfer		= ns->livestemn_storage;
		nf->deadstemn_storage_to_deadstemn_transfer		= ns->deadstemn_storage;
		nf->livecrootn_storage_to_livecrootn_transfer	= ns->livecrootn_storage;
		nf->deadcrootn_storage_to_deadcrootn_transfer	= ns->deadcrootn_storage;
		

		/* update states variables */
		cs->leafc_transfer     += cf->leafc_storage_to_leafc_transfer;
		cs->leafc_storage      -= cf->leafc_storage_to_leafc_transfer;
		cs->frootc_transfer    += cf->frootc_storage_to_frootc_transfer;
		cs->frootc_storage     -= cf->frootc_storage_to_frootc_transfer;
		cs->fruitc_transfer    += cf->fruitc_storage_to_fruitc_transfer;
		cs->fruitc_storage     -= cf->fruitc_storage_to_fruitc_transfer;
		cs->softstemc_transfer += cf->softstemc_storage_to_softstemc_transfer;
		cs->softstemc_storage  -= cf->softstemc_storage_to_softstemc_transfer;
		cs->gresp_transfer     += cf->gresp_storage_to_gresp_transfer;
		cs->gresp_storage      -= cf->gresp_storage_to_gresp_transfer;

		ns->leafn_transfer     += nf->leafn_storage_to_leafn_transfer;
		ns->leafn_storage      -= nf->leafn_storage_to_leafn_transfer;
		ns->frootn_transfer    += nf->frootn_storage_to_frootn_transfer;
		ns->frootn_storage     -= nf->frootn_storage_to_frootn_transfer;
		ns->fruitn_transfer    += nf->fruitn_storage_to_fruitn_transfer;
		ns->fruitn_storage     -= nf->fruitn_storage_to_fruitn_transfer;
		ns->softstemn_transfer += nf->softstemn_storage_to_softstemn_transfer;
		ns->softstemn_storage  -= nf->softstemn_storage_to_softstemn_transfer;


		cs->livestemc_transfer  += cf->livestemc_storage_to_livestemc_transfer;
		cs->livestemc_storage   -= cf->livestemc_storage_to_livestemc_transfer;
		cs->deadstemc_transfer  += cf->deadstemc_storage_to_deadstemc_transfer;
		cs->deadstemc_storage   -= cf->deadstemc_storage_to_deadstemc_transfer;
		cs->livecrootc_transfer += cf->livecrootc_storage_to_livecrootc_transfer;
		cs->livecrootc_storage  -= cf->livecrootc_storage_to_livecrootc_transfer;
		cs->deadcrootc_transfer += cf->deadcrootc_storage_to_deadcrootc_transfer;
		cs->deadcrootc_storage  -= cf->deadcrootc_storage_to_deadcrootc_transfer;

		ns->livestemn_transfer  += nf->livestemn_storage_to_livestemn_transfer;
		ns->livestemn_storage   -= nf->livestemn_storage_to_livestemn_transfer;
		ns->deadstemn_transfer  += nf->deadstemn_storage_to_deadstemn_transfer;
		ns->deadstemn_storage   -= nf->deadstemn_storage_to_deadstemn_transfer;
		ns->livecrootn_transfer += nf->livecrootn_storage_to_livecrootn_transfer;
		ns->livecrootn_storage  -= nf->livecrootn_storage_to_livecrootn_transfer;
		ns->deadcrootn_transfer += nf->deadcrootn_storage_to_deadcrootn_transfer;
		ns->deadcrootn_storage  -= nf->deadcrootn_storage_to_deadcrootn_transfer;
		


		/* for deciduous system, force leafc and frootc to exactly 0.0 on the last day */
		if (!evergreen)
		{
			if (-cs->leafc > CRIT_PREC || -cs->frootc > CRIT_PREC || -cs->fruitc > CRIT_PREC || -cs->softstemc > CRIT_PREC)
			{
				printf("\n");
				printf("ERROR: negative plant carbon pool in state_update.c\n");
				ok=0;
			}
			if (cs->leafc     < 1e-10) cs->leafc = 0.0;
			if (cs->frootc    < 1e-10) cs->frootc = 0.0;
			if (cs->fruitc    < 1e-10) cs->fruitc = 0.0;
			if (cs->softstemc < 1e-10) cs->softstemc = 0.0;

		
			if (-ns->leafn > CRIT_PREC || -ns->frootn > CRIT_PREC || -ns->fruitn > CRIT_PREC || -ns->softstemn > CRIT_PREC)
			{
				printf("\n");
				printf("ERROR: negative plant nitrogen pool in state_update.c\n");
				ok=0;
			}
			if (ns->leafn     < 1e-10) ns->leafn = 0.0;
			if (ns->frootn    < 1e-10) ns->frootn = 0.0;
			if (ns->fruitn    < 1e-10) ns->fruitn = 0.0;
			if (ns->softstemn < 1e-10) ns->softstemn = 0.0;
		
		}
	} /* end if allocation day */

	return (!ok);
}			

int CNratio_control(control_struct* ctrl, double CNratio, double cpool, double npool, double cflux, double nflux, double CNratio_flux)
{
	int error = 0;
	double CNdiff = 0;

	if (CNratio_flux == 0) CNratio_flux = CNratio;

	/* control for leaf C:N ratio of pools */
	if ((npool ==0 && cpool > CRIT_PREC ) || (npool > CRIT_PREC  && cpool == 0))
	{
		printf("ERROR: CNratio_control in daily_CN_state_update.c\n");
		error = 1;
	}
	
	if(npool > 0 && cpool > 0) CNdiff = cpool/npool - CNratio;
	

	if(fabs(CNdiff) > 0)
	{
		npool = cpool / CNratio;
		if (fabs(CNdiff) > ctrl->CNerror) ctrl->CNerror = fabs(CNdiff);
	
	
	}
	CNdiff=0;

	/* control for leaf C:N ratio of fluxes */
	if ((nflux == 0 && cflux > CRIT_PREC ) || (nflux > CRIT_PREC  && cflux == 0))
	{
		printf("ERROR: CNratio_control in daily_CN_state_update.c\n");
		error = 1;
	}

	if(nflux > 0 && cflux > 0) CNdiff = cflux/nflux - CNratio_flux;


	if(fabs(CNdiff) > 0)
	{
		nflux = cflux / CNratio_flux;
		if (fabs(CNdiff) > ctrl->CNerror) ctrl->CNerror = fabs(CNdiff);
		
		CNdiff = cflux/nflux - CNratio;

	}
	
	return (!error);
}

int nsc_maintresp(const epconst_struct* epc, epvar_struct* epv, cflux_struct* cf, nflux_struct* nf, cstate_struct* cs, nstate_struct* ns)
{	
	/* Covering of maintananance respiration fluxes from storage pools */
	
	int ok=1;
	double mresp_nw, mresp_w, nsc_nw, nsc_w, sc_nw, sc_w, nsc_crit, diff_total, diff_total_nw, diff_total_w, diff, day_mr_ratio, excess;
	
	diff_total_nw = diff_total_w = day_mr_ratio = excess = 0;
	
	
	/* summarizing maint.resp fluxes and available non-structural carbohydrate fluxes - non-woody and woody */
	mresp_nw = cf->leaf_day_mr + cf->leaf_night_mr + cf->froot_mr + cf->fruit_mr + cf->softstem_mr;
	mresp_w  = cf->livestem_mr + cf->livecroot_mr;
	
		
	nsc_nw  = (cs->leafc_storage      +  cs->frootc_storage     + cs->fruitc_storage     + cs->softstemc_storage + 
			   cs->leafc_transfer     +  cs->frootc_transfer    + cs->fruitc_transfer    + cs->softstemc_transfer);

	nsc_w  = (cs->livestemc_storage  + cs->livecrootc_storage  + cs->deadstemc_storage  + cs->deadcrootc_storage +
		      cs->livestemc_transfer + cs->livecrootc_transfer + cs->deadstemc_transfer + cs->deadcrootc_transfer);

	sc_nw = cs->leafc     +  cs->frootc    + cs->fruitc     + cs->softstemc;

	sc_w = cs->livestemc  + cs->livecrootc  + cs->deadstemc  + cs->deadcrootc;

	if (fabs(nsc_nw) < CRIT_PREC) nsc_nw = 0;
	if (fabs(nsc_w)  < CRIT_PREC) nsc_w = 0;
	if (fabs(sc_nw)  < CRIT_PREC) sc_nw = 0;
	if (fabs(sc_w)   < CRIT_PREC) sc_w = 0;

	
	/* calculation of difference between between the demand (mresp) and the source (cpool) - non-woody and woody */
	diff_total  = mresp_nw + mresp_w - cs->cpool;

	diff_total_nw = diff_total * (mresp_nw / (mresp_nw + mresp_w));

	diff_total_w  = diff_total * (mresp_w  / (mresp_nw + mresp_w));
	


	/* 1: non-woody biomass */
	if (mresp_nw)
	{

		/* 1.1. calculation the difference between NSC and diff (based on available amount) */
		if (diff_total_nw > CRIT_PREC)
		{
			/* critical NSC value: fixed ratio of theroretical maximum os NSC value */
			nsc_crit = epc->NSC_avail_prop * (sc_nw * epc->NSC_SC_prop);
		
			if (nsc_nw < nsc_crit)
			{
				diff = 0;
				epv->NSC_limit_nw = 2;
			}
			else
			{
				if ((nsc_nw - diff_total_nw) > nsc_crit)
				{
					diff = diff_total_nw;
					epv->NSC_limit_nw = 0;
				}

				else
				{
					diff = nsc_nw - nsc_crit;
					epv->NSC_limit_nw = 1;
				}
			}

			/* 1.2. calculation of flxues from nsc pools */
			if (nsc_nw && diff)
			{
				cf->leafc_storage_to_maintresp		 = diff * cs->leafc_storage/nsc_nw;
				cf->frootc_storage_to_maintresp		 = diff * cs->frootc_storage/nsc_nw;
				cf->fruitc_storage_to_maintresp		 = diff * cs->fruitc_storage/nsc_nw;
				cf->softstemc_storage_to_maintresp	 = diff * cs->softstemc_storage/nsc_nw;

				cf->leafc_transfer_to_maintresp		 = diff * cs->leafc_transfer/nsc_nw;
				cf->frootc_transfer_to_maintresp	 = diff * cs->frootc_transfer/nsc_nw;
				cf->fruitc_transfer_to_maintresp	 = diff * cs->fruitc_transfer/nsc_nw;
				cf->softstemc_transfer_to_maintresp	 = diff * cs->softstemc_transfer/nsc_nw;
			

				cf->NSC_nw_to_maintresp  = cf->leafc_storage_to_maintresp + cf->frootc_storage_to_maintresp + cf->fruitc_storage_to_maintresp + cf->softstemc_storage_to_maintresp +
										   cf->leafc_transfer_to_maintresp + cf->frootc_transfer_to_maintresp + cf->fruitc_transfer_to_maintresp + cf->softstemc_transfer_to_maintresp;
	
				if (epc->leaf_cn)     nf->leafn_storage_to_maintresp		  = cf->leafc_storage_to_maintresp / epc->leaf_cn;
				if (epc->froot_cn)    nf->frootn_storage_to_maintresp		  = cf->frootc_storage_to_maintresp / epc->froot_cn;
				if (epc->fruit_cn)    nf->fruitn_storage_to_maintresp		  = cf->fruitc_storage_to_maintresp / epc->fruit_cn;
				if (epc->softstem_cn) nf->softstemn_storage_to_maintresp      = cf->softstemc_storage_to_maintresp / epc->softstem_cn;

				if (epc->leaf_cn)     nf->leafn_transfer_to_maintresp		  = cf->leafc_transfer_to_maintresp / epc->leaf_cn;
				if (epc->froot_cn)    nf->frootn_transfer_to_maintresp		  = cf->frootc_transfer_to_maintresp / epc->froot_cn;
				if (epc->fruit_cn)    nf->fruitn_transfer_to_maintresp		  = cf->fruitc_transfer_to_maintresp / epc->fruit_cn;
				if (epc->softstem_cn) nf->softstemn_transfer_to_maintresp     = cf->softstemc_transfer_to_maintresp / epc->softstem_cn;
	

			
				nf->NSN_nw_to_maintresp = nf->leafn_storage_to_maintresp + nf->frootn_storage_to_maintresp + nf->fruitn_storage_to_maintresp + nf->softstemn_storage_to_maintresp +
									      nf->leafn_transfer_to_maintresp + nf->frootn_transfer_to_maintresp + nf->fruitn_transfer_to_maintresp + nf->softstemn_transfer_to_maintresp;
			
			
				/* 1.3. state update of storage and transfer pools */
				cs->leafc_storage					-= cf->leafc_storage_to_maintresp;
				cs->frootc_storage					-= cf->frootc_storage_to_maintresp;
				cs->fruitc_storage					-= cf->fruitc_storage_to_maintresp;
				cs->softstemc_storage				-= cf->softstemc_storage_to_maintresp;
		

				cs->leafc_transfer					-= cf->leafc_transfer_to_maintresp;
				cs->frootc_transfer					-= cf->frootc_transfer_to_maintresp;
				cs->fruitc_transfer					-= cf->fruitc_transfer_to_maintresp;
				cs->softstemc_transfer				-= cf->softstemc_transfer_to_maintresp;
		
				ns->leafn_storage					-= nf->leafn_storage_to_maintresp;
				ns->frootn_storage					-= nf->frootn_storage_to_maintresp;
				ns->fruitn_storage					-= nf->fruitn_storage_to_maintresp;
				ns->softstemn_storage				-= nf->softstemn_storage_to_maintresp;
		
				ns->leafn_transfer					-= nf->leafn_transfer_to_maintresp;
				ns->frootn_transfer					-= nf->frootn_transfer_to_maintresp;
				ns->fruitn_transfer					-= nf->fruitn_transfer_to_maintresp;
				ns->softstemn_transfer				-= nf->softstemn_transfer_to_maintresp;
	
				ns->retransn                        += nf->NSN_nw_to_maintresp;
			
			}

			/* 1.4. if NSC is not enough -> transfer from actual pool */
			if (nsc_nw == 0 || (diff_total_nw - cf->NSC_nw_to_maintresp) > CRIT_PREC)
			{
				diff = diff_total_nw - cf->NSC_nw_to_maintresp;
				
				if (cf->leaf_day_mr > 0)
				{
					if (cs->leafc > CRIT_PREC)
					{
						cf->leafc_to_maintresp = diff * (cf->leaf_day_mr / mresp_nw); 
						nf->leafn_to_maintresp = cf->leafc_to_maintresp / epc->leaf_cn;
					}
					else
					{
						excess += diff * (cf->leaf_day_mr / mresp_nw);
					}
					
				}

				if (cf->leaf_night_mr > 0)
				{
					if (cs->leafc > CRIT_PREC)
					{
						cf->leafc_to_maintresp += diff * (cf->leaf_night_mr / mresp_nw); 
						nf->leafn_to_maintresp += (diff * (cf->leaf_night_mr / mresp_nw)) / epc->leaf_cn; 
					}
					else
					{
						excess += diff * (cf->leaf_night_mr / mresp_nw);
					}
				}

				if (cf->froot_mr > 0)
				{
					if (cs->frootc > CRIT_PREC)
					{
						cf->frootc_to_maintresp = diff * (cf->froot_mr / mresp_nw); 
						nf->frootn_to_maintresp = cf->frootc_to_maintresp / epc->froot_cn; 
					}
					else
					{
						excess += diff * (cf->froot_mr / mresp_nw);
					}
				}

				if (cf->fruit_mr > 0)
				{
					if (cs->fruitc > CRIT_PREC)
					{
						cf->fruitc_to_maintresp = diff * (cf->fruit_mr / mresp_nw); 
						nf->fruitn_to_maintresp = cf->fruitc_to_maintresp / epc->fruit_cn; 
					}
					else
					{
						excess += diff * (cf->fruit_mr / mresp_nw);
					}
				}

				if (cf->softstem_mr > 0)
				{
					if (cs->softstemc > CRIT_PREC)
					{
						cf->softstemc_to_maintresp = diff * (cf->softstem_mr / mresp_nw); 
						nf->softstemn_to_maintresp = cf->softstemc_to_maintresp / epc->softstem_cn; 
					}
					else
					{
						excess += diff * (cf->softstem_mr / mresp_nw);
					}
				}

			

				cf->actC_nw_to_maintresp  = cf->leafc_to_maintresp + cf->frootc_to_maintresp + cf->fruitc_to_maintresp + cf->softstemc_to_maintresp;

				nf->actN_nw_to_maintresp  = nf->leafn_to_maintresp + nf->frootn_to_maintresp + nf->fruitn_to_maintresp + nf->softstemn_to_maintresp;

				/* 1.5. state update of actual pools */
				cs->leafc					-= cf->leafc_to_maintresp;
				cs->frootc					-= cf->frootc_to_maintresp;
				cs->fruitc					-= cf->fruitc_to_maintresp;
				cs->softstemc				-= cf->softstemc_to_maintresp;

				ns->leafn					-= nf->leafn_to_maintresp;
				ns->frootn					-= nf->frootn_to_maintresp;
				ns->fruitn					-= nf->fruitn_to_maintresp;
				ns->softstemn				-= nf->softstemn_to_maintresp;
			
				ns->retransn                += nf->actN_nw_to_maintresp;
				
				
				mresp_nw = cf->leaf_day_mr + cf->leaf_night_mr + cf->froot_mr + cf->fruit_mr + cf->softstem_mr;

				/* if maintresp of non-woody biomass can not be covered from non-woody biomass -> added to woody demand */
				if (excess)
				{
					diff_total_w += excess;
				}

		
			}
		}
	}

	/* 2: woody biomass */
	
	if (mresp_w)
	{
		
		/* 2.1. calculation the difference between NSC and diff (based on available amount) */
		if (diff_total_w > CRIT_PREC)
		{
			/* critical NSC value: fixed proportion of theroretical maximum os NSC value */
			nsc_crit = epc->NSC_avail_prop * (sc_w * epc->NSC_SC_prop);
		
			if (nsc_w < nsc_crit)
			{
				diff = 0;
				epv->NSC_limit_w = 2;
			}
			else
			{
				if ((nsc_w - diff_total_w) > nsc_crit)
				{
					diff = diff_total_w;
					epv->NSC_limit_w = 0;
				}

				else
				{
					diff = nsc_crit - (nsc_w - diff_total_w);
					epv->NSC_limit_w = 1;
				}
			}

			/* 2.2. calculation of flxues from nsc pools */
			if (nsc_w && diff)
			{
				cf->livestemc_storage_to_maintresp	 = diff * cs->livestemc_storage/nsc_w;
				cf->livecrootc_storage_to_maintresp  = diff * cs->livecrootc_storage/nsc_w;
				cf->deadstemc_storage_to_maintresp	 = diff * cs->deadstemc_storage/nsc_w;
				cf->deadcrootc_storage_to_maintresp  = diff * cs->deadcrootc_storage/nsc_w;

				cf->livestemc_transfer_to_maintresp	 = diff * cs->livestemc_transfer/nsc_w;
				cf->livecrootc_transfer_to_maintresp = diff * cs->livecrootc_transfer/nsc_w;
				cf->deadstemc_transfer_to_maintresp	 = diff * cs->deadstemc_transfer/nsc_w;
				cf->deadcrootc_transfer_to_maintresp = diff * cs->deadcrootc_transfer/nsc_w;

				cf->NSC_w_to_maintresp  = cf->livestemc_storage_to_maintresp + cf->livecrootc_storage_to_maintresp + cf->deadstemc_storage_to_maintresp + cf->deadcrootc_storage_to_maintresp +
										  cf->livestemc_transfer_to_maintresp + cf->livecrootc_transfer_to_maintresp + cf->deadstemc_transfer_to_maintresp + cf->deadcrootc_transfer_to_maintresp;
	
				if (epc->livewood_cn) nf->livestemn_storage_to_maintresp  = cf->livestemc_storage_to_maintresp / epc->livewood_cn;
				if (epc->livewood_cn) nf->livecrootn_storage_to_maintresp = cf->livecrootc_storage_to_maintresp / epc->livewood_cn;
				if (epc->deadwood_cn) nf->deadstemn_storage_to_maintresp  = cf->deadstemc_storage_to_maintresp / epc->deadwood_cn;
				if (epc->deadwood_cn) nf->deadcrootn_storage_to_maintresp = cf->deadcrootc_storage_to_maintresp / epc->deadwood_cn;

				if (epc->livewood_cn) nf->livestemn_transfer_to_maintresp  = cf->livestemc_transfer_to_maintresp / epc->livewood_cn;
				if (epc->livewood_cn) nf->livecrootn_transfer_to_maintresp = cf->livecrootc_transfer_to_maintresp / epc->livewood_cn;
				if (epc->deadwood_cn) nf->deadstemn_transfer_to_maintresp  = cf->deadstemc_transfer_to_maintresp / epc->deadwood_cn;
				if (epc->deadwood_cn) nf->deadcrootn_transfer_to_maintresp = cf->deadcrootc_transfer_to_maintresp / epc->deadwood_cn;

			
				nf->NSN_w_to_maintresp = nf->livestemn_storage_to_maintresp + nf->livecrootn_storage_to_maintresp + nf->deadstemn_storage_to_maintresp + nf->deadcrootn_storage_to_maintresp +
									     nf->livestemn_transfer_to_maintresp + nf->livecrootn_transfer_to_maintresp + nf->deadstemn_transfer_to_maintresp + nf->deadcrootn_transfer_to_maintresp;
			
			
				/* 2.3. state update of storage and transfer pools */
				cs->livestemc_storage				-= cf->livestemc_storage_to_maintresp;
				cs->livecrootc_storage				-= cf->livecrootc_storage_to_maintresp;
				cs->deadstemc_storage				-= cf->deadstemc_storage_to_maintresp;
				cs->deadcrootc_storage				-= cf->deadcrootc_storage_to_maintresp;

				cs->livestemc_transfer				-= cf->livestemc_transfer_to_maintresp;
				cs->livecrootc_transfer				-= cf->livecrootc_transfer_to_maintresp;
				cs->deadstemc_transfer				-= cf->deadstemc_transfer_to_maintresp;
				cs->deadcrootc_transfer				-= cf->deadcrootc_transfer_to_maintresp;
			
				ns->livestemn_storage				-= nf->livestemn_storage_to_maintresp;
				ns->livecrootn_storage				-= nf->livecrootn_storage_to_maintresp;
				ns->deadstemn_storage				-= nf->deadstemn_storage_to_maintresp;
				ns->deadcrootn_storage				-= nf->deadcrootn_storage_to_maintresp;

				ns->livestemn_transfer				-= nf->livestemn_transfer_to_maintresp;
				ns->livecrootn_transfer				-= nf->livecrootn_transfer_to_maintresp;
				ns->deadstemn_transfer				-= nf->deadstemn_transfer_to_maintresp;
				ns->deadcrootn_transfer				-= nf->deadcrootn_transfer_to_maintresp;
			
				ns->retransn                        += nf->NSN_w_to_maintresp;
			
			}

			/* 1.5. if NSC is not enough -> transfer from actual pool */
			if (nsc_w == 0 || (diff_total_w - cf->NSC_w_to_maintresp) > CRIT_PREC)
			{
				diff = diff_total_w - cf->NSC_w_to_maintresp;

				if (cf->livestem_mr > 0)
				{
					if (cs->livestemc > CRIT_PREC)
					{
						cf->livestemc_to_maintresp = diff * (cf->livestem_mr / mresp_w); 
						nf->livestemn_to_maintresp = cf->livestemc_to_maintresp / epc->livewood_cn; 
					}
					else
					{
						cf->livestem_mr       = 0;
						printf("WARNING: zero livestem maintanance respiration (nsc_maintresp.c)\n");
					}
				}

				if (cf->livecroot_mr > 0)
				{
					if (cs->livecrootc > CRIT_PREC)
					{
						cf->livecrootc_to_maintresp = diff * (cf->livecroot_mr / mresp_w); 
						nf->livecrootn_to_maintresp = cf->livecrootc_to_maintresp / epc->livewood_cn;
					}
					else
					{
						cf->livecroot_mr       = 0;
						printf("WARNING: zero livecroot maintanance respiration (nsc_maintresp.c)\n");
					}
				}

				cf->actC_w_to_maintresp  = cf->livestemc_to_maintresp + cf->livecrootc_to_maintresp;

				nf->actN_w_to_maintresp  = nf->livestemn_to_maintresp + nf->livecrootn_to_maintresp;

				/* 1.6. state update of actual pools */
				cs->livestemc				-= cf->livestemc_to_maintresp;
				cs->livecrootc				-= cf->livecrootc_to_maintresp;

				ns->livestemn				-= nf->livestemn_to_maintresp;
				ns->livecrootn				-= nf->livecrootn_to_maintresp;
			
				ns->retransn                += nf->actN_w_to_maintresp;
				
				
				mresp_w = cf->livestem_mr + cf->livecroot_mr;

			
			}
		}

	

	}

	/* state update of cpool */
	cs->cpool			 -= (mresp_nw - cf->NSC_nw_to_maintresp - cf->actC_nw_to_maintresp);
	cs->cpool			 -= (mresp_w  - cf->NSC_w_to_maintresp  - cf->actC_w_to_maintresp);
	if (cs->cpool < 0 && fabs(cs->cpool) > CRIT_PREC)
	{
		cf->leaf_day_mr   += cs->cpool * cf->leaf_day_mr   / mresp_nw;
		cf->leaf_night_mr += cs->cpool * cf->leaf_night_mr / mresp_nw;
		cf->froot_mr      += cs->cpool * cf->froot_mr / mresp_nw;
		cf->fruit_mr      += cs->cpool * cf->fruit_mr / mresp_nw;
		cf->softstem_mr   += cs->cpool * cf->softstem_mr / mresp_nw;

		cs->cpool = 0;
		printf("\n");
		printf("WARNING: limited maintanence respiration (nsc_maintresp.c)\n");
	}
		
	/* 4. state update MR sink pools */
		
	cs->leaf_mr_snk		 += cf->leaf_day_mr;
	cs->leaf_mr_snk		 += cf->leaf_night_mr;
	cs->froot_mr_snk	 += cf->froot_mr;
	cs->fruit_mr_snk	 += cf->fruit_mr;
	cs->softstem_mr_snk  += cf->softstem_mr;
	cs->livestem_mr_snk  += cf->livestem_mr;
	cs->livecroot_mr_snk += cf->livecroot_mr;
	cs->NSC_mr_snk       += cf->NSC_nw_to_maintresp + cf->NSC_w_to_maintresp;
	cs->actC_mr_snk      += cf->actC_nw_to_maintresp + cf->actC_w_to_maintresp;
	
		

	cs->nsc_nw = cs->leafc_storage      +  cs->frootc_storage     + cs->fruitc_storage     + cs->softstemc_storage +
		         cs->leafc_transfer     +  cs->frootc_transfer    + cs->fruitc_transfer    + cs->softstemc_transfer;

	cs->nsc_w = cs->livestemc_storage  + cs->livecrootc_storage  + cs->deadstemc_storage  + cs->deadcrootc_storage +
			    cs->livestemc_transfer + cs->livecrootc_transfer + cs->deadstemc_transfer + cs->deadcrootc_transfer;

	cs->sc_nw = cs->leafc     +  cs->frootc    + cs->fruitc     + cs->softstemc;

	cs->sc_w = cs->livestemc  + cs->livecrootc  + cs->deadstemc  + cs->deadcrootc;

	return (!ok);
}			

