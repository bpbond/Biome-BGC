 /* 
ploughing.c
do ploughing  - decrease the plant material (leafc, leafn, canopy water)

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

int ploughing(const control_struct* ctrl, const epconst_struct* epc, siteconst_struct* sitec, metvar_struct* metv,  epvar_struct* epv,
			  ploughing_struct* PLG, cflux_struct* cf, nflux_struct* nf, wflux_struct* wf,  cstate_struct* cs, nstate_struct* ns, wstate_struct* ws)
{
	/* ploughing parameters */

	int ny, PLGdepth, layer, ok, mgmd;
	double PLGcoeff, sminn_SUM, soilw_SUM, tsoil_SUM, sand_SUM, silt_SUM;	
	double STDB_litr1c_to_PLG, STDB_litr2c_to_PLG, STDB_litr3c_to_PLG, STDB_litr4c_to_PLG;
	double STDB_litr1n_to_PLG, STDB_litr2n_to_PLG, STDB_litr3n_to_PLG, STDB_litr4n_to_PLG;
	double CTDB_litr1c_to_PLG, CTDB_litr2c_to_PLG, CTDB_litr3c_to_PLG, CTDB_litr4c_to_PLG;
	double CTDB_litr1n_to_PLG, CTDB_litr2n_to_PLG, CTDB_litr3n_to_PLG, CTDB_litr4n_to_PLG;
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
	
	/*-------------------------------------------------------*/
	/* 1. as result of the ploughing no plant material above the ground */
	/*-------------------------------------------------------*/

	/* 1.1. CARBON */

	/* leaf, froot, fruit, softstem */
	cf->leafc_to_PLG = cs->leafc * PLGcoeff;
	cf->leafc_storage_to_PLG = cs->leafc_storage * PLGcoeff * storage_MGMmort;
	cf->leafc_transfer_to_PLG = cs->leafc_transfer * PLGcoeff * storage_MGMmort;

	cf->gresp_storage_to_PLG = cs->gresp_storage * PLGcoeff * storage_MGMmort;
	cf->gresp_transfer_to_PLG = cs->gresp_transfer * PLGcoeff * storage_MGMmort;

	cf->frootc_to_PLG = cs->frootc * PLGcoeff;
	cf->frootc_storage_to_PLG = cs->frootc_storage * PLGcoeff * storage_MGMmort;
	cf->frootc_transfer_to_PLG = cs->frootc_transfer * PLGcoeff * storage_MGMmort;

	cf->fruitc_to_PLG = cs->fruitc * PLGcoeff;
	cf->fruitc_storage_to_PLG = cs->fruitc_storage * PLGcoeff * storage_MGMmort;
	cf->fruitc_transfer_to_PLG = cs->fruitc_transfer * PLGcoeff * storage_MGMmort;

	cf->softstemc_to_PLG = cs->softstemc * PLGcoeff;
	cf->softstemc_storage_to_PLG = cs->softstemc_storage * PLGcoeff * storage_MGMmort;
	cf->softstemc_transfer_to_PLG = cs->softstemc_transfer * PLGcoeff * storage_MGMmort;

   /* standing dead biomass */
	STDB_litr1c_to_PLG = cs->STDB_litr1c * PLGcoeff;
	STDB_litr2c_to_PLG = cs->STDB_litr2c * PLGcoeff;
	STDB_litr3c_to_PLG = cs->STDB_litr3c * PLGcoeff;
	STDB_litr4c_to_PLG = cs->STDB_litr4c * PLGcoeff;

	cf->STDBc_to_PLG = STDB_litr1c_to_PLG + STDB_litr2c_to_PLG + STDB_litr3c_to_PLG + STDB_litr4c_to_PLG;

	 /* cut-down dead biomass */
	CTDB_litr1c_to_PLG = cs->CTDB_litr1c * PLGcoeff;
	CTDB_litr2c_to_PLG = cs->CTDB_litr2c * PLGcoeff;
	CTDB_litr3c_to_PLG = cs->CTDB_litr3c * PLGcoeff;
	CTDB_litr4c_to_PLG = cs->CTDB_litr4c * PLGcoeff;
	
	cf->CTDBc_to_PLG = CTDB_litr1c_to_PLG + CTDB_litr2c_to_PLG + CTDB_litr3c_to_PLG + CTDB_litr4c_to_PLG;

	
	/* 1.2. NITROGEN */

	/* leaf, froot, fruit, softstem */
	nf->leafn_to_PLG = ns->leafn * PLGcoeff;
	nf->leafn_storage_to_PLG  = ns->leafn_storage * PLGcoeff * storage_MGMmort;;
	nf->leafn_transfer_to_PLG = ns->leafn_transfer * PLGcoeff * storage_MGMmort;

	nf->frootn_to_PLG = ns->frootn * PLGcoeff;
	nf->frootn_storage_to_PLG = ns->frootn_storage * PLGcoeff * storage_MGMmort;
	nf->frootn_transfer_to_PLG = ns->frootn_transfer * PLGcoeff * storage_MGMmort;

	nf->fruitn_to_PLG = ns->fruitn * PLGcoeff;
	nf->fruitn_storage_to_PLG = ns->fruitn_storage * PLGcoeff * storage_MGMmort;
	nf->fruitn_transfer_to_PLG = ns->fruitn_transfer * PLGcoeff * storage_MGMmort;

	nf->softstemn_to_PLG = ns->softstemn * PLGcoeff;
	nf->softstemn_storage_to_PLG = ns->softstemn_storage * PLGcoeff * storage_MGMmort;
	nf->softstemn_transfer_to_PLG = ns->softstemn_transfer * PLGcoeff * storage_MGMmort;

	/* standing dead biomass */
	STDB_litr1n_to_PLG = ns->STDB_litr1n * PLGcoeff;
	STDB_litr2n_to_PLG = ns->STDB_litr2n * PLGcoeff;
	STDB_litr3n_to_PLG = ns->STDB_litr3n * PLGcoeff;
	STDB_litr4n_to_PLG = ns->STDB_litr4n * PLGcoeff;

	nf->STDBn_to_PLG = STDB_litr1n_to_PLG + STDB_litr2n_to_PLG + STDB_litr3n_to_PLG + STDB_litr4n_to_PLG;

	/* CUT-DOWN dead biomass */
	CTDB_litr1n_to_PLG = ns->CTDB_litr1n * PLGcoeff;
	CTDB_litr2n_to_PLG = ns->CTDB_litr2n * PLGcoeff;
	CTDB_litr3n_to_PLG = ns->CTDB_litr3n * PLGcoeff;
	CTDB_litr4n_to_PLG = ns->CTDB_litr4n * PLGcoeff;
	
	nf->CTDBn_to_PLG = CTDB_litr1n_to_PLG + CTDB_litr2n_to_PLG + CTDB_litr3n_to_PLG + CTDB_litr4n_to_PLG;
   
	/* restranslocated N pool is decreasing also */
	nf->retransn_to_PLG = ns->retransn * PLGcoeff;

	/* 1.3. WATER */
	wf->canopyw_to_PLG = ws->canopyw * PLGcoeff;


	/*-------------------------------------------------------*/
	/* 2. increasing ploughing actual pool */
	/*-------------------------------------------------------*/
	
	/* 2.1 CARBON */
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


	/* 2.1 NITROGEN */
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

	/*-------------------------------------------------------*/
	/* 3. increasing litter content AND decreasing ploughing actual pool content */
	/*-------------------------------------------------------*/
	
	if (PLG->PLG_pool_litr1c > CRIT_PREC)
	{
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
		cf->PLG_to_litr1c = PLG->PLG_pool_litr1c;
		cf->PLG_to_litr2c = PLG->PLG_pool_litr2c;
		cf->PLG_to_litr3c = PLG->PLG_pool_litr3c;
		cf->PLG_to_litr4c = PLG->PLG_pool_litr4c;

		nf->PLG_to_litr1n = PLG->PLG_pool_litr1n;
		nf->PLG_to_litr2n = PLG->PLG_pool_litr2n; 
		nf->PLG_to_litr3n = PLG->PLG_pool_litr3n; 
		nf->PLG_to_litr4n = PLG->PLG_pool_litr4n; 
	
	}

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

	/* leaf, froot, fruit, sofstem, gresp */
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
	
	cs->PLGsnk += cf->fruitc_to_PLG;
	cs->fruitc -= cf->fruitc_to_PLG;
	cs->PLGsnk += cf->fruitc_transfer_to_PLG;
	cs->fruitc_transfer -= cf->fruitc_transfer_to_PLG;
	cs->PLGsnk += cf->fruitc_storage_to_PLG;
	cs->fruitc_storage -= cf->fruitc_storage_to_PLG;

	cs->PLGsnk += cf->softstemc_to_PLG;
	cs->softstemc -= cf->softstemc_to_PLG;
	cs->PLGsnk += cf->softstemc_transfer_to_PLG;
	cs->softstemc_transfer -= cf->softstemc_transfer_to_PLG;
	cs->PLGsnk += cf->softstemc_storage_to_PLG;
	cs->softstemc_storage -= cf->softstemc_storage_to_PLG;

	/*  standing dead biomass */
	cs->PLGsnk += STDB_litr1c_to_PLG;
	cs->STDB_litr1c -= STDB_litr1c_to_PLG;
	cs->PLGsnk += STDB_litr2c_to_PLG;
	cs->STDB_litr2c -= STDB_litr2c_to_PLG;
	cs->PLGsnk += STDB_litr3c_to_PLG;
	cs->STDB_litr3c -= STDB_litr3c_to_PLG;
	cs->PLGsnk += STDB_litr4c_to_PLG;
	cs->STDB_litr4c -=STDB_litr4c_to_PLG;

	cs->SNSCsrc += cf->STDBc_to_PLG;
	cs->STDBc -= cf->STDBc_to_PLG;



	/* CUT-DOWN biomass */
	cs->CTDB_litr1c  -= cs->CTDB_litr1c * PLGcoeff;
	cs->CTDB_litr2c  -= cs->CTDB_litr2c * PLGcoeff;
	cs->CTDB_litr3c  -= cs->CTDB_litr3c * PLGcoeff;
	cs->CTDB_litr4c  -= cs->CTDB_litr4c * PLGcoeff;


	/* litter */
	cs->litr1c += cf->PLG_to_litr1c;
	cs->litr2c += cf->PLG_to_litr2c;
	cs->litr3c += cf->PLG_to_litr3c;
	cs->litr4c += cf->PLG_to_litr4c;
	
	cs->PLGsrc += cf->PLG_to_litr1c + cf->PLG_to_litr2c + cf->PLG_to_litr3c + cf->PLG_to_litr4c;
	
	/* 2. NITROGEN */

	/* leaf, froot, fruit, sofstem, gresp */
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

	ns->PLGsnk += nf->fruitn_to_PLG;
	ns->fruitn -= nf->fruitn_to_PLG;
	ns->PLGsnk += nf->fruitn_transfer_to_PLG;
	ns->fruitn_transfer -= nf->fruitn_transfer_to_PLG;
	ns->PLGsnk += nf->fruitn_storage_to_PLG;
	ns->fruitn_storage -= nf->fruitn_storage_to_PLG;

	ns->PLGsnk += nf->softstemn_to_PLG;
	ns->softstemn -= nf->softstemn_to_PLG;
	ns->PLGsnk += nf->softstemn_transfer_to_PLG;
	ns->softstemn_transfer -= nf->softstemn_transfer_to_PLG;
	ns->PLGsnk += nf->softstemn_storage_to_PLG;
	ns->softstemn_storage -= nf->softstemn_storage_to_PLG;

   
	/* standing dead biomass */
	ns->PLGsnk += STDB_litr1n_to_PLG;
	ns->STDB_litr1n -= STDB_litr1n_to_PLG;
	ns->PLGsnk += STDB_litr2n_to_PLG;
	ns->STDB_litr2n -= STDB_litr2n_to_PLG;
	ns->PLGsnk += STDB_litr3n_to_PLG;
	ns->STDB_litr3n -= STDB_litr3n_to_PLG;
	ns->PLGsnk += STDB_litr4n_to_PLG;
	ns->STDB_litr4n -=STDB_litr4n_to_PLG;

	ns->SNSCsrc += nf->STDBn_to_PLG;
	ns->STDBn -= nf->STDBn_to_PLG;

	 /* CUT-DOWN standing biomass */
	ns->CTDB_litr1n  -= ns->CTDB_litr1n * PLGcoeff;
	ns->CTDB_litr2n  -= ns->CTDB_litr2n * PLGcoeff;
	ns->CTDB_litr3n  -= ns->CTDB_litr3n * PLGcoeff;
	ns->CTDB_litr4n  -= ns->CTDB_litr4n * PLGcoeff;

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
	