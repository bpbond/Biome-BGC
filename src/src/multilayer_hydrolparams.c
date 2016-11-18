/* 
multilayer_hydrolparams.c
calcultion of soil water potential, hydr. conductivity and hydr. diffusivity as a function of volumetric water content and
constants related to texture

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.1
Copyright 2016, D. Hidy [dori.hidy@gmail.com]
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
#include "bgc_func.h"
#include "bgc_constants.h"

int multilayer_hydrolparams(const siteconst_struct* sitec,  wstate_struct* ws, epvar_struct* epv, metvar_struct* metv)
{
	/* given a list of site constants and the soil water mass (kg/m2),
	this function returns the soil water potential (MPa)
	inputs:
	ws.soilw				     (kg/m2) water mass per unit area
	sitec.max_rootzone_depth     (m)     maximum depth of rooting zone               
	sitec.soil_b				 (DIM)   slope of log(psi) vs log(rwc)
	sitec.vwc_sat				 (DIM)   volumetric water content at saturation
	sitec.psi_sat			   	(MPa)   soil matric potential at saturation
	output:
	psi						 (MPa)   soil matric potential

	uses the relation:
	psi = psi_sat * (vwc/vwc_sat)^(-b)
	hydr_conduct = hydr_conduct_sat * (vwc/vwc_sat)^(2b+3)
	hydr_diffus = b* hydr_diffus * (-1*psi) * (vwc/vwc_sat)^(b+2)

	For further discussion see:
	Cosby, B.J., G.M. Hornberger, R.B. Clapp, and T.R. Ginn, 1984.  A     
	   statistical exploration of the relationships of soil moisture      
	   characteristics to the physical properties of soils.  Water Res.
	   Res. 20:682-690.
	
	Saxton, K.E., W.J. Rawls, J.S. Romberger, and R.I. Papendick, 1986.
		Estimating generalized soil-water characteristics from texture.
		Soil Sci. Soc. Am. J. 50:1031-1036.

	Balsamo et al 2009 - 
	A Revised Hydrology for the ECMWF Model - Verification from Field Site to Water Storage IFS - JHydromet.pdf

	Chen and Dudhia 2001 - 
	Coupling an Advanced Land Surface-Hydrology Model with the PMM5 Modeling System Part I - MonWRev.pdf


	*/

	int ok=1;
	int layer;
	double vwc_avg, psi_avg, tsoil_avg;
	vwc_avg=psi_avg=tsoil_avg=0;

	/* ***************************************************************************************************** */
	/* calculating vwc psi and hydr. cond. to every layer */


	for (layer=0; layer < N_SOILLAYERS; layer++)
	{
		
		/* convert kg/m2 --> m3/m2 --> m3/m3 */
		epv->vwc[layer] = ws->soilw[layer] / (water_density * sitec->soillayer_thickness[layer]);


	   
		/* psi, hydr_conduct and hydr_diffus ( Cosby et al.) from vwc ([1MPa=100m] [m/s] [m2/s] */
		epv->psi[layer]  = sitec->psi_sat[layer] * pow( (epv->vwc[layer] /sitec->vwc_sat[layer]), -1* sitec->soil_b[layer]);
		
	
		/* pF from psi: cm from MPa */
		epv->pF[layer] =log10(fabs(10000*epv->psi[layer] ));
	

		/*  calculating averages */

		if (layer < N_SOILLAYERS-1)
		{
			tsoil_avg += metv->tsoil[layer] * (sitec->soillayer_thickness[layer] / sitec->soillayer_depth[N_SOILLAYERS-2]);
			vwc_avg	  += epv->vwc[layer]    * (sitec->soillayer_thickness[layer] / sitec->soillayer_depth[N_SOILLAYERS-2]);
			psi_avg	  += epv->psi[layer]    * (sitec->soillayer_thickness[layer] / sitec->soillayer_depth[N_SOILLAYERS-2]);
		}
       

		/* CONTROL - unrealistic VWC content (higher than saturation value) */
		if (epv->vwc[layer] > sitec->vwc_sat[layer])       
		{
			if (epv->vwc[layer] - sitec->vwc_sat[layer] > 0.001)       
			{
				printf("Fatal error: soil water content is higher than saturation value (multilayer_hydrolparams.c)\n");
				ok=0;	
			}
			else
			{
				ws->deeppercolation_snk += epv->vwc[layer] - sitec->vwc_sat[layer];
				epv->vwc[layer]         = sitec->vwc_sat[layer];
				ws->soilw[layer]        = epv->vwc[layer] * sitec->soillayer_thickness[layer] * water_density;
			}
		}


	}



  	epv->vwc_avg	= vwc_avg;
	epv->psi_avg	= psi_avg;
	metv->tsoil_avg = tsoil_avg;


	return(!ok);
}

