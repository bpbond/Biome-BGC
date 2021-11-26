/* 
dayphen.c
transfer one day of phenological data from phenarr struct to phen struct

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

int dayphen(control_struct* ctrl, const epconst_struct* epc, const phenarray_struct* phenarr, const planting_struct* PLT, phenology_struct* phen)
{
	int errorCode=0;
	int nyear;


	/* determining onday, offday, n_growthday, n_transferday and n_litfallday value on the first simulation day */

 	if (PLT->PLT_num)
		nyear = PLT->PLT_num;
	else
		nyear = ctrl->simyears;

	/* spinup: resetting plantyr with simyr */
	if (ctrl->plantyr+1 == nyear)
	{
		ctrl->plantyr = -1;
	}

	if (ctrl->GSI_flag)
	{
		phen->tmin_index        = phenarr->tmin_index[ctrl->simyr][ctrl->yday];
		phen->vpd_index         = phenarr->vpd_index[ctrl->simyr][ctrl->yday];
		phen->dayl_index        = phenarr->dayl_index[ctrl->simyr][ctrl->yday];
		phen->gsi_indexAVG      = phenarr->gsi_indexAVG[ctrl->simyr][ctrl->yday];
		phen->heatsum_index     = phenarr->heatsum_index[ctrl->simyr][ctrl->yday];
		phen->heatsum           = phenarr->heatsum[ctrl->simyr][ctrl->yday];
	}
	else
	{
		phen->tmin_index        = DATA_GAP;
		phen->vpd_index         = DATA_GAP;
		phen->dayl_index        = DATA_GAP;
		phen->gsi_indexAVG      = DATA_GAP;
		phen->heatsum_index     = DATA_GAP;
	}

	/* start of year: when onday or in the first siulation day in year in case of bareground simulation (onday == DATA_GAP and offday ==  DATA_GAP) */
	if (ctrl->plantyr+1 < nyear && phen->onday == -1 && 
		((ctrl->simyr+ctrl->simstartyear == phenarr->onday_arr[ctrl->plantyr+1][0] && 
		 (ctrl->yday == phenarr->onday_arr[ctrl->plantyr+1][1] || phenarr->onday_arr[ctrl->plantyr+1][1] == DATA_GAP))))
	{
		ctrl->plantyr += 1;
		


		if (epc->onday == DATA_GAP)
		{
			if (epc->offday != DATA_GAP)
			{
				printf("FATAL ERROR: if onday is equal to -9999 offday must be equal to -9999 - bare soil simulation (dayphen.c)\n");
				errorCode=1;
			}
			phen->onday         = (double)(phenarr->onday_arr[ctrl->plantyr][1]);
			phen->offday        = (double)(phenarr->offday_arr[ctrl->plantyr][1]);
		}
		else
		{
			phen->onday         = (double)(phenarr->onday_arr[ctrl->plantyr][1]) + nDAYS_OF_YEAR * (phenarr->onday_arr[ctrl->plantyr][0] - ctrl->simstartyear);
			phen->offday        = (double)(phenarr->offday_arr[ctrl->plantyr][1] + nDAYS_OF_YEAR * (phenarr->offday_arr[ctrl->plantyr][0] - ctrl->simstartyear));
		}

		if (phen->offday <= phen->onday && (phen->offday != DATA_GAP && phen->onday != DATA_GAP))
		{
			printf("FATAL ERROR: onday is greater or equal than offday (dayphen.c)\n");
			errorCode=1;
		}

	}

	phen->n_growthday   = phen->offday - phen->onday + 1;


	/* specifying evergreen overrides any user input phenology data, and triggers a very simple treatment of the transfer, litterfall,
	   and current growth signals.  Treatment is the same for woody and non-woody types, and the same for model or user-input phenology */
	if (epc->evergreen)
	{
		phen->n_transferday = nDAYS_OF_YEAR;
		phen->n_litfallday  = nDAYS_OF_YEAR;
	}
	else
	{
		phen->n_transferday = floor(phen->n_growthday * epc->transfer_pdays);
		phen->n_litfallday  = floor(phen->n_growthday * epc->litfall_pdays);
	}
	


	
	

	return(errorCode);


}
	
