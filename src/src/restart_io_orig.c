/*
restart_io.c
functions called to copy restart info between restart structure and
active structures

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
BBGC MuSo v4
Copyright 2000, Peter E. Thornton
Numerical Terradynamics Simulation Group
Copyright 2014, D. Hidy
Hungarian Academy of Sciences
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

int restart_input(const control_struct* ctrl, const epconst_struct* epc, wstate_struct* ws, cstate_struct* cs, nstate_struct* ns,
				  epvar_struct* epv, int* metyr, restart_data_struct* restart)
{
	int ok=1;
	int layer;
	
	/* Hidy 2010 - multilayer soil */
	for (layer =0; layer < N_SOILLAYERS; layer++)
	{ 
		ws->soilw[layer]                  = restart->soilw[layer];
	}

	ws->snoww                             = restart->snoww;
	ws->canopyw                           = restart->canopyw;

	cs->leafc                             = restart->leafc;
	cs->leafc_storage                     = restart->leafc_storage;
	cs->leafc_transfer                    = restart->leafc_transfer;
	cs->frootc                            = restart->frootc;
	cs->frootc_storage                    = restart->frootc_storage;
	cs->frootc_transfer                   = restart->frootc_transfer;
	/* fruit simulation - Hidy 2013. */
	cs->fruitc                            = restart->fruitc;
	cs->fruitc_storage                    = restart->fruitc_storage;
	cs->fruitc_transfer                   = restart->fruitc_transfer;
	/* softstem simulation - Hidy 2013. */
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
	cs->cwdc                              = restart->cwdc;
	cs->litr1c                            = restart->litr1c;
	cs->litr2c                            = restart->litr2c;
	cs->litr3c                            = restart->litr3c;
	cs->litr4c                            = restart->litr4c;
	cs->soil1c                            = restart->soil1c;
	cs->soil2c                            = restart->soil2c;
	cs->soil3c                            = restart->soil3c;
	cs->soil4c                            = restart->soil4c;
	cs->cpool                             = restart->cpool;
	/* Hidy 2013 - temporary senescence pool */
	cs->litr1c_STDB				  = restart->litr1c_STDB;
	cs->litr2c_STDB				  = restart->litr2c_STDB;
	cs->litr3c_STDB				  = restart->litr3c_STDB;
	cs->litr4c_STDB				  = restart->litr4c_STDB;
	
	ns->leafn                             = restart->leafn;
	ns->leafn_storage                     = restart->leafn_storage;
	ns->leafn_transfer                    = restart->leafn_transfer;
	ns->frootn                            = restart->frootn;
	ns->frootn_storage                    = restart->frootn_storage;
	ns->frootn_transfer                   = restart->frootn_transfer;
	/* fruit simulation - Hidy 2013. */
	ns->fruitn                            = restart->fruitn;
	ns->fruitn_storage                    = restart->fruitn_storage;
	ns->fruitn_transfer                   = restart->fruitn_transfer;
	/* softstem simulation - Hidy 2015. */
	ns->softstemn                         = restart->softstemn;
	ns->softstemn_storage                 = restart->softstemn_storage;
	ns->softstemn_transfer                = restart->softstemn_transfer;

	ns->livestemn                         = restart->livestemn;
	ns->livestemn_storage                 = restart->livestemn_storage;
	ns->livestemn_transfer                = restart->livestemn_transfer;
	ns->deadstemn                         = restart->deadstemn;
	ns->deadstemn_storage                 = restart->deadstemn_storage;
	ns->deadstemn_transfer                = restart->deadstemn_transfer;
	ns->livecrootn                        = restart->livecrootn;
	ns->livecrootn_storage                = restart->livecrootn_storage;
	ns->livecrootn_transfer               = restart->livecrootn_transfer;
	ns->deadcrootn                        = restart->deadcrootn;
	ns->deadcrootn_storage                = restart->deadcrootn_storage;
	ns->deadcrootn_transfer               = restart->deadcrootn_transfer;
	ns->cwdn                              = restart->cwdn;
	ns->litr1n                            = restart->litr1n;
	ns->litr2n                            = restart->litr2n;
	ns->litr3n                            = restart->litr3n;
	ns->litr4n                            = restart->litr4n;
	ns->soil1n                            = restart->soil1n;
	ns->soil2n                            = restart->soil2n;
	ns->soil3n                            = restart->soil3n;
	ns->soil4n                            = restart->soil4n;
	/* Hidy 2013 - temporary senescence pool */
	ns->litr1n_STDB				  = restart->litr1n_STDB;
	ns->litr2n_STDB				  = restart->litr2n_STDB;
	ns->litr3n_STDB				  = restart->litr3n_STDB;
	ns->litr4n_STDB				  = restart->litr4n_STDB;

	/*-----------------------------*/
	/* Hidy 2011 - multilayer soil */
	
	for (layer =0; layer < N_SOILLAYERS; layer++)
	{ 
		ns->sminn[layer]                  = restart->sminn[layer];
	}
	/*-----------------------------*/
	
	ns->retransn                          = restart->retransn;
	ns->npool                             = restart->npool;

	epv->day_leafc_litfall_increment      = restart->day_leafc_litfall_increment;
	epv->day_frootc_litfall_increment     = restart->day_frootc_litfall_increment;
	/* fruit simulation - Hidy 2013. */
	epv->day_fruitc_litfall_increment     = restart->day_fruitc_litfall_increment;
	/* softstem simulation - Hidy 2015. */
	epv->day_softstemc_litfall_increment     = restart->day_softstemc_litfall_increment;
	
	epv->day_livestemc_turnover_increment = restart->day_livestemc_turnover_increment;
	epv->day_livecrootc_turnover_increment= restart->day_livecrootc_turnover_increment;
	epv->annmax_leafc                     = restart->annmax_leafc;
	epv->annmax_frootc                    = restart->annmax_frootc;
	/* fruit simulation - Hidy 2013. */
	epv->annmax_fruitc                    = restart->annmax_fruitc;
	/* softstem simulation - Hidy 2015. */
	epv->annmax_softstemc                 = restart->annmax_softstemc;

	epv->annmax_livestemc                 = restart->annmax_livestemc;
	epv->annmax_livecrootc                = restart->annmax_livecrootc;
	epv->dsr                              = restart->dsr;
	
	if (ctrl->keep_metyr) *metyr          = restart->metyr; 
	else                  *metyr          = 0;
	
	return(!ok);
}

int restart_output(wstate_struct* ws,cstate_struct* cs, nstate_struct* ns, epvar_struct* epv, int metyr, 
				   restart_data_struct* restart)
{
	int ok=1;
	int layer;

	/* Hidy 2010 - multilayer soil */
	for (layer =0; layer < N_SOILLAYERS; layer++)
	{ 
		restart->soilw[layer]  				  = ws->soilw[layer];
	}
	

	restart->snoww  						  = ws->snoww;
	restart->canopyw						  = ws->canopyw;

	restart->leafc  						  = cs->leafc;
	restart->leafc_storage  				  = cs->leafc_storage;
	restart->leafc_transfer 				  = cs->leafc_transfer;
	restart->frootc 						  = cs->frootc;
	restart->frootc_storage 				  = cs->frootc_storage;
	restart->frootc_transfer				  = cs->frootc_transfer;
	/* fruit simulation - Hidy 2013. */
	restart->fruitc 						  = cs->fruitc;
	restart->fruitc_storage 				  = cs->fruitc_storage;
	restart->fruitc_transfer				  = cs->fruitc_transfer;
	/* softstem simulation - Hidy 2013. */
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
	restart->cwdc							  = cs->cwdc;
	restart->litr1c 						  = cs->litr1c;
	restart->litr2c 						  = cs->litr2c;
	restart->litr3c 						  = cs->litr3c;
	restart->litr4c 						  = cs->litr4c;
	/* Hidy 2013 - temporary senescence pool */
	restart->litr1c_STDB				  = cs->litr1c_STDB;
	restart->litr2c_STDB				  = cs->litr2c_STDB;
	restart->litr3c_STDB				  = cs->litr3c_STDB;
	restart->litr4c_STDB				  = cs->litr4c_STDB;
	restart->soil1c 						  = cs->soil1c;
	restart->soil2c 						  = cs->soil2c;
	restart->soil3c 						  = cs->soil3c;
	restart->soil4c 						  = cs->soil4c;
	restart->cpool  						  = cs->cpool;

	restart->leafn  						  = ns->leafn;
	restart->leafn_storage  				  = ns->leafn_storage;
	restart->leafn_transfer 				  = ns->leafn_transfer;
	restart->frootn 						  = ns->frootn;
	restart->frootn_storage 				  = ns->frootn_storage;
	restart->frootn_transfer				  = ns->frootn_transfer;
	/* fruit simulation - Hidy 2013. */
	restart->fruitn  						  = ns->fruitn;
	restart->fruitn_storage  				  = ns->fruitn_storage;
	restart->fruitn_transfer 				  = ns->fruitn_transfer;
	/* softstem simulation - Hidy 2013. */
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
	restart->cwdn							  = ns->cwdn;
	restart->litr1n 						  = ns->litr1n;
	restart->litr2n 						  = ns->litr2n;
	restart->litr3n 						  = ns->litr3n;
	restart->litr4n 						  = ns->litr4n;
	/* Hidy 2013 - temporary senescence pool */
	restart->litr1n_STDB				  = ns->litr1n_STDB;
	restart->litr2n_STDB				  = ns->litr2n_STDB;
	restart->litr3n_STDB				  = ns->litr3n_STDB;
	restart->litr4n_STDB				  = ns->litr4n_STDB;
	restart->soil1n 						  = ns->soil1n;
	restart->soil2n 						  = ns->soil2n;
	restart->soil3n 						  = ns->soil3n;
	restart->soil4n 						  = ns->soil4n;
	/*-----------------------------*/
	/* Hidy 2011 - multilayer soil */
	
	for (layer =0; layer < N_SOILLAYERS; layer++)
	{ 
		restart->sminn[layer]                 = ns->sminn[layer];
	}
	/*-----------------------------*/
	
	restart->retransn						  = ns->retransn;
	restart->npool  						  = ns->npool;

	restart->day_leafc_litfall_increment	  = epv->day_leafc_litfall_increment;
	restart->day_frootc_litfall_increment	  = epv->day_frootc_litfall_increment;
	/* fruit simulation - Hidy 2013. */
	restart->day_fruitc_litfall_increment	  = epv->day_fruitc_litfall_increment;
	/* softstem simulation - Hidy 2013. */
	restart->day_softstemc_litfall_increment  = epv->day_softstemc_litfall_increment;

	restart->day_livestemc_turnover_increment = epv->day_livestemc_turnover_increment;
	restart->day_livecrootc_turnover_increment= epv->day_livecrootc_turnover_increment;
	restart->annmax_leafc					  = epv->annmax_leafc;
	restart->annmax_frootc  				  = epv->annmax_frootc;
	/* fruit simulation - Hidy 2013. */
	restart->annmax_fruitc  				  = epv->annmax_fruitc;
	/* softstem simulation - Hidy 2013. */
	restart->annmax_softstemc  				  = epv->annmax_softstemc;

	restart->annmax_livestemc				  = epv->annmax_livestemc;
	restart->annmax_livecrootc  			  = epv->annmax_livecrootc;
	restart->dsr							  = epv->dsr;

	restart->metyr                            = metyr;



	         
	
	return(!ok);
}
