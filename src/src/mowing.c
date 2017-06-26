/* 
mowing.c
do mowing  - decrease the plant material (leafc, leafn, canopy water)

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.6
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

int mowing(const control_struct* ctrl, const epconst_struct* epc, mowing_struct* MOW, cflux_struct* cf, nflux_struct* nf, wflux_struct* wf,
				  cstate_struct* cs, nstate_struct* ns, wstate_struct* ws)
{
	/* mowing parameters */
	double LAI_limit;
	double remained_prop;						/* remained proportion of plabnt material is calculated from transport coefficient */
	double MOW_to_litr1c_strg, MOW_to_litr2c_strg, MOW_to_litr3c_strg, MOW_to_litr4c_strg, MOW_to_transpC;
	double MOW_to_litr1n_strg, MOW_to_litr2n_strg, MOW_to_litr3n_strg, MOW_to_litr4n_strg, MOW_to_transpN;
	double diffC, diffN;
	/* local parameters */
	double befgrass_LAI;						/* value of LAI before mowing */
	double MOWcoeff;							/* coefficient determining the decrease of plant material caused by mowing */
	double litr1c_STDB_to_MOW, litr2c_STDB_to_MOW, litr3c_STDB_to_MOW, litr4c_STDB_to_MOW;
	double litr1n_STDB_to_MOW, litr2n_STDB_to_MOW, litr3n_STDB_to_MOW, litr4n_STDB_to_MOW;

	int ok=1;
	int ny;
	int mgmd = MOW->mgmd;
	double MOW_to_litter_coeff = epc->mort_CnW_to_litter;


	/* test variable */
	double storage_MGMmort=epc->storage_MGMmort;

	/* yearly varied or constant management parameters */
	if(MOW->MOW_flag == 2)
	{
		ny = ctrl->simyr;
	}
	else ny=0;

	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                                    CALCULATING FLUXES 
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
	

	/* 1. mowing calculation based on LAI_limit: we assume that due to the mowing LAI (leafc) reduces to the value of LAI_limit  */
	befgrass_LAI = cs->leafc * epc->avg_proj_sla;

	/* 2. mowing type: fixday or LAIlimit */
	if (MOW->fixday_or_fixLAI_flag == 0 || MOW->MOW_flag == 2)
	{	
		if (mgmd >=0) 
		{
			remained_prop = (100 - MOW->transport_coeff_array[mgmd][ny])/100.;
			LAI_limit = MOW->LAI_limit_array[mgmd][ny];
		}
		else 
		{
			remained_prop = 0;
			LAI_limit = befgrass_LAI;
		}
	}
	else
	{
		/* mowing if LAI greater than a given value (fixLAI_befMOW) */
		if (cs->leafc * epc->avg_proj_sla > MOW->fixLAI_befMOW)
		{
			remained_prop = (100 - MOW->transport_coeff_array[0][0])/100.;
			LAI_limit = MOW->fixLAI_aftMOW;
		}
		else
		{
			remained_prop = 0;
			LAI_limit = befgrass_LAI;
		}
	}

	/* 3. effect of mowing: MOWcoeff */
	if (befgrass_LAI > LAI_limit)
	{	
		MOWcoeff = (1- LAI_limit/befgrass_LAI);
	}
	else
	{
		/* if LAI before mowing is less than LAI_limit_aftermowing -> no  mowing  */
		MOWcoeff  = 0.0;
	}	
					

	/**********************************************************************************************/
	/* 1. as results of the mowing the carbon, nitrogen and water content of the leaf decreases*/
	/* fruit simulation - Hidy 2013.: harvested fruit is transported from the site  */
	

	cf->leafc_to_MOW          = cs->leafc * MOWcoeff;
	cf->leafc_transfer_to_MOW = cs->leafc_transfer * MOWcoeff * storage_MGMmort;
	cf->leafc_storage_to_MOW  = cs->leafc_storage * MOWcoeff * storage_MGMmort;

	cf->fruitc_to_MOW          = cs->fruitc * MOWcoeff;
	cf->fruitc_transfer_to_MOW = cs->fruitc_transfer * MOWcoeff * storage_MGMmort;
	cf->fruitc_storage_to_MOW  = cs->fruitc_storage * MOWcoeff * storage_MGMmort;

	cf->softstemc_to_MOW          = cs->softstemc * MOWcoeff;
	cf->softstemc_transfer_to_MOW = cs->softstemc_transfer * MOWcoeff * storage_MGMmort;
	cf->softstemc_storage_to_MOW  = cs->softstemc_storage * MOWcoeff * storage_MGMmort;
	
	cf->gresp_transfer_to_MOW = cs->gresp_transfer * MOWcoeff * storage_MGMmort;
	cf->gresp_storage_to_MOW  = cs->gresp_storage * MOWcoeff * storage_MGMmort;

	/* standing dead biome */
	litr1c_STDB_to_MOW = cs->litr1c_STDB * MOWcoeff;
	litr2c_STDB_to_MOW = cs->litr2c_STDB * MOWcoeff;
	litr3c_STDB_to_MOW = cs->litr3c_STDB * MOWcoeff;
	litr4c_STDB_to_MOW = cs->litr4c_STDB * MOWcoeff;

	cf->STDBc_to_MOW = litr1c_STDB_to_MOW + litr2c_STDB_to_MOW + litr3c_STDB_to_MOW + litr4c_STDB_to_MOW;

	nf->leafn_to_MOW          = ns->leafn * MOWcoeff;
	nf->leafn_transfer_to_MOW = ns->leafn_transfer * MOWcoeff * storage_MGMmort;
	nf->leafn_storage_to_MOW  = ns->leafn_storage * MOWcoeff * storage_MGMmort;

	nf->fruitn_to_MOW          = ns->fruitn * MOWcoeff;
	nf->fruitn_transfer_to_MOW = ns->fruitn_transfer * MOWcoeff * storage_MGMmort;
	nf->fruitn_storage_to_MOW  = ns->fruitn_storage * MOWcoeff * storage_MGMmort;

	nf->softstemn_to_MOW          = ns->softstemn * MOWcoeff;
	nf->softstemn_transfer_to_MOW = ns->softstemn_transfer * MOWcoeff * storage_MGMmort;
	nf->softstemn_storage_to_MOW  = ns->softstemn_storage * MOWcoeff * storage_MGMmort;

	nf->retransn_to_MOW        = ns->retransn * MOWcoeff * storage_MGMmort;


	/* standing dead biome */
	litr1n_STDB_to_MOW = ns->litr1n_STDB * MOWcoeff;
	litr2n_STDB_to_MOW = ns->litr2n_STDB * MOWcoeff;
	litr3n_STDB_to_MOW = ns->litr3n_STDB * MOWcoeff;
	litr4n_STDB_to_MOW = ns->litr4n_STDB * MOWcoeff;

	nf->STDBn_to_MOW = litr1n_STDB_to_MOW + litr2n_STDB_to_MOW + litr3n_STDB_to_MOW + litr4n_STDB_to_MOW;
   
	/**********************************************************************************************/
	/* 2. part of the plant material is transported (MOW_to_transpC and MOW_to_transpN; transp_coeff = 1-remained_prop),
	      the rest remains at the site (MOW_to_litrc_strg, MOW_to_litrn_strg)*/

	MOW_to_transpC = (cf->leafc_to_MOW + cf->fruitc_to_MOW + cf->softstemc_to_MOW + cf->STDBc_to_MOW)  * (1-remained_prop);

	MOW_to_transpN = (nf->leafn_to_MOW + nf->fruitn_to_MOW + nf->softstemn_to_MOW + nf->STDBn_to_MOW) 	* (1-remained_prop);
	
	MOW_to_litr1c_strg = (cf->leafc_to_MOW * epc->leaflitr_flab + cf->fruitc_to_MOW* epc->fruitlitr_flab  + 
						  cf->softstemc_to_MOW* epc->softstemlitr_flab + litr1c_STDB_to_MOW) * remained_prop +
						 (cf->leafc_transfer_to_MOW  + cf->leafc_storage_to_MOW + cf->fruitc_transfer_to_MOW + cf->fruitc_storage_to_MOW + 
						  cf->softstemc_transfer_to_MOW + cf->softstemc_storage_to_MOW + 
						  cf->gresp_storage_to_MOW  + cf->gresp_transfer_to_MOW);

	MOW_to_litr2c_strg = (cf->leafc_to_MOW * epc->leaflitr_fucel  + cf->fruitc_to_MOW * epc->fruitlitr_fucel
							+ cf->softstemc_to_MOW * epc->softstemlitr_fucel
						    + litr2c_STDB_to_MOW) * remained_prop;

	MOW_to_litr3c_strg = (cf->leafc_to_MOW * epc->leaflitr_fscel  + cf->fruitc_to_MOW * epc->fruitlitr_fscel
		                    + cf->softstemc_to_MOW * epc->softstemlitr_fscel
							+ litr3c_STDB_to_MOW) * remained_prop;

	MOW_to_litr4c_strg = (cf->leafc_to_MOW * epc->leaflitr_flig   + cf->fruitc_to_MOW * epc->fruitlitr_flig
							+ cf->softstemc_to_MOW * epc->softstemlitr_flig
							+ litr4c_STDB_to_MOW) * remained_prop;

	MOW_to_litr1n_strg =  (nf->leafn_to_MOW * epc->leaflitr_flab  + nf->fruitn_to_MOW* epc->fruitlitr_flab + 
		                   nf->softstemn_to_MOW* epc->softstemlitr_flab + litr1n_STDB_to_MOW) * remained_prop + 
						  (nf->leafn_transfer_to_MOW  + nf->leafn_storage_to_MOW + nf->fruitn_transfer_to_MOW + nf->fruitn_storage_to_MOW +
						   nf->softstemn_transfer_to_MOW + nf->softstemn_storage_to_MOW + nf->retransn_to_MOW);

	MOW_to_litr2n_strg =  (nf->leafn_to_MOW * epc->leaflitr_fucel + nf->fruitn_to_MOW * epc->fruitlitr_fucel
		                    + nf->softstemn_to_MOW * epc->softstemlitr_fucel
							+ litr2n_STDB_to_MOW) * remained_prop;

	MOW_to_litr3n_strg =  (nf->leafn_to_MOW * epc->leaflitr_fscel + nf->fruitn_to_MOW * epc->fruitlitr_fscel
							+ nf->softstemn_to_MOW * epc->softstemlitr_fscel
							+ litr3n_STDB_to_MOW) * remained_prop;

	MOW_to_litr4n_strg =  (nf->leafn_to_MOW * epc->leaflitr_flig  + nf->fruitn_to_MOW * epc->fruitlitr_flig
							+ nf->softstemn_to_MOW * epc->softstemlitr_flig
							+ litr4n_STDB_to_MOW) * remained_prop;
   

	cs->litr1c_strg_MOW += MOW_to_litr1c_strg;
	cs->litr2c_strg_MOW += MOW_to_litr2c_strg;
	cs->litr3c_strg_MOW += MOW_to_litr3c_strg;
	cs->litr4c_strg_MOW += MOW_to_litr4c_strg;

	cs->MOW_transportC  += MOW_to_transpC;


	ns->litr1n_strg_MOW +=  MOW_to_litr1n_strg;
	ns->litr2n_strg_MOW +=  MOW_to_litr2n_strg;
	ns->litr3n_strg_MOW +=  MOW_to_litr3n_strg;
	ns->litr4n_strg_MOW +=  MOW_to_litr4n_strg;

	

	ns->MOW_transportN  += MOW_to_transpN;

	/**********************************************************************************************/
	/* 3.the remained part of the plant material gradually goes into the litter pool (litrc_strg_MOW, litrn_strg_MOW) */
 
	cf->MOW_to_litr1c = cs->litr1c_strg_MOW * MOW_to_litter_coeff;
	cf->MOW_to_litr2c = cs->litr2c_strg_MOW * MOW_to_litter_coeff;
	cf->MOW_to_litr3c = cs->litr3c_strg_MOW * MOW_to_litter_coeff;
	cf->MOW_to_litr4c = cs->litr4c_strg_MOW * MOW_to_litter_coeff;

	nf->MOW_to_litr1n = ns->litr1n_strg_MOW * MOW_to_litter_coeff;
	nf->MOW_to_litr2n = ns->litr2n_strg_MOW * MOW_to_litter_coeff;
	nf->MOW_to_litr3n = ns->litr3n_strg_MOW * MOW_to_litter_coeff;
	nf->MOW_to_litr4n = ns->litr4n_strg_MOW * MOW_to_litter_coeff;
	

	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                                    STATE UPDATE 
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/ 

	/* 1. CARBON */

	/* leaf */
	cs->MOWsnk += cf->leafc_to_MOW;
	cs->leafc -= cf->leafc_to_MOW;
	cs->MOWsnk += cf->leafc_transfer_to_MOW;
	cs->leafc_transfer -= cf->leafc_transfer_to_MOW;
	cs->MOWsnk += cf->leafc_storage_to_MOW;
	cs->leafc_storage -= cf->leafc_storage_to_MOW;
	cs->MOWsnk += cf->gresp_transfer_to_MOW;
	cs->gresp_transfer -= cf->gresp_transfer_to_MOW;
	cs->MOWsnk += cf->gresp_storage_to_MOW;
	cs->gresp_storage -= cf->gresp_storage_to_MOW;

	/* dead standing biomass */
	cs->MOWsnk += litr1c_STDB_to_MOW;
	cs->litr1c_STDB -= litr1c_STDB_to_MOW;
	cs->MOWsnk += litr2c_STDB_to_MOW;
	cs->litr2c_STDB -= litr2c_STDB_to_MOW;
	cs->MOWsnk += litr3c_STDB_to_MOW;
	cs->litr3c_STDB -= litr3c_STDB_to_MOW;
	cs->MOWsnk += litr4c_STDB_to_MOW;
	cs->litr4c_STDB -=litr4c_STDB_to_MOW;

	cs->SNSCsrc += cf->STDBc_to_MOW;
	cs->STDBc -= cf->STDBc_to_MOW;

	/* fruit simulation - Hidy 2013. */
	cs->MOWsnk += cf->fruitc_to_MOW;
	cs->fruitc -= cf->fruitc_to_MOW;
	cs->MOWsnk += cf->fruitc_transfer_to_MOW;
	cs->fruitc_transfer -= cf->fruitc_transfer_to_MOW;
	cs->MOWsnk += cf->fruitc_storage_to_MOW;
	cs->fruitc_storage -= cf->fruitc_storage_to_MOW;

	/* softstem simulation - Hidy 2013. */
	cs->MOWsnk += cf->softstemc_to_MOW;
	cs->softstemc -= cf->softstemc_to_MOW;
	cs->MOWsnk += cf->softstemc_transfer_to_MOW;
	cs->softstemc_transfer -= cf->softstemc_transfer_to_MOW;
	cs->MOWsnk += cf->softstemc_storage_to_MOW;
	cs->softstemc_storage -= cf->softstemc_storage_to_MOW;


	cs->litr1c += cf->MOW_to_litr1c;
	cs->litr2c += cf->MOW_to_litr2c;
	cs->litr3c += cf->MOW_to_litr3c;
	cs->litr4c += cf->MOW_to_litr4c;
	
    /* decreasing litter storage state variables*/
	cs->litr1c_strg_MOW -= cf->MOW_to_litr1c;
	cs->litr2c_strg_MOW -= cf->MOW_to_litr2c;
	cs->litr3c_strg_MOW -= cf->MOW_to_litr3c;
	cs->litr4c_strg_MOW -= cf->MOW_to_litr4c;

	/* litter plus because of mowing and returning of dead plant material */
	cs->MOWsrc += cf->MOW_to_litr1c + cf->MOW_to_litr2c + cf->MOW_to_litr3c + cf->MOW_to_litr4c;


	/* 2. NITROGEN */

	/* leaf */
	ns->MOWsnk += nf->leafn_to_MOW;
	ns->leafn -= nf->leafn_to_MOW;
	ns->MOWsnk += nf->leafn_transfer_to_MOW;
	ns->leafn_transfer -= nf->leafn_transfer_to_MOW;
	ns->MOWsnk += nf->leafn_storage_to_MOW;
	ns->leafn_storage -= nf->leafn_storage_to_MOW;

	/* dead standing biomass */
	ns->MOWsnk += litr1n_STDB_to_MOW;
	ns->litr1n_STDB -= litr1n_STDB_to_MOW;
	ns->MOWsnk += litr2n_STDB_to_MOW;
	ns->litr2n_STDB -= litr2n_STDB_to_MOW;
	ns->MOWsnk += litr3n_STDB_to_MOW;
	ns->litr3n_STDB -= litr3n_STDB_to_MOW;
	ns->MOWsnk += litr4n_STDB_to_MOW;
	ns->litr4n_STDB -=litr4n_STDB_to_MOW;

	ns->SNSCsrc += nf->STDBn_to_MOW;
	ns->STDBn -= nf->STDBn_to_MOW;


	/* fruit simulation - Hidy 2013. */
	ns->MOWsnk += nf->fruitn_to_MOW;
	ns->fruitn -= nf->fruitn_to_MOW;
	ns->MOWsnk += nf->fruitn_transfer_to_MOW;
	ns->fruitn_transfer -= nf->fruitn_transfer_to_MOW;
	ns->MOWsnk += nf->fruitn_storage_to_MOW;
	ns->fruitn_storage -= nf->fruitn_storage_to_MOW;
	/* softstem simulation - Hidy 2013. */
	ns->MOWsnk += nf->softstemn_to_MOW;
	ns->softstemn -= nf->softstemn_to_MOW;
	ns->MOWsnk += nf->softstemn_transfer_to_MOW;
	ns->softstemn_transfer -= nf->softstemn_transfer_to_MOW;
	ns->MOWsnk += nf->softstemn_storage_to_MOW;
	ns->softstemn_storage -= nf->softstemn_storage_to_MOW;
	ns->MOWsnk += nf->retransn_to_MOW;
	ns->retransn -= nf->retransn_to_MOW;

	ns->litr1n += nf->MOW_to_litr1n;
	ns->litr2n += nf->MOW_to_litr2n;
	ns->litr3n += nf->MOW_to_litr3n;
	ns->litr4n += nf->MOW_to_litr4n;



	/* decreasing litter storage state variables*/
	ns->litr1n_strg_MOW -= nf->MOW_to_litr1n;
	ns->litr2n_strg_MOW -= nf->MOW_to_litr2n;
	ns->litr3n_strg_MOW -= nf->MOW_to_litr3n;
	ns->litr4n_strg_MOW -= nf->MOW_to_litr4n;

	ns->MOWsrc += nf->MOW_to_litr1n + nf->MOW_to_litr2n + nf->MOW_to_litr3n + nf->MOW_to_litr4n;


	/* 3. water */
	ws->canopyw_MOWsnk += wf->canopyw_to_MOW;
	ws->canopyw -= wf->canopyw_to_MOW;

	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                                    TEMPORARY POOLS
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
	/* temporary MOWed plant material pools: if litr1c_strg_MOW is less than a crit. value, the temporary pool becomes empty */

	if (cs->litr1c_strg_MOW < CRIT_PREC && cs->litr1c_strg_MOW != 0) 
	{
		cs->MOWsrc += (cs->litr1c_strg_MOW + cs->litr2c_strg_MOW + cs->litr3c_strg_MOW + cs->litr4c_strg_MOW);
		cs->litr1c_strg_MOW = 0;
		cs->litr2c_strg_MOW = 0;
		cs->litr3c_strg_MOW = 0;
		cs->litr4c_strg_MOW = 0;
		ns->MOWsrc += (ns->litr1n_strg_MOW + ns->litr2n_strg_MOW + ns->litr3n_strg_MOW + ns->litr4n_strg_MOW);
		ns->litr1n_strg_MOW = 0;
		ns->litr2n_strg_MOW = 0;
		ns->litr3n_strg_MOW = 0;
		ns->litr4n_strg_MOW = 0;
	}
		
	/* CONTROL */
	diffC = (cs->MOWsnk-cs->MOWsrc) - cs->MOW_transportC - 
		    (cs->litr1c_strg_MOW+cs->litr2c_strg_MOW+cs->litr3c_strg_MOW+cs->litr4c_strg_MOW);

	diffN = (ns->MOWsnk-ns->MOWsrc) - ns->MOW_transportN - 
		    (ns->litr1n_strg_MOW+ns->litr2n_strg_MOW+ns->litr3n_strg_MOW+ns->litr4n_strg_MOW);

	if (fabs(diffC) > 1e-3 || fabs(diffN) > 1e-3)
	{
	 //	if (ctrl->onscreen) printf("WARNING: small rounding error in mowing pools (mowing.c)\n");
	}
	


  return (!ok);
}	