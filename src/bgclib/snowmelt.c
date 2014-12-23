/* 
snowmelt.c
daily snowmelt and sublimation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "bgc.h"

int snowmelt(const metvar_struct* metv, wflux_struct* wf, double snoww)
{
	/* temperature and radiation snowmelt, 
	from Joseph Coughlan PhD thesis, 1991 */

	int ok=1;
	static double sn_abs = 0.6;    /* absorptivity of snow */
	static double lh_fus = 335.0;  /* (kJ/kg) latent heat of fusion */
	static double lh_sub = 2845.0; /* (kJ/kg) latent heal of sublimation */
	static double tcoef  = 0.65;   /* (kg/m2/deg C/d) temp. snowmelt coef */
	double rn;                     /* (kJ/m2/d) incident radiation */
	double melt, tmelt, rmelt, rsub;

	/* canopy transmitted radiation: convert from W/m2 --> KJ/m2/d */	
	rn = metv->swtrans * metv->dayl * sn_abs * 0.001;
	tmelt = rmelt = rsub = 0.0;
	
	if (metv->tavg > 0.0)  /* temperature and radiaiton melt from snowpack */

	{
		tmelt = tcoef * metv->tavg;
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
