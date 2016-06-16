/* 
groundwater.c
calculate the effect of gound water depth in the soil

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
BBGC MuSo v4
Copyright 2014, D. Hidy (dori.hidy@gmail.com)
Hungarian Academy of Sciences
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "ini.h"
#include "bgc_struct.h"
#include "pointbgc_struct.h"
#include "pointbgc_func.h"
#include "bgc_constants.h"



int groundwater(const control_struct* ctrl, siteconst_struct* sitec, epvar_struct* epv, wstate_struct* ws, wflux_struct* wf)
{
	int groundwater=0;
	int layerSAT;
	int ok=1;
	int layer=0;
	double weight;
	double soilw_sat, soilw_befgound, soilw_aftground;
	double PARAM = 0.99;
	
	/* if gound water depth information is available using in multilayer calculation */
	if (ctrl->GWD_flag)	
	{
		/* in spinup and transient run only 365 values are used (daily averages) */
		if (ctrl->spinup == 0)
			sitec->gwd_act = sitec->gwd_array[(ctrl->simyr * NDAY_OF_YEAR) + ctrl->yday]; 
		else
			sitec->gwd_act = sitec->gwd_array[ctrl->yday];
		

		/* calculate processes layer to layer (i0: actual layer, i1:deeper layer)  - EXCEPT OF THE BOTTOM LAYER */
 		while (groundwater == 0 && layer < N_SOILLAYERS-1)
		{
			/* actual groundwater level is above the lower boundary of bottom layer (3m) */
			if (sitec->gwd_act < sitec->soillayer_depth[layer])
			{
			
				soilw_sat                = sitec->vwc_sat[layer] * PARAM * sitec->soillayer_thickness[layer] * water_density;
				soilw_befgound           = ws->soilw[layer]; 

				weight = (sitec->soillayer_depth[layer] - sitec->gwd_act)/sitec->soillayer_thickness[layer];
				
				soilw_aftground          = soilw_sat * PARAM * weight + soilw_befgound * (1-weight);
				wf->soilw_from_GW[layer] = soilw_aftground - soilw_befgound;

				ws->soilw[layer]         = soilw_aftground;
				epv->vwc[layer]          = ws->soilw[layer] / water_density / sitec->soillayer_thickness[layer];

				groundwater = 1;

				/* soil layers below the groundwater level are saturated - net water gain from soil system */
				for (layerSAT = 1; layer+layerSAT < N_SOILLAYERS-1; layerSAT++)
				{
					soilw_sat                         = sitec->vwc_sat[layer+layerSAT] * PARAM * sitec->soillayer_thickness[layer+layerSAT] * water_density;
					wf->soilw_from_GW[layer+layerSAT] = soilw_sat - ws->soilw[layer+layerSAT];
					ws->soilw[layer+layerSAT]         = soilw_sat;
					epv->vwc[layer+layerSAT]          = sitec->vwc_sat[layer+layerSAT] * PARAM;

				}
				wf->soilw_from_GW[N_SOILLAYERS-1] = 0;

			}
			layer += 1;
		}
	}
	else
		sitec->gwd_act = DATA_GAP;

	if (sitec->gwd_act != DATA_GAP && sitec->gwd_act < sitec->soillayer_depth[N_SOILLAYERS-2])
	{
		epv->vwc[N_SOILLAYERS-1]  = sitec->vwc_sat[N_SOILLAYERS-1] * PARAM;
		ws->soilw[N_SOILLAYERS-1] = epv->vwc[N_SOILLAYERS-1] * (sitec->soillayer_thickness[N_SOILLAYERS-1]) * 1000.0;
	}
	else
	{
		epv->vwc[N_SOILLAYERS-1]  = sitec->vwc_fc[N_SOILLAYERS-1];
		ws->soilw[N_SOILLAYERS-1] = sitec->vwc_fc[N_SOILLAYERS-1] * (sitec->soillayer_thickness[N_SOILLAYERS-1]) * 1000.0;
	}
		
	return (!ok);
}
