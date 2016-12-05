/* 
make_zero_flux_struct.c
create structures initialized with zero for forcing fluxes to zero
between simulation days

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.2
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2016, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
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

int make_zero_flux_struct(wflux_struct* wf, cflux_struct* cf, nflux_struct* nf)
{
	int ok=1;
	int layer;
	/* water */
	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		wf->soilw_trans[layer]=0;
		wf->soilw_percolated[layer]=0;
		wf->soilw_diffused[layer]=0;
		wf->soilw_from_GW[layer]=0;
	}

	wf->prcp_to_canopyw=0;
	wf->prcp_to_soilw=0;
	wf->prcp_to_snoww=0;
	wf->prcp_to_runoff=0;
    wf->prcp_to_pondw = 0.;
	wf->canopyw_evap=0;
	wf->canopyw_to_soilw=0;
	wf->pondw_evap=0;
	wf->snoww_subl=0;
	wf->snoww_to_soilw=0;
	wf->soilw_evap=0;
	wf->soilw_trans_SUM=0;
	wf->evapotransp=0;
	wf->prcp_to_pondw=0;
	wf->pondw_to_soilw=0;
	wf->soilw_to_pondw=0;
	wf->canopyw_to_THN=0;
	wf->canopyw_to_MOW=0;
	wf->canopyw_to_HRV=0;
	wf->canopyw_to_PLG=0;
	wf->canopyw_to_GRZ=0;
	wf->IRG_to_prcp=0;
	wf->pot_evap=0;
	
	/* Hidy 2010 - multilayer soil */
	for (layer = 0; layer < N_SOILLAYERS; layer ++)
	{
		wf->soilw_percolated[layer] = 0.0;
		wf->soilw_trans[layer] = 0.0;
		wf->soilw_diffused[layer] = 0.0;
		wf->soilw_from_GW[layer] = 0.0;
	}

	/* daily carbon fluxes */
	cf->m_leafc_to_litr1c = 0.0;
	cf->m_leafc_to_litr2c = 0.0;
	cf->m_leafc_to_litr3c = 0.0;
	cf->m_leafc_to_litr4c = 0.0;
	cf->m_frootc_to_litr1c = 0.0;
	cf->m_frootc_to_litr2c = 0.0;
	cf->m_frootc_to_litr3c = 0.0;
	cf->m_frootc_to_litr4c = 0.0;
	cf->m_leafc_storage_to_litr1c = 0.0;
	cf->m_frootc_storage_to_litr1c = 0.0;
	cf->m_livestemc_storage_to_litr1c = 0.0;
	cf->m_deadstemc_storage_to_litr1c = 0.0;
	cf->m_livecrootc_storage_to_litr1c = 0.0;
	cf->m_deadcrootc_storage_to_litr1c = 0.0;
	cf->m_leafc_transfer_to_litr1c = 0.0;
	cf->m_frootc_transfer_to_litr1c = 0.0;
	cf->m_livestemc_transfer_to_litr1c = 0.0;
	cf->m_deadstemc_transfer_to_litr1c = 0.0;
	cf->m_livecrootc_transfer_to_litr1c = 0.0;
	cf->m_deadcrootc_transfer_to_litr1c = 0.0;
    cf->m_livestemc_to_cwdc = 0.0;
    cf->m_deadstemc_to_cwdc = 0.0;
    cf->m_livecrootc_to_cwdc = 0.0;
    cf->m_deadcrootc_to_cwdc = 0.0;
	cf->m_gresp_storage_to_litr1c = 0.0;
    cf->m_gresp_transfer_to_litr1c = 0.0;
	cf->m_leafc_to_fire = 0.0;
	cf->m_frootc_to_fire = 0.0;
	cf->m_leafc_storage_to_fire = 0.0;
	cf->m_frootc_storage_to_fire = 0.0;
	cf->m_livestemc_storage_to_fire = 0.0;
	cf->m_deadstemc_storage_to_fire = 0.0;
	cf->m_livecrootc_storage_to_fire = 0.0;
	cf->m_deadcrootc_storage_to_fire = 0.0;
	cf->m_leafc_transfer_to_fire = 0.0;
	cf->m_frootc_transfer_to_fire = 0.0;
	cf->m_livestemc_transfer_to_fire = 0.0;
	cf->m_deadstemc_transfer_to_fire = 0.0;
	cf->m_livecrootc_transfer_to_fire = 0.0;
	cf->m_deadcrootc_transfer_to_fire = 0.0;
    cf->m_livestemc_to_fire = 0.0;
    cf->m_deadstemc_to_fire = 0.0;
    cf->m_livecrootc_to_fire = 0.0;
    cf->m_deadcrootc_to_fire = 0.0;
	cf->m_gresp_storage_to_fire = 0.0;
    cf->m_gresp_transfer_to_fire = 0.0;
	cf->m_litr1c_to_fire = 0.0;
	cf->m_litr2c_to_fire = 0.0;
	cf->m_litr3c_to_fire = 0.0;
	cf->m_litr4c_to_fire = 0.0;
	cf->m_cwdc_to_fire = 0.0;
	cf->m_STDBc_to_SNSC = 0;
    cf->leafc_transfer_to_leafc = 0.0;
    cf->frootc_transfer_to_frootc = 0.0;
    cf->livestemc_transfer_to_livestemc = 0.0;
    cf->deadstemc_transfer_to_deadstemc = 0.0;
    cf->livecrootc_transfer_to_livecrootc = 0.0;
    cf->deadcrootc_transfer_to_deadcrootc = 0.0;
    cf->leafc_to_litr1c = 0.0;
    cf->leafc_to_litr2c = 0.0;
    cf->leafc_to_litr3c = 0.0;
    cf->leafc_to_litr4c = 0.0;
    cf->frootc_to_litr1c = 0.0;
    cf->frootc_to_litr2c = 0.0;
    cf->frootc_to_litr3c = 0.0;
    cf->frootc_to_litr4c = 0.0;
    cf->leaf_day_mr = 0.0;
    cf->leaf_night_mr = 0.0;
    cf->froot_mr = 0.0;
    cf->livestem_mr = 0.0;
	cf->livecroot_mr = 0.0;
    cf->psnsun_to_cpool = 0.0;
    cf->psnshade_to_cpool = 0.0;
    cf->cwdc_to_litr2c = 0.0;
    cf->cwdc_to_litr3c = 0.0;
    cf->cwdc_to_litr4c = 0.0;
    cf->litr1_hr = 0.0;
    cf->litr1c_to_soil1c = 0.0;
    cf->litr2_hr = 0.0;
    cf->litr2c_to_soil2c = 0.0;
    cf->litr3c_to_litr2c = 0.0;
    cf->litr4_hr = 0.0;
    cf->litr4c_to_soil3c = 0.0;
    cf->soil1_hr = 0.0;
    cf->soil1c_to_soil2c = 0.0;
    cf->soil2_hr = 0.0;
    cf->soil2c_to_soil3c = 0.0;
    cf->soil3_hr = 0.0;
    cf->soil3c_to_soil4c = 0.0;
    cf->soil4_hr = 0.0;
    cf->cpool_to_leafc = 0.0;
    cf->cpool_to_leafc_storage = 0.0;
    cf->cpool_to_frootc = 0.0;
    cf->cpool_to_frootc_storage = 0.0;
    cf->cpool_to_livestemc = 0.0;
    cf->cpool_to_livestemc_storage = 0.0;
    cf->cpool_to_deadstemc = 0.0;
    cf->cpool_to_deadstemc_storage = 0.0;
    cf->cpool_to_livecrootc = 0.0;
    cf->cpool_to_livecrootc_storage = 0.0;
    cf->cpool_to_deadcrootc = 0.0;
    cf->cpool_to_deadcrootc_storage = 0.0;
    cf->cpool_to_gresp_storage = 0.0;
    cf->cpool_leaf_gr = 0.0;
    cf->cpool_leaf_storage_gr = 0.0;
    cf->transfer_leaf_gr = 0.0;
    cf->cpool_froot_gr = 0.0;
    cf->cpool_froot_storage_gr = 0.0;
    cf->transfer_froot_gr = 0.0;
    cf->cpool_livestem_gr = 0.0;
    cf->cpool_livestem_storage_gr = 0.0;
    cf->transfer_livestem_gr = 0.0;
    cf->cpool_deadstem_gr = 0.0;
    cf->cpool_deadstem_storage_gr = 0.0;
    cf->transfer_deadstem_gr = 0.0;
    cf->cpool_livecroot_gr = 0.0;
    cf->cpool_livecroot_storage_gr = 0.0;
    cf->transfer_livecroot_gr = 0.0;
    cf->cpool_deadcroot_gr = 0.0;
    cf->cpool_deadcroot_storage_gr = 0.0;
    cf->transfer_deadcroot_gr = 0.0;
    cf->leafc_storage_to_leafc_transfer = 0.0;
    cf->frootc_storage_to_frootc_transfer = 0.0;
    cf->livestemc_storage_to_livestemc_transfer = 0.0;
    cf->deadstemc_storage_to_deadstemc_transfer = 0.0;
    cf->livecrootc_storage_to_livecrootc_transfer = 0.0;
    cf->deadcrootc_storage_to_deadcrootc_transfer = 0.0;
    cf->gresp_storage_to_gresp_transfer = 0.0;
    cf->livestemc_to_deadstemc = 0.0;
    cf->livecrootc_to_deadcrootc = 0.0;

	/* !!!  Hidy 2012. - management !!! */
	/* planting */
	cf->leafc_transfer_from_PLT = 0.0;
	cf->frootc_transfer_from_PLT = 0.0;
	/* thinning */
	cf->leafc_to_THN =0.0;
	cf->leafc_storage_to_THN =0.0;
	cf->leafc_transfer_to_THN = 0.0;
	cf->frootc_to_THN = 0;
	cf->frootc_storage_to_THN = 0;
	cf->frootc_transfer_to_THN = 0;
	cf->livecrootc_to_THN = 0;
	cf->livecrootc_storage_to_THN = 0;
	cf->livecrootc_transfer_to_THN = 0;
	cf->deadcrootc_to_THN = 0;
	cf->deadcrootc_storage_to_THN = 0;
	cf->deadcrootc_transfer_to_THN = 0;
	cf->livestemc_to_THN = 0;
	cf->livestemc_storage_to_THN = 0;
	cf->livestemc_transfer_to_THN = 0;
	cf->deadstemc_to_THN = 0;
	cf->deadstemc_storage_to_THN = 0;
	cf->deadstemc_transfer_to_THN = 0;
	cf->gresp_transfer_to_THN =0.0;
    cf->gresp_storage_to_THN =0.0;
	cf->leafc_transfer_to_THN =0.0;
	cf->THN_to_litr1c = 0.0;				
	cf->THN_to_litr2c = 0.0;				 
	cf->THN_to_litr3c = 0.0;				
	cf->THN_to_litr4c = 0.0;
	cf->THN_to_cwdc = 0.0;
	cf->STDBc_to_THN = 0.0;
	/* mowing */
	cf->leafc_to_MOW =0.0;
	cf->leafc_storage_to_MOW =0.0;
	cf->leafc_transfer_to_MOW =0.0;
	cf->STDBc_to_MOW = 0.0;
	cf->gresp_transfer_to_MOW =0.0;
    cf->gresp_storage_to_MOW =0.0;
	cf->leafc_transfer_to_MOW =0.0;
	cf->STDBc_to_MOW = 0.0;
	cf->MOW_to_litr1c = 0.0;				
	cf->MOW_to_litr2c = 0.0;				 
	cf->MOW_to_litr3c = 0.0;				
	cf->MOW_to_litr4c = 0.0;
	cf->STDBc_to_MOW = 0.0;
	/* harvesting */
	cf->leafc_to_HRV =0.0;
	cf->leafc_storage_to_HRV =0.0;
	cf->leafc_transfer_to_HRV =0.0;
	cf->gresp_storage_to_HRV =0.0;
	cf->gresp_transfer_to_HRV =0.0;
	cf->HRV_to_litr1c = 0.0;				
	cf->HRV_to_litr2c = 0.0;				 
	cf->HRV_to_litr3c = 0.0;				
	cf->HRV_to_litr4c = 0.0;
	cf->STDBc_to_HRV = 0.0;
	/* ploughing */
	cf->leafc_to_PLG =0.0;
	cf->leafc_storage_to_PLG =0.0;
	cf->leafc_transfer_to_PLG =0.0;
	cf->gresp_storage_to_PLG =0.0;
	cf->gresp_transfer_to_PLG =0.0;
	cf->frootc_to_PLG =0.0;
	cf->frootc_storage_to_PLG =0.0;
	cf->frootc_transfer_to_PLG =0.0;
	cf->PLG_to_litr1c = 0.0;				
	cf->PLG_to_litr2c = 0.0;				 
	cf->PLG_to_litr3c = 0.0;				
	cf->PLG_to_litr4c = 0.0;
	cf->STDBc_to_PLG = 0.0;
	cf->CTDBc_to_PLG = 0.0;
	/* grazing */
	cf->leafc_to_GRZ =0.0;
	cf->leafc_storage_to_GRZ =0.0;
	cf->leafc_transfer_to_GRZ =0.0;
	cf->gresp_storage_to_GRZ =0.0;
	cf->gresp_transfer_to_GRZ =0.0;
	cf->GRZ_to_litr1c = 0.0;				
	cf->GRZ_to_litr2c = 0.0;				 
	cf->GRZ_to_litr3c = 0.0;				
	cf->GRZ_to_litr4c = 0.0;
	cf->STDBc_to_GRZ = 0.0;
	/* fertilizing */
	cf->FRZ_to_litr1c = 0.0;				
	cf->FRZ_to_litr2c = 0.0;				 
	cf->FRZ_to_litr3c = 0.0;	
	cf->FRZ_to_litr4c = 0.0;
	/* Hidy 2015 - OTHER GHG */
	cf->CH4_flux_soil = 0.0;
	cf->CH4_flux_MANURE = 0.0;
	cf->CH4_flux_FERMENT = 0.0;
       
      /* Hidy 2010 -  senescence */
	cf->SNSC_to_litr1c = 0.0;
	cf->SNSC_to_litr2c = 0.0;
	cf->SNSC_to_litr3c = 0.0;
	cf->SNSC_to_litr4c = 0.0;
	cf->m_vegc_to_SNSC = 0.0;
	cf->m_leafc_to_SNSC = 0.0;
	cf->m_frootc_to_SNSC = 0.0;
	cf->m_leafc_storage_to_SNSC = 0.0;
	cf->m_frootc_storage_to_SNSC = 0.0;
	cf->m_leafc_transfer_to_SNSC = 0.0;
	cf->m_frootc_transfer_to_SNSC = 0.0;
	cf->m_gresp_storage_to_SNSC = 0.0;
	cf->m_gresp_transfer_to_SNSC = 0.0;

	/* fruit simulation - Hidy 2013. */
	cf->fruit_mr = 0.0;
	cf->fruitc_storage_to_fruitc_transfer = 0.0;
	cf->fruitc_storage_to_GRZ = 0.0;
	cf->fruitc_storage_to_HRV = 0.0;
	cf->fruitc_storage_to_PLG = 0.0;
	cf->fruitc_storage_to_THN = 0.0;
	cf->fruitc_storage_to_MOW = 0.0;
	cf->fruitc_transfer_to_GRZ = 0.0;
	cf->fruitc_transfer_to_HRV = 0.0;
	cf->fruitc_transfer_to_PLG = 0.0;
	cf->fruitc_transfer_to_THN = 0.0;
	cf->fruitc_transfer_to_MOW = 0.0;
	cf->fruitc_to_GRZ = 0.0;
	cf->fruitc_to_HRV = 0.0;
	cf->fruitc_to_PLG = 0.0;
	cf->fruitc_to_THN = 0.0;
	cf->fruitc_to_MOW = 0.0;
	cf->fruitc_to_litr1c = 0.0;
	cf->fruitc_to_litr2c = 0.0;
	cf->fruitc_to_litr2c = 0.0;
	cf->fruitc_to_litr3c = 0.0;
	cf->fruitc_to_litr4c = 0.0;
	cf->fruitc_transfer_from_PLT = 0.0;
	cf->m_fruitc_storage_to_fire = 0.0;
	cf->m_fruitc_storage_to_litr1c = 0.0;
	cf->m_fruitc_storage_to_SNSC = 0.0;
	cf->m_fruitc_transfer_to_fire = 0.0;
	cf->m_fruitc_transfer_to_litr1c = 0.0;
	cf->m_fruitc_transfer_to_SNSC = 0.0;
	cf->m_fruitc_to_fire = 0.0;
	cf->m_fruitc_to_litr1c = 0.0;
	cf->m_fruitc_to_SNSC = 0.0;
	cf->cpool_fruit_gr = 0.0;
	cf->cpool_fruit_storage_gr = 0.0;
	cf->cpool_to_fruitc = 0.0;
	cf->cpool_to_fruitc_storage = 0.0;
	cf->transfer_fruit_gr = 0.0;
	cf->fruitc_transfer_to_fruitc = 0.0;

	/* softstem simulation - Hidy 2013. */
	cf->softstem_mr = 0.0;
	cf->softstemc_storage_to_softstemc_transfer = 0.0;
	cf->softstemc_storage_to_GRZ = 0.0;
	cf->softstemc_storage_to_HRV = 0.0;
	cf->softstemc_storage_to_PLG = 0.0;
	cf->softstemc_storage_to_MOW = 0.0;
	cf->softstemc_transfer_to_GRZ = 0.0;
	cf->softstemc_transfer_to_HRV = 0.0;
	cf->softstemc_transfer_to_PLG = 0.0;
	cf->softstemc_transfer_to_MOW = 0.0;
	cf->softstemc_to_GRZ = 0.0;
	cf->softstemc_to_HRV = 0.0;
	cf->softstemc_to_PLG = 0.0;
	cf->softstemc_to_MOW = 0.0;
	cf->softstemc_to_litr1c = 0.0;
	cf->softstemc_to_litr2c = 0.0;
	cf->softstemc_to_litr2c = 0.0;
	cf->softstemc_to_litr3c = 0.0;
	cf->softstemc_to_litr4c = 0.0;
	cf->softstemc_transfer_from_PLT = 0.0;
	cf->m_softstemc_storage_to_fire = 0.0;
	cf->m_softstemc_storage_to_litr1c = 0.0;
	cf->m_softstemc_storage_to_SNSC = 0.0;
	cf->m_softstemc_transfer_to_fire = 0.0;
	cf->m_softstemc_transfer_to_litr1c = 0.0;
	cf->m_softstemc_transfer_to_SNSC = 0.0;
	cf->m_softstemc_to_fire = 0.0;
	cf->m_softstemc_to_litr1c = 0.0;
	cf->m_softstemc_to_SNSC = 0.0;
	cf->cpool_softstem_gr = 0.0;
	cf->cpool_softstem_storage_gr = 0.0;
	cf->cpool_to_softstemc = 0.0;
	cf->cpool_to_softstemc_storage = 0.0;
	cf->transfer_softstem_gr = 0.0;
	cf->softstemc_transfer_to_softstemc = 0.0;
	         	
	/* daily nitrogen fluxes */
	nf->m_leafn_to_litr1n = 0.0;
	nf->m_leafn_to_litr2n = 0.0;
	nf->m_leafn_to_litr3n = 0.0;
	nf->m_leafn_to_litr4n = 0.0;
	nf->m_frootn_to_litr1n = 0.0;
	nf->m_frootn_to_litr2n = 0.0;
	nf->m_frootn_to_litr3n = 0.0;
	nf->m_frootn_to_litr4n = 0.0;
	nf->m_leafn_storage_to_litr1n = 0.0;
    nf->m_frootn_storage_to_litr1n = 0.0;
    nf->m_livestemn_storage_to_litr1n = 0.0;
    nf->m_deadstemn_storage_to_litr1n = 0.0;
    nf->m_livecrootn_storage_to_litr1n = 0.0;
    nf->m_deadcrootn_storage_to_litr1n = 0.0;
    nf->m_leafn_transfer_to_litr1n = 0.0;
    nf->m_frootn_transfer_to_litr1n = 0.0;
    nf->m_livestemn_transfer_to_litr1n = 0.0;
    nf->m_deadstemn_transfer_to_litr1n = 0.0;
    nf->m_livecrootn_transfer_to_litr1n = 0.0;
    nf->m_deadcrootn_transfer_to_litr1n = 0.0;
    nf->m_livestemn_to_litr1n = 0.0;
    nf->m_livestemn_to_cwdn = 0.0;
    nf->m_deadstemn_to_cwdn = 0.0;
    nf->m_livecrootn_to_litr1n = 0.0;
    nf->m_livecrootn_to_cwdn = 0.0;
    nf->m_deadcrootn_to_cwdn = 0.0;
	nf->m_retransn_to_litr1n = 0.0;
	nf->m_leafn_to_fire = 0.0;
	nf->m_frootn_to_fire = 0.0;
	nf->m_leafn_storage_to_fire = 0.0;
    nf->m_frootn_storage_to_fire = 0.0;
    nf->m_livestemn_storage_to_fire = 0.0;
    nf->m_deadstemn_storage_to_fire = 0.0;
    nf->m_livecrootn_storage_to_fire = 0.0;
    nf->m_deadcrootn_storage_to_fire = 0.0;
    nf->m_leafn_transfer_to_fire = 0.0;
    nf->m_frootn_transfer_to_fire = 0.0;
    nf->m_livestemn_transfer_to_fire = 0.0;
    nf->m_deadstemn_transfer_to_fire = 0.0;
    nf->m_livecrootn_transfer_to_fire = 0.0;
    nf->m_deadcrootn_transfer_to_fire = 0.0;
    nf->m_livestemn_to_fire = 0.0;
    nf->m_deadstemn_to_fire = 0.0;
    nf->m_livecrootn_to_fire = 0.0;
    nf->m_deadcrootn_to_fire = 0.0;
	nf->m_retransn_to_fire = 0.0;
	nf->m_litr1n_to_fire = 0.0;
	nf->m_litr2n_to_fire = 0.0;
	nf->m_litr3n_to_fire = 0.0;
	nf->m_litr4n_to_fire = 0.0;
	nf->m_cwdn_to_fire = 0.0;
    nf->leafn_transfer_to_leafn = 0.0;
    nf->frootn_transfer_to_frootn = 0.0;
    nf->livestemn_transfer_to_livestemn = 0.0;
    nf->deadstemn_transfer_to_deadstemn = 0.0;
    nf->livecrootn_transfer_to_livecrootn = 0.0;
    nf->deadcrootn_transfer_to_deadcrootn = 0.0;
    nf->leafn_to_litr1n = 0.0;
    nf->leafn_to_litr2n = 0.0;
    nf->leafn_to_litr3n = 0.0;
    nf->leafn_to_litr4n = 0.0;
    nf->leafn_to_retransn = 0.0;
    nf->frootn_to_litr1n = 0.0;
    nf->frootn_to_litr2n = 0.0;
    nf->frootn_to_litr3n = 0.0;
    nf->frootn_to_litr4n = 0.0;
    nf->ndep_to_sminn = 0.0; 
    nf->nfix_to_sminn = 0.0; 
    nf->cwdn_to_litr2n = 0.0;
    nf->cwdn_to_litr3n = 0.0;
    nf->cwdn_to_litr4n = 0.0;
    nf->litr1n_to_soil1n = 0.0;
    nf->sminn_to_soil1n_l1 = 0.0;
    nf->litr2n_to_soil2n = 0.0;
    nf->sminn_to_soil2n_l2 = 0.0;
    nf->litr3n_to_litr2n = 0.0;
    nf->litr4n_to_soil3n = 0.0;
    nf->sminn_to_soil3n_l4 = 0.0;
    nf->soil1n_to_soil2n = 0.0;
    nf->sminn_to_soil2n_s1 = 0.0;
    nf->soil2n_to_soil3n = 0.0;
    nf->sminn_to_soil3n_s2 = 0.0;
    nf->soil3n_to_soil4n = 0.0;
    nf->sminn_to_soil4n_s3 = 0.0;
    nf->soil4n_to_sminn = 0.0;
	nf->sminn_to_nvol_l1s1 = 0.0;
	nf->sminn_to_nvol_l2s2 = 0.0;
	nf->sminn_to_nvol_l4s3 = 0.0;
	nf->sminn_to_nvol_s1s2 = 0.0;
	nf->sminn_to_nvol_s2s3 = 0.0;
	nf->sminn_to_nvol_s3s4 = 0.0;
	nf->sminn_to_nvol_s4 = 0.0;
	/* Hidy 2011 - multilayer soil */
	nf->sminn_to_soil_SUM = 0.0;
	for (layer = 0; layer < N_SOILLAYERS; layer ++)
	{
		nf->sminn_leached[layer]  = 0.0;
		nf->sminn_diffused[layer] = 0.0;
		nf->sminn_to_soil[layer]  = 0.0; 
	}
	nf->retransn_to_npool = 0.0;  
	nf->nplus = 0.0;
    nf->sminn_to_npool = 0.0;
    nf->npool_to_leafn = 0.0;
    nf->npool_to_leafn_storage = 0.0;
    nf->npool_to_frootn = 0.0;
    nf->npool_to_frootn_storage = 0.0;
    nf->npool_to_livestemn = 0.0;
    nf->npool_to_livestemn_storage = 0.0;
    nf->npool_to_deadstemn = 0.0;
    nf->npool_to_deadstemn_storage = 0.0;
    nf->npool_to_livecrootn = 0.0;
    nf->npool_to_livecrootn_storage = 0.0;
    nf->npool_to_deadcrootn = 0.0;
    nf->npool_to_deadcrootn_storage = 0.0;
    nf->leafn_storage_to_leafn_transfer = 0.0;
    nf->frootn_storage_to_frootn_transfer = 0.0;
    nf->livestemn_storage_to_livestemn_transfer = 0.0;
    nf->deadstemn_storage_to_deadstemn_transfer = 0.0;
    nf->livecrootn_storage_to_livecrootn_transfer = 0.0;
    nf->deadcrootn_storage_to_deadcrootn_transfer = 0.0;
    nf->livestemn_to_deadstemn = 0.0;
    nf->livestemn_to_retransn = 0.0;
    nf->livecrootn_to_deadcrootn = 0.0;
    nf->livecrootn_to_retransn = 0.0;
	nf->sminn_to_denitrif =0.0;
	/* planting - Hidy 2012. */
	nf->leafn_transfer_from_PLT = 0.0;
	nf->frootn_transfer_from_PLT= 0.0;
	/* thinning - by Hidy 2012. */
	nf->leafn_to_THN= 0.0;
	nf->leafn_storage_to_THN= 0.0;
	nf->leafn_transfer_to_THN = 0.0;
	nf->frootn_to_THN= 0.0;
	nf->frootn_storage_to_THN= 0.0;
	nf->frootn_transfer_to_THN = 0.0;
	nf->livecrootn_to_THN= 0.0;
	nf->livecrootn_storage_to_THN= 0.0;
	nf->livecrootn_transfer_to_THN = 0.0;
	nf->deadcrootn_to_THN= 0.0;
	nf->deadcrootn_storage_to_THN= 0.0;
	nf->deadcrootn_transfer_to_THN = 0.0;
	nf->livestemn_to_THN= 0.0;
	nf->livestemn_storage_to_THN= 0.0;
	nf->livestemn_transfer_to_THN = 0.0;
	nf->deadstemn_to_THN= 0.0;
	nf->deadstemn_storage_to_THN= 0.0;
	nf->deadstemn_transfer_to_THN = 0.0;
	nf->THN_to_litr1n = 0.0;
	nf->THN_to_litr2n = 0.0;
	nf->THN_to_litr3n = 0.0;
	nf->THN_to_litr4n = 0.0;				 
	nf->THN_to_cwdn = 0.0;
	nf->STDBn_to_THN = 0.0;
	/* mowing - by Hidy 2008. */
	nf->leafn_to_MOW= 0.0;                 
	nf->leafn_storage_to_MOW = 0.0;        
	nf->leafn_transfer_to_MOW = 0.0;
	nf->STDBn_to_MOW = 0.0;
	nf->MOW_to_litr1n = 0.0;				 
	nf->MOW_to_litr2n = 0.0;				 
	nf->MOW_to_litr3n = 0.0;				 
	nf->MOW_to_litr4n = 0.0;
	nf->retransn_to_MOW = 0.0;
	nf->STDBn_to_MOW = 0.0;
	/* harvesting - by Hidy 2012. */
	nf->leafn_to_HRV= 0.0;                 
	nf->leafn_storage_to_HRV= 0.0;         
	nf->leafn_transfer_to_HRV = 0.0;
	nf->HRV_to_litr1n = 0.0;				 
	nf->HRV_to_litr2n = 0.0;				 
	nf->HRV_to_litr3n = 0.0;				 
	nf->HRV_to_litr4n = 0.0;
	nf->retransn_to_HRV = 0.0;
	nf->STDBn_to_HRV = 0.0;
	/* ploughing - Hidy 2012. */
	nf->leafn_to_PLG = 0.0;                 
	nf->leafn_storage_to_PLG = 0.0;        
	nf->leafn_transfer_to_PLG = 0.0;
	nf->frootn_to_PLG = 0.0;				
	nf->frootn_storage_to_PLG = 0.0;        
	nf->frootn_transfer_to_PLG = 0.0;
	nf->PLG_to_litr1n = 0.0;
	nf->PLG_to_litr2n = 0.0;
	nf->PLG_to_litr3n = 0.0;
	nf->PLG_to_litr4n = 0.0;
	nf->retransn_to_PLG = 0.0;
	nf->STDBn_to_PLG = 0.0;
	/* grazing - by Hidy 2008. */
	nf->leafn_to_GRZ = 0.0;                 
	nf->leafn_storage_to_GRZ = 0.0;        
	nf->leafn_transfer_to_GRZ = 0.0;
	nf->GRZ_to_litr1n = 0.0;				 
	nf->GRZ_to_litr2n = 0.0;				 
	nf->GRZ_to_litr3n = 0.0;				 
	nf->GRZ_to_litr4n = 0.0;
	nf->retransn_to_GRZ = 0.0;
	/* fertiliziation -  by Hidy 2008 */
	nf->FRZ_to_sminn = 0.0;      
	nf->FRZ_to_litr1n = 0.0;				 
	nf->FRZ_to_litr2n = 0.0;				 
	nf->FRZ_to_litr3n = 0.0;				 
	nf->FRZ_to_litr4n = 0.0;	
	/* Hidy 2015 - OTHER GHG */
	nf->N2O_flux_soil = 0.0;
	nf->N2O_flux_GRZ  = 0.0;
	nf->N2O_flux_FRZ  = 0.0;
    /* Hidy 2010 - senescence */
	nf->SNSC_to_litr1n = 0.0;
	nf->SNSC_to_litr2n = 0.0;
	nf->SNSC_to_litr3n = 0.0;
	nf->SNSC_to_litr4n = 0.0;
	nf->m_leafn_to_SNSC = 0.0;
	nf->m_frootn_to_SNSC = 0.0;
	nf->m_leafn_storage_to_SNSC = 0.0;
	nf->m_frootn_storage_to_SNSC = 0.0;
	nf->m_leafn_transfer_to_SNSC = 0.0;
	nf->m_frootn_transfer_to_SNSC = 0.0;
	/* fruit simulation - Hidy 2013. */
	nf->fruitn_transfer_to_fruitn = 0.0;
	nf->fruitn_storage_to_fruitn_transfer = 0.0;
	nf->fruitn_storage_to_GRZ = 0.0;
	nf->fruitn_storage_to_HRV = 0.0;
	nf->fruitn_storage_to_PLG = 0.0;
	nf->fruitn_storage_to_THN = 0.0;
	nf->fruitn_storage_to_MOW = 0.0;
	nf->fruitn_transfer_to_GRZ = 0.0;
	nf->fruitn_transfer_to_HRV = 0.0;
	nf->fruitn_transfer_to_PLG = 0.0;
	nf->fruitn_transfer_to_THN = 0.0;
	nf->fruitn_transfer_to_MOW = 0.0;
	nf->fruitn_to_GRZ = 0.0;
	nf->fruitn_to_HRV = 0.0;
	nf->fruitn_to_PLG = 0.0;
	nf->fruitn_to_THN = 0.0;
	nf->fruitn_to_MOW = 0.0;
	nf->fruitn_to_litr1n = 0.0;
	nf->fruitn_to_litr2n = 0.0;
	nf->fruitn_to_litr2n = 0.0;
	nf->fruitn_to_litr3n = 0.0;
	nf->fruitn_to_litr4n = 0.0;
	nf->fruitn_transfer_from_PLT = 0.0;
	nf->m_fruitn_storage_to_fire = 0.0;
	nf->m_fruitn_storage_to_litr1n = 0.0;
	nf->m_fruitn_storage_to_SNSC = 0.0;
	nf->m_fruitn_transfer_to_fire = 0.0;
	nf->m_fruitn_transfer_to_litr1n = 0.0;
	nf->m_fruitn_transfer_to_SNSC = 0.0;
	nf->m_fruitn_to_fire = 0.0;
	nf->m_fruitn_to_litr1n = 0.0;
	nf->m_fruitn_to_SNSC = 0.0;

	/* softstem simulation - Hidy 2013. */
	nf->softstemn_transfer_to_softstemn = 0.0;
	nf->softstemn_storage_to_softstemn_transfer = 0.0;
	nf->softstemn_storage_to_GRZ = 0.0;
	nf->softstemn_storage_to_HRV = 0.0;
	nf->softstemn_storage_to_PLG = 0.0;
	nf->softstemn_storage_to_MOW = 0.0;
	nf->softstemn_transfer_to_GRZ = 0.0;
	nf->softstemn_transfer_to_HRV = 0.0;
	nf->softstemn_transfer_to_PLG = 0.0;
	nf->softstemn_transfer_to_MOW = 0.0;
	nf->softstemn_to_GRZ = 0.0;
	nf->softstemn_to_HRV = 0.0;
	nf->softstemn_to_PLG = 0.0;
	nf->softstemn_to_MOW = 0.0;
	nf->softstemn_to_litr1n = 0.0;
	nf->softstemn_to_litr2n = 0.0;
	nf->softstemn_to_litr2n = 0.0;
	nf->softstemn_to_litr3n = 0.0;
	nf->softstemn_to_litr4n = 0.0;
	nf->softstemn_transfer_from_PLT = 0.0;
	nf->m_softstemn_storage_to_fire = 0.0;
	nf->m_softstemn_storage_to_litr1n = 0.0;
	nf->m_softstemn_storage_to_SNSC = 0.0;
	nf->m_softstemn_transfer_to_fire = 0.0;
	nf->m_softstemn_transfer_to_litr1n = 0.0;
	nf->m_softstemn_transfer_to_SNSC = 0.0;
	nf->m_softstemn_to_fire = 0.0;
	nf->m_softstemn_to_litr1n = 0.0;
	nf->m_softstemn_to_SNSC = 0.0;


	return (!ok);
}
