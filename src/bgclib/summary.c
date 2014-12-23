/* 
summary.c
summary variables for potential output

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "bgc.h"

int csummary(cflux_struct* cf, cstate_struct* cs, summary_struct* summary)
{
	int ok=1;
	double gpp,mr,gr,hr,fire;
	double npp,nep,nee;
	
	/* calculate daily NPP, positive for net growth */
	/* NPP = Gross PSN - Maintenance Resp - Growth Resp */
	gpp = cf->psnsun_to_cpool + cf->psnshade_to_cpool;
	mr = cf->leaf_day_mr + cf->leaf_night_mr + cf->froot_mr + 
		cf->livestem_mr + cf->livecroot_mr;
	gr = cf->cpool_leaf_gr + cf->cpool_leaf_storage_gr + cf->transfer_leaf_gr +
		cf->cpool_froot_gr + cf->cpool_froot_storage_gr + cf->transfer_froot_gr + 
		cf->cpool_livestem_gr + cf->cpool_livestem_storage_gr + cf->transfer_livestem_gr +
		cf->cpool_deadstem_gr + cf->cpool_deadstem_storage_gr + cf->transfer_deadstem_gr + 
		cf->cpool_livecroot_gr + cf->cpool_livecroot_storage_gr + cf->transfer_livecroot_gr + 
		cf->cpool_deadcroot_gr + cf->cpool_deadcroot_storage_gr + cf->transfer_deadcroot_gr;
	npp = gpp - mr - gr;
	
	/* calculate daily NEP, positive for net sink */
	/* NEP = NPP - Autotrophic Resp */
	hr = cf->litr1_hr + cf->litr2_hr + cf->litr4_hr + cf->soil1_hr +
		cf->soil2_hr + cf->soil3_hr + cf->soil4_hr;
	nep = npp - hr;
	
	/* calculate daily NEE, positive for net sink */
	/* NEE = NEP - fire losses */
	fire = cf->m_leafc_to_fire + cf->m_frootc_to_fire + cf->m_leafc_storage_to_fire +
		cf->m_frootc_storage_to_fire + cf->m_livestemc_storage_to_fire + 
		cf->m_deadstemc_storage_to_fire + cf->m_livecrootc_storage_to_fire +
		cf->m_deadcrootc_storage_to_fire + cf->m_leafc_transfer_to_fire +
		cf->m_frootc_transfer_to_fire +  cf->m_livestemc_transfer_to_fire + 
		cf->m_deadstemc_transfer_to_fire + cf->m_livecrootc_transfer_to_fire +
		cf->m_deadcrootc_transfer_to_fire + cf->m_livestemc_to_fire +
		cf->m_deadstemc_to_fire + cf->m_livecrootc_to_fire + cf->m_deadcrootc_to_fire +
		cf->m_gresp_storage_to_fire + cf->m_gresp_transfer_to_fire + cf->m_litr1c_to_fire +
		cf->m_litr2c_to_fire + cf->m_litr3c_to_fire + cf->m_litr4c_to_fire +
		cf->m_cwdc_to_fire;
	nee = nep - fire;
	
	summary->daily_npp = npp;
	summary->daily_nep = nep;
	summary->daily_nee = nee;
	summary->daily_gpp = gpp;
	summary->daily_mr = mr;
	summary->daily_gr = gr;
	summary->daily_hr = hr;
	summary->daily_fire = fire;
	summary->cum_npp += npp;
	summary->cum_nep += nep;
	summary->cum_nee += nee;
	summary->cum_gpp += gpp;
	summary->cum_mr += mr;
	summary->cum_gr += gr;
	summary->cum_hr += hr;
	summary->cum_fire += fire;
	
	/* other flux summary variables */
	summary->daily_litfallc = cf->m_leafc_to_litr1c + cf->m_leafc_to_litr2c + 
		cf->m_leafc_to_litr3c + cf->m_leafc_to_litr4c + cf->m_frootc_to_litr1c +
		cf->m_frootc_to_litr2c + cf->m_frootc_to_litr3c + cf->m_frootc_to_litr4c +
		cf->m_leafc_storage_to_litr1c + cf->m_frootc_storage_to_litr1c +
		cf->m_livestemc_storage_to_litr1c + cf->m_deadstemc_storage_to_litr1c +
		cf->m_livecrootc_storage_to_litr1c + cf->m_deadcrootc_storage_to_litr1c +
		cf->m_leafc_transfer_to_litr1c + cf->m_frootc_transfer_to_litr1c +
		cf->m_livestemc_transfer_to_litr1c + cf->m_deadstemc_transfer_to_litr1c +
		cf->m_livecrootc_transfer_to_litr1c + cf->m_deadcrootc_transfer_to_litr1c +
		cf->m_livestemc_to_cwdc + cf->m_deadstemc_to_cwdc + 
		cf->m_livecrootc_to_cwdc + cf->m_deadcrootc_to_cwdc +
		cf->m_gresp_storage_to_litr1c + cf->m_gresp_transfer_to_litr1c +
		cf->leafc_to_litr1c + cf->leafc_to_litr2c + cf->leafc_to_litr3c + 
		cf->leafc_to_litr4c + cf->frootc_to_litr1c + cf->frootc_to_litr2c +
		cf->frootc_to_litr3c + cf->frootc_to_litr4c;
		
	/* summarize carbon stocks */
	summary->vegc = cs->leafc + cs->leafc_storage + cs->leafc_transfer + 
		cs->frootc + cs->frootc_storage + cs->frootc_transfer +
		cs->livestemc + cs->livestemc_storage + cs->livestemc_transfer +
		cs->deadstemc + cs->deadstemc_storage + cs->deadstemc_transfer +
		cs->livecrootc + cs->livecrootc_storage + cs->livecrootc_transfer +
		cs->deadcrootc + cs->deadcrootc_storage + cs->deadcrootc_transfer +
		cs->gresp_storage + cs->gresp_transfer + cs->cpool;
	summary->litrc = cs->cwdc + cs->litr1c + cs->litr2c + cs->litr3c + 
		cs->litr4c;
	summary->soilc = cs->soil1c + cs->soil2c + cs->soil3c + cs->soil4c;
	summary->totalc = summary->vegc + summary->litrc + summary->soilc;
	
	
	return(!ok);
}

int wsummary(wstate_struct *ws,wflux_struct* wf, summary_struct* summary)
{
	/* Water state summary variables
		Purpose: calculates ET from individual flux variables.  Assigns daily outflow to summary struct.
		Added: WMJ, 26 Oct 04         */
		double et = 0.0,evap = 0.0,trans = 0.0;

		int ok=1;

		et = wf->canopyw_evap + wf->soilw_evap + wf->soilw_trans + wf->snoww_subl;

		evap = wf->canopyw_evap + wf->soilw_evap + wf->snoww_subl;
		trans = wf->soilw_trans;

		summary->daily_et = et;
		summary->daily_evap = evap;
		summary->daily_trans = trans;
		summary->daily_soilw = ws->soilw;
		summary->daily_snoww = ws->snoww;
		summary->daily_outflow = wf->soilw_outflow;

		return(!ok);
}
