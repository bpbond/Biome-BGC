/* 
firstday.c
Initializes the state variables for the first day of a simulation that
is not using a restart file.

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.4
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
#include <math.h>
#include <malloc.h>
#include "bgc_struct.h"     /* structure definitions */
#include "bgc_func.h"       /* function prototypes */
#include "bgc_constants.h"


int firstday(const siteconst_struct* sitec, const epconst_struct* epc, const cinit_struct* cinit,
	epvar_struct* epv, phenarray_struct* phen, cstate_struct* cs, nstate_struct* ns, metvar_struct* metv)
{
	int ok=1;
	int woody;
	int predays,remdays;
	int layer;
	double max_leafc,max_frootc,max_fruitc;
	double max_softstemc =0; /* fruit and softstem simulation - Hidy 2013. */
	double max_stemc,new_stemc;
	double prop_transfer,transfer;
	double prop_litfall;
	
	/* initialize the c and n storage state variables */
	cs->leafc_storage = 0.0;
	cs->frootc_storage = 0.0;
	cs->livestemc_storage = 0.0;
	cs->deadstemc_storage = 0.0;
	cs->livecrootc_storage = 0.0;
	cs->deadcrootc_storage = 0.0;
	cs->gresp_storage = 0.0;
	cs->cpool = 0.0;
	ns->leafn_storage = 0.0;
	ns->frootn_storage = 0.0;
	ns->livestemn_storage = 0.0;
	ns->deadstemn_storage = 0.0;
	ns->livecrootn_storage = 0.0;
	ns->deadcrootn_storage = 0.0;
	ns->retransn = 0.0;
	ns->npool = 0.0;
	/* fruit simulation - Hidy 2013. */
	cs->fruitc_storage = 0.0;
	ns->fruitn_storage = 0.0;
	/* softstem simulation - Hidy 2015. */
	cs->softstemc_storage = 0.0;
	ns->softstemn_storage = 0.0;
	
	/* *****************************************************************************- */
	/* Hidy 2010 - initialize days-since-rain and day-since-waterstress counter */
	epv->dsr = 0.0;
	epv->dsws = 0.0;
	epv->dsws_FULL = 0.0;
	epv->flowering = 0;
	epv->maturity  = 0;

	/* Hidy 2010 - initialize multilayer variables (first approximation: field cap.) and multipliers for stomatal limitation calculation */
	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		epv->vwc[layer]				  = sitec->vwc_fc[layer];
		epv->psi[layer]				  = sitec->psi_fc[layer];
		epv->hydr_conduct_S[layer]	  = sitec->hydr_conduct_fc[layer];
		epv->hydr_diffus_S[layer]	  = sitec->hydr_diffus_fc[layer];
		epv->hydr_conduct_E[layer]	  = sitec->hydr_conduct_fc[layer];
		epv->hydr_diffus_E[layer]	  = sitec->hydr_diffus_fc[layer];
		epv->pF[layer]				  = log10(fabs(10000*sitec->psi_fc[layer]));	// dimension of psi: MPa to cm (10000 MPa = 1 cm)
		epv->m_soilstress_layer[layer]= 1;
	}

	epv->vwc_avg		    = sitec->vwc_fc[0];
	epv->psi_avg		    = sitec->psi_fc[0];
	metv->tsoil_avg	   	    = sitec->tair_annavg;
	metv->tsoil_surface_pre	= sitec->tair_annavg;	
	epv->m_soilstress	    = 1;

	/* *****************************************************************************- */


	woody = epc->woody;
	
	/* establish the initial partitioning between displayed growth
	and growth ready for transfer */
	max_leafc = cinit->max_leafc;
	cs->leafc_transfer = max_leafc * epc->leaf_turnover;
	cs->leafc = max_leafc - cs->leafc_transfer;
	max_frootc = max_leafc * epc->alloc_frootc_leafc;
	cs->frootc_transfer = cinit->max_leafc * epc->alloc_frootc_leafc * 
		epc->froot_turnover;
	cs->frootc = max_frootc - cs->frootc_transfer;
	/* fruit simulation - Hidy 2013. */
	max_fruitc = max_leafc * epc->alloc_fruitc_leafc;
	cs->fruitc_transfer = cinit->max_leafc * epc->alloc_fruitc_leafc * epc->fruit_turnover;
	cs->fruitc = max_fruitc - cs->fruitc_transfer;

    /* TREE-specific and NON-WOODY SPECIFIC fluxes */
	if (epc->woody)
	{	
		max_stemc = cinit->max_stemc;
		new_stemc = cs->leafc_transfer * epc->alloc_newstemc_newleafc;
		cs->livestemc_transfer = new_stemc * epc->alloc_newlivewoodc_newwoodc;
		cs->livestemc = cs->livestemc_transfer / epc->livewood_turnover;
		cs->deadstemc_transfer = new_stemc - cs->livestemc_transfer;
		cs->deadstemc = max_stemc - cs->livestemc_transfer - cs->livestemc - 
			cs->deadstemc_transfer;
		if (cs->deadstemc < 0.0) cs->deadstemc = 0.0;
		cs->livecrootc_transfer = cs->livestemc_transfer * epc->alloc_crootc_stemc;
		cs->livecrootc = cs->livestemc * epc->alloc_crootc_stemc;
		cs->deadcrootc_transfer = cs->deadstemc_transfer * epc->alloc_crootc_stemc;
		cs->deadcrootc = cs->deadstemc * epc->alloc_crootc_stemc;
	} 
	else
	{
		/* SOFT STEM SIMULATION of non-woody biomes - Hidy 2015 */
		max_softstemc = max_leafc * epc->alloc_softstemc_leafc;
		cs->softstemc_transfer = cinit->max_leafc * epc->alloc_softstemc_leafc * epc->softstem_turnover;
		cs->softstemc = max_softstemc - cs->softstemc_transfer;
	}

	/* calculate initial leaf and froot nitrogen pools from carbon pools and
	user-specified initial C:N for each component */
	ns->leafn_transfer = cs->leafc_transfer / epc->leaf_cn;
	ns->leafn = cs->leafc / epc->leaf_cn;
	ns->frootn_transfer = cs->frootc_transfer / epc->froot_cn;
	ns->frootn = cs->frootc / epc->froot_cn;
	/* fruit simulation - Hidy 2013. */
	ns->fruitn_transfer = cs->fruitc_transfer / epc->fruit_cn;
	ns->fruitn = cs->fruitc / epc->fruit_cn;

	/* TREE-specific and NON-WOODY SPECIFIC fluxes */
	if (epc->woody)
	{
		ns->livestemn_transfer = cs->livestemc_transfer / epc->livewood_cn;
		ns->livestemn = cs->livestemc / epc->livewood_cn;
		ns->deadstemn_transfer = cs->deadstemc_transfer / epc->deadwood_cn;
		ns->deadstemn = cs->deadstemc / epc->deadwood_cn;
		ns->livecrootn_transfer = cs->livecrootc_transfer / epc->livewood_cn;
		ns->livecrootn = cs->livecrootc / epc->livewood_cn;
		ns->deadcrootn_transfer = cs->deadcrootc_transfer / epc->deadwood_cn;
		ns->deadcrootn = cs->deadcrootc / epc->deadwood_cn;
	}
	else
	{
		/* SOFT STEM SIMULATION of non-woody biomes - Hidy 2015 */
		ns->softstemn_transfer = cs->softstemc_transfer / epc->softstem_cn;
		ns->softstemn = cs->softstemc / epc->softstem_cn;

	}
	
	/* use then penology array information to determine, for the first
	day of simulation, how many days of transfer and litterfall have
	already occurred for this year */
	predays = phen->predays_transfer[0];
	remdays = phen->remdays_transfer[0];
	if (predays > 0)
	{
		prop_transfer = (double)predays/(double)(predays+remdays);
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
		/* fruit simulation - Hidy 2013. */
		transfer = prop_transfer * cs->fruitc_transfer;
		cs->fruitc          += transfer;
		cs->fruitc_transfer -= transfer;
		transfer = prop_transfer * ns->fruitn_transfer;
		ns->fruitn          += transfer;
		ns->fruitn_transfer -= transfer;

		/* TREE-specific and NON-WOODY SPECIFIC fluxes */
		if (woody)
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
			/* SOFT STEM SIMULATION of non-woody biomes - Hidy 2015 */
			transfer = prop_transfer * cs->softstemc_transfer;
			cs->softstemc          += transfer;
			cs->softstemc_transfer -= transfer;
			transfer = prop_transfer * ns->softstemn_transfer;
			ns->softstemn          += transfer;
			ns->softstemn_transfer -= transfer;

		}
		
		/* only test for litterfall if there has already been some
		transfer growth this year */
		predays = phen->predays_litfall[0];
		remdays = phen->remdays_litfall[0];
		if (predays > 0)
		{
			/* some litterfall has already occurred. in this case, just
			remove material from the displayed compartments, and don't
			bother with the transfer to litter compartments */
			prop_litfall = (double)predays/(double)(predays+remdays);
			cs->leafc  -= prop_litfall * cs->leafc * epc->leaf_turnover;
			cs->frootc -= prop_litfall * cs->frootc * epc->froot_turnover;
			/* fruit simulation - Hidy 2013. */
			cs->fruitc -= prop_litfall * cs->fruitc * epc->fruit_turnover;
			/* softstem simulation - Hidy 2013. */
			cs->softstemc -= prop_litfall * cs->softstemc * epc->softstem_turnover;
		}
	} /* end if transfer */

	/* add the growth respiration requirement for the first year's
	leaf and fine root growth from transfer pools to the 
	gresp_transfer pool */
	cs->gresp_transfer = 0.0;
	cs->gresp_transfer += (cs->leafc_transfer + cs->frootc_transfer + cs->frootc_transfer) * epc->GR_ratio; /* fruit simulation - Hidy 2013. */
	
	/* TREE-specific and NON-WOODY SPECIFIC fluxes */
	if (woody)
	{
		cs->gresp_transfer += (cs->livestemc_transfer + cs->deadstemc_transfer + 
								cs->livecrootc_transfer + cs->deadcrootc_transfer) * epc->GR_ratio;
	}
	else
	{	
		cs->gresp_transfer += (cs->softstemc_transfer) * epc->GR_ratio;
	}
	
	/* set the initial rates of litterfall and live wood turnover */
	if (epc->evergreen)
	{
		/* leaf and fineroot litterfall rates */
		epv->day_leafc_litfall_increment = max_leafc * epc->leaf_turnover / NDAY_OF_YEAR;
		epv->day_frootc_litfall_increment = max_frootc * epc->froot_turnover / NDAY_OF_YEAR;
		/* fruit simulation - Hidy 2013. */
		epv->day_fruitc_litfall_increment = max_fruitc * epc->fruit_turnover / NDAY_OF_YEAR;
		/* softstem simulation - Hidy 2013. */
		epv->day_softstemc_litfall_increment = max_softstemc * epc->softstem_turnover / NDAY_OF_YEAR;
	}
	else
	{
		/* deciduous: reset the litterfall rates to 0.0 for the start of the
		next litterfall season */
		epv->day_leafc_litfall_increment = 0.0;
		epv->day_frootc_litfall_increment = 0.0;
		/* fruit simulation - Hidy 2013. */
		epv->day_fruitc_litfall_increment = 0.0;
		/* softstem simulation - Hidy 2013. */
		epv->day_softstemc_litfall_increment = 0.0;
	}
	/* all types can use annmax leafc and frootc */
	epv->annmax_leafc = 0.0;
	epv->annmax_frootc = 0.0;
	/* fruit simulation - Hidy 2013. */
	epv->annmax_fruitc = 0.0;
	epv->annmax_softstemc = 0.0;
	epv->annmax_livestemc = 0.0;
	epv->annmax_livecrootc = 0.0;

	epv->day_livestemc_turnover_increment = cs->livestemc * epc->livewood_turnover / NDAY_OF_YEAR;
	epv->day_livecrootc_turnover_increment = cs->livecrootc * epc->livewood_turnover / NDAY_OF_YEAR;
	

	
	return (!ok);
}
