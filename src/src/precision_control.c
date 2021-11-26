/* 
precision_control.c
Detects very low values in state variable structures, and forces them to
0.0, in order to avoid rounding and overflow errors.

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2020, D. Hidy [dori.hidy@gmail.com]
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
#include "ini.h"
#include "bgc_struct.h"
#include "bgc_func.h"
#include "bgc_constants.h"

int precision_control(wstate_struct* ws, cstate_struct* cs, nstate_struct* ns)
{
	int errorCode=0;
	int layer;
	
	/* CARBON AND NITROGEN STATE VARIABLES */
	/* force very low leaf C to 0.0, to avoid roundoff
	error in canopy radiation routines. Send excess to litter 1.
	Fine root C and N follow leaf C and N. This control is triggered
	at a higher value than the others because leafc is used in exp()
	in radtrans, and so can cause rounding error at larger values. */

	/************************/
	if ((fabs(cs->leafc) < 1e-8 && fabs(cs->leafc) != 0) || (fabs(ns->leafn) < 1e-9 && fabs(ns->leafn) != 0))
	{
		cs->litr1c[0] += cs->leafc;
		ns->litr1n[0] += ns->leafn;
		cs->leafc = 0.0;
		ns->leafn = 0.0;
	}													 

    if ((cs->leafc_storage != 0 && fabs(cs->leafc_storage) < CRIT_PREC_RIG) || (ns->leafn_storage != 0 && fabs(ns->leafn_storage) < CRIT_PREC_RIG) )
	{
		cs->litr1c[0] += cs->leafc_storage;
		ns->litr1n[0] += ns->leafn_storage;
		cs->leafc_storage = 0.0;
		ns->leafn_storage = 0.0;
	}

	if ((cs->leafc_transfer != 0 && fabs(cs->leafc_transfer) < CRIT_PREC_RIG) || (ns->leafn_transfer != 0 && fabs(ns->leafn_transfer) < CRIT_PREC_RIG) )
	{
		cs->litr1c[0] += cs->leafc_transfer;
		ns->litr1n[0] += ns->leafn_transfer;
		cs->leafc_transfer = 0.0;
		ns->leafn_transfer = 0.0;
	}

	/************************/
	if ((fabs(cs->frootc) < CRIT_PREC && fabs(cs->frootc) != 0) || (fabs(ns->frootn) < CRIT_PREC && fabs(ns->frootn) != 0))
	{
		cs->litr1c[0] += cs->frootc;
		ns->litr1n[0] += ns->frootn;
		cs->frootc = 0.0;
		ns->frootn = 0.0;
	}

    if ((cs->frootc_storage != 0 && fabs(cs->frootc_storage) < CRIT_PREC_RIG) || (ns->frootn_storage != 0 && fabs(ns->frootn_storage) < CRIT_PREC_RIG) )
	{
		cs->litr1c[0] += cs->frootc_storage;
		ns->litr1n[0] += ns->frootn_storage;
		cs->frootc_storage = 0.0;
		ns->frootn_storage = 0.0;
	}

	if ((cs->frootc_transfer != 0 && fabs(cs->frootc_transfer) < CRIT_PREC_RIG) || (ns->frootn_transfer != 0 && fabs(ns->frootn_transfer) < CRIT_PREC_RIG) )
	{
		cs->litr1c[0] += cs->frootc_transfer;
		ns->litr1n[0] += ns->frootn_transfer;
		cs->frootc_transfer = 0.0;
		ns->frootn_transfer = 0.0;
	}

	/************************/
	if ((fabs(cs->fruitc) < CRIT_PREC && fabs(cs->fruitc) != 0) || (fabs(ns->fruitn) < CRIT_PREC && fabs(ns->fruitn) != 0))
	{
		cs->litr1c[0] += cs->fruitc;
		ns->litr1n[0] += ns->fruitn;
		cs->fruitc = 0.0;
		ns->fruitn = 0.0;
	}

    if ((cs->fruitc_storage != 0 && fabs(cs->fruitc_storage) < CRIT_PREC_RIG) || (ns->fruitn_storage != 0 && fabs(ns->fruitn_storage) < CRIT_PREC_RIG) )
	{
		cs->litr1c[0] += cs->fruitc_storage;
		ns->litr1n[0] += ns->fruitn_storage;
		cs->fruitc_storage = 0.0;
		ns->fruitn_storage = 0.0;
	}

	if ((cs->fruitc_transfer != 0 && fabs(cs->fruitc_transfer) < CRIT_PREC_RIG) || (ns->fruitn_transfer < 0 && fabs(ns->fruitn_transfer) < CRIT_PREC_RIG) )
	{
		cs->litr1c[0] += cs->fruitc_transfer;
		ns->litr1n[0] += ns->fruitn_transfer;
		cs->fruitc_transfer = 0.0;
		ns->fruitn_transfer = 0.0;
	}

	/************************/
	if ((fabs(cs->softstemc) != 0  && fabs(cs->softstemc) < CRIT_PREC) || (fabs(ns->softstemn) != 0 && fabs(ns->softstemn) < CRIT_PREC))
	{
		cs->litr1c[0] += cs->softstemc;
		ns->litr1n[0] += ns->softstemn;
		cs->softstemc = 0.0;
		ns->softstemn = 0.0;
	}


    if ((cs->softstemc_storage != 0 && fabs(cs->softstemc_storage) < CRIT_PREC_RIG) || (ns->softstemn_storage != 0 && fabs(ns->softstemn_storage) < CRIT_PREC_RIG) )
	{
		cs->litr1c[0] += cs->softstemc_storage;
		ns->litr1n[0] += ns->softstemn_storage;
		cs->softstemc_storage = 0.0;
		ns->softstemn_storage = 0.0;
	}

	if ((cs->softstemc_transfer != 0 && fabs(cs->softstemc_transfer) < CRIT_PREC_RIG) || (ns->softstemn_transfer != 0 && fabs(ns->softstemn_transfer) < CRIT_PREC_RIG) )
	{
		cs->litr1c[0] += cs->softstemc_transfer;
		ns->litr1n[0] += ns->softstemn_transfer;
		cs->softstemc_transfer = 0.0;
		ns->softstemn_transfer = 0.0;
	}

	/************************/
	if ((fabs(cs->livestemc) < CRIT_PREC && fabs(cs->livestemc) != 0) || (fabs(ns->livestemn) < CRIT_PREC && fabs(ns->livestemn) != 0))
	{
		cs->litr1c[0] += cs->livestemc;
		ns->litr1n[0] += ns->livestemn;
		cs->livestemc = 0.0;
		ns->livestemn = 0.0;
	}
    
    if ((cs->livestemc_storage != 0 && fabs(cs->livestemc_storage) < CRIT_PREC_RIG) || (ns->livestemn_storage != 0 && fabs(ns->livestemn_storage) < CRIT_PREC_RIG) )
	{
		cs->litr1c[0] += cs->livestemc_storage;
		ns->litr1n[0] += ns->livestemn_storage;
		cs->livestemc_storage = 0.0;
		ns->livestemn_storage = 0.0;
	}

	if ((cs->livestemc_transfer != 0 && fabs(cs->livestemc_transfer) < CRIT_PREC_RIG) || (ns->livestemn_transfer != 0 && fabs(ns->livestemn_transfer) < CRIT_PREC_RIG) )
	{
		cs->litr1c[0] += cs->livestemc_transfer;
		ns->litr1n[0] += ns->livestemn_transfer;
		cs->livestemc_transfer = 0.0;
		ns->livestemn_transfer = 0.0;
	}

	/************************/
	if ((fabs(cs->deadstemc) < CRIT_PREC && fabs(cs->deadstemc) != 0) || (fabs(ns->deadstemn) < CRIT_PREC && fabs(ns->deadstemn) != 0))
	{
		cs->litr1c[0] += cs->deadstemc;
		ns->litr1n[0] += ns->deadstemn;
		cs->deadstemc = 0.0;
		ns->deadstemn = 0.0;
	}
   
    if ((cs->deadstemc_storage != 0 && fabs(cs->deadstemc_storage) < CRIT_PREC_RIG) || (ns->deadstemn_storage != 0 && fabs(ns->deadstemn_storage) < CRIT_PREC_RIG) )
	{
		cs->litr1c[0] += cs->deadstemc_storage;
		ns->litr1n[0] += ns->deadstemn_storage;
		cs->deadstemc_storage = 0.0;
		ns->deadstemn_storage = 0.0;
	}

	if ((cs->deadstemc_transfer != 0 && fabs(cs->deadstemc_transfer) < CRIT_PREC_RIG) || (ns->deadstemn_transfer != 0 && fabs(ns->deadstemn_transfer) < CRIT_PREC_RIG) )
	{
		cs->litr1c[0] += cs->deadstemc_transfer;
		ns->litr1n[0] += ns->deadstemn_transfer;
		cs->deadstemc_transfer = 0.0;
		ns->deadstemn_transfer = 0.0;
	}

	/************************/
	if ((fabs(cs->livecrootc) < CRIT_PREC && fabs(cs->livecrootc) != 0) || (fabs(ns->livecrootn) < CRIT_PREC && fabs(ns->livecrootn) != 0))
	{
		cs->litr1c[0] += cs->livecrootc;
		ns->litr1n[0] += ns->livecrootn;
		cs->livecrootc = 0.0;
		ns->livecrootn = 0.0;
	}

    if ((cs->livecrootc_storage < 0 && fabs(cs->livecrootc_storage) < CRIT_PREC) || (ns->livecrootn_storage < 0 && fabs(ns->livecrootn_storage) < CRIT_PREC) )
	{
		cs->litr1c[0] += cs->livecrootc_storage;
		ns->litr1n[0] += ns->livecrootn_storage;
		cs->livecrootc_storage = 0.0;
		ns->livecrootn_storage = 0.0;
	}

	if ((cs->livecrootc_transfer < 0 && fabs(cs->livecrootc_transfer) < CRIT_PREC) || (ns->livecrootn_transfer < 0 && fabs(ns->livecrootn_transfer) < CRIT_PREC) )
	{
		cs->litr1c[0] += cs->livecrootc_transfer;
		ns->litr1n[0] += ns->livecrootn_transfer;
		cs->livecrootc_transfer = 0.0;
		ns->livecrootn_transfer = 0.0;
	}

	/************************/
	if ((fabs(cs->deadcrootc) < CRIT_PREC && fabs(cs->deadcrootc) != 0) || (fabs(ns->deadcrootn) < CRIT_PREC && fabs(ns->deadcrootn) != 0))
	{
		cs->litr1c[0] += cs->deadcrootc;
		ns->litr1n[0] += ns->deadcrootn;
		cs->deadcrootc = 0.0;
		ns->deadcrootn = 0.0;
	}

    if ((cs->deadcrootc_storage < 0 && fabs(cs->deadcrootc_storage) < CRIT_PREC) || (ns->deadcrootn_storage < 0 && fabs(ns->deadcrootn_storage) < CRIT_PREC) )
	{
		cs->litr1c[0] += cs->deadcrootc_storage;
		ns->litr1n[0] += ns->deadcrootn_storage;
		cs->deadcrootc_storage = 0.0;
		ns->deadcrootn_storage = 0.0;
	}

	if ((cs->deadcrootc_transfer < 0 && fabs(cs->deadcrootc_transfer) < CRIT_PREC) || (ns->deadcrootn_transfer < 0 && fabs(ns->deadcrootn_transfer) < CRIT_PREC) )
	{
		cs->litr1c[0] += cs->deadcrootc_transfer;
		ns->litr1n[0] += ns->deadcrootn_transfer;
		cs->deadcrootc_transfer = 0.0;
		ns->deadcrootn_transfer = 0.0;
	}

	/************************/
	if (fabs(cs->gresp_transfer) < CRIT_PREC && cs->gresp_transfer != 0)
	{
 		cs->litr1c[0] += cs->gresp_transfer;
		cs->gresp_transfer = 0.0;

	}

	if (fabs(cs->cpool) < CRIT_PREC && cs->cpool != 0)
	{
 		cs->litr1c[0] += cs->cpool;
		cs->cpool = 0.0;

	}

	if (fabs(cs->nsc_nw) < CRIT_PREC && cs->nsc_nw != 0)
	{
 		cs->litr1c[0] += cs->nsc_nw;
		cs->nsc_nw = 0.0;

	}

	if (fabs(cs->nsc_w) < CRIT_PREC && cs->nsc_w != 0)
	{
 		cs->litr1c[0] += cs->nsc_w;
		cs->nsc_w = 0.0;

	}

	if (fabs(cs->sc_nw) < CRIT_PREC && cs->sc_nw != 0)
	{
 		cs->litr1c[0] += cs->sc_nw;
		cs->sc_nw = 0.0;

	}

	if (fabs(cs->sc_w) < CRIT_PREC && cs->sc_w != 0)
	{
 		cs->litr1c[0] += cs->sc_w;
		cs->sc_w = 0.0;

	}

	/************************/
	if ((fabs(cs->STDBc_leaf) < CRIT_PREC && fabs(cs->STDBc_leaf) != 0) || (fabs(ns->STDBn_leaf) < CRIT_PREC && fabs(ns->STDBn_leaf) != 0))
	{
		cs->litr1c[0] += cs->STDBc_leaf;
		ns->litr1n[0] += ns->STDBn_leaf;
		cs->STDBc_leaf = 0.0;
		ns->STDBn_leaf = 0.0;
	}

	if ((fabs(cs->STDBc_froot) < CRIT_PREC && fabs(cs->STDBc_froot) != 0) || (fabs(ns->STDBn_froot) < CRIT_PREC && fabs(ns->STDBn_froot) != 0))
	{
		cs->litr1c[0] += cs->STDBc_froot;
		ns->litr1n[0] += ns->STDBn_froot;
		cs->STDBc_froot = 0.0;
		ns->STDBn_froot = 0.0;
	}

	if ((fabs(cs->STDBc_softstem) < CRIT_PREC && fabs(cs->STDBc_softstem) != 0) || (fabs(ns->STDBn_softstem) < CRIT_PREC && fabs(ns->STDBn_softstem) != 0))
	{
		cs->litr1c[0] += cs->STDBc_softstem;
		ns->litr1n[0] += ns->STDBn_softstem;
		cs->STDBc_softstem = 0.0;
		ns->STDBn_softstem = 0.0;
	}

	if ((fabs(cs->STDBc_fruit) < CRIT_PREC && fabs(cs->STDBc_fruit) != 0) || (fabs(ns->STDBn_fruit) < CRIT_PREC && fabs(ns->STDBn_fruit) != 0))
	{
		cs->litr1c[0] += cs->STDBc_fruit;
		ns->litr1n[0] += ns->STDBn_fruit;
		cs->STDBc_fruit = 0.0;
		ns->STDBn_fruit = 0.0;
	}

	if ((fabs(cs->STDBc_nsc) < CRIT_PREC && fabs(cs->STDBc_nsc) != 0) || (fabs(ns->STDBn_nsc) < CRIT_PREC && fabs(ns->STDBn_nsc) != 0))
	{
		cs->litr1c[0] += cs->STDBc_nsc;
		ns->litr1n[0] += ns->STDBn_nsc;
		cs->STDBc_nsc = 0.0;
		ns->STDBn_nsc = 0.0;
	}

	/************************/
	if ((fabs(cs->CTDBc_leaf) < CRIT_PREC && fabs(cs->CTDBc_leaf) != 0) || (fabs(ns->CTDBn_leaf) < CRIT_PREC && fabs(ns->CTDBn_leaf) != 0))
	{
		cs->litr1c[0] += cs->CTDBc_leaf;
		ns->litr1n[0] += ns->CTDBn_leaf;
		cs->CTDBc_leaf = 0.0;
		ns->CTDBn_leaf = 0.0;
	}

	if ((fabs(cs->CTDBc_froot) < CRIT_PREC && fabs(cs->CTDBc_froot) != 0) || (fabs(ns->CTDBn_froot) < CRIT_PREC && fabs(ns->CTDBn_froot) != 0))
	{
		cs->litr1c[0] += cs->CTDBc_froot;
		ns->litr1n[0] += ns->CTDBn_froot;
		cs->CTDBc_froot = 0.0;
		ns->CTDBn_froot = 0.0;
	}

	if ((fabs(cs->CTDBc_softstem) < CRIT_PREC && fabs(cs->CTDBc_softstem) != 0) || (fabs(ns->CTDBn_softstem) < CRIT_PREC && fabs(ns->CTDBn_softstem) != 0))
	{
		cs->litr1c[0] += cs->CTDBc_softstem;
		ns->litr1n[0] += ns->CTDBn_softstem;
		cs->CTDBc_softstem = 0.0;
		ns->CTDBn_softstem = 0.0;
	}

	if ((fabs(cs->CTDBc_fruit) < CRIT_PREC && fabs(cs->CTDBc_fruit) != 0) || (fabs(ns->CTDBn_fruit) < CRIT_PREC && fabs(ns->CTDBn_fruit) != 0))
	{
		cs->litr1c[0] += cs->CTDBc_fruit;
		ns->litr1n[0] += ns->CTDBn_fruit;
		cs->CTDBc_fruit = 0.0;
		ns->CTDBn_fruit = 0.0;
	}

	if ((fabs(cs->CTDBc_nsc) < CRIT_PREC && fabs(cs->CTDBc_nsc) != 0) || (fabs(ns->CTDBn_nsc) < CRIT_PREC && fabs(ns->CTDBn_nsc) != 0))
	{
		cs->litr1c[0] += cs->CTDBc_nsc;
		ns->litr1n[0] += ns->CTDBn_nsc;
		cs->CTDBc_nsc = 0.0;
		ns->CTDBn_nsc = 0.0;
	}

	/************************/	
	/* test for litter and soil poils in multilayer soil. Excess goes to hr sink (C) or volatilized sink (N) */


	for (layer=0; layer < N_SOILLAYERS; layer++)
	{
		if ((cs->soil1c[layer] != 0 && fabs(cs->soil1c[layer]) < CRIT_PREC) || (ns->soil1n[layer] != 0 && fabs(ns->soil1n[layer])  < CRIT_PREC))
		{
			cs->soil1_hr_snk  += cs->soil1c[layer];
			ns->Nprec_snk      += ns->soil1n[layer];
			cs->soil1c[layer] = 0.0;
			ns->soil1n[layer] = 0.0;
		}
		if ((cs->soil2c[layer] != 0 && fabs(cs->soil2c[layer]) < CRIT_PREC) || (ns->soil2n[layer] != 0 && fabs(ns->soil2n[layer])  < CRIT_PREC))
		{
			cs->soil2_hr_snk  += cs->soil2c[layer];
			ns->Nprec_snk      += ns->soil2n[layer];
			cs->soil2c[layer] = 0.0;
			ns->soil2n[layer] = 0.0;
		}
		if ((cs->soil3c[layer] != 0 && fabs(cs->soil3c[layer]) < CRIT_PREC) || (ns->soil3n[layer] != 0 && fabs(ns->soil3n[layer])  < CRIT_PREC))
		{
			cs->soil3_hr_snk  += cs->soil3c[layer];
			ns->Nprec_snk      += ns->soil3n[layer];
			cs->soil3c[layer] = 0.0;
			ns->soil3n[layer] = 0.0;
		}
		if ((cs->soil4c[layer] != 0 && fabs(cs->soil4c[layer]) < CRIT_PREC) || (ns->soil4n[layer] != 0 && fabs(ns->soil4n[layer])  < CRIT_PREC))
		{
			cs->soil4_hr_snk  += cs->soil4c[layer];
			ns->Nprec_snk      += ns->soil4n[layer];
			cs->soil4c[layer] = 0.0;
			ns->soil4n[layer] = 0.0;
		}


		if ((cs->litr1c[layer] != 0 && fabs(cs->litr1c[layer]) < CRIT_PREC) || (ns->litr1n[layer] != 0 && fabs(ns->litr1n[layer])  < CRIT_PREC))
		{
			cs->litr1_hr_snk += cs->litr1c[layer];
			ns->Nprec_snk += ns->litr1n[layer];
			cs->litr1c[layer] = 0.0;
			ns->litr1n[layer] = 0.0;
		}
		if ((cs->litr2c[layer] != 0 && fabs(cs->litr2c[layer]) < CRIT_PREC) || (ns->litr2n[layer] != 0 && fabs(ns->litr2n[layer])  < CRIT_PREC))
		{
			cs->litr2_hr_snk += cs->litr2c[layer];
			ns->Nprec_snk += ns->litr2n[layer];
			cs->litr2c[layer] = 0.0;
			ns->litr2n[layer] = 0.0;
		}
		if ((cs->litr3c[layer] != 0 && fabs(cs->litr3c[layer]) < CRIT_PREC) || (ns->litr3n[layer] != 0 && ns->litr3n[layer] < 0 && fabs(ns->litr3n[layer])  < CRIT_PREC))
		{
			cs->litr4_hr_snk += cs->litr3c[layer]; /* NO LITR3C HR SINK */
			ns->Nprec_snk += ns->litr3n[layer];
			cs->litr3c[layer] = 0.0;
			ns->litr3n[layer] = 0.0;
		}
		if ((cs->litr4c[layer] != 0 && fabs(cs->litr4c[layer]) < CRIT_PREC) || (ns->litr4n[layer] != 0 && fabs(ns->litr4n[layer])  < CRIT_PREC))
		{
			cs->litr4_hr_snk += cs->litr4c[layer];
			ns->Nprec_snk += ns->litr4n[layer];
			cs->litr4c[layer] = 0.0;
			ns->litr4n[layer] = 0.0;
		}	

		if ((cs->cwdc[layer] != 0 && fabs(cs->cwdc[layer]) < CRIT_PREC) || (ns->cwdn[layer] != 0 && fabs(ns->cwdn[layer])  < CRIT_PREC))
		{
			cs->litr4_hr_snk += cs->cwdc[layer];
			ns->Nprec_snk += ns->cwdn[layer];
			cs->cwdc[layer] = 0.0;
			ns->cwdn[layer] = 0.0;
		}	

		if ((cs->soil1_DOC[layer] != 0 && fabs(cs->soil1_DOC[layer]) < CRIT_PREC) || (ns->soil1_DON[layer] != 0 && fabs(ns->soil1_DON[layer])  < CRIT_PREC))
		{
			cs->soil1_hr_snk += cs->soil1_DOC[layer];
			ns->Nprec_snk += ns->soil1_DON[layer];
			cs->soil1_DOC[layer] = 0.0;
			ns->soil1_DON[layer] = 0.0;
		}

		if ((cs->soil2_DOC[layer] != 0 && fabs(cs->soil2_DOC[layer]) < CRIT_PREC) || (ns->soil2_DON[layer] != 0 && fabs(ns->soil2_DON[layer])  < CRIT_PREC))
		{
			cs->soil2_hr_snk += cs->soil2_DOC[layer];
			ns->Nprec_snk += ns->soil2_DON[layer];
			cs->soil2_DOC[layer] = 0.0;
			ns->soil2_DON[layer] = 0.0;
		}

		if ((cs->soil3_DOC[layer] != 0 && fabs(cs->soil3_DOC[layer]) < CRIT_PREC) || (ns->soil3_DON[layer] != 0 && fabs(ns->soil3_DON[layer])  < CRIT_PREC))
		{
			cs->soil3_hr_snk += cs->soil3_DOC[layer];
			ns->Nprec_snk += ns->soil3_DON[layer];
			cs->soil3_DOC[layer] = 0.0;
			ns->soil3_DON[layer] = 0.0;
		}

		if ((cs->soil4_DOC[layer] != 0 && fabs(cs->soil4_DOC[layer]) < CRIT_PREC) || (ns->soil4_DON[layer] != 0 && fabs(ns->soil4_DON[layer])  < CRIT_PREC))
		{
			cs->soil4_hr_snk += cs->soil4_DOC[layer];
			ns->Nprec_snk += ns->soil4_DON[layer];
			cs->soil4_DOC[layer] = 0.0;
			ns->soil4_DON[layer] = 0.0;
		}
		
	}


	if (fabs(ns->retransn) < CRIT_PREC && ns->retransn != 0)
	{
		ns->Nprec_snk        += ns->retransn;
		ns->retransn        = 0.0;
	}
	/* additional tests for soil mineral N */
	
	for (layer = 0; layer < N_SOILLAYERS;layer++)
	{
		if (fabs(ns->sminNH4[layer]) < CRIT_PREC_RIG && ns->sminNH4[layer] != 0)
		{
			ns->Nprec_snk         += ns->sminNH4[layer];
			ns->sminNH4[layer] = 0.0;
		}

		if (fabs(ns->sminNO3[layer]) < CRIT_PREC_RIG && ns->sminNO3[layer] != 0)
		{
			ns->Nprec_snk         += ns->sminNO3[layer];
			ns->sminNO3[layer] = 0.0;
		}
	
	}



	
	/* WATER STATE VARIABLES */

	/* multilayer soil */
	for (layer = 0; layer < N_SOILLAYERS; layer ++)
	{
		if (ws->soilw[layer] < 0 && fabs(ws->soilw[layer]) < CRIT_PREC)
		{
			ws->soilEvap_snk += ws->soilw[layer];
			ws->soilw[layer] = 0.0;
		}

	}

	if (ws->snoww < 0 && fabs(ws->snoww) < CRIT_PREC)
	{
		ws->snowsubl_snk += ws->snoww;
		ws->snoww = 0.0;
	}
	if (ws->canopyw < 0 && fabs(ws->canopyw) < CRIT_PREC)
	{
		ws->canopyevap_snk += ws->canopyw;
		ws->canopyw = 0.0;
	}

	if (ws->pondw < 0 && fabs(ws->pondw) < CRIT_PREC)
	{
		ws->pondEvap_snk += ws->pondw;
		ws->pondw = 0.0;
	}
	
	return(errorCode);
}	
