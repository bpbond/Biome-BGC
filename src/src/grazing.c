/* 
grazing.c
grazing  - decrease the plant material (leafc, leafn, canopy water) and increase N-content (mature)

method: Vuichard et al, 2007
NOTE: LSU: livestock unit = unit used to compare or aggregate different species and it is equivalnet to the liveweight of an average cattle (1 adult cattle = 1 LSU)

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

int grazing(const control_struct* ctrl, const epconst_struct* epc, grazing_struct* GRZ, 
			cflux_struct* cf, nflux_struct* nf, wflux_struct* wf,  cstate_struct* cs, nstate_struct* ns, wstate_struct* ws)
{

	/* grazing parameters */
	int ny;
	double DMintake, stocking_rate,weight_LSU;			
	double litr1c_STDB_to_GRZ, litr2c_STDB_to_GRZ, litr3c_STDB_to_GRZ, litr4c_STDB_to_GRZ;
	double litr1n_STDB_to_GRZ, litr2n_STDB_to_GRZ, litr3n_STDB_to_GRZ, litr4n_STDB_to_GRZ;

	/* local variables */
	double EFman_N2O, Nexrate, EFman_CH4, EFfer_CH4;
	double prop_DMintake2excr;	
	double DM_Ccontent;				    
	double EXCR_Ccontent_array;				
	double EXCR_Ncontent_array;				
    double prop_excr2litter;				/* proportion of excrement return to litter */
	double GRZcoeff;						/* coefficient determining decrease of plant material caused by grazing  */
	double befgrazing_leafc = 0;			/* value of leafc before grazing */
	double aftergrazing_leafc = 0;			/* value of leafc before grazing */
	double daily_excr_prod = 0;				/* daily excrement production */	
	double daily_C_loss = 0;				/* daily carbon loss due to grazing */
	double Cplus_from_excrement = 0;		/* daily carbon plus from excrement */
	double Nplus_from_excrement = 0;        /* daily nitrogen plus from excrement */

	int mgmd = GRZ->mgmd;
	
	int ok=1;

	/* test variable */
	double storage_MGMmort=epc->storage_MGMmort;

	/* Hidy 2015 - fraction of total annual nitrogen excretion for pasture management system*/
	double MS_N2O = 0.015;

	/* yearly varied or constant management parameters */
	if(GRZ->GRZ_flag == 2)
	{
		ny = ctrl->simyr;
	}
	else ny=0;



	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                                    CALCULATING FLUXES 
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


	if (mgmd >= 0) 
	{
		GRZ->trampleff_act  = GRZ->trampling_effect[mgmd][ny];
		DMintake            = GRZ->DMintake_array[mgmd][ny];						 /*  unit: kgDM/LSU (DM:dry matter)*/
		stocking_rate       = GRZ->stocking_rate_array[mgmd][ny]/10000;				 /*  unit: LSU/ha -> new unit: LSU/m2 */
		weight_LSU			= GRZ->weight_LSU[mgmd][ny];

		prop_DMintake2excr  = GRZ->prop_DMintake2excr_array[mgmd][ny] / 100.;	     /* from proporiton(%) to ratio(number) */
		DM_Ccontent         = GRZ->DM_Ccontent_array[mgmd][ny] / 100.;				 /* from proporiton(%) to ratio(number) */
		EXCR_Ccontent_array = GRZ->EXCR_Ccontent_array[mgmd][ny] / 100.;			 /* from proporiton(%) to ratio(number) */
		EXCR_Ncontent_array = GRZ->EXCR_Ncontent_array[mgmd][ny] / 100.;			 /* from proporiton(%) to ratio(number) */
		prop_excr2litter    = GRZ->prop_excr2litter_array[mgmd][ny] / 100;

		Nexrate   = GRZ->Nexrate[mgmd][ny];
		EFman_N2O = GRZ->EFman_N2O[mgmd][ny];
		EFman_CH4 = GRZ->EFman_CH4[mgmd][ny]/NDAY_OF_YEAR;;
		EFfer_CH4 = GRZ->EFfer_CH4[mgmd][ny]/NDAY_OF_YEAR;;

		
		/* daily total ingested carbon per m2 from daily ingested drymatter and carbon content of drymatter and stocking rate
						[kgC/m2 = kgDM/LSU * (kgC/kgDM) * (LSU/m2)] */	
		daily_C_loss = (DMintake * DM_Ccontent) * stocking_rate;	
	
		
		/* effect of grazing: decrease of leafc and increase of soilc and soiln (manure)*/
		befgrazing_leafc = cs->leafc;
		
		if (befgrazing_leafc - daily_C_loss > 0)
		{
			
			aftergrazing_leafc = befgrazing_leafc - daily_C_loss;
			daily_excr_prod = (daily_C_loss/DM_Ccontent) * prop_DMintake2excr;/* kg manure/m2/day -> kgC/m2/day */
		
			GRZcoeff  = 1-aftergrazing_leafc/befgrazing_leafc;
			GRZcoeff = GRZcoeff;
			GRZcoeff = GRZcoeff;
		}
		else
		{
			GRZcoeff  = 0.0;
			daily_excr_prod = 0;
			prop_excr2litter = 0;
		
			if (ctrl->onscreen) printf("not enough grass for grazing on yday: %i\n", ctrl->yday);

		}			
	}
	else 
	{
		GRZcoeff = 0.0;
		daily_excr_prod = 0;
		prop_excr2litter = 0;
		EXCR_Ccontent_array = 0;
		EXCR_Ncontent_array = 0;
		stocking_rate = 0;
		EFman_N2O =  0;
		EFman_CH4 =  0;
		EFfer_CH4 =  0;
		Nexrate = 0;
		weight_LSU=0;
	}
	
	/* daily manure production per m2 (return to the litter) from daily total ingested dry matter and litter_return_ratio and its C and N content
					[kgMANURE = (kgDM/LSU) * (LSU/m2) * (%)] */

	Cplus_from_excrement = daily_excr_prod * EXCR_Ccontent_array;
	Nplus_from_excrement = daily_excr_prod * EXCR_Ncontent_array;

	/* CARBON */
	cf->leafc_to_GRZ          = cs->leafc * GRZcoeff;
	cf->leafc_transfer_to_GRZ = cs->leafc_transfer * GRZcoeff * storage_MGMmort;
	cf->leafc_storage_to_GRZ  = cs->leafc_storage * GRZcoeff * storage_MGMmort;
	
    /* fruit simulation - Hidy 2013. */
	cf->fruitc_to_GRZ          = cs->fruitc * GRZcoeff;
	cf->fruitc_transfer_to_GRZ = cs->fruitc_transfer * GRZcoeff * storage_MGMmort;
	cf->fruitc_storage_to_GRZ  = cs->fruitc_storage * GRZcoeff * storage_MGMmort;

	 /* softstem simulation - Hidy 2013. */
	cf->softstemc_to_GRZ          = cs->softstemc * GRZcoeff;
	cf->softstemc_transfer_to_GRZ = cs->softstemc_transfer * GRZcoeff * storage_MGMmort;
	cf->softstemc_storage_to_GRZ  = cs->softstemc_storage * GRZcoeff * storage_MGMmort;
		
	cf->gresp_transfer_to_GRZ = cs->gresp_transfer * GRZcoeff * storage_MGMmort;
	cf->gresp_storage_to_GRZ  = cs->gresp_storage * GRZcoeff * storage_MGMmort;

	/* standing dead biome */
	litr1c_STDB_to_GRZ = cs->litr1c_STDB * GRZcoeff;
	litr2c_STDB_to_GRZ = cs->litr2c_STDB * GRZcoeff;
	litr3c_STDB_to_GRZ = cs->litr3c_STDB * GRZcoeff;
	litr4c_STDB_to_GRZ = cs->litr4c_STDB * GRZcoeff;

	cf->STDBc_to_GRZ = litr1c_STDB_to_GRZ + litr2c_STDB_to_GRZ + litr3c_STDB_to_GRZ + litr4c_STDB_to_GRZ;


	/* CARBON */
	nf->leafn_to_GRZ          = ns->leafn * GRZcoeff;
	nf->leafn_transfer_to_GRZ = ns->leafn_transfer * GRZcoeff * storage_MGMmort;
	nf->leafn_storage_to_GRZ  = ns->leafn_storage * GRZcoeff * storage_MGMmort;

	/* fruit simulation - Hidy 2013. */
	nf->fruitn_to_GRZ          = ns->fruitn * GRZcoeff;
	nf->fruitn_transfer_to_GRZ = ns->fruitn_transfer * GRZcoeff * storage_MGMmort;
	nf->fruitn_storage_to_GRZ  = ns->fruitn_storage * GRZcoeff * storage_MGMmort;

	/* softstem simulation - Hidy 2013. */
	nf->softstemn_to_GRZ          = ns->softstemn * GRZcoeff;
	nf->softstemn_transfer_to_GRZ = ns->softstemn_transfer * GRZcoeff * storage_MGMmort;
	nf->softstemn_storage_to_GRZ  = ns->softstemn_storage * GRZcoeff * storage_MGMmort;
	
	/* standing dead biome */
	litr1n_STDB_to_GRZ = ns->litr1n_STDB * GRZcoeff;
	litr2n_STDB_to_GRZ = ns->litr2n_STDB * GRZcoeff;
	litr3n_STDB_to_GRZ = ns->litr3n_STDB * GRZcoeff;
	litr4n_STDB_to_GRZ = ns->litr4n_STDB * GRZcoeff;

	nf->STDBn_to_GRZ = litr1n_STDB_to_GRZ + litr2n_STDB_to_GRZ + litr3n_STDB_to_GRZ + litr4n_STDB_to_GRZ;
   
	/* restranslocated N pool is decreasing also */
	nf->retransn_to_GRZ        = ns->retransn * GRZcoeff * storage_MGMmort;
	 
	wf->canopyw_to_GRZ = ws->canopyw * GRZcoeff;

	/* if grazing manure production is taken account (plant material goes into the litter pool)
	   - gain to ecosystem is loss for "atmosphere" - negatice sign*/	

	/* if grazing manure production is taken account (plant material goes into the litter pool) - gain to ecosystem is loss for "atmosphere" - negatice sign*/	
	cf->GRZ_to_litr1c = (Cplus_from_excrement) * epc->leaflitr_flab * prop_excr2litter;
	cf->GRZ_to_litr2c = (Cplus_from_excrement) * epc->leaflitr_fucel * prop_excr2litter;
	cf->GRZ_to_litr3c = (Cplus_from_excrement) * epc->leaflitr_fscel * prop_excr2litter;
	cf->GRZ_to_litr4c = (Cplus_from_excrement) * epc->leaflitr_flig * prop_excr2litter;

	nf->GRZ_to_litr1n = (Nplus_from_excrement) * epc->leaflitr_flab * prop_excr2litter;  
	nf->GRZ_to_litr2n = (Nplus_from_excrement) * epc->leaflitr_fucel * prop_excr2litter; 
	nf->GRZ_to_litr3n = (Nplus_from_excrement) * epc->leaflitr_fscel * prop_excr2litter; 
	nf->GRZ_to_litr4n = (Nplus_from_excrement) * epc->leaflitr_flig * prop_excr2litter;

	
	/* !!!!!!!!CH4 and N2O emissions !!!!!!!!!!*/ 

	// dimension: db animal/m2 * kgN/(kg animal * day) * kg animal/db animal = mgN/m2/day
	nf->N2O_flux_GRZ     = stocking_rate * Nexrate * weight_LSU/1000 *  EFman_N2O * MS_N2O * 1e+6;

	// dimension: kgCH4/head/day * head / m2 * (C/CH4) * mg/kg = mgC/m2/day
	cf->CH4_flux_MANURE  = EFman_CH4  * stocking_rate * 12./16. * 1e+6;  
	cf->CH4_flux_FERMENT = EFfer_CH4  * stocking_rate * 12./16. * 1e+6;  


	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                                    STATE UPDATE 
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/ 

	/* 1. carbon */	
	cs->GRZsnk += cf->leafc_to_GRZ;
	cs->leafc -= cf->leafc_to_GRZ;
	cs->GRZsnk += cf->leafc_transfer_to_GRZ;
	cs->leafc_transfer -= cf->leafc_transfer_to_GRZ;
	cs->GRZsnk += cf->leafc_storage_to_GRZ;
	cs->leafc_storage -= cf->leafc_storage_to_GRZ;
	/* fruit simulation - Hidy 2013. */
	cs->GRZsnk += cf->fruitc_to_GRZ;
	cs->fruitc -= cf->fruitc_to_GRZ;
	cs->GRZsnk += cf->fruitc_transfer_to_GRZ;
	cs->fruitc_transfer -= cf->fruitc_transfer_to_GRZ;
	cs->GRZsnk += cf->fruitc_storage_to_GRZ;
	cs->fruitc_storage -= cf->fruitc_storage_to_GRZ;
	/* softstem simulation - Hidy 2013. */
	cs->GRZsnk += cf->softstemc_to_GRZ;
	cs->softstemc -= cf->softstemc_to_GRZ;
	cs->GRZsnk += cf->softstemc_transfer_to_GRZ;
	cs->softstemc_transfer -= cf->softstemc_transfer_to_GRZ;
	cs->GRZsnk += cf->softstemc_storage_to_GRZ;
	cs->softstemc_storage -= cf->softstemc_storage_to_GRZ;

	cs->GRZsnk += cf->gresp_transfer_to_GRZ;

	/* dead standing biomass */
	cs->GRZsnk += litr1c_STDB_to_GRZ;
	cs->litr1c_STDB -= litr1c_STDB_to_GRZ;
	cs->GRZsnk += litr2c_STDB_to_GRZ;
	cs->litr2c_STDB -= litr2c_STDB_to_GRZ;
	cs->GRZsnk += litr3c_STDB_to_GRZ;
	cs->litr3c_STDB -= litr3c_STDB_to_GRZ;
	cs->GRZsnk += litr4c_STDB_to_GRZ;
	cs->litr4c_STDB -=litr4c_STDB_to_GRZ;

	cs->SNSCsrc += cf->STDBc_to_GRZ;
	cs->STDBc -= cf->STDBc_to_GRZ;


	cs->gresp_transfer -= cf->gresp_transfer_to_GRZ;
	cs->GRZsnk += cf->gresp_storage_to_GRZ;
	cs->gresp_storage -= cf->gresp_storage_to_GRZ;
	
	cs->litr1c += cf->GRZ_to_litr1c;
	cs->litr2c += cf->GRZ_to_litr2c;
	cs->litr3c += cf->GRZ_to_litr3c;
	cs->litr4c += cf->GRZ_to_litr4c;
	
	cs->GRZsrc += cf->GRZ_to_litr1c + cf->GRZ_to_litr2c + cf->GRZ_to_litr3c + cf->GRZ_to_litr4c;

	/* 2. nitrogen */
	ns->GRZsnk   += nf->leafn_to_GRZ;
	ns->leafn    -= nf->leafn_to_GRZ;
	ns->GRZsnk   += nf->leafn_transfer_to_GRZ;
	ns->leafn_transfer -= nf->leafn_transfer_to_GRZ;
	ns->GRZsnk   += nf->leafn_storage_to_GRZ;
	ns->leafn_storage -= nf->leafn_storage_to_GRZ;
	ns->GRZsnk   += nf->retransn_to_GRZ;
	ns->retransn -= nf->retransn_to_GRZ;
	/* fruit simulation - Hidy 2013. */
	ns->GRZsnk += nf->fruitn_to_GRZ;
	ns->fruitn -= nf->fruitn_to_GRZ;
	ns->GRZsnk += nf->fruitn_transfer_to_GRZ;
	ns->fruitn_transfer -= nf->fruitn_transfer_to_GRZ;
	ns->GRZsnk += nf->fruitn_storage_to_GRZ;
	ns->fruitn_storage -= nf->fruitn_storage_to_GRZ;

	/* softstem simulation - Hidy 2013. */
	ns->GRZsnk += nf->softstemn_to_GRZ;
	ns->softstemn -= nf->softstemn_to_GRZ;
	ns->GRZsnk += nf->softstemn_transfer_to_GRZ;
	ns->softstemn_transfer -= nf->softstemn_transfer_to_GRZ;
	ns->GRZsnk += nf->softstemn_storage_to_GRZ;
	ns->softstemn_storage -= nf->softstemn_storage_to_GRZ;

/* dead standing biomass */
	ns->GRZsnk += litr1n_STDB_to_GRZ;
	ns->litr1n_STDB -= litr1n_STDB_to_GRZ;
	ns->GRZsnk += litr2n_STDB_to_GRZ;
	ns->litr2n_STDB -= litr2n_STDB_to_GRZ;
	ns->GRZsnk += litr3n_STDB_to_GRZ;
	ns->litr3n_STDB -= litr3n_STDB_to_GRZ;
	ns->GRZsnk += litr4n_STDB_to_GRZ;
	ns->litr4n_STDB -=litr4n_STDB_to_GRZ;

	ns->SNSCsrc += nf->STDBn_to_GRZ;
	ns->STDBn -= nf->STDBn_to_GRZ;


	ns->litr1n += nf->GRZ_to_litr1n;
	ns->litr2n += nf->GRZ_to_litr2n;
	ns->litr3n += nf->GRZ_to_litr3n;
	ns->litr4n += nf->GRZ_to_litr4n;
	
	ns->GRZsrc += nf->GRZ_to_litr1n + nf->GRZ_to_litr2n + nf->GRZ_to_litr3n + nf->GRZ_to_litr4n;

	/* 3. water */
	ws->canopyw_GRZsnk += wf->canopyw_to_GRZ;
	ws->canopyw -= wf->canopyw_to_GRZ;

		

	
   return (!ok);
}
	