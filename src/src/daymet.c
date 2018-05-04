/*daymet.c
transfer one day of meteorological data from metarr struct to metv struct

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2018, D. Hidy [dori.hidy@gmail.com]
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

int daymet(const metarr_struct* metarr, const siteconst_struct* sitec, metvar_struct* metv, double* tair_annavg_ptr, double snoww, int metday)
{
	/* generates daily meteorological variables from the metarray struct */
	double tmax,tmin,tavg,tavg11_ra,tavg30_ra,tavg10_ra,tday,tdiff, tsoil_top;
	int ok=1;


	/* convert prcp from cm --> kg/m2 */
	metv->prcp = metarr->prcp[metday] * 10.0;

	/* air temperature calculations (all temperatures deg C) */
	metv->tmax			= tmax    = metarr->tmax[metday];
	metv->tmin			= tmin    = metarr->tmin[metday];
	metv->tavg		    = tavg  = metarr->tavg[metday];

	metv->tday			= tday	= metarr->tday[metday];
	metv->tnight		= (tday + tmin) / 2.0;
	metv->tavg11_ra	= tavg11_ra = metarr->tavg11_ra[metday];
	metv->tavg30_ra	= tavg30_ra = metarr->tavg30_ra[metday];
	metv->tavg10_ra	= tavg10_ra = metarr->tavg10_ra[metday];

	metv->F_temprad     = metarr->F_temprad[metday];
	metv->F_temprad_ra  = metarr->F_temprad_ra[metday];




	
	/* **********************************************************************************/
	/* new estimation of tsoil () - on the first day original method is used */
	
	/* ORIGINAL: for this version, an 11-day running weighted average of daily average temperature is used as the soil temperature at 10 cm.
	For days 1-10, a 1-10 day running weighted average is used instead.The tail of the running average is weighted linearly from 1 to 11.
	There are no corrections for snowpack or vegetation cover.	*/

	if (metday < 1)
	{
		tsoil_top = metv->tavg11_ra;
		/* soil temperature correction using difference from annual average tair */
		tdiff =  *tair_annavg_ptr - tsoil_top;
		
		if (snoww)
		{
			tsoil_top += 0.2 * tdiff;
		}
		else
		{
			tsoil_top += 0.1 * tdiff;
		}

		metv->tsoil_surface     = tsoil_top;

		metv->tsoil_surface_pre = tsoil_top;
		metv->tday_pre          = (metv->tmax + metv->tmin)/2.;
	}

	/* 3 m below the ground surface (last layer) is specified by the annual mean surface air temperature */
	metv->tsoil[N_SOILLAYERS-1] = sitec->tair_annavg;
	

	
	/* **********************************************************************************/
	
	/* daylight average vapor pressure deficit (Pa) */
	metv->vpd = metarr->vpd[metday];

	/* daylight average	shortwave flux density (W/m2) */
	metv->swavgfd =  metarr->swavgfd[metday];
	
	/* PAR (W/m2) */
	metv->par = metarr->par[metday];

	/* daylength (s) */
	metv->dayl = metarr->dayl[metday];


	return (!ok);
}
