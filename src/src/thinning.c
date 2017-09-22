/* 
thinning.c
do thinning  - decrease the plant material (leafc, leafn, canopy water, frootc, frootn, stemc, stemn, crootc, crootn)

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.1
Copyright 2017, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "ini.h"     
#include "pointbgc_struct.h"
#include "bgc_struct.h"
#include "pointbgc_func.h"
#include "bgc_constants.h"

int thinning(const control_struct* ctrl, const epconst_struct* epc, thinning_struct* THN, cflux_struct* cf,nflux_struct* nf,wflux_struct* wf, 
				 cstate_struct* cs, nstate_struct* ns, wstate_struct* ws)
{
	/* thinning parameters */
	double remained_prop_nwoody, remained_prop_woody;	
	
	double THNcoeff;					/* coefficient determining decrease of plant material caused by thinning  */		
	double THN_to_CTDB_litr1c, THN_to_CTDB_litr2c, THN_to_CTDB_litr3c, THN_to_CTDB_litr4c, THN_to_CTDB_cwdc;
	double THN_to_CTDB_litr1n, THN_to_CTDB_litr2n, THN_to_CTDB_litr3n, THN_to_CTDB_litr4n, THN_to_CTDB_cwdn;
	double STDB_litr1c_to_THN, STDB_litr2c_to_THN, STDB_litr3c_to_THN, STDB_litr4c_to_THN;
	double STDB_litr1n_to_THN, STDB_litr2n_to_THN, STDB_litr3n_to_THN, STDB_litr4n_to_THN;
	double THN_to_transpC, THN_to_transpN;
	double diffC = 0;
	double diffN = 0;
	int ok=1;
	int ny;
	int mgmd = THN->mgmd;
	double THN_to_litter_coeff_nw = epc->mort_CnW_to_litter;
	double THN_to_litter_coeff_w  = epc->mort_CnW_to_litter/100;

	double storage_MGMmort=epc->storage_MGMmort;

	/* yearly varied or constant management parameters */
	if(THN->THN_flag == 2)
	{
		ny = ctrl->simyr;
	}
	else ny=0;

	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                                    CALCULATING FLUXES 
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

	/* thinning if flag=1 */
	if (mgmd >= 0)
	{	

		THNcoeff    = THN->thinning_rate_array[mgmd][ny];
		remained_prop_nwoody = (100 - THN->transpcoeff_nwoody_array[mgmd][ny])/100.;
		remained_prop_woody = (100 - THN->transpcoeff_woody_array[mgmd][ny])/100.;

	
	}
	else
	{
		THNcoeff    = 0;
		THNcoeff   = 0;
		THNcoeff   = 0;
		remained_prop_nwoody = 0;
		remained_prop_woody = 0;
	}

	/**********************************************************************************************/
	/* 1. as results of the thinning the carbon, nitrogen and water content of the leaf/root/stem decreases*/
	/* fruit simulation - Hidy 2013.: harvested fruit is transported from the site  */

	/* leaf */
	cf->leafc_to_THN          = cs->leafc * THNcoeff;
	cf->leafc_transfer_to_THN = cs->leafc_transfer * THNcoeff * storage_MGMmort; 
	cf->leafc_storage_to_THN  = cs->leafc_storage * THNcoeff * storage_MGMmort; 

	nf->leafn_to_THN          = ns->leafn * THNcoeff;
	nf->leafn_transfer_to_THN = ns->leafn_transfer * THNcoeff * storage_MGMmort; 
	nf->leafn_storage_to_THN  = ns->leafn_storage * THNcoeff * storage_MGMmort;  

	/* fine root */
	cf->frootc_to_THN          = cs->frootc * THNcoeff;
	cf->frootc_transfer_to_THN = cs->frootc_transfer * THNcoeff * storage_MGMmort; 
	cf->frootc_storage_to_THN  = cs->frootc_storage * THNcoeff * storage_MGMmort; 

	nf->frootn_to_THN          = ns->frootn * THNcoeff;
	nf->frootn_transfer_to_THN = ns->frootn_transfer * THNcoeff * storage_MGMmort; 
	nf->frootn_storage_to_THN  = ns->frootn_storage * THNcoeff * storage_MGMmort; 

	/* fruit */
	cf->fruitc_to_THN          = cs->fruitc * THNcoeff;
	cf->fruitc_transfer_to_THN = cs->fruitc_transfer * THNcoeff * storage_MGMmort; 
	cf->fruitc_storage_to_THN  = cs->fruitc_storage * THNcoeff * storage_MGMmort; 

	nf->fruitn_to_THN          = ns->fruitn * THNcoeff;
	nf->fruitn_transfer_to_THN = ns->fruitn_transfer * THNcoeff * storage_MGMmort; 
	nf->fruitn_storage_to_THN  = ns->fruitn_storage * THNcoeff * storage_MGMmort; 

	/* coarse root */
	cf->livecrootc_to_THN          = cs->livecrootc * THNcoeff;
	cf->livecrootc_transfer_to_THN = cs->livecrootc_transfer * THNcoeff * storage_MGMmort; 
	cf->livecrootc_storage_to_THN  = cs->livecrootc_storage * THNcoeff * storage_MGMmort;  


	nf->livecrootn_to_THN          = ns->livecrootn * THNcoeff;
	nf->livecrootn_transfer_to_THN = ns->livecrootn_transfer * THNcoeff * storage_MGMmort; 
	nf->livecrootn_storage_to_THN  = ns->livecrootn_storage * THNcoeff * storage_MGMmort;  

	cf->deadcrootc_to_THN          = cs->deadcrootc * THNcoeff;
	cf->deadcrootc_transfer_to_THN = cs->deadcrootc_transfer * THNcoeff * storage_MGMmort; 
	cf->deadcrootc_storage_to_THN  = cs->deadcrootc_storage * THNcoeff * storage_MGMmort; 

	nf->deadcrootn_to_THN          = ns->deadcrootn * THNcoeff;
	nf->deadcrootn_transfer_to_THN = ns->deadcrootn_transfer * THNcoeff * storage_MGMmort; 
	nf->deadcrootn_storage_to_THN  = ns->deadcrootn_storage * THNcoeff * storage_MGMmort;  

	/* gresp pools */
	cf->gresp_storage_to_THN      = cs->gresp_storage * THNcoeff * storage_MGMmort; 
	cf->gresp_transfer_to_THN     = cs->gresp_transfer * THNcoeff * storage_MGMmort; 


	/* stem */
	cf->livestemc_to_THN           = cs->livestemc * THNcoeff;
	cf->livestemc_transfer_to_THN  = cs->livestemc_transfer * THNcoeff * storage_MGMmort; 
	cf->livestemc_storage_to_THN   = cs->livestemc_storage * THNcoeff * storage_MGMmort;  

	nf->livestemn_to_THN           = ns->livestemn * THNcoeff;
	nf->livestemn_transfer_to_THN  = ns->livestemn_transfer * THNcoeff * storage_MGMmort; 
	nf->livestemn_storage_to_THN   = ns->livestemn_storage * THNcoeff * storage_MGMmort;  

	cf->deadstemc_to_THN          = cs->deadstemc * THNcoeff;
	cf->deadstemc_transfer_to_THN = cs->deadstemc_transfer * THNcoeff * storage_MGMmort; 
	cf->deadstemc_storage_to_THN  = cs->deadstemc_storage * THNcoeff * storage_MGMmort; 

	nf->deadstemn_to_THN          = ns->deadstemn * THNcoeff;
	nf->deadstemn_transfer_to_THN = ns->deadstemn_transfer * THNcoeff * storage_MGMmort; 
	nf->deadstemn_storage_to_THN  = ns->deadstemn_storage * THNcoeff * storage_MGMmort; 

	/* restranslocated N pool is decreasing also */
	nf->retransn_to_THN        = ns->retransn * THNcoeff;

	wf->canopyw_to_THN = ws->canopyw * THNcoeff;

	/* standing dead biome */
	STDB_litr1c_to_THN = cs->STDB_litr1c * THNcoeff;
	STDB_litr2c_to_THN = cs->STDB_litr2c * THNcoeff;
	STDB_litr3c_to_THN = cs->STDB_litr3c * THNcoeff;
	STDB_litr4c_to_THN = cs->STDB_litr4c * THNcoeff;

	cf->STDBc_to_THN = STDB_litr1c_to_THN + STDB_litr2c_to_THN + STDB_litr3c_to_THN + STDB_litr4c_to_THN;

	STDB_litr1n_to_THN = ns->STDB_litr1n * THNcoeff;
	STDB_litr2n_to_THN = ns->STDB_litr2n * THNcoeff;
	STDB_litr3n_to_THN = ns->STDB_litr3n * THNcoeff;
	STDB_litr4n_to_THN = ns->STDB_litr4n * THNcoeff;

	nf->STDBn_to_THN = STDB_litr1n_to_THN + STDB_litr2n_to_THN + STDB_litr3n_to_THN + STDB_litr4n_to_THN;
   

	/**********************************************************************************************/
	/* 2. part of the plant material is transported (THN_to_transpC and THN_to_transpN; transp_coeff = 1-remained_prop),*/	
	/* transp:(leaf_total+fruit_total+gresp)*(1-remprop_nwoody) + (livestem_total+deadstem_total)* (1-remprop_woody) */
	

	THN_to_transpC = (cf->leafc_to_THN + cf->fruitc_to_THN)                            * (1-remained_prop_nwoody) +
					 (cf->livestemc_to_THN +  cf->deadstemc_to_THN + cf->STDBc_to_THN) * (1-remained_prop_woody);

	THN_to_transpN = (nf->leafn_to_THN + nf->fruitn_to_THN )				           * (1-remained_prop_nwoody) +
					 (nf->livestemn_to_THN + nf->deadstemn_to_THN + nf->STDBn_to_THN)  * (1-remained_prop_woody);
	
	/**********************************************************************************************/
	/* 3. the rest remains at the site (THN_to_litrc, THN_to_litrn, THN_to_cwdn, THN_to_cwdn) */
	
	/* litter:  */
	THN_to_CTDB_litr1c = (cf->leafc_to_THN * epc->leaflitr_flab   + cf->fruitc_to_THN * epc->fruitlitr_flab + cf->STDBc_to_THN)  * (remained_prop_nwoody) +
						  cf->frootc_to_THN * epc->frootlitr_flab + cf->frootc_transfer_to_THN  + cf->frootc_storage_to_THN +
						  cf->leafc_transfer_to_THN  + cf->leafc_storage_to_THN +
						  cf->fruitc_transfer_to_THN + cf->fruitc_storage_to_THN + 
						  cf->gresp_storage_to_THN  + cf->gresp_transfer_to_THN + 
						  cf->livecrootc_transfer_to_THN + cf->livecrootc_storage_to_THN + 
						  cf->deadcrootc_transfer_to_THN + cf->deadcrootc_storage_to_THN +
						  cf->livestemc_transfer_to_THN + cf->livestemc_storage_to_THN +
						  cf->deadstemc_transfer_to_THN + cf->deadstemc_storage_to_THN;
       


	THN_to_CTDB_litr2c = (cf->leafc_to_THN * epc->leaflitr_fucel  + cf->fruitc_to_THN * epc->fruitlitr_fucel) * remained_prop_nwoody +
		                  (cf->frootc_to_THN * epc->frootlitr_fucel);
	
	THN_to_CTDB_litr3c = (cf->leafc_to_THN * epc->leaflitr_fscel  + cf->fruitc_to_THN * epc->fruitlitr_fscel) * remained_prop_nwoody +
						  (cf->frootc_to_THN * epc->frootlitr_fscel);
	
	THN_to_CTDB_litr4c = (cf->leafc_to_THN * epc->leaflitr_flig   + cf->fruitc_to_THN * epc->fruitlitr_flig)  * remained_prop_nwoody +
		                  (cf->frootc_to_THN * epc->frootlitr_flig);
	
	/* cwd: temporary cwd storage pool which contains the cut-down part of coarse root and stem*/
	THN_to_CTDB_cwdc   = (cf->livecrootc_to_THN + cf->deadcrootc_to_THN) +
		                  (cf->livestemc_to_THN + cf->deadstemc_to_THN) * remained_prop_woody;


	/* litter:  */
	THN_to_CTDB_litr1n = (nf->leafn_to_THN * epc->leaflitr_flab  + nf->fruitn_to_THN * epc->fruitlitr_flab + nf->STDBn_to_THN) * remained_prop_nwoody +                                                            
		                  nf->frootn_to_THN * epc->frootlitr_flab+ nf->frootn_transfer_to_THN  + nf->frootn_storage_to_THN +	
						  nf->leafn_transfer_to_THN  + nf->leafn_storage_to_THN +
						  nf->fruitn_transfer_to_THN + nf->fruitn_storage_to_THN + 
						  nf->retransn_to_THN + 
						  nf->livestemn_transfer_to_THN + nf->livestemn_storage_to_THN + 
						  nf->deadstemn_transfer_to_THN + nf->deadstemn_storage_to_THN +  
						  nf->livecrootn_transfer_to_THN + nf->livecrootn_storage_to_THN + 
						  nf->deadcrootn_transfer_to_THN + nf->deadcrootn_storage_to_THN;

	THN_to_CTDB_litr2n =  (nf->leafn_to_THN * epc->leaflitr_fucel + nf->fruitn_to_THN * epc->fruitlitr_fucel) * remained_prop_nwoody +
						   nf->frootn_to_THN * epc->frootlitr_fucel;
	THN_to_CTDB_litr3n =  (nf->leafn_to_THN * epc->leaflitr_fscel + nf->fruitn_to_THN * epc->fruitlitr_fscel) * remained_prop_nwoody +
		                   nf->frootn_to_THN * epc->frootlitr_fscel;
	THN_to_CTDB_litr4n =  (nf->leafn_to_THN * epc->leaflitr_flig  + nf->fruitn_to_THN * epc->fruitlitr_flig)  * remained_prop_nwoody +
		                   nf->frootn_to_THN * epc->frootlitr_flig;
    /* cwd: temporary cwd storage pool which contains the cut-down part of coarse root and stem */
	THN_to_CTDB_cwdn   =  (nf->livecrootn_to_THN + nf->deadcrootn_to_THN) + 
		                  (nf->livestemn_to_THN + nf->deadstemn_to_THN) * remained_prop_woody;


	cs->CTDB_litr1c += THN_to_CTDB_litr1c;
	cs->CTDB_litr2c += THN_to_CTDB_litr2c;
	cs->CTDB_litr3c += THN_to_CTDB_litr3c;
	cs->CTDB_litr4c += THN_to_CTDB_litr4c;
	cs->CTDB_cwdc   += THN_to_CTDB_cwdc;

	cs->THN_transportC  += THN_to_transpC;


	ns->CTDB_litr1n += THN_to_CTDB_litr1n;
	ns->CTDB_litr2n += THN_to_CTDB_litr2n;
	ns->CTDB_litr3n += THN_to_CTDB_litr3n;
	ns->CTDB_litr4n += THN_to_CTDB_litr4n;
	ns->CTDB_cwdn   += THN_to_CTDB_cwdn;

	ns->THN_transportN  += THN_to_transpN;

	/**********************************************************************************************/
	/* 3.the remained part of the plant material gradually goes into the litter pool (litrc, litrn) */
 
	cf->THN_to_litr1c = cs->CTDB_litr1c * THN_to_litter_coeff_nw;
	cf->THN_to_litr2c = cs->CTDB_litr2c * THN_to_litter_coeff_nw;
	cf->THN_to_litr3c = cs->CTDB_litr3c * THN_to_litter_coeff_nw;
	cf->THN_to_litr4c = cs->CTDB_litr4c * THN_to_litter_coeff_nw;
	cf->THN_to_cwdc   = cs->CTDB_cwdc   * THN_to_litter_coeff_w;

	nf->THN_to_litr1n = ns->CTDB_litr1n * THN_to_litter_coeff_nw;
	nf->THN_to_litr2n = ns->CTDB_litr2n * THN_to_litter_coeff_nw;
	nf->THN_to_litr3n = ns->CTDB_litr3n * THN_to_litter_coeff_nw;
	nf->THN_to_litr4n = ns->CTDB_litr4n * THN_to_litter_coeff_nw;
	nf->THN_to_cwdn   = ns->CTDB_cwdn   * THN_to_litter_coeff_w;

		
	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                                    STATE UPDATE 
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/ 

	/* 1.CARBON */

	/* leaf */
	cs->THNsnk         += cf->leafc_to_THN;
	cs->leafc          -= cf->leafc_to_THN;
	cs->THNsnk         += cf->leafc_transfer_to_THN;
	cs->leafc_transfer -= cf->leafc_transfer_to_THN;
	cs->THNsnk         += cf->leafc_storage_to_THN;
	cs->leafc_storage  -= cf->leafc_storage_to_THN;

	/* froot */
	cs->THNsnk          += cf->frootc_to_THN;
	cs->frootc          -= cf->frootc_to_THN;
	cs->THNsnk          += cf->frootc_transfer_to_THN;
	cs->frootc_transfer -= cf->frootc_transfer_to_THN;
	cs->THNsnk          += cf->frootc_storage_to_THN;
	cs->frootc_storage  -= cf->frootc_storage_to_THN;

	/* fruit simulation*/
	cs->THNsnk          += cf->fruitc_to_THN;
	cs->fruitc          -= cf->fruitc_to_THN;
	cs->THNsnk          += cf->fruitc_transfer_to_THN;
	cs->fruitc_transfer -= cf->fruitc_transfer_to_THN;
	cs->THNsnk          += cf->fruitc_storage_to_THN;
	cs->fruitc_storage  -= cf->fruitc_storage_to_THN;

	/* gresp */
	cs->THNsnk          += cf->gresp_storage_to_THN;
	cs->gresp_storage   -= cf->gresp_storage_to_THN;
    cs->THNsnk          += cf->gresp_transfer_to_THN;
	cs->gresp_transfer  -= cf->gresp_transfer_to_THN;

	/* croot */
	cs->THNsnk              += cf->livecrootc_to_THN;
	cs->livecrootc          -= cf->livecrootc_to_THN;
	cs->THNsnk              += cf->livecrootc_transfer_to_THN;
	cs->livecrootc_transfer -= cf->livecrootc_transfer_to_THN;
	cs->THNsnk              += cf->livecrootc_storage_to_THN;
	cs->livecrootc_storage  -= cf->livecrootc_storage_to_THN;
	cs->THNsnk              += cf->deadcrootc_to_THN;
	cs->deadcrootc          -= cf->deadcrootc_to_THN;
	cs->THNsnk              += cf->deadcrootc_transfer_to_THN;
	cs->deadcrootc_transfer -= cf->deadcrootc_transfer_to_THN;
	cs->THNsnk              += cf->deadcrootc_storage_to_THN;
	cs->deadcrootc_storage  -= cf->deadcrootc_storage_to_THN; 

	/* livestem */
	cs->THNsnk              += cf->livestemc_to_THN;
	cs->livestemc           -= cf->livestemc_to_THN;
	cs->THNsnk              += cf->livestemc_transfer_to_THN;
	cs->livestemc_transfer  -= cf->livestemc_transfer_to_THN;
	cs->THNsnk              += cf->livestemc_storage_to_THN;
	cs->livestemc_storage   -= cf->livestemc_storage_to_THN;

	/* deadstem */
	cs->THNsnk              += cf->deadstemc_to_THN;
	cs->deadstemc           -= cf->deadstemc_to_THN;
	cs->THNsnk              += cf->deadstemc_transfer_to_THN;
	cs->deadstemc_transfer  -= cf->deadstemc_transfer_to_THN;
	cs->THNsnk              += cf->deadstemc_storage_to_THN;
	cs->deadstemc_storage   -= cf->deadstemc_storage_to_THN;

	/* dead standing biomass */
	cs->THNsnk += STDB_litr1c_to_THN;
	cs->STDB_litr1c -= STDB_litr1c_to_THN;
	cs->THNsnk += STDB_litr2c_to_THN;
	cs->STDB_litr2c -= STDB_litr2c_to_THN;
	cs->THNsnk += STDB_litr3c_to_THN;
	cs->STDB_litr3c -= STDB_litr3c_to_THN;
	cs->THNsnk += STDB_litr4c_to_THN;
	cs->STDB_litr4c -=STDB_litr4c_to_THN;

	cs->SNSCsrc += cf->STDBc_to_THN;
	cs->STDBc -= cf->STDBc_to_THN;

	/* litter */
	cs->litr1c += cf->THN_to_litr1c;
	cs->litr2c += cf->THN_to_litr2c;
	cs->litr3c += cf->THN_to_litr3c;
	cs->litr4c += cf->THN_to_litr4c;
	cs->cwdc   += cf->THN_to_cwdc;


	/* decreasing litter storage state variables*/
	cs->CTDB_litr1c -= cf->THN_to_litr1c;
	cs->CTDB_litr2c -= cf->THN_to_litr2c;
	cs->CTDB_litr3c -= cf->THN_to_litr3c;
	cs->CTDB_litr4c -= cf->THN_to_litr4c;
	cs->CTDB_cwdc   -= cf->THN_to_cwdc;

	cs->THNsrc += cf->THN_to_litr1c + cf->THN_to_litr2c + cf->THN_to_litr3c + cf->THN_to_litr4c + cf->THN_to_cwdc;

	/* 2. NITROGEN */

	/* leaf */
	ns->THNsnk += nf->leafn_to_THN;
	ns->leafn -= nf->leafn_to_THN;
	ns->THNsnk += nf->leafn_transfer_to_THN;
	ns->leafn_transfer -= nf->leafn_transfer_to_THN;
	ns->THNsnk += nf->leafn_storage_to_THN;
	ns->leafn_storage -= nf->leafn_storage_to_THN;	
	 
	/* froot */
	ns->THNsnk += nf->frootn_to_THN;
	ns->frootn -= nf->frootn_to_THN;
	ns->THNsnk += nf->frootn_transfer_to_THN;
	ns->frootn_transfer -= nf->frootn_transfer_to_THN;
	ns->THNsnk += nf->frootn_storage_to_THN;
	ns->frootn_storage -= nf->frootn_storage_to_THN;  	
	
	/* fruit simulation - Hidy 2013 */
	ns->THNsnk += nf->fruitn_to_THN;
	ns->fruitn -= nf->fruitn_to_THN;
	ns->THNsnk += nf->fruitn_transfer_to_THN;
	ns->fruitn_transfer -= nf->fruitn_transfer_to_THN;
	ns->THNsnk += nf->fruitn_storage_to_THN;
	ns->fruitn_storage -= nf->fruitn_storage_to_THN;	

	/* livestem */
	ns->THNsnk += nf->livestemn_to_THN;
	ns->livestemn -= nf->livestemn_to_THN;
	ns->THNsnk += nf->livestemn_transfer_to_THN;
	ns->livestemn_transfer -= nf->livestemn_transfer_to_THN;
	ns->THNsnk += nf->livestemn_storage_to_THN;
	ns->livestemn_storage -= nf->livestemn_storage_to_THN;	

	/* deadstem */	
	ns->THNsnk += nf->deadstemn_to_THN;
	ns->deadstemn -= nf->deadstemn_to_THN;
	ns->THNsnk += nf->deadstemn_transfer_to_THN;
	ns->deadstemn_transfer -= nf->deadstemn_transfer_to_THN;
	ns->THNsnk += nf->deadstemn_storage_to_THN;
	ns->deadstemn_storage -= nf->deadstemn_storage_to_THN;	

	/* croot */
	ns->THNsnk += nf->livecrootn_to_THN;
	ns->livecrootn -= nf->livecrootn_to_THN;
	ns->THNsnk += nf->livecrootn_transfer_to_THN;
	ns->livecrootn_transfer -= nf->livecrootn_transfer_to_THN;
	ns->THNsnk += nf->livecrootn_storage_to_THN;
	ns->livecrootn_storage -= nf->livecrootn_storage_to_THN;		

	ns->THNsnk += nf->deadcrootn_to_THN;
	ns->deadcrootn -= nf->deadcrootn_to_THN;
	ns->THNsnk += nf->deadcrootn_transfer_to_THN;
	ns->deadcrootn_transfer -= nf->deadcrootn_transfer_to_THN;
	ns->THNsnk += nf->deadcrootn_storage_to_THN;
	ns->deadcrootn_storage -= nf->deadcrootn_storage_to_THN; 

	/* retrans */
	ns->THNsnk   += nf->retransn_to_THN;
	ns->retransn -= nf->retransn_to_THN;

	/* dead standing biomass */
	ns->THNsnk += STDB_litr1n_to_THN;
	ns->STDB_litr1n -= STDB_litr1n_to_THN;
	ns->THNsnk += STDB_litr2n_to_THN;
	ns->STDB_litr2n -= STDB_litr2n_to_THN;
	ns->THNsnk += STDB_litr3n_to_THN;
	ns->STDB_litr3n -= STDB_litr3n_to_THN;
	ns->THNsnk += STDB_litr4n_to_THN;
	ns->STDB_litr4n -=STDB_litr4n_to_THN;

	ns->SNSCsrc += nf->STDBn_to_THN;
	ns->STDBn -= nf->STDBn_to_THN;
	
	/* litter */
	ns->litr1n += nf->THN_to_litr1n;
	ns->litr2n += nf->THN_to_litr2n;
	ns->litr3n += nf->THN_to_litr3n;
	ns->litr4n += nf->THN_to_litr4n;
	ns->cwdn   += nf->THN_to_cwdn;

	/* decreasing litter storage state variables*/
	ns->CTDB_litr1n -= nf->THN_to_litr1n;
	ns->CTDB_litr2n -= nf->THN_to_litr2n;
	ns->CTDB_litr3n -= nf->THN_to_litr3n;
	ns->CTDB_litr4n -= nf->THN_to_litr4n;
	ns->CTDB_cwdn   -= nf->THN_to_cwdn;
	
	ns->THNsrc += nf->THN_to_litr1n + nf->THN_to_litr2n + nf->THN_to_litr3n + nf->THN_to_litr4n + nf->THN_to_cwdn;

	/* 3. water */
	ws->canopyw_THNsnk += wf->canopyw_to_THN;
	ws->canopyw -= wf->canopyw_to_THN;

		/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                                    TEMPORARY POOLS
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
	/* temporary THNed plant material pools: if CTDBc_litr1c is less than a crit. value, the temporary pool becomes empty */

	if (cs->CTDB_litr1c < CRIT_PREC && cs->CTDB_litr1c != 0) 
	{
		cs->THNsrc += (cs->CTDB_litr1c + cs->CTDB_litr2c + cs->CTDB_litr3c + cs->CTDB_litr4c);
		cs->CTDB_litr1c = 0;
		cs->CTDB_litr2c = 0;
		cs->CTDB_litr3c = 0;
		cs->CTDB_litr4c = 0;
		ns->THNsrc += (ns->CTDB_litr1n + ns->CTDB_litr2n + ns->CTDB_litr3n + ns->CTDB_litr4n);
		ns->CTDB_litr1n = 0;
		ns->CTDB_litr2n = 0;
		ns->CTDB_litr3n = 0;
		ns->CTDB_litr4n = 0;
	}

	if (cs->CTDB_cwdc < CRIT_PREC && cs->CTDB_cwdc != 0) 
	{
		cs->THNsrc += cs->CTDB_cwdc;
		ns->THNsrc += ns->CTDB_cwdn;
	
		cs->CTDB_cwdc   = 0;
		ns->CTDB_cwdn   = 0;
	}

	/* CONTROL */
	diffC = (cs->THNsnk - cs->THNsrc) - cs->THN_transportC -  
		    (cs->CTDB_litr1c + cs->CTDB_litr2c + cs->CTDB_litr3c + cs->CTDB_litr4c) - cs->CTDB_cwdc;

	diffN = (ns->THNsnk - ns->THNsrc) - ns->THN_transportN -  
		    (ns->CTDB_litr1n + ns->CTDB_litr2n + ns->CTDB_litr3n + ns->CTDB_litr4n) - ns->CTDB_cwdn;

	if (fabs(diffC) > 10-8 || fabs(diffN) > 10-8)
	{
		 printf("Warning: rounding error in thinning pools (thinning.c)\n");
	}



   return (!ok);
}
	