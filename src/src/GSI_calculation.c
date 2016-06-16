/* 
GSI_calculation.c
Calculation of pheonological state of vegetation (onset and offset day) from Tmin, VPD, daylength and cumulative Tavg 
based on literure (Jolly et al, 2005) and own method. The goal is to replace prephenology.c subroutines becuse the calulation
of the model-defined onset and offset day does not work correctly

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
BBGC MuSo v4
Copyright 2014, D. Hidy (dori.hidy@gmail.com)
Hungarian Academy of Sciences
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include "ini.h"
#include "bgc_struct.h"
#include "pointbgc_struct.h"
#include "bgc_func.h"     
#include "bgc_constants.h"
#include "misc_func.h"


int GSI_calculation(const metarr_struct* metarr, const control_struct* ctrl, const siteconst_struct* sitec, const epconst_struct* epc, 
					GSI_struct* GSI, phenarray_struct* phenarr)

{
	int ok=1;
	int ny, yday, back;

	int firstdayLP = 240;		/* theoretically first day of litterfall */

	/* flags and counters */
	int onday_flag = 0;
	int offday_flag = 0;
	int onday = 0;
	int offday = 0;
	int nyears = ctrl->metyears;
	int n_yday = NDAY_OF_YEAR;
	
	/*  enviromental conditions taken account to calculate onset and offset days */
	double tmax_act, tmin_act, tavg_act, vpd_act, dayl_act, heatsum_act;	

	
	/* threshold limits for each variable, between assuming that phenological activity varied linearly from inactive to unconstrained */
	double base_temp = epc->base_temp;
	double heatsum_limit1 = GSI->heatsum_limit1;
	double heatsum_limit2 = GSI->heatsum_limit2;
	double tmin_limit1 = GSI->tmin_limit1;
	double tmin_limit2 = GSI->tmin_limit2;
	double vpd_limit1 = GSI->vpd_limit1;
	double vpd_limit2 = GSI->vpd_limit2;
	double dayl_limit1 = GSI->dayl_limit1;
	double dayl_limit2 = GSI->dayl_limit2;
	int    n_moving_avg = GSI->n_moving_avg-1;	/* moving averages are calculated from indicatiors 
											       to avoid the effects of single extreme events; -1: number to index */
	double GSI_limit_SGS = GSI->GSI_limit_SGS;	/* when GSI index fisrt time greater that limit -> start of the growing season */
	double GSI_limit_EGS = GSI->GSI_limit_EGS;	/* when GSI index fisrt time less that limit -> end of the growing season */


	/* indexes for the different variables and total index (multiplication of  partial indexes)*/
	double tmin_index = 0;
	double vpd_index = 0; 
	double dayl_index = 0; 
	double heatsum_index = 0; 
	double GSI_index = 0; 

	/* at the presence of snow cover no vegetation period (calculating snow cover from precipitation, Tavg and srad) */
	double snowcover, snowcover_limit, prcp_act, srad_act;
	double rn, tmelt, rmelt, melt, snow_loss, snow_plus, rsub;
	double albedo_sw = sitec->sw_alb;
	
	/* to calculate moving average from total index values */
	double GSI_index_SUM = 0;
	double GSI_index_avg = 0;
	double GSI_index_total = 0;
	
	int *onday_arr = 0;
	int *offday_arr = 0;

	onday_flag = 0;
	offday_flag = 1;

	/* limitation of snow cover */
	snowcover = 0;

	snowcover_limit = GSI->snowcover_limit;
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////allocate memory for arrays containing ondays and offdays /////////////////////////////////////
	if (ok) 
	{
		onday_arr = (int*) malloc((nyears+1) * sizeof(int));
		if (!onday_arr)
		{
			printf("Error allocating for onday_arr, prephenology()\n");
			ok=0;
		}
	}
	if (ok) 
	{
		offday_arr = (int*) malloc((nyears+1) * sizeof(int));
		if (!offday_arr)
		{
			printf("Error allocating for offday_arr, prephenology()\n");
			ok=0;
		}
	}
		
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (ctrl->onscreen)
	{
		fprintf(GSI->GSI_file.ptr, "year yday snowcover heatsum_act tmin_index vpd_index dayl_index heatsum_index GSI_index_avg GSI_index_total\n");
		
		if (ctrl->spinup == 0)
		{	
			printf("-----------------\n");
			printf("ONDAYS AND OFFDAYS\n");
		}
	}
	////////////////////////////////////////////////////////////////////

	for (ny=0; ny<nyears; ny++)
	{
		onday  = 0;
		offday = 0;

		for (yday=0; yday<NDAY_OF_YEAR; yday++)	
		{
		/* ******************************************************************* */
		/* 1. calculation of snow loss and plus*/
		
			tmax_act     = metarr->tmax[ny*n_yday+yday];
			tmin_act     = metarr->tmin[ny*n_yday+yday];
			tavg_act     = (tmax_act+tmin_act)/2.;
			prcp_act     = metarr->prcp[ny*n_yday+yday];	
			srad_act     = metarr->swavgfd[ny*n_yday+yday];
			dayl_act     = metarr->dayl[ny*n_yday+yday];

			/* canopy transmitted radiaiton: convert from W/m2 --> KJ/m2/d */	
			rn = srad_act * (1.0 - albedo_sw) * dayl_act * sn_abs * 0.001;

			/* 1.a. snow loss: melting and sublim. */
			if (tavg_act > 0.0)  
			{	/* temperature melt from snowpack */
				tmelt = tcoef * tavg_act;
				/* radiaiton melt from snowpack */
				rmelt = rn / lh_fus;
				melt = tmelt+rmelt;
				/* snowmelt decreases actual snow pool */
				snow_loss = melt;
			}
			else  
			{	/* sublimation from snowpack */
				rsub = rn / lh_sub;
				/* sublimation decreases actual snow pool */
				snow_loss = rsub;
			}
			if (snow_loss > snowcover) snow_loss = snowcover;	
		
			/* 1.b. snow plus: melting and sublim. */
			if (tavg_act < 0.0)  
			{
				snow_plus = prcp_act*10;
			}
			else  snow_plus = 0;

			snowcover -= snow_loss;
			snowcover += snow_plus;

			/* ******************************************************************* */
			/* 2. calculation of indexes (based on moving averages of evironmental parameters) */

			if (yday < n_moving_avg)
			{
				heatsum_act     = 0;
				tmin_index      = 0;
				heatsum_index   = 0;
				GSI_index_avg   = 0;
				GSI_index_total = 0;
			}
			else
			{
				GSI_index_SUM = 0;
				heatsum_act = 0;
				for (back=0; back<=n_moving_avg; back++)
				{
					/* search actual values of variables */
					tmax_act = metarr->tmax[ny*n_yday+yday-(n_moving_avg-back)];
					tmin_act = metarr->tmin[ny*n_yday+yday-(n_moving_avg-back)];
					tavg_act = (tmax_act+tmin_act)/2.;
					vpd_act  = metarr->vpd[ny*n_yday+yday-(n_moving_avg-back)];
					dayl_act = metarr->dayl[ny*n_yday+yday-(n_moving_avg-back)];
					 
			
					/* ******************************************************************* */
					/* 1.1 calculation of heatsum regarding to the basic temperature and n_moving_avg long period */
					if (tavg_act > base_temp) 
					{
						heatsum_act += (tavg_act-base_temp);
					}

					/* ******************************************************************* */
					/* 1.2. calculation of indexes regarding to the different variables */
					/* !!!!!!!!!!!!!!!!!!!!!!!!!!  A: tmin !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
					if (tmin_act < tmin_limit1)
					{
						tmin_index=0;
					}
					else
					{
						if (tmin_act < tmin_limit2)
						{
							tmin_index = (tmin_act-tmin_limit1)/(tmin_limit2-tmin_limit1);
						}
						else
						{
							tmin_index = 1;
						}

					}
					/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!  B: vpd !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
					if (vpd_act > vpd_limit1)
					{
						vpd_index=0;
					}
					else
					{
						if (vpd_act > vpd_limit2)
						{
							vpd_index = (vpd_act-vpd_limit1)/(vpd_limit2-vpd_limit1);
						}
						else
						{
							vpd_index = 1;
						}

					}
					/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!  C: dayl !!!!!!!!!!!!!!!!!!!!!!!!!!! */
					if (dayl_act < dayl_limit1)
					{
						dayl_index=0;
					}
					else
					{
						if (dayl_act < dayl_limit2)
						{
							dayl_index = (dayl_act-dayl_limit1)/(dayl_limit2-dayl_limit1);
						}
						else
						{
							dayl_index = 1;
						}

					}
						
					
					GSI_index = tmin_index * vpd_index * dayl_index;
					GSI_index_SUM += GSI_index;

				} /* endfor - calculating indexes for the n_moving_average long period  */

				/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  D: heatsum !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
				if (heatsum_act < heatsum_limit1)
				{
					heatsum_index=0;
				}
				else
				{
					if (heatsum_act < heatsum_limit2)
					{
						heatsum_index = (heatsum_act-heatsum_limit1)/(heatsum_limit2-heatsum_limit1);
					}
					else
					{
						heatsum_index = 1;
					}

				}

				GSI_index_avg = GSI_index_SUM / (n_moving_avg+1);
				GSI_index_total = GSI_index_avg * heatsum_index;
				
			} /* endelse - calculating indexes */

			if (onday_flag == 0 && offday_flag == 1 && GSI_index_total > GSI_limit_SGS && yday < firstdayLP && snowcover <= snowcover_limit) 
			{
				onday_flag    = 1;
				offday_flag   = 0;
				onday         = yday;
				onday_arr[ny] = onday;
				if (ctrl->onscreen && ctrl->spinup == 0) printf("Year: %i\n", ctrl->simstartyear+ny);
				if (ctrl->onscreen && ctrl->spinup == 0) printf("onday  - %i\n", onday);
			}

			if (onday_flag == 1 && offday_flag == 0 && GSI_index_total < GSI_limit_EGS && yday > firstdayLP) 
			{
 				onday_flag     = 0;
				offday_flag    = 1;
				offday         = yday;
				offday_arr[ny] = offday;
				if (ctrl->onscreen && ctrl->spinup == 0) printf("offday - %i\n", offday);
			}
		
		
			/* if vegetation period has not ended until the last day of year, the offday is equal to the last day of year */
			if (yday == NDAY_OF_YEAR-1 && offday == 0)
			{	/* if vegetation period has not began */
				if (onday_flag == 0) 
				{	
					onday_arr[ny] = yday-2;
					if (ctrl->onscreen) printf("WARNING: no real vegetation period - check meteorological data");
				}
				onday_flag     = 0;
				offday_flag    = 1;
				offday         = yday;
				offday_arr[ny] = offday;
			
			}
			/* ******************************************************************* */
			/* 4. writing out the enviromental parameters and GSI indexes */
			if (ctrl->onscreen)
			{
				fprintf(GSI->GSI_file.ptr, "%i %i %5.2f %5.1f %5.3f %5.3f %5.3f %5.3f %5.3f %5.3f\n", 
						ctrl->simstartyear+ny, yday, 
						snowcover, heatsum_act, tmin_index, vpd_index, dayl_index, heatsum_index, GSI_index_avg, GSI_index_total);
			}

		}/* endfor - simdays */

	
	} /* endfor - simyears */

	/* writing out the date of onday and offday for every simulation year */
	if (ctrl->onscreen)
	{
		for (ny=0 ; ny<nyears ; ny++)
		{
			fprintf(GSI->GSI_file.ptr, "%i\n", ctrl->simstartyear+ny);
			fprintf(GSI->GSI_file.ptr, "%i\n", onday_arr[ny]);
			fprintf(GSI->GSI_file.ptr, "%i\n", offday_arr[ny]);
		}
	}

	phenarr->onday_arr = onday_arr;
	phenarr->offday_arr= offday_arr;

	return (!ok);

} /* end - subroutine */



	