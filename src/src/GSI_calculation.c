/* 
GSI_calculation.c
Calculation of pheonological state of vegetation (onset and offset day) from Tmin, VPD, daylength and cumulative Tavg 
based on literure (Jolly et al, 2005) and own method. The goal is to replace prephenology.c subroutines becuse the calulation
of the model-defined onset and offset day does not work correctly

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Copyright 2020, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "ini.h"
#include "bgc_struct.h"
#include "pointbgc_struct.h"
#include "bgc_func.h"     
#include "bgc_constants.h"
#include "misc_func.h"


int GSI_calculation(const metarr_struct* metarr, const siteconst_struct* sitec, epconst_struct* epc, 
					phenarray_struct* phenarr, control_struct* ctrl)

{
	int errorCode=0;
	int ny, yday, back;

	int firstdayLP = 240;		/* theoretically first day of litterfall */

	/* flags and counters */
	int onday_flag = 0;
	int offday_flag = 0;
	int onday = 0;
	int offday = 0;
	int nyears = ctrl->simyears;
	int n_yday = nDAYS_OF_YEAR;
	
	/*  enviromental conditions taken account to calculate onset and offset days */
	double tmax_act, tmin_act, tavg_act, vpd_act, dayl_act, heatsum_act;	

	
	/* threshold limits for each variable, between assuming that phenological activity varied linearly from inactive to unconstrained */
	double base_temp = epc->base_temp;
	double heatsum_limit1 = epc->heatsum_limit1;
	double heatsum_limit2 = epc->heatsum_limit2;
	double tmin_limit1 = epc->tmin_limit1;
	double tmin_limit2 = epc->tmin_limit2;
	double vpd_limit1 = epc->vpd_limit1;
	double vpd_limit2 = epc->vpd_limit2;
	double dayl_limit1 = epc->dayl_limit1;
	double dayl_limit2 = epc->dayl_limit2;
	int    n_moving_avg = epc->n_moving_avg-1;	/* moving averages are calculated from indicatiors 
											       to avoid the effects of single extreme events; -1: number to index */
	double epc_limit_SGS = epc->GSI_limit_SGS;	/* when GSI index fisrt time greater that limit -> start of the growing season */
	double epc_limit_EGS = epc->GSI_limit_EGS;	/* when GSI index fisrt time less that limit -> end of the growing season */


	/* indexes for the different variables and total index (multiplication of  partial indexes)*/
	double tmin_index = 0;
	double vpd_index = 0; 
	double dayl_index = 0; 
	double gsi_index = 0; 
	double heatsum_index = 0; 

	/* at the presence of snow cover no vegetation period (calculating snow cover from precipitation, Tavg and srad) */
	double snowcover, snowcover_limit, prcp_act, srad_act;
	double rn, tmelt, rmelt, melt, snow_loss, snow_plus, rsub;
	double albedo_sw = sitec->albedo_sw;
	
	/* to calculate moving average from total index values */
	double gsi_indexSUM = 0;
	double gsi_indexAVG = 0;
	double hsgsi_index = 0;
	
	onday_flag = 0;
	offday_flag = 1;

	/* limitation of snow cover */
	snowcover = 0;

	snowcover_limit = epc->snowcover_limit;
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////allocate memory for arrays containing ondays and offdays /////////////////////////////////////
	if (!errorCode) 
	{
		/* allocate space for the onday_arr and offday_arr: first column - year, second column: day*/
		phenarr->onday_arr  = (int**) malloc(nyears*sizeof(int*));  
        phenarr->offday_arr = (int**) malloc(nyears*sizeof(int*));  

		phenarr->tmin_index     = (double**) malloc(nyears*sizeof(int*));  
		phenarr->vpd_index      = (double**) malloc(nyears*sizeof(int*));  
		phenarr->dayl_index     = (double**) malloc(nyears*sizeof(int*));  
		phenarr->gsi_indexAVG   = (double**) malloc(nyears*sizeof(int*));  
		phenarr->heatsum_index  = (double**) malloc(nyears*sizeof(int*));  
		phenarr->heatsum        = (double**) malloc(nyears*sizeof(int*));  

		for (ny = 0; ny<nyears; ny++)
		{
			phenarr->onday_arr[ny]  = (int*) malloc(2*sizeof(int));  
			phenarr->offday_arr[ny] = (int*) malloc(2*sizeof(int));  
			phenarr->onday_arr[ny]  = (int*) malloc(2*sizeof(int));  
			phenarr->offday_arr[ny] = (int*) malloc(2*sizeof(int)); 

			phenarr->tmin_index[ny]     = (double*) malloc(nDAYS_OF_YEAR*sizeof(double));  
			phenarr->vpd_index[ny]      = (double*) malloc(nDAYS_OF_YEAR*sizeof(double));  
			phenarr->dayl_index[ny]     = (double*) malloc(nDAYS_OF_YEAR*sizeof(double));  
			phenarr->gsi_indexAVG[ny]   = (double*) malloc(nDAYS_OF_YEAR*sizeof(double));  
			phenarr->heatsum_index[ny]  = (double*) malloc(nDAYS_OF_YEAR*sizeof(double));  
			phenarr->heatsum[ny]        = (double*) malloc(nDAYS_OF_YEAR*sizeof(double));  
		}
	
		if (!phenarr->onday_arr)
		{
			printf("\n");
			printf("ERROR allocating for onday_arr, GSI_calculation()\n");
			errorCode=1;
		}

		if (!phenarr->offday_arr)
		{
			printf("\n");
			printf("ERROR allocating for offday_arr, GSI_calculation()\n");
			errorCode=1;
		}
	}
	
	for (ny=0; ny<nyears; ny++)
	{
		onday  = 0;
		offday = 0;



		for (yday=0; yday<nDAYS_OF_YEAR; yday++)	
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
				vpd_index       = 0;
				dayl_index      = 0;
				gsi_indexAVG   = 0;
				heatsum_index   = 0;
				hsgsi_index = 0;
			}
			else
			{
				gsi_indexSUM = 0;
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
						
					
					gsi_index = tmin_index * vpd_index * dayl_index;
					gsi_indexSUM += gsi_index;

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

				gsi_indexAVG = gsi_indexSUM / (n_moving_avg+1);
				hsgsi_index   = gsi_indexAVG * heatsum_index;


				
			} /* endelse - calculating indexes */

			if (onday_flag == 0 && offday_flag == 1 && hsgsi_index > epc_limit_SGS && yday < firstdayLP && snowcover <= snowcover_limit) 
			{
				onday_flag    = 1;
				offday_flag   = 0;
				onday         = yday;

				if (ctrl->onscreen && ctrl->spinup == 0) printf("Year: %i\n", ctrl->simstartyear+ny);
				if (ctrl->onscreen && ctrl->spinup == 0) printf("onday  - %i\n", onday);
			}

			if (onday_flag == 1 && offday_flag == 0 && hsgsi_index < epc_limit_EGS && yday > firstdayLP) 
			{
 				onday_flag     = 0;
				offday_flag    = 1;
				offday         = yday;

				if (ctrl->onscreen && ctrl->spinup == 0) printf("offday - %i\n", offday);
			}
		
		
			/* if vegetation period has not ended until the last day of year, the offday is equal to the last day of year */
			if (yday == nDAYS_OF_YEAR-1 && offday == 0)
			{	/* if vegetation period has not began */
				if (onday_flag == 0) 
				{	
					onday = yday-2;
					ctrl->vegper_flag = 1;
				}
				onday_flag     = 0;
				offday_flag    = 1;
				offday         = yday;

			
			}


			phenarr->tmin_index[ny][yday]     = tmin_index;  
			phenarr->vpd_index[ny][yday]      = vpd_index;  
			phenarr->dayl_index[ny][yday]     = dayl_index;  
			phenarr->gsi_indexAVG[ny][yday]   = gsi_indexAVG;  
			phenarr->heatsum_index[ny][yday]  = heatsum_index;  
			phenarr->heatsum[ny][yday]        = heatsum_act;  

		}/* endfor - simdays */
		
		phenarr->onday_arr[ny][0] = ny + ctrl->simstartyear;
		phenarr->onday_arr[ny][1] = onday;

		phenarr->offday_arr[ny][0] = ny + ctrl->simstartyear;
		phenarr->offday_arr[ny][1] = offday;

	} /* endfor - simyears */





	return (errorCode);

} /* end - subroutine */



	