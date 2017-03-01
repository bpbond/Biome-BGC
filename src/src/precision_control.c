/* 
precision_control.c
Detects very low values in state variable structures, and forces them to
0.0, in order to avoid rounding and overflow errors.

*-*-*-*-*-*-*-*-*0-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.4
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2017, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

Updated:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "bgc_struct.h"
#include "bgc_func.h"
#include "bgc_constants.h"

int precision_control(wstate_struct* ws, cstate_struct* cs, nstate_struct* ns)
{
	int ok = 1;
	int layer;
	
	/* CARBON AND NITROGEN STATE VARIABLES */
	/* force very low leaf C to 0.0, to avoid roundoff
	error in canopy radiation routines. Send excess to litter 1.
	Fine root C and N follow leaf C and N. This control is triggered
	at a higher value than the others because leafc is used in exp()
	in radtrans, and so can cause rounding error at larger values. */

	/************************/
	if ((fabs(cs->leafc) < 1e-7 && fabs(cs->leafc) != 0) || (fabs(ns->leafn) < 1e-8 && fabs(ns->leafn) != 0))
	{
		cs->litr1c += cs->leafc;
		ns->litr1n += ns->leafn;
		cs->leafc = 0.0;
		ns->leafn = 0.0;
	}													 

    if ((cs->leafc_storage < 0 && fabs(cs->leafc_storage) < 1e-7) || (ns->leafn_storage < 0 && fabs(ns->leafn_storage) < 1e-8) )
	{
		cs->litr1c += cs->leafc_storage;
		ns->litr1n += ns->leafn_storage;
		cs->leafc_storage = 0.0;
		ns->leafn_storage = 0.0;
	}

	if ((cs->leafc_transfer < 0 && fabs(cs->leafc_transfer) < 1e-7) || (ns->leafn_transfer < 0 && fabs(ns->leafn_transfer) < 1e-8) )
	{
		cs->litr1c += cs->leafc_transfer;
		ns->litr1n += ns->leafn_transfer;
		cs->leafc_transfer = 0.0;
		ns->leafn_transfer = 0.0;
	}

	/************************/
	if ((fabs(cs->frootc) < 1e-7 && fabs(cs->frootc) != 0) || (fabs(ns->frootn) < 1e-8 && fabs(ns->frootn) != 0))
	{
		cs->litr1c += cs->frootc;
		ns->litr1n += ns->frootn;
		cs->frootc = 0.0;
		ns->frootn = 0.0;
	}

    if ((cs->frootc_storage < 0 && fabs(cs->frootc_storage) < 1e-7) || (ns->frootn_storage < 0 && fabs(ns->frootn_storage) < 1e-8) )
	{
		cs->litr1c += cs->frootc_storage;
		ns->litr1n += ns->frootn_storage;
		cs->frootc_storage = 0.0;
		ns->frootn_storage = 0.0;
	}

	if ((cs->frootc_transfer < 0 && fabs(cs->frootc_transfer) < 1e-7) || (ns->frootn_transfer < 0 && fabs(ns->frootn_transfer) < 1e-8) )
	{
		cs->litr1c += cs->frootc_transfer;
		ns->litr1n += ns->frootn_transfer;
		cs->frootc_transfer = 0.0;
		ns->frootn_transfer = 0.0;
	}

	/************************/
	if ((fabs(cs->fruitc) < 1e-7 && fabs(cs->fruitc) != 0) || (fabs(ns->fruitn) < 1e-8 && fabs(ns->fruitn) != 0))
	{
		cs->litr1c += cs->fruitc;
		ns->litr1n += ns->fruitn;
		cs->fruitc = 0.0;
		ns->fruitn = 0.0;
	}

    if ((cs->fruitc_storage < 0 && fabs(cs->fruitc_storage) < 1e-7) || (ns->fruitn_storage < 0 && fabs(ns->fruitn_storage) < 1e-8) )
	{
		cs->litr1c += cs->fruitc_storage;
		ns->litr1n += ns->fruitn_storage;
		cs->fruitc_storage = 0.0;
		ns->fruitn_storage = 0.0;
	}

	if ((cs->fruitc_transfer < 0 && fabs(cs->fruitc_transfer) < 1e-7) || (ns->fruitn_transfer < 0 && fabs(ns->fruitn_transfer) < 1e-8) )
	{
		cs->litr1c += cs->fruitc_transfer;
		ns->litr1n += ns->fruitn_transfer;
		cs->fruitc_transfer = 0.0;
		ns->fruitn_transfer = 0.0;
	}

	/************************/
	if ((fabs(cs->softstemc) < 1e-7 && fabs(cs->softstemc) != 0) || (fabs(ns->softstemn) < 1e-8 && fabs(ns->softstemn) != 0))
	{
		cs->litr1c += cs->softstemc;
		ns->litr1n += ns->softstemn;
		cs->softstemc = 0.0;
		ns->softstemn = 0.0;
	}


    if ((cs->softstemc_storage < 0 && fabs(cs->softstemc_storage) < 1e-7) || (ns->softstemn_storage < 0 && fabs(ns->softstemn_storage) < 1e-8) )
	{
		cs->litr1c += cs->softstemc_storage;
		ns->litr1n += ns->softstemn_storage;
		cs->softstemc_storage = 0.0;
		ns->softstemn_storage = 0.0;
	}

	if ((cs->softstemc_transfer < 0 && fabs(cs->softstemc_transfer) < 1e-7) || (ns->softstemn_transfer < 0 && fabs(ns->softstemn_transfer) < 1e-8) )
	{
		cs->litr1c += cs->softstemc_transfer;
		ns->litr1n += ns->softstemn_transfer;
		cs->softstemc_transfer = 0.0;
		ns->softstemn_transfer = 0.0;
	}

	/************************/
	if ((fabs(cs->livestemc) < 1e-7 && fabs(cs->livestemc) != 0) || (fabs(ns->livestemn) < 1e-8 && fabs(ns->livestemn) != 0))
	{
		cs->litr1c += cs->livestemc;
		ns->litr1n += ns->livestemn;
		cs->livestemc = 0.0;
		ns->livestemn = 0.0;
	}
    
    if ((cs->livestemc_storage < 0 && fabs(cs->livestemc_storage) < 1e-7) || (ns->livestemn_storage < 0 && fabs(ns->livestemn_storage) < 1e-8) )
	{
		cs->litr1c += cs->livestemc_storage;
		ns->litr1n += ns->livestemn_storage;
		cs->livestemc_storage = 0.0;
		ns->livestemn_storage = 0.0;
	}

	if ((cs->livestemc_transfer < 0 && fabs(cs->livestemc_transfer) < 1e-7) || (ns->livestemn_transfer < 0 && fabs(ns->livestemn_transfer) < 1e-8) )
	{
		cs->litr1c += cs->livestemc_transfer;
		ns->litr1n += ns->livestemn_transfer;
		cs->livestemc_transfer = 0.0;
		ns->livestemn_transfer = 0.0;
	}

	/************************/
	if ((fabs(cs->deadstemc) < 1e-7 && fabs(cs->deadstemc) != 0) || (fabs(ns->deadstemn) < 1e-8 && fabs(ns->deadstemn) != 0))
	{
		cs->litr1c += cs->deadstemc;
		ns->litr1n += ns->deadstemn;
		cs->deadstemc = 0.0;
		ns->deadstemn = 0.0;
	}
   
    if ((cs->deadstemc_storage < 0 && fabs(cs->deadstemc_storage) < 1e-7) || (ns->deadstemn_storage < 0 && fabs(ns->deadstemn_storage) < 1e-8) )
	{
		cs->litr1c += cs->deadstemc_storage;
		ns->litr1n += ns->deadstemn_storage;
		cs->deadstemc_storage = 0.0;
		ns->deadstemn_storage = 0.0;
	}

	if ((cs->deadstemc_transfer < 0 && fabs(cs->deadstemc_transfer) < 1e-7) || (ns->deadstemn_transfer < 0 && fabs(ns->deadstemn_transfer) < 1e-8) )
	{
		cs->litr1c += cs->deadstemc_transfer;
		ns->litr1n += ns->deadstemn_transfer;
		cs->deadstemc_transfer = 0.0;
		ns->deadstemn_transfer = 0.0;
	}

	/************************/
	if ((fabs(cs->livecrootc) < 1e-7 && fabs(cs->livecrootc) != 0) || (fabs(ns->livecrootn) < 1e-8 && fabs(ns->livecrootn) != 0))
	{
		cs->litr1c += cs->livecrootc;
		ns->litr1n += ns->livecrootn;
		cs->livecrootc = 0.0;
		ns->livecrootn = 0.0;
	}

    if ((cs->livecrootc_storage < 0 && fabs(cs->livecrootc_storage) < 1e-7) || (ns->livecrootn_storage < 0 && fabs(ns->livecrootn_storage) < 1e-8) )
	{
		cs->litr1c += cs->livecrootc_storage;
		ns->litr1n += ns->livecrootn_storage;
		cs->livecrootc_storage = 0.0;
		ns->livecrootn_storage = 0.0;
	}

	if ((cs->livecrootc_transfer < 0 && fabs(cs->livecrootc_transfer) < 1e-7) || (ns->livecrootn_transfer < 0 && fabs(ns->livecrootn_transfer) < 1e-8) )
	{
		cs->litr1c += cs->livecrootc_transfer;
		ns->litr1n += ns->livecrootn_transfer;
		cs->livecrootc_transfer = 0.0;
		ns->livecrootn_transfer = 0.0;
	}

	/************************/
	if ((fabs(cs->deadcrootc) < 1e-7 && fabs(cs->deadcrootc) != 0) || (fabs(ns->deadcrootn) < 1e-8 && fabs(ns->deadcrootn) != 0))
	{
		cs->litr1c += cs->deadcrootc;
		ns->litr1n += ns->deadcrootn;
		cs->deadcrootc = 0.0;
		ns->deadcrootn = 0.0;
	}

    if ((cs->deadcrootc_storage < 0 && fabs(cs->deadcrootc_storage) < 1e-7) || (ns->deadcrootn_storage < 0 && fabs(ns->deadcrootn_storage) < 1e-8) )
	{
		cs->litr1c += cs->deadcrootc_storage;
		ns->litr1n += ns->deadcrootn_storage;
		cs->deadcrootc_storage = 0.0;
		ns->deadcrootn_storage = 0.0;
	}

	if ((cs->deadcrootc_transfer < 0 && fabs(cs->deadcrootc_transfer) < 1e-7) || (ns->deadcrootn_transfer < 0 && fabs(ns->deadcrootn_transfer) < 1e-8) )
	{
		cs->litr1c += cs->deadcrootc_transfer;
		ns->litr1n += ns->deadcrootn_transfer;
		cs->deadcrootc_transfer = 0.0;
		ns->deadcrootn_transfer = 0.0;
	}

	/************************/
	if (fabs(cs->gresp_transfer) < CRIT_PREC && cs->gresp_transfer != 0)
	{
 		cs->litr1c += cs->gresp_transfer;
		cs->gresp_transfer = 0.0;

	}


	/************************/	
	/* Hidy 2016 - test for litter and soil poils in multilayer soil. Excess goes to hr sink (C) or volatilized sink (N) */


	if (fabs(cs->soil1c) < CRIT_PREC || fabs(ns->soil1n)  < CRIT_PREC)
	{
		cs->soil1_hr_snk  += cs->soil1c;
		ns->nvol_snk      += ns->soil1n;
		cs->soil1c = 0.0;
		ns->soil1n = 0.0;
	}
	if (fabs(cs->soil2c) < CRIT_PREC || fabs(ns->soil2n)  < CRIT_PREC)
	{
		cs->soil2_hr_snk  += cs->soil2c;
		ns->nvol_snk      += ns->soil2n;
		cs->soil2c = 0.0;
		ns->soil2n = 0.0;
	}
	if (fabs(cs->soil3c) < CRIT_PREC || fabs(ns->soil3n)  < CRIT_PREC)
	{
		cs->soil3_hr_snk  += cs->soil3c;
		ns->nvol_snk      += ns->soil3n;
		cs->soil3c = 0.0;
		ns->soil3n = 0.0;
	}
	if (fabs(cs->soil4c) < CRIT_PREC || fabs(ns->soil4n)  < CRIT_PREC)
	{
		cs->soil4_hr_snk  += cs->soil4c;
		ns->nvol_snk      += ns->soil4n;
		cs->soil4c = 0.0;
		ns->soil4n = 0.0;
	}

	if (fabs(cs->litr1c) < CRIT_PREC || fabs(ns->litr1n) < CRIT_PREC)
	{
		cs->litr1_hr_snk += cs->litr1c;
		ns->nvol_snk += ns->litr1n;
		cs->litr1c = 0.0;
		ns->litr1n = 0.0;
	}
	if (fabs(cs->litr2c) < CRIT_PREC || fabs(ns->litr2n)  < CRIT_PREC)
	{
		cs->litr2_hr_snk += cs->litr2c;
		ns->nvol_snk += ns->litr2n;
		cs->litr2c = 0.0;
		ns->litr2n = 0.0;
	}
	if (fabs(cs->litr3c) < CRIT_PREC || fabs(ns->litr3n)  < CRIT_PREC)
	{
		cs->litr4_hr_snk += cs->litr3c; /* NO LITR3C HR SINK */
		ns->nvol_snk += ns->litr3n;
		cs->litr3c = 0.0;
		ns->litr3n = 0.0;
	}
	if (fabs(cs->litr4c) < CRIT_PREC || fabs(ns->litr4n)  < CRIT_PREC)
	{
		cs->litr4_hr_snk += cs->litr4c;
		ns->nvol_snk += ns->litr4n;
		cs->litr4c = 0.0;
		ns->litr4n = 0.0;
	}	



	if (fabs(ns->retransn) < CRIT_PREC && ns->retransn != 0)
	{
		ns->nvol_snk        += ns->retransn;
		ns->retransn        = 0.0;
	}
	/* additional tests for soil mineral N */
	
	for (layer = 0; layer < N_SOILLAYERS;layer++)
	{
		if (fabs(ns->sminn[layer]) < CRIT_PREC && ns->sminn[layer] != 0)
		{
			ns->nvol_snk        += ns->sminn[layer];
			ns->sminn[layer]    = 0.0;
		}
	
	}



	
	/* WATER STATE VARIABLES */

	/* hidy 2010 - multilayer soil */
	for (layer = 0; layer < N_SOILLAYERS; layer ++)
	{
		if (fabs(ws->soilw[layer]) < CRIT_PREC)
		{
			ws->soilevap_snk += ws->soilw[layer];
			ws->soilw[layer] = 0.0;
		}

	}

	if (fabs(ws->snoww) < CRIT_PREC)
	{
		ws->snowsubl_snk += ws->snoww;
		ws->snoww = 0.0;
	}
	if (fabs(ws->canopyw) < CRIT_PREC)
	{
		ws->canopyevap_snk += ws->canopyw;
		ws->canopyw = 0.0;
	}

	if (fabs(ws->pond_water) < CRIT_PREC)
	{
		ws->pondwevap_snk += ws->pond_water;
		ws->pond_water = 0.0;
	}
	
	return(!ok);
}	
