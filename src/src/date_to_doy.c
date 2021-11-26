/* date_to_doy.c
calculate date to DOY

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Copyright 2020, D. Hidy [dori.hidy@gmail.com]
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

int date_to_doy(int month, int day)
{
	int nd;
	int yday = -1;

	int montarr[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

	for (nd=0; nd < month-1; nd++) 
	{
		yday += montarr[nd];
	}

	yday += day;

	return (yday);

}