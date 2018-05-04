/*
restart_io.c
functions called to copy restart info between restart structure and
active structures

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0.
Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group (NTSG)
School of Forestry, University of Montana
Missoula, MT 59812
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "bgc_struct.h"     /* structure definitions */
#include "bgc_func.h"       /* function prototypes */
#include "bgc_constants.h"

int restart_input(const epconst_struct* epc, wstate_struct* ws, cstate_struct* cs, nstate_struct* ns, epvar_struct* epv, restart_data_struct* restart)
{
	int ok=1;
	int layer;
	
	/* 1. water */
	for (layer =0; layer < N_SOILLAYERS; layer++)
	{ 
		ws->soilw[layer]                  = restart->soilw[layer];
	}

	ws->snoww                             = restart->snoww;
	ws->canopyw                           = restart->canopyw;

	/* 2. carbon and nitrogen plant pools */
	cs->leafc                             = restart->leafc;
	cs->leafc_storage                     = restart->leafc_storage;
	cs->leafc_transfer                    = restart->leafc_transfer;
	cs->frootc                            = restart->frootc;
	cs->frootc_storage                    = restart->frootc_storage;
	cs->frootc_transfer                   = restart->frootc_transfer;
	cs->fruitc                            = restart->fruitc;
	cs->fruitc_storage                    = restart->fruitc_storage;
	cs->fruitc_transfer                   = restart->fruitc_transfer;
	cs->softstemc                         = restart->softstemc;
	cs->softstemc_storage                 = restart->softstemc_storage;
	cs->softstemc_transfer                = restart->softstemc_transfer;

	cs->livestemc                         = restart->livestemc;
	cs->livestemc_storage                 = restart->livestemc_storage;
	cs->livestemc_transfer                = restart->livestemc_transfer;
	cs->deadstemc                         = restart->deadstemc;
	cs->deadstemc_storage                 = restart->deadstemc_storage;
	cs->deadstemc_transfer                = restart->deadstemc_transfer;
	cs->livecrootc                        = restart->livecrootc;
	cs->livecrootc_storage                = restart->livecrootc_storage;
	cs->livecrootc_transfer               = restart->livecrootc_transfer;
	cs->deadcrootc                        = restart->deadcrootc;
	cs->deadcrootc_storage                = restart->deadcrootc_storage;
	cs->deadcrootc_transfer               = restart->deadcrootc_transfer;
	cs->gresp_storage                     = restart->gresp_storage;
	cs->gresp_transfer                    = restart->gresp_transfer;

	cs->cpool                             = restart->cpool;
	
	ns->npool                             = restart->npool;

	ns->retransn			              = restart->retransn;

	/* spinup - normal C and N pool adjustment in order to avoud negative N pools in case of land use change (changing EOC) */
	if (epc->leaf_cn > 0)
	{
		ns->leafn                         = cs->leafc           / epc->leaf_cn;
		ns->leafn_storage                 = cs->leafc_storage   / epc->leaf_cn;
		ns->leafn_transfer                = cs->leafc_transfer  / epc->leaf_cn;
	}
	else
	{
		ns->leafn                         = 0;
		ns->leafn_storage                 = 0;
		ns->leafn_transfer                = 0;
	}

	if (epc->froot_cn)
	{
		ns->frootn                        = cs->frootc          / epc->froot_cn;
		ns->frootn_storage                = cs->frootc_storage  / epc->froot_cn;
		ns->frootn_transfer               = cs->frootc_transfer / epc->froot_cn;
	}
	else
	{
		ns->frootn                        = 0;
		ns->frootn_storage                = 0;
		ns->frootn_transfer               = 0;
	}

	if (epc->fruit_cn > 0)
	{
		ns->fruitn                        = cs->fruitc          / epc->fruit_cn;
		ns->fruitn_storage                = cs->fruitc_storage  / epc->fruit_cn;
		ns->fruitn_transfer               = cs->fruitc_transfer / epc->fruit_cn;
	}
	else
	{
		ns->fruitn                        =	0;
		ns->fruitn_storage                = 0;
		ns->fruitn_transfer               = 0;
	}
	
	if (epc->softstem_cn)
	{
		ns->softstemn                     = cs->softstemc          / epc->softstem_cn;
		ns->softstemn_storage             = cs->softstemc_storage  / epc->softstem_cn;
		ns->softstemn_transfer            = cs->softstemc_transfer / epc->softstem_cn;
	}
	else
	{
		ns->softstemn                     = 0;
		ns->softstemn_storage             = 0;
		ns->softstemn_transfer            = 0;
	}

	if (epc->livewood_cn > 0)
	{
		ns->livestemn                     = cs->livestemc          / epc->livewood_cn;
		ns->livestemn_storage             = cs->livestemc_storage  / epc->livewood_cn;
		ns->livestemn_transfer            = cs->livestemc_transfer / epc->livewood_cn;
		ns->livecrootn                    = cs->livecrootc         / epc->livewood_cn;
		ns->livecrootn_storage            = cs->livecrootc_storage / epc->livewood_cn;
		ns->livecrootn_transfer           = cs->livecrootc_transfer/ epc->livewood_cn;
	}
	else
	{
		ns->livestemn                     = 0;
		ns->livestemn_storage             = 0;
		ns->livestemn_transfer            = 0;
		ns->livecrootn                    = 0;
		ns->livecrootn_storage            = 0;
		ns->livecrootn_transfer           = 0;
	}
	
	if (epc->deadwood_cn > 0)
	{
		ns->deadstemn                     = cs->deadstemc          / epc->deadwood_cn; 
		ns->deadstemn_storage             = cs->deadstemc_storage  / epc->deadwood_cn;
		ns->deadstemn_transfer            = cs->deadstemc_transfer / epc->deadwood_cn;
		ns->deadcrootn                    = cs->deadcrootc         / epc->deadwood_cn;
		ns->deadcrootn_storage            = cs->deadcrootc_storage / epc->deadwood_cn;
		ns->deadcrootn_transfer           = cs->deadcrootc_transfer/ epc->deadwood_cn;
	
	}
	else
	{
		ns->deadstemn                     = 0; 
		ns->deadstemn_storage             = 0;
		ns->deadstemn_transfer            = 0;
		ns->deadcrootn                    = 0;
		ns->deadcrootn_storage            = 0;
		ns->deadcrootn_transfer           = 0;
	
	}

	/* 3. standing dead biomass, cut-down dead biomass and litter pools */
	cs->STDBc_leaf		= restart->STDBc_leaf;
	cs->STDBc_froot		= restart->STDBc_froot;
	cs->STDBc_fruit		= restart->STDBc_fruit;
	cs->STDBc_softstem	= restart->STDBc_softstem;
	cs->STDBc_transfer	= restart->STDBc_transfer;
	cs->CTDBc_leaf		= restart->CTDBc_leaf;
	cs->CTDBc_froot		= restart->CTDBc_froot;
	cs->CTDBc_fruit		= restart->CTDBc_fruit;
	cs->CTDBc_softstem	= restart->CTDBc_softstem;
	cs->CTDBc_transfer	= restart->CTDBc_transfer;
	cs->CTDBc_cstem		= restart->CTDBc_cstem;
	cs->CTDBc_croot		= restart->CTDBc_croot;
	
	ns->STDBn_leaf		= restart->STDBn_leaf;
	ns->STDBn_froot		= restart->STDBn_froot;
	ns->STDBn_fruit		= restart->STDBn_fruit;
	ns->STDBn_softstem	= restart->STDBn_softstem;
	ns->STDBn_transfer	= restart->STDBn_transfer;
	ns->CTDBn_leaf		= restart->CTDBn_leaf;
	ns->CTDBn_froot		= restart->CTDBn_froot;
	ns->CTDBn_fruit		= restart->CTDBn_fruit;
	ns->CTDBn_softstem	= restart->CTDBn_softstem;
	ns->CTDBn_transfer	= restart->CTDBn_transfer;
	ns->CTDBn_cstem		= restart->CTDBn_cstem;
	ns->CTDBn_croot		= restart->CTDBn_croot;

	/* 4. litter*/
	for (layer=0; layer < N_SOILLAYERS; layer++)
	{
		
 		cs->litr1c[layer]                 = restart->litr1c[layer];
		cs->litr2c[layer]                 = restart->litr2c[layer];
		cs->litr3c[layer]                 = restart->litr3c[layer];
		cs->litr4c[layer]                 = restart->litr4c[layer];
		cs->soil1c[layer]                 = restart->soil1c[layer];
		cs->soil2c[layer]                 = restart->soil2c[layer];
		cs->soil3c[layer]                 = restart->soil3c[layer];
		cs->soil4c[layer]                 = restart->soil4c[layer];
		cs->cwdc[layer]                   = restart->cwdc[layer];

 		ns->litr1n[layer]                 = restart->litr1n[layer];
		ns->litr2n[layer]                 = restart->litr2n[layer];
		ns->litr3n[layer]                 = restart->litr3n[layer];
		ns->litr4n[layer]                 = restart->litr4n[layer];
		ns->soil1n[layer]                 = restart->soil1n[layer];
		ns->soil2n[layer]                 = restart->soil2n[layer];
		ns->soil3n[layer]                 = restart->soil3n[layer];
		ns->soil4n[layer]                 = restart->soil4n[layer];
		ns->cwdn[layer]                   = restart->cwdn[layer];
		
		ns->sminNH4[layer]      = restart->sminNH4[layer];
		ns->sminNO3[layer]      = restart->sminNO3[layer];
	}

	/* 4. ecophysiological variables */
	epv->annmax_leafc                     = restart->annmax_leafc;
	epv->annmax_frootc                    = restart->annmax_frootc;
	epv->annmax_fruitc                    = restart->annmax_fruitc;
	epv->annmax_softstemc                 = restart->annmax_softstemc;
	epv->annmax_livestemc                 = restart->annmax_livestemc;
	epv->annmax_livecrootc                = restart->annmax_livecrootc;
 
	
	return(!ok);
}

int restart_output(const wstate_struct* ws, const cstate_struct* cs, const nstate_struct* ns, const epvar_struct* epv, restart_data_struct* restart)
{
	int ok=1;
	int layer;

	/* 1. soil water */
	for (layer =0; layer < N_SOILLAYERS; layer++)
	{ 
		restart->soilw[layer]  				  = ws->soilw[layer];
	}
	restart->snoww  						  = ws->snoww;
	restart->canopyw						  = ws->canopyw;

	/* 2. plant carbon and nitrogen pools */

	restart->leafc  						  = cs->leafc;
	restart->leafc_storage  				  = cs->leafc_storage;
	restart->leafc_transfer 				  = cs->leafc_transfer;
	restart->frootc 						  = cs->frootc;
	restart->frootc_storage 				  = cs->frootc_storage;
	restart->frootc_transfer				  = cs->frootc_transfer;
	restart->fruitc 						  = cs->fruitc;
	restart->fruitc_storage 				  = cs->fruitc_storage;
	restart->fruitc_transfer				  = cs->fruitc_transfer;
	restart->softstemc 						  = cs->softstemc;
	restart->softstemc_storage 				  = cs->softstemc_storage;
	restart->softstemc_transfer				  = cs->softstemc_transfer;
	restart->livestemc  					  = cs->livestemc;
	restart->livestemc_storage  			  = cs->livestemc_storage;
	restart->livestemc_transfer 			  = cs->livestemc_transfer;
	restart->deadstemc  					  = cs->deadstemc;
	restart->deadstemc_storage  			  = cs->deadstemc_storage;
	restart->deadstemc_transfer 			  = cs->deadstemc_transfer;
	restart->livecrootc 					  = cs->livecrootc;
	restart->livecrootc_storage 			  = cs->livecrootc_storage;
	restart->livecrootc_transfer			  = cs->livecrootc_transfer;
	restart->deadcrootc 					  = cs->deadcrootc;
	restart->deadcrootc_storage 			  = cs->deadcrootc_storage;
	restart->deadcrootc_transfer			  = cs->deadcrootc_transfer;
	restart->gresp_storage  				  = cs->gresp_storage;
	restart->gresp_transfer 				  = cs->gresp_transfer;
	restart->cpool  						  = cs->cpool;

	restart->leafn  						  = ns->leafn;
	restart->leafn_storage  				  = ns->leafn_storage;
	restart->leafn_transfer 				  = ns->leafn_transfer;
	restart->frootn 						  = ns->frootn;
	restart->frootn_storage 				  = ns->frootn_storage;
	restart->frootn_transfer				  = ns->frootn_transfer;
	restart->fruitn  						  = ns->fruitn;
	restart->fruitn_storage  				  = ns->fruitn_storage;
	restart->fruitn_transfer 				  = ns->fruitn_transfer;
	restart->softstemn  					  = ns->softstemn;
	restart->softstemn_storage  			  = ns->softstemn_storage;
	restart->softstemn_transfer 			  = ns->softstemn_transfer;
	restart->livestemn  					  = ns->livestemn;
	restart->livestemn_storage  			  = ns->livestemn_storage;
	restart->livestemn_transfer 			  = ns->livestemn_transfer;
	restart->deadstemn  					  = ns->deadstemn;
	restart->deadstemn_storage  			  = ns->deadstemn_storage;
	restart->deadstemn_transfer 			  = ns->deadstemn_transfer;
	restart->livecrootn 					  = ns->livecrootn;
	restart->livecrootn_storage 			  = ns->livecrootn_storage;
	restart->livecrootn_transfer			  = ns->livecrootn_transfer;
	restart->deadcrootn 					  = ns->deadcrootn;
	restart->deadcrootn_storage 			  = ns->deadcrootn_storage;
	restart->deadcrootn_transfer			  = ns->deadcrootn_transfer;

	restart->retransn           			  = ns->retransn;
	restart->npool  						  = ns->npool;

	/* 2. standing dead biomass, cut-down dead biomass*/
	restart->STDBc_leaf			= cs->STDBc_leaf;
	restart->STDBc_froot		= cs->STDBc_froot;
	restart->STDBc_fruit		= cs->STDBc_fruit;
	restart->STDBc_softstem		= cs->STDBc_softstem;
	restart->STDBc_transfer		= cs->STDBc_transfer;
	restart->CTDBc_leaf			= cs->CTDBc_leaf;
	restart->CTDBc_froot		= cs->CTDBc_froot;
	restart->CTDBc_fruit		= cs->CTDBc_fruit;
	restart->CTDBc_softstem		= cs->CTDBc_softstem;
	restart->CTDBc_transfer		= cs->CTDBc_transfer;
	restart->CTDBc_cstem		= cs->CTDBc_cstem;
	restart->CTDBc_croot		= cs->CTDBc_croot;

	restart->STDBn_leaf			= ns->STDBn_leaf;
	restart->STDBn_froot		= ns->STDBn_froot;
	restart->STDBn_fruit		= ns->STDBn_fruit;
	restart->STDBn_softstem		= ns->STDBn_softstem;
	restart->STDBn_transfer		= ns->STDBn_transfer;
	restart->CTDBn_leaf			= ns->CTDBn_leaf;
	restart->CTDBn_froot		= ns->CTDBn_froot;
	restart->CTDBn_fruit		= ns->CTDBn_fruit;
	restart->CTDBn_softstem		= ns->CTDBn_softstem;
	restart->CTDBn_transfer		= ns->CTDBn_transfer;
	restart->CTDBn_cstem		= ns->CTDBn_cstem;
	restart->CTDBn_croot		= ns->CTDBn_croot;

	/* 3. multilayer litter and soil */
	for (layer=0; layer < N_SOILLAYERS; layer++)
	{
		restart->soil1c[layer] 	= cs->soil1c[layer];
		restart->soil2c[layer]  = cs->soil2c[layer];
		restart->soil3c[layer]  = cs->soil3c[layer];
		restart->soil4c[layer]  = cs->soil4c[layer];
		restart->litr1c[layer]  = cs->litr1c[layer];
		restart->litr2c[layer]  = cs->litr2c[layer];
		restart->litr3c[layer]  = cs->litr3c[layer];
		restart->litr4c[layer]  = cs->litr4c[layer];
		restart->cwdc[layer]	= cs->cwdc[layer];

		restart->soil1n[layer] 	= ns->soil1n[layer];
		restart->soil2n[layer]  = ns->soil2n[layer];
		restart->soil3n[layer]  = ns->soil3n[layer];
		restart->soil4n[layer]  = ns->soil4n[layer];
		restart->litr1n[layer]  = ns->litr1n[layer];
		restart->litr2n[layer]  = ns->litr2n[layer];
		restart->litr3n[layer]  = ns->litr3n[layer];
		restart->litr4n[layer]  = ns->litr4n[layer];
		restart->cwdn[layer]	= ns->cwdn[layer];

		restart->sminNH4[layer]  = ns->sminNH4[layer];
		restart->sminNO3[layer]  = ns->sminNO3[layer];
	}
	
	/* 4. ecophysiological variables */
	
	
	restart->annmax_leafc					  = epv->annmax_leafc;
	restart->annmax_frootc  				  = epv->annmax_frootc;
	restart->annmax_fruitc  				  = epv->annmax_fruitc;
	restart->annmax_softstemc  				  = epv->annmax_softstemc;
	restart->annmax_livestemc				  = epv->annmax_livestemc;
	restart->annmax_livecrootc  			  = epv->annmax_livecrootc;

	
	
	return(!ok);
}
