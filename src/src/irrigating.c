/* 
irrigating.c
irrigating  - irrigating seeds in soil - increase transfer pools

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

int irrigating(const control_struct* ctrl, const irrigating_struct* IRG, epvar_struct* epv, wstate_struct* ws, wflux_struct* wf)
{
	/* irrigating parameters */	   
	
	int errorCode=0;
	int condIRG;
	double critVWCbef, critVWCaft, critSOILWaft, condIRG_amount;

	int md, year;

	year = ctrl->simstartyear + ctrl->simyr;
	md = IRG->mgmdIRG-1;


	critVWCbef=condIRG_amount=critSOILWaft=0;
	condIRG=0;

	/*  1. CALCULATING FLUXES: amount of water (kgH2O) * (%_to_prop) */
	if (IRG->IRG_num && md >= 0)
	{
		if (year == IRG->IRGyear_array[md] && ctrl->month == IRG->IRGmonth_array[md] && ctrl->day == IRG->IRGday_array[md])
		{
 			wf->IRG_to_prcp=IRG->IRGquantity_array[md];
		}
		
		else
			wf->IRG_to_prcp=0;
	}


	/* 2. conditional irrigating: in case of dry soil */
	if (IRG->condIRG_flag)
	{
		/* in case of critical VWCratio is used */
		if (IRG->condIRG_flag == 1)
		{
			critVWCbef = epv->VWCwp_RZ + IRG->befVWCratio_condIRG * (epv->VWCfc_RZ - epv->VWCwp_RZ);
			if (epv->VWC_RZ < critVWCbef) condIRG = 1;
		}
		/* in case of critical SMSI is used */
		else
		{
			if (epv->SMSI > IRG->befSMSI_condIRG) condIRG = 1;
		}

		/* if condIRG - calculation of irrigating amount */
		if (condIRG && epv->rootdepth)
		{
			critVWCaft = epv->VWCwp_RZ + IRG->aftVWCratio_condIRG * (epv->VWCfc_RZ - epv->VWCwp_RZ);

			critSOILWaft = critVWCaft * epv->rootlength * water_density;
			condIRG_amount = critSOILWaft - ws->soilw_RZ;
			if (IRG->condIRG_flag == 2)
			{
				if (ctrl->onscreen && ctrl->spinup == 0) printf("WARNING: too low befSMSI_condIRG parameter in conditional IRRIGATING\n");
				condIRG_amount = 0;
			}
		}
		
		if (condIRG_amount < 0)
		{
			printf("ERROR in conditional irrigating calculation (irrigating.c)\n");
			errorCode=1;
		}

		/* irrigating amount is added to IRG_to_prcp (limitation is possible based on condIRG parameters*/	
		if (condIRG_amount)
		{
			if (condIRG_amount <= IRG->maxAMOUNT_condIRG)
			{
				wf->IRG_to_prcp = condIRG_amount;
				if (ctrl->onscreen && ctrl->spinup == 0) printf("conditional IRRIGATING on %i%s%i\n", ctrl->month, "/", ctrl->day);
			}
			else
			{
				wf->IRG_to_prcp = IRG->maxAMOUNT_condIRG;
				if (ctrl->onscreen && ctrl->spinup == 0) printf("limited conditional IRRIGATING on %i%s%i\n", ctrl->month, "/", ctrl->day);
			}

			ws->condIRGsrc += wf->IRG_to_prcp;
		}

	}
	

   return (errorCode);
}
	