/* 
decomp.c
daily decomposition fluxes
Note that final immobilization fluxes are not reconciled until the
end of the daily allocation function, in order to allow competition
between microbes and plants for available N.

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.1
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2017, D. Hidy [dori.hidy@gmail.com]
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

int decomp(const metvar_struct* metv,const epconst_struct* epc, epvar_struct* epv, 
const siteconst_struct* sitec, cstate_struct* cs, cflux_struct* cf,
nstate_struct* ns, nflux_struct* nf, ntemp_struct* nt)

{
	int ok=1;
	int layer;
	double t_scalar, w_scalar;
	double rate_scalar = 0;
	double rate_scalar_total = 0;
	double tk, tsoil;
	double minvwc, maxvwc, opt1vwc, opt2vwc, vwc;
	double rfl1s1, rfl2s2,rfl4s3,rfs1s2,rfs2s3,rfs3s4;
	double kl1_base,kl2_base,kl4_base,ks1_base,ks2_base,ks3_base,ks4_base,kfrag_base;
	double kl1,kl2,kl4,ks1,ks2,ks3,ks4,kfrag;
	double cn_l1,cn_l2,cn_l4,cn_s1,cn_s2,cn_s3,cn_s4;
	double cwdc_loss;
	double plitr1c_loss, plitr2c_loss, plitr4c_loss;
	double psoil1c_loss, psoil2c_loss, psoil3c_loss, psoil4c_loss;
	double pmnf_l1s1,pmnf_l2s2,pmnf_l4s3,pmnf_s1s2,pmnf_s2s3,pmnf_s3s4,pmnf_s4;
	double potential_immob,mineralized;
	double ratio;
	double rate_scalar_avg;

	/* Hidy 2015 - calculate the C and N content in multilayer soil: layer by layer */
	double litr1c, litr2c, litr3c, litr4c, soil1c, soil2c, soil3c, soil4c, cwdc; 
	double litr1n, litr2n, litr3n, litr4n, soil1n, soil2n, soil3n, soil4n; 

	double cwdc_to_litr2c, cwdc_to_litr3c , cwdc_to_litr4c;
	double cwdn_to_litr2n, cwdn_to_litr3n , cwdn_to_litr4n;

	/* empirical estimation of N2O flux */
	double CNR, BD, N2O_flux, CH4_flux;

	/* initialize partial carbon and nitrogen content in litter and soil pool */
	litr1c=litr2c=litr3c=litr4c=soil1c=soil2c=soil3c=soil4c=0;
	litr1n=litr2n=litr3n=litr4n=soil1n=soil2n=soil3n=soil4n=0;

	rate_scalar_avg=0;

	potential_immob=mineralized=kl4=N2O_flux=CH4_flux=0;

	/* initialize the potential loss and mineral N flux variables */

	potential_immob=mineralized=kl4=N2O_flux=CH4_flux=0;
	plitr1c_loss=plitr2c_loss=plitr4c_loss=psoil1c_loss=psoil2c_loss=psoil3c_loss=psoil4c_loss=0.0;
	pmnf_l1s1=pmnf_l2s2=pmnf_l4s3=pmnf_s1s2=pmnf_s2s3=pmnf_s3s4=pmnf_s4=0.0;
	cwdc_to_litr2c=cwdc_to_litr3c =cwdc_to_litr4c=cwdn_to_litr2n=cwdn_to_litr3n =cwdn_to_litr4n=0;



	
	/* Hidy 2015 - calculate the rate constant scalar in multilayer soil: layer by layer */
	for (layer=0; layer < epv->n_maxrootlayers; layer++)
	{
		tsoil = metv->tsoil[layer];

		/* calculate the rate constant scalar for soil temperature,
		assuming that the base rate constants are assigned for non-moisture
		limiting conditions at 25 C. The function used here is taken from
		Lloyd, J., and J.A. Taylor, 1994. On the temperature dependence of 
		soil respiration. Functional Ecology, 8:315-323.
		This equation is a modification of their eqn. 11, changing the base
		temperature from 10 C to 25 C, since most of the microcosm studies
		used to get the base decomp rates were controlled at 25 C. */
		if (tsoil < -10.0)
		{
			/* no decomp processes for tsoil < -10.0 C */
			t_scalar = 0.0;
		}
		else
		{
			if (tsoil < 25)
			{
				tk = tsoil + 273.15;
				t_scalar = exp(308.56*((1.0/71.02)-(1.0/(tk-227.13))));
			}
			else // !!!!!!!!!!!!! NEW BUG FIX - Hidy 2015 !!!!!!!!!!!!!!!!
				t_scalar = 1;
			
		}
		
		/* calculate the rate constant scalar for soil water content.
		Uses the log relationship with water potential given in
		Andren, O., and K. Paustian, 1987. Barley straw decomposition in the field:
		a comparison of models. Ecology, 68(5):1190-1200.
		and supported by data in
		Orchard, V.A., and F.J. Cook, 1983. Relationship between soil respiration
		and soil moisture. Soil Biol. Biochem., 15(4):447-453.
		*/
		/* Hidy 2013 . set the maximum and minimum values for water content limits (m3/m3) */
		minvwc  = sitec->vwc_hw[layer];
		maxvwc  = sitec->vwc_sat[layer];
		opt1vwc = epv->vwc_crit1[layer]; 
		opt2vwc = epv->vwc_crit2[layer]; 
		vwc     = epv->vwc[layer];

		if (vwc <= minvwc)
		{
			/* no decomp below the minimum soil water potential*/
			w_scalar = 0.0;
		}
		else
		{
			/* increasing decomp approaching to optimum range (between opt1 and opt2) */
			if (vwc < opt1vwc) 
			{
				w_scalar = (vwc - minvwc) / (opt1vwc - minvwc);
			}
			else 
			{
				/* optimalrange (between opt1 and opt2) */
				if (vwc <= opt2vwc) 
				{
					w_scalar = 1;
				}
				else
				{
					/* decreasing decomp near to total saturation and no decomp above saturation */
					if (vwc < maxvwc) 
					{
						w_scalar = (maxvwc - vwc) / (maxvwc - opt2vwc);
					}
					else
					{
						w_scalar = 0;	
					}
				}
				
			}
					
		}


		/* CONTROL - w_scalar must be grater than 0 */
		if (w_scalar < 0)
		{
 			printf("Error in w_scalar calculation in decomp.c\n");
			ok=0;
		}

		
		/* calculate the final rate scalar as the product of the temperature andwater scalars */
		rate_scalar				= w_scalar * t_scalar;
		rate_scalar_total       += rate_scalar;

		epv->t_scalar[layer]	= t_scalar;
		epv->w_scalar[layer]	= w_scalar;
		epv->rate_scalar[layer] = rate_scalar;
		rate_scalar_avg        += rate_scalar * (sitec->soillayer_thickness[layer]/sitec->soillayer_depth[epv->n_maxrootlayers-1]);

	}

	epv->rate_scalar_avg = rate_scalar_avg;


	litr1c = cs->litr1c;
	litr2c = cs->litr2c;
	litr3c = cs->litr3c;
	litr4c = cs->litr4c;
	soil1c = cs->soil1c;
	soil2c = cs->soil2c;
	soil3c = cs->soil3c;
	soil4c = cs->soil4c;
	litr1n = ns->litr1n;
	litr2n = ns->litr2n;
	litr3n = ns->litr3n;
	litr4n = ns->litr4n;
	soil1n = ns->soil1n;
	soil2n = ns->soil2n;
	soil3n = ns->soil3n;
	soil4n = ns->soil4n;
	cwdc   = cs->cwdc  ;
	

	/* calculate compartment C:N ratios */
	cn_l1 = litr1c/litr1n;
	cn_l2 = litr2c/litr2n;
	cn_l4 = litr4c/litr4n;
	cn_s1 = SOIL1_CN;
	cn_s2 = SOIL2_CN;
	cn_s3 = SOIL3_CN;
	cn_s4 = SOIL4_CN;
	
	/* respiration fractions for fluxes between compartments */
	rfl1s1 = epc->rfl1s1;
	rfl2s2 = epc->rfl2s2;
	rfl4s3 = epc->rfl4s3;
	rfs1s2 = epc->rfs1s2;
	rfs2s3 = epc->rfs2s3;
	rfs3s4 = epc->rfs3s4;
	
	/* calculate the corrected rate constants from the rate scalar and their
	base values. All rate constants are (1/day) */
	kl1_base	= epc->kl1_base;   /* labile litter pool */
	kl2_base	= epc->kl2_base;   /* cellulose litter pool */
	kl4_base	= epc->kl4_base;   /* lignin litter pool */
	ks1_base	= epc->ks1_base;   /* fast microbial recycling pool */
	ks2_base	= epc->ks2_base;   /* medium microbial recycling pool */
	ks3_base	= epc->ks3_base;   /* slow microbial recycling pool */
	ks4_base	= epc->ks4_base;   /* recalcitrant SOM (humus) pool */
	kfrag_base	= epc->kfrag_base; /* physical fragmentation of coarse woody debris */
	kl1 = kl1_base * epv->rate_scalar_avg;
	kl2 = kl2_base * epv->rate_scalar_avg;
	kl4 = kl4_base * epv->rate_scalar_avg;
	ks1 = ks1_base * epv->rate_scalar_avg;
	ks2 = ks2_base * epv->rate_scalar_avg;
	ks3 = ks3_base * epv->rate_scalar_avg;
	ks4 = ks4_base * epv->rate_scalar_avg;
	kfrag = kfrag_base * epv->rate_scalar_avg;
	
	/* woody vegetation type fluxes */
	if (epc->woody)
	{
		/* calculate the flux from CWD to litter lignin and cellulose
		compartments, due to physical fragmentation */
		cwdc_loss	   = kfrag * cwdc;
		cwdc_to_litr2c = cwdc_loss * epc->deadwood_fucel;
		cwdc_to_litr3c = cwdc_loss * epc->deadwood_fscel;
		cwdc_to_litr4c = cwdc_loss * epc->deadwood_flig;
		cwdn_to_litr2n = cwdc_to_litr2c/epc->deadwood_cn;
		cwdn_to_litr3n = cwdc_to_litr3c/epc->deadwood_cn;
		cwdn_to_litr4n = cwdc_to_litr4c/epc->deadwood_cn;
	}
	

	
	/* calculate the non-nitrogen limited fluxes between litter and
	soil compartments. These will be ammended for N limitation if it turns
	out the potential gross immobilization is greater than potential gross
	mineralization. */
	/* 1. labile litter to fast microbial recycling pool */
	if (litr1c > 0.0)
	{
		plitr1c_loss = kl1 * litr1c;
		if (litr1n > 0.0) ratio = cn_s1/cn_l1;
		else ratio = 0.0;
		pmnf_l1s1 = (plitr1c_loss * (1.0 - rfl1s1 - (ratio)))/cn_s1;
	}
	
	/* 2. cellulose litter to medium microbial recycling pool */
	if (litr2c > 0.0)
	{
		plitr2c_loss = kl2 * litr2c;
		if (litr2n > 0.0) ratio = cn_s2/cn_l2;
		else ratio = 0.0;
		pmnf_l2s2 = (plitr2c_loss * (1.0 - rfl2s2 - (ratio)))/cn_s2;
	}
	
	/* 3. lignin litter to slow microbial recycling pool */
	if (litr4c > 0.0)
	{
		plitr4c_loss = kl4 * litr4c;
		if (litr4n > 0.0) ratio = cn_s3/cn_l4;
		else ratio = 0.0;
		pmnf_l4s3 = (plitr4c_loss * (1.0 - rfl4s3 - (ratio)))/cn_s3;
	}
	
	/* 4. fast microbial recycling pool to medium microbial recycling pool */
	if (soil1c > 0.0)
	{
		psoil1c_loss = ks1 * soil1c;
		pmnf_s1s2 = (psoil1c_loss * (1.0 - rfs1s2 - (cn_s2/cn_s1)))/cn_s2;
	}
	
	/* 5. medium microbial recycling pool to slow microbial recycling pool */
	if (soil2c > 0.0)
	{
		psoil2c_loss = ks2 * soil2c;
		pmnf_s2s3 = (psoil2c_loss * (1.0 - rfs2s3 - (cn_s3/cn_s2)))/cn_s3;
	}
	
	/* 6. slow microbial recycling pool to recalcitrant SOM pool */
	if (soil3c > 0.0)
	{
		psoil3c_loss = ks3 * soil3c;
		pmnf_s3s4 = (psoil3c_loss * (1.0 - rfs3s4 - (cn_s4/cn_s3)))/cn_s4;
	}
	
	/* 7. mineralization of recalcitrant SOM */
	if (soil4c > 0.0)
	{
		psoil4c_loss = ks4 * soil4c;
		pmnf_s4 = -psoil4c_loss/cn_s4;
	}
	
	/* determine if there is sufficient mineral N to support potential
	immobilization. Immobilization fluxes are positive, mineralization fluxes
	are negative */

	if (pmnf_l1s1 > 0.0) potential_immob += pmnf_l1s1;
	else mineralized += -pmnf_l1s1;
	if (pmnf_l2s2 > 0.0) potential_immob += pmnf_l2s2;
	else mineralized += -pmnf_l2s2;
	if (pmnf_l4s3 > 0.0) potential_immob += pmnf_l4s3;
	else mineralized += -pmnf_l4s3;
	if (pmnf_s1s2 > 0.0) potential_immob += pmnf_s1s2;
	else mineralized += -pmnf_s1s2;
	if (pmnf_s2s3 > 0.0) potential_immob += pmnf_s2s3;
	else mineralized += -pmnf_s2s3;
	if (pmnf_s3s4 > 0.0) potential_immob += pmnf_s3s4;
	else mineralized += -pmnf_s3s4;
	mineralized += -pmnf_s4;

	/* OTHER GHG FLUX - only from the first layer (Hidy 2016. )*/
	CNR = (soil1c + soil2c + soil3c + soil4c)/(soil1n + soil2n + soil3n + soil4n);
	BD  = sitec->BD[0];
	if (ok && otherGHGflux_estimation(epc, CNR, BD, epv->vwc[0], metv->tsoil[0], &N2O_flux, &CH4_flux))
	{
		printf("Error: otherGHGflux_estimation() in decomp.c\n");
		ok=0;
	}	
	nf->N2O_flux_soil = N2O_flux;
	cf->CH4_flux_soil = CH4_flux;
		






	/* save the potential fluxes until plant demand has been assessed,
	to allow competition between immobilization fluxes and plant growth
	demands */
	nt->mineralized		= mineralized;
	nt->potential_immob = potential_immob;
	nt->plitr1c_loss	= plitr1c_loss;
	nt->pmnf_l1s1		= pmnf_l1s1;
	nt->plitr2c_loss	= plitr2c_loss;
	nt->pmnf_l2s2		= pmnf_l2s2;
	nt->plitr4c_loss	= plitr4c_loss;
	nt->pmnf_l4s3		= pmnf_l4s3;
	nt->psoil1c_loss	= psoil1c_loss;
	nt->pmnf_s1s2		= pmnf_s1s2;
	nt->psoil2c_loss	= psoil2c_loss;
	nt->pmnf_s2s3		= pmnf_s2s3;
	nt->psoil3c_loss	= psoil3c_loss;
	nt->pmnf_s3s4		= pmnf_s3s4;
	nt->psoil4c_loss	= psoil4c_loss;
	nt->kl4				= kl4;
	
	/* store the day's gross mineralization */
	epv->daily_gross_nmin = mineralized;

	cf->cwdc_to_litr2c = cwdc_to_litr2c;
	cf->cwdc_to_litr3c = cwdc_to_litr3c;
	cf->cwdc_to_litr4c = cwdc_to_litr4c;


	nf->cwdn_to_litr2n = cwdn_to_litr2n;
	nf->cwdn_to_litr3n = cwdn_to_litr3n;
	nf->cwdn_to_litr4n = cwdn_to_litr4n;
	
	
	return (!ok);
}

