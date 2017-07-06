 /* 
ploughing.c
do ploughing  - decrease the plant material (leafc, leafn, canopy water)

 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.7
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

int ploughing(const control_struct* ctrl, const epconst_struct* epc, siteconst_struct* sitec, metvar_struct* metv,  epvar_struct* epv,
			  ploughing_struct* PLG, cflux_struct* cf, nflux_struct* nf, wflux_struct* wf,  cstate_struct* cs, nstate_struct* ns, wstate_struct* ws)
{
	/* ploughing parameters */

	int ny, PLGdepth, layer, ok, mgmd;
	double PLGcoeff, sminn_SUM, soilw_SUM, tsoil_SUM, sand_SUM, silt_SUM;	
	double litr1c_STDB_to_PLG, litr2c_STDB_to_PLG, litr3c_STDB_to_PLG, litr4c_STDB_to_PLG;
	double litr1n_STDB_to_PLG, litr2n_STDB_to_PLG, litr3n_STDB_to_PLG, litr4n_STDB_to_PLG;
	double litr1c_CTDB_to_PLG, litr2c_CTDB_to_PLG, litr3c_CTDB_to_PLG, litr4c_CTDB_to_PLG;
	double litr1n_CTDB_to_PLG, litr2n_CTDB_to_PLG, litr3n_CTDB_to_PLG, litr4n_CTDB_to_PLG;
	double diffC, diffN;

	/* test variable */
	double storage_MGMmort=epc->storage_MGMmort;

	PLGdepth=0;
	ok=1;
	mgmd = PLG->mgmd;

	/* yearly varied or constant management parameters */
	if(PLG->PLG_flag == 2)
	{
		ny = ctrl->simyr;
	}
	else ny=0;

	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                                    INITALIZING PARAMETERS
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

	/* ploughing if gapflag=1 */
	if (mgmd >=0)
	{
		PLGcoeff      = 1.0; /* decrease of plant material caused by ploughing: difference between plant material before and after harvesting */
		PLG->DC_act   = PLG->dissolv_coeff_array[mgmd][ny]; 	 
		
		if (PLG->PLGdepths_array[mgmd][ny] >= 1 && PLG->PLGdepths_array[mgmd][ny] <= 3)
		{
			if (PLG->PLGdepths_array[mgmd][ny] == 1)
			{
				PLGdepth=1;
				printf("Shallow ploughing at 0-10 cm (ploughing.c)\n");
			}
			else
			{
				if (PLG->PLGdepths_array[mgmd][ny] == 2)
				{
					PLGdepth=2;
					printf("Medium ploughing at 10-30 cm (ploughing.c)\n");
				}
				else
				{
					if (PLG->PLGdepths_array[mgmd][ny] == 3)
					{
						PLGdepth=3;
						printf("Deep ploughing at 30-60 cm (ploughing.c)\n");
					}
					else
					{
						printf("Error: incorrect ploughing depth data(1, 2 or 3) (ploughing.c)\n");
						ok=0;
					}
				}
			}
		
		}
		else
		{
			printf("Error: missing ploughing depth data (ploughing.c)\n");
			ok=0;
		}
	}
			
	else
	{
		PLGcoeff=0.0;
	}
	
	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                                    UNIFORM DISTRIBUTION OF SMINN AND VWC AFTER PLOUGHING
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

	soilw_SUM=sminn_SUM=sand_SUM=silt_SUM=tsoil_SUM=0;


	if (mgmd >=0)
	{
		for (layer = 0; layer<PLGdepth; layer++)
		{
			soilw_SUM += ws->soilw[layer];
			sminn_SUM += ns->sminn[layer];
			sand_SUM  += sitec->sand[layer];
			silt_SUM  += sitec->silt[layer];

			tsoil_SUM += metv->tsoil[layer] * sitec->soillayer_thickness[layer] / sitec->soillayer_depth[PLGdepth-1];
		}

		for (layer = 0; layer<PLGdepth; layer++)
		{
			ws->soilw[layer]   = soilw_SUM * sitec->soillayer_thickness[layer] / sitec->soillayer_depth[PLGdepth-1];
			ns->sminn[layer]   = sminn_SUM * sitec->soillayer_thickness[layer] / sitec->soillayer_depth[PLGdepth-1];
			epv->vwc[layer]    = ws->soilw[layer] / (water_density * sitec->soillayer_thickness[layer]);

			metv->tsoil[layer] = tsoil_SUM;

			sitec->sand[layer] = sand_SUM/PLGdepth;
			sitec->silt[layer] = silt_SUM/PLGdepth;
			sitec->clay[layer] = 100-sitec->sand[layer]-sitec->silt[layer];
		}

		/* update TSOIL values */
		metv->tsoil_surface_pre = metv->tsoil[0];
		metv->tsoil_surface     = metv->tsoil[0];



	}


	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                                    CALCULATING FLUXES 
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
	/* 1. CARBON */

	/* as result of the ploughing no plant material above the ground */
	cf->leafc_to_PLG = cs->leafc * PLGcoeff;
	cf->leafc_storage_to_PLG = cs->leafc_storage * PLGcoeff * storage_MGMmort;
	cf->leafc_transfer_to_PLG = cs->leafc_transfer * PLGcoeff * storage_MGMmort;

	cf->gresp_storage_to_PLG = cs->gresp_storage * PLGcoeff * storage_MGMmort;
	cf->gresp_transfer_to_PLG = cs->gresp_transfer * PLGcoeff * storage_MGMmort;

	cf->frootc_to_PLG = cs->frootc * PLGcoeff;
	cf->frootc_storage_to_PLG = cs->frootc_storage * PLGcoeff * storage_MGMmort;
	cf->frootc_transfer_to_PLG = cs->frootc_transfer * PLGcoeff * storage_MGMmort;

	/* fruit simulation */
	cf->fruitc_to_PLG = cs->fruitc * PLGcoeff;
	cf->fruitc_storage_to_PLG = cs->fruitc_storage * PLGcoeff * storage_MGMmort;
	cf->fruitc_transfer_to_PLG = cs->fruitc_transfer * PLGcoeff * storage_MGMmort;

	/* softstem simulation */
	cf->softstemc_to_PLG = cs->softstemc * PLGcoeff;
	cf->softstemc_storage_to_PLG = cs->softstemc_storage * PLGcoeff * storage_MGMmort;
	cf->softstemc_transfer_to_PLG = cs->softstemc_transfer * PLGcoeff * storage_MGMmort;

   /* standing dead biome */
	litr1c_STDB_to_PLG = cs->litr1c_STDB * PLGcoeff;
	litr2c_STDB_to_PLG = cs->litr2c_STDB * PLGcoeff;
	litr3c_STDB_to_PLG = cs->litr3c_STDB * PLGcoeff;
	litr4c_STDB_to_PLG = cs->litr4c_STDB * PLGcoeff;

	cf->STDBc_to_PLG = litr1c_STDB_to_PLG + litr2c_STDB_to_PLG + litr3c_STDB_to_PLG + litr4c_STDB_to_PLG;

	 /* cut-down dead biome */
	litr1c_CTDB_to_PLG = cs->litr1c_strg_HRV * PLGcoeff + cs->litr1c_strg_MOW * PLGcoeff + cs->litr1c_strg_THN * PLGcoeff;
	litr2c_CTDB_to_PLG = cs->litr2c_strg_HRV * PLGcoeff + cs->litr2c_strg_MOW * PLGcoeff + cs->litr2c_strg_THN * PLGcoeff;
	litr3c_CTDB_to_PLG = cs->litr3c_strg_HRV * PLGcoeff + cs->litr3c_strg_MOW * PLGcoeff + cs->litr3c_strg_THN * PLGcoeff;
	litr4c_CTDB_to_PLG = cs->litr4c_strg_HRV * PLGcoeff + cs->litr4c_strg_MOW * PLGcoeff + cs->litr4c_strg_THN * PLGcoeff;
	
	cf->CTDBc_to_PLG = litr1c_CTDB_to_PLG + litr2c_CTDB_to_PLG + litr3c_CTDB_to_PLG + litr4c_CTDB_to_PLG;

	/* 2. NITROGEN */

	nf->leafn_to_PLG = ns->leafn * PLGcoeff;
	nf->leafn_storage_to_PLG  = ns->leafn_storage * PLGcoeff * storage_MGMmort;;
	nf->leafn_transfer_to_PLG = ns->leafn_transfer * PLGcoeff * storage_MGMmort;

	nf->frootn_to_PLG = ns->frootn * PLGcoeff;
	nf->frootn_storage_to_PLG = ns->frootn_storage * PLGcoeff * storage_MGMmort;
	nf->frootn_transfer_to_PLG = ns->frootn_transfer * PLGcoeff * storage_MGMmort;

	/* fruit simulation - Hidy 2013 */
	nf->fruitn_to_PLG = ns->fruitn * PLGcoeff;
	nf->fruitn_storage_to_PLG = ns->fruitn_storage * PLGcoeff * storage_MGMmort;
	nf->fruitn_transfer_to_PLG = ns->fruitn_transfer * PLGcoeff * storage_MGMmort;

	/* softstem simulation - Hidy 2013 */
	nf->softstemn_to_PLG = ns->softstemn * PLGcoeff;
	nf->softstemn_storage_to_PLG = ns->softstemn_storage * PLGcoeff * storage_MGMmort;
	nf->softstemn_transfer_to_PLG = ns->softstemn_transfer * PLGcoeff * storage_MGMmort;


	/* softstem simulation - Hidy 2013 */
	nf->softstemn_to_PLG = ns->softstemn * PLGcoeff;
	nf->softstemn_storage_to_PLG = ns->softstemn_storage * PLGcoeff * storage_MGMmort;
	nf->softstemn_transfer_to_PLG = ns->softstemn_transfer * PLGcoeff * storage_MGMmort;

	/* standing dead biome */
	litr1n_STDB_to_PLG = ns->litr1n_STDB * PLGcoeff;
	litr2n_STDB_to_PLG = ns->litr2n_STDB * PLGcoeff;
	litr3n_STDB_to_PLG = ns->litr3n_STDB * PLGcoeff;
	litr4n_STDB_to_PLG = ns->litr4n_STDB * PLGcoeff;

	nf->STDBn_to_PLG = litr1n_STDB_to_PLG + litr2n_STDB_to_PLG + litr3n_STDB_to_PLG + litr4n_STDB_to_PLG;

	/* CUT-DOWN dead biome */
	litr1n_CTDB_to_PLG = ns->litr1n_strg_HRV * PLGcoeff + ns->litr1n_strg_MOW * PLGcoeff + ns->litr1n_strg_THN * PLGcoeff;
	litr2n_CTDB_to_PLG = ns->litr2n_strg_HRV * PLGcoeff + ns->litr2n_strg_MOW * PLGcoeff + ns->litr2n_strg_THN * PLGcoeff;
	litr3n_CTDB_to_PLG = ns->litr3n_strg_HRV * PLGcoeff + ns->litr3n_strg_MOW * PLGcoeff + ns->litr3n_strg_THN * PLGcoeff;
	litr4n_CTDB_to_PLG = ns->litr4n_strg_HRV * PLGcoeff + ns->litr4n_strg_MOW * PLGcoeff + ns->litr4n_strg_THN * PLGcoeff;


	nf->CTDBn_to_PLG = litr1n_CTDB_to_PLG + litr2n_CTDB_to_PLG + litr3n_CTDB_to_PLG + litr4n_CTDB_to_PLG;
   
	/* restranslocated N pool is decreasing also */
	nf->retransn_to_PLG = ns->retransn * PLGcoeff;

	wf->canopyw_to_PLG = ws->canopyw * PLGcoeff;


	/* increasing ploughing actual pool */
	/* carbon */
	PLG->PLG_pool_litr1c += cf->leafc_to_PLG * epc->leaflitr_flab + 
						   cf->frootc_to_PLG * epc->frootlitr_flab +
						   cf->fruitc_to_PLG * epc->fruitlitr_flab +
						   cf->softstemc_to_PLG * epc->softstemlitr_flab +
						   cf->leafc_storage_to_PLG + cf->leafc_transfer_to_PLG +
						   cf->frootc_storage_to_PLG + cf->frootc_transfer_to_PLG +
						   cf->fruitc_storage_to_PLG + cf->fruitc_transfer_to_PLG +
						   cf->softstemc_storage_to_PLG + cf->softstemc_transfer_to_PLG +
						   cf->gresp_storage_to_PLG + cf->gresp_transfer_to_PLG;
	
	PLG->PLG_pool_litr2c += cf->leafc_to_PLG* epc->leaflitr_fucel + 
						   cf->frootc_to_PLG * epc->frootlitr_fucel +
						   cf->fruitc_to_PLG * epc->fruitlitr_fucel +
						   cf->softstemc_to_PLG * epc->softstemlitr_fucel;

	PLG->PLG_pool_litr3c += cf->leafc_to_PLG * epc->leaflitr_fscel + 
						   cf->frootc_to_PLG * epc->frootlitr_fscel +
						   cf->fruitc_to_PLG * epc->fruitlitr_fscel +
						   cf->softstemc_to_PLG * epc->softstemlitr_fscel;

	PLG->PLG_pool_litr4c += cf->leafc_to_PLG * epc->leaflitr_flig + 
						   cf->frootc_to_PLG * epc->frootlitr_flig +
						   cf->fruitc_to_PLG * epc->fruitlitr_flig +
						   cf->softstemc_to_PLG * epc->softstemlitr_flig;


	/* nitrogen*/
	PLG->PLG_pool_litr1n += nf->leafn_to_PLG * epc->leaflitr_flab + 
						   nf->frootn_to_PLG * epc->frootlitr_flab +
						   nf->fruitn_to_PLG * epc->fruitlitr_flab +
						   nf->softstemn_to_PLG * epc->softstemlitr_flab +
						   nf->leafn_storage_to_PLG + nf->leafn_transfer_to_PLG +
						   nf->frootn_storage_to_PLG + nf->frootn_transfer_to_PLG +
						   nf->fruitn_storage_to_PLG + nf->fruitn_transfer_to_PLG + 
						   nf->softstemn_storage_to_PLG + nf->softstemn_transfer_to_PLG + 
						   nf->retransn_to_PLG;
	
	PLG->PLG_pool_litr2n += nf->leafn_to_PLG* epc->leaflitr_fucel + 
						   nf->frootn_to_PLG * epc->frootlitr_fucel +
						   nf->fruitn_to_PLG * epc->fruitlitr_fucel +
						   nf->softstemn_to_PLG * epc->softstemlitr_fucel;

	PLG->PLG_pool_litr3n += nf->leafn_to_PLG * epc->leaflitr_fscel + 
						   nf->frootn_to_PLG * epc->frootlitr_fscel +
						   nf->fruitn_to_PLG * epc->fruitlitr_fscel +
						   nf->softstemn_to_PLG * epc->softstemlitr_fscel;

	PLG->PLG_pool_litr4n += nf->leafn_to_PLG * epc->leaflitr_flig + 
						   nf->frootn_to_PLG * epc->frootlitr_flig +
						   nf->fruitn_to_PLG * epc->fruitlitr_flig +
						   nf->softstemn_to_PLG * epc->softstemlitr_flig;

	
	if (PLG->PLG_pool_litr1c > CRIT_PREC)
	{
		/* increasing litter content*/
		cf->PLG_to_litr1c = PLG->PLG_pool_litr1c * PLG->DC_act;
		cf->PLG_to_litr2c = PLG->PLG_pool_litr2c * PLG->DC_act;
		cf->PLG_to_litr3c = PLG->PLG_pool_litr3c * PLG->DC_act;
		cf->PLG_to_litr4c = PLG->PLG_pool_litr4c * PLG->DC_act;

		nf->PLG_to_litr1n = PLG->PLG_pool_litr1n * PLG->DC_act;
		nf->PLG_to_litr2n = PLG->PLG_pool_litr2n * PLG->DC_act; 
		nf->PLG_to_litr3n = PLG->PLG_pool_litr3n * PLG->DC_act; 
		nf->PLG_to_litr4n = PLG->PLG_pool_litr4n * PLG->DC_act; 
	}
	else
	{
		/* increasing litter content*/
		cf->PLG_to_litr1c = PLG->PLG_pool_litr1c;
		cf->PLG_to_litr2c = PLG->PLG_pool_litr2c;
		cf->PLG_to_litr3c = PLG->PLG_pool_litr3c;
		cf->PLG_to_litr4c = PLG->PLG_pool_litr4c;

		nf->PLG_to_litr1n = PLG->PLG_pool_litr1n;
		nf->PLG_to_litr2n = PLG->PLG_pool_litr2n; 
		nf->PLG_to_litr3n = PLG->PLG_pool_litr3n; 
		nf->PLG_to_litr4n = PLG->PLG_pool_litr4n; 
	
	}
	/* decreasing ploughing actual pool content*/
	PLG->PLG_pool_litr1c -= cf->PLG_to_litr1c;
	PLG->PLG_pool_litr2c -= cf->PLG_to_litr2c;
	PLG->PLG_pool_litr3c -= cf->PLG_to_litr3c;
	PLG->PLG_pool_litr4c -= cf->PLG_to_litr4c;

	PLG->PLG_pool_litr1n -= nf->PLG_to_litr1n;
	PLG->PLG_pool_litr2n -= nf->PLG_to_litr2n; 
	PLG->PLG_pool_litr3n -= nf->PLG_to_litr3n; 
	PLG->PLG_pool_litr4n -= nf->PLG_to_litr4n; 
	

	cs->PLG_cpool = PLG->PLG_pool_litr1c + PLG->PLG_pool_litr2c + PLG->PLG_pool_litr3c + PLG->PLG_pool_litr4c;
	ns->PLG_npool = PLG->PLG_pool_litr1n + PLG->PLG_pool_litr2n + PLG->PLG_pool_litr3n + PLG->PLG_pool_litr4n;




	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                                    STATE UPDATE 
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/ 

	/* 1. CARBON */

	/* leaf, froot */
	cs->PLGsnk += cf->leafc_to_PLG;
	cs->leafc -= cf->leafc_to_PLG;
	cs->PLGsnk += cf->leafc_transfer_to_PLG;
	cs->leafc_transfer -= cf->leafc_transfer_to_PLG;
	cs->PLGsnk += cf->leafc_storage_to_PLG;
	cs->leafc_storage -= cf->leafc_storage_to_PLG;
	cs->PLGsnk += cf->gresp_transfer_to_PLG;
	
	cs->gresp_transfer -= cf->gresp_transfer_to_PLG;
	cs->PLGsnk += cf->gresp_storage_to_PLG;
	cs->gresp_storage -= cf->gresp_storage_to_PLG;
	
	cs->PLGsnk += cf->frootc_to_PLG;
	cs->frootc -= cf->frootc_to_PLG;
	cs->PLGsnk += cf->frootc_transfer_to_PLG;
	cs->frootc_transfer -= cf->frootc_transfer_to_PLG;
	cs->PLGsnk += cf->frootc_storage_to_PLG;
	cs->frootc_storage -= cf->frootc_storage_to_PLG;
	
	/* dead standing biomass */
	cs->PLGsnk += litr1c_STDB_to_PLG;
	cs->litr1c_STDB -= litr1c_STDB_to_PLG;
	cs->PLGsnk += litr2c_STDB_to_PLG;
	cs->litr2c_STDB -= litr2c_STDB_to_PLG;
	cs->PLGsnk += litr3c_STDB_to_PLG;
	cs->litr3c_STDB -= litr3c_STDB_to_PLG;
	cs->PLGsnk += litr4c_STDB_to_PLG;
	cs->litr4c_STDB -=litr4c_STDB_to_PLG;

	cs->SNSCsrc += cf->STDBc_to_PLG;
	cs->STDBc -= cf->STDBc_to_PLG;


	/* fruit simulation - Hidy 2013. */
	cs->PLGsnk += cf->fruitc_to_PLG;
	cs->fruitc -= cf->fruitc_to_PLG;
	cs->PLGsnk += cf->fruitc_transfer_to_PLG;
	cs->fruitc_transfer -= cf->fruitc_transfer_to_PLG;
	cs->PLGsnk += cf->fruitc_storage_to_PLG;
	cs->fruitc_storage -= cf->fruitc_storage_to_PLG;

	/* softstem simulation - Hidy 2013. */
	cs->PLGsnk += cf->softstemc_to_PLG;
	cs->softstemc -= cf->softstemc_to_PLG;
	cs->PLGsnk += cf->softstemc_transfer_to_PLG;
	cs->softstemc_transfer -= cf->softstemc_transfer_to_PLG;
	cs->PLGsnk += cf->softstemc_storage_to_PLG;
	cs->softstemc_storage -= cf->softstemc_storage_to_PLG;

	cs->litr1c += cf->PLG_to_litr1c;
	cs->litr2c += cf->PLG_to_litr2c;
	cs->litr3c += cf->PLG_to_litr3c;
	cs->litr4c += cf->PLG_to_litr4c;
	
	cs->PLGsrc += cf->PLG_to_litr1c + cf->PLG_to_litr2c + cf->PLG_to_litr3c + cf->PLG_to_litr4c;

	/* CUT-DOWN biomass */
	cs->litr1c_strg_HRV  -= cs->litr1c_strg_HRV * PLGcoeff;
	cs->litr1c_strg_MOW  -= cs->litr1c_strg_HRV * PLGcoeff;
	cs->litr1c_strg_THN  -= cs->litr1c_strg_HRV * PLGcoeff;
	cs->litr2c_strg_HRV  -= cs->litr2c_strg_HRV * PLGcoeff;
	cs->litr2c_strg_MOW  -= cs->litr2c_strg_HRV * PLGcoeff;
	cs->litr2c_strg_THN  -= cs->litr2c_strg_HRV * PLGcoeff;
	cs->litr3c_strg_HRV  -= cs->litr3c_strg_HRV * PLGcoeff;
	cs->litr3c_strg_MOW  -= cs->litr3c_strg_HRV * PLGcoeff;
	cs->litr3c_strg_THN -= cs->litr3c_strg_HRV * PLGcoeff;
	cs->litr4c_strg_HRV -= cs->litr4c_strg_HRV * PLGcoeff;
	cs->litr4c_strg_MOW -= cs->litr4c_strg_HRV * PLGcoeff;
	cs->litr4c_strg_THN -= cs->litr4c_strg_HRV * PLGcoeff;

	
	/* 2. NITROGEN */

	/* leaf, froot */
	ns->PLGsnk += nf->leafn_to_PLG;
	ns->leafn -= nf->leafn_to_PLG;
	ns->PLGsnk += nf->leafn_transfer_to_PLG;
	ns->leafn_transfer -= nf->leafn_transfer_to_PLG;
	ns->PLGsnk += nf->leafn_storage_to_PLG;
	ns->leafn_storage -= nf->leafn_storage_to_PLG;
	
	ns->PLGsnk += nf->frootn_to_PLG;
	ns->frootn -= nf->frootn_to_PLG;
	ns->PLGsnk += nf->frootn_transfer_to_PLG;
	ns->frootn_transfer -= nf->frootn_transfer_to_PLG;
	ns->PLGsnk += nf->frootn_storage_to_PLG;
	ns->frootn_storage -= nf->frootn_storage_to_PLG;
   
	/* dead standing biomass */
	ns->PLGsnk += litr1n_STDB_to_PLG;
	ns->litr1n_STDB -= litr1n_STDB_to_PLG;
	ns->PLGsnk += litr2n_STDB_to_PLG;
	ns->litr2n_STDB -= litr2n_STDB_to_PLG;
	ns->PLGsnk += litr3n_STDB_to_PLG;
	ns->litr3n_STDB -= litr3n_STDB_to_PLG;
	ns->PLGsnk += litr4n_STDB_to_PLG;
	ns->litr4n_STDB -=litr4n_STDB_to_PLG;

	ns->SNSCsrc += nf->STDBn_to_PLG;
	ns->STDBn -= nf->STDBn_to_PLG;

	 /* CUT-DOWN standing biomass */
	ns->litr1n_strg_HRV  -= ns->litr1n_strg_HRV * PLGcoeff;
	ns->litr1n_strg_MOW  -= ns->litr1n_strg_HRV * PLGcoeff;
	ns->litr1n_strg_THN  -= ns->litr1n_strg_HRV * PLGcoeff;
	ns->litr2n_strg_HRV  -= ns->litr2n_strg_HRV * PLGcoeff;
	ns->litr2n_strg_MOW  -= ns->litr2n_strg_HRV * PLGcoeff;
	ns->litr2n_strg_THN  -= ns->litr2n_strg_HRV * PLGcoeff;
	ns->litr3n_strg_HRV  -= ns->litr3n_strg_HRV * PLGcoeff;
	ns->litr3n_strg_MOW  -= ns->litr3n_strg_HRV * PLGcoeff;
	ns->litr3n_strg_THN  -= ns->litr3n_strg_HRV * PLGcoeff;
	ns->litr4n_strg_HRV  -= ns->litr4n_strg_HRV * PLGcoeff;
	ns->litr4n_strg_MOW  -= ns->litr4n_strg_HRV * PLGcoeff;
	ns->litr4n_strg_THN  -= ns->litr4n_strg_HRV * PLGcoeff;

	/* fruit simulation - Hidy 2013. */
	ns->PLGsnk += nf->fruitn_to_PLG;
	ns->fruitn -= nf->fruitn_to_PLG;
	ns->PLGsnk += nf->fruitn_transfer_to_PLG;
	ns->fruitn_transfer -= nf->fruitn_transfer_to_PLG;
	ns->PLGsnk += nf->fruitn_storage_to_PLG;
	ns->fruitn_storage -= nf->fruitn_storage_to_PLG;
	/* softstem simulation - Hidy 2013. */
	ns->PLGsnk += nf->softstemn_to_PLG;
	ns->softstemn -= nf->softstemn_to_PLG;
	ns->PLGsnk += nf->softstemn_transfer_to_PLG;
	ns->softstemn_transfer -= nf->softstemn_transfer_to_PLG;
	ns->PLGsnk += nf->softstemn_storage_to_PLG;
	ns->softstemn_storage -= nf->softstemn_storage_to_PLG;

	ns->PLGsnk += nf->retransn_to_PLG;
	ns->retransn -= nf->retransn_to_PLG;

	ns->litr1n += nf->PLG_to_litr1n;
	ns->litr2n += nf->PLG_to_litr2n;
	ns->litr3n += nf->PLG_to_litr3n;
	ns->litr4n += nf->PLG_to_litr4n;
	ns->PLGsrc += nf->PLG_to_litr1n + nf->PLG_to_litr2n + nf->PLG_to_litr3n + nf->PLG_to_litr4n;

	/* 3. water */
	ws->canopyw_PLGsnk += wf->canopyw_to_PLG;
	ws->canopyw -= wf->canopyw_to_PLG;
	ws->soilw[0] += wf->canopyw_to_PLG;


		
	/* CONTROL */
	diffC = (cs->PLGsnk-cs->PLGsrc) - (PLG->PLG_pool_litr1c + PLG->PLG_pool_litr2c + PLG->PLG_pool_litr3c + PLG->PLG_pool_litr4c);

	diffN = (ns->PLGsnk-ns->PLGsrc) - (PLG->PLG_pool_litr1n + PLG->PLG_pool_litr2n + PLG->PLG_pool_litr3n + PLG->PLG_pool_litr4n);

	if (mgmd >= 0 && (fabs(diffC) > 1e-3 || fabs(diffN) > 1e-4))
	{
	//	printf("Warning: small rounding error in harvesting pools (harvesting.c)\n");
	}
	

   return (!ok);
}
	