/* 
daymet.c
transfer one day of meteorological data from metarr struct to metv struct

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

int daymet(const metarr_struct* metarr, metvar_struct* metv, int metday)
{
	/* generates daily meteorological variables from the metarray struct */
	int ok=1;
	double tmax,tmin,tavg,tday;
	
	/* convert prcp from cm --> kg/m2 */
	metv->prcp = metarr->prcp[metday] * 10.0;

	/* air temperature calculations (all temperatures deg C) */
	metv->tmax = tmax = metarr->tmax[metday];
	metv->tmin = tmin = metarr->tmin[metday];
	metv->tavg = tavg = metarr->tavg[metday];
	metv->tday = tday = 0.212 * (tmax - tavg) + tavg;
	metv->tnight = (tday + tmin) / 2.0;

	/* for this version, an 11-day running weighted average of daily
	average temperature is used as the soil temperature at 10 cm.
	For days 1-10, a 1-10 day running weighted average is used instead.
	The tail of the running average is weighted linearly from 1 to 11.
	There are no corrections for snowpack or vegetation cover. 
	*/
	metv->tsoil = metarr->tavg_ra[metday];

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
