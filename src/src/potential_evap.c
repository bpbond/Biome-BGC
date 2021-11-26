/* 
potential_evap.c
daily bare soil evaporation

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

int potential_evap(const soilprop_struct* sprop, const metvar_struct* metv, wflux_struct* wf)
{
	int errorCode=0;
	double rbl;					/* (m/s) boundary layer resistance */
	double soilw_evapPOT;			/* (kg/m2/s) potential evaporation (daytime) */
	double rcorr;				 /* correction factor for temp and pressure */
	double lhvap, evap_limit; 
	pmet_struct pmet_in;		/* input structure for penmon function */

	
	/* correct conductances for temperature and pressure based on Jones (1992)
	with standard conditions assumed to be 20 deg C, 101300 Pa */
	rcorr = 1.0/(pow((metv->tday+273.15)/293.15, 1.75) * 101300/metv->pa);

	/* new bare-soil evaporation routine */
	/* first calculate potential evaporation, assuming the resistance for vapor transport is equal to the resistance for sensible heat transport.  
	That is, no additional resistance for vapor transport to the soil surface. This represents evaporation from a wet surface with
	a specified aerodynamic resistance (= boundary layer resistance). The aerodynamic resistance is for now set as a constant, and is
	taken from observations over bare soil in tiger-bush in south-west Niger: rbl = 107 s m-1 (Wallace and Holwill, 1997). */
	rbl = rcorr * sprop->aerodyn_resist;
	
	/* fill the pmet_in structure */
	pmet_in.ta = metv->tday;
	pmet_in.pa = metv->pa;
	pmet_in.vpd = metv->vpd;
	pmet_in.irad = metv->swtrans;
	pmet_in.rv = rbl;
	pmet_in.rh = rbl;

	/* calculate soilw_evapPOT in kg/m2/s */
	penmon(&pmet_in, 0, &soilw_evapPOT);
	

	/* convert to daily total kg/m2 */
	soilw_evapPOT *= metv->dayl;

	if (metv->tavg < 0)
		wf->soilw_evapPOT=0;
	else
		wf->soilw_evapPOT=soilw_evapPOT;

	/* ENERGETIC CONTROL - maximum energy */
    lhvap = 2.5023e6 - 2430.54 * pmet_in.ta;
	evap_limit = (pmet_in.irad * metv->dayl)/lhvap;
	if (wf->soilw_evapPOT > evap_limit)
	{
		wf->soilw_evapPOT = evap_limit;
	}
	
		return(errorCode);
}

