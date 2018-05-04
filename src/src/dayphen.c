/* 
dayphen.c
transfer one day of phenological data from phenarr struct to phen struct

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2018, D. Hidy [dori.hidy@gmail.com]
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

int dayphen(const control_struct* ctrl, const epconst_struct* epc, const phenarray_struct* phenarr, phenology_struct* phen)
{
	int ok=1;


	/* determining onday, offday, n_growthday, n_transferday and n_litfallday value on the first simulation day */
	if (ctrl->yday == 0)
	{
		
		phen->onday         = (double)(phenarr->onday_arr[ctrl->metyr]);
		phen->offday        = (double)(phenarr->offday_arr[ctrl->metyr]);

		if (phen->offday <= phen->onday) 
		{
			if(phen->onday == -1 && phen->offday == -1)
			{
				if (ctrl->metyr == 0) printf("WARNING: user-defined bare-ground run (onday and offday set to -1 in EPC)\n");
			}
			else
			{
				if (ctrl->HRV_flag && ctrl->PLT_flag)
				{
					phen->offday = NDAYS_OF_YEAR + phen->offday;
					printf("WARNING: autumn sowing (onday and offday set to -1 in EPC)\n");
				}
				else
				{
					printf("FATAL ERROR: onday is greater or equal than offday (dayphen.c)\n");
					ok = 0;
				}
			}
		}
		else
			phen->yday_phen     = ctrl->yday;

		phen->n_growthday   = phen->offday - phen->onday + 1;
		phen->n_transferday = floor(phen->n_growthday * epc->transfer_pdays);
		phen->n_litfallday  = floor(phen->n_growthday * epc->litfall_pdays);
	}


	
	

	return(!ok);


}
	
