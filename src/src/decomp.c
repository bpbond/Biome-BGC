/* 
decomp.c
daily decomposition fluxes
Note that final immobilization fluxes are not reconciled until the
end of the daily allocation function, in order to allow competition
between microbes and plants for available N.

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


int decomp(const metvar_struct* metv,const epconst_struct* epc, const soilprop_struct* sprop, const siteconst_struct* sitec, const cstate_struct* cs, const nstate_struct* ns, 
	       epvar_struct* epv, cflux_struct* cf, nflux_struct* nf, ntemp_struct* nt)

{
	int errorCode=0;
	int layer;
	double ts_decomp, ws_decomp, z_scalar;
	double rs_decomp, rs_decomp_avg;
	double tsoil;
	double minVWC, maxVWC, opt1VWC, opt2VWC, VWC;
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


	/* calculate the C and N content in multilayer soil: layer by layer */
	double litr1c, litr2c, litr3c, litr4c, soil1c, soil2c, soil3c, soil4c, cwdc; 
	double litr1n, litr2n, litr3n, litr4n, soil1n, soil2n, soil3n, soil4n; 

	double cwdc_to_litr2c, cwdc_to_litr3c , cwdc_to_litr4c;
	double cwdn_to_litr2n, cwdn_to_litr3n , cwdn_to_litr4n;

	/* empirical estimation of CH4 flux */
	double CH4_flux;


	
	/* initialize partial carbon and nitrogen content in litter and soil pool */
	litr1c=litr2c=litr3c=litr4c=soil1c=soil2c=soil3c=soil4c=0;
	litr1n=litr2n=litr3n=litr4n=soil1n=soil2n=soil3n=soil4n=0;

	rs_decomp_avg=0;

	cf->cwdc_to_litr2c_total = 0;					
	cf->cwdc_to_litr3c_total = 0;					
	cf->cwdc_to_litr4c_total = 0;	

	epv->grossMINER_total = 0;
	epv->potIMMOB_total = 0;;

	/* 1. calculate the rate constant scalar in multilayer soil: layer by layer  */
	for (layer=0; layer < N_SOILLAYERS; layer++)
	{

		/* initialize the potential loss and mineral N flux variables */
		potential_immob=mineralized=kl4=CH4_flux=0;
		plitr1c_loss=plitr2c_loss=plitr4c_loss=psoil1c_loss=psoil2c_loss=psoil3c_loss=psoil4c_loss=0.0;
		pmnf_l1s1=pmnf_l2s2=pmnf_l4s3=pmnf_s1s2=pmnf_s2s3=pmnf_s3s4=pmnf_s4=0.0;
		cwdc_to_litr2c=cwdc_to_litr3c =cwdc_to_litr4c=cwdn_to_litr2n=cwdn_to_litr3n =cwdn_to_litr4n=0;

		tsoil = metv->tsoil[layer]; 
	
		/* 1.1: calculate the rate constant scalar for soil temperature, assuming that the base rate constants are assigned for non-moisture
		limiting conditions at 25 C. The function used here is taken from Lloyd, J., and J.A. Taylor, 1994. On the temperature dependence of 
		soil respiration. Functional Ecology, 8:315-323. Function: 	t_scalar = exp(308.56*((1.0/71.02)-(1.0/(tk-227.13))));
		This equation is a modification of their eqn. 11, changing the base temperature from 10 C to 25 C, since most of the microcosm studies
		used to get the base decomp rates were controlled at 25 C. */
		
		/* modification by Hidy 2021: new shape of tsoil function - similar to nitrification
		                              parameter for no decomp lmitation */

		
		if (sprop->Tp1_decomp == DATA_GAP)
		{
			/* no decomp processes for tsoil < -10.0 C */
			if (tsoil < sprop->Tmin_decomp)	
					ts_decomp = 0.0;
			else
				ts_decomp = exp(sprop->Tp2_decomp*((1.0/sprop->Tp3_decomp)-(1.0/((tsoil+Celsius2Kelvin)-sprop->Tp4_decomp))));
		}
		else
		{
			/* no decomp processes for tsoil < -10.0 C */
			if (tsoil < sprop->Tmin_decomp)	
					ts_decomp = 0.0;
			else
				ts_decomp = sprop->Tp1_decomp/(1+pow(fabs((tsoil-sprop->Tp4_decomp)/sprop->Tp2_decomp),sprop->Tp3_decomp));
			
		}
			

		/* 1.2: calculate the rate constant scalar for soil water content.
		Uses the log relationship with water potential given in Andren, O., and K. Paustian, 1987. Barley straw decomposition in the field:
		a comparison of models. Ecology, 68(5):1190-1200. and supported by data in Orchard, V.A., and F.J. Cook, 1983. Relationship between soil respiration
		and soil moisture. Soil Biol. Biochem., 15(4):447-453.*/
		/* set the maximum and minimum values for water content limits (m3/m3) */

		minVWC = sprop->VWChw[layer];
		maxVWC = sprop->VWCsat[layer];
		opt1VWC = sprop->VWCfc[layer];
		opt2VWC = epv->VWC_crit2[layer]; 

		VWC    = epv->VWC[layer];
	
		if (VWC < minVWC)
		{
			/* no decomp below  hygroscopic water */
			ws_decomp = 0.0;
		}
		else
		{
			/* increasing decomp near to field capacity */
			if (VWC < opt2VWC) 
			{
				/* unlimited decomp between optimal VWC values */
				if (VWC < opt1VWC)
					ws_decomp = (VWC - minVWC) / (opt1VWC - minVWC);	
				else
					ws_decomp = 1;	
			}
			else
			{
				/* decreasing decomp near to total saturation*/
				ws_decomp = (maxVWC - VWC) / (maxVWC - opt2VWC);

				/* lower limit for saturation: m_fullstress2 */
				if (ws_decomp < epc->m_fullstress2) ws_decomp = epc->m_fullstress2;}
		}
		
	
		/* CONTROL - ws_decomp must be grater than 0 */
		if (ws_decomp < 0 || ws_decomp > 1)
		{
			printf("\n");
 			printf("ERROR in ws_decomp calculation in decomp.c\n");
			errorCode=1;
		}


		/* 1.3: depth dependence of decompostion rate */
		z_scalar = exp(-1*(sitec->soillayer_midpoint[layer] / sprop->efolding_depth));

	
		/* 1.4: calculate the final rate scalar as the product of the temperature water and depth scalars */
		rs_decomp				= ws_decomp * ts_decomp * z_scalar;
		


		epv->ts_decomp[layer]	= ts_decomp;
		epv->ws_decomp[layer]	= ws_decomp;
		epv->rs_decomp[layer] = rs_decomp;
		rs_decomp_avg        += rs_decomp * (sitec->soillayer_thickness[layer]/sitec->soillayer_depth[N_SOILLAYERS-1]);


		epv->rs_decomp_avg = rs_decomp_avg;


		litr1c = cs->litr1c[layer];
		litr2c = cs->litr2c[layer];
		litr3c = cs->litr3c[layer];
		litr4c = cs->litr4c[layer];
		soil1c = cs->soil1c[layer];
		soil2c = cs->soil2c[layer];
		soil3c = cs->soil3c[layer];
		soil4c = cs->soil4c[layer];
		litr1n = ns->litr1n[layer];
		litr2n = ns->litr2n[layer];
		litr3n = ns->litr3n[layer];
		litr4n = ns->litr4n[layer];
		soil1n = ns->soil1n[layer];
		soil2n = ns->soil2n[layer];
		soil3n = ns->soil3n[layer];
		soil4n = ns->soil4n[layer];
		cwdc   = cs->cwdc[layer] ;
		

		/* 2. calculate compartment C:N ratios */
		cn_l1 = litr1c/litr1n;
		cn_l2 = litr2c/litr2n;
		cn_l4 = litr4c/litr4n;
		cn_s1 = sprop->soil1_CN;
		cn_s2 = sprop->soil2_CN;
		cn_s3 = sprop->soil3_CN;
		cn_s4 = sprop->soil4_CN;
		
		/* 3. respiration fractions for fluxes between compartments */
		rfl1s1 = sprop->rfl1s1;
		rfl2s2 = sprop->rfl2s2;
		rfl4s3 = sprop->rfl4s3;
		rfs1s2 = sprop->rfs1s2;
		rfs2s3 = sprop->rfs2s3;
		rfs3s4 = sprop->rfs3s4;
		
		/* 4. calculate the corrected rate constants from the rate scalar and their
		base values. All rate constants are (1/day) */
		kl1_base	= sprop->kl1_base;   /* labile litter pool */
		kl2_base	= sprop->kl2_base;   /* cellulose litter pool */
		kl4_base	= sprop->kl4_base;   /* lignin litter pool */
		ks1_base	= sprop->ks1_base;   /* labile SOM pool  */
		ks2_base	= sprop->ks2_base;   /* fast SOM pool  */
		ks3_base	= sprop->ks3_base;   /* slow SOM pool  */
		ks4_base	= sprop->ks4_base;   /* stable SOM pool  */
		kfrag_base	= sprop->kfrag_base; /* physical fragmentation of coarse woody debris */
		kl1 = kl1_base * epv->rs_decomp[layer];
		kl2 = kl2_base * epv->rs_decomp[layer];
		kl4 = kl4_base * epv->rs_decomp[layer];
		ks1 = ks1_base * epv->rs_decomp[layer];
		ks2 = ks2_base * epv->rs_decomp[layer];
		ks3 = ks3_base * epv->rs_decomp[layer];
		ks4 = ks4_base * epv->rs_decomp[layer];
		kfrag = kfrag_base * epv->rs_decomp[layer];
		
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
		/* 1. labile litter to labile SOM pool  */
		if (litr1c > 0)
		{
			plitr1c_loss = kl1 * litr1c;
			if (plitr1c_loss > litr1c) plitr1c_loss = litr1c;
			if (litr1n > 0.0) ratio = cn_s1/cn_l1;
			else ratio = 0.0;
			pmnf_l1s1 = (plitr1c_loss * (1.0 - rfl1s1 - (ratio)))/cn_s1;
		}
		
		/* 2. cellulose litter to fast SOM pool  */
		if (litr2c > 0)
		{
			plitr2c_loss = kl2 * litr2c;
			if (plitr2c_loss > litr2c) plitr2c_loss = litr2c;
			if (litr2n > 0.0) ratio = cn_s2/cn_l2;
			else ratio = 0.0;
			pmnf_l2s2 = (plitr2c_loss * (1.0 - rfl2s2 - (ratio)))/cn_s2;
		}
		
		/* 3. lignin litter to slow SOM pool  */
		if (litr4c > 0)
		{
			plitr4c_loss = kl4 * litr4c;
			if (plitr4c_loss > litr4c) plitr4c_loss = litr4c;
			if (litr4n > 0.0) ratio = cn_s3/cn_l4;
			else ratio = 0.0;
			pmnf_l4s3 = (plitr4c_loss * (1.0 - rfl4s3 - (ratio)))/cn_s3;
		}
		
		/* 4. labile SOM pool to fast SOM pool  */
		if (soil1c > 0)
		{
			psoil1c_loss = ks1 * soil1c;
			if (psoil1c_loss > soil1c) psoil1c_loss = soil1c;
			pmnf_s1s2 = (psoil1c_loss * (1.0 - rfs1s2 - (cn_s2/cn_s1)))/cn_s2;
		}
		
		/* 5. fast SOM pool to slow SOM pool */
		if (soil2c > 0)
		{
			psoil2c_loss = ks2 * soil2c;
			if (psoil2c_loss > soil2c) psoil2c_loss = soil2c;
			pmnf_s2s3 = (psoil2c_loss * (1.0 - rfs2s3 - (cn_s3/cn_s2)))/cn_s3;
		}
		
		/* 6. slow SOM pool to stable SOM pool */
		if (soil3c > 0)
		{
			psoil3c_loss = ks3 * soil3c;
			if (psoil3c_loss > soil3c) psoil3c_loss = soil3c;
			pmnf_s3s4 = (psoil3c_loss * (1.0 - rfs3s4 - (cn_s4/cn_s3)))/cn_s4;
		}
		
		/* 7. mineralization of stable SOM */
		if (soil4c > 0)
		{
			psoil4c_loss = ks4 * soil4c;
			if (psoil4c_loss > soil4c) psoil4c_loss = soil4c;
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

		/* CH4 FLUX - only from the first layer */
		if (!errorCode && CH4flux_estimation(sprop, layer, epv->VWC[layer], metv->tsoil[layer], &CH4_flux))
		{
			printf("\n");
			printf("ERROR: CH4flux_estimation() in decomp.c\n");
			errorCode=1;
		}	
		cf->CH4_flux_soil += CH4_flux;
			

		/* save the potential fluxes until plant demand has been assessed,
		to allow competition between immobilization fluxes and plant growth
		demands 
		nt: temporary variables for reconciliation of decomposition immobilization fluxes and plant growth N demands */
		nt->mineralized[layer]		= mineralized;
		nt->potential_immob[layer]  = potential_immob;
		nt->plitr1c_loss[layer]		= plitr1c_loss;
		nt->pmnf_l1s1[layer]		= pmnf_l1s1;
		nt->plitr2c_loss[layer]		= plitr2c_loss;
		nt->pmnf_l2s2[layer]		= pmnf_l2s2;
		nt->plitr4c_loss[layer]		= plitr4c_loss;
		nt->pmnf_l4s3[layer]		= pmnf_l4s3;
		nt->psoil1c_loss[layer]		= psoil1c_loss;
		nt->pmnf_s1s2[layer]		= pmnf_s1s2;
		nt->psoil2c_loss[layer]		= psoil2c_loss;
		nt->pmnf_s2s3[layer]		= pmnf_s2s3;
		nt->psoil3c_loss[layer]		= psoil3c_loss;
		nt->pmnf_s3s4[layer]		= pmnf_s3s4;
		nt->psoil4c_loss[layer]		= psoil4c_loss;
		nt->pmnf_s4[layer]		    = pmnf_s4;
		nt->kl4[layer]				= kl4;


		/* store the mineralization-immobilization-litter fluxes */
		epv->grossMINER[layer] = mineralized;
		epv->potIMMOB[layer]   = potential_immob;
		epv->grossMINER_total += mineralized;
		epv->potIMMOB_total	  += potential_immob;

		cf->cwdc_to_litr2c[layer]  = cwdc_to_litr2c;
		cf->cwdc_to_litr3c[layer]  = cwdc_to_litr3c;
		cf->cwdc_to_litr4c[layer]  = cwdc_to_litr4c;

		nf->cwdn_to_litr2n[layer]  = cwdn_to_litr2n;
		nf->cwdn_to_litr3n[layer]  = cwdn_to_litr3n;
		nf->cwdn_to_litr4n[layer]  = cwdn_to_litr4n;

		cf->cwdc_to_litr2c_total += cf->cwdc_to_litr2c[layer];					
		cf->cwdc_to_litr3c_total += cf->cwdc_to_litr3c[layer];					
		cf->cwdc_to_litr4c_total += cf->cwdc_to_litr4c[layer];	

		nf->cwdn_to_litr2n_total += nf->cwdn_to_litr2n[layer];                
		nf->cwdn_to_litr3n_total += nf->cwdn_to_litr3n[layer];              
		nf->cwdn_to_litr4n_total += nf->cwdn_to_litr4n[layer];              

	
}
	
	
	
	return (errorCode);
}

