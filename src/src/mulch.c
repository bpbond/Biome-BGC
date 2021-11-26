 /* 
mulch.c
CALCULATING the effect of mulch on the top soil layers: evaporation limitation effect + UNIFORM DISTRIBUTION OF VWC and tsoil 

 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Copyright 2020, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "ini.h"     
#include "pointbgc_struct.h"
#include "bgc_struct.h"
#include "pointbgc_func.h"
#include "bgc_constants.h"

int mulch(siteconst_struct* sitec, soilprop_struct* sprop, metvar_struct* metv,  epvar_struct* epv, cstate_struct* cs)
{

	/* mulching parameters */
	int layer_mulcheff, layer, errorCode;
	double soilw_SUM, tsoil_SUM, mulch;	 


	errorCode=0;
	tsoil_SUM=soilw_SUM=0;	 
	

	layer_mulcheff = ( int) sprop->pLAYER_mulch;

	/**********************************************************************************************/
	/* I. CALCULATING the effect of mulch on the top soil layers: UNIFORM DISTRIBUTION OF tsoil */

	mulch =cs->litrc_above + cs->cwdc_above;
	epv->mulch_coverage = sprop->p1_mulch * pow((mulch)/sprop->p2_mulch, sprop->p3_mulch);
	epv->evapREDmulch   = pow(sprop->pRED_mulch, epv->mulch_coverage/100);

	if (mulch > sprop->pCRIT_mulch)
	{
		for (layer = 0; layer<layer_mulcheff; layer++)
		{
			tsoil_SUM += metv->tsoil[layer] * sitec->soillayer_thickness[layer] / sitec->soillayer_depth[layer_mulcheff-1];
		}


		for (layer = 0; layer<layer_mulcheff; layer++)
		{
			metv->tsoil[layer] = tsoil_SUM;
		}


		metv->tsoil_surface_pre = metv->tsoil[0];
		metv->tsoil_surface     = metv->tsoil[0];
	}

   return (errorCode);
}
	