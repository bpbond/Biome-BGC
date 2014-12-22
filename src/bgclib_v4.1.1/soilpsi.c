/* 
soilpsi.c
soil water potential as a function of volumetric water content and
constants related to texture

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

int soilpsi(const siteconst_struct* sitec, double soilw, double* psi,
double* vwc_out)
{
	/* given a list of site constants and the soil water mass (kg/m2),
	this function returns the soil water potential (MPa)
	inputs:
	ws.soilw           (kg/m2) water mass per unit area
	sitec.soil_depth   (m)     effective soil depth               
	sitec.soil_b       (DIM)   slope of log(psi) vs log(rwc)
	sitec.vwc_sat      (DIM)   volumetric water content at saturation
	sitec.psi_sat      (MPa)   soil matric potential at saturation
	output:
	psi_s              (MPa)   soil matric potential

	uses the relation:
	psi_s = psi_sat * (vwc/vwc_sat)^b

	For further discussion see:
	Cosby, B.J., G.M. Hornberger, R.B. Clapp, and T.R. Ginn, 1984.  A     
	   statistical exploration of the relationships of soil moisture      
	   characteristics to the physical properties of soils.  Water Res.
	   Res. 20:682-690.
	
	Saxton, K.E., W.J. Rawls, J.S. Romberger, and R.I. Papendick, 1986.
		Estimating generalized soil-water characteristics from texture.
		Soil Sci. Soc. Am. J. 50:1031-1036.
	*/

	int ok=1;
	double vwc;

	/* convert kg/m2 --> m3/m2 --> m3/m3 */
	vwc = soilw / (1000.0 * sitec->soil_depth);
	*vwc_out = vwc;

	/* calculate psi */
	*psi = sitec->psi_sat * pow((vwc/sitec->vwc_sat), sitec->soil_b);

	return(!ok);
}

