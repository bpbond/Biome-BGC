 /*
CH4flux_estimation.c
Estimation of the N2O and CH4 flux based on empirical fuctions of C:N, BD, VWC and TS (Hashimoto el al. 2011)

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0.
Copyright 2018, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "bgc_struct.h"
#include "bgc_func.h"
#include "bgc_constants.h"

int CH4flux_estimation(const epconst_struct* epc, double BD, double VWC, double T, double* CH4_flux)
{
	int ok=1;
	double WFPS, f_BD, f1_WFPS, f2_WFPS, f_WPFS, f_TS;



	/* -------------------------------------- CH4 flux --------------------------------------------------*/

	/* function of C:N ratio of soil */
	f_BD = epc->C_pBD1 * exp(-1* epc->C_pBD2 * BD);
	
	/* function of VWC using water filled pore space */
	WFPS = VWC / (1- BD/TP);

	f1_WFPS = pow(((WFPS - epc->C_pVWC1)/(epc->C_pVWC2 - epc->C_pVWC1)),epc->C_pVWC4);
	f2_WFPS = pow(((WFPS - epc->C_pVWC3)/(epc->C_pVWC2 - epc->C_pVWC3)),
		          -epc->C_pVWC4 * (epc->C_pVWC2 - epc->C_pVWC3)/(epc->C_pVWC2 - epc->C_pVWC1));

	f_WPFS = f1_WFPS*f2_WFPS;

	/* function of soil temperature */
	f_TS = exp(epc->C_pTS * T);

	/* summarized function */
	*CH4_flux = f_BD * f_WPFS * f_TS * NHOUR_IN_DAY * 1e-6; // daily data, mikrogram to gramm




	return (!ok);

}
