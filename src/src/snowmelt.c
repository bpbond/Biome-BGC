/* 
snowmelt.c
daily snowmelt and sublimation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.3
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2016, D. Hidy [dori.hidy@gmail.com]
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
