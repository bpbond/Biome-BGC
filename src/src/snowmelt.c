/* 
snowmelt.c
daily snowmelt and sublimation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
BBGC MuSo v4
Copyright 2000, Peter E. Thornton
Numerical Terradynamics Simulation Group
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
#include "bgc_func.h"
#include "bgc_constants.h"

int snowmelt(const metvar_struct* metv, wflux_struct* wf, double snoww)
{
	/* temperature and radiation snowmelt, 
	from Joseph Coughlan PhD thesis, 1991 */

	int ok=1;
	double rn;								/* (kJ/m2/d) incident radiation */
	double melt, tmelt, rmelt, rsub;

	/* canopy transmitted radiaiton: convert from W/m2 --> KJ/m2/d */	
	rn = metv->swtrans * metv->dayl * sn_abs * 0.001;
	tmelt = rmelt = rsub = 0.0;
	
	if (metv->tavg > 0.0)  /* temperature and radiaiton melt from snowpack */

	{
		tmelt = tcoef * (metv->tmax + metv->tmin)/2.;
		rmelt = rn / lh_fus;
		melt = tmelt+rmelt;
	
		if (melt > snoww)
			melt = snoww;
	
		wf->snoww_to_soilw = melt;
	}
	else  /* sublimation from snowpack */
	{
		rsub = rn / lh_sub;
	
		if (rsub > snoww)
			rsub = snoww;
	
		wf->snoww_subl = rsub;
	}	
	
	return (!ok);
}
