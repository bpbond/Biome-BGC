/* 
mortality.c
daily mortality fluxes

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2018, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

Updated:
6 March 2000 (PET): Appears from tests with P.Pine data that combustion of deadstem should be higher: setting to 20% for testing.
9 February 1999, PET: Changed the treatment of fire mortality, so that instead of the entire deadstem and cwd pools being subject to the mortality rates,
only 5% of deadstem and 30% of cwd are subject to fire losses. Mike White found that there was a big difference between literature estimates of fire
emissions and model estimates for the boreal forest, and this is also a topic that Bob Keane had pointed out a while ago as a first-order improvement
to the fire treatment. Got the values for consumption efficiency from Agee, James K., 1993. Fire Ecology of Pacific Northwest Forests. Island Press, 
Washington, D.C. p 42.
The deadstem material that is not wilted (95%) is sent to CWD pools. CWD that is not burned (70%) stays in CWD pools.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "bgc_struct.h"
#include "bgc_func.h"
#include "bgc_constants.h"

int mortality(const control_struct* ctrl, const epconst_struct* epc, const epvar_struct* epv, 
	          cstate_struct* cs, cflux_struct* cf, nstate_struct* ns, nflux_struct* nf, int simyr)
{
	int ok=1;
	int layer;
	double mort, flux_from_carbon;

	
	/******************************************************************/
	/* I. Non-fire mortality: these fluxes all enter litter or CWD pools */
	/******************************************************************/


	/* 1. if no changing data constant EPC parameter are used - ATTENTION: WPM = daily_mortality_turnover * NDAYS_OF_YEAR */
	if (ctrl->varWPM_flag && ctrl->spinup != 1)
	{
		mort = epc->wpm_array[simyr]/NDAYS_OF_YEAR;
	}
	else
	{
		mort = epc->daily_mortality_turnover;	
	}

	
	/* 2. daily fluxes due to mortality */

	/* in order to save the C:N ratio: N-fluxes are calculated from C-fluxes using C:N ratio parameters */
	if (epc->leaf_cn)
	{
		cf->m_leafc_to_litr1c  = mort * cs->leafc * epc->leaflitr_flab;  	 
		cf->m_leafc_to_litr2c  = mort * cs->leafc * epc->leaflitr_fucel;
		cf->m_leafc_to_litr3c  = mort * cs->leafc * epc->leaflitr_fscel;  	 
		cf->m_leafc_to_litr4c  = mort * cs->leafc * epc->leaflitr_flig;  	 

		nf->m_leafn_to_litr1n  = cf->m_leafc_to_litr1c / epc->leaf_cn;  	
		nf->m_leafn_to_litr2n  = cf->m_leafc_to_litr2c / epc->leaf_cn;  	
		nf->m_leafn_to_litr3n  = cf->m_leafc_to_litr3c / epc->leaf_cn;  	
		nf->m_leafn_to_litr4n  = cf->m_leafc_to_litr4c / epc->leaf_cn;  

		cf->m_leafc_storage_to_litr1c	= mort * cs->leafc_storage;
		cf->m_leafc_transfer_to_litr1c  = mort * cs->leafc_transfer;
		
		nf->m_leafn_storage_to_litr1n	= cf->m_leafc_storage_to_litr1c / epc->leaf_cn;
		nf->m_leafn_transfer_to_litr1n	= cf->m_leafc_transfer_to_litr1c / epc->leaf_cn;
	
	}
	
	if (epc->froot_cn)
	{
		cf->m_frootc_to_litr1c = mort * cs->frootc * epc->frootlitr_flab; 	 
		cf->m_frootc_to_litr2c = mort * cs->frootc * epc->frootlitr_fucel;
		cf->m_frootc_to_litr3c = mort * cs->frootc * epc->frootlitr_fscel; 	 
		cf->m_frootc_to_litr4c = mort * cs->frootc * epc->frootlitr_flig;

		nf->m_frootn_to_litr1n = cf->m_frootc_to_litr1c / epc->froot_cn; 	
		nf->m_frootn_to_litr2n = cf->m_frootc_to_litr2c / epc->froot_cn; 	
		nf->m_frootn_to_litr3n = cf->m_frootc_to_litr3c / epc->froot_cn; 	
		nf->m_frootn_to_litr4n = cf->m_frootc_to_litr4c / epc->froot_cn;

		cf->m_frootc_storage_to_litr1c	= mort * cs->frootc_storage;
		cf->m_frootc_transfer_to_litr1c	= mort * cs->frootc_transfer;
		nf->m_frootn_storage_to_litr1n	= cf->m_frootc_storage_to_litr1c / epc->froot_cn;
		nf->m_frootn_transfer_to_litr1n	= cf->m_frootc_transfer_to_litr1c / epc->froot_cn;

	}
	
	if (epc->fruit_cn)
	{
		cf->m_fruitc_to_litr1c = mort * cs->fruitc * epc->fruitlitr_flab;  	 
		cf->m_fruitc_to_litr2c = mort * cs->fruitc * epc->fruitlitr_fucel;
		cf->m_fruitc_to_litr3c = mort * cs->fruitc * epc->fruitlitr_fscel;  	 
		cf->m_fruitc_to_litr4c = mort * cs->fruitc * epc->fruitlitr_flig;  	

		nf->m_fruitn_to_litr1n = cf->m_fruitc_to_litr1c / epc->fruit_cn;  	
		nf->m_fruitn_to_litr2n = cf->m_fruitc_to_litr2c / epc->fruit_cn;  	
		nf->m_fruitn_to_litr3n = cf->m_fruitc_to_litr3c / epc->fruit_cn;  	
		nf->m_fruitn_to_litr4n = cf->m_fruitc_to_litr4c / epc->fruit_cn;  	

		cf->m_fruitc_storage_to_litr1c	= mort * cs->fruitc_storage;
		cf->m_fruitc_transfer_to_litr1c	= mort * cs->fruitc_transfer;
		
		nf->m_fruitn_storage_to_litr1n	= cf->m_fruitc_storage_to_litr1c / epc->fruit_cn;
		nf->m_fruitn_transfer_to_litr1n	= cf->m_fruitc_transfer_to_litr1c / epc->fruit_cn;
	
	}

	if (epc->softstem_cn)
	{
		cf->m_softstemc_to_litr1c = mort * cs->softstemc * epc->softstemlitr_flab;  	 
		cf->m_softstemc_to_litr2c = mort * cs->softstemc * epc->softstemlitr_fucel;
		cf->m_softstemc_to_litr3c = mort * cs->softstemc * epc->softstemlitr_fscel;  	 
		cf->m_softstemc_to_litr4c = mort * cs->softstemc * epc->softstemlitr_flig;  	

		nf->m_softstemn_to_litr1n = cf->m_softstemc_to_litr1c / epc->softstem_cn;  	
		nf->m_softstemn_to_litr2n = cf->m_softstemc_to_litr2c / epc->softstem_cn;  	
		nf->m_softstemn_to_litr3n = cf->m_softstemc_to_litr3c / epc->softstem_cn;  	
		nf->m_softstemn_to_litr4n = cf->m_softstemc_to_litr4c / epc->softstem_cn;  	

		cf->m_softstemc_storage_to_litr1c	= mort * cs->softstemc_storage;
		cf->m_softstemc_transfer_to_litr1c	= mort * cs->softstemc_transfer;
		
		nf->m_softstemn_storage_to_litr1n	= cf->m_softstemc_storage_to_litr1c / epc->softstem_cn;
		nf->m_softstemn_transfer_to_litr1n	= cf->m_softstemc_transfer_to_litr1c / epc->softstem_cn;
	
	}
	
	cf->m_gresp_storage_to_litr1c		= mort * cs->gresp_storage;
	cf->m_gresp_transfer_to_litr1c		= mort * cs->gresp_transfer;	

	nf->m_retransn_to_litr1n			= mort * ns->retransn;


	/* livewood */
	if (epc->livewood_cn)
	{
		cf->m_livestemc_to_cwdc    = mort * cs->livestemc;	 
		cf->m_livecrootc_to_cwdc   = mort * cs->livecrootc;   

		nf->m_livestemn_to_cwdn    = cf->m_livestemc_to_cwdc  / epc->livewood_cn;	
		nf->m_livecrootn_to_cwdn   = cf->m_livecrootc_to_cwdc / epc->livewood_cn;  

		cf->m_livestemc_storage_to_litr1c	= mort * cs->livestemc_storage;
		cf->m_livestemc_transfer_to_litr1c	= mort * cs->livestemc_transfer;
		
		nf->m_livestemn_storage_to_litr1n	= cf->m_livestemc_storage_to_litr1c   / epc->livewood_cn;
		nf->m_livestemn_transfer_to_litr1n	= cf->m_livestemc_transfer_to_litr1c  / epc->livewood_cn;

		cf->m_livecrootc_storage_to_litr1c  = mort * cs->livecrootc_storage;
		cf->m_livecrootc_transfer_to_litr1c = mort * cs->livecrootc_transfer;
		
		nf->m_livecrootn_storage_to_litr1n  = cf->m_livecrootc_storage_to_litr1c  / epc->livewood_cn;
		nf->m_livecrootn_transfer_to_litr1n = cf->m_livecrootc_transfer_to_litr1c / epc->livewood_cn;

		nf->m_livestemn_to_litr1n  = (mort * ns->livestemn) - nf->m_livestemn_to_cwdn;
		nf->m_livecrootn_to_litr1n = (mort * ns->livecrootn) - nf->m_livecrootn_to_cwdn;
			
	}

	/* deadwood */
	if (epc->deadwood_cn)
	{
		cf->m_deadstemc_to_cwdc    = mort * cs->deadstemc;	 
		cf->m_deadcrootc_to_cwdc   = mort * cs->deadcrootc;   

		nf->m_deadstemn_to_cwdn    = cf->m_deadstemc_to_cwdc  / epc->deadwood_cn;	
		nf->m_deadcrootn_to_cwdn   = cf->m_deadcrootc_to_cwdc / epc->deadwood_cn;  

		cf->m_deadstemc_storage_to_litr1c	= mort * cs->deadstemc_storage;
		cf->m_deadstemc_transfer_to_litr1c	= mort * cs->deadstemc_transfer;
		
		nf->m_deadstemn_storage_to_litr1n	= cf->m_deadstemc_storage_to_litr1c   / epc->deadwood_cn;
		nf->m_deadstemn_transfer_to_litr1n	= cf->m_deadstemc_transfer_to_litr1c  / epc->deadwood_cn;

		cf->m_deadcrootc_storage_to_litr1c  = mort * cs->deadcrootc_storage;
		cf->m_deadcrootc_transfer_to_litr1c = mort * cs->deadcrootc_transfer;
		
		nf->m_deadcrootn_storage_to_litr1n  = cf->m_deadcrootc_storage_to_litr1c  / epc->deadwood_cn;
		nf->m_deadcrootn_transfer_to_litr1n = cf->m_deadcrootc_transfer_to_litr1c / epc->deadwood_cn;
			
	}
	
	
	
	/* 3. UPDATE STATE VARIALBES */

	/* Mortality is taken care of last and given special treatment for state update so that it doesn't interfere
	with the other fluxes that are based on proportions of state variables, especially the phenological fluxes */

	/* 3.1 ABOVEGROUND BIOMASS  - into the first layer in multilayer soil */
	cs->leafc       -= (cf->m_leafc_to_litr1c     + cf->m_leafc_to_litr2c     + cf->m_leafc_to_litr3c     + cf->m_leafc_to_litr4c);
	cs->fruitc      -= (cf->m_fruitc_to_litr1c    + cf->m_fruitc_to_litr2c    + cf->m_fruitc_to_litr3c    + cf->m_fruitc_to_litr4c);
	cs->softstemc   -= (cf->m_softstemc_to_litr1c + cf->m_softstemc_to_litr2c + cf->m_softstemc_to_litr3c + cf->m_softstemc_to_litr4c);
	cs->livestemc	-= cf->m_livestemc_to_cwdc;
	cs->deadstemc	-= cf->m_deadstemc_to_cwdc;

	cs->litr1c[0]   += (cf->m_leafc_to_litr1c     + cf->m_fruitc_to_litr1c    + cf->m_softstemc_to_litr1c);
	cs->litr2c[0]   += (cf->m_leafc_to_litr2c     + cf->m_fruitc_to_litr2c    + cf->m_softstemc_to_litr2c);
	cs->litr3c[0]   += (cf->m_leafc_to_litr3c     + cf->m_fruitc_to_litr3c    + cf->m_softstemc_to_litr3c);
	cs->litr4c[0]   += (cf->m_leafc_to_litr4c     + cf->m_fruitc_to_litr4c    + cf->m_softstemc_to_litr4c);
	cs->cwdc[0]     += (cf->m_livestemc_to_cwdc   + cf->m_deadstemc_to_cwdc);
	
	ns->leafn       -= (nf->m_leafn_to_litr1n     + nf->m_leafn_to_litr2n     + nf->m_leafn_to_litr3n     + nf->m_leafn_to_litr4n);
	ns->fruitn      -= (nf->m_fruitn_to_litr1n    + nf->m_fruitn_to_litr2n    + nf->m_fruitn_to_litr3n    + nf->m_fruitn_to_litr4n);
	ns->softstemn   -= (nf->m_softstemn_to_litr1n + nf->m_softstemn_to_litr2n + nf->m_softstemn_to_litr3n + nf->m_softstemn_to_litr4n);
	ns->livestemn	-= nf->m_livestemn_to_cwdn;
	ns->deadstemn	-= nf->m_deadstemn_to_cwdn;

	ns->litr1n[0]   += (nf->m_leafn_to_litr1n     + nf->m_fruitn_to_litr1n    + nf->m_softstemn_to_litr1n);
	ns->litr2n[0]   += (nf->m_leafn_to_litr2n     + nf->m_fruitn_to_litr2n    + nf->m_softstemn_to_litr2n);
	ns->litr3n[0]   += (nf->m_leafn_to_litr3n     + nf->m_fruitn_to_litr3n    + nf->m_softstemn_to_litr3n);
	ns->litr4n[0]   += (nf->m_leafn_to_litr4n     + nf->m_fruitn_to_litr4n    + nf->m_softstemn_to_litr4n);
	ns->cwdn[0]     += (nf->m_livestemn_to_cwdn   + nf->m_deadstemn_to_cwdn);
	
	/* special N-flux: live woody biomass to litter  */
	ns->livestemn  -= nf->m_livestemn_to_litr1n;
	ns->litr1n[0]  += nf->m_livestemn_to_litr1n;

	ns->livecrootn -= nf->m_livecrootn_to_litr1n;
	ns->litr1n[0]  += nf->m_livecrootn_to_litr1n;

	
	/* 3.2 BELOWGROUND BIOMASS  - DIVIDED BETWEEN THE DIFFERENT SOIL LAYERS in multilayer soil  */
	
	cs->frootc      -= (cf->m_frootc_to_litr1c    + cf->m_frootc_to_litr2c    + cf->m_frootc_to_litr3c    + cf->m_frootc_to_litr4c);
	cs->livecrootc	-=  cf->m_livecrootc_to_cwdc;
	cs->deadcrootc	-=  cf->m_deadcrootc_to_cwdc;

	cs->leafc_storage       -= cf->m_leafc_storage_to_litr1c;
	cs->frootc_storage      -= cf->m_frootc_storage_to_litr1c;
	cs->fruitc_storage      -= cf->m_fruitc_storage_to_litr1c;
	cs->softstemc_storage   -= cf->m_softstemc_storage_to_litr1c;
	cs->livestemc_storage   -= cf->m_livestemc_storage_to_litr1c;
	cs->deadstemc_storage   -= cf->m_deadstemc_storage_to_litr1c;
	cs->livecrootc_storage  -= cf->m_livecrootc_storage_to_litr1c;
	cs->deadcrootc_storage  -= cf->m_deadcrootc_storage_to_litr1c;
	
	cs->leafc_transfer      -= cf->m_leafc_transfer_to_litr1c;
	cs->frootc_transfer     -= cf->m_frootc_transfer_to_litr1c;
	cs->fruitc_transfer     -= cf->m_fruitc_transfer_to_litr1c;
	cs->softstemc_transfer  -= cf->m_softstemc_transfer_to_litr1c;
	cs->livestemc_transfer  -= cf->m_livestemc_transfer_to_litr1c;
	cs->deadstemc_transfer  -= cf->m_deadstemc_transfer_to_litr1c;
	cs->livecrootc_transfer -= cf->m_livecrootc_transfer_to_litr1c;
	cs->deadcrootc_transfer -= cf->m_deadcrootc_transfer_to_litr1c;
	
	cs->gresp_storage       -= cf->m_gresp_storage_to_litr1c;
	cs->gresp_transfer      -= cf->m_gresp_transfer_to_litr1c;
	

	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		cs->litr1c[layer]  += (cf->m_frootc_to_litr1c + 
			                   cf->m_leafc_storage_to_litr1c      + cf->m_frootc_storage_to_litr1c     + cf->m_fruitc_storage_to_litr1c      + cf->m_softstemc_storage_to_litr1c +
							   cf->m_livestemc_storage_to_litr1c  + cf->m_deadstemc_storage_to_litr1c  + cf->m_livecrootc_storage_to_litr1c  + cf->m_deadcrootc_storage_to_litr1c + 
							   cf->m_leafc_transfer_to_litr1c     + cf->m_frootc_transfer_to_litr1c    + cf->m_fruitc_transfer_to_litr1c     + cf->m_softstemc_transfer_to_litr1c +
							   cf->m_livestemc_transfer_to_litr1c + cf->m_deadstemc_transfer_to_litr1c + cf->m_livecrootc_transfer_to_litr1c + cf->m_deadcrootc_transfer_to_litr1c +
							   cf->m_gresp_storage_to_litr1c      + cf->                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           m_gresp_transfer_to_litr1c) * epv->rootlength_prop[layer];	
		
		cs->litr2c[layer]  += (cf->m_frootc_to_litr2c) * epv->rootlength_prop[layer];	
		
		cs->litr3c[layer]  += (cf->m_frootc_to_litr3c) * epv->rootlength_prop[layer];	
		
		cs->litr4c[layer]  += (cf->m_frootc_to_litr4c) * epv->rootlength_prop[layer];	

		cs->cwdc[layer]    += (cf->m_livecrootc_to_cwdc + cf->m_deadcrootc_to_cwdc) * epv->rootlength_prop[layer];
		
	}
		
	ns->frootn     -= (nf->m_frootn_to_litr1n    + nf->m_frootn_to_litr2n    + nf->m_frootn_to_litr3n    + nf->m_frootn_to_litr4n);
	ns->livecrootn -=  nf->m_livecrootn_to_cwdn;
	ns->deadcrootn -=  nf->m_deadcrootn_to_cwdn;

	ns->leafn_storage       -= nf->m_leafn_storage_to_litr1n;
	ns->frootn_storage      -= nf->m_frootn_storage_to_litr1n;
	ns->fruitn_storage      -= nf->m_fruitn_storage_to_litr1n;
	ns->softstemn_storage   -= nf->m_softstemn_storage_to_litr1n;
	ns->livestemn_storage   -= nf->m_livestemn_storage_to_litr1n;
	ns->deadstemn_storage   -= nf->m_deadstemn_storage_to_litr1n;
	ns->livecrootn_storage  -= nf->m_livecrootn_storage_to_litr1n;
	ns->deadcrootn_storage  -= nf->m_deadcrootn_storage_to_litr1n;
	
	ns->leafn_transfer      -= nf->m_leafn_transfer_to_litr1n;
	ns->frootn_transfer     -= nf->m_frootn_transfer_to_litr1n;
	ns->fruitn_transfer     -= nf->m_fruitn_transfer_to_litr1n;
	ns->softstemn_transfer  -= nf->m_softstemn_transfer_to_litr1n;
	ns->livestemn_transfer  -= nf->m_livestemn_transfer_to_litr1n;
	ns->deadstemn_transfer  -= nf->m_deadstemn_transfer_to_litr1n;
	ns->livecrootn_transfer -= nf->m_livecrootn_transfer_to_litr1n;
	ns->deadcrootn_transfer -= nf->m_deadcrootn_transfer_to_litr1n;
	
	ns->retransn       -= nf->m_retransn_to_litr1n;

	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		ns->litr1n[layer]  += (nf->m_frootn_to_litr1n + 
			                   nf->m_leafn_storage_to_litr1n      + nf->m_frootn_storage_to_litr1n     + nf->m_fruitn_storage_to_litr1n      + nf->m_softstemn_storage_to_litr1n  + 
							   nf->m_livestemn_storage_to_litr1n  + nf->m_deadstemn_storage_to_litr1n  + nf->m_livecrootn_storage_to_litr1n  + nf->m_deadcrootn_storage_to_litr1n + 
							   nf->m_leafn_transfer_to_litr1n     + nf->m_frootn_transfer_to_litr1n    + nf->m_fruitn_transfer_to_litr1n     + nf->m_softstemn_transfer_to_litr1n + 
							   nf->m_livestemn_transfer_to_litr1n + nf->m_deadstemn_transfer_to_litr1n + nf->m_livecrootn_transfer_to_litr1n + nf->m_deadcrootn_transfer_to_litr1n +
							   nf->m_retransn_to_litr1n) * epv->rootlength_prop[layer];	
		
		ns->litr2n[layer]  += (nf->m_frootn_to_litr2n) * epv->rootlength_prop[layer];	
		
		ns->litr3n[layer]  += (nf->m_frootn_to_litr3n) * epv->rootlength_prop[layer];	
		
		ns->litr4n[layer]  += (nf->m_frootn_to_litr4n) * epv->rootlength_prop[layer];	

		ns->cwdn[layer]    += (nf->m_livecrootn_to_cwdn + nf->m_deadcrootn_to_cwdn) * epv->rootlength_prop[layer];
		
	}

	
	/************************************************************/
	/* II. Fire mortality: these fluxes all enter atmospheric sinks */
	/************************************************************/

	
	mort = epc->daily_fire_turnover;
	
	/* 1. Daily fluxes due to mortality */

	/* 1.1 non-woody pools */
	if (epc->leaf_cn)
	{
		cf->m_leafc_to_fire          = mort * cs->leafc; 
		cf->m_leafc_storage_to_fire  = mort * cs->leafc_storage;
		cf->m_leafc_transfer_to_fire = mort * cs->leafc_transfer;
		
		nf->m_leafn_to_fire	          = cf->m_leafc_to_fire          / epc->leaf_cn;  
		nf->m_leafn_storage_to_fire	  = cf->m_leafc_storage_to_fire  / epc->leaf_cn; 
		nf->m_leafn_transfer_to_fire  = cf->m_leafc_transfer_to_fire / epc->leaf_cn; 
	}
	
	if (epc->froot_cn)
	{
		cf->m_frootc_to_fire          = mort * cs->frootc; 
		cf->m_frootc_storage_to_fire  = mort * cs->frootc_storage;
		cf->m_frootc_transfer_to_fire = mort * cs->frootc_transfer;
		
		nf->m_frootn_to_fire	       = cf->m_frootc_to_fire          / epc->froot_cn;  
		nf->m_frootn_storage_to_fire   = cf->m_frootc_storage_to_fire  / epc->froot_cn; 
		nf->m_frootn_transfer_to_fire  = cf->m_frootc_transfer_to_fire / epc->froot_cn; 
	}

	if (epc->fruit_cn)
	{
		cf->m_fruitc_to_fire          = mort * cs->fruitc; 
		cf->m_fruitc_storage_to_fire  = mort * cs->fruitc_storage;
		cf->m_fruitc_transfer_to_fire = mort * cs->fruitc_transfer;
		
		nf->m_fruitn_to_fire	      = cf->m_fruitc_to_fire          / epc->fruit_cn;  
		nf->m_fruitn_storage_to_fire  = cf->m_fruitc_storage_to_fire  / epc->fruit_cn; 
		nf->m_fruitn_transfer_to_fire = cf->m_fruitc_transfer_to_fire / epc->fruit_cn; 
	}

	if (epc->softstem_cn)
	{
		cf->m_softstemc_to_fire          = mort * cs->softstemc; 
		cf->m_softstemc_storage_to_fire  = mort * cs->softstemc_storage;
		cf->m_softstemc_transfer_to_fire = mort * cs->softstemc_transfer;
		
		nf->m_softstemn_to_fire	          = cf->m_softstemc_to_fire          / epc->softstem_cn;  
		nf->m_softstemn_storage_to_fire	  = cf->m_softstemc_storage_to_fire  / epc->softstem_cn; 
		nf->m_softstemn_transfer_to_fire  = cf->m_softstemc_transfer_to_fire / epc->softstem_cn; 
	}

	/* 1.2 woody pools */
	if (epc->livewood_cn)
	{
		cf->m_livestemc_to_fire           = mort * cs->livestemc;	
		cf->m_livestemc_storage_to_fire   = mort * cs->livestemc_storage;	
		cf->m_livestemc_transfer_to_fire  = mort * cs->livestemc_transfer;	 
		cf->m_livecrootc_to_fire          = mort * cs->livecrootc;   
		cf->m_livecrootc_storage_to_fire  = mort * cs->livecrootc_storage;	
		cf->m_livecrootc_transfer_to_fire = mort * cs->livecrootc_transfer;	 

		nf->m_livestemn_to_fire           = cf->m_livestemc_to_fire           / epc->livewood_cn;	
		nf->m_livestemn_storage_to_fire   = cf->m_livestemc_storage_to_fire   / epc->livewood_cn;		
		nf->m_livestemn_transfer_to_fire  = cf->m_livestemc_transfer_to_fire  / epc->livewood_cn;		 
		nf->m_livecrootn_to_fire          = cf->m_livecrootc_to_fire          / epc->livewood_cn;	   
		nf->m_livecrootn_storage_to_fire  = cf->m_livecrootc_storage_to_fire  / epc->livewood_cn;		
		nf->m_livecrootn_transfer_to_fire = cf->m_livecrootc_transfer_to_fire / epc->livewood_cn;		 
	}

	if (epc->deadwood_cn)
	{
		cf->m_deadstemc_to_fire           = mort * cs->deadstemc;	
		cf->m_deadstemc_storage_to_fire   = mort * cs->deadstemc_storage;	
		cf->m_deadstemc_transfer_to_fire  = mort * cs->deadstemc_transfer;	 
		cf->m_deadcrootc_to_fire          = mort * cs->deadcrootc;   
		cf->m_deadcrootc_storage_to_fire  = mort * cs->deadcrootc_storage;	
		cf->m_deadcrootc_transfer_to_fire = mort * cs->deadcrootc_transfer;	 

		nf->m_deadstemn_to_fire           = cf->m_deadstemc_to_fire           / epc->deadwood_cn;	
		nf->m_deadstemn_storage_to_fire   = cf->m_deadstemc_storage_to_fire   / epc->deadwood_cn;		
		nf->m_deadstemn_transfer_to_fire  = cf->m_deadstemc_transfer_to_fire  / epc->deadwood_cn;		 
		nf->m_deadcrootn_to_fire          = cf->m_deadcrootc_to_fire          / epc->deadwood_cn;	   
		nf->m_deadcrootn_storage_to_fire  = cf->m_deadcrootc_storage_to_fire  / epc->deadwood_cn;		
		nf->m_deadcrootn_transfer_to_fire = cf->m_deadcrootc_transfer_to_fire / epc->deadwood_cn;		 
	}
	
	
	/* 1.3 transfer pools */
	cf->m_gresp_storage_to_fire     = mort * cs->gresp_storage;
	cf->m_gresp_transfer_to_fire    = mort * cs->gresp_transfer;
		
	nf->m_retransn_to_fire			= mort * ns->retransn;


	/* 1.4 standing and cut-down dead biomass simulation */
	cf->m_STDBc_to_fire = mort * (cs->STDBc_leaf + cs->STDBc_froot + cs->STDBc_fruit + cs->STDBc_softstem + cs->STDBc_transfer);
	cf->m_CTDBc_to_fire = mort * (cs->CTDBc_leaf + cs->CTDBc_froot + cs->CTDBc_fruit + cs->CTDBc_softstem + cs->CTDBc_transfer + cs->CTDBc_cstem + cs->CTDBc_croot);
	
	nf->m_STDBn_to_fire = mort * (ns->STDBn_leaf + ns->STDBn_froot + ns->STDBn_fruit + ns->STDBn_softstem + ns->STDBn_transfer);
	nf->m_CTDBn_to_fire = mort * (ns->CTDBn_leaf + ns->CTDBn_froot + ns->CTDBn_fruit + ns->CTDBn_softstem + ns->CTDBn_transfer + ns->CTDBn_cstem + ns->CTDBn_croot);
	
	
	
	/* 1.5 litter and CWD fire fluxes: fire affects only the top layer */
	cf->m_litr1c_to_fire = mort * cs->litr1c[0];
	cf->m_litr2c_to_fire = mort * cs->litr2c[0];
	cf->m_litr3c_to_fire = mort * cs->litr3c[0];
	cf->m_litr4c_to_fire = mort * cs->litr4c[0];
	cf->m_cwdc_to_fire	 = mort * cs->cwdc[0];
	

	nf->m_litr1n_to_fire = mort * ns->litr1n[0];
	flux_from_carbon = cf->m_litr1c_to_fire * (cs->litr1c[0] / ns->litr1n[0]);
	if (nf->m_litr1n_to_fire > flux_from_carbon) nf->m_litr1n_to_fire = flux_from_carbon;

	nf->m_litr2n_to_fire = mort * ns->litr2n[0];
	flux_from_carbon = cf->m_litr2c_to_fire * (cs->litr2c[0] / ns->litr2n[0]);
	if (nf->m_litr2n_to_fire > flux_from_carbon) nf->m_litr2n_to_fire = flux_from_carbon;
		
	nf->m_litr3n_to_fire = mort * ns->litr3n[0];
	flux_from_carbon = cf->m_litr3c_to_fire * (cs->litr3c[0] / ns->litr3n[0]);
	if (nf->m_litr3n_to_fire > flux_from_carbon) nf->m_litr3n_to_fire = flux_from_carbon;
		
	nf->m_litr4n_to_fire = mort * ns->litr4n[0];
	flux_from_carbon = cf->m_litr4c_to_fire * (cs->litr4c[0] / ns->litr4n[0]);
	if (nf->m_litr4n_to_fire > flux_from_carbon) nf->m_litr4n_to_fire = flux_from_carbon;

	nf->m_cwdn_to_fire = mort * ns->cwdn[0];
	flux_from_carbon = cf->m_cwdc_to_fire * (cs->cwdc[0] / ns->cwdn[0]);
	if (nf->m_cwdn_to_fire > flux_from_carbon) nf->m_cwdn_to_fire = flux_from_carbon;

	
	/* 2. update state variables for fire fluxes */
	/* this is the only place other than daily_state_update() routines wherestate variables get changed.  Mortality is taken care of last and  given special treatment for state update so that it doesn't interfere
	with the other fluxes that are based on proportions of state variables, especially the phenological fluxes */
	
	/* 2.1 ABOVEGROUND variables */	
	
	cs->leafc      -= cf->m_leafc_to_fire;
	cs->fruitc     -= cf->m_fruitc_to_fire;
	cs->softstemc  -= cf->m_softstemc_to_fire;
	cs->livestemc  -= cf->m_livestemc_to_fire;
	cs->deadstemc  -= cf->m_deadstemc_to_fire;

	cs->fire_snk       += cf->m_leafc_to_fire + cf->m_fruitc_to_fire + cf->m_softstemc_to_fire + cf->m_livestemc_to_fire + cf->m_deadstemc_to_fire;

	ns->leafn      -= nf->m_leafn_to_fire;
	ns->fruitn     -= nf->m_fruitn_to_fire;
	ns->softstemn  -= nf->m_softstemn_to_fire;
	ns->livestemn  -= nf->m_livestemn_to_fire;
	ns->deadstemn  -= nf->m_deadstemn_to_fire;

	ns->fire_snk       += nf->m_leafn_to_fire + nf->m_fruitn_to_fire + nf->m_softstemn_to_fire + nf->m_livestemn_to_fire + nf->m_deadstemn_to_fire;
	

	/* 2.2 BELOWGROUND variables */	
		
	cs->frootc     -= cf->m_frootc_to_fire;
	cs->livecrootc -= cf->m_livecrootc_to_fire;
	cs->deadcrootc -= cf->m_deadcrootc_to_fire;

	cs->leafc_storage       -= cf->m_leafc_storage_to_fire;
	cs->frootc_storage      -= cf->m_frootc_storage_to_fire;
	cs->fruitc_storage      -= cf->m_fruitc_storage_to_fire;
	cs->softstemc_storage   -= cf->m_softstemc_storage_to_fire;
	cs->livestemc_storage   -= cf->m_livestemc_storage_to_fire;
	cs->deadstemc_storage   -= cf->m_deadstemc_storage_to_fire;
	cs->livecrootc_storage  -= cf->m_livecrootc_storage_to_fire;
	cs->deadcrootc_storage  -= cf->m_deadcrootc_storage_to_fire;

	cs->leafc_transfer      -= cf->m_leafc_transfer_to_fire;
	cs->frootc_transfer     -= cf->m_frootc_transfer_to_fire;
	cs->fruitc_transfer     -= cf->m_fruitc_transfer_to_fire;
	cs->softstemc_transfer  -= cf->m_softstemc_transfer_to_fire;
	cs->livestemc_transfer  -= cf->m_livestemc_transfer_to_fire;
	cs->deadstemc_transfer  -= cf->m_deadstemc_transfer_to_fire;
	cs->livecrootc_transfer -= cf->m_livecrootc_transfer_to_fire;
	cs->deadcrootc_transfer -= cf->m_deadcrootc_transfer_to_fire;
	cs->gresp_storage       -= cf->m_gresp_storage_to_fire;
	cs->gresp_transfer      -= cf->m_gresp_transfer_to_fire;
	
	cs->fire_snk   += cf->m_frootc_to_fire + cf->m_livecrootc_to_fire + cf->m_deadcrootc_to_fire + 
		              cf->m_leafc_storage_to_fire + cf->m_frootc_storage_to_fire + cf->m_fruitc_storage_to_fire + cf->m_softstemc_storage_to_fire + 
		              cf->m_livestemc_storage_to_fire + cf->m_deadstemc_storage_to_fire + cf->m_livecrootc_storage_to_fire + cf->m_deadcrootc_storage_to_fire + 
		              cf->m_leafc_transfer_to_fire + cf->m_frootc_transfer_to_fire + cf->m_fruitc_transfer_to_fire + cf->m_softstemc_transfer_to_fire + 
		              cf->m_livestemc_transfer_to_fire + cf->m_deadstemc_transfer_to_fire + cf->m_livecrootc_transfer_to_fire + cf->m_deadcrootc_transfer_to_fire +
					  cf->m_gresp_storage_to_fire + cf->m_gresp_transfer_to_fire;

	ns->frootn     -= nf->m_frootn_to_fire;
	ns->livecrootn -= nf->m_livecrootn_to_fire;
	ns->deadcrootn -= nf->m_deadcrootn_to_fire;

	ns->leafn_storage       -= nf->m_leafn_storage_to_fire;
	ns->frootn_storage      -= nf->m_frootn_storage_to_fire;
	ns->fruitn_storage      -= nf->m_fruitn_storage_to_fire;
	ns->softstemn_storage   -= nf->m_softstemn_storage_to_fire;
	ns->livestemn_storage   -= nf->m_livestemn_storage_to_fire;
	ns->deadstemn_storage   -= nf->m_deadstemn_storage_to_fire;
	ns->livecrootn_storage  -= nf->m_livecrootn_storage_to_fire;
	ns->deadcrootn_storage  -= nf->m_deadcrootn_storage_to_fire;

	ns->leafn_transfer      -= nf->m_leafn_transfer_to_fire;
	ns->frootn_transfer     -= nf->m_frootn_transfer_to_fire;
	ns->fruitn_transfer     -= nf->m_fruitn_transfer_to_fire;
	ns->softstemn_transfer  -= nf->m_softstemn_transfer_to_fire;
	ns->livestemn_transfer  -= nf->m_livestemn_transfer_to_fire;
	ns->deadstemn_transfer  -= nf->m_deadstemn_transfer_to_fire;
	ns->livecrootn_transfer -= nf->m_livecrootn_transfer_to_fire;
	ns->deadcrootn_transfer -= nf->m_deadcrootn_transfer_to_fire;
	ns->retransn             -= nf->m_retransn_to_fire;
	
	ns->fire_snk   += nf->m_frootn_to_fire + nf->m_livecrootn_to_fire + nf->m_deadcrootn_to_fire + 
		              nf->m_leafn_storage_to_fire + nf->m_frootn_storage_to_fire + nf->m_fruitn_storage_to_fire + nf->m_softstemn_storage_to_fire + 
		              nf->m_livestemn_storage_to_fire + nf->m_deadstemn_storage_to_fire + nf->m_livecrootn_storage_to_fire + nf->m_deadcrootn_storage_to_fire + 
		              nf->m_leafn_transfer_to_fire + nf->m_frootn_transfer_to_fire + nf->m_fruitn_transfer_to_fire + nf->m_softstemn_transfer_to_fire + 
		              nf->m_livestemn_transfer_to_fire + nf->m_deadstemn_transfer_to_fire + nf->m_livecrootn_transfer_to_fire + nf->m_deadcrootn_transfer_to_fire +
					  nf->m_retransn_to_fire;

	/* 2.3. standing and cut-down dead biomass */
	cs->STDBc_leaf     -= mort * cs->STDBc_leaf;
	cs->STDBc_froot    -= mort * cs->STDBc_froot;
	cs->STDBc_fruit    -= mort * cs->STDBc_fruit;
	cs->STDBc_softstem -= mort * cs->STDBc_softstem;
	cs->STDBc_transfer -= mort * cs->STDBc_transfer;
	
	cs->fire_snk     += cf->m_STDBc_to_fire;

	ns->STDBn_leaf     -= mort * ns->STDBn_leaf;
	ns->STDBn_froot    -= mort * ns->STDBn_froot;
	ns->STDBn_fruit    -= mort * ns->STDBn_fruit;
	ns->STDBn_softstem -= mort * ns->STDBn_softstem;
	ns->STDBn_transfer -= mort * ns->STDBn_transfer;

	ns->fire_snk     += nf->m_STDBn_to_fire;

	cs->CTDBc_leaf     -= mort * cs->CTDBc_leaf;
	cs->CTDBc_froot    -= mort * cs->CTDBc_froot;
	cs->CTDBc_fruit    -= mort * cs->CTDBc_fruit;
	cs->CTDBc_softstem -= mort * cs->CTDBc_softstem;
	cs->CTDBc_transfer -= mort * cs->CTDBc_transfer;
	cs->CTDBc_cstem    -= mort * cs->CTDBc_transfer;
	cs->CTDBc_croot    -= mort * cs->CTDBc_cstem;
	
	cs->fire_snk     += cf->m_CTDBc_to_fire;

	ns->CTDBn_leaf     -= mort * ns->CTDBn_leaf;
	ns->CTDBn_froot    -= mort * ns->CTDBn_froot;
	ns->CTDBn_fruit    -= mort * ns->CTDBn_fruit;
	ns->CTDBn_softstem -= mort * ns->CTDBn_softstem;
	ns->CTDBn_transfer -= mort * ns->CTDBn_transfer;
	ns->CTDBn_cstem    -= mort * ns->CTDBn_transfer;
	ns->CTDBn_croot    -= mort * ns->CTDBn_cstem;

	ns->fire_snk     += nf->m_CTDBn_to_fire;

	/* 2.4 litter and CWD carbon state updates: fire addects only top soil layer */
	cs->litr1c[0]   -= cf->m_litr1c_to_fire;
	cs->litr2c[0]   -= cf->m_litr2c_to_fire;
	cs->litr3c[0]   -= cf->m_litr3c_to_fire;
	cs->litr4c[0]   -= cf->m_litr4c_to_fire;
	cs->cwdc[0]		-= cf->m_cwdc_to_fire;

	cs->fire_snk	+= cf->m_litr1c_to_fire + cf->m_litr2c_to_fire + cf->m_litr3c_to_fire + cf->m_litr4c_to_fire + cf->m_cwdc_to_fire;

	ns->litr1n[0]   -= nf->m_litr1n_to_fire;
	ns->litr2n[0]   -= nf->m_litr2n_to_fire;
	ns->litr3n[0]   -= nf->m_litr3n_to_fire;
	ns->litr4n[0]   -= nf->m_litr4n_to_fire;
	ns->cwdn[0]		-= nf->m_cwdn_to_fire;

	ns->fire_snk	+= nf->m_litr1n_to_fire + nf->m_litr2n_to_fire + nf->m_litr3n_to_fire + nf->m_litr4n_to_fire + nf->m_cwdn_to_fire;
	
	
	return (!ok);
}

