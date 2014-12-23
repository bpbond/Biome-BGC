/*
metarr_init.c
Initialize meteorological data arrays for pointbgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information

Revisions from version 4.1.1:
The ratio of PAR:shortwave radiation is now being used as a
macro definition from bgc_constants.h (RAD2PAR)
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "pointbgc.h"

/* It is assumed here that the meteorological datafile contains the following 
list of variables, with the indicated units.  All other variables in the metv
arrays are derived from this basic set:

VARIABLE    UNITS
yday        (none) (yearday)
prcp        cm     (daily total precipitation, water equivalent)  
tmax        deg C  (daily maximum temperature) 
tmin        deg C  (daily minimum temperature)
VPD         Pa     (daylight average VPD)
swavgfd     W/m2   (daylight average shortwave flux density)
daylength   s      (daylight duration)

*/

int metarr_init(file metf, metarr_struct* metarr, const climchange_struct* scc,
int nyears) 
{
	int ok = 1;
	int i;
	int ndays;
	int year;
	double tmax,tmin,prcp,vpd,swavgfd,dayl;
	
	ndays = 365 * nyears;

	/* allocate space for the metv arrays */
	if (ok && !(metarr->tmax = (double*) malloc(ndays * sizeof(double))))
	{
		bgc_printf(BV_ERROR, "Error allocating for tmax array\n");
		ok=0;
	}
	if (ok && !(metarr->tmin = (double*) malloc(ndays * sizeof(double))))
	{
		bgc_printf(BV_ERROR, "Error allocating for tmin array\n");
		ok=0;
	}
	if (ok && !(metarr->prcp = (double*) malloc(ndays * sizeof(double))))
	{
		bgc_printf(BV_ERROR, "Error allocating for prcp array\n");
		ok=0;
	}
	if (ok && !(metarr->vpd = (double*) malloc(ndays * sizeof(double))))
	{
		bgc_printf(BV_ERROR, "Error allocating for vpd array\n");
		ok=0;
	}
	if (ok && !(metarr->tavg = (double*) malloc(ndays * sizeof(double))))
	{
		bgc_printf(BV_ERROR, "Error allocating for tavg array\n");
		ok=0;
	}
	if (ok && !(metarr->tavg_ra = (double*) malloc(ndays * sizeof(double))))
	{
		bgc_printf(BV_ERROR, "Error allocating for tavg_ra array\n");
		ok=0;
	}
	if (ok && !(metarr->swavgfd = (double*) malloc(ndays * sizeof(double))))
	{
		bgc_printf(BV_ERROR, "Error allocating for swavgfd array\n");
		ok=0;
	}
	if (ok && !(metarr->par = (double*) malloc(ndays * sizeof(double))))
	{
		bgc_printf(BV_ERROR, "Error allocating for par array\n");
		ok=0;
	}
	if (ok && !(metarr->dayl = (double*) malloc(ndays * sizeof(double))))
	{
		bgc_printf(BV_ERROR, "Error allocating for dayl array\n");
		ok=0;
	}
	
	/* begin daily loop: read input file, generate array values */
	for (i=0 ; ok && i<ndays ; i++)
	{
		/* read year field  */
		if (fscanf(metf.ptr,"%d",&year)==EOF)
		{
			bgc_printf(BV_ERROR, "Error reading year field: metarr_init()\n");
			ok=0;
		}
		/* read tmax, tmin, prcp, vpd, and srad */
		/* the following scan statement discards the tday field in the
		standard MTCLIM version 3.1 input file */
		if (ok && fscanf(metf.ptr,"%*d%lf%lf%*lf%lf%lf%lf",&tmax,&tmin,&prcp,&vpd,&swavgfd)==EOF)
		{
			bgc_printf(BV_ERROR, "Error reading met file, metarr_init()\n");
			ok=0;
		}
		/* read daylength */
		if (ok && fscanf(metf.ptr,"%lf",&dayl)==EOF)
		{
			bgc_printf(BV_ERROR, "Error reading met file, metv_init()\n");
			ok=0;
		}

		/* Fixed 02/05/04 */
		if( swavgfd < 0.0 )
		{
			swavgfd = 0.0;
		}

		if( dayl < 0.0 )
		{
			dayl = 0.0;
		}
		
		/* apply the climate change scenario and store */
		metarr->tmax[i] = tmax + scc->s_tmax;
		metarr->tmin[i] = tmin + scc->s_tmin;
		metarr->prcp[i] = prcp * scc->s_prcp;
		metarr->vpd[i] = vpd * scc->s_vpd;
		metarr->swavgfd[i] = swavgfd * scc->s_swavgfd;
		metarr->par[i] = swavgfd * RAD2PAR * scc->s_swavgfd;
		metarr->dayl[i] = dayl;
		metarr->tavg[i] = (metarr->tmax[i] + metarr->tmin[i]) / 2.0;
	}

	/* perform running averages of daily average temperature for 
	use in soil temperature routine. 

	This implementation uses a linearly ramped 11-day running average 
	of daily mean air temperature, with days 1-10 based on a 1-10 day
	running average, respectively. 
	*/
	
	if (ok && run_avg(metarr->tavg, metarr->tavg_ra, ndays, 11, 1))
	{
		bgc_printf(BV_ERROR, "Error: run_avg() in metv_init.c \n");
		ok = 0;
	}
	
	return (!ok);
}
