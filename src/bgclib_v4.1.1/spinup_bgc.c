/*
spinup_bgc.c
Core BGC model logic, modified for spinup runs.  Instead of running for
a fixed number of years, as in a normal simulation, a spinup run proceeds
until steady-state is reached for the soil carbon, or until a specified
maximum number of years is reached.

Includes in-line output handling routines that write to daily and annual
output files. 

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.1.1
Copyright 1999, Peter E. Thornton
Numerical Terradynamics Simulation Group (NTSG)
School of Forestry, University of Montana
Missoula, MT 59812
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "bgc_struct.h"     
#include "bgc_constants.h"
#include "bgc_func.h"
#include "ini.h"     
#include "bgc_io.h"

/* #define DEBUG  set this to see function roll-call on-screen */
/* #define DEBUG_SPINUP set this to see the spinup details on-screen */

int spinup_bgc(bgcin_struct* bgcin, bgcout_struct* bgcout)
{
	
	/* variable declarations */
	int ok=1;

	/* iofiles and program control variables */
	control_struct     ctrl;

	/* meteorological variables */
	metarr_struct      metarr;
	metvar_struct      metv;
	co2control_struct  co2;
	
	/* state and flux variables for water, carbon, and nitrogen */
	wstate_struct      ws;
	wflux_struct       wf, zero_wf;
	cinit_struct       cinit;
	cstate_struct      cs;
	cflux_struct       cf, zero_cf;
	nstate_struct      ns;
	nflux_struct       nf, zero_nf;

	/* primary ecophysiological variables */
	epvar_struct       epv;

	/* site physical constants */
	siteconst_struct   sitec;

	/* phenological data */
	phenarray_struct   phenarr;
	phenology_struct   phen;
	
	/* ecophysiological constants */
	epconst_struct     epc;

	/* photosynthesis structures */
	psn_struct         psn_sun, psn_shade;
	
	/* temporary nitrogen variables for decomposition and allocation */
	ntemp_struct       nt;
	
	/* summary variable structure */
	summary_struct     summary;
	
	/* output mapping (array of pointers to double) */
	double **output_map;
	
	/* local storage for daily and annual output variables */
	float *dayarr, *monavgarr, *annavgarr, *annarr;

	/* miscelaneous variables for program control in main */
	int simyr, yday, metyr, metday;
	int first_balance;
	int annual_alloc;
	int outv;
	int i, nmetdays;
	double tair_avg, tdiff;
	int dayout;
	
	/* variables used for monthly average output */
	int curmonth;
	int mondays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	int endday[12] = {30,58,89,119,150,180,211,242,272,303,333,364};
	
	/* spinup control */
	int ntimesmet, nblock;
	int steady1, steady2, rising, metcycle, spinyears;
	double tally1, tally1b, tally2, tally2b, t1;
	double naddfrac;
	
	/* copy the input structures into local structures */
	ws = bgcin->ws;
	cinit = bgcin->cinit;
	cs = bgcin->cs;
	ns = bgcin->ns;
	sitec = bgcin->sitec;
	epc = bgcin->epc;
	/* note that the following three structures have dynamic memory elements,
	and so the notion of copying the input structure to a local structure
	value-by-value is not the same as above. In this case, the array pointers
	are being copied, so the local members use the same memory that was
	allocated in the calling function. Note also that bgc() does not modify
	the contents of these structures. */
	ctrl = bgcin->ctrl;
	metarr = bgcin->metarr;
	co2 = bgcin->co2;
	
#ifdef DEBUG
	printf("done copy input\n");
#endif

	/* local variable that signals the need for daily output array */
	dayout = (ctrl.dodaily || ctrl.domonavg || ctrl.doannavg);
	
	/* allocate memory for local output arrays */
	if (ok && dayout &&	!(dayarr = (float*) malloc(ctrl.ndayout * sizeof(float))))
	{
		printf("Error allocating for local daily output array in bgc()\n");
		ok=0;
	}
	if (ok && ctrl.domonavg && !(monavgarr = (float*) malloc(ctrl.ndayout * sizeof(float))))
	{
		printf("Error allocating for monthly average output array in bgc()\n");
		ok=0;
	}
	if (ok && ctrl.doannavg && !(annavgarr = (float*) malloc(ctrl.ndayout * sizeof(float))))
	{
		printf("Error allocating for annual average output array in bgc()\n");
		ok=0;
	}
	if (ok && ctrl.doannual && !(annarr = (float*) malloc(ctrl.nannout * sizeof(float))))
	{
		printf("Error allocating for local annual output array in bgc()\n");
		ok=0;
	}
	/* allocate space for the output map pointers */
	if (ok && !(output_map = (double**) malloc(NMAP * sizeof(double*))))
	{
		printf("Error allocating for output map in output_map_init()\n");
		ok=0;
	}
	
#ifdef DEBUG
	printf("done allocate out arrays\n");
#endif
	
	/* initialize monavg and annavg to 0.0 */
	if (ctrl.domonavg)
	{
		for (outv=0 ; outv<ctrl.ndayout ; outv++)
		{
			monavgarr[outv] = 0.0;
		}
	}
	if (ctrl.doannavg)
	{
		for (outv=0 ; outv<ctrl.ndayout ; outv++)
		{
			annavgarr[outv] = 0.0;
		}
	}
	
	/* initialize the output mapping array */
	if (ok && output_map_init(output_map,&metv,&ws,&wf,&cs,&cf,&ns,&nf,&phen,
		&epv,&psn_sun,&psn_shade,&summary))
	{
		printf("Error in call to output_map_init() from bgc()\n");
		ok=0;
	}
	
#ifdef DEBUG
	printf("done initialize outmap\n");
#endif
	
	/* make zero-flux structures for use inside annual and daily loops */
	if (ok && make_zero_flux_struct(&zero_wf, &zero_cf, &zero_nf))
	{
		printf("Error in call to make_zero_flux_struct() from bgc()\n");
		ok=0;
	}

#ifdef DEBUG
	printf("done make_zero_flux\n");
#endif
	
	/* atmospheric pressure (Pa) as a function of elevation (m) */
	if (ok && atm_pres(sitec.elev, &metv.pa))
	{
		printf("Error in atm_pres() from bgc()\n");
		ok=0;
	}
	
#ifdef DEBUG
	printf("done atm_pres\n");
#endif
	
	/* determine phenological signals */
	if (ok && prephenology(&ctrl, &epc, &sitec, &metarr, &phenarr))
	{
		printf("Error in call to prephenology(), from bgc()\n");
		ok=0;
	}
	
#ifdef DEBUG
	printf("done prephenology\n");
#endif
	
	/* calculate the annual average air temperature for use in soil 
	temperature corrections. This code added 9 February 1999, in
	conjunction with soil temperature testing done with Mike White. */
	tair_avg = 0.0;
	nmetdays = ctrl.metyears * 365;
	for (i=0 ; i<nmetdays ; i++)
	{
		tair_avg += metarr.tavg[i];
	}
	tair_avg /= (double)nmetdays;
	
	/* if this simulation is using a restart file for its initial
	conditions, then copy restart info into structures */
	if (ok && ctrl.read_restart)
	{
		if (ok && restart_input(&ctrl, &ws, &cs, &ns, &epv, &metyr,
			&(bgcin->restart_input)))
		{
			printf("Error in call to restart_input() from bgc()\n");
			ok=0;
		}
		
#ifdef DEBUG
		printf("done restart_input\n");
#endif
	
	}
	else
	/* no restart file, user supplies initial conditions */
	{
		/* initialize leaf C and N pools depending on phenology signals for
		the first metday */
		if (ok && firstday(&epc, &cinit, &epv, &phenarr, &cs, &ns))
		{
			printf("Error in call to firstday(), from bgc()\n");
			ok=0;
		}
		
		/* initial value for metyr */
		metyr = 0;
		
#ifdef DEBUG
		printf("done firstday\n");
#endif
	}

	/* zero water, carbon, and nitrogen source and sink variables */
	if (ok && zero_srcsnk(&cs,&ns,&ws,&summary))
	{
		printf("Error in call to zero_srcsnk(), from bgc()\n");
		ok=0;
	}

#ifdef DEBUG
	printf("done zero_srcsnk\n");
#endif
	
	/* initialize the indicator for first day of current simulation, so
	that the checks for mass balance can have two days for comparison */
	first_balance = 1;
	
	/* for simulations with fewer than 50 metyears, find the multiple of
	metyears that gets close to 100, use this as the block size in
	spinup control */
	if (ctrl.metyears < 50)
	{
		ntimesmet = 100 / ctrl.metyears;
		nblock = ctrl.metyears * ntimesmet;
	}
	else
	{
		nblock = ctrl.metyears;
	}
	
	/* initialize spinup control variables */
	spinyears = 0;
	metcycle = 0;
	steady1 = 0;
	steady2 = 0;
	rising = 1;
	
	/* do loop for spinup */
	do
	{	
		/* annual model loop, one cycle of metyears at a time */
		for (simyr=0 ; ok && simyr<nblock ; simyr++)
		{
			/* set current month to 0 (january) at the beginning of each year */
			curmonth = 0;
			
			/* calculate scaling for N additions (decreasing with
			time since the beginning of metcycle = 0 block */
			naddfrac = 1.0 - ((double)simyr/(double)nblock);
			
			if (metcycle == 0)
			{
				tally1 = 0.0;
				tally1b = 0.0;
				tally2 = 0.0;
				tally2b = 0.0;
			}
			
			/* test whether metyr needs to be reset */
			if (metyr == ctrl.metyears)
			{
#ifdef DEBUG
				printf("Resetting met data for cyclic input\n");
#endif
				metyr = 0;
			}

			/* output to screen to indicate start of simulation year */
			if (ctrl.onscreen) printf("Year: %6d\n",spinyears);
			
			/* set the max lai variable, for annual diagnostic output */
			epv.ytd_maxplai = 0.0;

			/* atmospheric concentration of CO2 (ppm) */
			if (!(co2.varco2)) metv.co2 = co2.co2ppm;
			else metv.co2 = co2.co2ppm_array[simyr];

			/* begin the daily model loop */
			for (yday=0 ; ok && yday<365 ; yday++)
			{
#ifdef DEBUG
				printf("year %d\tyday %d\n",simyr,yday);
#endif

				/* Test for very low state variable values and force them
				to 0.0 to avoid rounding and floating point overflow errors */
				if (ok && precision_control(&ws, &cs, &ns))
				{
					printf("Error in call to precision_control() from bgc()\n");
					ok=0;
				} 

				/* set the day index for meteorological and phenological arrays */
				metday = metyr*365 + yday;

				/* zero all the daily flux variables */
				wf = zero_wf;
				cf = zero_cf;
				nf = zero_nf;

				/* daily meteorological variables from metarrays */
				if (ok && daymet(&metarr, &metv, metday))
				{
					printf("Error in daymet() from bgc()\n");
					ok=0;
				}

#ifdef DEBUG
				printf("%d\t%d\tdone daymet\n",simyr,yday);
#endif

				/* soil temperature correction using difference from
				annual average tair */
				tdiff = tair_avg - metv.tsoil;
				if (ws.snoww)
				{
					metv.tsoil += 0.83 * tdiff;
				}
				else
				{
					metv.tsoil += 0.2 * tdiff;
				}
			
				/* daily phenological variables from phenarrays */
				if (ok && dayphen(&phenarr, &phen, metday))
				{
					printf("Error in dayphen() from bgc()\n");
					ok=0;
				}

#ifdef DEBUG
				printf("%d\t%d\tdone dayphen\n",simyr,yday);
#endif

				/* test for the annual allocation day */
				if (phen.remdays_litfall == 1) annual_alloc = 1;
				else annual_alloc = 0;

				/* phenology fluxes */
				if (ok && phenology(&epc, &phen, &epv, &cs, &cf, &ns, &nf))
				{
					printf("Error in phenology() from bgc()\n");
					ok=0;
				}

#ifdef DEBUG
				printf("%d\t%d\tdone phenology\n",simyr,yday);
#endif

				/* calculate leaf area index, sun and shade fractions, and specific
				leaf area for sun and shade canopy fractions, then calculate
				canopy radiation interception and transmission */
				if (ok && radtrans(&cs, &epc, &metv, &epv, sitec.sw_alb))
				{
					printf("Error in radtrans() from bgc()\n");
					ok=0;
				}

				/* update the ann max LAI for annual diagnostic output */
				if (epv.proj_lai > epv.ytd_maxplai) epv.ytd_maxplai = epv.proj_lai;

#ifdef DEBUG
				printf("%d\t%d\tdone radtrans\n",simyr,yday);
#endif

				/* precip routing (when there is precip) */
				if (ok && metv.prcp && prcp_route(&metv, epc.int_coef, epv.all_lai,
					&wf))
				{
					printf("Error in prcp_route() from bgc()\n");
					ok=0;
				}

#ifdef DEBUG
				printf("%d\t%d\tdone prcp_route\n",simyr,yday);
#endif

				/* snowmelt (when there is a snowpack) */
				if (ok && ws.snoww && snowmelt(&metv, &wf, ws.snoww))
				{
					printf("Error in snowmelt() from bgc()\n");
					ok=0;
				}

#ifdef DEBUG
				printf("%d\t%d\tdone snowmelt\n",simyr,yday);
#endif

				/* bare-soil evaporation (when there is no snowpack) */
				if (ok && !ws.snoww && baresoil_evap(&metv, &wf, &epv.dsr))
				{
					printf("Error in baresoil_evap() from bgc()\n");
					ok=0;
				}

#ifdef DEBUG
				printf("%d\t%d\tdone bare_soil evap\n",simyr,yday);
#endif

				/* soil water potential */
				if (ok && soilpsi(&sitec, ws.soilw, &epv.psi, &epv.vwc))
				{
					printf("Error in soilpsi() from bgc()\n");
					ok=0;
				}

#ifdef DEBUG
				printf("%d\t%d\tdone soilpsi\n",simyr,yday);
#endif

				/* daily maintenance respiration */
				if (ok && maint_resp(&cs, &ns, &epc, &metv, &cf, &epv))
				{
					printf("Error in m_resp() from bgc()\n");
					ok=0;
				}

#ifdef DEBUG
				printf("%d\t%d\tdone maint resp\n",simyr,yday);
#endif

				/* begin canopy bio-physical process simulation */
				/* do canopy ET calculations whenever there is leaf area
				displayed, since there may be intercepted water on the 
				canopy that needs to be dealt with */
				if (ok && cs.leafc && metv.dayl)
				{
					/* conductance and evapo-transpiration */
					if (ok && canopy_et(&metv, &epc, &epv, &wf, 1))
					{
						printf("Error in canopy_et() from bgc()\n");
						ok=0;
					}

#ifdef DEBUG
					printf("%d\t%d\tdone canopy_et\n",simyr,yday);
#endif

				}
				/* do photosynthesis only when it is part of the current
				growth season, as defined by the remdays_curgrowth flag.  This
				keeps the occurrence of new growth consistent with the treatment
				of litterfall and allocation */
				if (ok && cs.leafc && phen.remdays_curgrowth && metv.dayl)
				{
					/* SUNLIT canopy fraction photosynthesis */
					/* set the input variables */
					psn_sun.c3 = epc.c3_flag;
					psn_sun.co2 = metv.co2;
					psn_sun.pa = metv.pa;
					psn_sun.t = metv.tday;
					psn_sun.lnc = 1.0 / (epv.sun_proj_sla * epc.leaf_cn);
					psn_sun.flnr = epc.flnr;
					psn_sun.ppfd = metv.ppfd_per_plaisun;
					/* convert conductance from m/s --> umol/m2/s/Pa, and correct
					for CO2 vs. water vapor */
					psn_sun.g = epv.gl_t_wv_sun * 1e6/(1.6*R*(metv.tday+273.15));
					psn_sun.dlmr = epv.dlmr_area_sun;
					if (ok && photosynthesis(&psn_sun))
					{
						printf("Error in photosynthesis() from bgc()\n");
						ok=0;
					}

#ifdef DEBUG
					printf("%d\t%d\tdone sun psn\n",simyr,yday);
#endif

					epv.assim_sun = psn_sun.A;

					/* for the final flux assignment, the assimilation output
					needs to have the maintenance respiration rate added, this
					sum multiplied by the projected leaf area in the relevant canopy
					fraction, and this total converted from umol/m2/s -> kgC/m2/d */
					cf.psnsun_to_cpool = (epv.assim_sun + epv.dlmr_area_sun) * 
						epv.plaisun * metv.dayl * 12.011e-9; 

					/* SHADED canopy fraction photosynthesis */
					psn_shade.c3 = epc.c3_flag;
					psn_shade.co2 = metv.co2;
					psn_shade.pa = metv.pa;
					psn_shade.t = metv.tday;
					psn_shade.lnc = 1.0 / (epv.shade_proj_sla * epc.leaf_cn);
					psn_shade.flnr = epc.flnr;
					psn_shade.ppfd = metv.ppfd_per_plaishade;
					/* convert conductance from m/s --> umol/m2/s/Pa, and correct
					for CO2 vs. water vapor */
					psn_shade.g = epv.gl_t_wv_shade * 1e6/(1.6*R*(metv.tday+273.15));
					psn_shade.dlmr = epv.dlmr_area_shade;
					if (ok && photosynthesis(&psn_shade))
					{
						printf("Error in photosynthesis() from bgc()\n");
						ok=0;
					}

#ifdef DEBUG
					printf("%d\t%d\tdone shade_psn\n",simyr,yday);
#endif

					epv.assim_shade = psn_shade.A;

					/* for the final flux assignment, the assimilation output
					needs to have the maintenance respiration rate added, this
					sum multiplied by the projected leaf area in the relevant canopy
					fraction, and this total converted from umol/m2/s -> kgC/m2/d */
					cf.psnshade_to_cpool = (epv.assim_shade + epv.dlmr_area_shade) *
						epv.plaishade * metv.dayl * 12.011e-9; 

				} /* end of photosynthesis calculations */
				else
				{
					epv.assim_sun = epv.assim_shade = 0.0;
				}

				/* nitrogen deposition and fixation */
				nf.ndep_to_sminn = sitec.ndep/365.0;
				nf.nfix_to_sminn = sitec.nfix/365.0;

				/* calculate outflow */
				if (ok && outflow(&sitec, &ws, &wf))
				{
					printf("Error in outflow() from bgc.c\n");
					ok=0;
				}

#ifdef DEBUG
				printf("%d\t%d\tdone outflow\n",simyr,yday);
#endif

				/* daily litter and soil decomp and nitrogen fluxes */
				if (ok && decomp(metv.tsoil,&epc,&epv,&sitec,&cs,&cf,&ns,&nf,&nt))
				{
					printf("Error in decomp() from bgc.c\n");
					ok=0;
				}

#ifdef DEBUG
				printf("%d\t%d\tdone decomp\n",simyr,yday);
#endif

				/* Daily allocation gets called whether or not this is a
				current growth day, because the competition between decomp
				immobilization fluxes and plant growth N demand is resolved
				here.  On days with no growth, no allocation occurs, but
				immobilization fluxes are updated normally */
				/* spinup control */
				/* in the rising limb, use the spinup allocation code
				that supplements N supply */
				if (!steady1 && rising && metcycle == 0)
				{
					if (ok && spinup_daily_allocation(&cf,&cs,&nf,&ns,&epc,&epv,&nt,naddfrac))
					{
						printf("Error in daily_allocation() from bgc.c\n");
						ok=0;
					}
				}
				else
				{
					if (ok && daily_allocation(&cf,&cs,&nf,&ns,&epc,&epv,&nt))
					{
						printf("Error in daily_allocation() from bgc.c\n");
						ok=0;
					}
				}

#ifdef DEBUG
				printf("%d\t%d\tdone daily_allocation\n",simyr,yday);
#endif

				/* reassess the annual turnover rates for livewood --> deadwood,
				and for evergreen leaf and fine root litterfall. This happens
				once each year, on the annual_alloc day (the last litterfall day) */
				if (ok && annual_alloc)
				{
					if (ok && annual_rates(&epc,&epv))
					{
						printf("Error in annual_rates() from bgc()\n");
						ok=0;
					}

#ifdef DEBUG
					printf("%d\t%d\tdone annual rates\n",simyr,yday);
#endif
				} 


				/* daily growth respiration */
				if (ok && growth_resp(&epc, &cf))
				{
					printf("Error in daily_growth_resp() from bgc.c\n");
					ok=0;
				}

#ifdef DEBUG
				printf("%d\t%d\tdone growth_resp\n",simyr,yday);
#endif

				/* daily update of the water state variables */
				if (ok && daily_water_state_update(&wf, &ws))
				{
					printf("Error in daily_water_state_update() from bgc()\n");
					ok=0;
				}

#ifdef DEBUG
				printf("%d\t%d\tdone water state update\n",simyr,yday);
#endif

				/* daily update of carbon state variables */
				if (ok && daily_carbon_state_update(&cf, &cs, annual_alloc,
					epc.woody, epc.evergreen))
				{
					printf("Error in daily_carbon_state_update() from bgc()\n");
					ok=0;
				}

#ifdef DEBUG
				printf("%d\t%d\tdone carbon state update\n",simyr,yday);
#endif

				/* daily update of nitrogen state variables */
				if (ok && daily_nitrogen_state_update(&nf, &ns, annual_alloc,
					epc.woody, epc.evergreen))
				{
					printf("Error in daily_nitrogen_state_update() from bgc()\n");
					ok=0;
				}

#ifdef DEBUG
				printf("%d\t%d\tdone nitrogen state update\n",simyr,yday);
#endif

				/* calculate N leaching loss.  This is a special state variable
				update routine, done after the other fluxes and states are
				reconciled in order to avoid negative sminn under heavy leaching
				potential */
				if (ok && nleaching(&ns, &nf, &ws, &wf))
				{
					printf("Error in nleaching() from bgc()\n");
					ok=0;
				}

#ifdef DEBUG
				printf("%d\t%d\tdone nitrogen leaching\n",simyr,yday);
#endif

				/* calculate daily mortality fluxes and update state variables */
				/* this is done last, with a special state update procedure, to
				insure that pools don't go negative due to mortality fluxes
				conflicting with other proportional fluxes */
				if (ok && mortality(&epc,&cs,&cf,&ns,&nf))
				{
					printf("Error in mortality() from bgc()\n");
					ok=0;
				}

#ifdef DEBUG
				printf("%d\t%d\tdone mortality\n",simyr,yday);
#endif

				/* test for water balance */
				if (ok && check_water_balance(&ws, first_balance))
				{
					printf("Error in check_water_balance() from bgc()\n");
					printf("%d\n",metday);
					ok=0;
				}

#ifdef DEBUG
				printf("%d\t%d\tdone water balance\n",simyr,yday);
#endif

				/* test for carbon balance */
				if (ok && check_carbon_balance(&cs, first_balance))
				{
					printf("Error in check_carbon_balance() from bgc()\n");
					printf("%d\n",metday);
					ok=0;
				}

#ifdef DEBUG
				printf("%d\t%d\tdone carbon balance\n",simyr,yday);
#endif

				/* test for nitrogen balance */
				if (ok && check_nitrogen_balance(&ns, first_balance))
				{
					printf("Error in check_nitrogen_balance() from bgc()\n");
					printf("%d\n",metday);
					ok=0;
				}

#ifdef DEBUG
				printf("%d\t%d\tdone nitrogen balance\n",simyr,yday);
#endif

				/* calculate carbon summary variables */
				if (ok && csummary(&cf, &cs, &summary))
				{
					printf("Error in csummary() from bgc()\n");
					ok=0;
				} 

#ifdef DEBUG
				printf("%d\t%d\tdone carbon summary\n",simyr,yday);
#endif

				/* DAILY OUTPUT HANDLING */
				/* fill the daily output array if daily output is requested,
				or if the monthly or annual average of daily output variables
				have been requested */
				if (ok && dayout)
				{
					/* fill the daily output array */
					for (outv=0 ; outv<ctrl.ndayout ; outv++)
					{
						dayarr[outv] = (float) *(output_map[ctrl.daycodes[outv]]);
					}
				}
				/* only write daily outputs if requested */
				if (ok && ctrl.dodaily)
				{
					/* write the daily output array to daily output file */
					if (fwrite(dayarr, sizeof(float), ctrl.ndayout, bgcout->dayout.ptr)
						!= (size_t)ctrl.ndayout)
					{
						printf("Error writing to %s: simyear = %d, simday = %d\n",
							bgcout->dayout.name,simyr,yday);
						ok=0;
					}

#ifdef DEBUG
					printf("%d\t%d\tdone daily output\n",simyr,yday);
#endif

				}
			
				/* MONTHLY AVERAGE OF DAILY OUTPUT VARIABLES */
				if (ctrl.domonavg)
				{
					/* update the monthly average array */
					for (outv=0 ; outv<ctrl.ndayout ; outv++)
					{
						monavgarr[outv] += dayarr[outv];
					}

					/* if this is the last day of the current month, output... */
					if (yday == endday[curmonth])
					{
						/* finish the averages */
						for (outv=0 ; outv<ctrl.ndayout ; outv++)
						{
							monavgarr[outv] /= (float)mondays[curmonth];
						}

						/* write to file */
						if (fwrite(monavgarr, sizeof(float), ctrl.ndayout, bgcout->monavgout.ptr)
							!= (size_t)ctrl.ndayout)
						{
							printf("Error writing to %s: simyear = %d, simday = %d\n",
								bgcout->monavgout.name,simyr,yday);
							ok=0;
						}

						/* reset monthly average variables for next month */
						for (outv=0 ; outv<ctrl.ndayout ; outv++)
						{
							monavgarr[outv] = 0.0;
						}
					
						/* increment current month counter */
						curmonth++;

#ifdef DEBUG
						printf("%d\t%d\tdone monavg output\n",simyr,yday);
#endif

					}
				}

				/* ANNUAL AVERAGE OF DAILY OUTPUT VARIABLES */
				if (ctrl.doannavg)
				{
					/* update the annual average array */
					for (outv=0 ; outv<ctrl.ndayout ; outv++)
					{
						annavgarr[outv] += dayarr[outv];
					}

					/* if this is the last day of the year, output... */
					if (yday == 364)
					{
						/* finish averages */
						for (outv=0 ; outv<ctrl.ndayout ; outv++)
						{
							annavgarr[outv] /= 365.0;
						}

						/* write to file */
						if (fwrite(annavgarr, sizeof(float), ctrl.ndayout, bgcout->annavgout.ptr)
							!= (size_t)ctrl.ndayout)
						{
							printf("Error writing to %s: simyear = %d, simday = %d\n",
								bgcout->annavgout.name,simyr,yday);
							ok=0;
						}

						/* reset annual average variables for next month */
						for (outv=0 ; outv<ctrl.ndayout ; outv++)
						{
							annavgarr[outv] = 0.0;
						}
					
#ifdef DEBUG
						printf("%d\t%d\tdone annavg output\n",simyr,yday);
#endif

					}
				}
				
				/* spinup control */
				/* keep a tally of total soil C during successive
				met cycles for comparison */
				if (metcycle == 1)
				{
					tally1 += summary.soilc;
					tally1b += summary.totalc;
				}
				if (metcycle == 2)
				{
					tally2 += summary.soilc;
					tally2b += summary.totalc;
				}

				/* at the end of first day of simulation, turn off the 
				first_balance switch */
				if (first_balance) first_balance = 0;

			}   /* end of daily model loop */

			/* ANNUAL OUTPUT HANDLING */
			/* only write annual outputs if requested */
			if (ok && ctrl.doannual)
			{
				/* fill the annual output array */
				for (outv=0 ; outv<ctrl.nannout ; outv++)
				{
					annarr[outv] = (float) *output_map[ctrl.anncodes[outv]];
				}
				/* write the annual output array to annual output file */
				if (fwrite(annarr, sizeof(float), ctrl.nannout, bgcout->annout.ptr)
					!= (size_t)ctrl.nannout)
				{
					printf("Error writing to %s: simyear = %d, simday = %d\n",
						bgcout->annout.name,simyr,yday);
					ok=0;
				}

#ifdef DEBUG
				printf("%d\t%d\tdone annual output\n",simyr,yday);
#endif
			}
			metyr++;
			
			/* spinup control */
			spinyears++;
			
		}   /* end of annual model loop */
		
		/* spinup control */
		/* if this is the third pass through metcycle, do comparison */
		/* first block is during the rising phase */
		if (!steady1 && metcycle == 2)
		{
			/* convert tally1 and tally2 to average daily soilc */
			tally1 /= (double)nblock * 365.0;
			tally2 /= (double)nblock * 365.0;
			rising = (tally2 > tally1);
			t1 = (tally2-tally1)/(double)nblock;
			steady1 = (fabs(t1) < SPINUP_TOLERANCE);
			
#ifdef DEBUG_SPINUP
			printf("spinyears = %d rising = %d steady1 = %d\n",spinyears,
				rising,steady1);
			printf("metcycle = %d tally1 = %lf tally2 = %lf pdif = %lf\n\n",
				metcycle,tally1,tally2,t1);
			if (steady1) printf("SWITCH\n\n");
#endif
				
			metcycle = 0;
		}
		/* second block is after supplemental N turned off */
		else if (steady1 && metcycle == 2)
		{
			/* convert tally1 and tally2 to average daily soilc */
			tally1 /= (double)nblock * 365.0;
			tally2 /= (double)nblock * 365.0;
			t1 = (tally2-tally1)/(double)nblock;
			steady2 = (fabs(t1) < SPINUP_TOLERANCE);

			/* if rising above critical rate, back to steady1=0 */
			if (t1 > SPINUP_TOLERANCE)
			{
#ifdef DEBUG_SPINUP
				printf("\nSWITCH BACK\n");
#endif
				
				steady1 = 0;
				rising = 1;
			}
			
#ifdef DEBUG_SPINUP
			printf("spinyears = %d rising = %d steady2 = %d\n",spinyears,
				rising,steady2);
			printf("metcycle = %d tally1 = %lf tally2 = %lf pdif = %lf\n\n",
				metcycle,tally1,tally2,t1);
#endif
				
			metcycle = 0;
		}
			
		else
		{
#ifdef DEBUG_SPINUP
			printf("spinyears = %d rising = %d steady1 = %d\n",spinyears,
				rising,steady1);
			printf("metcycle = %d tally1 = %lf tally2 = %lf pdif = %lf\n",
				metcycle,tally1,tally2,t1);
#endif
		
			metcycle++;
		}

		
	/* end of do block, test for steady state */	
	} while (!(steady1 && steady2) && (spinyears < ctrl.maxspinyears || 
		metcycle != 0));
	
	/* save some information on the end status of spinup */
	tally1b /= (double)nblock * 365.0;
	tally2b /= (double)nblock * 365.0;
	bgcout->spinup_resid_trend = (tally2b-tally1b)/(double)nblock;
	bgcout->spinup_years = spinyears;

	/* RESTART OUTPUT HANDLING */
	/* if write_restart flag is set, copy data to the output restart struct */
	if (ok && ctrl.write_restart)
	{
		if (restart_output(&ctrl, &ws, &cs, &ns, &epv, metyr, 
			&(bgcout->restart_output)))
		{
			printf("Error in call to restart_output() from bgc()\n");
			ok=0;
		}
		
#ifdef DEBUG
		printf("%d\t%d\tdone restart output\n",simyr,yday);
#endif
	}

	/* free phenology memory */
	if (ok && free_phenmem(&phenarr))
	{
		printf("Error in free_phenmem() from bgc()\n");
		ok=0;
	}

#ifdef DEBUG
	printf("%d\t%d\tdone free phenmem\n",simyr,yday);
#endif
	
	/* free memory for local output arrays */
	if (dayout) free(dayarr);
	if (ctrl.domonavg) free(monavgarr);
	if (ctrl.doannavg) free(annavgarr);
	if (ctrl.doannual) free(annarr);
	free(output_map);
		
	/* print timing info if error */
	if (!ok)
	{
		printf("ERROR at year %d\n",simyr-1);
		printf("ERROR at yday %d\n",yday-1);
	}
	
	/* return error status */	
	return (!ok);
}
