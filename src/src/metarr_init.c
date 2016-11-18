/*
metarr_init.c
Initialize meteorological data arrays for pointbgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.1
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2016, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "ini.h"
#include "bgc_struct.h"
#include "bgc_constants.h"
#include "pointbgc_struct.h"
#include "pointbgc_func.h"
#include "misc_func.h"

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

int metarr_init(file metf, metarr_struct* metarr, const climchange_struct* scc,const siteconst_struct* sitec,int nyears) 
{
	int ok = 1;
	int i;
	int ndays;
	int year;
	double tmax = 0;
	double tmin = 0;
	double tday = 0;
	double prcp = 0;
	double vpd = 0;
	double swavgfd = 0;
	double dayl = 0;
	double sw_MJ;

	
	ndays = NDAY_OF_YEAR * nyears;

	/* allocate space for the metv arrays */
	if (ok)
	{
		metarr->tmax = (double*) malloc(ndays * sizeof(double));
		if (!metarr->tmax)
		{
			printf("Error allocating for tmax array\n");
			ok=0;
		}
	}

	if (ok)
	{
		metarr->tmin = (double*) malloc(ndays * sizeof(double));
		if (!metarr->tmin)
		{
			printf("Error allocating for tmin array\n");
			ok=0;
		}
	}

	if (ok)
	{
		metarr->prcp = (double*) malloc(ndays * sizeof(double));
		if (!metarr->prcp)
		{
			printf("Error allocating for prcp array\n");
			ok=0;
		}
	}

	if (ok)
	{
		metarr->vpd = (double*) malloc(ndays * sizeof(double));
		if (!metarr->vpd)
		{
			printf("Error allocating for vpd array\n");
			ok=0;
		}
	}

	if (ok)
	{
		metarr->tday = (double*) malloc(ndays * sizeof(double));
		if (!metarr->tday)
		{
			printf("Error allocating for tday array\n");
			ok=0;
		}
	}

	if (ok)
	{
		metarr->tavg = (double*) malloc(ndays * sizeof(double));
		if (!metarr->tavg)
		{
			printf("Error allocating for tavg11 array\n");
			ok=0;
		}
	}

	if (ok)
	{
		metarr->tavg11_ra = (double*) malloc(ndays * sizeof(double));
		if (!metarr->tavg11_ra)
		{
			printf("Error allocating for tavg11_ra array\n");
			ok=0;
		}
	}

	/* Hidy 2015 - new arrays */
	
	if (ok)
	{
		metarr->tavg30_ra = (double*) malloc(ndays * sizeof(double));
		if (!metarr->tavg30_ra)
		{
			printf("Error allocating for tavg30_ra array\n");
			ok=0;
		}
	}

	if (ok)
	{
		metarr->tavg10_ra = (double*) malloc(ndays * sizeof(double));
		if (!metarr->tavg10_ra)
		{
			printf("Error allocating for tavg10_ra array\n");
			ok=0;
		}
	}

	if (ok)
	{
		metarr->F_temprad = (double*) malloc(ndays * sizeof(double));
		if (!metarr->F_temprad)
		{
			printf("Error allocating for F_temprad array\n");
			ok=0;
		}
	}

	if (ok)
	{
		metarr->F_temprad_ra = (double*) malloc(ndays * sizeof(double));
		if (!metarr->F_temprad_ra)
		{
			printf("Error allocating for F_temprad_ra array\n");
			ok=0;
		}
	}

	if (ok)
	{
		metarr->swavgfd = (double*) malloc(ndays * sizeof(double));
		if (!metarr->swavgfd)
		{
			printf("Error allocating for swavgfd array\n");
			ok=0;
		}
	}
	
	if (ok)
	{
		metarr->par = (double*) malloc(ndays * sizeof(double));
		if (!metarr->par)
		{
			printf("Error allocating for par array\n");
			ok=0;
		}
	}
	
	if (ok)
	{
		metarr->dayl = (double*) malloc(ndays * sizeof(double));
		if (!metarr->dayl)
		{
			printf("Error allocating for dayl array\n");
			ok=0;
		}
	}

	
	/* begin daily loop: read input file, generate array values */
	for (i=0 ; ok && i<ndays ; i++)
	{
		/* read year field  */
		if (fscanf(metf.ptr,"%d",&year)==EOF)
		{
			printf("Error reading year field: metarr_init()\n");
			ok=0;
		}
		/* read tmax, tmin, prcp, vpd, and srad */
		/* the following scan statement discards the tday field in the
		standard MTCLIM version 3.1 input file */
		if (ok && fscanf(metf.ptr,"%*d%lf%lf%lf%lf%lf%lf",
			&tmax,&tmin,&tday,&prcp,&vpd,&swavgfd)==EOF)
		{
			printf("Error reading met file, metarr_init()\n");
			ok=0;
		}
		/* read daylength */
		if (ok && fscanf(metf.ptr,"%lf",&dayl)==EOF)
		{
			printf("Error reading met file, metv_init()\n");
			ok=0;
		}
		
		/* Hidy 2013 - control to avoid negative meteorological data */
 		if (prcp < 0 || vpd < 0 || swavgfd < 0 || dayl < 0)
		{
			printf("Error in met file: negative prcp/vpd/swavgfd/dayl, metv_init()\n");
			ok=0;
		}

		/* apply the climate change scenario and store */
		metarr->tmax[i] = tmax + scc->s_tmax;
		metarr->tmin[i] = tmin + scc->s_tmin;
		metarr->tday[i] = tday; // pasted by Hidy 2012.;
		metarr->tavg[i] = (tmax + tmin) / 2.0;
		metarr->prcp[i] = prcp * scc->s_prcp;
		metarr->vpd[i] = vpd * scc->s_vpd;
		metarr->swavgfd[i] = swavgfd * scc->s_swavgfd;
		metarr->par[i] = swavgfd * 0.45 * scc->s_swavgfd;
		metarr->dayl[i] = dayl;

		/* factor for soil temp. calculation - Hidy 2015 */
		sw_MJ = swavgfd * dayl / 1000000;
		if (i == 0)
			metarr->F_temprad[i] = metarr->tavg[i] +  (tmax - metarr->tavg[i]) * sqrt(sw_MJ*0.03);
		else
			metarr->F_temprad[i] = (1 - sitec->sw_alb) * (metarr->tavg[i] +  (tmax - metarr->tavg[i]) * sqrt(sw_MJ*0.03))
			                      + sitec->sw_alb * metarr->F_temprad[i-1];

	}

	/* perform running averages of daily average temperature for 
	use in soil temperature routine. 

	This implementation uses a linearly ramped 11-day running average 
	of daily mean air temperature, with days 1-10 based on a 1-10 day
	running average, respectively. 
	*/
	
	if (ok && run_avg(metarr->tavg, metarr->tavg11_ra, ndays, 11, 1))
	{
		printf("Error: run_avg() in metv_init.c \n");
		ok = 0;
	}
	
	/* Hidy 2015- new averages */
	if (ok && run_avg(metarr->tavg, metarr->tavg30_ra, ndays, 30, 0))
	{
		printf("Error: run_avg() in metv_init.c \n");
		ok = 0;
	}

	if (ok && run_avg(metarr->tavg, metarr->tavg10_ra, ndays, 10, 0))
	{
		printf("Error: run_avg() in metv_init.c \n");
		ok = 0;
	}

	if (ok && run_avg(metarr->F_temprad, metarr->F_temprad_ra, ndays, 5, 0))
	{
		printf("Error: run_avg() in metv_init.c \n");
		ok = 0;
	}
	return (!ok);
}
