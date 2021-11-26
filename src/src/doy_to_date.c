/* date_to_doy.c
calculate date from DOY (from1=0: yday=0-364, from1=1: yday=1-365)

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

int doy_to_date(int yday, int* month, int* day, int from1)
{
	int nd, done, errorCode;
	int endday[nMONTHS_OF_YEAR] = {30,58,89,119,150,180,211,242,272,303,333,364};
	
	errorCode=nd=done=0;
	yday-=from1;
	while (done == 0 && nd < nMONTHS_OF_YEAR) 
	{
		if (yday <= endday[nd])
		{
			*month = nd+1;
			if (nd > 0)
				*day= yday-endday[nd-1];
			else
				*day=yday+1;
			done = 1;
		}
		else
			nd=nd+1;
	}


	return (errorCode);

}