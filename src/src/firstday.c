/* 
firstday.c
Initializes the state variables for the first day of a simulation that
is not using a restart file.

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2018, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "bgc_struct.h"     /* structure definitions */
#include "bgc_func.h"       /* function prototypes */
#include "bgc_constants.h"


int firstday(const control_struct* ctrl, const siteconst_struct* sitec, const epconst_struct* epc, const cinit_struct* cinit,
	phenology_struct* phen, epvar_struct* epv, cstate_struct* cs, nstate_struct* ns, metvar_struct* metv)
{
	int ok=1;
	int layer, day, pp;
	double prop_transfer, transfer, prop_litfall;
	
	
	/* *****************************************************************************- */
	/* 1. Initialize ecophysiological variables */

	epv->dsr = 0.0;
	epv->dsws = 0.0;
	epv->transfer_ratio = 0.0;
	epv->leafday = 0;
	epv->leafday_lastmort = 0;
	epv->n_rootlayers = 0;
	epv->n_maxrootlayers = 0;
    epv->proj_lai = 0;
    epv->all_lai = 0;
	epv->sla_avg = 0;
    epv->plaisun = 0;
    epv->plaishade = 0;
    epv->sun_proj_sla = 0;
    epv->shade_proj_sla = 0;
	epv->plant_height = 0;
	epv->n_actphen = 0;
	epv->flowHS_mort = 0;

	epv->plant_calloc = 0; 
	epv->plant_nalloc = 0;
	epv->excess_c = 0;
	epv->pnow = 0;
	epv->NSC_limit_nw = 0;
	epv->NSC_limit_w = 0;
	epv->NDVI = 0;
	epv->rooting_depth = 0;

	/* set the initial rates of litterfall and live wood turnover */
	if (epc->evergreen)
	{
		/* leaf and fineroot litterfall rates */
		epv->day_leafc_litfall_increment = cinit->max_leafc * epc->nonwoody_turnover / NDAYS_OF_YEAR;
		epv->day_frootc_litfall_increment = cinit->max_frootc * epc->nonwoody_turnover / NDAYS_OF_YEAR;
		epv->day_fruitc_litfall_increment = cinit->max_fruitc * epc->nonwoody_turnover / NDAYS_OF_YEAR;
		epv->day_softstemc_litfall_increment = cinit->max_softstemc * epc->nonwoody_turnover / NDAYS_OF_YEAR;
	}
	else
	{
		/* deciduous: reset the litterfall rates to 0.0 for the start of the
		next litterfall season */
		epv->day_leafc_litfall_increment = 0.0;
		epv->day_frootc_litfall_increment = 0.0;
		epv->day_fruitc_litfall_increment = 0.0;
		epv->day_softstemc_litfall_increment = 0.0;
	}

	epv->day_livestemc_turnover_increment = cs->livestemc * epc->woody_turnover / NDAYS_OF_YEAR;
	epv->day_livecrootc_turnover_increment = cs->livecrootc * epc->woody_turnover / NDAYS_OF_YEAR;


	/* initialize multilayer variables (first approximation: field cap.) and multipliers for stomatal limitation calculation */
	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		epv->vwc[layer]				    = sitec->vwc_fc[layer];
		epv->psi[layer]				    = sitec->psi_fc[layer];
		epv->hydr_conduct_S[layer]	    = sitec->hydr_conduct_fc[layer];
		epv->hydr_diffus_S[layer]	    = sitec->hydr_diffus_fc[layer];
		epv->hydr_conduct_E[layer]	    = sitec->hydr_conduct_fc[layer];
		epv->hydr_diffus_E[layer]	    = sitec->hydr_diffus_fc[layer];
		epv->pF[layer]				    = log10(fabs(10000*sitec->psi_fc[layer]));	// dimension of psi: MPa to cm (10000 MPa = 1 cm)
		epv->m_soilstress_layer[layer]  = 1;
	    epv->rootlength_prop[layer]     = 0;
		
	}

	/* initialize genetical senescence variables */
	epv->thermal_timeSUM_max = 0;
	epv->thermal_time        = 0;
	for (day = 0; day < NDAYS_OF_YEAR; day++)
	{
		epv->thermal_timeSUM[day]     = 0;
		epv->cpool_to_leafcARRAY[day] = 0;
		epv->npool_to_leafnARRAY[day] = 0;
	}
	
	epv->vwc_avg		    = sitec->vwc_fc[0];
	epv->psi_avg		    = sitec->psi_fc[0];
	epv->m_soilstress	    = 1;
	epv->SMSI               = 1;
	metv->tsoil_avg	   	    = sitec->tair_annavg;
	metv->tsoil_surface_pre	= sitec->tair_annavg;	
	metv->GDDpre            = 0;

	phen->GDD_emergSTART = 0;
	phen->GDD_emergEND   = 0;
	phen->GDD_limit      = 0;
	phen->yday_phen      = 0;

	/* initialize sum of leafC in a given phenophase */
	for (pp = 0; pp < N_PHENPHASES; pp++) cs->leafcSUM_phenphase[pp] = 0;


	/* *****************************************************************************- */
	/* 3. Initialize the C and N  variables, but NOT in transient phase and NOT in normal phase without reading restart file */


	if (ctrl->read_restart == 0 && ctrl->spinup != 2)
	{
		/* all types can use annmax leafc and frootc */
		epv->annmax_leafc = 0.0;
		epv->annmax_frootc = 0.0;
		epv->annmax_fruitc = 0.0;
		epv->annmax_softstemc = 0.0;
		epv->annmax_livestemc = 0.0;
		epv->annmax_livecrootc = 0.0;

		/* initialize the c and n storage state variables, but NOT in transient phase */
		cs->leafc_storage = 0.0;
		cs->frootc_storage = 0.0;
		cs->fruitc_storage = 0.0;
		cs->softstemc_storage = 0.0;
		cs->livestemc_storage = 0.0;
		cs->deadstemc_storage = 0.0;
		cs->livecrootc_storage = 0.0;
		cs->deadcrootc_storage = 0.0;
		cs->gresp_storage = 0.0;
		cs->cpool = 0.0;
		
		ns->leafn_storage = 0.0;
		ns->frootn_storage = 0.0;
		ns->fruitn_storage = 0.0;
		ns->softstemn_storage = 0.0;
		ns->livestemn_storage = 0.0;
		ns->deadstemn_storage = 0.0;
		ns->livecrootn_storage = 0.0;
		ns->deadcrootn_storage = 0.0;
		ns->npool = 0.0;
	
	
		/* establish the initial partitioning between displayed growth and growth ready for transfer */
		/* calculate initial leaf and froot nitrogen pools from carbon pools and user-specified initial C:N for each component */

		if (epc->leaf_cn)
		{
			cs->leafc_transfer       = cinit->max_leafc      * epc->nonwoody_turnover;
			cs->leafc                = cinit->max_leafc      - cs->leafc_transfer;
			ns->leafn_transfer = cs->leafc_transfer / epc->leaf_cn;
			ns->leafn = cs->leafc / epc->leaf_cn;
		}
 		
		if (epc->froot_cn)
		{
			cs->frootc_transfer       = cinit->max_frootc      * epc->nonwoody_turnover;
			cs->frootc                = cinit->max_frootc      - cs->frootc_transfer;
			ns->frootn_transfer = cs->frootc_transfer / epc->froot_cn;
			ns->frootn = cs->frootc / epc->froot_cn;
		}

		if (epc->fruit_cn)
		{
			cs->fruitc_transfer       = cinit->max_fruitc      * epc->nonwoody_turnover;
			cs->fruitc                = cinit->max_fruitc      - cs->fruitc_transfer;
			ns->fruitn_transfer = cs->fruitc_transfer / epc->fruit_cn;
			ns->fruitn = cs->fruitc / epc->fruit_cn;
		}

		if (epc->softstem_cn)
		{
			cs->softstemc_transfer       = cinit->max_softstemc      * epc->nonwoody_turnover;
			cs->softstemc                = cinit->max_softstemc      - cs->softstemc_transfer;
			ns->softstemn_transfer = cs->softstemc_transfer / epc->softstem_cn;
			ns->softstemn = cs->softstemc / epc->softstem_cn;
		}

		
		if (epc->livewood_cn)
		{
			cs->livestemc_transfer   = cinit->max_livestemc    * epc->woody_turnover;
			cs->livestemc            = cinit->max_livestemc    - cs->livestemc_transfer;
            ns->livestemn_transfer  = cs->livestemc_transfer / epc->livewood_cn;
			ns->livestemn           = cs->livestemc / epc->livewood_cn;

			cs->livecrootc_transfer  = cinit->max_livecrootc   * epc->woody_turnover;
			cs->livecrootc           = cinit->max_livecrootc   - cs->livecrootc_transfer;
			ns->livecrootn_transfer = cs->livecrootc_transfer / epc->livewood_cn;
			ns->livecrootn          = cs->livecrootc / epc->livewood_cn;
		}
	
		if (epc->deadwood_cn)
		{
			if (epc->alloc_livestemc[0]) 
			{
				cs->deadstemc_transfer   = cs->livestemc_transfer  * (epc->alloc_deadstemc[0] / epc->alloc_livestemc[0]);
				cs->deadstemc            = cs->livestemc           * (epc->alloc_deadstemc[0] / epc->alloc_livestemc[0]);
			}
			if (epc->alloc_livecrootc[0])
			{
				cs->deadcrootc_transfer  = cs->livecrootc_transfer * (epc->alloc_deadcrootc[0] / epc->alloc_livecrootc[0]);
				cs->deadcrootc           = cs->livecrootc          * (epc->alloc_deadcrootc[0] / epc->alloc_livecrootc[0]);
			}

			ns->deadstemn_transfer  = cs->deadstemc_transfer / epc->deadwood_cn;
			ns->deadstemn           = cs->deadstemc / epc->deadwood_cn;
			ns->deadcrootn_transfer = cs->deadcrootc_transfer / epc->deadwood_cn;
			ns->deadcrootn          = cs->deadcrootc / epc->deadwood_cn;
		}

		/* *****************************************************************************- */
		/* 4. use then penology array information to determine, for the first day of simulation, how many days of transfer and litterfall have already occurred for this year */
		
		if (phen->predays_transfer > 0)
		{
			prop_transfer = (double)phen->predays_transfer/(double)(phen->predays_transfer+phen->remdays_transfer);
			/* perform these transfers */
			transfer = prop_transfer * cs->leafc_transfer;
			cs->leafc          += transfer;
			cs->leafc_transfer -= transfer;
			transfer = prop_transfer * ns->leafn_transfer;
			ns->leafn          += transfer;
			ns->leafn_transfer -= transfer;
			transfer = prop_transfer * cs->frootc_transfer;
			cs->frootc          += transfer;
			cs->frootc_transfer -= transfer;
			transfer = prop_transfer * ns->frootn_transfer;
			ns->frootn          += transfer;
			ns->frootn_transfer -= transfer;
			/* fruit simulation  */
			transfer = prop_transfer * cs->fruitc_transfer;
			cs->fruitc          += transfer;
			cs->fruitc_transfer -= transfer;
			transfer = prop_transfer * ns->fruitn_transfer;
			ns->fruitn          += transfer;
			ns->fruitn_transfer -= transfer;

			/* TREE-specific and NON-WOODY SPECIFIC fluxes */
			if (epc->woody)
			{
				transfer = prop_transfer * cs->livestemc_transfer;
				cs->livestemc          += transfer;
				cs->livestemc_transfer -= transfer;
				transfer = prop_transfer * ns->livestemn_transfer;
				ns->livestemn          += transfer;
				ns->livestemn_transfer -= transfer;
				transfer = prop_transfer * cs->deadstemc_transfer;
				cs->deadstemc          += transfer;
				cs->deadstemc_transfer -= transfer;
				transfer = prop_transfer * ns->deadstemn_transfer;
				ns->deadstemn          += transfer;
				ns->deadstemn_transfer -= transfer;
				transfer = prop_transfer * cs->livecrootc_transfer;
				cs->livecrootc          += transfer;
				cs->livecrootc_transfer -= transfer;
				transfer = prop_transfer * ns->livecrootn_transfer;
				ns->livecrootn          += transfer;
				ns->livecrootn_transfer -= transfer;
				transfer = prop_transfer * cs->deadcrootc_transfer;
				cs->deadcrootc          += transfer;
				cs->deadcrootc_transfer -= transfer;
				transfer = prop_transfer * ns->deadcrootn_transfer;
				ns->deadcrootn          += transfer;
				ns->deadcrootn_transfer -= transfer;
			}
			else
			{
				/* SOFT STEM SIMULATION of non-woody biomes */
				transfer = prop_transfer * cs->softstemc_transfer;
				cs->softstemc          += transfer;
				cs->softstemc_transfer -= transfer;
				transfer = prop_transfer * ns->softstemn_transfer;
				ns->softstemn          += transfer;
				ns->softstemn_transfer -= transfer;

			}
		
			/* only test for litterfall if there has already been some transfer growth this year */
			if (phen->predays_litfall > 0)
			{
				/* some litterfall has already occurred. in this case, just
				remove material from the displayed compartments, and don't
				bother with the transfer to litter compartments */
				prop_litfall = (double)phen->predays_litfall/(double)(phen->predays_litfall+phen->remdays_litfall);
				cs->leafc  -= prop_litfall * cs->leafc * epc->nonwoody_turnover;
				cs->frootc -= prop_litfall * cs->frootc * epc->nonwoody_turnover;
				cs->fruitc -= prop_litfall * cs->fruitc * epc->nonwoody_turnover;
				cs->softstemc -= prop_litfall * cs->softstemc * epc->nonwoody_turnover;
			}
		} /* end if transfer */
		/* add the growth respiration requirement for the first year's leaf and fine root growth from transfer pools to the gresp_transfer pool */
		cs->gresp_transfer = 0.0;
		cs->gresp_transfer += (cs->leafc_transfer + cs->frootc_transfer + cs->fruitc_transfer + cs->softstemc_transfer +
			                   cs->livestemc_transfer + cs->deadstemc_transfer + cs->livecrootc_transfer + cs->deadcrootc_transfer) * epc->GR_ratio; 
	

	}
	


	
	return (!ok);
}
