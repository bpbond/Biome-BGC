/*
state_update.c
Resolve the fluxes in bgc() daily loop to update state variables

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.3
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2016, D. Hidy [dori.hidy@gmail.com]
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


int daily_water_state_update(wflux_struct* wf, wstate_struct* ws)
{
	/* daily update of the water state variables */
	 
	int ok=1;

	/* Hidy 2011 - multilayer soil */
	int layer;
	double soilw_SUM = 0;
	
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
	
	 /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
	/* MODIFICATIONS -  Hidy 2011 */
	
	/* bare soil evaporation */
	ws->soilevap_snk   += wf->soilw_evap;
	
	/* transpiration */
	ws->trans_snk      += wf->soilw_trans_SUM;
	
	/* runoff - from the top soil layer (net loss) */
	ws->runoff_snk	   += wf->prcp_to_runoff;

	/* deep percolation: percolation of the bottom layer is net loss for the sytem*/
	ws->deeppercolation_snk += wf->soilw_percolated[N_SOILLAYERS-1];

	/* deep diffusion: diffusion (downward) of the bottom layer is net loss for the sytem*/
	ws->deepdiffusion_snk += wf->soilw_diffused[N_SOILLAYERS-1];

	/* deep transpiration: transpiration from bottom layer is net gain for the sytem*/
	ws->deeptrans_src += wf->soilw_trans[N_SOILLAYERS-1];

   
	
	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! MULTILAYER SOIL !!!!!!!!!!!!!!!!!!!!!!!!!! */	
	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		soilw_SUM           +=  ws->soilw[layer];
		ws->groundwater_src += wf->soilw_from_GW[layer];
	}

	ws->soilw_SUM = soilw_SUM;

	wf->evapotransp = wf->canopyw_evap + wf->soilw_evap + wf->soilw_trans_SUM + wf->snoww_subl;
	
	
	return (!ok);
}

int daily_carbon_state_update(cflux_struct* cf, cstate_struct* cs,
int alloc, int woody, int evergreen)
{
	/* daily update of the carbon state variables */
	
	int ok=1;
	
	/* C state variables are updated below in the order of the relevant
	fluxes in the daily model loop */
	
	/* NOTE: Mortality fluxes are all accounted for in a separate routine, 
	which is to be called after this routine.  This is a special case
	where the updating of state variables is order-sensitive, since 
	otherwise the complications of possibly having mortality fluxes drive 
	the pools negative would create big, unnecessary headaches. */
	
	/* Phenology fluxes */
	/* leaf and fine root transfer growth */
	cs->leafc            += cf->leafc_transfer_to_leafc;
	cs->leafc_transfer   -= cf->leafc_transfer_to_leafc;
	cs->frootc           += cf->frootc_transfer_to_frootc;
	cs->frootc_transfer  -= cf->frootc_transfer_to_frootc;
	/* fruit simulation - Hidy 2013 */
	cs->fruitc           += cf->fruitc_transfer_to_fruitc;
	cs->fruitc_transfer  -= cf->fruitc_transfer_to_fruitc;

 
	/* TREE-specific and NON-WOODY SPECIFIC fluxes */
	if (woody)
	{
		/* Stem and coarse root transfer growth */
		cs->livestemc             += cf->livestemc_transfer_to_livestemc;
		cs->livestemc_transfer    -= cf->livestemc_transfer_to_livestemc;
		cs->deadstemc             += cf->deadstemc_transfer_to_deadstemc;
		cs->deadstemc_transfer    -= cf->deadstemc_transfer_to_deadstemc;
		cs->livecrootc            += cf->livecrootc_transfer_to_livecrootc;
		cs->livecrootc_transfer   -= cf->livecrootc_transfer_to_livecrootc;
		cs->deadcrootc            += cf->deadcrootc_transfer_to_deadcrootc;
		cs->deadcrootc_transfer   -= cf->deadcrootc_transfer_to_deadcrootc;
	}
	else
	{ 	/* SOFT STEM SIMULATION of non-woody biomes - Hidy 2015 */
		cs->softstemc           += cf->softstemc_transfer_to_softstemc;
		cs->softstemc_transfer  -= cf->softstemc_transfer_to_softstemc;
	
	}
	/* Leaf and fine root litterfall */
	cs->litr1c     += cf->leafc_to_litr1c;
	cs->leafc      -= cf->leafc_to_litr1c;
	cs->litr2c     += cf->leafc_to_litr2c;
	cs->leafc      -= cf->leafc_to_litr2c;
	cs->litr3c     += cf->leafc_to_litr3c;
	cs->leafc      -= cf->leafc_to_litr3c;
	cs->litr4c     += cf->leafc_to_litr4c;
	cs->leafc      -= cf->leafc_to_litr4c;
	cs->litr1c     += cf->frootc_to_litr1c;
	cs->frootc     -= cf->frootc_to_litr1c;
	cs->litr2c     += cf->frootc_to_litr2c;
	cs->frootc     -= cf->frootc_to_litr2c;
	cs->litr3c     += cf->frootc_to_litr3c;
	cs->frootc     -= cf->frootc_to_litr3c;
	cs->litr4c     += cf->frootc_to_litr4c;
	cs->frootc     -= cf->frootc_to_litr4c;
	/* fruit simulation - Hidy 2013. */
	cs->litr1c     += cf->fruitc_to_litr1c;
	cs->fruitc     -= cf->fruitc_to_litr1c;
	cs->litr2c     += cf->fruitc_to_litr2c;
	cs->fruitc     -= cf->fruitc_to_litr2c;
	cs->litr3c     += cf->fruitc_to_litr3c;
	cs->fruitc     -= cf->fruitc_to_litr3c;
	cs->litr4c     += cf->fruitc_to_litr4c;
	cs->fruitc     -= cf->fruitc_to_litr4c;

	/* TREE-specific and NON-WOODY SPECIFIC fluxes */
	if (woody)
	{  /* livewood turnover fluxes */
		cs->deadstemc  += cf->livestemc_to_deadstemc;
		cs->livestemc  -= cf->livestemc_to_deadstemc;
		cs->deadcrootc += cf->livecrootc_to_deadcrootc;
		cs->livecrootc -= cf->livecrootc_to_deadcrootc;
	}
	else
	{
		/* soft stem simulation - Hidy 2015. */
		cs->litr1c     += cf->softstemc_to_litr1c;
		cs->softstemc  -= cf->softstemc_to_litr1c;
		cs->litr2c     += cf->softstemc_to_litr2c;
		cs->softstemc  -= cf->softstemc_to_litr2c;
		cs->litr3c     += cf->softstemc_to_litr3c;
		cs->softstemc  -= cf->softstemc_to_litr3c;
		cs->litr4c     += cf->softstemc_to_litr4c;
		cs->softstemc  -= cf->softstemc_to_litr4c;
	
	}
	
	/* Maintenance respiration fluxes */
	cs->leaf_mr_snk  += cf->leaf_day_mr;
	cs->cpool        -= cf->leaf_day_mr;
	cs->leaf_mr_snk  += cf->leaf_night_mr;
	cs->cpool        -= cf->leaf_night_mr;
	cs->froot_mr_snk += cf->froot_mr;
	cs->cpool        -= cf->froot_mr;
	 /*fruit simulation - Hidy 2013. */
    cs->fruit_mr_snk += cf->fruit_mr;
	cs->cpool        -= cf->fruit_mr;

	

	/* TREE-specific and NON-WOODY SPECIFIC fluxes */
	if (woody)
	{
		cs->livestem_mr_snk  += cf->livestem_mr;
		cs->cpool            -= cf->livestem_mr;
		cs->livecroot_mr_snk += cf->livecroot_mr;
		cs->cpool            -= cf->livecroot_mr;
	}
	else
	{
		/*softstem simulation - Hidy 2013. */
		cs->softstem_mr_snk += cf->softstem_mr;
		cs->cpool        -= cf->softstem_mr;
	
	}
	
	/* Photosynthesis fluxes */
	cs->cpool        += cf->psnsun_to_cpool;
	cs->psnsun_src   += cf->psnsun_to_cpool;
	cs->cpool        += cf->psnshade_to_cpool;
	cs->psnshade_src += cf->psnshade_to_cpool;
	
	/* Litter decomposition fluxes */
	/* Fluxes out of coarse woody debris into litter pools */
	cs->litr2c       += cf->cwdc_to_litr2c;
	cs->cwdc         -= cf->cwdc_to_litr2c;
	cs->litr3c       += cf->cwdc_to_litr3c;
	cs->cwdc         -= cf->cwdc_to_litr3c;
	cs->litr4c       += cf->cwdc_to_litr4c;
	cs->cwdc         -= cf->cwdc_to_litr4c;
	/* Fluxes out of labile litter pool */
	cs->litr1_hr_snk += cf->litr1_hr;
	cs->litr1c       -= cf->litr1_hr;
	cs->soil1c       += cf->litr1c_to_soil1c;
	cs->litr1c       -= cf->litr1c_to_soil1c;
	/* Fluxes out of cellulose litter pool */
	cs->litr2_hr_snk += cf->litr2_hr;
	cs->litr2c       -= cf->litr2_hr;
	cs->soil2c       += cf->litr2c_to_soil2c;
	cs->litr2c       -= cf->litr2c_to_soil2c;
	/* Fluxes from shielded to unshielded cellulose pools */
	cs->litr2c       += cf->litr3c_to_litr2c;
	cs->litr3c       -= cf->litr3c_to_litr2c;
	/* Fluxes out of lignin litter pool */
	cs->litr4_hr_snk += cf->litr4_hr;
	cs->litr4c       -= cf->litr4_hr;
	cs->soil3c       += cf->litr4c_to_soil3c;
	cs->litr4c       -= cf->litr4c_to_soil3c;
	/* Fluxes out of fast soil pool */
	cs->soil1_hr_snk += cf->soil1_hr;
	cs->soil1c       -= cf->soil1_hr;
	cs->soil2c       += cf->soil1c_to_soil2c;
	cs->soil1c       -= cf->soil1c_to_soil2c;
	/* Fluxes out of medium soil pool */
	cs->soil2_hr_snk += cf->soil2_hr;
	cs->soil2c       -= cf->soil2_hr;
	cs->soil3c       += cf->soil2c_to_soil3c;
	cs->soil2c       -= cf->soil2c_to_soil3c;
	/* Fluxes out of slow soil pool */
	cs->soil3_hr_snk += cf->soil3_hr;
	cs->soil3c       -= cf->soil3_hr;
	cs->soil4c       += cf->soil3c_to_soil4c;
	cs->soil3c       -= cf->soil3c_to_soil4c;
	/* Fluxes out of recalcitrant SOM pool */
	cs->soil4_hr_snk += cf->soil4_hr;
	cs->soil4c       -= cf->soil4_hr;

	
	/* Daily allocation fluxes */
	/* daily leaf allocation fluxes */
	cs->leafc          += cf->cpool_to_leafc;
	cs->cpool          -= cf->cpool_to_leafc;
	cs->leafc_storage  += cf->cpool_to_leafc_storage;
	cs->cpool          -= cf->cpool_to_leafc_storage;
	/* Daily fine root allocation fluxes */
	cs->frootc         += cf->cpool_to_frootc;
	cs->cpool          -= cf->cpool_to_frootc;
	cs->frootc_storage += cf->cpool_to_frootc_storage;
	cs->cpool          -= cf->cpool_to_frootc_storage;
	/* fruit simulation - Hidy 2013. */
	cs->fruitc         += cf->cpool_to_fruitc;
	cs->cpool          -= cf->cpool_to_fruitc;
	cs->fruitc_storage += cf->cpool_to_fruitc_storage;
	cs->cpool          -= cf->cpool_to_fruitc_storage; 

	/* TREE-specific and NON-WOODY SPECIFIC fluxes */
	if (woody)
	{
		/* Daily live stem wood allocation fluxes */
		cs->livestemc          += cf->cpool_to_livestemc;
		cs->cpool              -= cf->cpool_to_livestemc;
		cs->livestemc_storage  += cf->cpool_to_livestemc_storage;
		cs->cpool              -= cf->cpool_to_livestemc_storage;
		/* Daily dead stem wood allocation fluxes */
		cs->deadstemc          += cf->cpool_to_deadstemc;
		cs->cpool              -= cf->cpool_to_deadstemc;
		cs->deadstemc_storage  += cf->cpool_to_deadstemc_storage;
		cs->cpool              -= cf->cpool_to_deadstemc_storage;
		/* Daily live coarse root wood allocation fluxes */
		cs->livecrootc         += cf->cpool_to_livecrootc;
		cs->cpool              -= cf->cpool_to_livecrootc;
		cs->livecrootc_storage += cf->cpool_to_livecrootc_storage;
		cs->cpool              -= cf->cpool_to_livecrootc_storage;
		/* Daily dead coarse root wood allocation fluxes */
		cs->deadcrootc         += cf->cpool_to_deadcrootc;
		cs->cpool              -= cf->cpool_to_deadcrootc;
		cs->deadcrootc_storage += cf->cpool_to_deadcrootc_storage;
		cs->cpool              -= cf->cpool_to_deadcrootc_storage;
	}
	else
	{ /* SOFT STEM SIMULATION of non-woody biomes - Hidy 2015 */
		/* softstem simulation - Hidy 2013. */
		cs->softstemc			+= cf->cpool_to_softstemc;
		cs->cpool				-= cf->cpool_to_softstemc;
		cs->softstemc_storage	+= cf->cpool_to_softstemc_storage;
		cs->cpool				-= cf->cpool_to_softstemc_storage; 
	
	}
	/* Daily allocation for transfer growth respiration */
	cs->gresp_storage  += cf->cpool_to_gresp_storage;
	cs->cpool          -= cf->cpool_to_gresp_storage;
	
	/* Daily growth respiration fluxes */
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
		/* fruit simulation - Hidy 2013. */
		cs->fruit_gr_snk     += cf->cpool_fruit_gr;
		cs->cpool            -= cf->cpool_fruit_gr;
		cs->fruit_gr_snk     += cf->cpool_fruit_storage_gr;
		cs->cpool            -= cf->cpool_fruit_storage_gr;
		cs->fruit_gr_snk     += cf->transfer_fruit_gr;
		cs->gresp_transfer   -= cf->transfer_fruit_gr;
	

		/* TREE-specific and NON-WOODY SPECIFIC fluxes */
		if (woody)
		{	
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
		}
		else
		{	/* SOFT STEM SIMULATION of non-woody biomes - Hidy 2015 */

			cs->softstem_gr_snk     += cf->cpool_softstem_gr;
			cs->cpool            -= cf->cpool_softstem_gr;
			cs->softstem_gr_snk     += cf->cpool_softstem_storage_gr;
			cs->cpool            -= cf->cpool_softstem_storage_gr;
			cs->softstem_gr_snk     += cf->transfer_softstem_gr;
			cs->gresp_transfer   -= cf->transfer_softstem_gr;
		}

	
	/* Annual allocation fluxes, one day per year */
	if (alloc)
	{
		/* Move storage material into transfer compartments on the annual
		allocation day. This is a special case, where a flux is assessed in
		the state_variable update routine.  This is required to have the
		allocation of excess C and N show up as new growth in the next growing
		season, instead of two growing seasons from now. */
		cf->leafc_storage_to_leafc_transfer = cs->leafc_storage;
		cf->frootc_storage_to_frootc_transfer = cs->frootc_storage;
		cf->gresp_storage_to_gresp_transfer = cs->gresp_storage;
		/* fruit simulation - Hidy 2013. */
		cf->fruitc_storage_to_fruitc_transfer = cs->fruitc_storage;
	
		/* TREE-specific and NON-WOODY SPECIFIC fluxes */
		if (woody)
		{
			cf->livestemc_storage_to_livestemc_transfer = cs->livestemc_storage;
			cf->deadstemc_storage_to_deadstemc_transfer = cs->deadstemc_storage;
			cf->livecrootc_storage_to_livecrootc_transfer = cs->livecrootc_storage;
			cf->deadcrootc_storage_to_deadcrootc_transfer = cs->deadcrootc_storage;
		}
		else
		{ 	/* SOFT STEM SIMULATION  of non-woody biomes - Hidy 2015. */
			cf->softstemc_storage_to_softstemc_transfer = cs->softstemc_storage;
	
		}
		/* update states variables */
		cs->leafc_transfer    += cf->leafc_storage_to_leafc_transfer;
		cs->leafc_storage     -= cf->leafc_storage_to_leafc_transfer;
		cs->frootc_transfer   += cf->frootc_storage_to_frootc_transfer;
		cs->frootc_storage    -= cf->frootc_storage_to_frootc_transfer;
		cs->gresp_transfer    += cf->gresp_storage_to_gresp_transfer;
		cs->gresp_storage     -= cf->gresp_storage_to_gresp_transfer;
		/* fruit simulation - Hidy 2013. */
		cs->fruitc_transfer    += cf->fruitc_storage_to_fruitc_transfer;
		cs->fruitc_storage     -= cf->fruitc_storage_to_fruitc_transfer;

		/* TREE-specific and NON-WOODY SPECIFIC fluxes */
		if (woody)
		{
			cs->livestemc_transfer  += cf->livestemc_storage_to_livestemc_transfer;
			cs->livestemc_storage   -= cf->livestemc_storage_to_livestemc_transfer;

			cs->deadstemc_transfer  += cf->deadstemc_storage_to_deadstemc_transfer;
			cs->deadstemc_storage   -= cf->deadstemc_storage_to_deadstemc_transfer;
			cs->livecrootc_transfer += cf->livecrootc_storage_to_livecrootc_transfer;
			cs->livecrootc_storage  -= cf->livecrootc_storage_to_livecrootc_transfer;
			cs->deadcrootc_transfer += cf->deadcrootc_storage_to_deadcrootc_transfer;
			cs->deadcrootc_storage  -= cf->deadcrootc_storage_to_deadcrootc_transfer;
		}
		else
		{	/* softstem simulation - Hidy 2013. */
			cs->softstemc_transfer    += cf->softstemc_storage_to_softstemc_transfer;
			cs->softstemc_storage     -= cf->softstemc_storage_to_softstemc_transfer;
		}
		
		/* for deciduous system, force leafc and frootc to exactly 0.0 on the
		last day */
		if (!evergreen)
		{
			if (cs->leafc < 1e-10) cs->leafc = 0.0;
			if (cs->frootc < 1e-10) cs->frootc = 0.0;
		}
	} /* end if allocation day */

	return (!ok);
}		

int daily_nitrogen_state_update(const epconst_struct* epc, nflux_struct* nf, nstate_struct* ns,
int alloc, int woody, int evergreen)
{
	int ok=1;

	/* N state variables are updated below in the order of the relevant
	fluxes in the daily model loop */
	
	/* NOTE: Mortality fluxes are all accounted for in a separate routine, 
	which is to be called after this routine.  This is a special case
	where the updating of state variables is order-sensitive, since 
	otherwise the complications of possibly having mortality fluxes drive 
	the pools negative would create big, unnecessary headaches. */
	
	/* Phenology fluxes */
	/* Leaf and fine root transfer growth */
	ns->leafn           += nf->leafn_transfer_to_leafn;
	ns->leafn_transfer  -= nf->leafn_transfer_to_leafn;
	ns->frootn          += nf->frootn_transfer_to_frootn;
	ns->frootn_transfer -= nf->frootn_transfer_to_frootn;
	/* fruit simulation - Hidy 2013. */
	ns->fruitn           += nf->fruitn_transfer_to_fruitn;
	ns->fruitn_transfer  -= nf->fruitn_transfer_to_fruitn;
	
	/* TREE-specific and NON-WOODY SPECIFIC fluxes */
	if (woody)
	{
		/* Stem and coarse root transfer growth */
		ns->livestemn           += nf->livestemn_transfer_to_livestemn;
		ns->livestemn_transfer  -= nf->livestemn_transfer_to_livestemn;
		ns->deadstemn           += nf->deadstemn_transfer_to_deadstemn;
		ns->deadstemn_transfer  -= nf->deadstemn_transfer_to_deadstemn;
		ns->livecrootn          += nf->livecrootn_transfer_to_livecrootn;
		ns->livecrootn_transfer -= nf->livecrootn_transfer_to_livecrootn;
		ns->deadcrootn          += nf->deadcrootn_transfer_to_deadcrootn;
		ns->deadcrootn_transfer -= nf->deadcrootn_transfer_to_deadcrootn;
	}
	else
	{
		/* softstem simulation - Hidy 2013. */
		ns->softstemn           += nf->softstemn_transfer_to_softstemn;
		ns->softstemn_transfer  -= nf->softstemn_transfer_to_softstemn;
	}
	/* Leaf and fine root litterfall */
	ns->litr1n     += nf->leafn_to_litr1n;
	ns->leafn      -= nf->leafn_to_litr1n;
	ns->litr2n     += nf->leafn_to_litr2n;
	ns->leafn      -= nf->leafn_to_litr2n;
	ns->litr3n     += nf->leafn_to_litr3n;
	ns->leafn      -= nf->leafn_to_litr3n;
	ns->litr4n     += nf->leafn_to_litr4n;
	ns->leafn      -= nf->leafn_to_litr4n;
	ns->retransn   += nf->leafn_to_retransn;   /* N retranslocation */
	ns->leafn      -= nf->leafn_to_retransn;
	ns->litr1n     += nf->frootn_to_litr1n;
	ns->frootn     -= nf->frootn_to_litr1n;
	ns->litr2n     += nf->frootn_to_litr2n;
	ns->frootn     -= nf->frootn_to_litr2n;
	ns->litr3n     += nf->frootn_to_litr3n;
	ns->frootn     -= nf->frootn_to_litr3n;
	ns->litr4n     += nf->frootn_to_litr4n;
	ns->frootn     -= nf->frootn_to_litr4n;
	/* fruit simulation - Hidy 2013. */
	ns->litr1n     += nf->fruitn_to_litr1n;
	ns->fruitn     -= nf->fruitn_to_litr1n;
	ns->litr2n     += nf->fruitn_to_litr2n;
	ns->fruitn     -= nf->fruitn_to_litr2n;
	ns->litr3n     += nf->fruitn_to_litr3n;
	ns->fruitn     -= nf->fruitn_to_litr3n;
	ns->litr4n     += nf->fruitn_to_litr4n;
	ns->fruitn     -= nf->fruitn_to_litr4n;
	/* softstemn simulation - Hidy 2013. */
	ns->litr1n     += nf->softstemn_to_litr1n;
	ns->softstemn  -= nf->softstemn_to_litr1n;
	ns->litr2n     += nf->softstemn_to_litr2n;
	ns->softstemn  -= nf->softstemn_to_litr2n;
	ns->litr3n     += nf->softstemn_to_litr3n;
	ns->softstemn  -= nf->softstemn_to_litr3n;
	ns->litr4n     += nf->softstemn_to_litr4n;
	ns->softstemn  -= nf->softstemn_to_litr4n;

	/* live wood turnover to dead wood */
	ns->deadstemn  += nf->livestemn_to_deadstemn;
	ns->livestemn  -= nf->livestemn_to_deadstemn;
	ns->retransn   += nf->livestemn_to_retransn;   /* N retranslocation */
	ns->livestemn  -= nf->livestemn_to_retransn;
	ns->deadcrootn += nf->livecrootn_to_deadcrootn;
	ns->livecrootn -= nf->livecrootn_to_deadcrootn;
	ns->retransn   += nf->livecrootn_to_retransn;   /* N retranslocation */
	ns->livecrootn -= nf->livecrootn_to_retransn;


	/* Litter and soil decomposition fluxes */
	/* Fluxes out of coarse woody debris into litter pools */
	ns->litr2n     += nf->cwdn_to_litr2n;
	ns->cwdn       -= nf->cwdn_to_litr2n;
	ns->litr3n     += nf->cwdn_to_litr3n;
	ns->cwdn       -= nf->cwdn_to_litr3n;
	ns->litr4n     += nf->cwdn_to_litr4n;
	ns->cwdn       -= nf->cwdn_to_litr4n;

	if (ns->cwdn < 0)
	{
		double cwdnflux=nf->cwdn_to_litr2n+nf->cwdn_to_litr3n+nf->cwdn_to_litr4n;
		nf->cwdn_to_litr2n += ns->cwdn * nf->cwdn_to_litr2n/cwdnflux;
		nf->cwdn_to_litr3n += ns->cwdn * nf->cwdn_to_litr3n/cwdnflux;
		nf->cwdn_to_litr4n += ns->cwdn * nf->cwdn_to_litr4n/cwdnflux;
		ns->cwdn = 0;
	}
	/* N fluxes for immobilization and mineralization */
	ns->soil1n     += nf->litr1n_to_soil1n;
	ns->litr1n     -= nf->litr1n_to_soil1n;
	if (nf->sminn_to_soil1n_l1 < 0.0)
	{
		nf->sminn_to_nvol_l1s1 = -epc->denitrif_prop * nf->sminn_to_soil1n_l1;
	}
	else
	{
		nf->sminn_to_nvol_l1s1 = 0.0;
	}
	ns->soil1n             += nf->sminn_to_soil1n_l1;
	nf->sminn_to_soil_SUM  += nf->sminn_to_soil1n_l1;
	ns->nvol_snk	       += nf->sminn_to_nvol_l1s1;
	nf->sminn_to_soil_SUM  += nf->sminn_to_nvol_l1s1;
	
	ns->soil2n     += nf->litr2n_to_soil2n;
	ns->litr2n     -= nf->litr2n_to_soil2n;
	if (nf->sminn_to_soil2n_l2 < 0.0)
	{
		nf->sminn_to_nvol_l2s2 = -epc->denitrif_prop * nf->sminn_to_soil2n_l2;
	}
	else
	{
		nf->sminn_to_nvol_l2s2 = 0.0;
	}
	ns->soil2n            += nf->sminn_to_soil2n_l2;
	nf->sminn_to_soil_SUM += nf->sminn_to_soil2n_l2;
	ns->nvol_snk          += nf->sminn_to_nvol_l2s2;
	nf->sminn_to_soil_SUM += nf->sminn_to_nvol_l2s2;
	
	ns->litr2n     += nf->litr3n_to_litr2n;
	ns->litr3n     -= nf->litr3n_to_litr2n;
	
	ns->soil3n     += nf->litr4n_to_soil3n;
	ns->litr4n     -= nf->litr4n_to_soil3n;
	if (nf->sminn_to_soil3n_l4 < 0.0)
	{
		nf->sminn_to_nvol_l4s3 = -epc->denitrif_prop * nf->sminn_to_soil3n_l4;
	}
	else
	{
		nf->sminn_to_nvol_l4s3 = 0.0;
	}
	ns->soil3n            += nf->sminn_to_soil3n_l4;
	nf->sminn_to_soil_SUM += nf->sminn_to_soil3n_l4;
	ns->nvol_snk          += nf->sminn_to_nvol_l4s3;
	nf->sminn_to_soil_SUM += nf->sminn_to_nvol_l4s3;
	
	ns->soil2n     += nf->soil1n_to_soil2n;
	ns->soil1n     -= nf->soil1n_to_soil2n;
	if (nf->sminn_to_soil2n_s1 < 0.0)
	{
		nf->sminn_to_nvol_s1s2 = -epc->denitrif_prop * nf->sminn_to_soil2n_s1;
	}
	else
	{
		nf->sminn_to_nvol_s1s2 = 0.0;
	}
	ns->soil2n            += nf->sminn_to_soil2n_s1;
	nf->sminn_to_soil_SUM += nf->sminn_to_soil2n_s1;
	ns->nvol_snk          += nf->sminn_to_nvol_s1s2;
	nf->sminn_to_soil_SUM += nf->sminn_to_nvol_s1s2;
	
	ns->soil3n     += nf->soil2n_to_soil3n;
	ns->soil2n     -= nf->soil2n_to_soil3n;
	if (nf->sminn_to_soil3n_s2 < 0.0)
	{
		nf->sminn_to_nvol_s2s3 = -epc->denitrif_prop * nf->sminn_to_soil3n_s2;
	}
	else
	{
		nf->sminn_to_nvol_s2s3 = 0.0;
	}
	ns->soil3n            += nf->sminn_to_soil3n_s2;
	nf->sminn_to_soil_SUM += nf->sminn_to_soil3n_s2;
	ns->nvol_snk          += nf->sminn_to_nvol_s2s3;
	nf->sminn_to_soil_SUM += nf->sminn_to_nvol_s2s3;
	
	ns->soil4n     += nf->soil3n_to_soil4n;
	ns->soil3n     -= nf->soil3n_to_soil4n;
	if (nf->sminn_to_soil4n_s3 < 0.0)
	{
		nf->sminn_to_nvol_s3s4 = -epc->denitrif_prop * nf->sminn_to_soil4n_s3;
	}
	else
	{
		nf->sminn_to_nvol_s3s4 = 0.0;
	}
	ns->soil4n            += nf->sminn_to_soil4n_s3;
	nf->sminn_to_soil_SUM += nf->sminn_to_soil4n_s3;
	ns->nvol_snk          += nf->sminn_to_nvol_s3s4;
	nf->sminn_to_soil_SUM += nf->sminn_to_nvol_s3s4;
	
	nf->sminn_to_nvol_s4  = epc->denitrif_prop * nf->soil4n_to_sminn;
	nf->sminn_to_soil_SUM -= nf->soil4n_to_sminn;
	ns->soil4n            -= nf->soil4n_to_sminn;
	ns->nvol_snk          += nf->sminn_to_nvol_s4;
	nf->sminn_to_soil_SUM += nf->sminn_to_nvol_s4;

	/* Bulk denitrification of soil mineral N */
	ns->nvol_snk            += nf->sminn_to_denitrif;
	
	/* Plant allocation flux, from N retrans pool */
	ns->npool     += nf->retransn_to_npool;
	ns->retransn  -= nf->retransn_to_npool;

		
	/* Daily allocation fluxes */
	/* Daily leaf allocation fluxes */
	ns->leafn          += nf->npool_to_leafn;
	ns->npool          -= nf->npool_to_leafn;
	ns->leafn_storage  += nf->npool_to_leafn_storage;
	ns->npool          -= nf->npool_to_leafn_storage;
	/* Daily fine root allocation fluxes */
	ns->frootn         += nf->npool_to_frootn;
	ns->npool          -= nf->npool_to_frootn;
	ns->frootn_storage += nf->npool_to_frootn_storage;
	ns->npool          -= nf->npool_to_frootn_storage;
	/* Daily fruit allocation fluxes - Hidy 2013. */
	ns->fruitn          += nf->npool_to_fruitn;
	ns->npool           -= nf->npool_to_fruitn;
	ns->fruitn_storage  += nf->npool_to_fruitn_storage;
	ns->npool           -= nf->npool_to_fruitn_storage;
	
	/* TREE-specific and NON-WOODY SPECIFIC fluxes */
	if (woody)
	{
		/* Daily live stem allocation fluxes */
		ns->livestemn          += nf->npool_to_livestemn;
		ns->npool              -= nf->npool_to_livestemn;
		ns->livestemn_storage  += nf->npool_to_livestemn_storage;
		ns->npool              -= nf->npool_to_livestemn_storage;
		/* Daily dead stem allocation fluxes */
		ns->deadstemn          += nf->npool_to_deadstemn;
		ns->npool              -= nf->npool_to_deadstemn;
		ns->deadstemn_storage  += nf->npool_to_deadstemn_storage;
		ns->npool              -= nf->npool_to_deadstemn_storage;
		/* Daily live coarse root allocation fluxes */
		ns->livecrootn         += nf->npool_to_livecrootn;
		ns->npool              -= nf->npool_to_livecrootn;
		ns->livecrootn_storage += nf->npool_to_livecrootn_storage;
		ns->npool              -= nf->npool_to_livecrootn_storage;
		/* Daily dead coarse root allocation fluxes */
		ns->deadcrootn         += nf->npool_to_deadcrootn;
		ns->npool              -= nf->npool_to_deadcrootn;
		ns->deadcrootn_storage += nf->npool_to_deadcrootn_storage;
		ns->npool              -= nf->npool_to_deadcrootn_storage;
	}
	else
	{
		/* Daily softstem allocation fluxes - Hidy 2013. */
		ns->softstemn          += nf->npool_to_softstemn;
		ns->npool              -= nf->npool_to_softstemn;
		ns->softstemn_storage  += nf->npool_to_softstemn_storage;
		ns->npool              -= nf->npool_to_softstemn_storage;
	}
	
	
	/* Annual allocation fluxes, one day per year */
	if (alloc)
	{
		/* Move storage material into transfer compartments on the annual
		allocation day. This is a special case, where a flux is assessed in
		the state_variable update routine.  This is required to have the
		allocation of excess C and N show up as new growth in the next growing
		season, instead of two growing seasons from now. */
		nf->leafn_storage_to_leafn_transfer = ns->leafn_storage;
		nf->frootn_storage_to_frootn_transfer = ns->frootn_storage;
		/* fruit simulation - Hidy 2013. */
		nf->fruitn_storage_to_fruitn_transfer = ns->fruitn_storage;
		/* TREE-specific and NON-WOODY SPECIFIC fluxes */
		if (woody)
		{
			nf->livestemn_storage_to_livestemn_transfer = ns->livestemn_storage;
			nf->deadstemn_storage_to_deadstemn_transfer = ns->deadstemn_storage;
			nf->livecrootn_storage_to_livecrootn_transfer = ns->livecrootn_storage;
			nf->deadcrootn_storage_to_deadcrootn_transfer = ns->deadcrootn_storage;
		}
		else
		{
			/* softstem simulation - Hidy 2013. */
			nf->softstemn_storage_to_softstemn_transfer = ns->softstemn_storage;
		}
		/* update states variables */
		ns->leafn_transfer    += nf->leafn_storage_to_leafn_transfer;
		ns->leafn_storage     -= nf->leafn_storage_to_leafn_transfer;
		ns->frootn_transfer   += nf->frootn_storage_to_frootn_transfer;
		ns->frootn_storage    -= nf->frootn_storage_to_frootn_transfer;
		/* fruit simulation - Hidy 2013. */
		ns->fruitn_transfer    += nf->fruitn_storage_to_fruitn_transfer;
		ns->fruitn_storage     -= nf->fruitn_storage_to_fruitn_transfer;

		
		/* TREE-specific and NON-WOODY SPECIFIC fluxes */
		if (woody)
		{
			ns->livestemn_transfer  += nf->livestemn_storage_to_livestemn_transfer;
			ns->livestemn_storage   -= nf->livestemn_storage_to_livestemn_transfer;
			ns->deadstemn_transfer  += nf->deadstemn_storage_to_deadstemn_transfer;
			ns->deadstemn_storage   -= nf->deadstemn_storage_to_deadstemn_transfer;
			ns->livecrootn_transfer += nf->livecrootn_storage_to_livecrootn_transfer;
			ns->livecrootn_storage  -= nf->livecrootn_storage_to_livecrootn_transfer;
			ns->deadcrootn_transfer += nf->deadcrootn_storage_to_deadcrootn_transfer;
			ns->deadcrootn_storage  -= nf->deadcrootn_storage_to_deadcrootn_transfer;
		}
		else
		{
			/* softstem simulation - Hidy 2013. */
			ns->softstemn_transfer    += nf->softstemn_storage_to_softstemn_transfer;
			ns->softstemn_storage     -= nf->softstemn_storage_to_softstemn_transfer;

		}
		/* for deciduous system, force leafn and frootn to exactly 0.0 on the
		last day */
		if (!evergreen)
		{
			if (ns->leafn < 1e-10) ns->leafn = 0.0;
			if (ns->frootn < 1e-10) ns->frootn = 0.0;
		}
	} /* end if annual allocation day */ 
	
	return (!ok);
}
