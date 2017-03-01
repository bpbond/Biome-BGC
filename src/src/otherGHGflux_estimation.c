 /*
otherGHGflux_estimation.c
Estimation of the N2O and CH4 flux based on empirical fuctions of C:N, BD, VWC and TS (Hashimoto el al. 2011)

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.4
Copyright 2017, D. Hidy [dori.hidy@gmail.com]
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

int otherGHGflux_estimation(const epconst_struct* epc, double CNR, double BD, double VWC, double T, double* N2O_flux, double* CH4_flux)
{
	int ok=1;
	double WFPS, f_CNR, f_BD, f1_WFPS, f2_WFPS, f_WPFS, f_TS;


	/* -------------------------------------- N2O flux --------------------------------------------------*/

	/* function of C:N ratio of soil */
	f_CNR = epc->N_pCNR1 * exp(-1* epc->N_pCNR2 * CNR);
	
	/* function of VWC using water filled pore space */
	WFPS = VWC / (1- BD/TP);

	f1_WFPS = pow(((WFPS - epc->N_pVWC1)/(epc->N_pVWC2 - epc->N_pVWC1)),epc->N_pVWC4);
	f2_WFPS = pow(((WFPS - epc->N_pVWC3)/(epc->N_pVWC2 - epc->N_pVWC3)),
		          -epc->N_pVWC4 * (epc->N_pVWC2 - epc->N_pVWC3)/(epc->N_pVWC2 - epc->N_pVWC1));

	f_WPFS = f1_WFPS*f2_WFPS;

	/* function of soil temperature */
	f_TS = exp(epc->N_pTS * T);

	/* summarized function */
	*N2O_flux = f_CNR * f_WPFS * f_TS * n_hour_in_day * 1e-3; // daily data, mikrogram to milligramm

	

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
	*CH4_flux = f_BD * f_WPFS * f_TS * n_hour_in_day * 1e-3; // daily data, mikrogram to milligramm




	return (!ok);

}
