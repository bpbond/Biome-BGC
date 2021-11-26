/* 
conduct_limit_factors.c
calculate the limitation factors of conductance
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
#include "bgc_struct.h"
#include "pointbgc_struct.h"
#include "pointbgc_func.h"
#include "bgc_constants.h"


int conduct_limit_factors(file logfile, const control_struct* ctrl, const soilprop_struct* sprop, const epconst_struct* epc, 
						  epvar_struct* epv)
{
	int errorCode=0;
	int layer;
	double VWCsat,VWCfc,VWCwp, VWC_crit1, VWC_crit2;
    
	VWCsat=VWCfc=VWCwp=VWC_crit1=VWC_crit2=0;

	

	/* calculations layer by layer (due to different soil properties) */
	for (layer=0; layer < N_SOILLAYERS; layer++)
	{
		VWCsat = sprop->VWCsat[layer]; 
		VWCfc  = sprop->VWCfc[layer]; 
		VWCwp  = sprop->VWCwp[layer];

		if (epc->VWCratio_crit1 != DATA_GAP)
			VWC_crit1 = VWCwp + epc->VWCratio_crit1 * (VWCfc - VWCwp);
		else
			VWC_crit1 = VWCfc;

		if (epc->VWCratio_crit2 != DATA_GAP)
			VWC_crit2 = VWCfc + epc->VWCratio_crit2 * (VWCsat - VWCfc);
		else
			VWC_crit2 = VWCsat;


		/* CONTROL */
		if (VWC_crit2 > VWCsat)
		{
			printf("\n");
			printf("ERROR: VWC_crit2 data is greater than saturation value in layer:%i\n", layer);
			errorCode=1;
		}

		if (!errorCode && VWC_crit2 < VWC_crit1)
		{
			printf("\n");
			printf("ERROR: VWC_crit1 data is greater then VWC_crit2 data in layer:%i\n", layer);
			errorCode=1;
		}
	
		epv->VWC_crit1[layer]		= VWC_crit1;
		epv->VWC_crit2[layer]		= VWC_crit2;

	}
	if (ctrl->spinup < 2) fprintf(logfile.ptr, "LIMITATION VALUES OF SWC (m3/m3) IN TOP SOIL LAYER \n");  
	if (ctrl->spinup < 2) fprintf(logfile.ptr, "SWC (limit1 and limit2):  %12.3f %12.3f\n", epv->VWC_crit1[0], epv->VWC_crit2[0]);
	if (ctrl->spinup < 2) fprintf(logfile.ptr, " \n");
		
	return (errorCode);
}
