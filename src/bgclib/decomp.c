/* 
decomp.c
daily decomposition fluxes
Note that final immobilization fluxes are not reconciled until the
end of the daily allocation function, in order to allow competition
between microbes and plants for available N.

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "bgc.h"

int decomp(double tsoil, const epconst_struct* epc, epvar_struct* epv, 
const siteconst_struct* sitec, cstate_struct* cs, cflux_struct* cf,
nstate_struct* ns, nflux_struct* nf, ntemp_struct* nt)
{
	int ok=1;
	double rate_scalar, t_scalar, w_scalar;
	double tk;
	double minpsi, maxpsi;
	double rfl1s1, rfl2s2,rfl4s3,rfs1s2,rfs2s3,rfs3s4;
	double kl1_base,kl2_base,kl4_base,ks1_base,ks2_base,ks3_base,ks4_base,kfrag_base;
	double kl1,kl2,kl4,ks1,ks2,ks3,ks4,kfrag;
	double cn_l1,cn_l2,cn_l4,cn_s1,cn_s2,cn_s3,cn_s4;
	double cwdc_loss;
	double plitr1c_loss, plitr2c_loss, plitr4c_loss;
	double psoil1c_loss, psoil2c_loss, psoil3c_loss, psoil4c_loss;
	double pmnf_l1s1,pmnf_l2s2,pmnf_l4s3,pmnf_s1s2,pmnf_s2s3,pmnf_s3s4,pmnf_s4;
	double potential_immob,mineralized;
	int nlimit;
	double ratio;
	
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
		tk = tsoil + 273.15;
		t_scalar = exp(308.56*((1.0/71.02)-(1.0/(tk-227.13))));
	}
	
	/* calculate the rate constant scalar for soil water content.
	Uses the log relationship with water potential given in
	Andren, O., and K. Paustian, 1987. Barley straw decomposition in the field:
	a comparison of models. Ecology, 68(5):1190-1200.
	and supported by data in
	Orchard, V.A., and F.J. Cook, 1983. Relationship between soil respiration
	and soil moisture. Soil Biol. Biochem., 15(4):447-453.
	*/
	/* set the maximum and minimum values for water potential limits (MPa) */
	minpsi = -10.0;
	maxpsi = sitec->psi_sat;
	if (epv->psi < minpsi)
	{
		/* no decomp below the minimum soil water potential */
		w_scalar = 0.0;
	}
	else if (epv->psi > maxpsi)
	{
		/* this shouldn't ever happen, but just in case... */
		w_scalar = 1.0;
	}
	else
	{
		w_scalar = log(minpsi/epv->psi)/log(minpsi/maxpsi);
	}
	
	/* calculate the final rate scalar as the product of the temperature and
	water scalars */
	rate_scalar = w_scalar * t_scalar;
	
	/* assign output variables */
	epv->t_scalar = t_scalar;
	epv->w_scalar = w_scalar;
	epv->rate_scalar = rate_scalar;
	
	/* calculate compartment C:N ratios */
	if (ns->litr1n > 0.0) cn_l1 = cs->litr1c/ns->litr1n;
	if (ns->litr2n > 0.0) cn_l2 = cs->litr2c/ns->litr2n;
	if (ns->litr4n > 0.0) cn_l4 = cs->litr4c/ns->litr4n;
	cn_s1 = SOIL1_CN;
	cn_s2 = SOIL2_CN;
	cn_s3 = SOIL3_CN;
	cn_s4 = SOIL4_CN;
	
	/* respiration fractions for fluxes between compartments */
	rfl1s1 = RFL1S1;
	rfl2s2 = RFL2S2;
	rfl4s3 = RFL4S3;
	rfs1s2 = RFS1S2;
	rfs2s3 = RFS2S3;
	rfs3s4 = RFS3S4;
	
	/* calculate the corrected rate constants from the rate scalar and their
	base values. All rate constants are (1/day) */
	kl1_base = KL1_BASE;	/* labile litter pool */
	kl2_base = KL2_BASE;	/* cellulose litter pool */
	kl4_base = KL4_BASE;	/* lignin litter pool */
	ks1_base = KS1_BASE;	/* fast microbial recycling pool */
	ks2_base = KS2_BASE;	/* medium microbial recycling pool */
	ks3_base = KS3_BASE;	/* slow microbial recycling pool */
	ks4_base = KS4_BASE;	/* recalcitrant SOM (humus) pool */
	kfrag_base = KFRAG_BASE; /* physical fragmentation of coarse woody debris */
	kl1 = kl1_base * rate_scalar;
	kl2 = kl2_base * rate_scalar;
	kl4 = kl4_base * rate_scalar;
	ks1 = ks1_base * rate_scalar;
	ks2 = ks2_base * rate_scalar;
	ks3 = ks3_base * rate_scalar;
	ks4 = ks4_base * rate_scalar;
	kfrag = kfrag_base * rate_scalar;
	
	/* woody vegetation type fluxes */
	if (epc->woody)
	{
		/* calculate the flux from CWD to litter lignin and cellulose
		compartments, due to physical fragmentation */
		cwdc_loss = kfrag * cs->cwdc;
		cf->cwdc_to_litr2c = cwdc_loss * epc->deadwood_fucel;
		cf->cwdc_to_litr3c = cwdc_loss * epc->deadwood_fscel;
		cf->cwdc_to_litr4c = cwdc_loss * epc->deadwood_flig;
		nf->cwdn_to_litr2n = cf->cwdc_to_litr2c/epc->deadwood_cn;
		nf->cwdn_to_litr3n = cf->cwdc_to_litr3c/epc->deadwood_cn;
		nf->cwdn_to_litr4n = cf->cwdc_to_litr4c/epc->deadwood_cn;
	}
	
	/* initialize the potential loss and mineral N flux variables */
	plitr1c_loss = plitr2c_loss = plitr4c_loss = 0.0;
	psoil1c_loss = psoil2c_loss = psoil3c_loss = psoil4c_loss = 0.0;
	pmnf_l1s1 = pmnf_l2s2 = pmnf_l4s3 = 0.0;
	pmnf_s1s2 = pmnf_s2s3 = pmnf_s3s4 = pmnf_s4 = 0.0;
	
	/* calculate the non-nitrogen limited fluxes between litter and
	soil compartments. These will be ammended for N limitation if it turns
	out the potential gross immobilization is greater than potential gross
	mineralization. */
	/* 1. labile litter to fast microbial recycling pool */
	if (cs->litr1c > 0.0)
	{
		plitr1c_loss = kl1 * cs->litr1c;
		if (ns->litr1n > 0.0) ratio = cn_s1/cn_l1;
		else ratio = 0.0;
		pmnf_l1s1 = (plitr1c_loss * (1.0 - rfl1s1 - (ratio)))/cn_s1;
	}
	
	/* 2. cellulose litter to medium microbial recycling pool */
	if (cs->litr2c > 0.0)
	{
		plitr2c_loss = kl2 * cs->litr2c;
		if (ns->litr2n > 0.0) ratio = cn_s2/cn_l2;
		else ratio = 0.0;
		pmnf_l2s2 = (plitr2c_loss * (1.0 - rfl2s2 - (ratio)))/cn_s2;
	}
	
	/* 3. lignin litter to slow microbial recycling pool */
	if (cs->litr4c > 0.0)
	{
		plitr4c_loss = kl4 * cs->litr4c;
		if (ns->litr4n > 0.0) ratio = cn_s3/cn_l4;
		else ratio = 0.0;
		pmnf_l4s3 = (plitr4c_loss * (1.0 - rfl4s3 - (ratio)))/cn_s3;
	}
	
	/* 4. fast microbial recycling pool to medium microbial recycling pool */
	if (cs->soil1c > 0.0)
	{
		psoil1c_loss = ks1 * cs->soil1c;
		pmnf_s1s2 = (psoil1c_loss * (1.0 - rfs1s2 - (cn_s2/cn_s1)))/cn_s2;
	}
	
	/* 5. medium microbial recycling pool to slow microbial recycling pool */
	if (cs->soil2c > 0.0)
	{
		psoil2c_loss = ks2 * cs->soil2c;
		pmnf_s2s3 = (psoil2c_loss * (1.0 - rfs2s3 - (cn_s3/cn_s2)))/cn_s3;
	}
	
	/* 6. slow microbial recycling pool to recalcitrant SOM pool */
	if (cs->soil3c > 0.0)
	{
		psoil3c_loss = ks3 * cs->soil3c;
		pmnf_s3s4 = (psoil3c_loss * (1.0 - rfs3s4 - (cn_s4/cn_s3)))/cn_s4;
	}
	
	/* 7. mineralization of recalcitrant SOM */
	if (cs->soil4c > 0.0)
	{
		psoil4c_loss = ks4 * cs->soil4c;
		pmnf_s4 = -psoil4c_loss/cn_s4;
	}
	
	/* determine if there is sufficient mineral N to support potential
	immobilization. Immobilization fluxes are positive, mineralization fluxes
	are negative */
	nlimit = 0;
	potential_immob = 0.0;
	mineralized = 0.0;
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
	
	/* save the potential fluxes until plant demand has been assessed,
	to allow competition between immobilization fluxes and plant growth
	demands */
	nt->mineralized = mineralized;
	nt->potential_immob = potential_immob;
	nt->plitr1c_loss = plitr1c_loss;
	nt->pmnf_l1s1 = pmnf_l1s1;
	nt->plitr2c_loss = plitr2c_loss;
	nt->pmnf_l2s2 = pmnf_l2s2;
	nt->plitr4c_loss = plitr4c_loss;
	nt->pmnf_l4s3 = pmnf_l4s3;
	nt->psoil1c_loss = psoil1c_loss;
	nt->pmnf_s1s2 = pmnf_s1s2;
	nt->psoil2c_loss = psoil2c_loss;
	nt->pmnf_s2s3 = pmnf_s2s3;
	nt->psoil3c_loss = psoil3c_loss;
	nt->pmnf_s3s4 = pmnf_s3s4;
	nt->psoil4c_loss = psoil4c_loss;
	nt->kl4 = kl4;
	
	/* store the day's gross mineralization */
	epv->daily_gross_nmin = mineralized ;
	
	return (!ok);
}

