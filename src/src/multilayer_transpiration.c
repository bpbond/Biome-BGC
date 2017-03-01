/* 
multilayer_transpiration.c
Hidy 2011 - part-transpiration (regarding to the different layers of the soil) calculation based on the layer's soil water content

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.4
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
	double soilw_hw, ratio,soilw_trans_ctrl1, soilw_trans_ctrl2, transp_diff_SUM;
	double transp_diff = 0;
	int ok=1;

	soilw_hw=ratio=soilw_trans_ctrl1=soilw_trans_ctrl2=transp_diff=transp_diff_SUM=0;

	/* *****************************************************************************************************************/
	/* 1. PART-TRANSPIRATION: first approximation tanspiration from every soil layer equally */

	for (layer = 0; layer < epv->n_rootlayers; layer++)
	{
		/* actual soil water content at theoretical lower limit of water content: hygroscopic water point */
		soilw_hw = sitec->vwc_hw[layer] * sitec->soillayer_thickness[layer] * water_density;

		/* root water uptake is be possible from the layers where root is located  */ 
		if (epv->n_rootlayers > 1)
		{
			if (epv->m_soilstress > 0)
				ratio=(epv->m_soilstress_layer[layer] * epv->rootlength_prop[layer]) / epv->m_soilstress;
			else
				ratio=0;
		}
		
		else 
			ratio = 1;

		wf->soilw_trans[layer] = wf->soilw_trans_SUM * ratio;
		soilw_trans_ctrl1 += wf->soilw_trans[layer];
	}

	/* control */
	if (fabs(soilw_trans_ctrl1 - wf->soilw_trans_SUM) > CRIT_PREC)
	{
		printf("FATAL ERRROR: transpiration calculation error in multilayer_transpiration.c:\n");
		ok=0;
	}


	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{

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
	if (fabs(soilw_trans_ctrl2 - wf->soilw_trans_SUM) > CRIT_PREC)
	{
		printf("FATAL ERRROR: transpiration calculation error in multilayer_transpiration.c:\n");
		ok=0;
	}




	return (!ok);
}



