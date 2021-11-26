/*daymet.c
transfer one day of meteorological data from metarr struct to metv struct

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2020, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "ini.h"
#include "bgc_struct.h"
#include "bgc_func.h"
#include "bgc_constants.h"

int daymet(const control_struct* ctrl,const metarr_struct* metarr, const epconst_struct* epc, const siteconst_struct* sitec, 
	       metvar_struct* metv, double* tair_annavg_ptr, double snoww)
{
	/* generates daily meteorological variables from the metarray struct */
	double tmax,tmin,tavg,tavg11_ra,tavg30_ra,tavg10_ra,tday,tdiff, tsoil_top;
	int errorCode=0;


	/* convert prcp from cm --> kg/m2 */
	metv->prcp = metarr->prcp[ctrl->metday] * 10.0;
	

	/* air temperature calculations (all temperatures deg C) */
	metv->tmax			= tmax    = metarr->tmax[ctrl->metday];
	metv->tmin			= tmin    = metarr->tmin[ctrl->metday];
	metv->tavg		    = tavg  = metarr->tavg[ctrl->metday];

	metv->tday			= tday	= metarr->tday[ctrl->metday];

	metv->tnight		= (tday + tmin) / 2.0;
	metv->tavg11_ra	= tavg11_ra = metarr->tavg11_ra[ctrl->metday];
	metv->tavg30_ra	= tavg30_ra = metarr->tavg30_ra[ctrl->metday];
	metv->tavg10_ra	= tavg10_ra = metarr->tavg10_ra[ctrl->metday];

	metv->F_temprad     = metarr->F_temprad[ctrl->metday];
	metv->F_temprad_ra  = metarr->F_temprad_ra[ctrl->metday];

	if (!ctrl->metday)
	{
		metv->tACCLIM            = metv->tday;
		metv->tACCLIMpre         = metv->tday;
	}
	else
	{
		if (epc->tau)
			metv->tACCLIM = metv->tACCLIMpre + ((metv->tday - metv->tACCLIMpre) / epc->tau);
		else
			metv->tACCLIM = metv->tday; 
		metv->tACCLIMpre = metv->tACCLIM;
	
	}

	
	/* **********************************************************************************/
	/* new estimation of tsoil () - on the first day original method is used */
	
	/* ORIGINAL: for this version, an 11-day running weighted average of daily average temperature is used as the soil temperature at 10 cm.
	For days 1-10, a 1-10 day running weighted average is used instead.The tail of the running average is weighted linearly from 1 to 11.
	There are no corrections for snowpack or vegetation cover.	*/

	if (ctrl->metday < 1)
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
	}

	/* 3 m below the ground surface (last layer) is specified by the annual mean surface air temperature */
	metv->tsoil[N_SOILLAYERS-1] = sitec->tair_annavg;
	

	
	/* **********************************************************************************/
	
	/* daylight average vapor pressure deficit (Pa) */
	metv->vpd = metarr->vpd[ctrl->metday];

	/* daylight average	shortwave flux density (W/m2) */
	metv->swavgfd =  metarr->swavgfd[ctrl->metday];
	
	/* PAR (W/m2) */
	metv->par = metarr->par[ctrl->metday];

	/* daylength (s) */
	metv->dayl = metarr->dayl[ctrl->metday];


	return (errorCode);
}
