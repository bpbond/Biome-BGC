/* 
outflow.c
daily hydrologic outflow

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.1.1
Copyright 2000, Peter E. Thornton
Numerical Terradynamics Simulation Group (NTSG)
School of Forestry, University of Montana
Missoula, MT 59812
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

int outflow(const siteconst_struct* sitec, const wstate_struct* ws,
wflux_struct* wf)
{
	/* calculates the outflow flux from the difference between soilwater
	and maximum soilwater */
	int ok=1;
	/* water in excess of saturation to outflow */
	if (ws->soilw > sitec->soilw_sat)  
	{
		wf->soilw_outflow = ws->soilw - sitec->soilw_sat;
	}
	/* slow drainage from saturation to field capacity */
	else if (ws->soilw > sitec->soilw_fc)
	{
		wf->soilw_outflow = 0.5 * (ws->soilw - sitec->soilw_fc);
	}
	/* otherwise, no outflow */
	else
	{
		wf->soilw_outflow = 0.0;
	}

	return(!ok);
}

