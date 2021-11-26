/* 
multilayer_sminn.c
Calculating the change in content of soil mineral nitrogen in multilayer soil (plant N upate, soil processes, 
depostion and fixing). 

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Copyright 2020, D. Hidy [dori.hidy@gmail.com]
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

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

int multilayer_sminn(const control_struct* ctrl, const metvar_struct* metv,const soilprop_struct* sprop, const siteconst_struct* sitec, const cflux_struct* cf, 
	                 epvar_struct* epv, nstate_struct* ns, nflux_struct* nf)
{
	int errorCode=0;
	int layer=0;
	double NH4_prop,SOMresp,sminNO3avail,sminNO3avail_ppm;
	double pH, tsoil, WFPS, net_miner,sminNH4avail, N2O_flux_NITRIF,sminNH4_to_nitrif;
	double weight,g_per_cm3_to_kg_per_m3;
	double sminn_layer[N_SOILLAYERS];
	double sminNH4_change[N_SOILLAYERS];
	double sminNO3_change[N_SOILLAYERS];
	double sminn_to_soilCTRL, sminn_to_npoolCTRL, ndep_to_sminnCTRL, nfix_to_sminnCTRL;
	double SOMrespTOTAL,sminNO3_to_denitr,ratioN2_N2O;
	
	NH4_prop=net_miner=SOMresp=sminn_to_soilCTRL=sminn_to_npoolCTRL=ndep_to_sminnCTRL=nfix_to_sminnCTRL=0;
	g_per_cm3_to_kg_per_m3 = 1000;

	/* *****************************************************************************************************

	1.Deposition and fixation - INPUT
	2.Plant N uptake from SMINN: sminn_to_npool 
	3.Immobilization-mineralization: sminn_to_soil_SUM - due microbial soil processes SMINN is changing in the soil (determined in daily_allocation).
	4. Denitrification and Nitrification */

	/* if no root - no N-fixation */
	if (epv->n_rootlayers == 0) nf->nfix_to_sminnTOTAL = 0;

	/* SOMrespTOTAL calculation - unit: kgC/ha */
	SOMrespTOTAL = 0;
	for (layer = 0; layer < N_SOILLAYERS; layer++) 
		SOMrespTOTAL += (cf->soil1_hr[layer] + cf->soil2_hr[layer] + cf->soil3_hr[layer] + cf->soil4_hr[layer])* 10000;
	

	/*-----------------------------------------------------------------------------*/
	/* Calculations layer by layer */

	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		/*************************************/
		/* 0: calculation of sminn and NH4 prop in soil layers */

		sminn_layer[layer] = (ns->sminNH4avail[layer] + ns->sminNO3avail[layer]);

		if (sminn_layer[layer]) 
			NH4_prop   = ns->sminNH4avail[layer] / sminn_layer[layer];
		else
			NH4_prop   = sitec->NdepNH4_coeff;
		
		/*********************************************/
		/* 1. Deposition  only in 0-30 cm, fixation in rooting zone - based on rootlenght proportion */
		
		if (layer < 2) 
		{
 			weight = sitec->soillayer_thickness[layer]/sitec->soillayer_depth[1];
			nf->ndep_to_sminNH4[layer] = (nf->ndep_to_sminnTOTAL * weight) * sitec->NdepNH4_coeff;
			nf->ndep_to_sminNO3[layer] = (nf->ndep_to_sminnTOTAL * weight) * (1-sitec->NdepNH4_coeff);
		}
		else
		{
			nf->ndep_to_sminNH4[layer] = 0;
			nf->ndep_to_sminNO3[layer] = 0;
		}
		
		nf->nfix_to_sminNH4[layer] = nf->nfix_to_sminnTOTAL * epv->rootlength_prop[layer];

		ndep_to_sminnCTRL += nf->ndep_to_sminNH4[layer] + nf->ndep_to_sminNO3[layer];
		nfix_to_sminnCTRL += nf->nfix_to_sminNH4[layer];
		
		/*********************************************/
		/* 2. Immobilization-mineralization fluxes */

		nf->sminNH4_to_soil_SUM[layer] = nf->sminn_to_soil_SUM[layer] * NH4_prop;
		nf->sminNO3_to_soil_SUM[layer] = nf->sminn_to_soil_SUM[layer] * (1-NH4_prop);
		
		sminn_to_soilCTRL             += nf->sminNH4_to_soil_SUM[layer] + nf->sminNO3_to_soil_SUM[layer];

		/*********************************************/
		/* 3. Plant N uptake  in rootzone */
		
		nf->sminNH4_to_npool[layer] = nf->sminn_to_npool[layer] * NH4_prop;
		nf->sminNO3_to_npool[layer] = nf->sminn_to_npool[layer] * (1-NH4_prop);
		sminn_to_npoolCTRL         += nf->sminNH4_to_npool[layer] + nf->sminNO3_to_npool[layer];

		/*********************************************/
		/* 4. Nitrification */
		
		if (nf->sminNH4_to_soil_SUM[layer] > 0)
			net_miner    = nf->soil4n_to_sminNH4[layer];
		else
			net_miner    = nf->soil4n_to_sminNH4[layer] - nf->sminNH4_to_soil_SUM[layer];
		
	 	WFPS         = epv->WFPS[layer];
		sminNH4avail = ns->sminNH4avail[layer];
		tsoil        = metv->tsoil[layer]; 
		pH           = sprop->pH[layer];

		if (!errorCode && nitrification(layer, sprop, net_miner,tsoil,pH,WFPS,sminNH4avail, epv, &N2O_flux_NITRIF, &sminNH4_to_nitrif))
		{
			printf("\n");
			printf("ERROR in nitrification() for multilayer_sminn.c \n");
		}	

		nf->N2O_flux_NITRIF[layer]   =  N2O_flux_NITRIF;
		nf->sminNH4_to_nitrif[layer] = sminNH4_to_nitrif;
	

		/*********************************************/
		/* 5. Denitfirication  */

		sminNO3avail = ns->sminNO3avail[layer] - nf->sminNO3_to_npool[layer];
		if (nf->sminNO3_to_soil_SUM[layer] > 0) sminNO3avail -= nf->sminNO3_to_soil_SUM[layer];

		sminNO3avail_ppm = sminNO3avail / (sprop->BD[layer] / g_per_cm3_to_kg_per_m3 * sitec->soillayer_thickness[layer]);
		
		SOMresp = (cf->soil1_hr[layer] + cf->soil2_hr[layer] + cf->soil3_hr[layer] + cf->soil4_hr[layer])* 1000;

		if (!errorCode && denitrification(ctrl->soiltype, sminNO3avail_ppm, pH,WFPS, SOMrespTOTAL, &sminNO3_to_denitr,&ratioN2_N2O))
		{
			printf("\n");
			printf("ERROR in denitrification() for multilayer_sminn.c \n");
		}
		
		
		if (epv->VWC[layer] / sprop->VWCsat[layer] > sprop->critWFPS_denitr && sminNO3avail > 0)
			nf->sminNO3_to_denitr[layer] = sprop->denitr_coeff * SOMresp * sminNO3avail * (epv->VWC[layer] / sprop->VWCsat[layer]);
		else
			nf->sminNO3_to_denitr[layer] = 0;
		
		
	
		nf->N2O_flux_DENITR[layer]  = nf->sminNO3_to_denitr[layer] / (1 + ratioN2_N2O *  sprop->N2Oratio_denitr);
		nf->N2_flux_DENITR[layer]   = nf->sminNO3_to_denitr[layer] - nf->N2O_flux_DENITR[layer];


	
		/*********************************************/
		/* 7. STATE UPDATE */

		sminNH4_change[layer] = (nf->nfix_to_sminNH4[layer] + nf->ndep_to_sminNH4[layer] + nf->soil4n_to_sminNH4[layer] - 
			                     nf->sminNH4_to_soil_SUM[layer] - nf->sminNH4_to_npool[layer] - nf->sminNH4_to_nitrif[layer]);
		sminNO3_change[layer] = (nf->ndep_to_sminNO3[layer] + (nf->sminNH4_to_nitrif[layer]- nf->N2O_flux_NITRIF[layer]) -
			                     nf->sminNO3_to_soil_SUM[layer] - nf->sminNO3_to_npool[layer] - nf->sminNO3_to_denitr[layer]);

		ns->sminNH4[layer] += sminNH4_change[layer]; 
		ns->sminNO3[layer] += sminNO3_change[layer];

		ns->sminNH4avail[layer] = ns->sminNH4[layer] * sprop->NH4_mobilen_prop;
		ns->sminNO3avail[layer] = ns->sminNO3[layer] * NO3_mobilen_prop;

		ns->soil1n[layer] += nf->sminn_to_soil1n_l1[layer];
		ns->soil2n[layer] += nf->sminn_to_soil2n_l2[layer] + nf->sminn_to_soil2n_s1[layer];
		ns->soil3n[layer] += nf->sminn_to_soil3n_l4[layer] + nf->sminn_to_soil3n_s2[layer];
		ns->soil4n[layer] += nf->sminn_to_soil4n_s3[layer] - nf->soil4n_to_sminNH4[layer];
		
		ns->npool         += (nf->sminNH4_to_npool[layer] + nf->sminNO3_to_npool[layer]);
		ns->Nfix_src      += nf->nfix_to_sminNH4[layer];
		ns->Ndep_src      += nf->ndep_to_sminNH4[layer] + nf->ndep_to_sminNO3[layer];

		ns->Nvol_snk      +=  nf->N2O_flux_NITRIF[layer];
		ns->Nvol_snk      +=  nf->sminNO3_to_denitr[layer];
		
		/*********************************************/
		/* 8. Total flux calculation */

		nf->N2_flux_DENITR_total		+= nf->N2_flux_DENITR[layer];
		nf->N2O_flux_DENITR_total		+= nf->N2O_flux_DENITR[layer];
		nf->N2O_flux_NITRIF_total		+= nf->N2O_flux_NITRIF[layer];
         
		nf->sminNH4_to_soil_SUM_total	+= nf->sminNH4_to_soil_SUM[layer];           
		nf->sminNO3_to_soil_SUM_total	+= nf->sminNO3_to_soil_SUM[layer];          
		nf->sminNO3_to_denitr_total		+= nf->sminNO3_to_denitr[layer];
		nf->sminNH4_to_nitrif_total		+= nf->sminNH4_to_nitrif[layer];
		nf->sminNH4_to_npoolTOTAL		+= nf->sminNH4_to_npool[layer];
		nf->sminNO3_to_npoolTOTAL		+= nf->sminNO3_to_npool[layer];

		/*********************************************/
		/* 9. CONTROL */
		if (ns->sminNH4[layer] < 0.0)       
		{	
			if (fabs(ns->sminNH4[layer]) > CRIT_PREC)
			{
				printf("\n");
				printf("ERROR: negative NH4 pool (multilayer_sminn.c)\n");
				errorCode=1;
			}
			else
			{
				ns->Nprec_snk     += ns->sminNH4[layer];
				ns->sminNH4[layer] = 0.0;
			}
		}

		if (ns->sminNO3[layer] < 0.0)       
		{	
			if (fabs(ns->sminNO3[layer]) > CRIT_PREC)
			{
				printf("\n");
				printf("ERROR: negative NO3 pool (multilayer_sminn.c)\n");
				errorCode=1;
			}
			else
			{
				ns->Nprec_snk     += ns->sminNO3[layer];
				ns->sminNO3[layer] = 0.0;
			}
		}

		if ((ns->soil1n[layer] < 0))
		{
			if (fabs (ns->soil1n[layer]) > CRIT_PREC)
			{
				printf("\n");
				printf("ERROR: negative soil N pool (multilayer_sminn.c)\n");
				errorCode=1;
			}
			else
			{
				ns->Nprec_snk     += ns->soil1n[layer];
				ns->soil1n[layer] = 0.0;
			}
		}

		if ((ns->soil2n[layer] < 0))
		{
			if (fabs (ns->soil2n[layer]) > CRIT_PREC)
			{
				printf("\n");
				printf("ERROR: negative soil N pool (multilayer_sminn.c)\n");
				errorCode=1;
			}
			else
			{
				ns->Nprec_snk     += ns->soil2n[layer];
				ns->soil2n[layer] = 0.0;
			}
		}

		if ((ns->soil3n[layer] < 0))
		{
			if (fabs (ns->soil3n[layer]) > CRIT_PREC)
			{
				printf("\n");
				printf("ERROR: negative soil N pool (multilayer_sminn.c)\n");
				errorCode=1;
			}
			else
			{
				ns->Nprec_snk     += ns->soil3n[layer];
				ns->soil3n[layer] = 0.0;
			}
		}

		if ((ns->soil4n[layer] < 0))
		{
			if (fabs (ns->soil4n[layer]) > CRIT_PREC)
			{
				printf("\n");
				printf("ERROR: negative soil N pool (multilayer_sminn.c)\n");
				errorCode=1;
			}
			else
			{
				ns->Nprec_snk     += ns->soil4n[layer];
				ns->soil4n[layer] = 0.0;
			}
		}
	}

	if (fabs(sminn_to_soilCTRL - nf->sminn_to_soil_SUM_total) > CRIT_PREC || fabs(sminn_to_npoolCTRL - nf->sminn_to_npoolTOTAL) > CRIT_PREC ||
		fabs(ndep_to_sminnCTRL - nf->ndep_to_sminnTOTAL) > CRIT_PREC || fabs(nfix_to_sminnCTRL - nf->nfix_to_sminnTOTAL) > CRIT_PREC)
	{
		printf("\n");
		printf("ERROR: in calculation of nitrogen state update (multilayer_sminn.c)\n");
		errorCode=1;
	}



	return (errorCode);
}

int nitrification(int layer, const soilprop_struct* sprop, double net_miner, double tsoil, double pH, double WFPS, double sminNH4avail, 
	                  epvar_struct* epv, double* N2O_flux_NITRIF, double* sminNH4_to_nitrif)
{
	int errorCode = 0;
	double sminNH4_to_nit,N2O_flux_NIT;
	
	/* calculation of scalar functions: Tsoil response function, ps_nitrif and WFPS_scalar */

	epv->ps_nitrif[layer] = sprop->pHp2_nitrif + (sprop->pHp1_nitrif-sprop->pHp2_nitrif)/(1 + exp((pH-sprop->pHp3_nitrif)/sprop->pHp4_nitrif));	

				
	if (sprop->Tp1_decomp == DATA_GAP)
	{
		/* no decomp processes for tsoil < -10.0 C */
		if (tsoil < sprop->Tmin_decomp)	
				epv->ts_nitrif[layer] = 0.0;
		else
			epv->ts_nitrif[layer] = exp(sprop->Tp2_nitrif*((1.0/sprop->Tp3_nitrif)-(1.0/((tsoil+Celsius2Kelvin)-sprop->Tp4_nitrif))));
	}
	else
	{
		/* no decomp processes for tsoil < -10.0 C */
		if (tsoil < sprop->Tmin_decomp)	
				epv->ts_nitrif[layer] = 0.0;
		else
			epv->ts_nitrif[layer] = sprop->Tp1_nitrif/(1+pow(fabs((tsoil-sprop->Tp4_nitrif)/sprop->Tp2_nitrif),sprop->Tp3_nitrif));
			
	}

	if (WFPS < sprop->minWFPS_nitrif)
	{
		epv->ws_nitrif[layer] = 0;
	}
	else
	{
		if (WFPS < sprop->opt1WFPS_nitrif)
		{
			epv->ws_nitrif[layer] = (WFPS - sprop->minWFPS_nitrif) / (sprop->opt1WFPS_nitrif - sprop->minWFPS_nitrif);	
		}
		else
		{
			if (WFPS < sprop->opt2WFPS_nitrif)
			{
				epv->ws_nitrif[layer] = 1;
			}
			else
			{
				epv->ws_nitrif[layer] = (1 - WFPS) / (1 - sprop->opt2WFPS_nitrif) + (WFPS-sprop->opt2WFPS_nitrif)/(1-sprop->opt2WFPS_nitrif)* sprop->scalarWFPSmin_nitrif;
			}
		}
	}
		
	if (net_miner > 0)
	{
		sminNH4_to_nit= sprop->netMiner_to_nitrif * net_miner + 
										sprop->maxNitrif_rate * sminNH4avail * epv->ts_nitrif[layer] * epv->ws_nitrif[layer] * epv->ps_nitrif[layer];
	}
	else
	{
		sminNH4_to_nit = sprop->maxNitrif_rate * sminNH4avail *  epv->ts_nitrif[layer] * epv->ws_nitrif[layer] * epv->ps_nitrif[layer];
	}
		
	N2O_flux_NIT   = sminNH4_to_nit * sprop->N2Ocoeff_nitrif;



	
	*sminNH4_to_nitrif = sminNH4_to_nit;
	*N2O_flux_NITRIF   =  N2O_flux_NIT;

	return (errorCode);
}

int denitrification(double soiltype, double sminNO3avail_ppm, double pH, double WFPS, double SOMrespTOTAL, double* sminNO3_to_denitr, double* ratioN2_N2O)
{
	int errorCode = 0;
	double pDEN1, FrNO3, FrCO2, FrWFPS, FrPH;
	double FdNO3, FdWFPS, FdCO2, FdPH;
	double a,b,c,d,e;
	double denitr_flux, denitr_ratio;

	FrNO3=FrCO2=FrWFPS=FrPH=0;
	FdNO3=FdWFPS=FdCO2=FdPH=0;

	/* coarse: sand, loamy sand, sandy loam */
	if (soiltype <= 2)
	{
		a=1.56;
		b=12;
		c=16;
		d=2.01;
	}
	else
	{
		/* medium: loam types */
		if (soiltype <= 8)
		{
			a=4.82;
			b=14;
			c=16;
			d=1.39;
		}
		/* fine: clay types */
		else
		{
			a=60;
			b=18;
			c=22;
			d=1.06;
		}
	}
		
	FdNO3 = 11000+ (40000 * atan(PI*0.002*(sminNO3avail_ppm - 180)))/PI;
	
	e = c/pow(b,d*WFPS);
		
	FdWFPS = a / pow(b,e);
		
	FdCO2 = 24000/(1 + (200/exp(0.35*SOMrespTOTAL)));
		
		
	if (pH < 6.5)
	{
		if (pH <=3.5)
			FdPH = 0.001;
		else
			FdPH = (pH - 3.5)/3;				
	}
	else
		FdPH = 1;
	
	denitr_flux = MIN(FdNO3, FdCO2) * FdWFPS * FdPH;
		
	/* unit of denitr_flux: gN/ha/d -> kgN/m2/day */
	*sminNO3_to_denitr = denitr_flux * 0.001 * 0.0001;


	FrNO3 = (1 - (0.5 + ((atan(PI * 0.01 * (sminNO3avail_ppm - 190))) / PI))) * 25;
		
	FrCO2 = 13 + ((30.78 * atan(PI * 0.07 * (SOMrespTOTAL-13))) / PI);
	
	pDEN1 = pow(13,2.2*WFPS);
	FrWFPS = 1.4 / pow(13,17/pDEN1);
		
	FrPH = 1. / (1470 * exp(-1.1 * pH));

	denitr_ratio = MIN(FrNO3, FrCO2) * FrWFPS * FrPH;

	*ratioN2_N2O = denitr_ratio;

return (errorCode);
}
