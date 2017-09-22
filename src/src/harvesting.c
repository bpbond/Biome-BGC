/* 
harvesting.c
do harvesting  - decrease the plant material (leafc, leafn, canopy water)

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

int harvesting(const control_struct* ctrl, const epconst_struct* epc, harvesting_struct* HRV, 
			cflux_struct* cf, nflux_struct* nf, wflux_struct* wf,  cstate_struct* cs, nstate_struct* ns, wstate_struct* ws)
{
	/* harvesting parameters */
	double remained_prop;					/* remained proportion of plant  */
	double snag;
	double befharv_stem;						/* value of LAI before harvesting */
	double HRVcoeff_leaf, HRVcoeff_fruit, HRVcoeff_softstem;	/* decrease of plant material caused by harvest: difference between plant material before and after harvesting */
	double HRV_to_CTDB_litr1c, HRV_to_CTDB_litr2c, HRV_to_CTDB_litr3c, HRV_to_CTDB_litr4c;
	double HRV_to_CTDB_litr1n, HRV_to_CTDB_litr2n, HRV_to_CTDB_litr3n, HRV_to_CTDB_litr4n;
	double STDB_litr1c_to_HRV, STDB_litr2c_to_HRV, STDB_litr3c_to_HRV, STDB_litr4c_to_HRV;
	double STDB_litr1n_to_HRV, STDB_litr2n_to_HRV, STDB_litr3n_to_HRV, STDB_litr4n_to_HRV;

	double HRV_to_transpC, HRV_to_transpN;
	double diffC, diffN;

	/* local parameters */
	int ok = 1;
	int ny;
	int mgmd = HRV->mgmd;
	double HRV_to_litter_coeff = epc->mort_CnW_to_litter;

	/* test variable */
	double storage_MGMmort=epc->storage_MGMmort;


	/* yearly varied or constant management parameters */
	if(HRV->HRV_flag == 2)
	{
		ny = ctrl->simyr;
	}
	else ny=0;



	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                                    CALCULATING FLUXES 
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

	/* harvesting if gapflag=1  */
	if (mgmd >= 0)
	{ 
		remained_prop = (100 - HRV->transport_coeff_array[mgmd][ny])/100.; /* remained prop. of plant mat.is calculated from transport coeff. */
		snag = HRV->snag_array[mgmd][ny];
		
	
		/* if before harvesting the value of the LAI is less than snag (limit value) - > no harvesting
	   if harvest: plant material decreases as the rate of "harvest effect", which is th ratio of LAI before harvest and LAI snag */
			
		befharv_stem = cs->softstemc;
		if (befharv_stem > snag && befharv_stem > 0)
		{	
			HRVcoeff_leaf  = 1.0;
			HRVcoeff_fruit = 1.0;
			HRVcoeff_softstem = 1. - (snag/befharv_stem);
		
		}
		else
		{
			HRVcoeff_leaf = 1.0;
			HRVcoeff_fruit = 1.0;
			HRVcoeff_softstem = 1.0;
			if (ctrl->onscreen) printf("sofstem carbon content is less than snag set in INI file\n");
		}
	}
	else
	{
		HRVcoeff_leaf  = 0.0;
		HRVcoeff_fruit = 0.0;
		HRVcoeff_softstem = 0.0;
		remained_prop = 0.0;
	}

	/**********************************************************************************************/
	/* 1. as results of the harvesting the carbon, nitrogen and water content of the leaf decreases*/
	/* fruit simulation - Hidy 2013.: harvested fruit is transported from the site  */
	
	cf->leafc_to_HRV          = cs->leafc * HRVcoeff_leaf;
	cf->leafc_storage_to_HRV  = cs->leafc_storage * HRVcoeff_leaf * storage_MGMmort;
	cf->leafc_transfer_to_HRV = cs->leafc_transfer * HRVcoeff_leaf * storage_MGMmort;

	cf->fruitc_to_HRV          = cs->fruitc * HRVcoeff_fruit;
	cf->fruitc_storage_to_HRV  = cs->fruitc_storage * HRVcoeff_fruit * storage_MGMmort;
	cf->fruitc_transfer_to_HRV = cs->fruitc_transfer * HRVcoeff_fruit * storage_MGMmort;

	cf->softstemc_to_HRV          = cs->softstemc * HRVcoeff_softstem;
	cf->softstemc_storage_to_HRV  = cs->softstemc_storage * HRVcoeff_softstem * storage_MGMmort;
	cf->softstemc_transfer_to_HRV = cs->softstemc_transfer * HRVcoeff_softstem * storage_MGMmort;

	cf->gresp_storage_to_HRV  = cs->gresp_storage * HRVcoeff_leaf * storage_MGMmort;
	cf->gresp_transfer_to_HRV = cs->gresp_transfer * HRVcoeff_leaf * storage_MGMmort;

       /* standing dead biome */
	STDB_litr1c_to_HRV = cs->STDB_litr1c * HRVcoeff_leaf;
	STDB_litr2c_to_HRV = cs->STDB_litr2c * HRVcoeff_leaf;
	STDB_litr3c_to_HRV = cs->STDB_litr3c * HRVcoeff_leaf;
	STDB_litr4c_to_HRV = cs->STDB_litr4c * HRVcoeff_leaf;

	cf->STDBc_to_HRV = STDB_litr1c_to_HRV + STDB_litr2c_to_HRV + STDB_litr3c_to_HRV + STDB_litr4c_to_HRV;


	nf->leafn_to_HRV          = ns->leafn * HRVcoeff_leaf;
	nf->leafn_storage_to_HRV  = ns->leafn_storage * HRVcoeff_leaf * storage_MGMmort;
	nf->leafn_transfer_to_HRV = ns->leafn_transfer * HRVcoeff_leaf * storage_MGMmort;

	nf->fruitn_to_HRV          = ns->fruitn * HRVcoeff_fruit;
	nf->fruitn_storage_to_HRV  = ns->fruitn_storage * HRVcoeff_fruit * storage_MGMmort;
	nf->fruitn_transfer_to_HRV = ns->fruitn_transfer * HRVcoeff_fruit * storage_MGMmort;

	nf->softstemn_to_HRV          = ns->softstemn * HRVcoeff_softstem;
	nf->softstemn_storage_to_HRV  = ns->softstemn_storage * HRVcoeff_softstem * storage_MGMmort;
	nf->softstemn_transfer_to_HRV = ns->softstemn_transfer * HRVcoeff_softstem * storage_MGMmort;

	/* standing dead biome */
	STDB_litr1n_to_HRV = ns->STDB_litr1n * HRVcoeff_leaf;
	STDB_litr2n_to_HRV = ns->STDB_litr2n * HRVcoeff_leaf;
	STDB_litr3n_to_HRV = ns->STDB_litr3n * HRVcoeff_leaf;
	STDB_litr4n_to_HRV = ns->STDB_litr4n * HRVcoeff_leaf;

	nf->STDBn_to_HRV = STDB_litr1n_to_HRV + STDB_litr2n_to_HRV + STDB_litr3n_to_HRV + STDB_litr4n_to_HRV;
   
	nf->retransn_to_HRV        = ns->retransn * HRVcoeff_leaf * storage_MGMmort;

	wf->canopyw_to_HRV = ws->canopyw * HRVcoeff_leaf;

    /**********************************************************************************************/
	/* 2. part of the plant material is transported (HRV_to_transpC and HRV_to_transpN; transp_coeff = 1-remained_prop),
	      the rest remains at the site as cut-down plant material (HRV_to_CTDB_litrc, HRV_to_CTDB_litrn)*/

	HRV_to_transpC = (cf->leafc_to_HRV + cf->softstemc_to_HRV + cf->STDBc_to_HRV) * (1-remained_prop) +
				      cf->fruitc_to_HRV;

	HRV_to_transpN = (nf->leafn_to_HRV + nf->softstemn_to_HRV + nf->STDBn_to_HRV) * (1-remained_prop) +
				      nf->fruitn_to_HRV;
	
	HRV_to_CTDB_litr1c = (cf->leafc_to_HRV * epc->leaflitr_flab + cf->softstemc_to_HRV * epc->softstemlitr_flab + STDB_litr1c_to_HRV) * remained_prop +
						 (cf->leafc_transfer_to_HRV + cf->leafc_storage_to_HRV + cf->softstemc_transfer_to_HRV + cf->softstemc_storage_to_HRV + 
						  cf->gresp_storage_to_HRV  + cf->gresp_transfer_to_HRV);

	HRV_to_CTDB_litr2c = (cf->leafc_to_HRV * epc->leaflitr_fucel + cf->softstemc_to_HRV * epc->softstemlitr_fucel + 
		                  STDB_litr2c_to_HRV) * remained_prop;

	HRV_to_CTDB_litr3c = (cf->leafc_to_HRV * epc->leaflitr_fscel + cf->softstemc_to_HRV * epc->softstemlitr_fscel +
		                  STDB_litr3c_to_HRV) * remained_prop;

	HRV_to_CTDB_litr4c = (cf->leafc_to_HRV * epc->leaflitr_flig  + cf->softstemc_to_HRV * epc->softstemlitr_flig +
		                  STDB_litr4c_to_HRV) * remained_prop;


	HRV_to_CTDB_litr1n = (nf->leafn_to_HRV*epc->leaflitr_flab + nf->softstemn_to_HRV*epc->softstemlitr_flab  + STDB_litr1n_to_HRV) * remained_prop +
						 (nf->leafn_transfer_to_HRV + cf->leafc_storage_to_HRV + nf->softstemn_transfer_to_HRV + cf->softstemc_storage_to_HRV +
						  nf->retransn_to_HRV) * remained_prop;

	HRV_to_CTDB_litr2n =  (nf->leafn_to_HRV * epc->leaflitr_fucel + nf->softstemn_to_HRV * epc->softstemlitr_fucel +
						   STDB_litr2n_to_HRV) * remained_prop;
	
	HRV_to_CTDB_litr3n =  (nf->leafn_to_HRV * epc->leaflitr_fscel + nf->softstemn_to_HRV * epc->softstemlitr_fscel +
		                   STDB_litr3n_to_HRV) * remained_prop;

	HRV_to_CTDB_litr4n =  (nf->leafn_to_HRV * epc->leaflitr_flig  + nf->softstemn_to_HRV * epc->softstemlitr_flig +
		                   STDB_litr4n_to_HRV) * remained_prop;
   

	cs->CTDB_litr1c += HRV_to_CTDB_litr1c;
	cs->CTDB_litr2c += HRV_to_CTDB_litr2c;
	cs->CTDB_litr3c += HRV_to_CTDB_litr3c;
	cs->CTDB_litr4c += HRV_to_CTDB_litr4c;

	cs->HRV_transportC  += HRV_to_transpC;


	ns->CTDB_litr1n += HRV_to_CTDB_litr1n;
	ns->CTDB_litr2n += HRV_to_CTDB_litr2n;
	ns->CTDB_litr3n += HRV_to_CTDB_litr3n;
	ns->CTDB_litr4n += HRV_to_CTDB_litr4n;

	ns->HRV_transportN  += HRV_to_transpN;

	/**********************************************************************************************/
	/* 3.the remained part of the plant material gradually goes into the litter pool (CTDB_litrc_HRV, CTDB_litrn_HRV) */
 
	cf->HRV_to_litr1c = cs->CTDB_litr1c * HRV_to_litter_coeff;
	cf->HRV_to_litr2c = cs->CTDB_litr2c * HRV_to_litter_coeff;
	cf->HRV_to_litr3c = cs->CTDB_litr3c * HRV_to_litter_coeff;
	cf->HRV_to_litr4c = cs->CTDB_litr4c * HRV_to_litter_coeff;

	nf->HRV_to_litr1n = ns->CTDB_litr1n * HRV_to_litter_coeff;
	nf->HRV_to_litr2n = ns->CTDB_litr2n * HRV_to_litter_coeff;
	nf->HRV_to_litr3n = ns->CTDB_litr3n * HRV_to_litter_coeff;
	nf->HRV_to_litr4n = ns->CTDB_litr4n * HRV_to_litter_coeff;
	

	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                                    STATE UPDATE 
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/ 

	/* 1. leaf, softstem, fruit and gresp */
	cs->HRVsnk           += cf->leafc_to_HRV;
	cs->leafc            -= cf->leafc_to_HRV;
	cs->HRVsnk           += cf->leafc_transfer_to_HRV;
	cs->leafc_transfer   -= cf->leafc_transfer_to_HRV;
	cs->HRVsnk           += cf->leafc_storage_to_HRV;
	cs->leafc_storage    -= cf->leafc_storage_to_HRV;
	cs->HRVsnk           += cf->gresp_transfer_to_HRV;
	cs->gresp_transfer   -= cf->gresp_transfer_to_HRV;
	cs->HRVsnk           += cf->gresp_storage_to_HRV;
	cs->gresp_storage    -= cf->gresp_storage_to_HRV;
	
	cs->HRVsnk             += cf->fruitc_to_HRV;
	cs->fruitc             -= cf->fruitc_to_HRV;
	cs->HRVsnk             += cf->fruitc_transfer_to_HRV;
	cs->fruitc_transfer    -= cf->fruitc_transfer_to_HRV;
	cs->HRVsnk             += cf->fruitc_storage_to_HRV;
	cs->fruitc_storage     -= cf->fruitc_storage_to_HRV;
	
	cs->HRVsnk             += cf->softstemc_to_HRV;
	cs->softstemc          -= cf->softstemc_to_HRV;
	cs->HRVsnk             += cf->softstemc_transfer_to_HRV;
	cs->softstemc_transfer -= cf->softstemc_transfer_to_HRV;
	cs->HRVsnk             += cf->softstemc_storage_to_HRV;
	cs->softstemc_storage  -= cf->softstemc_storage_to_HRV;

	/* standing dead biomass */
	cs->HRVsnk           += STDB_litr1c_to_HRV;
	cs->STDB_litr1c      -= STDB_litr1c_to_HRV;
	cs->HRVsnk           += STDB_litr2c_to_HRV;
	cs->STDB_litr2c      -= STDB_litr2c_to_HRV;
	cs->HRVsnk           += STDB_litr3c_to_HRV;
	cs->STDB_litr3c      -= STDB_litr3c_to_HRV;
	cs->HRVsnk           += STDB_litr4c_to_HRV;
	cs->STDB_litr4c      -=STDB_litr4c_to_HRV;

	cs->SNSCsrc          += cf->STDBc_to_HRV;
	cs->STDBc            -= cf->STDBc_to_HRV;


	/* litter: incresing litter pools and decreasing litter storage state variables*/
	cs->litr1c             += cf->HRV_to_litr1c;
	cs->litr2c             += cf->HRV_to_litr2c;
	cs->litr3c             += cf->HRV_to_litr3c;
	cs->litr4c             += cf->HRV_to_litr4c;
	
	cs->CTDB_litr1c    -= cf->HRV_to_litr1c;
	cs->CTDB_litr2c    -= cf->HRV_to_litr2c;
	cs->CTDB_litr3c    -= cf->HRV_to_litr3c;
	cs->CTDB_litr4c    -= cf->HRV_to_litr4c;

	/* litter plus because of HRVing and returning of dead plant material */
	cs->HRVsrc += cf->HRV_to_litr1c + cf->HRV_to_litr2c + cf->HRV_to_litr3c + cf->HRV_to_litr4c;


	/* 2. nitrogen */
	ns->HRVsnk += nf->leafn_to_HRV;
	ns->leafn -= nf->leafn_to_HRV;
	ns->HRVsnk += nf->leafn_transfer_to_HRV;
	ns->leafn_transfer -= nf->leafn_transfer_to_HRV;
	ns->HRVsnk += nf->leafn_storage_to_HRV;
	ns->leafn_storage -= nf->leafn_storage_to_HRV;
       
     /* dead standing biomass */
	ns->HRVsnk += STDB_litr1n_to_HRV;
	ns->STDB_litr1n -= STDB_litr1n_to_HRV;
	ns->HRVsnk += STDB_litr2n_to_HRV;
	ns->STDB_litr2n -= STDB_litr2n_to_HRV;
	ns->HRVsnk += STDB_litr3n_to_HRV;
	ns->STDB_litr3n -= STDB_litr3n_to_HRV;
	ns->HRVsnk += STDB_litr4n_to_HRV;
	ns->STDB_litr4n -=STDB_litr4n_to_HRV;

	ns->SNSCsrc += nf->STDBn_to_HRV;
	ns->STDBn -= nf->STDBn_to_HRV;


	/* fruit simulation - Hidy 2013. */
	ns->HRVsnk += nf->fruitn_to_HRV;
	ns->fruitn -= nf->fruitn_to_HRV;
	ns->HRVsnk += nf->fruitn_transfer_to_HRV;
	ns->fruitn_transfer -= nf->fruitn_transfer_to_HRV;
	ns->HRVsnk += nf->fruitn_storage_to_HRV;
	ns->fruitn_storage -= nf->fruitn_storage_to_HRV;
	ns->HRVsnk += nf->retransn_to_HRV;
	ns->retransn -= nf->retransn_to_HRV;
	/* softstem simulation - Hidy 2013. */
	ns->HRVsnk += nf->softstemn_to_HRV;
	ns->softstemn -= nf->softstemn_to_HRV;
	ns->HRVsnk += nf->softstemn_transfer_to_HRV;
	ns->softstemn_transfer -= nf->softstemn_transfer_to_HRV;
	ns->HRVsnk += nf->softstemn_storage_to_HRV;
	ns->softstemn_storage -= nf->softstemn_storage_to_HRV;
	ns->HRVsnk += nf->retransn_to_HRV;
	ns->retransn -= nf->retransn_to_HRV;

	ns->litr1n += nf->HRV_to_litr1n;
	ns->litr2n += nf->HRV_to_litr2n;
	ns->litr3n += nf->HRV_to_litr3n;
	ns->litr4n += nf->HRV_to_litr4n;

	/* decreasing litter storage state variables*/
	ns->CTDB_litr1n -= nf->HRV_to_litr1n;
	ns->CTDB_litr2n -= nf->HRV_to_litr2n;
	ns->CTDB_litr3n -= nf->HRV_to_litr3n;
	ns->CTDB_litr4n -= nf->HRV_to_litr4n;

	ns->HRVsrc += nf->HRV_to_litr1n + nf->HRV_to_litr2n + nf->HRV_to_litr3n + nf->HRV_to_litr4n;

	/* 3. water */
	ws->canopyw_HRVsnk += wf->canopyw_to_HRV;
	ws->canopyw -= wf->canopyw_to_HRV;

		/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                                    TEMPORARY POOLS
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
	/* temporary HRVed plant material pools: if CTDB_litr_HRV is less than a crit. value, the temporary pool becomes empty */

	if (cs->CTDB_litr1c < CRIT_PREC && cs->CTDB_litr1c != 0) 
	{
		cs->HRVsrc += cs->CTDB_litr1c + cs->CTDB_litr2c + cs->CTDB_litr3c + cs->CTDB_litr4c;
		cs->CTDB_litr1c = 0;
		cs->CTDB_litr2c = 0;
		cs->CTDB_litr3c = 0;
		cs->CTDB_litr4c = 0;
		ns->HRVsrc += ns->CTDB_litr1n + ns->CTDB_litr2n + ns->CTDB_litr3n + ns->CTDB_litr4n;
		ns->CTDB_litr1n = 0;
		ns->CTDB_litr2n = 0;
		ns->CTDB_litr3n = 0;
		ns->CTDB_litr4n = 0;
	}
		
	/* CONTROL */
	diffC = (cs->HRVsnk-cs->HRVsrc) - cs->HRV_transportC - 
			(cs->CTDB_litr1c+cs->CTDB_litr2c+cs->CTDB_litr3c+cs->CTDB_litr4c);

	diffN = (ns->HRVsnk-ns->HRVsrc) - ns->HRV_transportN - 
		    (ns->CTDB_litr1n+ns->CTDB_litr1n+ns->CTDB_litr1n+ns->CTDB_litr1n) ;

	if (mgmd >= 0 && (fabs(diffC) > 1e-3 || fabs(diffN) > 1e-3 || cs->litr_belowground < 0))
	{
	//	printf("Warning: small rounding error in harvesting pools (harvesting.c)\n");
	}
	

   return (!ok);
}
	