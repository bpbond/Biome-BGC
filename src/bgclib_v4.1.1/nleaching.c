/* 
nleaching.c
daily nitrogen leaching to groundwater

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

int nleaching(nstate_struct* ns, nflux_struct* nf, wstate_struct* ws, 
wflux_struct* wf)
{
	int ok=1;
	double soilwater_nconc;
	
	/* N leaching flux is calculated after all the other nfluxes are reconciled
	to avoid the possibility of removing more N than is there. This follows
	the implicit logic of precedence for soil mineral N resources:
	1) microbial processes and plant uptake (competing)
	2) leaching
	
	leaching happens when there is outflow, as a function of the
	presumed proportion of the soil mineral N pool which is soluble
	(nitrates), the soil water content, and the outflow */
	if (wf->soilw_outflow)
	{
		soilwater_nconc = MOBILEN_PROPORTION * ns->sminn/ws->soilw;
		nf->sminn_leached = soilwater_nconc * wf->soilw_outflow;
		/* update state variables */
		ns->nleached_snk += nf->sminn_leached;
		ns->sminn        -= nf->sminn_leached;
	}
	return (!ok);
}

