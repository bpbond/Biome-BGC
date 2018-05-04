/* 
senescence.c
calculation of daily senescence mortality fluxes (due to drought/water stress)
Senescence mortality: these fluxes all enter litter sinks due to  low VWC during a long period

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0.
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

int senescence(int yday, const epconst_struct* epc, const grazing_struct* GRZ, const metvar_struct* metv, const phenology_struct* phen, 
			   cstate_struct* cs, cflux_struct* cf,nstate_struct* ns, nflux_struct* nf, epvar_struct* epv)
{
	int ok=1;
	int layer;

	/* mortality parameters */
	double SNSCmort_abovebiom, SNSCmort_belowbiom, SNSCmort_nsc;

	double mort_SNSC_to_litter = 0;
	double SMSI, dsws_effect, extremT_effect;
	
	int onday          = (int) phen->onday;
	int offday         = (int) phen->offday;


	SNSCmort_abovebiom=SNSCmort_belowbiom=SNSCmort_nsc=SMSI = 0;
	
	/****************************************************************************************/
	/* 1. calculating mortality parameter (maximum: 1) */

		
	/* 1.1 calculating WATER STRESS DAYS regarding to the average soil moisture conditions */

	if (epv->m_soilstress == 1) epv->dsws = 0;
	if (yday > onday && yday < offday)
		epv->dsws += (1 - epv->m_soilstress);
	else
		epv->dsws = 0;
	
	if (epv->dsws < epc->dsws_crit)
		dsws_effect = epv->dsws/epc->dsws_crit;
	else
		dsws_effect = 1;

	/* if less than min or greater than max -> 0 */
	if (metv->tmax < epc->SNSC_extremT1)
		extremT_effect = 1;
	else
	{
		/*  above critical temperature -> max, below linearly decreasing */
		if (metv->tmax < epc->SNSC_extremT2)
			extremT_effect = 1 + (metv->tmax - epc->SNSC_extremT1) / (epc->SNSC_extremT2 - epc->SNSC_extremT1) * (epc->extremT_effect_max - 1);
		else
			extremT_effect = epc->extremT_effect_max;
			
	}
	/* control */
	if (extremT_effect < 0 || extremT_effect > epc->extremT_effect_max)
	{
		printf("\n");
		printf("FATAL ERROR in extremT_effect calculation (senescence.c)\n");
		ok=0;
	}

	
	/* 1.2 sms_effect */

	if ((cs->leafc > 0 && ns->leafn > 0) || (cs->frootc > 0 && ns->frootn > 0) || (cs->softstemc > 0 && ns->softstemn > 0))
	{	
		
		SMSI = (1 - epv->m_soilstress) * dsws_effect;


		SNSCmort_abovebiom = (epc->SNSCmort_abovebiom_max * SMSI) * extremT_effect;
		if (SNSCmort_abovebiom > 1) SNSCmort_abovebiom = 1;
		
		SNSCmort_belowbiom = (epc->SNSCmort_belowbiom_max * SMSI) * extremT_effect; 
		if (SNSCmort_belowbiom > 1) SNSCmort_belowbiom = 1;

		SNSCmort_nsc = (epc->SNSCmort_nsc_max * SMSI) * extremT_effect; 
		if (SNSCmort_nsc > 1) SNSCmort_nsc = 1;
	
	}	

	epv->SMSI = SMSI;


	/* 1.3. determine the actual turnover rate of wilted standing biomass to litter parameter */
	if (cf->leafc_to_GRZ > 0)
	{
		mort_SNSC_to_litter = GRZ->trampleff_act * epc->mort_SNSC_to_litter;
		if (mort_SNSC_to_litter > 1) mort_SNSC_to_litter = 1;
	}
	else mort_SNSC_to_litter = epc->mort_SNSC_to_litter;


	/****************************************************************************************/
	/* 2. genetically programmed senescence */

	if (epc->mort_MAX_lifetime != DATA_GAP)
	{	
		if (ok && genprog_senescence(epc, metv, epv, cf, nf))
		{
			printf("\n");
			printf("Error in call to genprog_senescence() from senescence()\n");
			ok=0;
		}
	}
	
	/****************************************************************************************/
	/* 3.  mortality fluxes:leaf, fine root, fruit, softstem, gresp, retrans */

	if (SNSCmort_belowbiom > 0 || SNSCmort_belowbiom > 0 || SNSCmort_nsc > 0)
	{
		/* in order to save the C:N ratio: N-fluxes are calculated from C-fluxes using C:N ratio parameters */
		if (epc->leaf_cn)
		{
			cf->m_leafc_to_SNSC				= SNSCmort_abovebiom * cs->leafc;  
			cf->m_leafc_storage_to_SNSC		= SNSCmort_nsc * cs->leafc_storage;
			cf->m_leafc_transfer_to_SNSC	= SNSCmort_nsc * cs->leafc_transfer;
	
			nf->m_leafn_to_SNSC				= cf->m_leafc_to_SNSC / epc->leaf_cn;  	
			nf->m_leafn_storage_to_SNSC		= cf->m_leafc_storage_to_SNSC / epc->leaf_cn; 
			nf->m_leafn_transfer_to_SNSC	= cf->m_leafc_transfer_to_SNSC / epc->leaf_cn; 
		}

		if (epc->froot_cn)
		{
			cf->m_frootc_to_SNSC			= SNSCmort_belowbiom * cs->frootc;	 
			cf->m_frootc_storage_to_SNSC	= SNSCmort_nsc * cs->frootc_storage;	
			cf->m_frootc_transfer_to_SNSC	= SNSCmort_nsc * cs->frootc_transfer;
	
			nf->m_frootn_to_SNSC			= cf->m_frootc_to_SNSC / epc->froot_cn;
			nf->m_frootn_storage_to_SNSC	= cf->m_frootc_storage_to_SNSC / epc->froot_cn;
			nf->m_frootn_transfer_to_SNSC	= cf->m_frootc_transfer_to_SNSC / epc->froot_cn;
		}
	
		if (epc->softstem_cn)
		{
			cf->m_softstemc_to_SNSC			= SNSCmort_abovebiom * cs->softstemc;	 
			cf->m_softstemc_storage_to_SNSC	= SNSCmort_nsc * cs->softstemc_storage;	
			cf->m_softstemc_transfer_to_SNSC= SNSCmort_nsc * cs->softstemc_transfer;

			nf->m_softstemn_to_SNSC			= cf->m_softstemc_to_SNSC / epc->softstem_cn;
			nf->m_softstemn_storage_to_SNSC	= cf->m_softstemc_storage_to_SNSC / epc->softstem_cn;	
			nf->m_softstemn_transfer_to_SNSC= cf->m_softstemc_transfer_to_SNSC / epc->softstem_cn;
		}
	
		cf->m_fruitc_to_SNSC			= 0; 
		cf->m_fruitc_storage_to_SNSC	= 0;	
		cf->m_fruitc_transfer_to_SNSC	= 0;
	
		nf->m_fruitn_to_SNSC			= 0; 
		nf->m_fruitn_storage_to_SNSC	= 0;	
		nf->m_fruitn_transfer_to_SNSC	= 0;

		cf->m_gresp_storage_to_SNSC		= SNSCmort_nsc * cs->gresp_storage;
		cf->m_gresp_transfer_to_SNSC	= SNSCmort_nsc * cs->gresp_transfer;
	
		nf->m_retransn_to_SNSC			= SNSCmort_nsc * ns->retransn;	
	}

	/****************************************************************************************/
	/* 4. update state variables - decreasing state variables */
	
	if (SNSCmort_belowbiom > 0 || SNSCmort_belowbiom > 0 || SNSCmort_nsc > 0)
	{
		cs->leafc			-= cf->m_leafc_to_SNSC;
		cs->leafc_storage	-= cf->m_leafc_storage_to_SNSC;
		cs->leafc_transfer	-= cf->m_leafc_transfer_to_SNSC;
	
		cs->frootc			-= cf->m_frootc_to_SNSC;
		cs->frootc_storage	-= cf->m_frootc_storage_to_SNSC; 
		cs->frootc_transfer -= cf->m_frootc_transfer_to_SNSC; 
	
		cs->fruitc			-= cf->m_fruitc_to_SNSC; 
		cs->fruitc_storage	-= cf->m_fruitc_storage_to_SNSC; 
		cs->fruitc_transfer -= cf->m_fruitc_transfer_to_SNSC; 
	
		cs->softstemc		   -= cf->m_softstemc_to_SNSC; 
		cs->softstemc_storage  -= cf->m_softstemc_storage_to_SNSC; 
		cs->softstemc_transfer -= cf->m_softstemc_transfer_to_SNSC; 

		cs->gresp_storage	-= cf->m_gresp_storage_to_SNSC;
		cs->gresp_transfer	-= cf->m_gresp_transfer_to_SNSC;

		ns->leafn				-= nf->m_leafn_to_SNSC;
		ns->leafn_storage		-= nf->m_leafn_storage_to_SNSC;
		ns->leafn_transfer		-= nf->m_leafn_transfer_to_SNSC;

		ns->frootn				-= nf->m_frootn_to_SNSC;
		ns->frootn_storage		-= nf->m_frootn_storage_to_SNSC;
		ns->frootn_transfer     -= nf->m_frootn_transfer_to_SNSC;

		ns->fruitn				-= nf->m_fruitn_to_SNSC;
		ns->fruitn_storage		-= nf->m_fruitn_storage_to_SNSC;
		ns->fruitn_transfer     -= nf->m_fruitn_transfer_to_SNSC;

		ns->softstemn			-= nf->m_softstemn_to_SNSC;
		ns->softstemn_storage   -= nf->m_softstemn_storage_to_SNSC;
		ns->softstemn_transfer  -= nf->m_softstemn_transfer_to_SNSC;

		ns->retransn			-= nf->m_retransn_to_SNSC;	
	}


	/****************************************************************************************/
	/* 5. update state variables with genetically programmed leaf senescence */

	if (epc->mort_MAX_lifetime != DATA_GAP && cf->m_leafc_to_SNSCgenprog != 0)
	{
		if (cf->m_leafc_to_SNSCgenprog <= cs->leafc)
		{
			cs->leafc           -= cf->m_leafc_to_SNSCgenprog;  
		}
		else
		{
			cf->m_leafc_to_SNSCgenprog = cs->leafc;
			cs->leafc = 0;
			printf("INFORMATION: limited genetically programmed leaf senescence\n");
		}

		if (nf->m_leafn_to_SNSCgenprog <= ns->leafn)
		{
			ns->leafn           -= nf->m_leafn_to_SNSCgenprog;  
		}
		else
		{
			nf->m_leafn_to_SNSCgenprog = ns->leafn;
			ns->leafn = 0;
			printf("INFORMATION: limited genetically programmed leaf senescence\n");
		}

		/* control */
		if ((ns->leafn == 0 && cs->leafc !=0) || (ns->leafn != 0 && cs->leafc ==0))
		{
			printf("ERROR: in genetically programmed leaf senescence calculation in senescence.c\n");
			ok=0;
		}
	}
	/****************************************************************************************/
	/* 6. plant material losses because of senescence */
	
	cf->m_vegc_to_SNSC  += cf->m_leafc_to_SNSC     + cf->m_leafc_storage_to_SNSC     + cf->m_leafc_transfer_to_SNSC  +   cf->m_leafc_to_SNSCgenprog + 
						   cf->m_frootc_to_SNSC    + cf->m_frootc_storage_to_SNSC    + cf->m_frootc_transfer_to_SNSC +  
						   cf->m_fruitc_to_SNSC    + cf->m_fruitc_storage_to_SNSC    + cf->m_fruitc_transfer_to_SNSC +
						   cf->m_softstemc_to_SNSC + cf->m_softstemc_storage_to_SNSC + cf->m_softstemc_transfer_to_SNSC + 
						   cf->m_gresp_storage_to_SNSC  + cf->m_gresp_transfer_to_SNSC;
	cs->SNSCsnk         += cf->m_vegc_to_SNSC; 

	nf->m_vegn_to_SNSC += nf->m_leafn_to_SNSC     + nf->m_leafn_storage_to_SNSC     + nf->m_leafn_transfer_to_SNSC     + nf->m_leafn_to_SNSCgenprog + 
		                  nf->m_frootn_to_SNSC    + nf->m_frootn_storage_to_SNSC    + nf->m_frootn_transfer_to_SNSC    +
						  nf->m_fruitn_to_SNSC    + nf->m_fruitn_storage_to_SNSC    + nf->m_fruitn_transfer_to_SNSC    +  
						  nf->m_softstemn_to_SNSC + nf->m_softstemn_storage_to_SNSC + nf->m_softstemn_transfer_to_SNSC +
					      nf->m_retransn_to_SNSC;

	ns->SNSCsnk         += nf->m_vegn_to_SNSC;

    /****************************************************************************************/
	/* 7. update litter variables - increasing litter storage state variables 
	MULTILAYER SOIL: separate above- and belowground litr1 */
	
	cs->STDBc_leaf     += cf->m_leafc_to_SNSC + cf->m_leafc_to_SNSCgenprog; 											
	cs->STDBc_froot    += cf->m_frootc_to_SNSC;
	cs->STDBc_fruit    += cf->m_fruitc_to_SNSC; 
	cs->STDBc_softstem += cf->m_softstemc_to_SNSC; 
	cs->STDBc_transfer += cf->m_leafc_storage_to_SNSC  + cf->m_leafc_transfer_to_SNSC  + cf->m_frootc_storage_to_SNSC    + cf->m_frootc_transfer_to_SNSC +
		                  cf->m_fruitc_storage_to_SNSC + cf->m_fruitc_transfer_to_SNSC + cf->m_softstemc_storage_to_SNSC + cf->m_softstemc_transfer_to_SNSC +
				          cf->m_gresp_storage_to_SNSC + cf->m_gresp_transfer_to_SNSC;
	
	ns->STDBn_leaf     += nf->m_leafn_to_SNSC + nf->m_leafn_to_SNSCgenprog; 											
	ns->STDBn_froot    += nf->m_frootn_to_SNSC;
	ns->STDBn_fruit    += nf->m_fruitn_to_SNSC; 
	ns->STDBn_softstem += nf->m_softstemn_to_SNSC; 
	ns->STDBn_transfer += nf->m_leafn_storage_to_SNSC  + nf->m_leafn_transfer_to_SNSC  + nf->m_frootn_storage_to_SNSC    + nf->m_frootn_transfer_to_SNSC +
		                  nf->m_fruitn_storage_to_SNSC + nf->m_fruitn_transfer_to_SNSC + nf->m_softstemn_storage_to_SNSC + nf->m_softstemn_transfer_to_SNSC +
				          nf->m_retransn_to_SNSC;

	 /****************************************************************************************/
	/* 8. mortality fluxes into litter pools */


	cf->STDBc_leaf_to_litr     = cs->STDBc_leaf     * mort_SNSC_to_litter;
	cf->STDBc_froot_to_litr    = cs->STDBc_froot    * mort_SNSC_to_litter;                                                         
	cf->STDBc_fruit_to_litr    = cs->STDBc_fruit    * mort_SNSC_to_litter;
	cf->STDBc_softstem_to_litr = cs->STDBc_softstem * mort_SNSC_to_litter;
	cf->STDBc_transfer_to_litr = cs->STDBc_transfer * mort_SNSC_to_litter;

	nf->STDBn_leaf_to_litr     = ns->STDBn_leaf     * mort_SNSC_to_litter;
	nf->STDBn_froot_to_litr    = ns->STDBn_froot    * mort_SNSC_to_litter;                                                         
	nf->STDBn_fruit_to_litr    = ns->STDBn_fruit    * mort_SNSC_to_litter;
	nf->STDBn_softstem_to_litr = ns->STDBn_softstem * mort_SNSC_to_litter;
	nf->STDBn_transfer_to_litr = ns->STDBn_transfer * mort_SNSC_to_litter;

	cf->STDBc_to_litr = cf->STDBc_leaf_to_litr + cf->STDBc_froot_to_litr + cf->STDBc_fruit_to_litr + cf->STDBc_softstem_to_litr + cf->STDBc_transfer_to_litr;

	nf->STDBn_to_litr = nf->STDBn_leaf_to_litr + nf->STDBn_froot_to_litr + nf->STDBn_fruit_to_litr + nf->STDBn_softstem_to_litr + nf->STDBn_transfer_to_litr;

	/****************************************************************************************/
	/* 9. mortality fluxes turn into litter pools: 	aboveground biomass into the top soil layer, belowground biomass divided between soil layers based on their root content */
	
	/* 9.1 aboveground biomass into the top soil layer */
	cs->litr1c[0] += cf->STDBc_leaf_to_litr * epc->leaflitr_flab  + cf->STDBc_fruit_to_litr * epc->fruitlitr_flab  + cf->STDBc_softstem_to_litr * epc->softstemlitr_flab + cf->STDBc_transfer_to_litr;
	cs->litr2c[0] += cf->STDBc_leaf_to_litr * epc->leaflitr_fucel + cf->STDBc_fruit_to_litr * epc->fruitlitr_fucel + cf->STDBc_softstem_to_litr * epc->softstemlitr_fucel;
	cs->litr3c[0] += cf->STDBc_leaf_to_litr * epc->leaflitr_fscel + cf->STDBc_fruit_to_litr * epc->fruitlitr_fscel + cf->STDBc_softstem_to_litr * epc->softstemlitr_fscel;
	cs->litr4c[0] += cf->STDBc_leaf_to_litr * epc->leaflitr_flig  + cf->STDBc_fruit_to_litr * epc->fruitlitr_flig  + cf->STDBc_softstem_to_litr * epc->softstemlitr_flig;

	ns->litr1n[0] += nf->STDBn_leaf_to_litr * epc->leaflitr_flab  + nf->STDBn_fruit_to_litr * epc->fruitlitr_flab  + nf->STDBn_softstem_to_litr * epc->softstemlitr_flab + nf->STDBn_transfer_to_litr;
	ns->litr2n[0] += nf->STDBn_leaf_to_litr * epc->leaflitr_fucel + nf->STDBn_fruit_to_litr * epc->fruitlitr_fucel + nf->STDBn_softstem_to_litr * epc->softstemlitr_fucel;
	ns->litr3n[0] += nf->STDBn_leaf_to_litr * epc->leaflitr_fscel + nf->STDBn_fruit_to_litr * epc->fruitlitr_fscel + nf->STDBn_softstem_to_litr * epc->softstemlitr_fscel;
	ns->litr4n[0] += nf->STDBn_leaf_to_litr * epc->leaflitr_flig  + nf->STDBn_fruit_to_litr * epc->fruitlitr_flig  + nf->STDBn_softstem_to_litr * epc->softstemlitr_flig;

	/* 9.2 	belowground biomass divided between soil layers based on their root content */ 
	for (layer=0; layer < N_SOILLAYERS; layer++)
	{
		cs->litr1c[layer]  += cf->STDBc_froot_to_litr * epc->frootlitr_flab  * epv->rootlength_prop[layer];
		cs->litr2c[layer]  += cf->STDBc_froot_to_litr * epc->frootlitr_fucel * epv->rootlength_prop[layer];
		cs->litr3c[layer]  += cf->STDBc_froot_to_litr * epc->frootlitr_fscel * epv->rootlength_prop[layer];
		cs->litr4c[layer]  += cf->STDBc_froot_to_litr * epc->frootlitr_flig * epv->rootlength_prop[layer];

		ns->litr1n[layer]  += nf->STDBn_froot_to_litr * epc->frootlitr_flab  * epv->rootlength_prop[layer];
		ns->litr2n[layer]  += nf->STDBn_froot_to_litr * epc->frootlitr_fucel * epv->rootlength_prop[layer];
		ns->litr3n[layer]  += nf->STDBn_froot_to_litr * epc->frootlitr_fscel * epv->rootlength_prop[layer];
		ns->litr4n[layer]  += nf->STDBn_froot_to_litr * epc->frootlitr_flig  * epv->rootlength_prop[layer];
	}
	

	/****************************************************************************************/
	/* 10. decreasing of temporary pool */
	
	cs->STDBc_leaf     -= cs->STDBc_leaf     * mort_SNSC_to_litter;
	cs->STDBc_froot    -= cs->STDBc_froot    * mort_SNSC_to_litter;
	cs->STDBc_fruit    -= cs->STDBc_fruit    * mort_SNSC_to_litter;
	cs->STDBc_softstem -= cs->STDBc_softstem * mort_SNSC_to_litter;
	cs->STDBc_transfer -= cs->STDBc_transfer * mort_SNSC_to_litter;

	ns->STDBn_leaf     -= ns->STDBn_leaf     * mort_SNSC_to_litter;
	ns->STDBn_froot    -= ns->STDBn_froot    * mort_SNSC_to_litter;
	ns->STDBn_fruit    -= ns->STDBn_fruit    * mort_SNSC_to_litter;
	ns->STDBn_softstem -= ns->STDBn_softstem * mort_SNSC_to_litter;
	ns->STDBn_transfer -= ns->STDBn_transfer * mort_SNSC_to_litter;


	/************************************************************/
	/* 11. precision control */

   if ((cs->STDBc_leaf != 0 && fabs(cs->STDBc_leaf) < CRIT_PREC) || (ns->STDBn_leaf != 0 && fabs(ns->STDBn_leaf) < CRIT_PREC))
	{
		cs->fire_snk += cs->STDBc_leaf;
		ns->fire_snk += ns->STDBn_leaf; 
		cs->STDBc_leaf = 0;
		ns->STDBn_leaf = 0;
	}

	if ((cs->STDBc_fruit != 0 && fabs(cs->STDBc_fruit) < CRIT_PREC) || (ns->STDBn_fruit != 0 && fabs(ns->STDBn_fruit) < CRIT_PREC))
	{
		cs->fire_snk += cs->STDBc_fruit;
		ns->fire_snk += ns->STDBn_fruit; 
		cs->STDBc_fruit = 0;
		ns->STDBn_fruit = 0;
	}

	if ((cs->STDBc_softstem != 0 && fabs(cs->STDBc_softstem) < CRIT_PREC) || (ns->STDBn_softstem != 0 && fabs(ns->STDBn_softstem) < CRIT_PREC))
	{
		cs->fire_snk += cs->STDBc_softstem;
		ns->fire_snk += ns->STDBn_softstem; 
		cs->STDBc_softstem = 0;
		ns->STDBn_softstem = 0;
	}

	if ((cs->STDBc_froot != 0 && fabs(cs->STDBc_froot) < CRIT_PREC) || (ns->STDBn_froot != 0 && fabs(ns->STDBn_froot) < CRIT_PREC))
	{
		cs->fire_snk += cs->STDBc_froot;
		ns->fire_snk += ns->STDBn_froot; 
		cs->STDBc_froot = 0;
		ns->STDBn_froot = 0;
	}

	if ((cs->STDBc_transfer != 0 && fabs(cs->STDBc_transfer) < CRIT_PREC) || (ns->STDBn_transfer != 0 && fabs(ns->STDBn_transfer) < CRIT_PREC))
	{
		cs->fire_snk += cs->STDBc_transfer;
		ns->fire_snk += ns->STDBn_transfer; 
		cs->STDBc_transfer = 0;
		ns->STDBn_transfer = 0;
	}
	
	
	return (!ok);
}

int genprog_senescence(const epconst_struct* epc, const metvar_struct* metv, epvar_struct* epv, cflux_struct* cf, nflux_struct* nf)
{

	int ok, vd, leafday, leafday_lastmort;
	double thermal_timeSUM_base;

	leafday = (int) epv->leafday;
	leafday_lastmort = (int) epv->leafday_lastmort;
	ok=1;

	/* phenological phases: 1: seedling->germination, 2: germination->emergence, 3: leaf increasing */
	if (epv->n_actphen >= epc->n_emerg_phenophase)		
	{
		leafday += 1;
	
		if (metv->tavg - epc->base_temp > 0)
			epv->thermal_time = (metv->tavg - epc->base_temp);
		else
			epv->thermal_time = 0;

		if (leafday == 0)
			epv->thermal_timeSUM[leafday] = epv->thermal_time;
		else
			epv->thermal_timeSUM[leafday] = epv->thermal_timeSUM[leafday-1] + epv->thermal_time;
		
		

		if (leafday_lastmort == 0)
			thermal_timeSUM_base = 0;
		else
			thermal_timeSUM_base = epv->thermal_timeSUM[leafday_lastmort-1];
	

		epv->cpool_to_leafcARRAY[leafday] = cf->cpool_to_leafc;
		epv->npool_to_leafnARRAY[leafday] = nf->npool_to_leafn;

		if (epv->thermal_timeSUM[leafday] - thermal_timeSUM_base > epc->mort_MAX_lifetime)
		{
			cf->m_leafc_to_SNSCgenprog = epv->cpool_to_leafcARRAY[leafday_lastmort];
			nf->m_leafn_to_SNSCgenprog = epv->npool_to_leafnARRAY[leafday_lastmort];
			leafday_lastmort += 1;
		}



		if (leafday >= NDAYS_OF_YEAR)
		{
			printf("\n");
			printf("Error in leafday calculation() in senescence.c()\n");
			ok=0;
			
		}
			


	}	
	else
	{
		leafday = -1;
		leafday_lastmort = 0;
		epv->thermal_timeSUM_max = 0;
		epv->thermal_time = 0;

		for (vd=0 ; vd < NDAYS_OF_YEAR; vd++)
		{

			epv->thermal_timeSUM[vd] = 0;
			epv->cpool_to_leafcARRAY[vd] = 0;
			epv->npool_to_leafnARRAY[vd] = 0;
		}
	}

	epv->leafday = (double) leafday;
	epv->leafday_lastmort = (double) leafday_lastmort;
	if (leafday >=0 )
		epv->thermal_timeSUM_max = epv->thermal_timeSUM[leafday];
	else
		epv->thermal_timeSUM_max = 0;

	return (!ok);
}


