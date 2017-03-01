/* 
summary.c
summary variables for potential output

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.4
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

int cnw_summary(int yday, cstate_struct* cs, cflux_struct* cf, nstate_struct* ns, nflux_struct* nf, wflux_struct* wf, epvar_struct* epv,
				summary_struct* summary)
{
	int ok=1;
	double gpp,mr,gr,hr,tr, fire;
	double sr; /* Hidy 2012 - calculating soil respiration */
	double npp,nep,nee, nbp, disturb_loss, disturb_gain;
	double ratio, litter_preday, change;

	/* annual summaruzed cumNPP - Hidy 2015 */
	if (yday == 0) summary->cum_npp_ann = 0;

	/* summarize carbon stocks */

	summary->abgc = cs->leafc+cs->fruitc+cs->softstemc;
	

	summary->vegc = cs->leafc + cs->leafc_storage + cs->leafc_transfer + 
		cs->frootc + cs->frootc_storage + cs->frootc_transfer +
		cs->livestemc + cs->livestemc_storage + cs->livestemc_transfer +
		cs->deadstemc + cs->deadstemc_storage + cs->deadstemc_transfer +
		cs->livecrootc + cs->livecrootc_storage + cs->livecrootc_transfer +
		cs->deadcrootc + cs->deadcrootc_storage + cs->deadcrootc_transfer +
		cs->gresp_storage + cs->gresp_transfer + 
		/* fruit simulation - Hidy 2013. */
		cs->fruitc + cs->fruitc_storage + cs->fruitc_transfer +
		/* softstem simulation - Hidy 2013. */
		cs->softstemc + cs->softstemc_storage + cs->softstemc_transfer;
	summary->litrc = cs->cwdc + cs->litr1c + cs->litr2c + cs->litr3c + 
		cs->litr4c;
	summary->soilc = cs->soil1c + cs->soil2c + cs->soil3c + cs->soil4c;
	summary->totalc = summary->vegc + summary->litrc + summary->soilc;

	summary->soiln = ns->soil1n + ns->soil2n + ns->soil3n + ns->soil4n;
	summary->sminn = ns->sminn[0]+ns->sminn[1]+ns->sminn[2]+ns->sminn[3]+ns->sminn[4]+ns->sminn[5];

	/* calculate daily NPP, positive for net growth */
	/* NPP = Gross PSN - Maintenance Resp - Growth Resp */
	gpp = cf->psnsun_to_cpool + cf->psnshade_to_cpool;
	
	mr = cf->leaf_day_mr + cf->leaf_night_mr + 
		 cf->froot_mr + cf->livestem_mr + cf->livecroot_mr +
		 /* fruit simulation - Hidy 2013. */
		 cf->fruit_mr +
		 /* softstem simulation - Hidy 2013. */
		 cf->softstem_mr;

	gr = cf->cpool_leaf_gr + cf->cpool_leaf_storage_gr + cf->transfer_leaf_gr +
		cf->cpool_froot_gr + cf->cpool_froot_storage_gr + cf->transfer_froot_gr + 
		cf->cpool_livestem_gr + cf->cpool_livestem_storage_gr + cf->transfer_livestem_gr +
		cf->cpool_deadstem_gr + cf->cpool_deadstem_storage_gr + cf->transfer_deadstem_gr + 
		cf->cpool_livecroot_gr + cf->cpool_livecroot_storage_gr + cf->transfer_livecroot_gr + 
		cf->cpool_deadcroot_gr + cf->cpool_deadcroot_storage_gr + cf->transfer_deadcroot_gr +
		/* fruit simulation - Hidy 2013. */
		cf->cpool_fruit_gr + cf->cpool_fruit_storage_gr + cf->transfer_fruit_gr +
		/* softstem simulation - Hidy 2013. */
		cf->cpool_softstem_gr + cf->cpool_softstem_storage_gr + cf->transfer_softstem_gr;

	npp = gpp - mr - gr;

	
	/* calculate daily NEP, positive for net sink */
	/* NEP = NPP - Autotrophic Resp */
	hr = cf->litr1_hr + cf->litr2_hr + cf->litr4_hr + cf->soil1_hr +
		cf->soil2_hr + cf->soil3_hr + cf->soil4_hr;

	tr = mr + gr + hr;
	
	nep = npp - hr;
	
	/* Hidy 2012 - calculating soil respiration */
	sr = cf->froot_mr + cf->livecroot_mr + 
		 cf->cpool_froot_gr + cf->cpool_froot_storage_gr + cf->transfer_froot_gr + 
		 cf->cpool_livecroot_gr + cf->cpool_livecroot_storage_gr + cf->transfer_livecroot_gr + 
		 cf->cpool_deadcroot_gr + cf->cpool_deadcroot_storage_gr + cf->transfer_deadcroot_gr +
		 hr;
		
	
	/* calculate daily NEE, positive for net sink */
	/* NEE = NEP - fire losses */
	fire =  cf->m_leafc_to_fire +  cf->m_leafc_storage_to_fire + cf->m_leafc_transfer_to_fire +
			cf->m_frootc_to_fire + cf->m_frootc_storage_to_fire + cf->m_frootc_transfer_to_fire +  
			cf->m_livestemc_to_fire + cf->m_livestemc_storage_to_fire + 	cf->m_livestemc_transfer_to_fire + 
			cf->m_deadstemc_to_fire + cf->m_deadstemc_storage_to_fire + cf->m_deadstemc_transfer_to_fire + 
			cf->m_livecrootc_to_fire + cf->m_livecrootc_storage_to_fire + cf->m_livecrootc_transfer_to_fire +
			cf->m_deadcrootc_to_fire + cf->m_deadcrootc_storage_to_fire + cf->m_deadcrootc_transfer_to_fire + 
			cf->m_gresp_storage_to_fire + cf->m_gresp_transfer_to_fire + 
			cf->m_litr1c_to_fire + cf->m_litr2c_to_fire + cf->m_litr3c_to_fire + cf->m_litr4c_to_fire +
			cf->m_cwdc_to_fire +
			/* fruit simulation - Hidy 2013. */
			cf->m_fruitc_to_fire +  cf->m_fruitc_storage_to_fire + cf->m_fruitc_transfer_to_fire +
			/* softstem simulation - Hidy 2013. */
			cf->m_softstemc_to_fire +  cf->m_softstemc_storage_to_fire + cf->m_softstemc_transfer_to_fire;
	nee = nep - fire;
	
	summary->daily_nep = nep;
	summary->daily_npp = npp;
	summary->daily_nee = -1 * nee;	// Hidy: NEE is positive if ecosystem is net source and negative if it is net sink
	summary->daily_gpp = gpp;
	summary->daily_mr = mr;
	summary->daily_gr = gr;
	summary->daily_hr = hr;
	summary->daily_sr = sr;
	summary->daily_tr = tr;
	summary->daily_fire = fire;
	summary->cum_npp_ann += npp;
	summary->cum_npp += npp;
	summary->cum_nep += nep;
	summary->cum_nee += -1*nee;
	summary->cum_gpp += gpp;
	summary->cum_mr += mr;
	summary->cum_gr += gr;
	summary->cum_hr += hr;
	summary->cum_fire += fire;

	summary->cum_nplus += nf->nplus;

	summary->cum_ET += wf->evapotransp;
	summary->vwc_annavg += epv->vwc_avg;
	
	/* LITTTER - Hidy 2015 */

	summary->daily_litter = cs->litr1c + cs->litr2c + cs->litr3c + cs->litr4c;
	
	summary->daily_litdecomp = cf->litr1c_to_soil1c + cf->litr2c_to_soil2c + cf->litr4c_to_soil3c + 
		                       cf->litr1_hr + cf->litr2_hr + cf->litr4_hr;

	summary->daily_litfire = cf->m_litr1c_to_fire + cf->m_litr2c_to_fire + cf->m_litr3c_to_fire + cf->m_litr4c_to_fire;
	
	/* aboveground litter */
	summary->daily_litfallc_above = 
		cf->m_leafc_to_litr1c + cf->m_leafc_to_litr2c + cf->m_leafc_to_litr3c + cf->m_leafc_to_litr4c + 
		cf->m_livestemc_to_cwdc + cf->m_deadstemc_to_cwdc + cf->m_livecrootc_to_cwdc + cf->m_deadcrootc_to_cwdc +
		cf->leafc_to_litr1c + cf->leafc_to_litr2c + cf->leafc_to_litr3c + cf->leafc_to_litr4c + 
		/* SOFT STEM SIMULATION  of non-woody biomes - Hidy 2015. */
		cf->softstemc_to_litr1c + cf->softstemc_to_litr2c + cf->softstemc_to_litr3c + cf->softstemc_to_litr4c + 
		cf->m_softstemc_to_litr1c + cf->m_softstemc_to_litr2c + cf->m_softstemc_to_litr3c + cf->m_softstemc_to_litr4c + 
		/* fruit simulation - Hidy 2013. */
		cf->fruitc_to_litr1c + cf->fruitc_to_litr2c + cf->fruitc_to_litr3c + cf->fruitc_to_litr4c + 
		cf->m_fruitc_to_litr1c + cf->m_fruitc_to_litr2c + cf->m_fruitc_to_litr3c + cf->m_fruitc_to_litr4c +
		/* senescence simulation - Hidy 2015 */
		cf->SNSC_to_litr1c + cf->SNSC_to_litr2c + cf->SNSC_to_litr3c + cf->SNSC_to_litr4c +
		/* management simulation - Hidy 2015 */
		cf->MOW_to_litr1c + cf->MOW_to_litr2c + cf->MOW_to_litr3c + cf->MOW_to_litr4c+
		cf->GRZ_to_litr1c + cf->GRZ_to_litr2c + cf->GRZ_to_litr3c + cf->GRZ_to_litr4c+
		cf->HRV_to_litr1c + cf->HRV_to_litr2c + cf->HRV_to_litr3c + cf->HRV_to_litr4c+
		cf->THN_to_litr1c + cf->THN_to_litr2c + cf->THN_to_litr3c + cf->THN_to_litr4c;

	/* belowground */
	summary->daily_litfallc_below = 
		cf->m_frootc_to_litr1c + cf->m_frootc_to_litr2c + cf->m_frootc_to_litr3c + cf->m_frootc_to_litr4c +
		cf->m_leafc_storage_to_litr1c + cf->m_frootc_storage_to_litr1c +
		cf->m_leafc_transfer_to_litr1c + cf->m_frootc_transfer_to_litr1c +		
		cf->m_livestemc_storage_to_litr1c + cf->m_deadstemc_storage_to_litr1c +
		cf->m_livestemc_transfer_to_litr1c + cf->m_deadstemc_transfer_to_litr1c +
		cf->m_livecrootc_storage_to_litr1c + cf->m_deadcrootc_storage_to_litr1c +
		cf->m_livecrootc_transfer_to_litr1c + cf->m_deadcrootc_transfer_to_litr1c +
		cf->m_gresp_storage_to_litr1c + cf->m_gresp_transfer_to_litr1c +
		cf->frootc_to_litr1c + cf->frootc_to_litr2c + cf->frootc_to_litr3c + cf->frootc_to_litr4c +
		/* SOFT STEM SIMULATION  of non-woody biomes - Hidy 2015. */
		cf->m_softstemc_storage_to_litr1c + cf->m_softstemc_transfer_to_litr1c + 
		/* fruit simulation - Hidy 2013. */
		cf->m_fruitc_storage_to_litr1c + cf->m_fruitc_transfer_to_litr1c +
		/* ploughing */
		cf->PLG_to_litr1c + cf->PLG_to_litr2c + cf->PLG_to_litr3c + cf->PLG_to_litr4c;


	summary->daily_litfallc = summary->daily_litfallc_above + summary->daily_litfallc_below;

	/* --------------------------------------------------------------------------------*/
	/* below- and aboveground litter calculation - PLOUGHING AND PLANTING EFFECT!!! */
	/* aboveground and belowground litter calculation: above , below */
	

	litter_preday = cs->litr_aboveground + cs->litr_belowground;
	change = summary->daily_litter - litter_preday;

	if (change > 0 && summary->daily_litfallc > 0) 
	{
	
		ratio = summary->daily_litfallc_above/summary->daily_litfallc;
		cs->litr_aboveground += (summary->daily_litter - litter_preday) * ratio;

		ratio = summary->daily_litfallc_below/summary->daily_litfallc;
		cs->litr_belowground += (summary->daily_litter - litter_preday) * ratio;

		if (cs->litr_aboveground < 0) 
		{
			cs->litr_belowground += cs->litr_aboveground;
			cs->litr_aboveground = 0;
		}

	}
	else
	{
		if (litter_preday > 0)
		{
			cs->litr_aboveground += cs->litr_aboveground/litter_preday * change;
			cs->litr_belowground += cs->litr_belowground/litter_preday * change;
		}
		else
		{
			cs->litr_aboveground += 0.5 * change;
			cs->litr_belowground += 0.5 * change;
		}
	}


	if (cf->leafc_to_PLG > 0)
	{
		cs->litr_belowground += cs->litr_aboveground;
		cs->litr_aboveground  = 0;
	}

	/* --------------------------------------------------------------------------------*/
	
	/* ditrurbance is positive is it mean net carbon gain to the system and it is negative is net carbon loss */
	/* summerize thinning effect - Hidy 2012 */
	summary->Cchange_THN =  cf->THN_to_litr1c + cf->THN_to_litr2c + cf->THN_to_litr3c + cf->THN_to_litr4c + cf->THN_to_cwdc - 
								 cf->leafc_storage_to_THN - cf->leafc_transfer_to_THN - cf->leafc_to_THN -
								 /* fruit simulation - Hidy 2013. */
								 cf->fruitc_storage_to_THN - cf->fruitc_transfer_to_THN - cf->fruitc_to_THN -
								 cf->frootc_storage_to_THN - cf->frootc_transfer_to_THN - cf->frootc_to_THN -
								 cf->livecrootc_storage_to_THN - cf->livecrootc_transfer_to_THN - cf->livecrootc_to_THN -
								 cf->deadcrootc_storage_to_THN - cf->deadcrootc_transfer_to_THN - cf->deadcrootc_to_THN -
								 cf->livestemc_storage_to_THN - cf->livestemc_transfer_to_THN - cf->livestemc_to_THN -
								 cf->deadstemc_storage_to_THN - cf->deadstemc_transfer_to_THN - cf->deadstemc_to_THN -
								 cf->gresp_transfer_to_THN - cf->gresp_storage_to_THN;

	/* summerize mowing effect - Hidy 2008 */
	summary->Cchange_MOW =  cf->MOW_to_litr1c + cf->MOW_to_litr2c + cf->MOW_to_litr3c + cf->MOW_to_litr4c - 
								 cf->leafc_storage_to_MOW - cf->leafc_transfer_to_MOW - cf->leafc_to_MOW -
								 /* fruit simulation - Hidy 2013. */
								 cf->fruitc_storage_to_MOW - cf->fruitc_transfer_to_MOW - cf->fruitc_to_MOW -
								 /* softstem simulation - Hidy 2013. */
								 cf->softstemc_storage_to_MOW - cf->softstemc_transfer_to_MOW - cf->softstemc_to_MOW -
								 cf->gresp_transfer_to_MOW - cf->gresp_storage_to_MOW - cf->STDBc_to_MOW;

	/* summerize harvesting effect - Hidy 2008 */
	summary->Cchange_HRV = - cf->leafc_storage_to_HRV - cf->leafc_transfer_to_HRV - cf->leafc_to_HRV -
							/* fruit simulation - Hidy 2013. */
							 cf->fruitc_storage_to_HRV - cf->fruitc_transfer_to_HRV - cf->fruitc_to_HRV -
							 /* softstem simulation - Hidy 2013. */
							 cf->softstemc_storage_to_HRV - cf->softstemc_transfer_to_HRV - cf->softstemc_to_HRV -
		                     cf->gresp_transfer_to_HRV - cf->gresp_storage_to_HRV;

	/* summerize ploughing effect - Hidy 2008 */
	summary->Cchange_PLG = -cf->leafc_storage_to_PLG - cf->leafc_transfer_to_PLG - cf->leafc_to_PLG -
							/* fruit simulation - Hidy 2013. */
							cf->fruitc_storage_to_PLG - cf->fruitc_transfer_to_PLG - cf->fruitc_to_PLG -
							/* softstem simulation - Hidy 2013. */
							cf->softstemc_storage_to_PLG - cf->softstemc_transfer_to_PLG - cf->softstemc_to_PLG -
							cf->frootc_storage_to_PLG - cf->frootc_transfer_to_PLG - cf->frootc_to_PLG - 
							cf->gresp_transfer_to_PLG - cf->gresp_storage_to_PLG;

	/* summerize grazing effect - Hidy 2009 */
	summary->Cchange_GRZ =  cf->GRZ_to_litr1c + cf->GRZ_to_litr2c + cf->GRZ_to_litr3c + cf->GRZ_to_litr4c - 
								cf->leafc_storage_to_GRZ - cf->leafc_transfer_to_GRZ - cf->leafc_to_GRZ - 
								/* fruit simulation - Hidy 2013. */
								cf->fruitc_storage_to_GRZ - cf->fruitc_transfer_to_GRZ - cf->fruitc_to_GRZ - 
								/* softstem simulation - Hidy 2013. */
								cf->softstemc_storage_to_GRZ - cf->softstemc_transfer_to_GRZ - cf->softstemc_to_GRZ - 
								cf->gresp_transfer_to_GRZ - cf->gresp_storage_to_GRZ;


	summary->Cchange_FRZ = cf->FRZ_to_litr1c + cf->FRZ_to_litr2c + cf->FRZ_to_litr3c + cf->FRZ_to_litr4c;

	summary->Cchange_PLT = cf->leafc_transfer_from_PLT + cf->frootc_transfer_from_PLT + 
							/* fruit simulation - Hidy 2013. */
							cf->fruitc_transfer_from_PLT +
							/* softstem simulation - Hidy 2013. */
							cf->softstemc_transfer_from_PLT;

	/* summerize senescence effect - Hidy 2008 */
	summary->Cchange_SNSC =  cf->SNSC_to_litr1c + cf->SNSC_to_litr2c + cf->SNSC_to_litr3c + cf->SNSC_to_litr4c - 
								 cf->m_leafc_storage_to_SNSC - cf->m_leafc_transfer_to_SNSC - cf->m_leafc_to_SNSC -
								 /* fruit simulation - Hidy 2013. */
								 cf->m_fruitc_storage_to_SNSC - cf->m_fruitc_transfer_to_SNSC - cf->m_fruitc_to_SNSC -
								 /* softstem simulation - Hidy 2013. */
								 cf->m_softstemc_storage_to_SNSC - cf->m_softstemc_transfer_to_SNSC - cf->m_softstemc_to_SNSC -
								 cf->m_frootc_storage_to_SNSC - cf->m_frootc_transfer_to_SNSC - cf->m_frootc_to_SNSC -
								 cf->m_gresp_transfer_to_SNSC - cf->m_gresp_storage_to_SNSC;

	disturb_loss = cf->leafc_storage_to_MOW + cf->leafc_transfer_to_MOW + cf->leafc_to_MOW +  cf->gresp_transfer_to_MOW + cf->gresp_storage_to_MOW +
				   cf->leafc_storage_to_HRV + cf->leafc_transfer_to_HRV + cf->leafc_to_HRV + cf->gresp_transfer_to_HRV + cf->gresp_storage_to_HRV +
				   cf->leafc_storage_to_PLG + cf->leafc_transfer_to_PLG + cf->leafc_to_PLG + cf->gresp_transfer_to_PLG + cf->gresp_storage_to_PLG +
				   cf->leafc_storage_to_GRZ + cf->leafc_transfer_to_GRZ + cf->leafc_to_GRZ + cf->gresp_transfer_to_GRZ + cf->gresp_storage_to_GRZ +
				   cf->frootc_to_PLG + cf->frootc_storage_to_PLG + cf->frootc_transfer_to_PLG + 
				   cf->STDBc_to_MOW +
				     /* fruit simulation - Hidy 2013. */
				   cf->fruitc_storage_to_MOW + cf->fruitc_transfer_to_MOW + cf->fruitc_to_MOW + 
				   cf->fruitc_storage_to_HRV + cf->fruitc_transfer_to_HRV + cf->fruitc_to_HRV + 
				   cf->fruitc_storage_to_PLG + cf->fruitc_transfer_to_PLG + cf->fruitc_to_PLG + 
				   cf->fruitc_storage_to_GRZ + cf->fruitc_transfer_to_GRZ + cf->fruitc_to_GRZ +
				     /* softstem simulation - Hidy 2013. */
				   cf->softstemc_storage_to_MOW + cf->softstemc_transfer_to_MOW + cf->softstemc_to_MOW + 
				   cf->softstemc_storage_to_HRV + cf->softstemc_transfer_to_HRV + cf->softstemc_to_HRV + 
				   cf->softstemc_storage_to_PLG + cf->softstemc_transfer_to_PLG + cf->softstemc_to_PLG + 
				   cf->softstemc_storage_to_GRZ + cf->softstemc_transfer_to_GRZ + cf->softstemc_to_GRZ;
				
	disturb_gain = cf->MOW_to_litr1c + cf->MOW_to_litr2c + cf->MOW_to_litr3c + cf->MOW_to_litr4c + 
					  cf->GRZ_to_litr1c + cf->GRZ_to_litr2c + cf->GRZ_to_litr3c + cf->GRZ_to_litr4c + 
					  cf->FRZ_to_litr1c + cf->FRZ_to_litr2c + cf->FRZ_to_litr3c + cf->FRZ_to_litr4c + 
                      cf->leafc_transfer_from_PLT + cf->frootc_transfer_from_PLT + cf->frootc_transfer_from_PLT;
	
	/* nbp is positive is it mean net carbon gain to the system and it is negative is net carbon loss */
	nbp = nep + disturb_gain - disturb_loss;
	summary->daily_nbp = nbp;

	/* main nitrogen information */
	summary->Nplus_GRZ = (nf->GRZ_to_litr1n + nf->GRZ_to_litr2n + nf->GRZ_to_litr3n  + nf->GRZ_to_litr4n);  
	summary->Nplus_FRZ = (nf->FRZ_to_sminn+nf->FRZ_to_litr1n + nf->FRZ_to_litr2n + nf->FRZ_to_litr3n  + nf->FRZ_to_litr4n);  
	

	
	return(!ok);
}
