 /*
CH4flux_estimation.c
Estimation of the N2O and CH4 flux based on empirical fuctions of C:N, BD, VWC and TS (Hashimoto el al. 2011)

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Copyright 2020, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "ini.h"
#include "bgc_struct.h"
#include "bgc_func.h"
#include "bgc_constants.h"

int CH4flux_estimation(const soilprop_struct* sprop, int layer, double VWC, double T, double* CH4_flux)
{
	int errorCode=0;
	double WFPS, f_BD, f1_WFPS, f2_WFPS, f_WPFS, f_TS;



	/* -------------------------------------- CH4 flux --------------------------------------------------*/

	/* function of C:N ratio of soil */
 	f_BD = sprop->pBD1_CH4 * exp(-1* sprop->pBD2_CH4 * sprop->BD[layer]);
	
	/* function of VWC using water filled pore space */
	WFPS = VWC / sprop->VWCsat[layer];
	if (WFPS > 1) WFPS = 1;

	f1_WFPS = pow(((WFPS - sprop->pVWC1_CH4)/(sprop->pVWC2_CH4 - sprop->pVWC1_CH4)),sprop->C_pVWC4);
	f2_WFPS = pow(((WFPS - sprop->pVWC3_CH4)/(sprop->pVWC2_CH4 - sprop->pVWC3_CH4)),
		          -sprop->C_pVWC4 * (sprop->pVWC2_CH4 - sprop->pVWC3_CH4)/(sprop->pVWC2_CH4 - sprop->pVWC1_CH4));

	f_WPFS = f1_WFPS*f2_WFPS;

	/* function of soil temperature */
	f_TS = exp(sprop->pTS_CH4 * T);

	/* summarized function */
	*CH4_flux = f_BD * f_WPFS * f_TS * nHOURS_IN_DAY * 1e-9; // daily data, mikrogram to kilogramm




	return (errorCode);

}
