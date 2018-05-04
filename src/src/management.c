/* 
management.c
manages the management issues

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0
Copyright 2018, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "ini.h"     
#include "pointbgc_struct.h"
#include "bgc_struct.h"
#include "pointbgc_func.h"

int management(const control_struct* ctrl, fertilizing_struct* FRZ, grazing_struct* GRZ, harvesting_struct* HRV, 
			   mowing_struct* MOW,planting_struct* PLT, ploughing_struct* PLG, thinning_struct* THN, irrigation_struct* IRG)
{

	/* fertilizing parameters .*/
	int md, ny;
	int ok=1;
	int yday = ctrl->yday;

	
	/* do fertilizing if gapflag=1  */
	FRZ->mgmd=-1;
	if (FRZ->FRZ_flag)
	{
		/* yearly varied (GRZ_flag=2) or constant grazing parameter (GRZ_flag=1) */
		if (FRZ->FRZ_flag > 1) ny=ctrl->simyr;
		else ny=0;

		for(md=0; md < N_MGMDAYS; ++md)
		{
			if (yday == FRZ->FRZdays_array[md][ny])
			{
				FRZ->mgmd=md;
				if (ctrl->onscreen && (ctrl->spinup == 0 || ctrl->simyr == 0)) printf("FERTILIZING on yday %i\n", ctrl->yday);
			}
		}
	}



	/* do grazing if gapflag=1  */
	GRZ->mgmd=-1;
	if (GRZ->GRZ_flag > 0)
	{
		/* yearly varied (GRZ_flag=2) or constant grazing parameter (GRZ_flag=1) */
		if (GRZ->GRZ_flag > 1) ny=ctrl->simyr;
		else ny=0;

		/* determine grazing period */
		for(md=0; md < N_MGMDAYS; ++md)
		{
			if (yday >= GRZ->GRZ_start_array[md][ny] && yday <= GRZ->GRZ_end_array[md][ny] )
			{
				GRZ->mgmd=md;
				if (ctrl->onscreen && (ctrl->spinup == 0 || ctrl->simyr == 0))
				{
					if (yday == GRZ->GRZ_start_array[md][ny]) printf("FIRST DAY OF GRAZING on yday %i\n", ctrl->yday);
			
					if (yday == GRZ->GRZ_end_array[md][ny]) printf("LAST DAY OF GRAZING on yday %i\n", ctrl->yday);
				}
			}
		}
	}
	
	
	/* do harvesting if gapflag=1  */
	HRV->mgmd=-1;
	if (HRV->HRV_flag)
	{
		/* yearly varied (GRZ_flag=2) or constant grazing parameter (GRZ_flag=1) */
		if (HRV->HRV_flag > 1) ny=ctrl->simyr;
		else ny=0;

		for(md=0; md < N_MGMDAYS; ++md)
		{
			if (yday == HRV->HRVdays_array[md][ny])
			{
				HRV->mgmd=md;
				if (ctrl->onscreen && (ctrl->spinup == 0 || ctrl->simyr == 0)) printf("HARVESTING on yday %i\n", ctrl->yday);
			}
		}
	}


	/* do mowing if gapflag=1  */
	MOW->mgmd=-1;
	if (MOW->MOW_flag)
	{
		/* yearly varied (GRZ_flag=2) or constant grazing parameter (GRZ_flag=1) */
		if (MOW->MOW_flag > 1) ny=ctrl->simyr;
		else ny=0;

		for(md=0; md < N_MGMDAYS; ++md)
		{
			if (yday == MOW->MOWdays_array[md][ny])
			{
				MOW->mgmd=md;
				if (ctrl->onscreen && (ctrl->spinup == 0 || ctrl->simyr == 0)) printf("MOWING on yday %i\n", ctrl->yday);
			}
		}
	}


	/* do planting if gapflag=1  */
	PLT->mgmd=-1;
	if (PLT->PLT_flag)
	{
		/* yearly varied (GRZ_flag=2) or constant grazing parameter (GRZ_flag=1) */
		if (PLT->PLT_flag > 1) ny=ctrl->simyr;
		else ny=0;

		for(md=0; md < N_MGMDAYS; ++md)
		{
			if (yday == PLT->PLTdays_array[md][ny])
			{
				PLT->mgmd=md;
				if (ctrl->onscreen && (ctrl->spinup == 0 || ctrl->simyr == 0)) printf("PLANTING on yday %i\n", ctrl->yday);
			}
		}
	}

	/* do ploughing if gapflag=1  */
	PLG->mgmd=-1;
	if (PLG->PLG_flag)
	{
		/* yearly varied (GRZ_flag=2) or constant grazing parameter (GRZ_flag=1) */
		if (PLG->PLG_flag > 1) ny=ctrl->simyr;
		else ny=0;

		for(md=0; md < N_MGMDAYS; ++md)
		{
			if (yday == PLG->PLGdays_array[md][ny])
			{
				PLG->mgmd=md;
				if (ctrl->onscreen && (ctrl->spinup == 0 || ctrl->simyr == 0))  printf("PLOUGHING on yday %i\n", ctrl->yday);
			}
		}
	}


	/* do thinning if gapflag=1  */
	THN->mgmd=-1;
	if (THN->THN_flag)
	{
		/* yearly varied (GRZ_flag=2) or constant  parameter (GRZ_flag=1) */
		if (THN->THN_flag > 1) ny=ctrl->simyr;
		else ny=0;

		for(md=0; md < N_MGMDAYS; ++md)
		{
			if (yday == THN->THNdays_array[md][ny])
			{
				THN->mgmd=md;
				if (ctrl->onscreen && (ctrl->spinup == 0 || ctrl->simyr == 0))  printf("THINNING on yday %i\n", ctrl->yday);
			}
		}
	}

	/* do irrigation if gapflag=1 */
	IRG->mgmd=-1;
	if (IRG->IRG_flag)
	{
		/* yearly varied (GRZ_flag=2) or constant  parameter (GRZ_flag=1) */
		if (IRG->IRG_flag > 1) ny=ctrl->simyr;
		else ny=0;

		for(md=0; md < N_MGMDAYS; ++md)
		{
			if (yday == IRG->IRGdays_array[md][ny])
			{
				IRG->mgmd=md;
				if (ctrl->onscreen && (ctrl->spinup == 0 || ctrl->simyr == 0)) printf("IRRIGATION on yday %i\n", ctrl->yday);
			}
		}
	}

   return (!ok);
}
	