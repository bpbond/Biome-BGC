/* 
firstday.c
Initializes the state variables for the first day of a simulation that
is not using a restart file.

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2020, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "ini.h"
#include "bgc_struct.h"     /* structure definitions */
#include "bgc_func.h"       /* function prototypes */
#include "bgc_constants.h"


int firstday(const control_struct* ctrl, const soilprop_struct* sprop, const epconst_struct* epc, const planting_struct *PLT, 
	         siteconst_struct* sitec, cinit_struct* cinit, phenology_struct* phen, epvar_struct* epv, 
			 cstate_struct* cs, nstate_struct* ns, metvar_struct* metv, psn_struct* psn_sun, psn_struct* psn_shade)
{
	int errorCode=0;
	int layer, day, pp;
	double prop_transfer, transfer, prop_litfall;
	double max_deadstemc, max_deadcrootc;
	
	/* *****************************************************************************- */
	/* 1. Initialize ecophysiological variables */

	epv->DSR = 0.0;
	epv->cumSWCstress = 0.0;
	epv->cumNstress = 0.0;
	epv->SWCstressLENGTH = 0.0;
	epv->transfer_ratio = 0.0;
	epv->leafday = -1;
	epv->leafday_lastmort = -1;

	epv->n_rootlayers = 0;
	epv->germ_layer = 0;
	epv->germ_depth = 0.05;
    epv->proj_lai = 0;
    epv->all_lai = 0;
	epv->sla_avg = 0;
    epv->plaisun = 0;
    epv->plaishade = 0;
    epv->sun_proj_sla = 0;
    epv->shade_proj_sla = 0;
	epv->plant_height = 0;
	epv->n_actphen = 0;
	epv->flowHS_mort = 0;
	epv->assim_sun = 0;
	epv->assim_shade = 0;
	epv->dlmr_area_sun = 0;
	epv->dlmr_area_shade = 0;

	epv->plant_calloc = 0; 
	epv->plant_nalloc = 0;
	epv->excess_c = 0;
	epv->pnow = 0;
	epv->MRdeficit_nw = 0;
	epv->MRdeficit_w = 0;
	epv->NDVI = 0;

	epv->rootlength = 0;
	epv->rs_decomp_avg = 0;
	epv->m_tmin = 0;
	epv->m_co2 = 0;
	epv->stomaCONDUCT_max = 0;
	epv->albedo_LAI = 0;
	epv->assim_Tcoeff = 1;
	epv->assim_SScoeff = 1;
	epv->mulch_coverage = 0;
	epv->evapREDmulch = 1;

	psn_sun->A      = 0;
	psn_sun->Ci	    = 0;
	psn_sun->O2	    = 0;
	psn_sun->Ca	    = 0;
	psn_sun->gamma	= 0;
	psn_sun->Kc	    = 0;
	psn_sun->Ko	    = 0;
	psn_sun->Vmax	= 0;
	psn_sun->Jmax	= 0;
	psn_sun->J	    = 0;
	psn_sun->Av	    = 0;
	psn_sun->Aj	    = 0;
	
	psn_shade->A       = 0;
	psn_shade->Ci	    = 0;
	psn_shade->O2	    = 0;
	psn_shade->Ca	    = 0;
	psn_shade->gamma	= 0;
	psn_shade->Kc	    = 0;
	psn_shade->Ko	    = 0;
	psn_shade->Vmax	    = 0;
	psn_shade->Jmax  	= 0;
	psn_shade->J	    = 0;
	psn_shade->Av	    = 0;
	psn_shade->Aj	    = 0;



	/* initalize the number of the soil layers in which root can be found. It determines the rootzone depth (only on first day) */
	
	if (calc_nrootlayers(0, epc->max_rootzone_depth, cs->frootc, sitec, epv))
	{
		if (!errorCode) 
		{
			printf("\n");
			printf("ERROR: calc_nrootlayers() for multilayer_rootdepth.c\n");
		}
		errorCode=1;
	}

	/* initialize multilayer variables (first approximation: field cap.) and multipliers for stomatal limitation calculation */
	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		epv->VWC[layer]				    = sprop->VWCfc[layer];
		epv->WFPS[layer]	            = epv->VWC[layer] / sprop->VWCsat[layer];	
		epv->PSI[layer]				    = sprop->PSIfc[layer];
		epv->hydrCONDUCTact[layer]	    = sprop->hydrCONDUCTfc[layer];
		epv->hydrDIFFUSact[layer]	    = sprop->hydrDIFFUSfc[layer];
		epv->pF[layer]				    = log10(fabs(10000*sprop->PSIfc[layer]));	// dimension of PSI: MPa to cm (10000 MPa = 1 cm)
		epv->m_SWCstress_layer[layer]  = 1;
	    epv->rootlength_prop[layer]     = 0;
		epv->rootlengthLandD_prop[layer]= 0;
		ns->sminNH4avail[layer]         = ns->sminNH4[layer] * sprop->NH4_mobilen_prop;
		ns->sminNO3avail[layer]         = ns->sminNH4[layer] * NO3_mobilen_prop;
	}

    /* evergreen biome: root available also in the first day */
	epv->rootdepth = 0;
	if (epc->evergreen) 
	{
		epv->rootlength_prop[0] = 1;
		epv->n_rootlayers = 1;
		if (epc->woody) 
			epv->rootdepth = epc->max_rootzone_depth;
		else
			epv->rootdepth = CRIT_PREC;
	}
	/* initialize genetical senescence variables */

	epv->thermal_time = 0;

	epv->m_ppfd_sun = 1;
	epv->m_ppfd_shade = 1;
	epv->m_vpd = 1;
	epv->m_final_sun = 1;
	epv->m_final_shade = 1;
	epv->m_SWCstressLENGTH = 1;
	epv->m_extremT = 1;
	epv->gcorr = 0;
	epv->gl_bl = 0;
	epv->gl_c = 0;
	epv->gl_s_sun = 0;
	epv->gl_s_shade = 0;
	epv->gl_e_wv = 0;
	epv->gl_sh = 0;
	epv->gc_e_wv = 0;
	epv->gc_sh = 0;
	epv->gl_t_wv_sun = 0;
	epv->gl_t_wv_shade = 0;
	epv->gl_t_wv_sunPOT = 0;
	epv->gl_t_wv_shadePOT = 0;


	for (day = 0; day < 2*nDAYS_OF_YEAR; day++)
	{
		epv->thermal_timeARRAY[day]     = 0;
		epv->cpool_to_leafcARRAY[day] = 0;
		epv->npool_to_leafnARRAY[day] = 0;
		epv->gpSNSC_phenARRAY[day] = 0;
	}
	epv->VWC_avg		    = sprop->VWCfc[0];
	epv->VWC_RZ 		    = sprop->VWCfc[0];  
	epv->hydrCONDUCTsat_avg = sprop->hydrCONDUCTsat[0];
	epv->VWCsat_RZ 		    = sprop->VWCsat[0];  
	epv->VWCfc_RZ 		    = sprop->VWCfc[0];  
	epv->VWCwp_RZ 		    = sprop->VWCwp[0];  
	epv->VWChw_RZ 		    = sprop->VWChw[0];  
	epv->PSI_RZ		        = sprop->PSIfc[0];
	epv->m_SWCstress	    = 1;
	epv->SMSI               = 0;
	epv->flower_date        = 0;
	epv->winterEnd_date     = 0;
	metv->tsoil_avg	   	    = sitec->tair_annavg;
	metv->tsoil_surface_pre	= sitec->tair_annavg;	

	phen->GDD_emergSTART = 0;
	phen->GDD_emergEND   = 0;
	phen->GDD_limit      = 0;
	phen->yday_total     = 0;
	phen->onday          = -1;
	phen->offday         = -1;
	phen->planttype      = 0;



	/* initialize sum of leafC content and starting date of a given phenophase */
	for (pp = 0; pp < N_PHENPHASES; pp++) 
	{
		cs->leafcSUM_phenphase[pp] = 0;
		epv->phenphase_date[pp]    =-1;
		epv->rootdepth_phen[pp]    = 0;
	}


	/* *****************************************************************************- */
	/* 3. Initialize the C and N  variables, but NOT in transient phase and NOT in normal phase without reading restart file */


	if (ctrl->read_restart == 0 && ctrl->spinup != 2)
	{
		/* all types can use annmax leafc and frootc */
		epv->annmax_leafc = 0.0;
		epv->annmax_frootc = 0.0;
		epv->annmax_fruitc = 0.0;
		epv->annmax_softstemc = 0.0;
		epv->annmax_livestemc = 0.0;
		epv->annmax_livecrootc = 0.0;

		/* initialize the c and n storage state variables, but NOT in transient phase */
		cs->leafc_storage = 0.0;
		cs->frootc_storage = 0.0;
		cs->fruitc_storage = 0.0;
		cs->softstemc_storage = 0.0;
		cs->livestemc_storage = 0.0;
		cs->deadstemc_storage = 0.0;
		cs->livecrootc_storage = 0.0;
		cs->deadcrootc_storage = 0.0;
		cs->gresp_storage = 0.0;
		cs->cpool = 0.0;
		
		ns->leafn_storage = 0.0;
		ns->frootn_storage = 0.0;
		ns->fruitn_storage = 0.0;
		ns->softstemn_storage = 0.0;
		ns->livestemn_storage = 0.0;
		ns->deadstemn_storage = 0.0;
		ns->livecrootn_storage = 0.0;
		ns->deadcrootn_storage = 0.0;
		ns->npool = 0.0;
	
	
		/* establish the initial partitioning between displayed growth and growth ready for transfer */
		/* calculate initial leaf and froot nitrogen pools from carbon pools and user-specified initial C:N for each component */

		if (epc->leaf_cn)
		{
			cs->leafc_transfer       = cinit->max_leafc      * epc->nonwoody_turnover;
			cs->leafc                = cinit->max_leafc      - cs->leafc_transfer;
			ns->leafn_transfer = cs->leafc_transfer / epc->leaf_cn;
			ns->leafn = cs->leafc / epc->leaf_cn;
		}
 		
		if (epc->froot_cn)
		{
			cinit->max_frootc         = cinit->max_leafc    * (epc->alloc_frootc[0]/epc->alloc_leafc[0]);
			cs->frootc_transfer       = cinit->max_frootc   * epc->nonwoody_turnover;
			cs->frootc                = cinit->max_frootc   - cs->frootc_transfer;
			ns->frootn_transfer       = cs->frootc_transfer / epc->froot_cn;
			ns->frootn                 = cs->frootc         / epc->froot_cn;
		}

		if (epc->fruit_cn)
		{
			cinit->max_fruitc         = cinit->max_leafc    * (epc->alloc_fruitc[0]/epc->alloc_leafc[0]);
			cs->fruitc_transfer       = cinit->max_fruitc   * epc->nonwoody_turnover;
			cs->fruitc                = cinit->max_fruitc   - cs->fruitc_transfer;
			ns->fruitn_transfer       = cs->fruitc_transfer / epc->fruit_cn;
			ns->fruitn                = cs->fruitc          / epc->fruit_cn;
		}

		if (epc->softstem_cn)
		{
			cinit->max_softstemc         = cinit->max_leafc    * (epc->alloc_softstemc[0]/epc->alloc_leafc[0]);
			cs->softstemc_transfer       = cinit->max_softstemc   * epc->nonwoody_turnover;
			cs->softstemc                = cinit->max_softstemc   - cs->softstemc_transfer;
			ns->softstemn_transfer       = cs->softstemc_transfer / epc->softstem_cn;
			ns->softstemn                = cs->softstemc          / epc->softstem_cn;
		}

		
		if (epc->livewood_cn)
		{
			cinit->max_livestemc     = cinit->max_leafc        * (epc->alloc_livestemc[0]/epc->alloc_leafc[0]);
			cs->livestemc_transfer   = cinit->max_livestemc    * epc->woody_turnover;
			cs->livestemc            = cinit->max_livestemc    - cs->livestemc_transfer;
            ns->livestemn_transfer   = cs->livestemc_transfer  / epc->livewood_cn;
			ns->livestemn            = cs->livestemc           / epc->livewood_cn;

			cinit->max_livecrootc    = cinit->max_leafc        * (epc->alloc_livecrootc[0]/epc->alloc_leafc[0]);
			cs->livecrootc_transfer  = cinit->max_livecrootc   * epc->woody_turnover;
			cs->livecrootc           = cinit->max_livecrootc   - cs->livecrootc_transfer;
			ns->livecrootn_transfer  = cs->livecrootc_transfer / epc->livewood_cn;
			ns->livecrootn           = cs->livecrootc          / epc->livewood_cn;
		}
	
		if (epc->deadwood_cn)
		{
			max_deadstemc            = cinit->max_leafc        * (epc->alloc_deadstemc[0]/epc->alloc_leafc[0]);
			cs->deadstemc_transfer   = max_deadstemc    * epc->woody_turnover;
			cs->deadstemc            = max_deadstemc    - cs->deadstemc_transfer;
            ns->deadstemn_transfer   = cs->deadstemc_transfer  / epc->deadwood_cn;
			ns->deadstemn            = cs->deadstemc           / epc->deadwood_cn;

			max_deadcrootc           = cinit->max_leafc        * (epc->alloc_deadcrootc[0]/epc->alloc_leafc[0]);
			cs->deadcrootc_transfer  = max_deadcrootc   * epc->woody_turnover;
			cs->deadcrootc           = max_deadcrootc   - cs->deadcrootc_transfer;
			ns->deadcrootn_transfer  = cs->deadcrootc_transfer / epc->deadwood_cn;
			ns->deadcrootn           = cs->deadcrootc          / epc->deadwood_cn;

		}

		/* *****************************************************************************- */
		/* 4. use then penology array information to determine, for the first day of simulation, how many days of transfer and litterfall have already occurred for this year */
		
		if (phen->predays_transfer > 0)
		{
			prop_transfer = (double)phen->predays_transfer/(double)(phen->predays_transfer+phen->remdays_transfer);
			/* perform these transfers */
			transfer = prop_transfer * cs->leafc_transfer;
			cs->leafc          += transfer;
			cs->leafc_transfer -= transfer;
			transfer = prop_transfer * ns->leafn_transfer;
			ns->leafn          += transfer;
			ns->leafn_transfer -= transfer;
			transfer = prop_transfer * cs->frootc_transfer;
			cs->frootc          += transfer;
			cs->frootc_transfer -= transfer;
			transfer = prop_transfer * ns->frootn_transfer;
			ns->frootn          += transfer;
			ns->frootn_transfer -= transfer;
			/* fruit simulation  */
			transfer = prop_transfer * cs->fruitc_transfer;
			cs->fruitc          += transfer;
			cs->fruitc_transfer -= transfer;
			transfer = prop_transfer * ns->fruitn_transfer;
			ns->fruitn          += transfer;
			ns->fruitn_transfer -= transfer;

			/* TREE-specific and NON-WOODY SPECIFIC fluxes */
			if (epc->woody)
			{
				transfer = prop_transfer * cs->livestemc_transfer;
				cs->livestemc          += transfer;
				cs->livestemc_transfer -= transfer;
				transfer = prop_transfer * ns->livestemn_transfer;
				ns->livestemn          += transfer;
				ns->livestemn_transfer -= transfer;
				transfer = prop_transfer * cs->deadstemc_transfer;
				cs->deadstemc          += transfer;
				cs->deadstemc_transfer -= transfer;
				transfer = prop_transfer * ns->deadstemn_transfer;
				ns->deadstemn          += transfer;
				ns->deadstemn_transfer -= transfer;
				transfer = prop_transfer * cs->livecrootc_transfer;
				cs->livecrootc          += transfer;
				cs->livecrootc_transfer -= transfer;
				transfer = prop_transfer * ns->livecrootn_transfer;
				ns->livecrootn          += transfer;
				ns->livecrootn_transfer -= transfer;
				transfer = prop_transfer * cs->deadcrootc_transfer;
				cs->deadcrootc          += transfer;
				cs->deadcrootc_transfer -= transfer;
				transfer = prop_transfer * ns->deadcrootn_transfer;
				ns->deadcrootn          += transfer;
				ns->deadcrootn_transfer -= transfer;
			}
			else
			{
				/* SOFT STEM SIMULATION of non-woody biomes */
				transfer = prop_transfer * cs->softstemc_transfer;
				cs->softstemc          += transfer;
				cs->softstemc_transfer -= transfer;
				transfer = prop_transfer * ns->softstemn_transfer;
				ns->softstemn          += transfer;
				ns->softstemn_transfer -= transfer;

			}
		
			/* only test for litterfall if there has already been some transfer growth this year */
			if (phen->predays_litfall > 0)
			{
				/* some litterfall has already occurred. in this case, just
				remove material from the displayed compartments, and don't
				bother with the transfer to litter compartments */
				prop_litfall = (double)phen->predays_litfall/(double)(phen->predays_litfall+phen->remdays_litfall);
				cs->leafc  -= prop_litfall * cs->leafc * epc->nonwoody_turnover;
				cs->frootc -= prop_litfall * cs->frootc * epc->nonwoody_turnover;
				cs->fruitc -= prop_litfall * cs->fruitc * epc->nonwoody_turnover;
				cs->softstemc -= prop_litfall * cs->softstemc * epc->nonwoody_turnover;
			}
		} /* end if transfer */
		/* add the growth respiration requirement for the first year's leaf and fine root growth from transfer pools to the gresp_transfer pool */
		cs->gresp_transfer = 0.0;
		cs->gresp_transfer += (cs->leafc_transfer + cs->frootc_transfer + cs->fruitc_transfer + cs->softstemc_transfer +
			                   cs->livestemc_transfer + cs->deadstemc_transfer + cs->livecrootc_transfer + cs->deadcrootc_transfer) * epc->GR_ratio; 
	

	}

	/*  if planting: transfer pools are deplenished (set to zero) on the first simulation day */ 
	if (PLT->PLT_num > 0)
	{
		cs->HRV_transportC += cs->leafc + cs->leafc_transfer;
		cs->HRV_transportC += cs->frootc + cs->frootc_transfer;
		cs->HRV_transportC += cs->fruitc + cs->fruitc_transfer;
		cs->HRV_transportC += cs->softstemc + cs->softstemc_transfer;
		cs->HRV_transportC += cs->gresp_transfer;
		cs->HRV_transportC += cs->STDBc_leaf;
		cs->HRV_transportC += cs->STDBc_froot;
		cs->HRV_transportC += cs->STDBc_fruit;
		cs->HRV_transportC += cs->STDBc_softstem;
		cs->HRV_transportC += cs->STDBc_nsc;

		cs->leafc = 0;
		cs->frootc = 0;
		cs->fruitc = 0;
		cs->softstemc = 0;
		cs->leafc_transfer = 0;
		cs->frootc_transfer = 0;
		cs->fruitc_transfer = 0;
		cs->softstemc_transfer = 0;
		cs->gresp_transfer = 0;
		cs->STDBc_leaf = 0;
		cs->STDBc_froot = 0;
		cs->STDBc_fruit = 0;
		cs->STDBc_softstem = 0;
		cs->STDBc_nsc = 0;

		ns->HRV_transportN += ns->leafn + ns->leafn_transfer;
		ns->HRV_transportN += ns->frootn + ns->frootn_transfer;
		ns->HRV_transportN += ns->fruitn + ns->fruitn_transfer;
		ns->HRV_transportN += ns->softstemn + ns->softstemn_transfer;
		ns->HRV_transportN += ns->retransn;
		ns->HRV_transportN += ns->STDBn_leaf;
		ns->HRV_transportN += ns->STDBn_froot;
		ns->HRV_transportN += ns->STDBn_fruit;
		ns->HRV_transportN += ns->STDBn_softstem;
		ns->HRV_transportN += ns->STDBn_nsc;

		ns->leafn = 0;
		ns->frootn = 0;
		ns->fruitn = 0;
		ns->softstemn = 0;
		ns->leafn_transfer = 0;
		ns->frootn_transfer = 0;
		ns->fruitn_transfer = 0;
		ns->softstemn_transfer = 0;
		ns->retransn += 0;
		ns->STDBn_leaf = 0;
		ns->STDBn_froot = 0;
		ns->STDBn_fruit = 0;
		ns->STDBn_softstem = 0;
		ns->STDBn_nsc = 0;
	}
	
	/* set the initial rates of litterfall and live wood turnover */
	if (epc->evergreen)
	{
		/* leaf and fineroot litterfall rates */
		if (ctrl->spinup != 0)
		{
			epv->day_leafc_litfall_increment = cinit->max_leafc * epc->nonwoody_turnover / nDAYS_OF_YEAR;
			epv->day_frootc_litfall_increment = cinit->max_frootc * epc->nonwoody_turnover / nDAYS_OF_YEAR;
			epv->day_fruitc_litfall_increment = cinit->max_fruitc * epc->nonwoody_turnover / nDAYS_OF_YEAR;
			epv->day_softstemc_litfall_increment = cinit->max_softstemc * epc->nonwoody_turnover / nDAYS_OF_YEAR;
		}
		else
		{
			epv->day_leafc_litfall_increment = epv->annmax_leafc * epc->nonwoody_turnover / nDAYS_OF_YEAR;
			epv->day_frootc_litfall_increment = epv->annmax_frootc * epc->nonwoody_turnover / nDAYS_OF_YEAR;
			epv->day_fruitc_litfall_increment = epv->annmax_fruitc * epc->nonwoody_turnover / nDAYS_OF_YEAR;
			epv->day_softstemc_litfall_increment = epv->annmax_softstemc * epc->nonwoody_turnover / nDAYS_OF_YEAR;
		}
	}
	else
	{
		/* deciduous: reset the litterfall rates to 0.0 for the start of the
		next litterfall season */
		epv->day_leafc_litfall_increment = 0.0;
		epv->day_frootc_litfall_increment = 0.0;
		epv->day_fruitc_litfall_increment = 0.0;
		epv->day_softstemc_litfall_increment = 0.0;
	}

	epv->day_livestemc_turnover_increment = cs->livestemc * epc->woody_turnover / nDAYS_OF_YEAR;
	epv->day_livecrootc_turnover_increment = cs->livecrootc * epc->woody_turnover / nDAYS_OF_YEAR;


	/* in case of land-use change: agroecosystem to natural vegetation */
	if (!ctrl->spinup && !PLT->PLT_num)
	{
		if (!cs->leafc_transfer)
		{
			cs->leafc_transfer = 0.001;
			ns->leafn_transfer = cs->leafc_transfer/ epc->leaf_cn;
		}

		if (!cs->frootc_transfer)
		{
			cs->frootc_transfer = 0.001;
			ns->frootn_transfer = ns->frootn_transfer/ epc->leaf_cn;
		}
		
	}

	/* in case of land-use change: non-woody to woody and woody to non-woody */
	if (ctrl->spinup == 0)
	{
		if (epc->woody)
		{
			cs->softstemc = 0;
			cs->softstemc_storage = 0;
			cs->softstemc_transfer = 0;
			cs->HRV_transportC += cs->softstemc + cs->softstemc_storage + cs->softstemc_transfer;
			ns->softstemn = 0;
			ns->softstemn_storage = 0;
			ns->softstemn_transfer = 0;
			ns->HRV_transportN += ns->softstemn + ns->softstemn_storage + ns->softstemn_transfer;
		}
		else
		{
			cs->livestemc = 0;
			cs->livestemc_storage = 0;
			cs->livestemc_transfer = 0;
			cs->livecrootc = 0;
			cs->livecrootc_storage = 0;
			cs->livecrootc_transfer = 0;
			cs->deadstemc = 0;
			cs->deadstemc_storage = 0;
			cs->deadstemc_transfer = 0;
			cs->deadcrootc = 0;
			cs->deadcrootc_storage = 0;
			cs->deadcrootc_transfer = 0;
			cs->HRV_transportC += cs->livestemc + cs->livestemc_storage + cs->livestemc_transfer +
				                  cs->livecrootc + cs->livecrootc_storage + cs->livecrootc_transfer +
								  cs->deadstemc + cs->deadstemc_storage + cs->deadstemc_transfer +
				                  cs->deadcrootc + cs->deadcrootc_storage + cs->deadcrootc_transfer;
			ns->livestemn = 0;
			ns->livestemn_storage = 0;
			ns->livestemn_transfer = 0;
			ns->livecrootn = 0;
			ns->livecrootn_storage = 0;
			ns->livecrootn_transfer = 0;
			ns->deadstemn = 0;
			ns->deadstemn_storage = 0;
			ns->deadstemn_transfer = 0;
			ns->deadcrootn = 0;
			ns->deadcrootn_storage = 0;
			ns->deadcrootn_transfer = 0;
			ns->HRV_transportN += ns->livestemn + ns->livestemn_storage + ns->livestemn_transfer +
				                  ns->livecrootn + ns->livecrootn_storage + ns->livecrootn_transfer +
								  ns->deadstemn + ns->deadstemn_storage + ns->deadstemn_transfer +
				                  ns->deadcrootn + ns->deadcrootn_storage + ns->deadcrootn_transfer;
		}

	}
	return (errorCode);
}
