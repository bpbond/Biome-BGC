/*
state_init.c
Initialize water, carbon, and nitrogen state variables for pointbgc simulation  

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.1
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2016, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "ini.h"
#include "bgc_struct.h"
#include "bgc_constants.h"
#include "pointbgc_struct.h"
#include "pointbgc_func.h"

int wstate_init(file init, const siteconst_struct* sitec, wstate_struct* ws)
{
	int ok = 1;
	int layer;
	char key[] = "W_STATE";
	char keyword[80];
	double psat = 0;
	
	
	/* read water state variable initialization values from *.init */
	if (ok && scan_value(init, keyword, 's'))
	{
		printf("Error reading keyword, wstate_init()\n");
		ok=0;
	}
	if (ok && strcmp(keyword,key))
	{
		printf("Expexting keyword --> %s in %s\n",key,init.name);
		ok=0;
	}

	if (ok && scan_value(init, &ws->snoww, 'd'))
	{
		printf("Error reading snowpack, wstate_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &psat, 'd'))
	{
		printf("Error reading soilwater, wstate_init()\n");
		ok=0;
	}
	
	/* check that psat is an acceptable proportion  */
	if (ok && (psat < 0.0 || psat > 1.0))
	{
		printf("Error: initial soil water proportion must be >= 0.0 and <= 1.0\n");
		ok=0;
	}
	if (ok)
	{
		/* calculate initial soilwater in kg/m2 from proportion of
		field capacity volumetric water content, depth, and density of water */
		for (layer = 0; layer < N_SOILLAYERS; layer ++)
		{
			ws->soilw[layer] = sitec->vwc_fc[layer] * (sitec->soillayer_thickness[layer]) * 1000.0;
			ws->soilw_SUM += ws->soilw[layer];

		}
	
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
		printf("Error reading keyword, cstate_init()\n");
		ok=0;
	}
	if (ok && strcmp(keyword,key1))
	{
		printf("Expecting keyword --> %s in %s\n",key1,init.name);
		ok=0;
	}
	if (ok && scan_value(init, &cinit->max_leafc, 'd'))
	{
		printf("Error reading first-year maximum leaf carbon, cstate_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &cinit->max_stemc, 'd'))
	{
		printf("Error reading first-year maximum stem carbon, cstate_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &cs->cwdc, 'd'))
	{
		printf("Error reading coarse woody debris carbon, cstate_init()\n");
		ok=0;
	}

    /* Hidy 2008 - to avoid dividing by 0: if no deadwood, cwdn is zero. */
	if (ok && epc->deadwood_cn > 0.0) 
	{
		ns->cwdn = cs->cwdc/epc->deadwood_cn;
	}
	else
	{
		ns->cwdn = 0;
	}
	
	/* read the litter carbon pool initial values */
	if (ok && scan_value(init, &cs->litr1c, 'd'))
	{
		printf("Error reading litter carbon in labile pool, cstate_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &cs->litr2c, 'd'))
	{
		printf("Error reading litter carbon in unshielded cellulose pool, cstate_int()\n");
		ok=0;
	}
	if (ok && scan_value(init, &cs->litr3c, 'd'))
	{
		printf("Error reading litter carbon in shielded cellulose pool, cstate_int()\n");
		ok=0;
	}
	if (ok && scan_value(init, &cs->litr4c, 'd'))
	{
		printf("Error reading litter carbon in lignin pool, cstate_init()\n");
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
		printf("Error reading fast microbial recycling carbon, cstate_init()\n");
		ok=0;
	}
	if (ok) ns->soil1n = cs->soil1c/SOIL1_CN;
 	if (ok && scan_value(init, &cs->soil2c, 'd'))
	{
		printf("Error reading medium microbial recycling carbon, cstate_init()\n");
		ok=0;
	}
	if (ok) ns->soil2n = cs->soil2c/SOIL2_CN;
	if (ok && scan_value(init, &cs->soil3c, 'd'))
	{
		printf("Error reading slow microbial recycling carbon, cstate_init()\n");
		ok=0;
	}
	if (ok) ns->soil3n = cs->soil3c/SOIL3_CN;
	if (ok && scan_value(init, &cs->soil4c, 'd'))
	{
		printf("Error reading recalcitrant SOM carbon, cstate_init()\n");
		ok=0;
	}
	if (ok) ns->soil4n = cs->soil4c/SOIL4_CN;
	
	
	/* read nitrogen state variable initial values from *.init */
	if (ok && scan_value(init, keyword, 's'))
	{
		printf("Error reading keyword, nstate_init()\n");
		ok=0;
	}
	if (ok && strcmp(keyword, key2))
	{
		printf("Expecting keyword --> %s in %s\n",key2,init.name);
		ok=0;
	}
	if (ok && scan_value(init, &ns->litr1n, 'd'))
	{
		printf("Error reading litter nitrogen in labile pool, cnstate_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &ns->sminn_RZ, 'd'))
	{
		printf("Error reading soil mineral nitrogen, nstate_init()\n");
		ok=0;
	} 

	return (!ok);
}
