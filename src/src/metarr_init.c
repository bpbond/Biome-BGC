/*
metarr_init.c
Initialize meteorological data arrays for pointbgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2018, D. Hidy [dori.hidy@gmail.com]
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

int metarr_init(point_struct* point, metarr_struct* metarr, const climchange_struct* scc,const siteconst_struct* sitec,int nyears) 
{
	int ok = 1;
	int i,j;
	int ndays, nsimdays, dd;
	int year;
	double tmax, tmin, tday, prcp, vpd, swavgfd, dayl;
	double sw_MJ;

	tmax=tmin=tday=prcp=vpd=swavgfd=dayl=0;

	ndays    = NDAYS_OF_YEAR * nyears;
	nsimdays = NDAYS_OF_YEAR * (nyears-1) + point->nday_lastsimyear;

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

	/* new arrays */
	
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
	for (i=0 ; ok && i < ndays ; i++)
	{
		/* if truncated last simyear -> estimated meteorological variables based on data from previous years */
		if (i < nsimdays)
		{
			/* read year field  */
			if (fscanf(point->metf.ptr,"%d",&year)==EOF)
			{
				printf("Error reading year field: metarr_init()\n");
				ok=0;
			}
			/* read tmax, tmin, prcp, vpd, and srad */
			/* the following scan statement discards the tday field in the
			standard MTCLIM version 3.1 input file */
			if (ok && fscanf(point->metf.ptr,"%*d%lf%lf%lf%lf%lf%lf",
				&tmax,&tmin,&tday,&prcp,&vpd,&swavgfd)==EOF)
			{
				printf("Error reading met file, metarr_init()\n");
				ok=0;
			}
			/* read daylength */
			if (ok && fscanf(point->metf.ptr,"%lf",&dayl)==EOF)
			{
				printf("Error reading met file, metv_init()\n");
				ok=0;
			}
		
			/* control to avoid negative meteorological data */
 			if (prcp < 0 || vpd < 0 || swavgfd < 0 || dayl < 0)
			{
				printf("Error in met file: negative prcp/vpd/swavgfd/dayl, metv_init()\n");
				ok=0;
			}
		}
		else
		{
			tmax=tmin=tday=prcp=vpd=swavgfd=dayl=0;
			for (j=0; j<nyears-1;j++)
			{
				dd = (j * NDAYS_OF_YEAR) + (point->nday_lastsimyear - 1);
				tmax     += metarr->tmax[dd];
				tmin     += metarr->tmin[dd];
				tday     += metarr->tday[dd];
				prcp     += metarr->prcp[dd];
				vpd      += metarr->vpd[dd];
				swavgfd  += metarr->swavgfd[dd];
				dayl     += metarr->dayl[dd];
			}
			tmax    = tmax    / (nyears-1);
			tmin    = tmin    / (nyears-1);
			tday    = tday    / (nyears-1);
			prcp    = prcp    / (nyears-1);
			vpd     = vpd     / (nyears-1);
			swavgfd = swavgfd / (nyears-1);
			dayl    = dayl    / (nyears-1);
		}

		/* apply the climate change scenario and store */
		metarr->tmax[i] = tmax + scc->s_tmax;
		metarr->tmin[i] = tmin + scc->s_tmin;
		metarr->tday[i] = tday; 
		metarr->tavg[i] = (tmax + tmin) / 2.0;
		metarr->prcp[i] = prcp * scc->s_prcp;
		metarr->vpd[i] = vpd * scc->s_vpd;
		metarr->swavgfd[i] = swavgfd * scc->s_swavgfd;
		metarr->par[i] = swavgfd * 0.45 * scc->s_swavgfd;
		metarr->dayl[i] = dayl;

		/* factor for soil temp. calculation */
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
	
	/*  new averages */
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
