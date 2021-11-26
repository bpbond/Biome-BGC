/* 
summary.c
summary variables for potential output

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

int cnw_summary(int yday, const epconst_struct* epc, const siteconst_struct* sitec, const soilprop_struct* sprop, const metvar_struct* metv, 
	            const cstate_struct* cs, const cflux_struct* cf, const nstate_struct* ns, const nflux_struct* nf, const wflux_struct* wf, 
				epvar_struct* epv, summary_struct* summary)
{
	int errorCode=0;
	int layer;
	double gpp,mr,gr,hr,tr, fire;
	double N2O_Ceq, CH4_Ceq;
	double sr; /* calculating soil respiration */
	double npp,nep,nee, nbp, disturb_loss, disturb_gain, BD_top30, BD_act, g_per_cm3_to_kg_per_m3, prop_to_percent;
	double Closs_THN_w, Closs_THN_nw, Closs_MOW, Closs_HRV, yieldC_HRV, Closs_PLG, Closs_PLT, Closs_GRZ, Cplus_PLT, Cplus_FRZ, Cplus_GRZ, Nplus_GRZ, Nplus_FRZ;
	double Closs_SNSC, daily_STDB_to_litr, daily_CTDB_to_litr;

	summary->leafCN = summary->frootCN = summary->fruitCN = summary->softstemCN =0;
	/*******************************************************************************/
	/* 0. cumulative SUMS: zero at yday 0 */

	if (yday == 0) 
	{
	
		summary->annprcp     = 0;
		summary->anntavg     = 0;
		summary->cum_runoff   = 0;
		summary->cum_WleachRZ  = 0;
		summary->cum_NleachRZ  = 0;
		summary->cum_npp  = 0;
		summary->cum_nep  = 0;
		summary->cum_nee  = 0;
		summary->cum_gpp  = 0;
		summary->cum_ngb  = 0;
		summary->cum_mr  = 0;
		summary->cum_gr  = 0;
		summary->cum_hr  = 0;
		summary->cum_tr  = 0;
		summary->cum_sr  = 0;
		summary->cum_n2o  = 0;
		summary->cum_Closs_MGM  = 0;
		summary->cum_Cplus_MGM  = 0;
		summary->cum_Closs_THN_w = 0;
		summary->cum_Closs_THN_nw = 0;
		summary->cum_Closs_MOW  = 0;

		summary->cum_Closs_GRZ  = 0;
		summary->cum_Cplus_GRZ  = 0;
		summary->cum_Cplus_FRZ  = 0;
		
		summary->cum_Cplus_PLT  = 0;
		summary->cum_Closs_PLT  = 0;
		summary->cum_Closs_HRV  = 0;
		summary->cum_yieldC_HRV = 0;
		summary->cum_Closs_PLG  = 0;

		summary->cum_Nplus_GRZ  = 0;
		summary->cum_Nplus_FRZ  = 0;
		summary->cum_Closs_SNSC  = 0;
		summary->cum_Cplus_STDB  = 0;
		summary->cum_Cplus_CTDB  = 0;
		summary->cum_evap  = 0;
		summary->cum_transp  = 0;
		summary->cum_ET  = 0;
		epv->cumNstress = 0;
		epv->cumSWCstress = 0;
	}
	
	/*******************************************************************************/
	/* 1. summarize meteorological and water variables */


	summary->annprcp += metv->prcp;
	summary->anntavg += metv->tavg / nDAYS_OF_YEAR;

		
	summary->cum_runoff += wf->prcp_to_runoff + wf->pondw_to_runoff;
	summary->cum_WleachRZ += wf->soilw_leached_RZ;	
	summary->cum_NleachRZ += nf->sminN_leached_RZ * 1000000;

	/*******************************************************************************/
	/* 2. summarize carbon and nitrogen stocks */


	/* aboveground biomass (live+dead) without NSC */
	summary->LDaboveC_nw      = cs->leafc + cs->fruitc + cs->softstemc + 
		                        cs->STDBc_leaf  + cs->STDBc_fruit + cs->STDBc_softstem;
	summary->LDaboveC_w       = cs->livestemc + cs->deadstemc;

	/* aboveground biomass (live+dead) with NSC */
	summary->LDaboveCnsc_nw = cs->leafc + cs->fruitc + cs->softstemc       + 
		                      cs->STDBc_above + 
		                      cs->leafc_transfer + cs->leafc_storage   + cs->frootc_storage + cs->frootc_transfer +
						      cs->fruitc_storage + cs->fruitc_transfer +  cs->softstemc_storage + cs->softstemc_transfer +
		                      cs->gresp_storage + cs->gresp_transfer;
	summary->LDaboveCnsc_w = cs->livestemc + cs->deadstemc +
		                   cs->livestemc_storage + cs->livestemc_transfer +
		                   cs->deadstemc_storage + cs->deadstemc_transfer +
		                   cs->livecrootc_storage + cs->livecrootc_transfer +
		                   cs->deadcrootc_storage + cs->deadcrootc_transfer;


	/* living aboveground biomass  */
	summary->LaboveC_nw = cs->leafc+cs->fruitc+cs->softstemc;
	summary->LaboveC_w = cs->livestemc;

	/* living aboveground biomass with NSC */
	summary->LaboveCnsc_nw = cs->leafc + cs->fruitc + cs->softstemc + 
		                     cs->leafc_transfer + cs->leafc_storage   + cs->frootc_storage + cs->frootc_transfer +
						     cs->fruitc_storage + cs->fruitc_transfer +  cs->softstemc_storage + cs->softstemc_transfer +
		                     cs->gresp_storage + cs->gresp_transfer;
	summary->LaboveCnsc_w = cs->livestemc + cs->livestemc_storage + cs->livestemc_transfer;
	


	/* dead aboveground biomass  */
	summary->DaboveC_nw = cs->STDBc_leaf  + cs->STDBc_fruit + cs->STDBc_softstem;
	summary->DaboveC_w = cs->deadstemc;

	/* dead aboveground biomass with NSC  */
	summary->DaboveCnsc_nw = cs->STDBc_leaf  + cs->STDBc_fruit + cs->STDBc_softstem;
	summary->DaboveCnsc_w = cs->deadstemc + cs->deadstemc_storage + cs->deadstemc_transfer;



	summary->vegC = cs->leafc + cs->leafc_storage + cs->leafc_transfer + 
		            cs->frootc + cs->frootc_storage + cs->frootc_transfer +
					cs->fruitc + cs->fruitc_storage + cs->fruitc_transfer +
		            cs->softstemc + cs->softstemc_storage + cs->softstemc_transfer +
		            cs->livestemc + cs->livestemc_storage + cs->livestemc_transfer +
		            cs->deadstemc + cs->deadstemc_storage + cs->deadstemc_transfer +
		            cs->livecrootc + cs->livecrootc_storage + cs->livecrootc_transfer +
		            cs->deadcrootc + cs->deadcrootc_storage + cs->deadcrootc_transfer +
		            cs->gresp_storage + cs->gresp_transfer + 
					cs->STDBc_above + cs->STDBc_below;

			

	summary->leafc_LandD     = cs->leafc     + cs->STDBc_leaf;
	summary->frootc_LandD    = cs->frootc    + cs->STDBc_froot;
	summary->fruitc_LandD    = cs->fruitc    + cs->STDBc_fruit;
	summary->softstemc_LandD = cs->softstemc + cs->STDBc_softstem;

	summary->leafDM     =  summary->leafc_LandD / epc->leafC_DM;
    summary->frootDM    =  summary->frootc_LandD / epc->frootC_DM;
	summary->fruitDM    =  summary->fruitc_LandD / epc->fruitC_DM;
    summary->softstemDM =  summary->softstemc_LandD / epc->softstemC_DM;


	if (ns->leafn + ns->STDBn_leaf != 0)         summary->leafCN     = (cs->leafc + cs->STDBc_leaf) / (ns->leafn + ns->STDBn_leaf);
	if (ns->frootn + ns->STDBn_froot != 0)       summary->frootCN    = (cs->frootc + cs->STDBc_froot) / (ns->frootn + ns->STDBn_froot);
	if (ns->fruitn + ns->STDBn_fruit != 0)       summary->fruitCN    = (cs->fruitc + cs->STDBc_fruit) / (ns->fruitn + ns->STDBn_fruit);
	if (ns->softstemn + ns->STDBn_softstem != 0) summary->softstemCN = (cs->softstemc + cs->STDBc_softstem) / (ns->softstemn + ns->STDBn_softstem);

	summary->yieldDM_HRV = cs->fruitC_HRV / epc->fruitC_DM;

	summary->leaflitrDM = (cs->litr1c_total + cs->litr2c_total + cs->litr3c_total + cs->litr4c_total) / epc->leaflitrC_DM;
    summary->livewoodDM = (cs->livestemc + cs->livecrootc) / epc->livewoodC_DM;
	summary->deadwoodDM = (cs->deadstemc + cs->deadcrootc) / epc->deadwoodC_DM;



	summary->litrC_total = cs->litr1c_total + cs->litr2c_total + cs->litr3c_total + cs->litr4c_total;
	summary->litrN_total = ns->litr1n_total + ns->litr2n_total + ns->litr3n_total + ns->litr4n_total;

	summary->litrCwdC_total = cs->litr1c_total + cs->litr2c_total + cs->litr3c_total + cs->litr4c_total + cs->cwdc_total;
	summary->litrCwdN_total = ns->litr1n_total + ns->litr2n_total + ns->litr3n_total + ns->litr4n_total + ns->cwdn_total;;
	
	summary->soilC_total = cs->soil1c_total + cs->soil2c_total + cs->soil3c_total + cs->soil4c_total;
	summary->soilN_total = ns->soil1n_total + ns->soil2n_total + ns->soil3n_total + ns->soil4n_total;
	
	if (summary->litrN_total) 
		summary->CNlitr_total = summary->litrC_total/summary->litrN_total;
	else
		summary->CNlitr_total = 0;

	if (summary->soilN_total) 
		summary->CNsoil_total = summary->soilC_total/summary->soilN_total;
	else
		summary->CNsoil_total = 0;

	summary->sminN_total = ns->sminNH4_total + ns->sminNO3_total;
	summary->sminNavail_total = ns->sminNH4_total * sprop->NH4_mobilen_prop + ns->sminNO3_total* NO3_mobilen_prop;
	
	summary->totalC      = summary->vegC + summary->litrC_total + summary->soilC_total + cs->cwdc_total;

	/* carbon and nitrogen content of top soil layer (10 cm layer depth):
	   kg (C or N)/m2 -> g (C or N) / kg soil: kgC/m2 = kgCN/0.1m3 = 10 * kgCN/m3 */

	g_per_cm3_to_kg_per_m3 = 1000;
	prop_to_percent = 100;
	
	summary->sminN_maxRZ = 0;
	summary->soilC_maxRZ = 0;
	summary->soilN_maxRZ = 0;
	summary->litrC_maxRZ = 0;
	summary->litrN_maxRZ = 0;
	summary->sminNavail_maxRZ = 0;

	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		/* sminNH4: kgN/m2; BD: g/cm3 -> kg/m3: *10-3; ppm: *1000000 */
		summary->sminNH4_ppm[layer] = ns->sminNH4[layer] / (sprop->BD[layer] * g_per_cm3_to_kg_per_m3 * sitec->soillayer_thickness[layer]) * 1000000;
		summary->sminNO3_ppm[layer] = ns->sminNO3[layer] / (sprop->BD[layer] * g_per_cm3_to_kg_per_m3 * sitec->soillayer_thickness[layer]) * 1000000;
		if (layer < epv->n_maxrootlayers) 
		{
			summary->sminN_maxRZ += (ns->sminNH4[layer] + ns->sminNO3[layer]);
			summary->sminNavail_maxRZ += (ns->sminNH4avail[layer] + ns->sminNO3avail[layer]);
			summary->soilC_maxRZ += (cs->soilC[layer]);
			summary->soilN_maxRZ += (ns->soilN[layer]);
			summary->litrC_maxRZ += (cs->litrC[layer]);
			summary->litrN_maxRZ += (ns->litrN[layer]);
		}
		BD_act=sprop->BD[layer] * g_per_cm3_to_kg_per_m3 * sitec->soillayer_thickness[layer];
		summary->SOM_C[layer]=(cs->soil1c[layer] + cs->soil2c[layer] + cs->soil3c[layer] + cs->soil4c[layer]) / BD_act * prop_to_percent;
	}

	/* g/cm3 to kg/m2 */
	BD_top30 = (sitec->soillayer_thickness[0] * sprop->BD[0] + 
		        sitec->soillayer_thickness[1] * sprop->BD[1] +  
				sitec->soillayer_thickness[2] * sprop->BD[2]) * g_per_cm3_to_kg_per_m3;

	summary->stableSOC_top30 = (cs->soil4c[0] + cs->soil4c[1] + cs->soil4c[2]) / BD_top30 * prop_to_percent;

	summary->SOM_C_top30 = (cs->soil1c[0] + cs->soil1c[1] + cs->soil1c[2] +
						    cs->soil2c[0] + cs->soil2c[1] + cs->soil2c[2] +
						    cs->soil3c[0] + cs->soil3c[1] + cs->soil3c[2] +
						    cs->soil4c[0] + cs->soil4c[1] + cs->soil4c[2]) / BD_top30 * prop_to_percent;

	summary->SOM_N_top30 = (ns->soil1n[0] + ns->soil1n[1] + ns->soil1n[2] +
							ns->soil2n[0] + ns->soil2n[1] + ns->soil2n[2] +
							ns->soil3n[0] + ns->soil3n[1] + ns->soil3n[2] +
							ns->soil4n[0] + ns->soil4n[1] + ns->soil4n[2]) / BD_top30 * prop_to_percent;


	summary->NH4_top30avail = (summary->sminNH4_ppm[0] * sitec->soillayer_thickness[0]/sitec->soillayer_depth[2] + 
		                       summary->sminNH4_ppm[1] * sitec->soillayer_thickness[1]/sitec->soillayer_depth[2] + 
							   summary->sminNH4_ppm[2] * sitec->soillayer_thickness[2]/sitec->soillayer_depth[2]) * sprop->NH4_mobilen_prop;

	summary->NO3_top30avail = (summary->sminNO3_ppm[0] * sitec->soillayer_thickness[0]/sitec->soillayer_depth[2] + 
		                       summary->sminNO3_ppm[1] * sitec->soillayer_thickness[1]/sitec->soillayer_depth[2] + 
							   summary->sminNO3_ppm[2] * sitec->soillayer_thickness[2]/sitec->soillayer_depth[2]) * NO3_mobilen_prop;

	summary->sminN_top30avail = summary->NO3_top30avail + summary->NH4_top30avail;

	summary->daily_n2o = nf->N2O_flux_NITRIF_total + nf->N2O_flux_DENITR_total + nf->N2O_flux_GRZ + nf->N2O_flux_FRZ;

	summary->CH4_flux_TOTAL = cf->CH4_flux_ANIMAL + cf->CH4_flux_MANURE + cf->CH4_flux_soil;

	/*******************************************************************************/
	/* 3. calculate daily fluxes (GPP, NPP, NEP, MR, GR, HR) positive for net growth: NPP = Gross PSN - Maintenance Resp - Growth Resp */

	gpp = cf->psnsun_to_cpool + cf->psnshade_to_cpool;
	
	mr = cf->leaf_day_mr + cf->leaf_night_mr + 
		 cf->froot_mr + cf->livestem_mr + cf->livecroot_mr +
		 /* fruit simulation */
		 cf->fruit_mr +
		 /* softstem simulation */
		 cf->softstem_mr;

	gr = cf->cpool_leaf_gr + cf->cpool_leaf_storage_gr + cf->transfer_leaf_gr +
		cf->cpool_froot_gr + cf->cpool_froot_storage_gr + cf->transfer_froot_gr + 
		cf->cpool_livestem_gr + cf->cpool_livestem_storage_gr + cf->transfer_livestem_gr +
		cf->cpool_deadstem_gr + cf->cpool_deadstem_storage_gr + cf->transfer_deadstem_gr + 
		cf->cpool_livecroot_gr + cf->cpool_livecroot_storage_gr + cf->transfer_livecroot_gr + 
		cf->cpool_deadcroot_gr + cf->cpool_deadcroot_storage_gr + cf->transfer_deadcroot_gr +
		/* fruit simulation */
		cf->cpool_fruit_gr + cf->cpool_fruit_storage_gr + cf->transfer_fruit_gr +
		/* softstem simulation */
		cf->cpool_softstem_gr + cf->cpool_softstem_storage_gr + cf->transfer_softstem_gr;

	npp = gpp - mr - gr;

	/* heterotrophic respiration  */
	hr=0;
	summary->litr1HR_total=0;
	summary->litr2HR_total=0;
	summary->litr4HR_total=0;
	summary->soil1HR_total=0;
	summary->soil2HR_total=0;
	summary->soil3HR_total=0;
	summary->soil4HR_total=0;

	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		summary->litr1HR_total += cf->litr1_hr[layer];
		summary->litr2HR_total += cf->litr2_hr[layer];
		summary->litr4HR_total += cf->litr4_hr[layer];
		summary->soil1HR_total += cf->soil1_hr[layer];
		summary->soil2HR_total += cf->soil2_hr[layer];
		summary->soil3HR_total += cf->soil3_hr[layer];
		summary->soil4HR_total += cf->soil4_hr[layer];

		hr += cf->litr1_hr[layer] + cf->litr2_hr[layer] + cf->litr4_hr[layer] + 
			  cf->soil1_hr[layer] + cf->soil2_hr[layer] + cf->soil3_hr[layer] + cf->soil4_hr[layer];
	}

	tr = mr + gr + hr;
	
	nep = npp - hr;
	
	/* soil respiration */
	sr = cf->froot_mr + cf->livecroot_mr +
		 cf->cpool_froot_gr + cf->cpool_froot_storage_gr + cf->transfer_froot_gr + 
		 cf->cpool_livecroot_gr + cf->cpool_livecroot_storage_gr + cf->transfer_livecroot_gr + 
		 cf->cpool_deadcroot_gr + cf->cpool_deadcroot_storage_gr + cf->transfer_deadcroot_gr +
		 hr;
		
	
	/* calculate daily NEE, positive for net sink: NEE = NEP - fire losses */
	fire =  cf->m_leafc_to_fire +  cf->m_leafc_storage_to_fire + cf->m_leafc_transfer_to_fire +
			cf->m_frootc_to_fire + cf->m_frootc_storage_to_fire + cf->m_frootc_transfer_to_fire +  
			cf->m_livestemc_to_fire + cf->m_livestemc_storage_to_fire + 	cf->m_livestemc_transfer_to_fire + 
			cf->m_deadstemc_to_fire + cf->m_deadstemc_storage_to_fire + cf->m_deadstemc_transfer_to_fire + 
			cf->m_livecrootc_to_fire + cf->m_livecrootc_storage_to_fire + cf->m_livecrootc_transfer_to_fire +
			cf->m_deadcrootc_to_fire + cf->m_deadcrootc_storage_to_fire + cf->m_deadcrootc_transfer_to_fire + 
			cf->m_gresp_storage_to_fire + cf->m_gresp_transfer_to_fire + 
			cf->m_litr1c_to_fireTOTAL + cf->m_litr2c_to_fireTOTAL + cf->m_litr3c_to_fireTOTAL + cf->m_litr4c_to_fireTOTAL +
			cf->m_cwdc_to_fireTOTAL +
			/* fruit simulation */
			cf->m_fruitc_to_fire +  cf->m_fruitc_storage_to_fire + cf->m_fruitc_transfer_to_fire +
			/* softstem simulation */
			cf->m_softstemc_to_fire +  cf->m_softstemc_storage_to_fire + cf->m_softstemc_transfer_to_fire;
	/* NEE is positive if ecosystem is net source and negative if it is net sink */
	nee = -1* (nep - fire);
	

	summary->daily_nep = nep;
	summary->daily_npp = npp;
	summary->daily_nee = nee;	
	summary->daily_gpp = gpp;
	summary->daily_mr = mr;
	summary->daily_gr = gr;
	summary->daily_hr = hr;
	summary->daily_sr = sr;
	summary->daily_tr = tr;
	summary->daily_fire = fire;
	summary->cum_npp += npp;
	summary->cum_nep += nep;
	summary->cum_nee += nee;
	summary->cum_gpp += gpp;
	summary->cum_mr += mr;
	summary->cum_gr += gr;
	summary->cum_hr += hr;
	summary->cum_tr += (mr+gr+hr);
	summary->cum_sr += sr;

	summary->cum_n2o += nf->N2O_flux_NITRIF_total + nf->N2O_flux_DENITR_total + nf->N2O_flux_GRZ + nf->N2O_flux_FRZ;

	summary->cum_evap   += (wf->canopyw_evap + wf->soilw_evap);
	summary->cum_transp += wf->soilw_transp_SUM;
	summary->cum_ET += wf->evapotransp;
	
	/*******************************************************************************/
	/* 4. calculation litter fluxes and pools */

	summary->daily_litter = cs->litr1c_total + cs->litr2c_total + cs->litr3c_total + cs->litr4c_total;
	
	summary->daily_litdecomp = 0;
	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		summary->daily_litdecomp += cf->litr1c_to_soil1c[layer] + cf->litr2c_to_soil2c[layer]  + cf->litr4c_to_soil3c[layer];
	}

	summary->daily_litfire = cf->m_litr1c_to_fireTOTAL + cf->m_litr2c_to_fireTOTAL + cf->m_litr3c_to_fireTOTAL + cf->m_litr4c_to_fireTOTAL;
	
	/* aboveground litter */
	summary->daily_litfallc_above = 
		cf->m_leafc_to_litr1c + cf->m_leafc_to_litr2c + cf->m_leafc_to_litr3c + cf->m_leafc_to_litr4c + 
		cf->m_livestemc_to_cwdc + cf->m_deadstemc_to_cwdc + cf->m_livecrootc_to_cwdc + cf->m_deadcrootc_to_cwdc +
		cf->leafc_to_litr1c + cf->leafc_to_litr2c + cf->leafc_to_litr3c + cf->leafc_to_litr4c + 
		cf->softstemc_to_litr1c + cf->softstemc_to_litr2c + cf->softstemc_to_litr3c + cf->softstemc_to_litr4c + 
		cf->m_softstemc_to_litr1c + cf->m_softstemc_to_litr2c + cf->m_softstemc_to_litr3c + cf->m_softstemc_to_litr4c + 
		cf->fruitc_to_litr1c + cf->fruitc_to_litr2c + cf->fruitc_to_litr3c + cf->fruitc_to_litr4c + 
		cf->m_fruitc_to_litr1c + cf->m_fruitc_to_litr2c + cf->m_fruitc_to_litr3c + cf->m_fruitc_to_litr4c;
	
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
		cf->m_softstemc_storage_to_litr1c + cf->m_softstemc_transfer_to_litr1c + 
		cf->m_fruitc_storage_to_litr1c + cf->m_fruitc_transfer_to_litr1c;


	summary->daily_litfallc = summary->daily_litfallc_above + summary->daily_litfallc_below;

	/*******************************************************************************/
	/* 5. calculation of disturbance and senescence effect  */

	/* 5.1 cut-down biomass and standing dead biome */
	daily_STDB_to_litr = cf->STDBc_to_litr;
	daily_CTDB_to_litr = cf->CTDBc_to_litr;

	summary->cum_Cplus_STDB += daily_STDB_to_litr;
	summary->cum_Cplus_CTDB += daily_CTDB_to_litr;

	/* 5.2 management */
	Closs_THN_w = cf->livestemc_storage_to_THN + cf->livestemc_transfer_to_THN + cf->livestemc_to_THN +
				  cf->deadstemc_storage_to_THN + cf->deadstemc_transfer_to_THN + cf->deadstemc_to_THN +
				  cf->livecrootc_storage_to_THN + cf->livecrootc_transfer_to_THN + cf->livecrootc_to_THN +
				  cf->deadcrootc_storage_to_THN + cf->deadcrootc_transfer_to_THN + cf->deadcrootc_to_THN;

	Closs_THN_nw = cf->leafc_storage_to_THN + cf->leafc_transfer_to_THN + cf->leafc_to_THN +
		           cf->frootc_storage_to_THN + cf->frootc_transfer_to_THN + cf->frootc_to_THN +
				   cf->fruitc_storage_to_THN + cf->fruitc_transfer_to_THN + cf->fruitc_to_THN +
				   cf->gresp_transfer_to_THN + cf->gresp_storage_to_THN;


	Closs_MOW = cf->leafc_storage_to_MOW + cf->leafc_transfer_to_MOW + cf->leafc_to_MOW +
				cf->fruitc_storage_to_MOW + cf->fruitc_transfer_to_MOW + cf->fruitc_to_MOW +
				cf->softstemc_storage_to_MOW + cf->softstemc_transfer_to_MOW + cf->softstemc_to_MOW +
				cf->gresp_transfer_to_MOW + cf->gresp_storage_to_MOW;


	Closs_HRV = cf->leafc_storage_to_HRV + cf->leafc_transfer_to_HRV + cf->leafc_to_HRV +
				cf->fruitc_storage_to_HRV + cf->fruitc_transfer_to_HRV + cf->fruitc_to_HRV +
				cf->softstemc_storage_to_HRV + cf->softstemc_transfer_to_HRV + cf->softstemc_to_HRV +
		        cf->gresp_transfer_to_HRV + cf->gresp_storage_to_HRV;

	yieldC_HRV = cf->fruitc_to_HRV;


	Closs_PLG = cf->leafc_storage_to_PLG - cf->leafc_transfer_to_PLG +  cf->leafc_to_PLG +
				cf->fruitc_storage_to_PLG + cf->fruitc_transfer_to_PLG + cf->fruitc_to_PLG + 
				cf->softstemc_storage_to_PLG - cf->softstemc_transfer_to_PLG - cf->softstemc_to_PLG +
				cf->frootc_storage_to_PLG + cf->frootc_transfer_to_PLG + cf->frootc_to_PLG + 
				cf->gresp_transfer_to_PLG + cf->gresp_storage_to_PLG;


	Closs_GRZ = cf->leafc_storage_to_GRZ + cf->leafc_transfer_to_GRZ + cf->leafc_to_GRZ +
				cf->fruitc_storage_to_GRZ + cf->fruitc_transfer_to_GRZ + cf->fruitc_to_GRZ + 
				cf->softstemc_storage_to_GRZ + cf->softstemc_transfer_to_GRZ + cf->softstemc_to_GRZ + 
				cf->gresp_transfer_to_GRZ + cf->gresp_storage_to_GRZ;

	Cplus_GRZ = cf->GRZ_to_litr1c + cf->GRZ_to_litr2c + cf->GRZ_to_litr3c + cf->GRZ_to_litr4c;

	Cplus_FRZ = cf->FRZ_to_litr1c + cf->FRZ_to_litr2c + cf->FRZ_to_litr3c + cf->FRZ_to_litr4c;

	Cplus_PLT = cf->leafc_transfer_from_PLT + cf->frootc_transfer_from_PLT + 
							cf->fruitc_transfer_from_PLT +
							cf->softstemc_transfer_from_PLT;

	Closs_PLT = cf->STDBc_leaf_to_PLT + cf->STDBc_froot_to_PLT + cf->STDBc_fruit_to_PLT + cf->STDBc_softstem_to_PLT;	

	Nplus_GRZ = (nf->GRZ_to_litr1n  + nf->GRZ_to_litr2n  + nf->GRZ_to_litr3n  + nf->GRZ_to_litr4n);  
	Nplus_FRZ = (nf->FRZ_to_sminNH4 + nf->FRZ_to_sminNO3) +
		         nf->FRZ_to_litr1n + nf->FRZ_to_litr2n + nf->FRZ_to_litr3n + nf->FRZ_to_litr4n;



	summary->cum_Closs_THN_w  += Closs_THN_w;
	summary->cum_Closs_THN_nw += Closs_THN_nw;
	summary->cum_Closs_MOW    += Closs_MOW;
	summary->cum_Closs_HRV    += Closs_HRV;
	summary->cum_yieldC_HRV   += yieldC_HRV;
	summary->cum_Closs_PLG    += Closs_PLG;
	summary->cum_Closs_GRZ    += Closs_GRZ;
	summary->cum_Cplus_GRZ    += Cplus_GRZ;
	summary->cum_Cplus_PLT    += Cplus_PLT;
	summary->cum_Closs_PLT    += Closs_PLT;
	summary->cum_Cplus_FRZ    += Cplus_FRZ;	
	summary->cum_Nplus_GRZ    += Nplus_GRZ;  
	summary->cum_Nplus_FRZ    += Nplus_FRZ;  

	if (cs->vegC_HRV)
		summary->harvest_index = cs->fruitC_HRV/cs->vegC_HRV;
	else
		summary->harvest_index = 0;

	/* senescence effect  */
	Closs_SNSC = cf->m_leafc_storage_to_SNSC + cf->m_leafc_transfer_to_SNSC + cf->m_leafc_to_SNSC +
				 cf->m_fruitc_storage_to_SNSC + cf->m_fruitc_transfer_to_SNSC + cf->m_fruitc_to_SNSC +
				 cf->m_softstemc_storage_to_SNSC + cf->m_softstemc_transfer_to_SNSC + cf->m_softstemc_to_SNSC +
				 cf->m_frootc_storage_to_SNSC + cf->m_frootc_transfer_to_SNSC + cf->m_frootc_to_SNSC +
				 cf->m_gresp_transfer_to_SNSC + cf->m_gresp_storage_to_SNSC +
				 cf->HRV_frootc_to_SNSC + cf->HRV_softstemc_to_SNSC + cf->HRV_frootc_storage_to_SNSC +cf->HRV_frootc_transfer_to_SNSC +
				 cf->HRV_softstemc_storage_to_SNSC + cf->HRV_softstemc_transfer_to_SNSC + cf->HRV_gresp_storage_to_SNSC + cf->HRV_gresp_transfer_to_SNSC;

	summary->cum_Closs_SNSC += Closs_SNSC ;

	/* NBP calculation: positive - mean net carbon gain to the system and negative - mean net carbon loss */
	disturb_loss = Closs_THN_w + Closs_THN_nw + Closs_MOW + Closs_HRV + Closs_PLG  + Closs_GRZ;		
	disturb_gain = Cplus_FRZ + Cplus_GRZ + Cplus_PLT;

	nbp = nep + disturb_gain - disturb_loss;
	summary->daily_nbp = nbp;

	/* lateral flux calculation */
	summary->cum_Closs_MGM += disturb_loss;
	summary->cum_Cplus_MGM += disturb_gain;
	summary->lateral_Cflux = disturb_loss - disturb_gain;

	/* NGB calculation: net greenhouse gas balance - NBP - N2O(Ceq) -CH(Ceq) */
	N2O_Ceq= summary->daily_n2o * (44/28) * 298 * (12/44);
	CH4_Ceq= summary->CH4_flux_TOTAL * (18/14) * 34 * (12/44);
	summary->daily_ngb = summary->daily_nbp - N2O_Ceq - CH4_Ceq;
	summary->cum_ngb += summary->daily_ngb;
	

	return(errorCode);
}
