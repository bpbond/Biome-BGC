/* 
dayphen.c
transfer one day of phenological data from phenarr struct to phen struct

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "bgc.h"

int dayphen(const phenarray_struct* phenarr, phenology_struct* phen, int metday)
{
	int ok=1;
	
	phen->remdays_curgrowth = (double)(phenarr->remdays_curgrowth[metday]);
	phen->remdays_transfer = (double)(phenarr->remdays_transfer[metday]);
	phen->remdays_litfall = (double)(phenarr->remdays_litfall[metday]);
	phen->predays_transfer = (double)(phenarr->predays_transfer[metday]);
	phen->predays_litfall = (double)(phenarr->predays_litfall[metday]);
	
	return(!ok);
}
	
