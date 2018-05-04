/* 
cutdown2litter.c
calculation of daily  mortality fluxes of cut-down biomass: these fluxes all enter litter sinks

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0
Copyright 2018, D. Hidy [dori.hidy@gmail.com]
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

int cutdown2litter(const epconst_struct* epc, const epvar_struct* epv, cstate_struct* cs, cflux_struct* cf,nstate_struct* ns, nflux_struct* nf)
{
	int ok=1;
	int layer;
	double mort_nW, mort_W;

	mort_nW = epc->mort_CnW_to_litter;
	mort_W  = epc->mort_CW_to_litter;

	/**************************************************************/
	/* 1. mortality fluxes to litter pools */

	cf->CTDBc_leaf_to_litr     = cs->CTDBc_leaf     * mort_nW;
	cf->CTDBc_froot_to_litr    = cs->CTDBc_froot    * mort_nW;
	cf->CTDBc_fruit_to_litr    = cs->CTDBc_fruit    * mort_nW;
	cf->CTDBc_softstem_to_litr = cs->CTDBc_softstem * mort_nW;
	cf->CTDBc_transfer_to_litr = cs->CTDBc_transfer * mort_nW;
	cf->CTDBc_cstem_to_cwd     = cs->CTDBc_cstem    * mort_W;
	cf->CTDBc_croot_to_cwd     = cs->CTDBc_croot    * mort_W;

    nf->CTDBn_leaf_to_litr     = ns->CTDBn_leaf     * mort_nW;
	nf->CTDBn_froot_to_litr    = ns->CTDBn_froot    * mort_nW;
	nf->CTDBn_fruit_to_litr    = ns->CTDBn_fruit    * mort_nW;
	nf->CTDBn_softstem_to_litr = ns->CTDBn_softstem * mort_nW;
	nf->CTDBn_transfer_to_litr = ns->CTDBn_transfer * mort_nW;
	nf->CTDBn_cstem_to_cwd     = ns->CTDBn_cstem    * mort_W;
	nf->CTDBn_croot_to_cwd     = ns->CTDBn_croot    * mort_W;
		
	/* returning of cut-down plant material into litter */
	cf->CTDBc_to_litr  = cf->CTDBc_leaf_to_litr     + cf->CTDBc_froot_to_litr + cf->CTDBc_fruit_to_litr + cf->CTDBc_softstem_to_litr +
		                 cf->CTDBc_transfer_to_litr + 
						 cf->CTDBc_cstem_to_cwd     + cf->CTDBc_croot_to_cwd;

	nf->CTDBn_to_litr  = nf->CTDBn_leaf_to_litr     + nf->CTDBn_froot_to_litr + nf->CTDBn_fruit_to_litr + nf->CTDBn_softstem_to_litr +
		                 nf->CTDBn_transfer_to_litr + 
						 nf->CTDBn_cstem_to_cwd     + nf->CTDBn_croot_to_cwd;


	/****************************************************************************************/
	/* 2. mortality fluxes turn into litter pools: 	aboveground biomass into the top soil layer, belowground biomass divided between soil layers based on their root content */
	
	/* 2.1 aboveground biomass into the top soil layer */
	cs->litr1c[0] += cf->CTDBc_leaf_to_litr * epc->leaflitr_flab  + cf->CTDBc_fruit_to_litr * epc->fruitlitr_flab  + cf->CTDBc_softstem_to_litr * epc->softstemlitr_flab;
	cs->litr2c[0] += cf->CTDBc_leaf_to_litr * epc->leaflitr_fucel + cf->CTDBc_fruit_to_litr * epc->fruitlitr_fucel + cf->CTDBc_softstem_to_litr * epc->softstemlitr_fucel;
	cs->litr3c[0] += cf->CTDBc_leaf_to_litr * epc->leaflitr_fscel + cf->CTDBc_fruit_to_litr * epc->fruitlitr_fscel + cf->CTDBc_softstem_to_litr * epc->softstemlitr_fscel;
	cs->litr4c[0] += cf->CTDBc_leaf_to_litr * epc->leaflitr_flig  + cf->CTDBc_fruit_to_litr * epc->fruitlitr_flig  + cf->CTDBc_softstem_to_litr * epc->softstemlitr_flig;
	cs->cwdc[0]   += cf->CTDBc_cstem_to_cwd;

	ns->litr1n[0] += nf->CTDBn_leaf_to_litr * epc->leaflitr_flab  + nf->CTDBn_fruit_to_litr * epc->fruitlitr_flab  + nf->CTDBn_softstem_to_litr * epc->softstemlitr_flab;
	ns->litr2n[0] += nf->CTDBn_leaf_to_litr * epc->leaflitr_fucel + nf->CTDBn_fruit_to_litr * epc->fruitlitr_fucel + nf->CTDBn_softstem_to_litr * epc->softstemlitr_fucel;
	ns->litr3n[0] += nf->CTDBn_leaf_to_litr * epc->leaflitr_fscel + nf->CTDBn_fruit_to_litr * epc->fruitlitr_fscel + nf->CTDBn_softstem_to_litr * epc->softstemlitr_fscel;
	ns->litr4n[0] += nf->CTDBn_leaf_to_litr * epc->leaflitr_flig  + nf->CTDBn_fruit_to_litr * epc->fruitlitr_flig  + nf->CTDBn_softstem_to_litr * epc->softstemlitr_flig;
	ns->cwdn[0]   += nf->CTDBn_cstem_to_cwd;

	/* 2.2 	belowground biomass divided between soil layers based on their root content */ 
	for (layer=0; layer < N_SOILLAYERS; layer++)
	{
		cs->litr1c[layer]  += (cf->CTDBc_transfer_to_litr + cf->CTDBc_froot_to_litr * epc->frootlitr_flab)  * epv->rootlength_prop[layer];
		cs->litr2c[layer]  += cf->CTDBc_froot_to_litr * epc->frootlitr_fucel * epv->rootlength_prop[layer];
		cs->litr3c[layer]  += cf->CTDBc_froot_to_litr * epc->frootlitr_fscel * epv->rootlength_prop[layer];
		cs->litr4c[layer]  += cf->CTDBc_froot_to_litr * epc->frootlitr_flig * epv->rootlength_prop[layer];
		cs->cwdc[layer]    += cf->CTDBc_croot_to_cwd;

		ns->litr1n[layer]  += (nf->CTDBn_transfer_to_litr + nf->CTDBn_froot_to_litr * epc->frootlitr_flab)  * epv->rootlength_prop[layer];
		ns->litr2n[layer]  += nf->CTDBn_froot_to_litr * epc->frootlitr_fucel * epv->rootlength_prop[layer];
		ns->litr3n[layer]  += nf->CTDBn_froot_to_litr * epc->frootlitr_fscel * epv->rootlength_prop[layer];
		ns->litr4n[layer]  += nf->CTDBn_froot_to_litr * epc->frootlitr_flig  * epv->rootlength_prop[layer];
		ns->cwdn[layer]    += nf->CTDBn_croot_to_cwd;
	}
	
	/************************************************************/
	/* 3. decreasing of temporary pool */

	cs->CTDBc_leaf     -= cf->CTDBc_leaf_to_litr;
	cs->CTDBc_froot    -= cf->CTDBc_froot_to_litr;
	cs->CTDBc_fruit    -= cf->CTDBc_fruit_to_litr;
	cs->CTDBc_softstem -= cf->CTDBc_softstem_to_litr;
	cs->CTDBc_transfer -= cf->CTDBc_transfer_to_litr;
	cs->CTDBc_cstem    -= cf->CTDBc_cstem_to_cwd;
	cs->CTDBc_croot    -= cf->CTDBc_croot_to_cwd;
	

    ns->CTDBn_leaf     -= nf->CTDBn_leaf_to_litr;
	ns->CTDBn_froot    -= nf->CTDBn_froot_to_litr;
	ns->CTDBn_fruit    -= nf->CTDBn_fruit_to_litr;
	ns->CTDBn_softstem -= nf->CTDBn_softstem_to_litr;
	ns->CTDBn_transfer -= nf->CTDBn_transfer_to_litr;
	ns->CTDBn_cstem    -= nf->CTDBn_cstem_to_cwd;
	ns->CTDBn_croot    -= nf->CTDBn_croot_to_cwd;

	/************************************************************/
	/* 4. precision control */

	if ((cs->CTDBc_leaf != 0 && fabs(cs->CTDBc_leaf) < CRIT_PREC) || (ns->CTDBn_leaf != 0 && fabs(ns->CTDBn_leaf) < CRIT_PREC))
	{
		cs->fire_snk += cs->CTDBc_leaf;
		ns->fire_snk += ns->CTDBn_leaf; 
		cs->CTDBc_leaf = 0;
		ns->CTDBn_leaf = 0;
	}

	if ((cs->CTDBc_fruit != 0 && fabs(cs->CTDBc_fruit) < CRIT_PREC) || (ns->CTDBn_fruit != 0 && fabs(ns->CTDBn_fruit) < CRIT_PREC))
	{
		cs->fire_snk += cs->CTDBc_fruit;
		ns->fire_snk += ns->CTDBn_fruit; 
		cs->CTDBc_fruit = 0;
		ns->CTDBn_fruit = 0;
	}

	if ((cs->CTDBc_softstem != 0 && fabs(cs->CTDBc_softstem) < CRIT_PREC) || (ns->CTDBn_softstem != 0 && fabs(ns->CTDBn_softstem) < CRIT_PREC))
	{
		cs->fire_snk += cs->CTDBc_softstem;
		ns->fire_snk += ns->CTDBn_softstem; 
		cs->CTDBc_softstem = 0;
		ns->CTDBn_softstem = 0;
	}

	if ((cs->CTDBc_froot != 0 && fabs(cs->CTDBc_froot) < CRIT_PREC) || (ns->CTDBn_froot != 0 && fabs(ns->CTDBn_froot) < CRIT_PREC))
	{
		cs->fire_snk += cs->CTDBc_froot;
		ns->fire_snk += ns->CTDBn_froot; 
		cs->CTDBc_froot = 0;
		ns->CTDBn_froot = 0;
	}

	if ((cs->CTDBc_transfer != 0 && fabs(cs->CTDBc_transfer) < CRIT_PREC) || (ns->CTDBn_transfer != 0 && fabs(ns->CTDBn_transfer) < CRIT_PREC))
	{
		cs->fire_snk += cs->CTDBc_transfer;
		ns->fire_snk += ns->CTDBn_transfer; 
		cs->CTDBc_transfer = 0;
		ns->CTDBn_transfer = 0;
	}


	if ((cs->CTDBc_cstem != 0 && fabs(cs->CTDBc_cstem) < CRIT_PREC) || (ns->CTDBn_cstem != 0 && fabs(ns->CTDBn_cstem) < CRIT_PREC))
	{
		cs->fire_snk += cs->CTDBc_cstem;
		ns->fire_snk += ns->CTDBn_cstem; 
		cs->CTDBc_cstem = 0;
		ns->CTDBn_cstem = 0;
	}

	if ((cs->CTDBc_croot != 0 && fabs(cs->CTDBc_croot) < CRIT_PREC) || (ns->CTDBn_croot != 0 && fabs(ns->CTDBn_croot) < CRIT_PREC))
	{
		cs->fire_snk += cs->CTDBc_croot;
		ns->fire_snk += ns->CTDBn_croot; 
		cs->CTDBc_croot = 0;
		ns->CTDBn_croot = 0;
	}


	return(!ok);
}