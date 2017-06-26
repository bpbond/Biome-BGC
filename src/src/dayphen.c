/* 
dayphen.c
transfer one day of phenological data from phenarr struct to phen struct

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.6
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2017, D. Hidy [dori.hidy@gmail.com]
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

int dayphen(const phenarray_struct* phenarr, phenology_struct* phen, int metday, int metyr)
{
	int ok=1;
	
	phen->remdays_curgrowth = (double)(phenarr->remdays_curgrowth[metday]);
	phen->remdays_transfer = (double)(phenarr->remdays_transfer[metday]);
	phen->remdays_litfall = (double)(phenarr->remdays_litfall[metday]);
	phen->predays_transfer = (double)(phenarr->predays_transfer[metday]);
	phen->predays_litfall = (double)(phenarr->predays_litfall[metday]);
	
	phen->onday = (int)(phenarr->onday_arr[metyr]);
	phen->offday = (int)(phenarr->offday_arr[metyr]);
	return(!ok);


}
	
