/*
state_init.c
Initialize water, carbon, and nitrogen state variables for pointbgc simulation  

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2018, D. Hidy [dori.hidy@gmail.com]
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
	double prop_fc = 0;
	
	
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
	if (ok && scan_value(init, &prop_fc, 'd'))
	{
		printf("Error reading initial soilwater (FCprop), wstate_init()\n");
		ok=0;
	}
	
	/* check that prop_fc is an acceptable proportion  */
	if (ok && (prop_fc < 0.0))
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
			if (prop_fc > sitec->vwc_sat[layer]/sitec->vwc_sat[layer])
			{
				printf("Error: initial soil water proportion must less than saturation proportion: %lf\n", sitec->vwc_sat[layer]/sitec->vwc_fc[layer]);
				ok=0;
			}
			else
			{
				ws->soilw[layer] = prop_fc * sitec->vwc_fc[layer] * (sitec->soillayer_thickness[layer]) * 1000.0;
				ws->soilw_SUM += ws->soilw[layer];
			}
				
			

		}
	
	}
	
	return (!ok);
}

int cnstate_init(file init, const epconst_struct* epc, cstate_struct* cs, cinit_struct* cinit, nstate_struct* ns)
{
	int ok=1;
	int layer, scanflag;
	char key1[] = "CN_STATE";
	char keyword[80];

	/* 1. read carbon state variable initial values from *.init */
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
	if (ok && scan_value(init, &cinit->max_frootc, 'd'))
	{
		printf("Error reading first-year maximum fine root carbon, cstate_init()\n");
		ok=0;
	}
		if (ok && scan_value(init, &cinit->max_fruitc, 'd'))
	{
		printf("Error reading first-year max_fruitc, cstate_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &cinit->max_softstemc, 'd'))
	{
		printf("Error reading first-year max_sofstemc, cstate_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &cinit->max_livestemc, 'd'))
	{
		printf("Error reading first-year max_livestemc, cstate_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &cinit->max_livecrootc, 'd'))
	{
		printf("Error reading first-year max_livecrootc, cstate_init()\n");
		ok=0;
	}

	/*--------------------------------------------------*/
	/* 2. read the cwdc initial values in multilayer soil  */

	scanflag=0; 
	for (layer=0; layer<N_SOILLAYERS; layer++)
	{
		if (layer==N_SOILLAYERS-1) scanflag=1;
		if (ok && scan_array(init, &(cs->cwdc[layer]), 'd', scanflag))
		{
			printf("Error reading cwdc in layer %i, cstate_init()\n", layer);
			ok=0;
		}
	}

    /* to avoid dividing by 0: if no deadwood, cwdn is zero. */
	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		if (ok && epc->deadwood_cn > 0.0) 
			ns->cwdn[layer] = cs->cwdc[layer]/epc->deadwood_cn;
		else
			ns->cwdn[layer] = 0;
	}


	/*--------------------------------------------------*/
	/* 3. read the litter carbon pool initial values in multilayer soil  */
	scanflag=0; 
	for (layer=0; layer<N_SOILLAYERS; layer++)
	{
		if (layer==N_SOILLAYERS-1) scanflag=1;
		if (ok && scan_array(init, &(cs->litr1c[layer]), 'd', scanflag))
		{
			printf("Error reading litter carbon in labile pool in layer %i, cstate_init()\n", layer);
			ok=0;
		}
	}
	
	scanflag=0; 
	for (layer=0; layer<N_SOILLAYERS; layer++)
	{
		if (layer==N_SOILLAYERS-1) scanflag=1;
		if (ok && scan_array(init, &(cs->litr2c[layer]), 'd', scanflag))
		{
			printf("Error reading litter carbon in unshielded cellulose pool in layer %i, cstate_init()\n", layer);
			ok=0;
		}
	}
	
	scanflag=0; 
	for (layer=0; layer<N_SOILLAYERS; layer++)
	{
		if (layer==N_SOILLAYERS-1) scanflag=1;
		if (ok && scan_array(init, &(cs->litr3c[layer]), 'd', scanflag))
		{
			printf("Error reading litter carbon in shielded cellulose pool in layer %i, cstate_init()\n", layer);
			ok=0;
		}
	}

	scanflag=0; 
	for (layer=0; layer<N_SOILLAYERS; layer++)
	{
		if (layer==N_SOILLAYERS-1) scanflag=1;
		if (ok && scan_array(init, &(cs->litr4c[layer]), 'd', scanflag))
		{
			printf("Error reading litter carbon in lignin pool in layer %i, cstate_init()\n", layer);
			ok=0;
		}
	}


	/* calculate the litter nitrogen pool initial values for cellulose and and lignin pools, 
	using the leaf litter C:N as the basis for determining N content in all litter components  */
	
	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		ns->litr1n[layer] = cs->litr1c[layer] / epc->leaflitr_cn;
		ns->litr2n[layer] = cs->litr2c[layer] / epc->leaflitr_cn;
		ns->litr3n[layer] = cs->litr3c[layer] / epc->leaflitr_cn;
		ns->litr4n[layer] = cs->litr4c[layer] / epc->leaflitr_cn;
	}


	/*--------------------------------------------------*/
	/* 4. read the soil carbon pool initial values in multilayer soil  */

	scanflag=0; 
	for (layer=0; layer<N_SOILLAYERS; layer++)
	{
		if (layer==N_SOILLAYERS-1) scanflag=1;
		if (ok && scan_array(init, &(cs->soil1c[layer]), 'd', scanflag))
		{
			printf("Error reading fast microbial recycling carbon pool in layer %i, cstate_init()\n", layer);
			ok=0;
		}
	}
		
	scanflag=0; 
	for (layer=0; layer<N_SOILLAYERS; layer++)
	{
		if (layer==N_SOILLAYERS-1) scanflag=1;
		if (ok && scan_array(init, &(cs->soil2c[layer]), 'd', scanflag))
		{
			printf("Error reading medium microbial recycling carbon pool in layer %i, cstate_init()\n", layer);
			ok=0;
		}
	}

 	scanflag=0; 
	for (layer=0; layer<N_SOILLAYERS; layer++)
	{
		if (layer==N_SOILLAYERS-1) scanflag=1;
		if (ok && scan_array(init, &(cs->soil3c[layer]), 'd', scanflag))
		{
			printf("Error reading slow microbial recycling carbon pool in layer %i, cstate_init()\n", layer);
			ok=0;
		}
	}


	scanflag=0; 
	for (layer=0; layer<N_SOILLAYERS; layer++)
	{
		if (layer==N_SOILLAYERS-1) scanflag=1;
		if (ok && scan_array(init, &(cs->soil3c[layer]), 'd', scanflag))
		{
			printf("Error reading  recalcitrant SOM carbon pool in layer %i, cstate_init()\n", layer);
			ok=0;
		}
	}


	/* multilayer soil */
	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		ns->soil1n[layer] = cs->soil1c[layer]/SOIL1_CN;
		ns->soil2n[layer] = cs->soil2c[layer]/SOIL2_CN;
		ns->soil3n[layer] = cs->soil3c[layer]/SOIL3_CN;
		ns->soil4n[layer] = cs->soil4c[layer]/SOIL4_CN;
	}
	
	/* 5. read nitrogen state variable initial values from *.init */
	scanflag=0; 
	for (layer=0; layer<N_SOILLAYERS; layer++)
	{
		if (layer==N_SOILLAYERS-1) scanflag=1;
		if (ok && scan_array(init, &(ns->litr1n[layer]), 'd', scanflag))
		{
			printf("Error reading litter nitrogen in labile pool layer %i, cnstate_init()\n", layer);
			ok=0;
		}
	}

	scanflag=0; 
	for (layer=0; layer<N_SOILLAYERS; layer++)
	{
		if (layer==N_SOILLAYERS-1) scanflag=1;
		if (ok && scan_array(init, &(ns->sminNH4[layer]), 'd', scanflag))
		{
			printf("Error reading soil mineral nitrogen (NH4 pool) in layer %i, cnstate_init()\n", layer);
			ok=0;
		}
	}

	scanflag=0; 
	for (layer=0; layer<N_SOILLAYERS; layer++)
	{
		if (layer==N_SOILLAYERS-1) scanflag=1;
		if (ok && scan_array(init, &(ns->sminNO3[layer]), 'd', scanflag))
		{
			printf("Error reading soil mineral nitrogen (NO3 pool) in layer %i, cnstate_init()\n", layer);
			ok=0;
		}
	}


	return (!ok);
}
