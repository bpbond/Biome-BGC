/* 
multilayer_hydrolparams.c
calcultion of soil water potential, hydr. conductivity and hydr. diffusivity as a function of volumetric water content and
constants related to texture

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
#include "bgc_func.h"
#include "bgc_constants.h"

int multilayer_hydrolparams(const siteconst_struct* sitec, const soilprop_struct* sprop, wstate_struct* ws, epvar_struct* epv)
{
	/* given a list of site constants and the soil water mass (kg/m2),
	this function returns the soil water potential (MPa)
	inputs:
	ws.soilw				     (kg/m2) water mass per unit area
	sprop.max_rootzone_depth     (m)     maximum depth of rooting zone               
	sprop.soilB				 (dimless)   slope of log(PSI) vs log(rwc)
	sprop.VWCsat				 (m3/m3)   volumetric water content at saturation
	sprop.PSIsat			   	(MPa)   soil matric potential at saturation

	uses the relation:
	PSI = PSIsat * (VWC/VWCsat)^(-b)
	hydrCONDUCT = hydrCONDUCTsat * (VWC/VWCsat)^(2b+3)
	hydrDIFFUS = b* hydrDIFFUS * (-1*PSI) * (VWC/VWCsat)^(b+2)

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

	int errorCode=0;
	int layer;


	/* ***************************************************************************************************** */
	/* calculating VWC PSI and hydr. cond. to every layer */


	for (layer=0; layer < N_SOILLAYERS; layer++)
	{
		
		/* convert kg/m2 --> m3/m2 --> m3/m3 */
		epv->VWC[layer] = ws->soilw[layer] / (water_density * sitec->soillayer_thickness[layer]);
		
		epv->WFPS[layer]	            = epv->VWC[layer] / sprop->VWCsat[layer];	
   
		/* PSI, hydrCONDUCT and hydrDIFFUS ( Cosby et al.) from VWC ([1MPa=100m] [m/s] [m2/s] */
		epv->PSI[layer]  = sprop->PSIsat[layer] * pow( (epv->VWC[layer] /sprop->VWCsat[layer]), -1* sprop->soilB[layer]);
		
	
		/* pF from PSI: cm from MPa */
		epv->pF[layer] =log10(fabs(10000*epv->PSI[layer] ));
	


		/* CONTROL - unrealistic VWC content (higher than saturation value) */
		if (epv->VWC[layer] > sprop->VWCsat[layer])       
		{
			if (epv->VWC[layer] - sprop->VWCsat[layer] > 0.001)       
			{
				printf("\n");
				printf("ERROR: soil water content is higher than saturation value (multilayer_hydrolparams.c)\n");
				errorCode=1;	
			}
			else
			{
				ws->deeppercolation_snk += epv->VWC[layer] - sprop->VWCsat[layer];
				epv->VWC[layer]         = sprop->VWCsat[layer];
				ws->soilw[layer]        = epv->VWC[layer] * sitec->soillayer_thickness[layer] * water_density;
			}
		}


	}


	 

	return(errorCode);
}

