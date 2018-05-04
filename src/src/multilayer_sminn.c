/* 
multilayer_sminn.c
Calculating the change in content of soil mineral nitrogen in multilayer soil (plant N upate, soil processes, 
depostion and fixing). 

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

int multilayer_sminn(const epconst_struct* epc, const epvar_struct* epv, const siteconst_struct* sitec, const cflux_struct* cf, 
	                 nstate_struct* ns, nflux_struct* nf)
{
	int ok=1;
	int layer=0;
	double NH4_prop, sminn_layer, diffNH4, diffNO3, change_ctrl,sminn_rootzone,layer_prop,net_miner,SOMresp, N2O_coeff_denitr;
	double p1_pH,p2_pH,p3_pH,p4_pH,pH_scalar;


	NH4_prop=sminn_layer=diffNH4=diffNO3=change_ctrl=sminn_rootzone=layer_prop=net_miner=SOMresp=N2O_coeff_denitr=0;

	p1_pH=0.15563;
	p2_pH=0.97859;
	p3_pH=2.91355;
	p4_pH=0.75220;

    
	/* *****************************************************************************************************

	0.Deposition and fixation
	1.Plant N uptake from SMINN: sminn_to_npool 
	2.Immobilization: sminn_to_soil_SUM - due microbial soil processes SMINN is changing in the soil (determined in daily_allocation).
	3 Mineralization
	4.Denitrification and Nitrification */


	/* 0. calulation of sminn contents in the whole soil */
	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{

		/*-----------------------------------------------------------------------------*/
		/* 1.A deposition: only in top soil layer */
		if (layer == 0) 
		{
			ns->sminNH4[0] += nf->ndep_to_sminn * epc->NdepNH4_coeff;
			ns->sminNO3[0] += nf->ndep_to_sminn * (1-epc->NdepNH4_coeff);

			ns->ndep_src     += nf->ndep_to_sminn;
		}

		/*-----------------------------------------------------------------------------*/
		/* 1.B fixation:  in root soil layer */
	
		ns->sminNH4[layer] += nf->nfix_to_sminn * epv->rootlength_prop[layer];
		ns->nfix_src     += nf->nfix_to_sminn * epv->rootlength_prop[layer];

	
		 /*-----------------------------------------------------------------------------*/
		/* NH4 pool is buffered to represent an exchangeable pool and a pool in solution that is available for plants */
		if (layer < epv->n_rootlayers) sminn_rootzone     += ns->sminNH4[layer] * epc->NH4_mobilen_prop + ns->sminNO3[layer] * epc->NO3_mobilen_prop;
	
		nf->sminn_to_soil_SUM[layer] = nf->sminn_to_soil1n_l1[layer]+nf->sminn_to_soil2n_l2[layer]+nf->sminn_to_soil3n_l4[layer] + 
			                           nf->sminn_to_soil2n_s1[layer]+nf->sminn_to_soil3n_s2[layer]+nf->sminn_to_soil4n_s3[layer];

	}

		
	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		/* sminn in the given layer */
		layer_prop  = 0;
		NH4_prop    = 0;
		sminn_layer = (ns->sminNH4[layer] * epc->NH4_mobilen_prop + ns->sminNO3[layer] * epc->NO3_mobilen_prop);

		if (sminn_rootzone && layer < epv->n_rootlayers) 
		{
			layer_prop = sminn_layer /sminn_rootzone;
		}
		
		if (sminn_layer) NH4_prop   = (ns->sminNH4[layer] * epc->NH4_mobilen_prop) / sminn_layer;

		/*-----------------------------------------------------------------------------*/
		/* 2.Plant N uptake from SMINN (only from rootzone) */
		nf->sminNH4_to_npool[layer]     = (nf->sminn_to_npool)  * layer_prop * NH4_prop;
		nf->sminNO3_to_npool[layer]     = (nf->sminn_to_npool)  * layer_prop * (1-NH4_prop);

		/* 3.Immobilization */ 
		nf->sminNH4_to_soil_SUM[layer] = nf->sminn_to_soil_SUM[layer] * NH4_prop;
		nf->sminNO3_to_soil_SUM[layer] = nf->sminn_to_soil_SUM[layer] * (1-NH4_prop);

  		/* 4.Mineralization: nf->soil4n_to_sminNH4[layer] - from recalcitrant SOM pool in daily allocation*/
	
		/* 2-4 CONTROL */
		diffNH4 = ns->sminNH4[layer] * epc->NH4_mobilen_prop - 
			      nf->sminNH4_to_npool[layer] - nf->sminNH4_to_soil_SUM[layer] + nf->soil4n_to_sminNH4[layer];
		diffNO3 = ns->sminNO3[layer] * epc->NO3_mobilen_prop - 
			      nf->sminNO3_to_npool[layer] - nf->sminNO3_to_soil_SUM[layer];
	
		if (diffNH4 < 0.0)       
		{	
			nf->sminNH4_to_soil_SUM[layer]+= diffNH4;
			nf->sminn_to_soil_SUM[layer]  += diffNH4;
			ns->nvol_snk                  += diffNH4;	
		}

		if (diffNO3 < 0.0)       
		{	
			nf->sminNO3_to_soil_SUM[layer]+= diffNO3;
			nf->sminn_to_soil_SUM[layer]  += diffNO3;
			ns->nvol_snk                  += diffNO3;	
		}

		/* 2-4 STATE UPDATE */
	
		ns->sminNH4[layer] = ns->sminNH4[layer] - nf->sminNH4_to_npool[layer] - nf->sminNH4_to_soil_SUM[layer] + nf->soil4n_to_sminNH4[layer];
		ns->sminNO3[layer] = ns->sminNO3[layer] - nf->sminNO3_to_npool[layer] - nf->sminNO3_to_soil_SUM[layer];

		/* control */
		if(ns->sminNH4[layer] < 0)
		{
			if (fabs (ns->sminNH4[layer]) > CRIT_PREC)
			{
				printf("\n");
				printf("FATAL ERROR: negative NH4 pool (multilayer_sminn.c)\n");
				ok=0;
			}
			else
			{
				ns->nvol_snk     += ns->sminNH4[layer];
				ns->sminNH4[layer] = 0;

			}

		}
	
		if(ns->sminNO3[layer] < 0)
		{
			if (fabs (ns->sminNO3[layer]) > CRIT_PREC)
			{
				printf("\n");
				printf("ERROR: negative NO3 pool (multilayer_sminn.c)\n");
				ok=0;
			}
			else
			{
				ns->nvol_snk     += ns->sminNO3[layer];
				ns->sminNO3[layer] = 0;

			}

		}

	
		ns->npool	      += nf->sminNH4_to_npool[layer] + nf->sminNO3_to_npool[layer];
	
		ns->soil1n[layer] += nf->sminn_to_soil1n_l1[layer];
		ns->soil2n[layer] += nf->sminn_to_soil2n_l2[layer] + nf->sminn_to_soil2n_s1[layer];
		ns->soil3n[layer] += nf->sminn_to_soil3n_l4[layer] + nf->sminn_to_soil3n_s2[layer];
		ns->soil4n[layer] += nf->sminn_to_soil4n_s3[layer] - nf->soil4n_to_sminNH4[layer];

		/* CONTROL */
		if ((ns->soil1n[layer] < 0))
		{
			if (fabs (ns->soil1n[layer]) > CRIT_PREC)
			{
				printf("\n");
				printf("ERROR: negative soil N pool (multilayer_sminn.c)\n");
				ok=0;
			}
			else
			{
				ns->sminNO3[layer] = 0;
			}
		}

		if ((ns->soil2n[layer] < 0))
		{
			if (fabs (ns->soil2n[layer]) > CRIT_PREC)
			{
				printf("\n");
				printf("ERROR: negative soil N pool (multilayer_sminn.c)\n");
				ok=0;
			}
			else
			{
				ns->soil2n[layer] = 0;
			}
		}

		if ((ns->soil3n[layer] < 0))
		{
			if (fabs (ns->soil3n[layer]) > CRIT_PREC)
			{
				printf("\n");
				printf("ERROR: negative soil N pool (multilayer_sminn.c)\n");
				ok=0;
			}
			else
			{
				ns->soil3n[layer] = 0;
			}
		}

		if ((ns->soil4n[layer] < 0))
		{
			if (fabs (ns->soil4n[layer]) > CRIT_PREC)
			{
				printf("\n");
				printf("ERROR: negative soil N pool (multilayer_sminn.c)\n");
				ok=0;
			}
			else
			{
				ns->soil4n[layer] = 0;
			}
		}

		/*-----------------------------------------------------------------------------*/
		/* 5. Nitrification */
		
		net_miner = nf->soil4n_to_sminNH4[layer] - nf->sminNH4_to_soil_SUM[layer];
				
		pH_scalar = p2_pH + (p1_pH-p2_pH)/(1 + exp((sitec->pH[layer]-p3_pH)/p4_pH));	
		
		if (net_miner > 0)
		{
			nf->sminNH4_to_nitrif[layer]  = epc->nitrif_coeff1 * net_miner + 
											epc->nitrif_coeff2 * (ns->sminNH4[layer] * epc->NH4_mobilen_prop) * 
											epv->t_scalar[layer] * epv->w_scalar[layer] * pH_scalar;
		}
		else
		{
			nf->sminNH4_to_nitrif[layer]  = epc->nitrif_coeff2 * (ns->sminNH4[layer] * epc->NH4_mobilen_prop) * 
						                    epv->t_scalar[layer] * epv->w_scalar[layer] * pH_scalar;
		}
		nf->N2O_flux_NITRIF[layer]    = nf->sminNH4_to_nitrif[layer] * epc->N2Ocoeff_nitrif;

		/* 6. denitrification */
		SOMresp = (cf->soil1_hr[layer] + cf->soil2_hr[layer] + cf->soil3_hr[layer] + cf->soil4_hr[layer])*1000;
		N2O_coeff_denitr  = 1. / (1 + 0.16 * epv->w_scalar[layer]);  /* Parton et al. 2001 */
		
		nf->sminNO3_to_denitr[layer] = epc->denitr_coeff * SOMresp * (ns->sminNO3[layer] * epc->NO3_mobilen_prop) * 
			                           (epv->vwc[layer] / sitec->vwc_sat[layer]);
		
		nf->N2O_flux_DENITR[layer]   = nf->sminNO3_to_denitr[layer] * N2O_coeff_denitr;
		nf->N2_flux_DENITR[layer]    = nf->sminNO3_to_denitr[layer] * (1 - N2O_coeff_denitr);


		/* 5-6 state update */
		ns->sminNH4[layer] -=  nf->sminNH4_to_nitrif[layer];
		ns->sminNO3[layer] += (nf->sminNH4_to_nitrif[layer]  - nf->N2O_flux_NITRIF[layer]);
		ns->nvol_snk       +=  nf->N2O_flux_NITRIF[layer];

        ns->sminNO3[layer] -=  nf->sminNO3_to_denitr[layer];
		ns->nvol_snk       +=  nf->sminNO3_to_denitr[layer];

		/* control */
		if(ns->sminNH4[layer] < 0)
		{
			if (fabs (ns->sminNH4[layer]) > CRIT_PREC)
			{
				printf("\n");
				printf("FATAL ERROR: negative NH4 pool (multilayer_sminn.c)\n");
				ok=0;
			}
			else
			{
				ns->nvol_snk     += ns->sminNH4[layer];
				ns->sminNH4[layer] = 0;

			}

		}
	
		if(ns->sminNO3[layer] < 0)
		{
			if (fabs (ns->sminNO3[layer]) > CRIT_PREC)
			{
				printf("FATAL ERROR: negative NO3 pool (multilayer_sminn.c)\n");
				ok=0;
			}
			else
			{
				ns->nvol_snk     += ns->sminNO3[layer];
				ns->sminNO3[layer] = 0;

			}

		}

		nf->N2_flux_DENITR_total		+= nf->N2_flux_DENITR[layer];
		nf->N2O_flux_DENITR_total		+= nf->N2O_flux_DENITR[layer];
		nf->N2O_flux_NITRIF_total		+= nf->N2O_flux_NITRIF[layer];
		nf->sminn_to_soil_SUM_total		+= nf->sminn_to_soil_SUM[layer];           
		nf->sminNH4_to_soil_SUM_total	+= nf->sminNH4_to_soil_SUM[layer];           
		nf->sminNO3_to_soil_SUM_total	+= nf->sminNO3_to_soil_SUM[layer];          
		nf->sminNO3_to_denitr_total		+= nf->sminNO3_to_denitr[layer];
		nf->sminNH4_to_nitrif_total		+= nf->sminNH4_to_nitrif[layer];
		nf->sminNH4_to_npool_total		+= nf->sminNH4_to_npool[layer];
		nf->sminNO3_to_npool_total		+= nf->sminNO3_to_npool[layer];
		nf->soil4n_to_sminNH4_total     += nf->soil4n_to_sminNH4[layer];
		change_ctrl  += layer_prop;
	}

	if ((sminn_rootzone) && fabs(1 - change_ctrl) > CRIT_PREC)
	{
		printf("\n");
		printf("ERROR in calculation of mineralized nitrogen in multilayer_sminn.c\n");
		ok=0;
	}


	
	return (!ok);
}

