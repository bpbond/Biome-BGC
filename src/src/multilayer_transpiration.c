/* 
multilayer_transpiration.c
Hidy 2011 - part-transpiration (regarding to the different layers of the soil) calculation based on the layer's soil water content

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
BBGC MuSo v4
Copyright 2014, D. Hidy (dori.hidy@gmail.com)
Hungarian Academy of Sciences
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "bgc_struct.h"
#include "bgc_constants.h"
#include "bgc_func.h"    

int multilayer_transpiration(const control_struct* ctrl, const siteconst_struct* sitec, epvar_struct* epv, wstate_struct* ws, wflux_struct* wf)
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
	double soilw_hw;
	double soilw_trans_ctrl1 = 0;
	double soilw_trans_ctrl2 = 0;
	double transp_diff = 0;
	int ok=1;
	double transp_diff_SUM = 0;



	/* *****************************************************************************************************************/
	/* 1. PART-TRANSPIRATION: first approximation tanspiration from every soil layer equally */

	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		/* actual soil water content at theoretical lower limit of water content: hygroscopic water point */
		soilw_hw = sitec->vwc_hw[layer] * sitec->soillayer_thickness[layer] * water_density;

		/* root water uptake is be possible from the layers where root is located  */
		if (layer < epv->n_rootlayers && epv->m_soilstress > 0)
		{
			/*  root water uptake is divided between soil layers whe enough soil moisture is available */
			wf->soilw_trans[layer] = wf->soilw_trans_SUM * 
				                          (epv->m_soilstress_layer[layer] * epv->rootlength_prop[layer]) / epv->m_soilstress;
		
		}
		else wf->soilw_trans[layer] = 0;

		soilw_trans_ctrl1 += wf->soilw_trans[layer];
	}


	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		if (wf->soilw_trans_SUM > 0) wf->soilw_trans[layer]=wf->soilw_trans[layer] *  wf->soilw_trans_SUM / soilw_trans_ctrl1;

		/* actual soil water content at theoretical lower limit of water content: hygroscopic water point */
		soilw_hw = sitec->vwc_hw[layer] * sitec->soillayer_thickness[layer] * water_density;

		/* transp_diff: control parameter to avoid negative soil water content (due to overestimated transpiration + dry soil) */
		transp_diff = ws->soilw[layer] - wf->soilw_trans[layer] - soilw_hw;

		/* theoretical lower limit of water content: hygroscopic water point (if transp_diff less than 0, limited transpiration flux)  */
		if (transp_diff < 0)
		{
			wf->soilw_trans[layer] += transp_diff;
			transp_diff_SUM += transp_diff;
			if (ctrl->onscreen && fabs(transp_diff) > CRIT_PREC) printf("Limited transpiration due to dry soil (multilayer_transpiration.c)\n");
		}

		ws->soilw[layer] -= wf->soilw_trans[layer];
		soilw_trans_ctrl2 += wf->soilw_trans[layer];
		epv->vwc[layer]  = ws->soilw[layer] / sitec->soillayer_thickness[layer] / water_density;
	}

	wf->soilw_trans_SUM += transp_diff_SUM;

	/* control */
	if (fabs(soilw_trans_ctrl2 - wf->soilw_trans_SUM) > 0.000001)
	{
		printf("FATAL ERRROR: transpiration calculation error in multilayer_transpiration.c:\n");
		ok=0;
	}




	return (!ok);
}



