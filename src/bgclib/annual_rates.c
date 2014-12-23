/*
annual_rates.c
Functions called annually from bgc()

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "bgc.h"

int annual_rates(const epconst_struct* epc, epvar_struct* epv)
{
	int ok=1;
	
	if (epc->evergreen)
	{
		/* leaf and fineroot litterfall rates */
		epv->day_leafc_litfall_increment = epv->annmax_leafc * epc->leaf_turnover / 365.0;
		epv->day_frootc_litfall_increment = epv->annmax_frootc * epc->froot_turnover / 365.0;
		epv->annmax_leafc = 0.0;
		epv->annmax_frootc = 0.0;
	}
	else
	{
		/* deciduous: reset the litterfall rates to 0.0 for the start of the
		next litterfall season */
		epv->day_leafc_litfall_increment = 0.0;
		epv->day_frootc_litfall_increment = 0.0;
	}
	
	if (epc->woody)
	{
		/* live wood turnover rates */
		epv->day_livestemc_turnover_increment = epv->annmax_livestemc * epc->livewood_turnover / 365.0;
		epv->day_livecrootc_turnover_increment = epv->annmax_livecrootc * epc->livewood_turnover / 365.0;
		epv->annmax_livestemc = 0.0;
		epv->annmax_livecrootc = 0.0;
	}
	
	return (!ok);
}
