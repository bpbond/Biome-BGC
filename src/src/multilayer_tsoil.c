 /* 
multilayer_tsoil.c
calculation of soil temperature in the different soil layers based on the change of air temperature (direct connection)
to top soil layer and based on empirical function of temperature gradient in soil (Zheng et al.1993)

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.4
Copyright 2017, D. Hidy [dori.hidy@gmail.com]
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

int multilayer_tsoil(int yday, const epconst_struct* epc, const siteconst_struct* sitec, const wstate_struct* ws, 
					 metvar_struct* metv, epvar_struct* epv)
{
	int ok=1;
	int layer;

	/* effect of air temperature change on top soil temperature */
	double effect_of_vegetation = 1;
	double heatcoeff_snow = 0.1;
	double heatcoeff_nosnow = 0.25;
	double tsoil_top_change, heating_coefficient;


	/* daily averaged air tempreture on the given day (calculated from tmax and tmin), temp.gradient and local temperatures */
	double temp_diff_total, temp_diff, tsoil;

	double STv1, STv2, WC, FX, f1, ALX, TA, Td, ZD;
	


	/* *********************************************************** */
	/* 1. FIRST LAYER PROPERTIES */
	/* surface soil temperature change caused by air temp. change can be estimated from the air temperature using empirical equations */	

	
	if (ws->snoww) 
		heating_coefficient = heatcoeff_snow;
	else
		heating_coefficient = heatcoeff_nosnow;
	
	/* shading effect of vegetation (if soil temperature is lower than air temperature the effect is zero) */
	if (metv->tday > metv->tsoil_surface_pre && epv->proj_lai > 0) 
		effect_of_vegetation = exp(-1 * epc->ext_coef * epv->proj_lai);
	else 
		effect_of_vegetation = 1.0;

	if (effect_of_vegetation < 0.5) effect_of_vegetation = 0.5;

		
	/* empirical function for the effect of tair changing */
	tsoil_top_change = (metv->tday - metv->tsoil_surface_pre) * heating_coefficient * effect_of_vegetation;	



	/* ************************************************- */
	/* 2. TEMPERATURE OF DEEPER LAYER BASED ON TEMPERATURE GRADIENT BETWEEN SURFACE LAYER AND LOWERMOST LAYER (BELOW 3M) */

	/* on the first day the temperature of the soil layers are calculated based on the temperature of top and bottom layer */

	metv->tsoil_surface += tsoil_top_change;
	
	temp_diff_total = sitec->tair_annavg - metv->tsoil_surface;
	
	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		
	//	temp_diff = temp_diff_total * (0.1526 * log(depth) + 0.703);	
		temp_diff = temp_diff_total * (0.1448 * log(sitec->soillayer_midpoint[layer]) + 0.6667); 
		metv->tsoil[layer] = metv->tsoil_surface + temp_diff;	

		STv1 = 1000 + 2500 * sitec->BD[layer]/((sitec->BD[layer] + 686 * exp(-5.63*sitec->BD[layer])));
		STv2 = log(500/STv1);
		WC = epv->vwc_avg / ((0.356-0.144*sitec->BD[layer]) * sitec->soillayer_depth[N_SOILLAYERS-2]*100); // max_rootzone_depth: m to cm 
		FX = exp(STv2*pow((1-WC)/(1+WC),2));
		f1 = 1/(FX*STv1);

		ALX = 0.0174*(yday-200);
		TA = sitec->tair_annavg + sitec->tair_annrange * cos(ALX)/2;
		Td = metv->F_temprad_ra - TA;
	
		ZD = -1 * sitec->soillayer_midpoint[layer] * 1000 * f1 * epc->c_param_tsoil; // m to mm
	
		tsoil = sitec->tair_annavg + (sitec->tair_annrange/2 * cos(ALX + ZD) + Td)  * exp(ZD); // depth: m to cm 

		if (epc->STCM_flag) 
			metv->tsoil[layer] = tsoil;

	}

	metv->tsoil_surface_pre = metv->tsoil_surface;
	metv->tday_pre          = metv->tday; 
   
	return (!ok);
}
	