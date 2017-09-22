/*
annual_rates.c
Functions called annually from bgc()

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.1
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2017, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "bgc_struct.h"
#include "bgc_func.h"
#include "bgc_constants.h"

int annual_rates(const epconst_struct* epc, epvar_struct* epv)
{
	int ok=1;
	
	if (epc->evergreen)
	{
		/* leaf and fineroot litterfall rates */
		epv->day_leafc_litfall_increment = epv->annmax_leafc * epc->leaf_turnover / NDAY_OF_YEAR;
		epv->day_frootc_litfall_increment = epv->annmax_frootc * epc->froot_turnover / NDAY_OF_YEAR;
		epv->annmax_leafc = 0.0;
		epv->annmax_frootc = 0.0;
		/* fruit simulation */
		epv->day_fruitc_litfall_increment = epv->annmax_fruitc * epc->fruit_turnover / NDAY_OF_YEAR;
		epv->annmax_fruitc = 0.0;
		/* softstem simulation */
		epv->day_softstemc_litfall_increment = epv->annmax_softstemc * epc->softstem_turnover / NDAY_OF_YEAR;
		epv->annmax_softstemc = 0.0;
	}
	else
	{
		/* deciduous: reset the litterfall rates to 0.0 for the start of the
		next litterfall season */
		epv->day_leafc_litfall_increment = 0.0;
		epv->day_frootc_litfall_increment = 0.0;
		/* fruit simulation */
		epv->day_fruitc_litfall_increment = 0.0;
	
	}
	/* TREE-specific and NON-WOODY SPECIFIC fluxes */
	if (epc->woody)
	{
		/* live wood turnover rates */
		epv->day_livestemc_turnover_increment = epv->annmax_livestemc * epc->livewood_turnover / NDAY_OF_YEAR;
		epv->day_livecrootc_turnover_increment = epv->annmax_livecrootc * epc->livewood_turnover / NDAY_OF_YEAR;
		epv->annmax_livestemc = 0.0;
		epv->annmax_livecrootc = 0.0;
	}
	else
	{
		/* softstem simulation */
		epv->day_softstemc_litfall_increment = 0.0;
	}
	
	return (!ok);
}
