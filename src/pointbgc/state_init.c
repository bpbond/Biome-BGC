/*
state_init.c
Initialize water, carbon, and nitrogen state variables for pointbgc simulation  

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "pointbgc.h"

int wstate_init(file init, const siteconst_struct* sitec, wstate_struct* ws)
{
	int ok = 1;
	char key[] = "W_STATE";
	char keyword[80];
	double psat;
	
	/* read water state variable initialization values from *.init */
	if (ok && scan_value(init, keyword, 's'))
	{
		bgc_printf(BV_ERROR, "Error reading keyword, wstate_init()\n");
		ok=0;
	}
	if (ok && strcmp(keyword,key))
	{
		bgc_printf(BV_ERROR, "Expexting keyword --> %s in %s\n",key,init.name);
		ok=0;
	}

	if (ok && scan_value(init, &ws->snoww, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading snowpack, wstate_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &psat, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading soilwater, wstate_init()\n");
		ok=0;
	}
	
	/* check that psat is an acceptable proportion  */
	if (ok && (psat < 0.0 || psat > 1.0))
	{
		bgc_printf(BV_ERROR, "Error: initial soil water proportion must be >= 0.0 and <= 1.0\n");
		ok=0;
	}
	if (ok)
	{
		/* calculate initial soilwater in kg/m2 from proportion of
		saturated volumetric water content, depth, and density of water */
		ws->soilw = sitec->vwc_sat * psat * sitec->soil_depth * 1000.0;
	}
	
	return (!ok);
}

int cnstate_init(file init, const epconst_struct* epc, cstate_struct* cs,
cinit_struct* cinit, nstate_struct* ns)
{
	int ok=1;
	char key1[] = "C_STATE";
	char key2[] = "N_STATE";
	char keyword[80];

	/* read carbon state variable initial values from *.init */
	if (ok && scan_value(init, keyword, 's'))
	{
		bgc_printf(BV_ERROR, "Error reading keyword, cstate_init()\n");
		ok=0;
	}
	if (ok && strcmp(keyword,key1))
	{
		bgc_printf(BV_ERROR, "Expecting keyword --> %s in %s\n",key1,init.name);
		ok=0;
	}
	if (ok && scan_value(init, &cinit->max_leafc, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading first-year maximum leaf carbon, cstate_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &cinit->max_stemc, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading first-year maximum stem carbon, cstate_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &cs->cwdc, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading coarse woody debris carbon, cstate_init()\n");
		ok=0;
	}
	if (ok) ns->cwdn = cs->cwdc/epc->deadwood_cn;
	/* read the litter carbon pool initial values */
	if (ok && scan_value(init, &cs->litr1c, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading litter carbon in labile pool, cstate_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &cs->litr2c, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading litter carbon in unshielded cellulose pool, cstate_int()\n");
		ok=0;
	}
	if (ok && scan_value(init, &cs->litr3c, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading litter carbon in shielded cellulose pool, cstate_int()\n");
		ok=0;
	}
	if (ok && scan_value(init, &cs->litr4c, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading litter carbon in lignin pool, cstate_init()\n");
		ok=0;
	}
	/* calculate the litter nitrogen pool initial values for cellulose and
	and lignin pools, using the leaf litter C:N as the basis for determining
	N content in all litter components */
	ns->litr2n = cs->litr2c / epc->leaflitr_cn;
	ns->litr3n = cs->litr3c / epc->leaflitr_cn;
	ns->litr4n = cs->litr4c / epc->leaflitr_cn;
		
	if (ok && scan_value(init, &cs->soil1c, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading fast microbial recycling carbon, cstate_init()\n");
		ok=0;
	}
	if (ok) ns->soil1n = cs->soil1c/SOIL1_CN;
	if (ok && scan_value(init, &cs->soil2c, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading medium microbial recycling carbon, cstate_init()\n");
		ok=0;
	}
	if (ok) ns->soil2n = cs->soil2c/SOIL2_CN;
	if (ok && scan_value(init, &cs->soil3c, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading slow microbial recycling carbon, cstate_init()\n");
		ok=0;
	}
	if (ok) ns->soil3n = cs->soil3c/SOIL3_CN;
	if (ok && scan_value(init, &cs->soil4c, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading recalcitrant SOM carbon, cstate_init()\n");
		ok=0;
	}
	if (ok) ns->soil4n = cs->soil4c/SOIL4_CN;
	
	
	/* read nitrogen state variable initial values from *.init */
	if (ok && scan_value(init, keyword, 's'))
	{
		bgc_printf(BV_ERROR, "Error reading keyword, nstate_init()\n");
		ok=0;
	}
	if (ok && strcmp(keyword, key2))
	{
		bgc_printf(BV_ERROR, "Expecting keyword --> %s in %s\n",key2,init.name);
		ok=0;
	}
	if (ok && scan_value(init, &ns->litr1n, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading litter nitrogen in labile pool, cnstate_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &ns->sminn, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading soil mineral nitrogen, nstate_init()\n");
		ok=0;
	} 

	return (!ok);
}
