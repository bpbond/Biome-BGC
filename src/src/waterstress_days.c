/* 
waterstress_days.c
calculating the number of the days since water stress occurs in the vegetation period based on soil water status

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.1
Copyright 2017, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "bgc_struct.h"
#include "bgc_func.h"
#include "bgc_constants.h"

int waterstress_days(int yday, phenology_struct* phen, epvar_struct* epv, epconst_struct* epc) //TEST
{

	int ok=1;
	double dsws, dsws_FULL;
	
	/* assign days since waterstress and end of water stress */
	dsws = epv->dsws;
	dsws_FULL = epv->dsws_FULL;

	/* ***************************************************************************************************** */
	/* Hidy 2010: calculating WATER STRESS DAYS regarding to the top soil layer */

	/* if vwcTOP less than a limit -> water stress, calculating day since water stress */
	if (yday > phen->onday && yday < phen->offday)
	{
		if (epv->m_soilstress < epc->m_soilstress_crit) 
		{
			dsws = dsws + 1;
		}
		else
		{
			dsws = 0;	
		}
	}
	else
	{
		dsws = 0;
	}

	if (yday > phen->onday && yday < phen->offday)
	{
		if (epv->m_soilstress == 0.0)
		{
			dsws_FULL = dsws_FULL + 1;
		}
		else
		{
			dsws_FULL = 0;	
		}
	}
	else
	{
		dsws = 0;
	}

	epv->dsws = dsws;

	return(!ok);
}

