/* 
harvesting.c
do harvesting  - decrease the plant material (leafc, leafn, canopy water)

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.1
Copyright 2016, D. Hidy [dori.hidy@gmail.com]
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
	double HRV_to_litr1c_strg, HRV_to_litr2c_strg, HRV_to_litr3c_strg, HRV_to_litr4c_strg, HRV_to_transpC;
	double HRV_to_litr1n_strg, HRV_to_litr2n_strg, HRV_to_litr3n_strg, HRV_to_litr4n_strg, HRV_to_transpN;
	double diffC, diffN;
	double litr1c_STDB_to_HRV, litr2c_STDB_to_HRV, litr3c_STDB_to_HRV, litr4c_STDB_to_HRV;
	double litr1n_STDB_to_HRV, litr2n_STDB_to_HRV, litr3n_STDB_to_HRV, litr4n_STDB_to_HRV;

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
	litr1c_STDB_to_HRV = cs->litr1c_STDB * HRVcoeff_leaf;
	litr2c_STDB_to_HRV = cs->litr2c_STDB * HRVcoeff_leaf;
	litr3c_STDB_to_HRV = cs->litr3c_STDB * HRVcoeff_leaf;
	litr4c_STDB_to_HRV = cs->litr4c_STDB * HRVcoeff_leaf;

	cf->STDBc_to_HRV = litr1c_STDB_to_HRV + litr2c_STDB_to_HRV + litr3c_STDB_to_HRV + litr4c_STDB_to_HRV;


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
	litr1n_STDB_to_HRV = ns->litr1n_STDB * HRVcoeff_leaf;
	litr2n_STDB_to_HRV = ns->litr2n_STDB * HRVcoeff_leaf;
	litr3n_STDB_to_HRV = ns->litr3n_STDB * HRVcoeff_leaf;
	litr4n_STDB_to_HRV = ns->litr4n_STDB * HRVcoeff_leaf;

	nf->STDBn_to_HRV = litr1n_STDB_to_HRV + litr2n_STDB_to_HRV + litr3n_STDB_to_HRV + litr4n_STDB_to_HRV;
   
	nf->retransn_to_HRV        = ns->retransn * HRVcoeff_leaf * storage_MGMmort;

	wf->canopyw_to_HRV = ws->canopyw * HRVcoeff_leaf;

    /**********************************************************************************************/
	/* 2. part of the plant material is transported (HRV_to_transpC and HRV_to_transpN; transp_coeff = 1-remained_prop),
	      the rest remains at the site (HRV_to_litrc_strg, HRV_to_litrn_strg)*/

	HRV_to_transpC = (cf->leafc_to_HRV + cf->softstemc_to_HRV + cf->STDBc_to_HRV) * (1-remained_prop) +
				      cf->fruitc_to_HRV;

	HRV_to_transpN = (nf->leafn_to_HRV + nf->softstemn_to_HRV + nf->STDBn_to_HRV) * (1-remained_prop) +
				      nf->fruitn_to_HRV;
	
	HRV_to_litr1c_strg = (cf->leafc_to_HRV * epc->leaflitr_flab + cf->softstemc_to_HRV * epc->softstemlitr_flab + litr1c_STDB_to_HRV) * remained_prop +
						 (cf->leafc_transfer_to_HRV + cf->leafc_storage_to_HRV + cf->softstemc_transfer_to_HRV + cf->softstemc_storage_to_HRV + 
						  cf->gresp_storage_to_HRV  + cf->gresp_transfer_to_HRV);

	HRV_to_litr2c_strg = (cf->leafc_to_HRV * epc->leaflitr_fucel + cf->softstemc_to_HRV * epc->softstemlitr_fucel + 
		                  litr2c_STDB_to_HRV) * remained_prop;

	HRV_to_litr3c_strg = (cf->leafc_to_HRV * epc->leaflitr_fscel + cf->softstemc_to_HRV * epc->softstemlitr_fscel +
		                  litr3c_STDB_to_HRV) * remained_prop;

	HRV_to_litr4c_strg = (cf->leafc_to_HRV * epc->leaflitr_flig  + cf->softstemc_to_HRV * epc->softstemlitr_flig +
		                  litr4c_STDB_to_HRV) * remained_prop;


	HRV_to_litr1n_strg = (nf->leafn_to_HRV*epc->leaflitr_flab + nf->softstemn_to_HRV*epc->softstemlitr_flab  + litr1n_STDB_to_HRV) * remained_prop +
						 (nf->leafn_transfer_to_HRV + cf->leafc_storage_to_HRV + nf->softstemn_transfer_to_HRV + cf->softstemc_storage_to_HRV +
						  nf->retransn_to_HRV) * remained_prop;

	HRV_to_litr2n_strg =  (nf->leafn_to_HRV * epc->leaflitr_fucel + nf->softstemn_to_HRV * epc->softstemlitr_fucel +
						   litr2n_STDB_to_HRV) * remained_prop;
	
	HRV_to_litr3n_strg =  (nf->leafn_to_HRV * epc->leaflitr_fscel + nf->softstemn_to_HRV * epc->softstemlitr_fscel +
		                   litr3n_STDB_to_HRV) * remained_prop;

	HRV_to_litr4n_strg =  (nf->leafn_to_HRV * epc->leaflitr_flig  + nf->softstemn_to_HRV * epc->softstemlitr_flig +
		                   litr4n_STDB_to_HRV) * remained_prop;
   

	cs->litr1c_strg_HRV += HRV_to_litr1c_strg;
	cs->litr2c_strg_HRV += HRV_to_litr2c_strg;
	cs->litr3c_strg_HRV += HRV_to_litr3c_strg;
	cs->litr4c_strg_HRV += HRV_to_litr4c_strg;

	cs->HRV_transportC  += HRV_to_transpC;

	ns->litr1n_strg_HRV +=  HRV_to_litr1n_strg;
	ns->litr2n_strg_HRV +=  HRV_to_litr2n_strg;
	ns->litr3n_strg_HRV +=  HRV_to_litr3n_strg;
	ns->litr4n_strg_HRV +=  HRV_to_litr4n_strg;

	ns->HRV_transportN  += HRV_to_transpN;

	/**********************************************************************************************/
	/* 3.the remained part of the plant material gradually goes into the litter pool (litrc_strg_HRV, litrn_strg_HRV) */
 
	cf->HRV_to_litr1c = cs->litr1c_strg_HRV * HRV_to_litter_coeff;
	cf->HRV_to_litr2c = cs->litr2c_strg_HRV * HRV_to_litter_coeff;
	cf->HRV_to_litr3c = cs->litr3c_strg_HRV * HRV_to_litter_coeff;
	cf->HRV_to_litr4c = cs->litr4c_strg_HRV * HRV_to_litter_coeff;

	nf->HRV_to_litr1n = ns->litr1n_strg_HRV * HRV_to_litter_coeff;
	nf->HRV_to_litr2n = ns->litr2n_strg_HRV * HRV_to_litter_coeff;
	nf->HRV_to_litr3n = ns->litr3n_strg_HRV * HRV_to_litter_coeff;
	nf->HRV_to_litr4n = ns->litr4n_strg_HRV * HRV_to_litter_coeff;
	

	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                                    STATE UPDATE 
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/ 

	/* 1. leaf and gresp */
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

	/* dead standing biomass */
	cs->HRVsnk           += litr1c_STDB_to_HRV;
	cs->litr1c_STDB      -= litr1c_STDB_to_HRV;
	cs->HRVsnk           += litr2c_STDB_to_HRV;
	cs->litr2c_STDB      -= litr2c_STDB_to_HRV;
	cs->HRVsnk           += litr3c_STDB_to_HRV;
	cs->litr3c_STDB      -= litr3c_STDB_to_HRV;
	cs->HRVsnk           += litr4c_STDB_to_HRV;
	cs->litr4c_STDB      -=litr4c_STDB_to_HRV;

	cs->SNSCsrc          += cf->STDBc_to_HRV;
	cs->STDBc            -= cf->STDBc_to_HRV;


	/* fruit simulation - Hidy 2013. */
	cs->HRVsnk             += cf->fruitc_to_HRV;
	cs->fruitc             -= cf->fruitc_to_HRV;
	cs->HRVsnk             += cf->fruitc_transfer_to_HRV;
	cs->fruitc_transfer    -= cf->fruitc_transfer_to_HRV;
	cs->HRVsnk             += cf->fruitc_storage_to_HRV;
	cs->fruitc_storage     -= cf->fruitc_storage_to_HRV;
	/* softstem simulation - Hidy 2013. */
	cs->HRVsnk             += cf->softstemc_to_HRV;
	cs->softstemc          -= cf->softstemc_to_HRV;
	cs->HRVsnk             += cf->softstemc_transfer_to_HRV;
	cs->softstemc_transfer -= cf->softstemc_transfer_to_HRV;
	cs->HRVsnk             += cf->softstemc_storage_to_HRV;
	cs->softstemc_storage  -= cf->softstemc_storage_to_HRV;

	/* litter: incresing litter pools and decreasing litter storage state variables*/
	cs->litr1c             += cf->HRV_to_litr1c;
	cs->litr2c             += cf->HRV_to_litr2c;
	cs->litr3c             += cf->HRV_to_litr3c;
	cs->litr4c             += cf->HRV_to_litr4c;
	
	cs->litr1c_strg_HRV    -= cf->HRV_to_litr1c;
	cs->litr2c_strg_HRV    -= cf->HRV_to_litr2c;
	cs->litr3c_strg_HRV    -= cf->HRV_to_litr3c;
	cs->litr4c_strg_HRV    -= cf->HRV_to_litr4c;

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
	ns->HRVsnk += litr1n_STDB_to_HRV;
	ns->litr1n_STDB -= litr1n_STDB_to_HRV;
	ns->HRVsnk += litr2n_STDB_to_HRV;
	ns->litr2n_STDB -= litr2n_STDB_to_HRV;
	ns->HRVsnk += litr3n_STDB_to_HRV;
	ns->litr3n_STDB -= litr3n_STDB_to_HRV;
	ns->HRVsnk += litr4n_STDB_to_HRV;
	ns->litr4n_STDB -=litr4n_STDB_to_HRV;

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
	ns->litr1n_strg_HRV -= nf->HRV_to_litr1n;
	ns->litr2n_strg_HRV -= nf->HRV_to_litr2n;
	ns->litr3n_strg_HRV -= nf->HRV_to_litr3n;
	ns->litr4n_strg_HRV -= nf->HRV_to_litr4n;

	ns->HRVsrc += nf->HRV_to_litr1n + nf->HRV_to_litr2n + nf->HRV_to_litr3n + nf->HRV_to_litr4n;

	/* 3. water */
	ws->canopyw_HRVsnk += wf->canopyw_to_HRV;
	ws->canopyw -= wf->canopyw_to_HRV;

		/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                                    TEMPORARY POOLS
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
	/* temporary HRVed plant material pools: if litr1c_strg_HRV is less than a crit. value, the temporary pool becomes empty */

	if (cs->litr1c_strg_HRV < CRIT_PREC && cs->litr1c_strg_HRV != 0) 
	{
		cs->HRVsrc += cs->litr1c_strg_HRV + cs->litr2c_strg_HRV + cs->litr3c_strg_HRV + cs->litr4c_strg_HRV;
		cs->litr1c_strg_HRV = 0;
		cs->litr2c_strg_HRV = 0;
		cs->litr3c_strg_HRV = 0;
		cs->litr4c_strg_HRV = 0;
		ns->HRVsrc += ns->litr1n_strg_HRV + ns->litr2n_strg_HRV + ns->litr3n_strg_HRV + ns->litr4n_strg_HRV;
		ns->litr1n_strg_HRV = 0;
		ns->litr2n_strg_HRV = 0;
		ns->litr3n_strg_HRV = 0;
		ns->litr4n_strg_HRV = 0;
	}
		
	/* CONTROL */
	diffC = (cs->HRVsnk-cs->HRVsrc) - cs->HRV_transportC - 
			(cs->litr1c_strg_HRV+cs->litr2c_strg_HRV+cs->litr3c_strg_HRV+cs->litr4c_strg_HRV);

	diffN = (ns->HRVsnk-ns->HRVsrc) - ns->HRV_transportN - 
		    (ns->litr1n_strg_HRV+ns->litr2n_strg_HRV+ns->litr3n_strg_HRV+ns->litr4n_strg_HRV) ;

	if (mgmd >= 0 && (fabs(diffC) > 1e-3 || fabs(diffN) > 1e-3 || cs->litr_belowground < 0))
	{
	//	printf("Warning: small rounding error in harvesting pools (harvesting.c)\n");
	}
	

   return (!ok);
}
	