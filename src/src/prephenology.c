/*
prephenology.c
Initialize phenology arrays, called prior to annual loop in bgc()

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2018, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
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

int prephenology(file logfile, const planting_struct* PLT, const harvesting_struct* HRV, const epconst_struct* epc, 
	const siteconst_struct* sitec, const metarr_struct* metarr, const control_struct* ctrl, phenarray_struct* phenarr)

{
	int ok=1;
	
	int model,woody,evergreen,south;
	double t1;
	int i,pday,ndays,py;
	int nyears,phenyears;
	int onday,offday;
	/* phenology model variables */
	int *onday_arr = 0;
	int *offday_arr = 0;
	int fall_tavg_count;
	int onset_day, offset_day;
	double mean_tavg,fall_tavg;
	double phensoilt,phendayl;
	double onset_critsum, sum_soilt;
	double critdayl = 39300.0; /* seconds */
	/* grass model parameters */
	double ann_prcp;
	double sum_prcp, phenprcp;
	double grass_stsumcrit;
	double grass_prcpcrit;
	double grass_stsummax = 1380.0;
	double grass_stsummid = 900.0;
	double grass_stsummin = 418.0;
	double grass_a = 32.9;
	double grass_k = 0.15;
	double grass_tmid = 9.0;
	double grass_prcpyear[365];
	double grass_prcpprevcrit = 1.14;
	double grass_prcpprev;
	double grass_prcpnextcrit = 0.97;
	double grass_prcpnext;
	double grass_tmaxyear[NDAYS_OF_YEAR];
	double grass_tminyear[NDAYS_OF_YEAR];
	double grass_3daytmin[NDAYS_OF_YEAR];
	int psum_startday, psum_stopday;
	double tmax_ann, tmax, new_tmax;
	double tmin_annavg;

	int onday_min, onday_max, offday_min, offday_max;


	onday = offday = 0;
	nyears = ctrl->metyears;
	ndays = NDAYS_OF_YEAR * nyears;

	onday_min = offday_min = 364;
	onday_max = offday_max = 0;
	

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
	
	if (!ctrl->GSI_flag)
	{
		if (ok)
		{
			phenarr->onday_arr = (int*) malloc((nyears+1) * sizeof(int));
			if (!phenarr->onday_arr)
			{
				printf("Error allocating for onday_arr, prephenology()\n");
				ok=0;
			}
		}
		if (ok)
		{
			phenarr->offday_arr = (int*) malloc((nyears+1) * sizeof(int));
			if (!phenarr->offday_arr)
			{
				printf("Error allocating for offday_arr, prephenology()\n");
				ok=0;
			}
		}
	}
	

	/* set some local flags to control the phenology model behavior */
	/* model=1 --> use phenology model   model=0 --> user specified phenology */
	/* woody=1 --> woody veg type        woody=0 --> non-woody veg type */
	/* evergreen=1 --> evergreen type    evergreen=0 --> deciduous type */
	/* south=1 --> southern hemisphere   south=0 --> northern hemisphere */
	model = epc->phenology_flag;
	woody = epc->woody;
	evergreen = epc->evergreen;
	south = (sitec->lat < 0.0);
	
	
	/* for southern hemisphere sites, use an extra phenology year */
	if (south) phenyears = nyears+1;
	else phenyears = nyears;
	

	fprintf(logfile.ptr, "Information about SGS and EGS values (yday of onday and offday)\n");

	/* control: in case of planting/harvesting, model-defined phenology is not possible: first day - planting day, last day - harvesting day */
	if (epc->phenology_flag == 1 && (PLT->PLT_flag || HRV->HRV_flag))
	{
		printf("\n");
		printf("ERROR in model parametrization: in case of planting/harvesting, model-defined phenology is not possible (firstday:planting, lastday:harvesting\n");
		printf("Please check EPC file and try again\n");
		ok=0;
	}

	/************************************************************************************************************
	/* I. CONSTANT onday and offday
	      define the phenology signals for cases in which the phenology signals are constant between years */
	/************************************************************************************************************/

	
	if (evergreen || !model)
	{
		
		/*  1. define USER-DEFINED onday and offdays (from EPC file or SGS/EGS file) */
		for (py=0 ; py<phenyears ; py++)
		{
	
			if (ctrl->varSGS_flag && ctrl->spinup != 1)
				onday = (int) epc->sgs_array[py];
			else
				onday = epc->onday;
			
			if (ctrl->varEGS_flag && ctrl->spinup != 1)
				offday = (int) epc->egs_array[py];
			else
				offday = epc->offday;

			if (evergreen)
			{
				onday=0;
				offday=364;
			}

		
			phenarr->onday_arr[py]  = onday;
			phenarr->offday_arr[py] = offday;

			/* for wrinting out log file */
			if (onday  < onday_min)  onday_min  = onday;
			if (onday  > onday_max)  onday_max  = onday;
			if (offday < offday_min) offday_min = offday;
			if (offday > offday_max) offday_max = offday;
		} /* end of user defined on and off days  */

	
		/* 2. if management information - use planting and harvest date for onday and offday	*/
		if (PLT->PLT_flag)
		{
			onday_min = 364;
			onday_max = 0;
			for (py=0 ; py<phenyears ; py++)
			{
				/* yearly variable (from MGM file) or constant (from INI file) management data */
				if (ctrl->PLT_flag == 2)
					onday_arr[py] = (int) PLT->PLTdays_array[0][py];
				else
					onday_arr[py] = (int) PLT->PLTdays_array[0][0];

				phenarr->onday_arr[py] = onday_arr[py];

				if (onday_arr[py]  < onday_min)  onday_min  = onday_arr[py];
				if (onday_arr[py]  > onday_max)  onday_max  = onday_arr[py];
				
			}
		}

		if (HRV->HRV_flag)
		{
			offday_min = 364;
			offday_max = 0;
			for (py=0 ; py<phenyears ; py++)
			{
				/* yearly variable (from MGM file) or constant (from INI file) management data */
				if (ctrl->PLT_flag == 2)
					offday_arr[py] = (int) HRV->HRVdays_array[0][py];
				else
					offday_arr[py] = (int) HRV->HRVdays_array[0][0];

				phenarr->offday_arr[py] = offday_arr[py];

				if (offday_arr[py]  < offday_min)  offday_min  = offday_arr[py];
				if (offday_arr[py]  > offday_max)  offday_max  = offday_arr[py];
				
			}
		}

		/* 3. wrinting log file */
		fprintf(logfile.ptr, "SGS value (min and max): %6i %6i\n", onday_min, onday_max);
		fprintf(logfile.ptr, "EGS value (min and max): %6i %6i\n", offday_min, offday_max);
		fprintf(logfile.ptr, " \n");

	} /* end if constant phenological signals */

	/************************************************************************************************************
	/* II. MODEL-CALCULATED onday and offday */
	/************************************************************************************************************/
	else
	{
		/*  1. define MODEL-DEFINED onday and offdays (original or GSI-method */
		/**************************************************************/
		/* A: ORIGINAL METHOD for calculating onset and offset day */
		/**************************************************************/
		if (!ctrl->GSI_flag)
		{
			/* Cases that have variable phenological signals between years */
			/* Use the phenology model described in White et al., 1997 */
			/* the two cases that make it to this block are:
			model, deciduous, woody   and
			model, deciduous, non-woody (grass), which are the two cases
			handled by the White et al. paper */
			if (woody)
			{
				/* use DECIDUOUS TREE PHENOLOGY MODEL */
				/* loop through the entire tavg timeseries to calculate long-term
				average tavg */
				mean_tavg = 0.0;
				for (i=0 ; i<ndays ; i++)
				{
					mean_tavg += (metarr->tmax[i] + metarr->tmin[i])/2.;
				}
				mean_tavg /= (double)ndays;
				/* tree onset equation from Mike White, Aug. 1997 */
				onset_critsum = exp(4.795 + 0.129*mean_tavg);
				
				/* now go through the phenological years and generate expansion
				and litterfall arrays. Some complications for Southern
				hemisphere sites... */
				/* calculate fall_tavg, the mean tavg from phenyday 244-304 */
				fall_tavg = 0.0;
				fall_tavg_count = 0;
				for (py=0 ; py<phenyears ; py++)
				{
					for (pday=244 ; pday<305 ; pday++)
					{
						if (south)
						{
							if (py==phenyears-1 && pday>181)
							{
								/* use the beginning of the last year to fill the
								end of the last phenological year */
								phensoilt = metarr->tavg11_ra[ndays-547+pday];
							}
							else
							{
								phensoilt = metarr->tavg11_ra[py*NDAYS_OF_YEAR-182+pday];
							}
						}
						else /* north */
						{
							phensoilt = metarr->tavg11_ra[py*NDAYS_OF_YEAR+pday];
						}
						
						fall_tavg += phensoilt;
						fall_tavg_count++;
						
					} /* end pday loop */
				} /* end py loop */
				fall_tavg /= (double)fall_tavg_count;
				
				/* loop through phenyears again, fill onset and offset arrays */
				for (py=0 ; py<phenyears ; py++)
				{
					sum_soilt = 0.0;
					onset_day = offset_day = -1;
					for (pday=0 ; pday<NDAYS_OF_YEAR ; pday++)
					{
						if (south)
						{
							if (py==0 && pday<182)
							{
								/* use the end of the first year to fill the 
								beginning of a southern hemisphere phenological
								year */
								phensoilt = metarr->tavg11_ra[183+pday];
								phendayl = metarr->dayl[183+pday];
							}
							else if (py==phenyears-1 && pday>181)
							{
								/* use the beginning of the last year to fill the
								end of the last phenological year */
								phensoilt = metarr->tavg11_ra[ndays-547+pday];
								phendayl = metarr->dayl[ndays-547+pday];
							}
							else
							{
								phensoilt = metarr->tavg11_ra[py*NDAYS_OF_YEAR-182+pday];
								phendayl = metarr->dayl[py*NDAYS_OF_YEAR-182+pday];
							}
						}
						else /* north */
						{
							phensoilt = metarr->tavg11_ra[py*NDAYS_OF_YEAR+pday];
							phendayl = metarr->dayl[py*NDAYS_OF_YEAR+pday];
						}
						
						/* tree onset test */
						if (onset_day == -1)
						{
							if (phensoilt > 0.0) sum_soilt += phensoilt;
							if (sum_soilt >= onset_critsum) onset_day = pday;
						}
						
						/* tree offset test */
						if (onset_day != -1 && offset_day == -1)
						{
							if ((pday>182) && 
							(((phendayl<=critdayl) && (phensoilt<=fall_tavg)) ||
							(phensoilt<=2.0))) offset_day = pday;
						}
						
					} /* end pday loop */
					
					/* now do some exception handling for this year's phenology */
					if (onset_day != -1)
					{
						/* leaves are turned on sometime this year */
						/* subtract 15 days from onset day to approximate the
						start of the new growth period, instead of the middle of
						the new growth period, as is used in the White et al. ms. */
						if (onset_day >= 15)
						{
							onset_day -= 15;
							/*onset_day-=1;*/
						}
						else onset_day = 0;

						/* if leaves never got turned off, force off on last day */
						if (offset_day == -1) offset_day = 364;
						/* add 15 days to offset day to approximate the
						end of the litterfall period, instead of the middle
						as in the White et al. ms. */
						if (offset_day <= 349)
						{
							offset_day += 15;
							/*offset_day+=1;*/
						}
						else offset_day = 364;
						
						/* force onset and offset to be at least one day apart */
						if (onset_day == offset_day)
						{
							if (onset_day > 0) onset_day--;
							else offset_day++;
						}
					}
					else
					{
						/* leaves never got turned on, this is a non-growth
						year.  This probably indicates a problem with the
						phenology model */
						onset_day = -1;
						offset_day = -1;
					}

					/* save these onset and offset days and go to the next
					phenological year */
					onday_arr[py] = onset_day;
					offday_arr[py] = offset_day;

			
					
				} /* end phenyears loop for filling onset and offset arrays */
			} /* end if woody (tree phenology model) */
			else
			{
			

				/* non-woody, use the GRASS PHENOLOGY MODEL to calculate the
				array of onset and offset days */
				/* loop through the entire tavg timeseries to calculate long-term
				average tavg and long-term average annual total precip */
				mean_tavg = 0.0;
				ann_prcp = 0.0;
				for (i=0 ; i<ndays ; i++)
				{
					mean_tavg += metarr->tday[i];
					ann_prcp += metarr->prcp[i];
				}
				mean_tavg /= (double)ndays;
				ann_prcp /= (double)ndays / NDAYS_OF_YEAR;
				
				/* grass onset equation from White et al., 1997, with parameter
				values specified by Mike White, Aug. 1997 */
				t1 = exp(grass_a * (mean_tavg - grass_tmid));
				grass_stsumcrit = ((grass_stsummax - grass_stsummin)* 0.5 *
					((t1-1)/(t1+1))) + grass_stsummid;
				grass_prcpcrit = ann_prcp * grass_k;
				
				/* now go through the phenological years and generate onset
				and offset days */
				
				/* calculate the long-term average annual high temperature
				for use in grass offset prediction */
				tmax_ann = 0.0;
				tmin_annavg = 0.0;
				for (py=0 ; py<phenyears ; py++)
				{
					new_tmax = -1000.0;
					for (pday=0 ; pday<NDAYS_OF_YEAR ; pday++)
					{
						if (south)
						{
							if (py==0 && pday<182)
							{
								/* use the end of the first year to fill the 
								beginning of a southern hemisphere phenological
								year */
								tmax = metarr->tmax[183+pday];
								tmin_annavg += metarr->tmin[183+pday];
							}
							else if (py==phenyears-1 && pday>181)
							{
								/* use the beginning of the last year to fill the
								end of the last phenological year */
								tmax = metarr->tmax[ndays-547+pday];
								tmin_annavg += metarr->tmin[ndays-547+pday];
							}
							else
							{
								tmax = metarr->tmax[py*NDAYS_OF_YEAR-182+pday];
								tmin_annavg += metarr->tmin[py*NDAYS_OF_YEAR-182+pday];
							}
						}
						else /* north */
						{
							tmax = metarr->tmax[py*NDAYS_OF_YEAR+pday];
							tmin_annavg += metarr->tmin[py*NDAYS_OF_YEAR+pday];
						}
						
						if (tmax > new_tmax) new_tmax = tmax;
						
					} /* end pday loop */
					
					tmax_ann += new_tmax;
				} /* end py loop */
				tmax_ann /= (double) phenyears;
				/* 92% of tmax_ann is the threshold used in grass offset below */
				tmax_ann *= 0.92;
				tmin_annavg /= (double) phenyears * NDAYS_OF_YEAR;
				
				/* loop through phenyears again, fill onset and offset arrays */
				for (py=0 ; py<phenyears ; py++)
				{
					sum_soilt = 0.0;
					sum_prcp = 0.0;
					onset_day = offset_day = -1;
					for (pday=0 ; pday<NDAYS_OF_YEAR ; pday++)
					{
						if (south)
						{
							if (py==0 && pday<182)
							{
								/* use the end of the first year to fill the 
								beginning of a southern hemisphere phenological
								year */
								phensoilt = metarr->tavg11_ra[183+pday];
								phenprcp = metarr->prcp[183+pday];
								grass_prcpyear[pday] = phenprcp;
								grass_tminyear[pday] = metarr->tmin[183+pday];
								grass_tmaxyear[pday] = metarr->tmax[183+pday];
							}
							else if (py==phenyears-1 && pday>181)
							{
								/* use the beginning of the last year to fill the
								end of the last phenological year */
								phensoilt = metarr->tavg11_ra[ndays-547+pday];
								phenprcp = metarr->prcp[ndays-547+pday];
								grass_prcpyear[pday] = phenprcp;
								grass_tminyear[pday] = metarr->tmin[ndays-547+pday];
								grass_tmaxyear[pday] = metarr->tmax[ndays-547+pday];
							}
							else
							{
								phensoilt = metarr->tavg11_ra[py*NDAYS_OF_YEAR-182+pday];
								phenprcp = metarr->prcp[py*NDAYS_OF_YEAR-182+pday];
								grass_prcpyear[pday] = phenprcp;
								grass_tminyear[pday] = metarr->tmin[py*NDAYS_OF_YEAR-182+pday];
								grass_tmaxyear[pday] = metarr->tmax[py*NDAYS_OF_YEAR-182+pday];
							}
						}
						else /* north */
						{
							phensoilt = metarr->tavg11_ra[py*NDAYS_OF_YEAR+pday];
							phenprcp = metarr->prcp[py*NDAYS_OF_YEAR+pday];
							grass_prcpyear[pday] = phenprcp;
							grass_tminyear[pday] = metarr->tmin[py*NDAYS_OF_YEAR+pday];
							grass_tmaxyear[pday] = metarr->tmax[py*NDAYS_OF_YEAR+pday];
						}
						
						/* grass onset test */
						if (onset_day == -1)
						{
							if (phensoilt > 0.0) sum_soilt += phensoilt;
							sum_prcp += phenprcp;
							if (sum_soilt >= grass_stsumcrit &&
								sum_prcp >= grass_prcpcrit) onset_day = pday;
						}

						
					} /* end pday loop */
					
					/* do averaging operations on grass_prcpyear and grass_tminyear,
					and do tests for offset day. Offset due to hot & dry can't
					happen within one month after the onset day, and offset due
					to cold can't happen before midyear (yearday 182) */
					if (onset_day != -1)
					{
						/* calculate three-day boxcar average of tmin */
						if (boxcar_smooth(grass_tminyear, grass_3daytmin, NDAYS_OF_YEAR,3,0))
						{
							printf("Error in prephenology() call to boxcar()\n");
							ok=0;
						}
						
						for (pday=onset_day+30 ; pday<NDAYS_OF_YEAR ; pday++)
						{
							/* calculate the previous 31-day prcp total */
							psum_startday = pday - 30;
							grass_prcpprev = 0.0;
							for (i=psum_startday ; i<=pday ; i++)
							{
								grass_prcpprev += grass_prcpyear[i];
							}

							/* calculate the next 7-day prcp total */
							if (pday > 358) psum_stopday = 364;
							else psum_stopday = pday + 6;
							grass_prcpnext = 0.0;
							for (i=pday ; i<=psum_stopday ; i++)
							{
								grass_prcpnext += grass_prcpyear[i];
							}
							
							/* test for hot and dry conditions */
							if (offset_day == -1)
							{
								if (grass_prcpprev < grass_prcpprevcrit && 
									grass_prcpnext < grass_prcpnextcrit &&
									grass_tmaxyear[pday] > tmax_ann)
									offset_day = pday;
							}
							
							/* test for cold offset condition */
							if (offset_day == -1)
							{
								if (pday > 182 &&
									grass_3daytmin[pday] <= tmin_annavg)
									offset_day = pday;
							}
							
						} /* end of pdays loop for grass offset testing */
					} /* end of if onset_day != -1 block */
						
					/* now do some exception handling for this year's phenology */
					if (onset_day != -1)
					{
						/* leaves are turned on sometime this year */
						/* subtract 15 days from onset day to approximate the
						start of the new growth period, instead of the middle of
						the new growth period, as is used in the White et al. ms. */
						if (onset_day >= 15)
						{
							onset_day -= 15;
						}
						else onset_day = 0;

						/* if leaves never got turned off, force off on last day */
						if (offset_day == -1) offset_day = 364;

						/* force onset and offset to be at least one day apart */
						if (onset_day == offset_day)
						{
							if (onset_day > 0) onset_day--;
							else offset_day++;
						}
					}
					else
					{
						/* leaves never got turned on, this is a non-growth
						year.  This probably indicates a problem with the
						phenology model */
						onset_day = -1;
						offset_day = -1;
					}
					
					/* save these onset and offset days and go to the next
					phenological year */
					onday_arr[py] = onset_day;
					offday_arr[py] = offset_day;
					
				} 	/* end phenyears loop for filling onset and offset arrays */
			} /* end else !woody (grass phenology model) */

			/* fix the onday and offday value */
			for (py=0 ; py<phenyears ; py++)
			{
				phenarr->onday_arr[py]  = onday_arr[py];
				phenarr->offday_arr[py] = offday_arr[py];

				/* wrinting out log file */
				if (onday_arr[py]  < onday_min)  onday_min  = onday_arr[py];
				if (onday_arr[py]  > onday_max)  onday_max  = onday_arr[py];
				if (offday_arr[py] < offday_min) offday_min = offday_arr[py];
				if (offday_arr[py] > offday_max) offday_max = offday_arr[py];

			}

		}
		/**************************************************************/
		/* B. GSI-METHOD for calculating onset and offset day */
		/**************************************************************/
		else
		{
			for (py=0 ; py<phenyears ; py++)
			{
				onday_arr[py] = phenarr->onday_arr[py];
				offday_arr[py] = phenarr->offday_arr[py];

				/* wrinting out log file */
				if (onday_arr[py]  < onday_min)  onday_min  = onday_arr[py];
				if (onday_arr[py]  > onday_max)  onday_max  = onday_arr[py];
				if (offday_arr[py] < offday_min) offday_min = offday_arr[py];
				if (offday_arr[py] > offday_max) offday_max = offday_arr[py];

				
			}
		}

		/* 2. if management information - use planting and harvest date for onday and offday	*/
		if (PLT->PLT_flag)
		{
			onday_min = 364;
			onday_max = 0;
			for (py=0 ; py<phenyears ; py++)
			{
				/* yearly variable (from MGM file) or constant (from INI file) management data */
				if (ctrl->PLT_flag == 2)
					onday_arr[py] = (int) PLT->PLTdays_array[0][py];
				else
					onday_arr[py] = (int) PLT->PLTdays_array[0][0];
				phenarr->onday_arr[py] = onday_arr[py];

				if (onday_arr[py]  < onday_min)  onday_min  = onday_arr[py];
				if (onday_arr[py]  > onday_max)  onday_max  = onday_arr[py];
				
			}
		}

		if (HRV->HRV_flag)
		{
			offday_min = 364;
			offday_max = 0;
			for (py=0 ; py<phenyears ; py++)
			{
				/* yearly variable (from MGM file) or constant (from INI file) management data */
				if (ctrl->HRV_flag == 2)
					offday_arr[py] = (int) HRV->HRVdays_array[0][py];
				else
					offday_arr[py] = (int) HRV->HRVdays_array[0][0];
				phenarr->offday_arr[py] = offday_arr[py];

				if (offday_arr[py]  < offday_min)  offday_min  = offday_arr[py];
				if (offday_arr[py]  > offday_max)  offday_max  = offday_arr[py];
				
			}
		}

	
		/* 3. wrinting log file */
		fprintf(logfile.ptr, "SGS value (min and max): %6i %6i\n", onday_min, onday_max);
		fprintf(logfile.ptr, "EGS value (min and max): %6i %6i\n", offday_min, offday_max);
		fprintf(logfile.ptr, " \n");
		
		/* 4. now the onset and offset days are established for each phenyear,either by the deciduous tree or the grass model.  
		Next loop through phenyears filling the phenology signal arrays and copying them to the permanent phen struct arrays */
		for (py=0 ; py<phenyears ; py++)
		{
			
			onday = onday_arr[py];
			offday = offday_arr[py];
		
		} /* end phenyears loop for filling permanent arrays */
	} /* end else phenology model block */
	
	/* free the local array memory */
	free(onday_arr); 
	free(offday_arr);

	return (!ok);
}

