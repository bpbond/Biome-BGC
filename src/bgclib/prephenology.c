/*
prephenology.c
Initialize phenology arrays, called prior to annual loop in bgc()

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "bgc.h"

int prephenology(const control_struct* ctrl, const epconst_struct* epc, 
const siteconst_struct* sitec, const metarr_struct* metarr,
phenarray_struct* phen)
{
	int ok=1;
	int model,woody,evergreen,south;
	double t1;
	char round[80];
	int i,pday,ndays,py;
	int nyears,phenyears;
	int ngrowthdays,ntransferdays,nlitfalldays;
	int onday,offday;
	int counter;
	int remdays_curgrowth[365];
	int remdays_transfer[365];
	int predays_transfer[365];
	int remdays_litfall[365];
	int predays_litfall[365];
	/* phenology model variables */
	int *onday_arr, *offday_arr;
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
	double grass_tmaxyear[365];
	double grass_tminyear[365];
	double grass_3daytmin[365];
	int psum_startday, psum_stopday;
	double tmax_ann, tmax, new_tmax;
	double tmin_annavg;

	/* allocate space for phenology arrays */
	nyears = ctrl->metyears;
	ndays = 365 * nyears;
	if (ok && !(phen->remdays_curgrowth = (int*) malloc(ndays*sizeof(int))))
	{
		bgc_printf(BV_ERROR, "Error allocating for phen->curgrowth, prephenology()\n");
		ok=0;
	}
	if (ok && !(phen->remdays_transfer = (int*) malloc(ndays*sizeof(int))))
	{
		bgc_printf(BV_ERROR, "Error allocating for phen->remdays_transfer, prephenology()\n");
		ok=0;
	}
	if (ok && !(phen->remdays_litfall = (int*) malloc(ndays*sizeof(int))))
	{
		bgc_printf(BV_ERROR, "Error allocating for phen->remdays_litfall, prephenology()\n");
		ok=0;
	}
	if (ok && !(phen->predays_transfer = (int*) malloc(ndays*sizeof(int))))
	{
		bgc_printf(BV_ERROR, "Error allocating for phen->predays_transfer, prephenology()\n");
		ok=0;
	}
	if (ok && !(phen->predays_litfall = (int*) malloc(ndays*sizeof(int))))
	{
		bgc_printf(BV_ERROR, "Error allocating for phen->predays_litfall, prephenology()\n");
		ok=0;
	}
	if (ok && !(onday_arr = (int*) malloc((nyears+1) * sizeof(int))))
	{
		bgc_printf(BV_ERROR, "Error allocating for onday_arr, prephenology()\n");
		ok=0;
	}
	if (ok && !(offday_arr = (int*) malloc((nyears+1) * sizeof(int))))
	{
		bgc_printf(BV_ERROR, "Error allocating for offday_arr, prephenology()\n");
		ok=0;
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
	
	/* define the phenology signals for cases in which the phenology signals
	are constant between years */
	if (evergreen || !model)
	{
		/* zero the 365-day phen arrays */
		for (pday=0 ; pday<365 ; pday++)
		{
			remdays_curgrowth[pday] = 0;
			remdays_transfer[pday] = 0;
			predays_transfer[pday] = 0;
			remdays_litfall[pday] = 0;
			predays_litfall[pday] = 0;
		}
		
		/* user defined on and off days (base zero) */
		onday = epc->onday;
		offday = epc->offday;
		if (!model && onday == -1 && offday == -1)
		{
			/* this is the special signal to repress all vegetation
			growth, for simulations of bare ground */
			for (pday=0 ; pday<365 ; pday++)
			{
				remdays_curgrowth[pday] = 0;
				remdays_transfer[pday] = 0;
				predays_transfer[pday] = 0;
				remdays_litfall[pday] = 0;
				predays_litfall[pday] = 0;
			}
		} /* end if special no-growth signal */
		else
		{
			/* normal growth */

			if (!model && !evergreen)
			{
				/* user-specified dates for onset and offset, but this
				gets overridden for evergreen types, so this case is only
				for USER-SPECIFIED DECIDUOUS (either woody or non-woody) */
				/* IMPORTANT NOTE:  the user specified yeardays for onset
				and offset are in relation to a phenological definition for
				a year, instead of the calendar year. In the Northern hemisphere,
				this is the same as the calendar year, but in the southern
				hemisphere, the phenological yeardays are defined to start on
				July 2 (N. Hem yearday 182, using base-zero).  This lets a
				user shift from N. Hem site to S. Hem site without having to
				change phenological yeardays in the ini file */
				/* force onset and offset to be at least one day apart */
				if (onday == offday)
				{
					if (onday > 0) onday--;
					else offday++;
				}
				ngrowthdays = offday - onday;
				/* define the length of the transfer and litfall periods */
				/* calculate number of transfer days and number of litfall days
				as proportions of number of growth days, as specified by user.
				Round and truncate to force these values between 1 and 
				ngrowthdays */
				t1 = epc->transfer_pdays * (double)ngrowthdays;
				sprintf(round,"%.0f",t1);
				ntransferdays = atoi(round);
				if (ntransferdays < 1) ntransferdays = 1;
				if (ntransferdays > ngrowthdays) ntransferdays = ngrowthdays;
				t1 = epc->litfall_pdays * (double)ngrowthdays;
				sprintf(round,"%.0f",t1);
				nlitfalldays = atoi(round);
				if (nlitfalldays < 1) nlitfalldays = 1;
				if (nlitfalldays > ngrowthdays) nlitfalldays = ngrowthdays;
				for (pday=0 ; pday<onday ; pday++)
				{
					remdays_curgrowth[pday] = 0;
					remdays_transfer[pday] = 0;
					remdays_litfall[pday] = 0;
					predays_transfer[pday] = 0;
					predays_litfall[pday] = 0;
				}
				counter = ngrowthdays;
				for (pday=onday ; pday<offday ; pday++)
				{
					remdays_curgrowth[pday] = counter;
					counter--;
				}
				for (pday=offday ; pday<365 ; pday++)
				{
					remdays_curgrowth[pday] = 0;
				}
				counter = ntransferdays;
				for (pday=onday ; pday<onday+ntransferdays ; pday++)
				{
					remdays_transfer[pday] = counter;
					predays_transfer[pday] = ntransferdays - counter;
					counter--;
				}
				for (pday=onday+ntransferdays ; pday<=offday ; pday++)
				{
					remdays_transfer[pday] = 0;
					predays_transfer[pday] = ntransferdays;
				}
				for (pday=offday+1 ; pday<365 ; pday++)
				{
					remdays_transfer[pday] = 0;
					predays_transfer[pday] = 0;
				}
				for (pday=onday ; pday<offday-nlitfalldays+1 ; pday++)
				{
					remdays_litfall[pday] = 0;
					predays_litfall[pday] = 0;
				}
				counter = nlitfalldays;
				for (pday=offday-nlitfalldays+1 ; pday<=offday ; pday++)
				{
					remdays_litfall[pday] = counter;
					predays_litfall[pday] = nlitfalldays - counter;
					counter--;
				}
				for (pday=offday+1 ; pday<365 ; pday++)
				{
					remdays_litfall[pday] = 0;
					predays_litfall[pday] = 0;
				}
			} /* end if user-specified and deciduous */

			if (evergreen)
			{	
				/* specifying evergreen overrides any user input phenology data,
				and triggers a very simple treatment of the transfer, litterfall,
				and current growth signals.  Treatment is the same for woody and
				non-woody types, and the same for model or user-input phenology */
				/* fill the local phenyear control arrays */
				for (pday=0 ; pday<365 ; pday++)
				{
					remdays_curgrowth[pday] = 365-pday;
					remdays_transfer[pday] = 365-pday;
					remdays_litfall[pday] = 365-pday;
					predays_transfer[pday] = pday;
					predays_litfall[pday] = pday;
				} 
			} /* end if evergreen */
		} /* end else normal growth */

		/* now copy this year multiple times to the permanent phenology struct
		arrays, with tests for southern hemisphere. */
		for (py=0 ; py<phenyears ; py++)
		{
			if (south)
			{
				if (py==0)
				{
					/* only copy the second half of this phenological
					year to the permanent phenology array */
					for (pday=182 ; pday<365 ; pday++)
					{
						phen->remdays_curgrowth[pday-182] = remdays_curgrowth[pday];
						phen->remdays_transfer[pday-182] = remdays_transfer[pday];
						phen->remdays_litfall[pday-182] = remdays_litfall[pday];
						phen->predays_transfer[pday-182] = predays_transfer[pday];
						phen->predays_litfall[pday-182] = predays_litfall[pday];
					}
				}
				else if (py==phenyears-1)
				{
					/* only copy the first half of this phenological
					year to the permanent phenology array */
					for (pday=0 ; pday<182 ; pday++)
					{
						phen->remdays_curgrowth[py*365-182+pday] = remdays_curgrowth[pday];
						phen->remdays_transfer[py*365-182+pday] = remdays_transfer[pday];
						phen->remdays_litfall[py*365-182+pday] = remdays_litfall[pday];
						phen->predays_transfer[py*365-182+pday] = predays_transfer[pday];
						phen->predays_litfall[py*365-182+pday] = predays_litfall[pday];
					}
				}
				else
				{
					for (pday=0 ; pday<365 ; pday++)
					{
						phen->remdays_curgrowth[py*365-182+pday] = remdays_curgrowth[pday];
						phen->remdays_transfer[py*365-182+pday] = remdays_transfer[pday];
						phen->remdays_litfall[py*365-182+pday] = remdays_litfall[pday];
						phen->predays_transfer[py*365-182+pday] = predays_transfer[pday];
						phen->predays_litfall[py*365-182+pday] = predays_litfall[pday];
					}
				}
			} /* end if south */
			else
			{
				/* north */
				for (pday=0 ; pday<365 ; pday++)
				{
					phen->remdays_curgrowth[py*365+pday] = remdays_curgrowth[pday];
					phen->remdays_transfer[py*365+pday] = remdays_transfer[pday];
					phen->remdays_litfall[py*365+pday] = remdays_litfall[pday];
					phen->predays_transfer[py*365+pday] = predays_transfer[pday];
					phen->predays_litfall[py*365+pday] = predays_litfall[pday];
				}
			} /* end if north */
		} /* end py loop */
	} /* end if constant phenological signals */
	else
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
				mean_tavg += metarr->tavg[i];
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
							phensoilt = metarr->tavg_ra[ndays-547+pday];
						}
						else
						{
							phensoilt = metarr->tavg_ra[py*365-182+pday];
						}
					}
					else /* north */
					{
						phensoilt = metarr->tavg_ra[py*365+pday];
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
				for (pday=0 ; pday<365 ; pday++)
				{
					if (south)
					{
						if (py==0 && pday<182)
						{
							/* use the end of the first year to fill the 
							beginning of a southern hemisphere phenological
							year */
							phensoilt = metarr->tavg_ra[183+pday];
							phendayl = metarr->dayl[183+pday];
						}
						else if (py==phenyears-1 && pday>181)
						{
							/* use the beginning of the last year to fill the
							end of the last phenological year */
							phensoilt = metarr->tavg_ra[ndays-547+pday];
							phendayl = metarr->dayl[ndays-547+pday];
						}
						else
						{
							phensoilt = metarr->tavg_ra[py*365-182+pday];
							phendayl = metarr->dayl[py*365-182+pday];
						}
					}
					else /* north */
					{
						phensoilt = metarr->tavg_ra[py*365+pday];
						phendayl = metarr->dayl[py*365+pday];
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
				mean_tavg += metarr->tavg[i];
				ann_prcp += metarr->prcp[i];
			}
			mean_tavg /= (double)ndays;
			ann_prcp /= (double)ndays / 365.0;
			
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
				for (pday=0 ; pday<365 ; pday++)
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
							tmax = metarr->tmax[py*365-182+pday];
							tmin_annavg += metarr->tmin[py*365-182+pday];
						}
					}
					else /* north */
					{
						tmax = metarr->tmax[py*365+pday];
						tmin_annavg += metarr->tmin[py*365+pday];
					}
					
					if (tmax > new_tmax) new_tmax = tmax;
					
				} /* end pday loop */
				
				tmax_ann += new_tmax;
			} /* end py loop */
			tmax_ann /= (double) phenyears;
			/* 92% of tmax_ann is the threshold used in grass offset below */
			tmax_ann *= 0.92;
			tmin_annavg /= (double) phenyears * 365.0;
			
			/* loop through phenyears again, fill onset and offset arrays */
			for (py=0 ; py<phenyears ; py++)
			{
				sum_soilt = 0.0;
				sum_prcp = 0.0;
				onset_day = offset_day = -1;
				for (pday=0 ; pday<365 ; pday++)
				{
					if (south)
					{
						if (py==0 && pday<182)
						{
							/* use the end of the first year to fill the 
							beginning of a southern hemisphere phenological
							year */
							phensoilt = metarr->tavg_ra[183+pday];
							phenprcp = metarr->prcp[183+pday];
							grass_prcpyear[pday] = phenprcp;
							grass_tminyear[pday] = metarr->tmin[183+pday];
							grass_tmaxyear[pday] = metarr->tmax[183+pday];
						}
						else if (py==phenyears-1 && pday>181)
						{
							/* use the beginning of the last year to fill the
							end of the last phenological year */
							phensoilt = metarr->tavg_ra[ndays-547+pday];
							phenprcp = metarr->prcp[ndays-547+pday];
							grass_prcpyear[pday] = phenprcp;
							grass_tminyear[pday] = metarr->tmin[ndays-547+pday];
							grass_tmaxyear[pday] = metarr->tmax[ndays-547+pday];
						}
						else
						{
							phensoilt = metarr->tavg_ra[py*365-182+pday];
							phenprcp = metarr->prcp[py*365-182+pday];
							grass_prcpyear[pday] = phenprcp;
							grass_tminyear[pday] = metarr->tmin[py*365-182+pday];
							grass_tmaxyear[pday] = metarr->tmax[py*365-182+pday];
						}
					}
					else /* north */
					{
						phensoilt = metarr->tavg_ra[py*365+pday];
						phenprcp = metarr->prcp[py*365+pday];
						grass_prcpyear[pday] = phenprcp;
						grass_tminyear[pday] = metarr->tmin[py*365+pday];
						grass_tmaxyear[pday] = metarr->tmax[py*365+pday];
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
					if (boxcar_smooth(grass_tminyear, grass_3daytmin, 365,3,0))
					{
						bgc_printf(BV_ERROR, "Error in prephenology() call to boxcar()\n");
						ok=0;
					}
					
					for (pday=onset_day+30 ; pday<365 ; pday++)
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
				
			} /* end phenyears loop for filling onset and offset arrays */
		} /* end else !woody (grass phenology model) */
		
		/* now the onset and offset days are established for each phenyear,
		either by the deciduous tree or the grass model.  Next loop through
		phenyears filling the phenology signal arrays and copying them to 
		the permanent phen struct arrays */
		for (py=0 ; py<phenyears ; py++)
		{
			/* zero the 365-day phen arrays */
			for (pday=0 ; pday<365 ; pday++)
			{
				remdays_curgrowth[pday] = 0;
				remdays_transfer[pday] = 0;
				predays_transfer[pday] = 0;
				remdays_litfall[pday] = 0;
				predays_litfall[pday] = 0;
			}
			
			onday = onday_arr[py];
			offday = offday_arr[py];
			
			if (onday == -1 && offday == -1)
			{
				/* this is the special signal to repress all vegetation
				growth */
				for (pday=0 ; pday<365 ; pday++)
				{
					remdays_curgrowth[pday] = 0;
					remdays_transfer[pday] = 0;
					predays_transfer[pday] = 0;
					remdays_litfall[pday] = 0;
					predays_litfall[pday] = 0;
				}
			} /* end if special no-growth signal */
			else
			{
				/* normal growth year */
				ngrowthdays = offday - onday;
				if (ngrowthdays < 1)
				{
					bgc_printf(BV_ERROR, "FATAL ERROR: ngrowthdays < 1\n");
					bgc_printf(BV_ERROR, "ngrowthdays = %d\n",ngrowthdays);
					bgc_printf(BV_ERROR, "onday = %d\toffday = %d\tphenyear = %d\n",
					onday,offday,py);
					ok=0;
				}
				/* define the length of the transfer and litfall periods */
				/* calculate number of transfer days and number of litfall days
				as proportions of number of growth days, as specified by user.
				Round and truncate to force these values between 1 and 
				ngrowthdays */
				t1 = epc->transfer_pdays * (double)ngrowthdays;
				sprintf(round,"%.0f",t1);
				ntransferdays = atoi(round);
				if (ntransferdays < 1) ntransferdays = 1;
				if (ntransferdays > ngrowthdays) ntransferdays = ngrowthdays;
				t1 = epc->litfall_pdays * (double)ngrowthdays;
				sprintf(round,"%.0f",t1);
				nlitfalldays = atoi(round);
				if (nlitfalldays < 1) nlitfalldays = 1;
				if (nlitfalldays > ngrowthdays) nlitfalldays = ngrowthdays;
				
				for (pday=0 ; pday<onday ; pday++)
				{
					remdays_curgrowth[pday] = 0;
					remdays_transfer[pday] = 0;
					remdays_litfall[pday] = 0;
					predays_transfer[pday] = 0;
					predays_litfall[pday] = 0;
				}
				counter = ngrowthdays;
				for (pday=onday ; pday<offday ; pday++)
				{
					remdays_curgrowth[pday] = counter;
					counter--;
				}
				for (pday=offday ; pday<365 ; pday++)
				{
					remdays_curgrowth[pday] = 0;
				}
				counter = ntransferdays;
				for (pday=onday ; pday<onday+ntransferdays ; pday++)
				{
					remdays_transfer[pday] = counter;
					predays_transfer[pday] = ntransferdays - counter;
					counter--;
				}
				for (pday=onday+ntransferdays ; pday<=offday ; pday++)
				{
					remdays_transfer[pday] = 0;
					predays_transfer[pday] = ntransferdays;
				}
				for (pday=offday+1 ; pday<365 ; pday++)
				{
					remdays_transfer[pday] = 0;
					predays_transfer[pday] = 0;
				}
				for (pday=onday ; pday<offday-nlitfalldays+1 ; pday++)
				{
					remdays_litfall[pday] = 0;
					predays_litfall[pday] = 0;
				}
				counter = nlitfalldays;
				for (pday=offday-nlitfalldays+1 ; pday<=offday ; pday++)
				{
					remdays_litfall[pday] = counter;
					predays_litfall[pday] = nlitfalldays - counter;
					counter--;
				}
				for (pday=offday+1 ; pday<365 ; pday++)
				{
					remdays_litfall[pday] = 0;
					predays_litfall[pday] = 0;
				}
			} /* end else normal growth year */
			
			/* now put the signals for this phenological year into the
			right place in the permanent phen struct arrays */ 
			if (south)
			{
				if (py==0)
				{
					/* only copy the second half of this phenological
					year to the permanent phenology array */
					for (pday=182 ; pday<365 ; pday++)
					{
						phen->remdays_curgrowth[pday-182] = remdays_curgrowth[pday];
						phen->remdays_transfer[pday-182] = remdays_transfer[pday];
						phen->remdays_litfall[pday-182] = remdays_litfall[pday];
						phen->predays_transfer[pday-182] = predays_transfer[pday];
						phen->predays_litfall[pday-182] = predays_litfall[pday];
					}
				}
				else if (py==phenyears-1)
				{
					/* only copy the first half of this phenological
					year to the permanent phenology array */
					for (pday=0 ; pday<182 ; pday++)
					{
						phen->remdays_curgrowth[py*365-182+pday] = remdays_curgrowth[pday];
						phen->remdays_transfer[py*365-182+pday] = remdays_transfer[pday];
						phen->remdays_litfall[py*365-182+pday] = remdays_litfall[pday];
						phen->predays_transfer[py*365-182+pday] = predays_transfer[pday];
						phen->predays_litfall[py*365-182+pday] = predays_litfall[pday];
					}
				}
				else
				{
					for (pday=0 ; pday<365 ; pday++)
					{
						phen->remdays_curgrowth[py*365-182+pday] = remdays_curgrowth[pday];
						phen->remdays_transfer[py*365-182+pday] = remdays_transfer[pday];
						phen->remdays_litfall[py*365-182+pday] = remdays_litfall[pday];
						phen->predays_transfer[py*365-182+pday] = predays_transfer[pday];
						phen->predays_litfall[py*365-182+pday] = predays_litfall[pday];
					}
				}
			} /* end if south */
			else
			{
				/* north */
				for (pday=0 ; pday<365 ; pday++)
				{
					phen->remdays_curgrowth[py*365+pday] = remdays_curgrowth[pday];
					phen->remdays_transfer[py*365+pday] = remdays_transfer[pday];
					phen->remdays_litfall[py*365+pday] = remdays_litfall[pday];
					phen->predays_transfer[py*365+pday] = predays_transfer[pday];
					phen->predays_litfall[py*365+pday] = predays_litfall[pday];
				}
			} /* end if north */
		} /* end phenyears loop for filling permanent arrays */
	} /* end else phenology model block */
				
	/* free the local array memory */
	free(onday_arr);
	free(offday_arr);
	
	return (!ok);
}

int free_phenmem(phenarray_struct* phen)
{
	int ok=1;
	
	/* free memory in phenology arrays */
	free(phen->remdays_curgrowth);
	free(phen->remdays_transfer);
	free(phen->remdays_litfall);
	free(phen->predays_transfer);
	free(phen->predays_litfall);
	
	return (!ok);
}
