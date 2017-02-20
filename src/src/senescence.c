/* 
senescence.c
calculation of daily senescence mortality fluxes (due to drought/water stress)
Senescence mortality: these fluxes all enter litter sinks due to  low VWC during a long period

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.3
Copyright 2016, D. Hidy [dori.hidy@gmail.com]
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

int senescence(const epconst_struct* epc, const grazing_struct* GRZ,
			   cstate_struct* cs, cflux_struct* cf,nstate_struct* ns, nflux_struct* nf, epvar_struct* epv)
{
	int ok=1;

	/* mortality parameters */
	double mort_SNSC_abovebiom = 0;
	double mort_SNSC_belowbiom = 0;
	double mort_SNSC_leafphen = 0;
	double SNSC_coeff = 0;
	double smsi_effect, dsws_effectA, dsws_effectB;

	double litr1c_STDB_to_SNSC, litr2c_STDB_to_SNSC, litr3c_STDB_to_SNSC, litr4c_STDB_to_SNSC;
	double litr1n_STDB_to_SNSC, litr2n_STDB_to_SNSC, litr3n_STDB_to_SNSC, litr4n_STDB_to_SNSC;


	
	/************************************************************/
	/* 1. calculating mortality parameter (maximum: 1) */

	/* 1.1 SWC EFFECT */
	

	if (cs->leafc > 0 && ns->leafn > 0 && epv->dsws > 0)
	{	
		/* if m_soilstress <= m_soilstress_crit -> 0; if m_soilstress = 1 -> 1; betweeen them monotonically increasing */
		if (epv->m_soilstress < epc->m_soilstress_crit)
			smsi_effect = (1 - epv->m_soilstress/epc->m_soilstress_crit);
		else
			smsi_effect = 1;


		if (epv->dsws < epc->n_stressdays_crit)
			dsws_effectA = (epv->dsws/epc->n_stressdays_crit);
		else
			dsws_effectA = 1;

		if (epv->dsws < epc->n_stressdays_crit)
			dsws_effectB = epv->dsws /epc->n_stressdays_crit;
		else
			dsws_effectB = 1;

		if (dsws_effectB > 0.1) dsws_effectB = 0.1;


		if (epc->mort_SNSC_abovebiom == 0)
			mort_SNSC_abovebiom = 0;
		else
			mort_SNSC_abovebiom = epc->mort_SNSC_abovebiom + (1 - epc->mort_SNSC_abovebiom) * smsi_effect * dsws_effectA;
		
		if (epc->mort_SNSC_belowbiom == 0)
			mort_SNSC_belowbiom = 0;
		else
			mort_SNSC_belowbiom = epc->mort_SNSC_belowbiom + (1 - epc->mort_SNSC_belowbiom) * smsi_effect * dsws_effectB; 
	
	}	
	else
	{
		smsi_effect = 0;
		dsws_effectA = 0;
		dsws_effectB = 0;
		mort_SNSC_abovebiom = 0;
		mort_SNSC_belowbiom = 0;
	}

	/* 1.2 PHENOLOGY EFFECT - only regarding to leaf */

	if (epv->maturity)
		mort_SNSC_leafphen = epc->mort_SNSC_leafphen;
	else
		mort_SNSC_leafphen = 0;

	if (mort_SNSC_abovebiom + mort_SNSC_leafphen > 1) mort_SNSC_leafphen = 1 - mort_SNSC_abovebiom;

	
	/************************************************************/
	/* 2. determine the actual turnover rate of wilted standing biomass to litter parameter */

	if (cf->leafc_to_GRZ > 0)
	{
		SNSC_coeff = GRZ->trampleff_act * epc->mort_SNSC_to_litter;
		if (SNSC_coeff > 1) SNSC_coeff = 1;
	}
	else SNSC_coeff = epc->mort_SNSC_to_litter;
	
	
	/************************************************************/
	/* 3. CARBON mortality state variable update */	

	/* leaf and fine root  mortality fluxes out of leaf and fine root pools */
	cf->m_leafc_to_SNSC				= (mort_SNSC_abovebiom + mort_SNSC_leafphen) * cs->leafc;  
	cf->m_leafc_storage_to_SNSC		=  mort_SNSC_belowbiom * cs->leafc_storage;
	cf->m_leafc_transfer_to_SNSC	=  mort_SNSC_belowbiom * cs->leafc_transfer;
	
	cf->m_gresp_storage_to_SNSC		= mort_SNSC_belowbiom * cs->gresp_storage;
	cf->m_gresp_transfer_to_SNSC	= mort_SNSC_belowbiom * cs->gresp_transfer;

	cf->m_frootc_to_SNSC			= mort_SNSC_belowbiom * cs->frootc;	 
	cf->m_frootc_storage_to_SNSC	= mort_SNSC_belowbiom * cs->frootc_storage;	
	cf->m_frootc_transfer_to_SNSC	= mort_SNSC_belowbiom * cs->frootc_transfer;

	/* fruit simulation - Hidy 2013. */
	cf->m_fruitc_to_SNSC			= 0;	 
	cf->m_fruitc_storage_to_SNSC	= 0;	
	cf->m_fruitc_transfer_to_SNSC	= 0;

	/* softstem simulation - Hidy 2013. */
	cf->m_softstemc_to_SNSC			= mort_SNSC_abovebiom * cs->softstemc;	 
	cf->m_softstemc_storage_to_SNSC	= mort_SNSC_belowbiom * cs->softstemc_storage;	
	cf->m_softstemc_transfer_to_SNSC= mort_SNSC_belowbiom * cs->softstemc_transfer;

	/* update state variables - decreasing state variables */
	cs->leafc			-= cf->m_leafc_to_SNSC;
	cs->leafc_storage	-= cf->m_leafc_storage_to_SNSC;
	cs->leafc_transfer	-= cf->m_leafc_transfer_to_SNSC; 
	cs->frootc			-= cf->m_frootc_to_SNSC;
	cs->frootc_storage	-= cf->m_frootc_storage_to_SNSC; 
	cs->frootc_transfer -= cf->m_frootc_transfer_to_SNSC; 

	/* fruit simulation - Hidy 2013. */
	cs->fruitc			-= cf->m_fruitc_to_SNSC;
	cs->fruitc_storage	-= cf->m_fruitc_storage_to_SNSC; 
	cs->fruitc_transfer -= cf->m_fruitc_transfer_to_SNSC; 
	/* softstem simulation - Hidy 2013. */
	cs->softstemc		   -= cf->m_softstemc_to_SNSC;
	cs->softstemc_storage  -= cf->m_softstemc_storage_to_SNSC; 
	cs->softstemc_transfer -= cf->m_softstemc_transfer_to_SNSC; 

	cs->gresp_storage	-= cf->m_gresp_storage_to_SNSC;
	cs->gresp_transfer	-= cf->m_gresp_transfer_to_SNSC;
	
	/* plant material losses because of senescence */
	cf->m_vegc_to_SNSC  += cf->m_leafc_to_SNSC + cf->m_frootc_to_SNSC + cf->m_fruitc_to_SNSC +  cf->m_softstemc_to_SNSC + 
						cf->m_leafc_storage_to_SNSC + cf->m_frootc_storage_to_SNSC + cf->m_fruitc_storage_to_SNSC + cf->m_softstemc_storage_to_SNSC + 
						cf->m_leafc_transfer_to_SNSC + cf->m_frootc_transfer_to_SNSC + cf->m_fruitc_transfer_to_SNSC + cf->m_softstemc_transfer_to_SNSC + 
						cf->m_gresp_storage_to_SNSC + cf->m_gresp_transfer_to_SNSC;
	cs->SNSCsnk  += cf->m_vegc_to_SNSC; 
						
	/* update state variables - increasing litter storage state variables*/
	cs->litr1c_STDB += (cf->m_leafc_to_SNSC     * epc->leaflitr_flab     + cf->m_leafc_storage_to_SNSC     + cf->m_leafc_transfer_to_SNSC + 
							 cf->m_fruitc_to_SNSC    * epc->fruitlitr_flab    + cf->m_fruitc_storage_to_SNSC    + cf->m_fruitc_transfer_to_SNSC + 
							 cf->m_softstemc_to_SNSC * epc->softstemlitr_flab + cf->m_softstemc_storage_to_SNSC + cf->m_softstemc_transfer_to_SNSC + 	
							 cf->m_gresp_storage_to_SNSC + cf->m_gresp_transfer_to_SNSC) ;
	cs->litr2c_STDB  += cf->m_leafc_to_SNSC * epc->leaflitr_fucel + 
							 cf->m_fruitc_to_SNSC * epc->fruitlitr_fucel + cf->m_softstemc_to_SNSC * epc->softstemlitr_fucel;
	cs->litr3c_STDB  += cf->m_leafc_to_SNSC  * epc->leaflitr_fscel   + 
							 cf->m_fruitc_to_SNSC * epc->fruitlitr_fscel  + cf->m_softstemc_to_SNSC * epc->softstemlitr_fscel;
	cs->litr4c_STDB  += cf->m_leafc_to_SNSC  * epc->leaflitr_flig    + 
							 cf->m_fruitc_to_SNSC * epc->fruitlitr_flig   + cf->m_softstemc_to_SNSC* epc->softstemlitr_flig;

	/* mortality fluxes into litter pools */
	cf->SNSC_to_litr1c = cs->litr1c_STDB * SNSC_coeff + 
						 cf->m_frootc_to_SNSC * epc->frootlitr_flab + cf->m_frootc_storage_to_SNSC + cf->m_frootc_transfer_to_SNSC;
	cf->SNSC_to_litr2c = cs->litr2c_STDB * SNSC_coeff +
						 cf->m_frootc_to_SNSC * epc->frootlitr_fucel;
	cf->SNSC_to_litr3c = cs->litr3c_STDB * SNSC_coeff +
						 cf->m_frootc_to_SNSC * epc->frootlitr_fscel;
	cf->SNSC_to_litr4c = cs->litr4c_STDB * SNSC_coeff +
						 cf->m_frootc_to_SNSC * epc->frootlitr_flig;


	cs->litr1c  += cf->SNSC_to_litr1c;
	cs->litr2c  += cf->SNSC_to_litr2c;
	cs->litr3c  += cf->SNSC_to_litr3c;
	cs->litr4c  += cf->SNSC_to_litr4c;
	
	/* litter plus because of senescenceing out and returning of dead plant material */
	cs->SNSCsrc       += cf->SNSC_to_litr1c + cf->SNSC_to_litr2c + cf->SNSC_to_litr3c + cf->SNSC_to_litr4c;

	
	/* standing dead biomass to litter */
	litr1c_STDB_to_SNSC = cs->litr1c_STDB * SNSC_coeff;
	litr2c_STDB_to_SNSC = cs->litr2c_STDB * SNSC_coeff;
	litr3c_STDB_to_SNSC = cs->litr3c_STDB * SNSC_coeff;
	litr4c_STDB_to_SNSC = cs->litr4c_STDB * SNSC_coeff;

	cs->litr1c_STDB -= litr1c_STDB_to_SNSC;
	cs->litr2c_STDB -= litr2c_STDB_to_SNSC;
	cs->litr3c_STDB -= litr3c_STDB_to_SNSC;
	cs->litr4c_STDB -= litr4c_STDB_to_SNSC;
	
	cs->STDBc           = cs->litr1c_STDB + cs->litr2c_STDB + cs->litr3c_STDB + cs->litr4c_STDB;
	cf->m_STDBc_to_SNSC = litr1c_STDB_to_SNSC + litr2c_STDB_to_SNSC + litr3c_STDB_to_SNSC + litr4c_STDB_to_SNSC;

	/************************************************************/
	/* 4. NITROGEN mortality state variable update */	
	/* Leaf and fine root mortality fluxes out of leaf and fine root pools */

	nf->m_leafn_to_SNSC				= cf->m_leafc_to_SNSC / epc->leaf_cn;  	
	nf->m_leafn_storage_to_SNSC		= mort_SNSC_belowbiom * ns->leafn_storage;
	nf->m_leafn_transfer_to_SNSC	= mort_SNSC_belowbiom * ns->leafn_transfer;
	nf->m_frootn_to_SNSC			= cf->m_frootc_to_SNSC / epc->froot_cn; 
	nf->m_frootn_storage_to_SNSC	= mort_SNSC_belowbiom * ns->frootn_storage;	
	nf->m_frootn_transfer_to_SNSC	= mort_SNSC_belowbiom * ns->frootn_transfer;

	/* fruit simulation - Hidy 2013. */
	nf->m_fruitn_to_SNSC			= 0; 
	nf->m_fruitn_storage_to_SNSC	= 0;	
	nf->m_fruitn_transfer_to_SNSC	= 0;
	
	/* softstem simulation - Hidy 2013. */
	nf->m_softstemn_to_SNSC			= cf->m_softstemc_to_SNSC / epc->softstem_cn; 
	nf->m_softstemn_storage_to_SNSC	= mort_SNSC_belowbiom * ns->softstemn_storage;	
	nf->m_softstemn_transfer_to_SNSC	= mort_SNSC_belowbiom * ns->softstemn_transfer;
	
	nf->m_retransn_to_SNSC			= mort_SNSC_belowbiom * ns->retransn;	


	/* precision control */
	ns->leafn				-= nf->m_leafn_to_SNSC;
	ns->leafn_storage		-= nf->m_leafn_storage_to_SNSC;
	ns->leafn_transfer		-= nf->m_leafn_transfer_to_SNSC;
	ns->frootn				-= nf->m_frootn_to_SNSC;
	ns->frootn_storage		-= nf->m_frootn_storage_to_SNSC;
	ns->frootn_transfer     -= nf->m_frootn_transfer_to_SNSC;
	/* fruit simulation - Hidy 2013. */
	ns->fruitn				-= nf->m_fruitn_to_SNSC;
	ns->fruitn_storage		-= nf->m_fruitn_storage_to_SNSC;
	ns->fruitn_transfer     -= nf->m_fruitn_transfer_to_SNSC;
	/* softstem simulation - Hidy 2013. */
	ns->softstemn			-= nf->m_softstemn_to_SNSC;
	ns->softstemn_storage   -= nf->m_softstemn_storage_to_SNSC;
	ns->softstemn_transfer  -= nf->m_softstemn_transfer_to_SNSC;
	
	ns->retransn			-= nf->m_retransn_to_SNSC;


	/* plant material losses because of senescence */
	ns->SNSCsnk  += nf->m_leafn_to_SNSC + nf->m_frootn_to_SNSC + nf->m_fruitn_to_SNSC + nf->m_softstemn_to_SNSC + 
					nf->m_leafn_storage_to_SNSC + nf->m_frootn_storage_to_SNSC + nf->m_fruitn_storage_to_SNSC +  + nf->m_softstemn_storage_to_SNSC + 
					nf->m_leafn_transfer_to_SNSC + nf->m_frootn_transfer_to_SNSC + nf->m_fruitn_transfer_to_SNSC + nf->m_softstemn_transfer_to_SNSC +
					nf->m_retransn_to_SNSC;


	/* increasing litter storage state variables*/
	ns->litr1n_STDB  += (nf->m_leafn_to_SNSC * epc->leaflitr_flab) + nf->m_leafn_storage_to_SNSC + nf->m_leafn_transfer_to_SNSC +
	                     	 (nf->m_fruitn_to_SNSC * epc->fruitlitr_flab) + nf->m_fruitn_storage_to_SNSC + nf->m_fruitn_transfer_to_SNSC +
							 (nf->m_softstemn_to_SNSC * epc->softstemlitr_flab) + nf->m_softstemn_storage_to_SNSC + nf->m_softstemn_transfer_to_SNSC +
							  nf->m_retransn_to_SNSC;
	ns->litr2n_STDB  += (nf->m_leafn_to_SNSC * epc->leaflitr_fucel +
		                      nf->m_fruitn_to_SNSC * epc->fruitlitr_fucel + nf->m_softstemn_to_SNSC * epc->softstemlitr_fucel);
	ns->litr3n_STDB  += (nf->m_leafn_to_SNSC * epc->leaflitr_fscel +
		                      nf->m_fruitn_to_SNSC * epc->fruitlitr_fscel + nf->m_softstemn_to_SNSC * epc->softstemlitr_fscel);
	ns->litr4n_STDB  += (nf->m_leafn_to_SNSC* epc->leaflitr_flig +
		                      nf->m_fruitn_to_SNSC* epc->fruitlitr_flig   + nf->m_softstemn_to_SNSC* epc->softstemlitr_flig);

	/* mortality fluxes into litter pools */
	nf->SNSC_to_litr1n = ns->litr1n_STDB * SNSC_coeff + 
						 nf->m_frootn_to_SNSC * epc->frootlitr_flab + nf->m_frootn_storage_to_SNSC + nf->m_frootn_transfer_to_SNSC;
	nf->SNSC_to_litr2n = ns->litr2n_STDB * SNSC_coeff +
						 nf->m_frootn_to_SNSC * epc->frootlitr_fucel;
	nf->SNSC_to_litr3n = ns->litr3n_STDB * SNSC_coeff +
						 nf->m_frootn_to_SNSC * epc->frootlitr_fscel;
	nf->SNSC_to_litr4n = ns->litr4n_STDB * SNSC_coeff +
						 nf->m_frootn_to_SNSC * epc->frootlitr_flig;

	/* update litter state variables */ 
	ns->litr1n  += nf->SNSC_to_litr1n;
	ns->litr2n  += nf->SNSC_to_litr2n;
	ns->litr3n  += nf->SNSC_to_litr3n;
	ns->litr4n  += nf->SNSC_to_litr4n;

	/* litter plus because of senescenceing out and returning of dead plant material */
	ns->SNSCsrc  += nf->SNSC_to_litr1n + nf->SNSC_to_litr2n + nf->SNSC_to_litr3n + nf->SNSC_to_litr4n;


	/* standing dead biomass to litter */
	litr1n_STDB_to_SNSC = ns->litr1n_STDB * SNSC_coeff;
	litr2n_STDB_to_SNSC = ns->litr2n_STDB * SNSC_coeff;
	litr3n_STDB_to_SNSC = ns->litr3n_STDB * SNSC_coeff;
	litr4n_STDB_to_SNSC = ns->litr4n_STDB * SNSC_coeff;

	ns->litr1n_STDB -= litr1n_STDB_to_SNSC;
	ns->litr2n_STDB -= litr2n_STDB_to_SNSC;
	ns->litr3n_STDB -= litr3n_STDB_to_SNSC;
	ns->litr4n_STDB -= litr4n_STDB_to_SNSC;
	
	ns->STDBn           = ns->litr1n_STDB + ns->litr2n_STDB + ns->litr3n_STDB + ns->litr4n_STDB;
	nf->m_STDBn_to_SNSC = litr1n_STDB_to_SNSC + litr2n_STDB_to_SNSC + litr3n_STDB_to_SNSC + litr4n_STDB_to_SNSC;

	/************************************************************/
	/* decreasing of temporary pool */


	if (cs->litr1c_STDB < CRIT_PREC && cs->litr1c_STDB != 0) 
	{
		cs->SNSCsrc += cs->litr1c_STDB + cs->litr2c_STDB + cs->litr3c_STDB + cs->litr4c_STDB;
		cs->litr1c_STDB = 0;
		cs->litr2c_STDB = 0;
		cs->litr3c_STDB = 0;
		cs->litr4c_STDB = 0;
		ns->SNSCsrc += ns->litr1n_STDB + ns->litr2n_STDB + ns->litr3n_STDB + ns->litr4n_STDB;
		ns->litr1n_STDB = 0;
		ns->litr2n_STDB = 0;
		ns->litr3n_STDB = 0;
		ns->litr4n_STDB = 0;
	}
	
	return (!ok);
}

