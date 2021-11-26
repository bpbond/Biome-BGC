/* 
maint_resp.c
daily maintenance respiration

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2020, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "ini.h"
#include "bgc_struct.h"
#include "bgc_func.h"
#include "bgc_constants.h"

int maint_resp(const planting_struct* PLT, const cstate_struct* cs, const nstate_struct* ns, const epconst_struct* epc, const metvar_struct* metv,
	          epvar_struct* epv, cflux_struct* cf)
{
	/*
	maintenance respiration routine
	Uses reference values at 20 deg C and an empirical relationship between
	tissue N content and respiration rate given in:

	Ryan, M.G., 1991. Effects of climate change on plant respiration.
	Ecological Applications, 1(2):157-167.
	
	Uses the same value of Q_10 (2.0) for all compartments, leaf, stem, 
	coarse and fine roots.
	
	From Ryan's figures and regressions equations, the maintenance respiration
	in kgC/day per kg of tissue N is:
	mrpern = 0.218 (kgC/kgN/d)
	
	Leaf maintenance respiration is calculated separately for day and
	night, since the PSN routine needs the daylight value.
	
	Leaf and fine root respiration are dependent on phenology.*/
	
	int errorCode=0;
	double t1;
	double exponent;
	double n_area_sun, n_area_shade, dlmr_area_sun, dlmr_area_shade;
	int layer;
	double tsoil, frootn_layer, livecrootn_layer, q10, froot_mr,livecroot_mr,softstem_mr,livestem_mr;


	double mrpern = epc->mrpern;
	double acclim_const = -0.00794; 	/* 1. calculate the tsoil exponenet regarding to froot_resp in multilayer soil */

	froot_mr = livecroot_mr = softstem_mr = livestem_mr = 0;


	/* ********************************************************* */
	/* 1. possibility to use temperature dependent Q10 */ 

	if (epc->q10depend_flag)
		q10= 3.22 - 0.046 * metv->tavg;
	else
		q10 = Q10_VALUE;

	/* ********************************************************* */
	/* 2. Leaf day and night maintenance respiration when leaves on */

	if (cs->leafc && epv->sun_proj_sla && epv->shade_proj_sla)
	{
		t1 = ns->leafn * mrpern;
		
		/* leaf, day */
		exponent = (metv->tday - 20.0) / 10.0;
		cf->leaf_day_mr = t1 * pow(q10, exponent) * metv->dayl / nSEC_IN_DAY;

		/* for day respiration, also determine rates of maintenance respiration
		per unit of projected leaf area in the sunlit and shaded portions of
		the canopy, for use in the photosynthesis routine */
		/* first, calculate the mass of N per unit of projected leaf area
		in each canopy fraction (kg N/m2 projected area) */
		n_area_sun   = 1.0/(epv->sun_proj_sla * epc->leaf_cn);
		n_area_shade = 1.0/(epv->shade_proj_sla * epc->leaf_cn);
		/* convert to respiration flux in kg C/m2 projected area/day, and
		correct for temperature */
		dlmr_area_sun   = n_area_sun * mrpern * pow(q10, exponent);
		dlmr_area_shade = n_area_shade * mrpern * pow(q10, exponent);
		/* finally, convert from mass to molar units, and from a daily rate to 
		a rate per second */
		epv->dlmr_area_sun = dlmr_area_sun/(nSEC_IN_DAY * 12.011e-9);
		epv->dlmr_area_shade = dlmr_area_shade/(nSEC_IN_DAY * 12.011e-9);
		
		/* leaf, night */
		exponent = (metv->tnight - 20.0) / 10.0;
		cf->leaf_night_mr = t1 * pow(q10, exponent) * (nSEC_IN_DAY - metv->dayl) / nSEC_IN_DAY;
	}
	else /* no leaves on */
	{
		cf->leaf_day_mr = 0.0;
		epv->dlmr_area_sun = 0.0;
		epv->dlmr_area_shade = 0.0;
		cf->leaf_night_mr = 0.0;
	}

	/* ********************************************************* */
	/* 2. fine root maintenance respiration when fine roots on */
	
	if (cs->frootc)
	{
		for (layer = epv->germ_layer; layer < epv->n_rootlayers; layer++)
		{
			tsoil = metv->tsoil[layer];
			
			frootn_layer = ns->frootn * epv->rootlength_prop[layer];
			exponent = (tsoil - 20.0) / 10.0;
			t1 = pow(q10, exponent);
			froot_mr += frootn_layer * mrpern * t1;
		}
		
	}
	else froot_mr = 0;/* no fine roots on */

	cf->froot_mr = froot_mr;

	/* ********************************************************* */
	/* 3. Fruit simulation maintenance respiration when fruits on */
   
	if (cs->fruitc && PLT->PLT_num == 0)
	{
		exponent = (metv->tavg - 20.0) / 10.0;
		t1 = pow(q10, exponent);
		cf->fruit_mr = ns->fruitn * mrpern * t1;
	}
	/* no fruits on */
	else cf->fruit_mr = 0.0;
		

	/* ********************************************************* */
	/* 4. softstem maintenance respiration */
	
	if (cs->softstemc)
	{
		exponent = (metv->tavg - 20.0) / 10.0;
		t1 = pow(q10, exponent);
		cf->softstem_mr = ns->softstemn * mrpern * t1;
	}

	/* ********************************************************* */
	/* 5. live stem maintenance respiration */
	
	if (cs->livestemc)
	{
		exponent = (metv->tavg - 20.0) / 10.0;
		t1 = pow(q10, exponent);
		cf->livestem_mr = ns->livestemn * mrpern * t1;
	}
	
    /* ********************************************************* */
	/* 6. live coarse root maintenance respiration */
	
	if (cs->livecrootc)
	{
		for (layer = epv->germ_layer; layer < epv->n_rootlayers; layer++)
		{
			tsoil = metv->tsoil[layer];

			livecrootn_layer = ns->livecrootn * epv->rootlength_prop[layer];
			
			exponent = (tsoil - 20.0) / 10.0;
			t1 = pow(q10, exponent);
			livecroot_mr += livecrootn_layer * mrpern * t1;
		}
		
	}
	
	cf->livecroot_mr = livecroot_mr;
	
	
	/* ********************************************************* */
	/* 7. acclimation (acc_flag=1 - only respiration is acclimated, acc_flag=3 - respiration and photosynt. are acclimated) */
	
	
	if (epc->resp_acclim_flag)
	{
		cf->leaf_day_mr   = cf->leaf_day_mr   * pow(10,(acclim_const * (metv->tavg10_ra-20.0)));
		cf->leaf_night_mr = cf->leaf_night_mr * pow(10,(acclim_const * (metv->tavg10_ra-20.0)));
		cf->froot_mr      = cf->froot_mr      * pow(10,(acclim_const * (metv->tavg10_ra-20.0)));
		cf->fruit_mr      = cf->fruit_mr      * pow(10,(acclim_const * (metv->tavg10_ra-20.0)));
		cf->livestem_mr   = cf->livestem_mr   * pow(10,(acclim_const * (metv->tavg10_ra-20.0)));
		cf->livecroot_mr  = cf->livecroot_mr  * pow(10,(acclim_const * (metv->tavg10_ra-20.0)));
		cf->softstem_mr   = cf->softstem_mr   * pow(10,(acclim_const * (metv->tavg10_ra-20.0)));
	}


	
	
	return (errorCode);
}
