/* 
ramp_ndep_init.c
Initialize the ramped N deposition parameters for bgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "pointbgc.h"

int ramp_ndep_init(file init, ramp_ndep_struct* ramp_ndep)
{
	int ok = 1;
	char key1[] = "RAMP_NDEP";
	char keyword[80];

	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** RAMP_NDEP                                                     ** 
	**                                                                 **
	********************************************************************/

	/* scan for the climate change block keyword, exit if not next */
	if (ok && scan_value(init, keyword, 's'))
	{
		bgc_printf(BV_ERROR, "Error reading keyword, ramp_ndep_init()\n");
		ok=0;
	}
	if (ok && strcmp(keyword,key1))
	{
		bgc_printf(BV_ERROR, "Expecting keyword --> %s in %s\n",key1,init.name);
		ok=0;
	}

	/* begin reading ramp_ndep information */
	if (ok && scan_value(init, &ramp_ndep->doramp, 'i'))
	{
		bgc_printf(BV_ERROR, "Error reading ramp Ndep flag: ramp_ndep_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &ramp_ndep->ind_year, 'i'))
	{
		bgc_printf(BV_ERROR, "Error reading year of reference for industrial Ndep: ramp_ndep_init()\n");
		ok=0;
	}
    if (scan_value(init,&ramp_ndep->ind_ndep,'d')) 
	{
		bgc_printf(BV_ERROR, "Error reading industrial Ndep value: ramp_ndep_init()\n");
		ok=0;
	}
	
	return (!ok);
}
