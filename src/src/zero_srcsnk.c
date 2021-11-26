/* 
zero_srcsnk.c
fill the source and sink variables with 0.0 at the start of the simulation

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

/* zero the source and sink state variables */
int zero_srcsnk(cstate_struct* cs, nstate_struct* ns, wstate_struct* ws, summary_struct* summary)
{
	int errorCode=0;
	
	/* zero the water sources and sinks  */
	ws->prcp_src = 0.0;
	ws->soilEvap_snk = 0.0;
	ws->snowsubl_snk = 0.0;
	ws->canopyevap_snk = 0.0;
	ws->trans_snk = 0.0;
	ws->prcp_src = 0.0;
	ws->soilEvap_snk = 0.0;
	ws->canopyw_THNsnk = 0.0;
	ws->canopyw_MOWsnk = 0.0;
	ws->canopyw_HRVsnk = 0.0;	
	ws->canopyw_PLGsnk = 0.0;
	ws->canopyw_GRZsnk = 0.0;	
	ws->IRGsrc_W = 0.0;
	ws->condIRGsrc = 0;
	ws->FRZsrc_W = 0.0;
	ws->runoff_snk = 0.0;
	ws->pondEvap_snk = 0;
	ws->deeppercolation_snk = 0.0;  
	ws->groundwater_src = 0.0;
	ws->groundwater_snk = 0.0;
	ws->WbalanceERR = 0.0;	
	ws->inW = 0.0;
	ws->outW = 0.0;
	ws->storeW = 0.0;
	ws->soilEvapCUM1 = 0.0;
	ws->soilEvapCUM2 = 0.0;

	/* zero the carbon sources and sinks */
	cs->psnsun_src = 0.0;
	cs->psnshade_src = 0.0;
	cs->NSC_mr_snk = 0;
	cs->actC_mr_snk = 0;
	cs->leaf_mr_snk = 0.0;
	cs->leaf_gr_snk = 0.0;
	cs->froot_mr_snk = 0.0;
	cs->froot_gr_snk = 0.0;
	cs->livestem_mr_snk = 0.0;
	cs->livestem_gr_snk = 0.0;
	cs->deadstem_gr_snk = 0.0;
	cs->livecroot_mr_snk = 0.0;
	cs->livecroot_gr_snk = 0.0;
	cs->deadcroot_gr_snk = 0.0;
	cs->litr1_hr_snk = 0.0;
	cs->litr2_hr_snk = 0.0;
	cs->litr4_hr_snk = 0.0;
	cs->soil1_hr_snk = 0.0;
	cs->soil2_hr_snk = 0.0;
	cs->soil3_hr_snk = 0.0;
	cs->soil4_hr_snk = 0.0;
	cs->Cdeepleach_snk = 0;
	cs->FIREsnk_C = 0.0;
	cs->SNSCsnk_C = 0.0;
	cs->PLTsrc_C = 0.0; 
	cs->THN_transportC = 0.0; 

	cs->MOW_transportC = 0;
	cs->GRZsnk_C = 0.0;  
	cs->GRZsrc_C = 0.0;
	cs->HRV_transportC = 0.0;
	cs->FRZsrc_C = 0.0;
	cs->fruitC_HRV = 0.0;
	cs->vegC_HRV = 0.0;
	cs->fruit_mr_snk = 0.0;
	cs->fruit_gr_snk = 0.0;
	cs->softstem_mr_snk = 0.0;
	cs->softstem_gr_snk = 0.0;
	cs->CbalanceERR = 0.0;
	cs->CNratioERR = 0.0;
	cs->inC = 0.0;
	cs->outC = 0.0;
	cs->storeC = 0.0;
	cs->nsc_nw = 0.0;
	cs->nsc_w = 0.0;
	cs->sc_nw = 0.0;
	cs->sc_w = 0.0;

	ns->Nfix_src = 0.0;
	ns->Ndep_src = 0.0;
	ns->Ndeepleach_snk = 0.0;
	ns->Nvol_snk = 0.0;
	ns->Nprec_snk = 0;
	ns->FIREsnk_N = 0.0;
	ns->SPINUPsrc = 0.0;
	ns->SNSCsnk_N = 0.0;
	ns->PLTsrc_N = 0.0; 
	ns->THN_transportN = 0.0; 
	ns->MOW_transportN = 0;
	ns->GRZsnk_N = 0.0;  
	ns->GRZsrc_N = 0.0;
	ns->HRV_transportN = 0.0; 
	ns->FRZsrc_N = 0.0; 
	ns->NbalanceERR = 0.0;
	ns->inN = 0.0;
	ns->outN = 0.0;
	ns->storeN = 0.0;
	
	/* zero the summary variables */
	summary->annprcp = 0.0;
	summary->anntavg = 0.0;
	summary->cum_runoff = 0.0;
	summary->cum_WleachRZ = 0.0;
	summary->cum_NleachRZ = 0.0;
	summary->cum_npp = 0.0;
	summary->cum_nep = 0.0;
	summary->cum_nee = 0.0;
	summary->cum_gpp = 0.0;
	summary->cum_ngb = 0.0;
	summary->cum_mr = 0.0;
	summary->cum_gr = 0.0;
	summary->cum_hr = 0.0;
	summary->cum_transp  = 0;
	summary->cum_ET  = 0;	
	summary->cum_ET	= 0.0;
	summary->cum_tr = 0.0;
	summary->cum_sr = 0.0;
	summary->cum_n2o = 0.0;
	summary->cum_Closs_MGM  = 0;
	summary->cum_Cplus_MGM  = 0;
	summary->cum_Closs_MOW = 0.0;
	summary->cum_Closs_THN_w = 0.0;
	summary->cum_Closs_THN_nw = 0.0;
	summary->cum_Closs_PLG = 0.0;
	summary->cum_Closs_HRV = 0.0;
	summary->cum_yieldC_HRV = 0.0;
	summary->cum_Closs_GRZ = 0.0;
	summary->cum_Cplus_GRZ = 0.0;
	summary->cum_Cplus_FRZ = 0.0;
	summary->cum_Cplus_PLT = 0.0;
	summary->cum_Closs_PLT = 0.0;
	summary->cum_Nplus_FRZ = 0.0;
	summary->cum_Nplus_GRZ = 0.0;
	summary->cum_Closs_SNSC = 0.0;
	summary->cum_Cplus_CTDB = 0.0;
	summary->cum_Cplus_STDB = 0.0;
	summary->daily_litdecomp = 0.0;
	summary->daily_litfallc = 0.0;
	summary->daily_litfallc_above = 0.0;
	summary->daily_litfallc_below = 0.0;
	summary->daily_litfire = 0.0;
	summary->daily_nbp = 0.0;
	summary->soilC_total = 0.0;
	summary->litrC_total = 0.0;
	summary->litrCwdC_total = 0.0;
	summary->soilN_total = 0.0;
	summary->litrN_total = 0.0;
	summary->litrCwdN_total = 0.0;
	summary->sminN_total = 0.0;
	summary->sminNavail_total = 0.0;
	summary->sminNavail_maxRZ = 0.0;
	summary->sminN_maxRZ = 0.0;
	summary->soilC_maxRZ = 0.0;
	summary->soilN_maxRZ = 0.0;
	summary->litrC_maxRZ = 0.0;
	summary->litrN_maxRZ = 0.0;
	summary->leafDM = 0.0;
	summary->leaflitrDM = 0.0;
    summary->frootDM = 0.0;
	summary->fruitDM = 0.0;
	summary->yieldDM_HRV = 0.0;
    summary->softstemDM = 0.0;
    summary->livewoodDM = 0.0;
	summary->deadwoodDM = 0.0;
	summary->vegC = 0.0;
	summary->LDaboveC_nw = 0.0;
	summary->LDaboveC_w = 0.0;
	summary->LDaboveCnsc_nw = 0.0;
	summary->LDaboveCnsc_w = 0.0;
	summary->LaboveC_nw = 0.0;
	summary->LaboveC_w = 0.0;
	summary->LaboveCnsc_nw = 0.0;
	summary->LaboveCnsc_w = 0.0;
	summary->DaboveC_nw = 0.0;
	summary->DaboveC_w = 0.0;
	summary->DaboveCnsc_nw = 0.0;
	summary->DaboveCnsc_w = 0.0;
	summary->totalC = 0.0;
	summary->CNlitr_total = 0.0;
	summary->CNsoil_total = 0.0;
	summary->leafCN = 0.0;
	summary->frootCN = 0.0;
	summary->softstemCN = 0.0;
	summary->fruitCN = 0.0;

	summary->NH4_top30avail = 0.0; 
	summary->NO3_top30avail = 0.0;
	summary->sminN_top30avail = 0.0;
	summary->SOM_C_top30 = 0.0;
	summary->SOM_N_top30 = 0.0;
	summary->stableSOC_top30 = 0.0;
	summary->leafc_LandD = 0.0;
	summary->frootc_LandD = 0.0;
	summary->fruitc_LandD = 0.0;
	summary->softstemc_LandD = 0.0;
	summary->lateral_Cflux = 0.0;
	summary->harvest_index = 0;

	
	return (errorCode);
}
