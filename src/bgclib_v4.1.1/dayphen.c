/* 
dayphen.c
transfer one day of phenological data from phenarr struct to phen struct

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

int dayphen(const phenarray_struct* phenarr, phenology_struct* phen, int metday)
{
	int ok=1;
	
	phen->remdays_curgrowth = (double)(phenarr->remdays_curgrowth[metday]);
	phen->remdays_transfer = (double)(phenarr->remdays_transfer[metday]);
	phen->remdays_litfall = (double)(phenarr->remdays_litfall[metday]);
	phen->predays_transfer = (double)(phenarr->predays_transfer[metday]);
	phen->predays_litfall = (double)(phenarr->predays_litfall[metday]);
	
	return(!ok);
}
	
