/* 
multilayer_transpiration.c
Hidy 2011 - part-transpiration (regarding to the different layers of the soil) calculation based on the layer's soil water content

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Copyright 2020, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "ini.h"
#include "bgc_struct.h"
#include "bgc_constants.h"
#include "bgc_func.h"    

int multilayer_transpiration(control_struct* ctrl, const siteconst_struct* sitec, const soilprop_struct* sprop, 
	                          epvar_struct* epv, wstate_struct* ws, wflux_struct* wf)
{
	/* given a list of site constants and the soil water mass (kg/m2),
	this function returns the soil water potential (MPa)
	inputs:


	For further discussion see:
	Cosby, B.J., G.M. Hornberger, R.B. Clapp, and T.R. Ginn, 1984.     

	Balsamo et al 2009 - 
	A Revised Hydrology for the ECMWF Model - Verification from Field Site to Water Storage IFS - JHydromet.pdf

	Chen and Dudhia 2001 - 
	Coupling an Advanced Land Surface-Hydrology Model with the PMM5 Modeling System Part I - MonWRev.pdf*/
	

	/* internal variables */
	int layer;
	double soilw_transp_SUM, soilw_wp, transp_lack;
	
	int errorCode=0;

	soilw_transp_SUM=soilw_wp=0;


	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		/* actual soil water content at theoretical lower limit of water content: hygroscopic water point */
		soilw_wp = sprop->VWCwp[layer] * sitec->soillayer_thickness[layer] * water_density;

		/* transp_lack: control parameter to avoid negative soil water content (due to overestimated transpiration + dry soil) */
		ws->soilw_avail[layer] = (ws->soilw[layer] - soilw_wp);
	}
	
		/* *****************************************************************************************************************/
	/* 1. PART-TRANSPIRATION: first approximation tanspiration from every soil layer equally */

	for (layer = epv->germ_layer; layer < epv->n_rootlayers; layer++)
	{		
		
		/* transpiration based on rootlenght proportion */
		wf->soilw_transpDEMAND[layer] = wf->soilw_transpDEMAND_SUM * epv->rootlength_prop[layer]; 


		/* transp_lack: control parameter to avoid negative soil water content (due to overestimated transpiration + dry soil) */
		transp_lack = wf->soilw_transpDEMAND[layer] - ws->soilw_avail[layer];

		/* if transpiration demand is greater than theoretical lower limit of water content: wilting point -> limited transpiration flux)  */
		if (transp_lack > 0)
		{
			/* theoretical limit */
			if (ws->soilw[layer] - soilw_wp > CRIT_PREC)
				wf->soilw_transp[layer] = ws->soilw[layer] - soilw_wp;
			else
				wf->soilw_transp[layer] = 0;

	
			/* limittransp_flag: writing in log file (only at first time) */
			if (transp_lack > CRIT_PREC && !ctrl->limittransp_flag) ctrl->limittransp_flag = 1;
		}
		else
			wf->soilw_transp[layer] = wf->soilw_transpDEMAND[layer];

		ws->soilw[layer] -= wf->soilw_transp[layer];
		epv->VWC[layer]  = ws->soilw[layer] / sitec->soillayer_thickness[layer] / water_density;
	
		soilw_transp_SUM += wf->soilw_transp[layer];
	}

	wf->soilw_transp_SUM = soilw_transp_SUM;

	/* control */
	if (wf->soilw_transp_SUM - wf->soilw_transpDEMAND_SUM > CRIT_PREC)
	{
		printf("\n");
		printf("ERROR: transpiration calculation error in multilayer_hydrolprocess.c:\n");
		errorCode=1;
	}

	/* extreme dry soil - no transpiration occurs */
	if (soilw_transp_SUM == 0 && wf->soilw_transp_SUM != 0)
	{
		wf->soilw_transp_SUM = 0;
		/* notransp_flag: flag of WARNING writing in log file (only at first time) */
		if (!ctrl->notransp_flag) ctrl->notransp_flag = 1;
	}





	return (errorCode);
}



