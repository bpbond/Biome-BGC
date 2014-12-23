/*
bgc.c
Core BGC model logic

Includes in-line output handling routines that write to daily and annual
output files. This is the only library module that has external
I/O connections, and so it is the only module that includes bgc_io.h.

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information

Revisions since 4.1.2
	Merged spinup_bgc.c with bgc.c to eliminate
	code duplication
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "bgc.h"

/* These DEBUG defines are now depricated. Please use 
   bgc_printf(BV_DIAG,...) instead. The only place where 
	 a DEBUG define is still used is inside bgc_printf(). */

/* #define DEBUG */
/* #define DEBUG_SPINUP set this to see the spinup details on-screen */

/*	SANE = Do 'Pan-Arctic' style summary. INSANE is traditional style 
		summary. See the '-p' cli flag in USAGE.TXT */
signed char summary_sanity = INSANE ;

int bgc(bgcin_struct* bgcin, bgcout_struct* bgcout, int mode)
{
	extern signed char summary_sanity;
	/* variable declarations */
	int ok=1;

	/* iofiles and program control variables */
	control_struct     ctrl;

	/* meteorological variables */
	metarr_struct      metarr;
	metvar_struct      metv;
	co2control_struct  co2;
	ramp_ndep_struct ramp_ndep;
	
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

	/* photosynthesis constructs */
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

	/* mode == MODE_MODEL only */
	double daily_ndep, daily_nfix, ndep_scalar, ndep_diff, ndep;
	int ind_simyr;
	
	/* variables used for monthly average output */
	int curmonth;
	int mondays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	int endday[12] = {30,58,89,119,150,180,211,242,272,303,333,364};
	float monmaxlai = 0.0,annmaxlai = 0.0,monmaxsnoww = 0.0;
	float eomsnoww = 0.0,eomsoilw = 0.0;

	int tmpyears;

	/* mode == MODE_SPINUP only */
	/* spinup control */
	int ntimesmet, nblock;
	int steady1, steady2, rising, metcycle = 0, spinyears;
	double tally1 = 0.0, tally1b = 0.0, tally2 = 0.0, tally2b = 0.0, t1 = 0.0;
	double naddfrac;
	
	/* mode == MODE_MODEL only */
	/* simple annual variables for text output */
	double annmaxplai,annet,annoutflow,annnpp,annnbp, annprcp,anntavg;

	if (mode != MODE_SPINUP && mode != MODE_MODEL)
	{
		bgc_printf(BV_ERROR, "Error: Unknown MODE given when calling bgc()\n");
		ok=0;
	}
	
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
	if (mode == MODE_MODEL)
	{
		ramp_ndep = bgcin->ramp_ndep;
	}
	
	bgc_printf(BV_DIAG, "done copy input\n");

	/* local variable that signals the need for daily output array */
	dayout = (ctrl.dodaily || ctrl.domonavg || ctrl.doannavg);

	/* allocate memory for local output arrays */
	if (ok && dayout &&	!(dayarr = (float*) malloc(ctrl.ndayout * sizeof(float))))
	{
		bgc_printf(BV_ERROR, "Error allocating for local daily output array in bgc()\n");
		ok=0;
	}
	if (ok && ctrl.domonavg && !(monavgarr = (float*) malloc(ctrl.ndayout * sizeof(float))))
	{
		bgc_printf(BV_ERROR, "Error allocating for monthly average output array in bgc()\n");
		ok=0;
	}
	if (ok && ctrl.doannavg && !(annavgarr = (float*) malloc(ctrl.ndayout * sizeof(float))))
	{
		bgc_printf(BV_ERROR, "Error allocating for annual average output array in bgc()\n");
		ok=0;
	}
	if (ok && ctrl.doannual && !(annarr = (float*) malloc(ctrl.nannout * sizeof(float))))
	{
		bgc_printf(BV_ERROR, "Error allocating for local annual output array in bgc()\n");
		ok=0;
	}
	/* allocate space for the output map pointers */
	if (ok && !(output_map = (double**) malloc(NMAP * sizeof(double*))))
	{
		bgc_printf(BV_ERROR, "Error allocating for output map in output_map_init()\n");
		ok=0;
	}
	
	bgc_printf(BV_DIAG, "done allocate out arrays\n");
	
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
		bgc_printf(BV_ERROR, "Error in call to output_map_init() from bgc()\n");
		ok=0;
	}
	
	bgc_printf(BV_DIAG, "done initialize outmap\n");
	
	/* make zero-flux structures for use inside annual and daily loops */
	if (ok && make_zero_flux_struct(&zero_wf, &zero_cf, &zero_nf))
	{
		bgc_printf(BV_ERROR, "Error in call to make_zero_flux_struct() from bgc()\n");
		ok=0;
	}

	bgc_printf(BV_DIAG, "done make_zero_flux\n");
	
	/* atmospheric pressure (Pa) as a function of elevation (m) */
	if (ok && atm_pres(sitec.elev, &metv.pa))
	{
		bgc_printf(BV_ERROR, "Error in atm_pres() from bgc()\n");
		ok=0;
	}
	
	bgc_printf(BV_DIAG, "done atm_pres\n");
	
	/* determine phenological signals */
	if (ok && prephenology(&ctrl, &epc, &sitec, &metarr, &phenarr))
	{
		bgc_printf(BV_ERROR, "Error in call to prephenology(), from bgc()\n");
		ok=0;
	}
	
	bgc_printf(BV_DIAG, "done prephenology\n");
	
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
			bgc_printf(BV_ERROR, "Error in call to restart_input() from bgc()\n");
			ok=0;
		}
		
		bgc_printf(BV_DIAG, "done restart_input\n");
	
	}
	else
	/* no restart file, user supplies initial conditions */
	{
		/* initialize leaf C and N pools depending on phenology signals for
		the first metday */
		if (ok && firstday(&epc, &cinit, &epv, &phenarr, &cs, &ns))
		{
			bgc_printf(BV_ERROR, "Error in call to firstday(), from bgc()\n");
			ok=0;
		}
		
		/* initial value for metyr */
		metyr = 0;
		
		bgc_printf(BV_DIAG, "done firstday\n");
	}

	/* zero water, carbon, and nitrogen source and sink variables */
	if (ok && zero_srcsnk(&cs,&ns,&ws,&summary))
	{
		bgc_printf(BV_ERROR, "Error in call to zero_srcsnk(), from bgc()\n");
		ok=0;
	}

	bgc_printf(BV_DIAG, "done zero_srcsnk\n");
	
	/* initialize the indicator for first day of current simulation, so
	that the checks for mass balance can have two days for comparison */
	first_balance = 1;

	/* mode == MODE_SPINUP only*/
	if (mode == MODE_SPINUP)
	{
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
	}

	if (mode == MODE_MODEL)
	{
		tmpyears = ctrl.simyears;
	}
	else if (mode == MODE_SPINUP)
	{
		tmpyears = nblock;
	}
	
	/* do loop for spinup. will only execute once for MODE_MODEL */
	do
	{
	
	/* begin the annual model loop */
	for (simyr=0 ; ok && simyr<tmpyears ; simyr++)
	{
		if (mode == MODE_MODEL)
		{
			/* reset the simple annual output variables for text output */
			annmaxlai = 0.0;
			annet = 0.0;
			annoutflow = 0.0;
			annnpp = 0.0;
			annnbp = 0.0;
			annprcp = 0.0;
			anntavg = 0.0;
		}
		
		/* set current month to 0 (january) at the beginning of each year */
		curmonth = 0;

		if (mode == MODE_SPINUP)
		{
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
		}
		
		/* test whether metyr needs to be reset */
		if (metyr == ctrl.metyears)
		{
			if (mode == MODE_MODEL)
			{
				if (ctrl.onscreen) bgc_printf(BV_DETAIL, "Resetting met data for cyclic input\n");
			}
			if (mode == MODE_SPINUP)
			{
				bgc_printf(BV_DIAG, "Resetting met data for cyclic input\n");
			}
			metyr = 0;
		}

		if (mode == MODE_MODEL)
		{
			/* output to screen to indicate start of simulation year */
			if (ctrl.onscreen) bgc_printf(BV_DETAIL, "Year: %6d\n",ctrl.simstartyear+simyr);
		}
		else if (mode == MODE_SPINUP)
		{
			/* output to screen to indicate start of simulation year */
			if (ctrl.onscreen) bgc_printf(BV_DETAIL, "Year: %6d\n",spinyears);
		}

		/* set the max lai variable, for annual diagnostic output */
		epv.ytd_maxplai = 0.0;
		
		if (mode == MODE_MODEL)
		{
			/* atmospheric CO2 and Ndep handling */
			if (!(co2.varco2))
			{
				/* constant CO2, constant Ndep */
				metv.co2 = co2.co2ppm;
				daily_ndep = sitec.ndep/365.0;
				daily_nfix = sitec.nfix/365.0;
			}
			else 
			{
				/* when varco2 = 1, use file for co2 */
				if (co2.varco2 == 1) metv.co2 = get_co2(&co2,(ctrl.simstartyear+simyr));
                bgc_printf(BV_DIAG,"CO2 val: %lf Year: %i\n",metv.co2,(ctrl.simstartyear+simyr));
				if(metv.co2 < -999)
				{
					bgc_printf(BV_ERROR,"Error finding CO2 value for year: %i\n",(ctrl.simstartyear+simyr));
					return(EXIT_FAILURE);
				}

				/* when varco2 = 2, use the constant CO2 value, but vary Ndep */
				if (co2.varco2 == 2) metv.co2 = co2.co2ppm;
				
				if (ramp_ndep.doramp && !bgcin->ndepctrl.varndep)
				{
					/* increasing CO2, ramped Ndep */
					ind_simyr = ramp_ndep.ind_year - ctrl.simstartyear;
					ndep_scalar = (ramp_ndep.ind_ndep - ramp_ndep.preind_ndep) / 
						(co2.co2ppm_array[ind_simyr]-co2.co2ppm_array[0]);
					ndep_diff = (co2.co2ppm_array[simyr] - co2.co2ppm_array[0]) * 
						ndep_scalar;
					ndep = ramp_ndep.preind_ndep + ndep_diff;
					/* don't allow the industrial ndep levels to be less than
					the preindustrial levels */
					if (ndep < ramp_ndep.preind_ndep) ndep = ramp_ndep.preind_ndep;
					daily_ndep = ndep/365.0;
					daily_nfix = sitec.nfix/365.0;
				}
				else
				{
					/* increasing CO2, constant Ndep */
					daily_ndep = sitec.ndep/365.0;
					daily_nfix = sitec.nfix/365.0;
					
				}
			}
			if(bgcin->ndepctrl.varndep && mode == MODE_MODEL)
			{

				daily_ndep = get_ndep(&bgcin->ndepctrl,(ctrl.simstartyear + simyr));
				if(daily_ndep < -999)
				{
					bgc_printf(BV_ERROR, "Error finding NDEP for year: %i\n",(ctrl.simstartyear+simyr));
					return(EXIT_FAILURE);
				}
				else
				{
					bgc_printf(BV_DIAG, "Using annual NDEP value: %lf\n",daily_ndep);				
					daily_ndep /= 365.0;	
				}
			}
		}
		else if (mode == MODE_SPINUP)
		{
			/* atmospheric concentration of CO2 (ppm) */
			/* Always assign a fixed CO2 value for spinups */
			metv.co2 = co2.co2ppm;

			/*if (!(co2.varco2)) 
			else metv.co2 = co2.co2ppm_array[simyr]; */
		}

		/* begin the daily model loop */
		for (yday=0 ; ok && yday<365 ; yday++)
		{
			bgc_printf(BV_DIAG, "year %d\tyday %d\n",simyr,yday);
			
			/* Test for very low state variable values and force them
			to 0.0 to avoid rounding and floating point overflow errors */
			if (ok && precision_control(&ws, &cs, &ns))
			{
				bgc_printf(BV_ERROR, "Error in call to precision_control() from bgc()\n");
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
				bgc_printf(BV_ERROR, "Error in daymet() from bgc()\n");
				ok=0;
			}
			
			bgc_printf(BV_DIAG, "%d\t%d\tdone daymet\n",simyr,yday);
	
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
				bgc_printf(BV_ERROR, "Error in dayphen() from bgc()\n");
				ok=0;
			}
			
			bgc_printf(BV_DIAG, "%d\t%d\tdone dayphen\n",simyr,yday);
	
			/* test for the annual allocation day */
			if (phen.remdays_litfall == 1) annual_alloc = 1;
			else annual_alloc = 0;
			
			/* phenology fluxes */
			if (ok && phenology(&epc, &phen, &epv, &cs, &cf, &ns, &nf))
			{
				bgc_printf(BV_ERROR, "Error in phenology() from bgc()\n");
				ok=0;
			}
			
			bgc_printf(BV_DIAG, "%d\t%d\tdone phenology\n",simyr,yday);
	
			/* calculate leaf area index, sun and shade fractions, and specific
			leaf area for sun and shade canopy fractions, then calculate
			canopy radiation interception and transmission */
			if (ok && radtrans(&cs, &epc, &metv, &epv, sitec.sw_alb))
			{
				bgc_printf(BV_ERROR, "Error in radtrans() from bgc()\n");
				ok=0;
			}
			
			/* update the ann max LAI for annual diagnostic output */
			if (epv.proj_lai > epv.ytd_maxplai) epv.ytd_maxplai = epv.proj_lai;
			
			bgc_printf(BV_DIAG, "%d\t%d\tdone radtrans\n",simyr,yday);
			
			/* precip routing (when there is precip) */
			if (ok && metv.prcp && prcp_route(&metv, epc.int_coef, epv.all_lai,
				&wf))
			{
				bgc_printf(BV_ERROR, "Error in prcp_route() from bgc()\n");
				ok=0;
			}
			
			bgc_printf(BV_DIAG, "%d\t%d\tdone prcp_route\n",simyr,yday);

			/* snowmelt (when there is a snowpack) */
			if (ok && ws.snoww && snowmelt(&metv, &wf, ws.snoww))
			{
				bgc_printf(BV_ERROR, "Error in snowmelt() from bgc()\n");
				ok=0;
			}
			
			bgc_printf(BV_DIAG, "%d\t%d\tdone snowmelt\n",simyr,yday);

			/* bare-soil evaporation (when there is no snowpack) */
			if (ok && !ws.snoww && baresoil_evap(&metv, &wf, &epv.dsr))
			{
				bgc_printf(BV_ERROR, "Error in baresoil_evap() from bgc()\n");
				ok=0;
			}

			bgc_printf(BV_DIAG, "%d\t%d\tdone bare_soil evap\n",simyr,yday);

			/* soil water potential */
			if (ok && soilpsi(&sitec, ws.soilw, &epv.psi, &epv.vwc))
			{
				bgc_printf(BV_ERROR, "Error in soilpsi() from bgc()\n");
				ok=0;
			}
			
			bgc_printf(BV_DIAG, "%d\t%d\tdone soilpsi\n",simyr,yday);

			/* daily maintenance respiration */
			if (ok && maint_resp(&cs, &ns, &epc, &metv, &cf, &epv))
			{
				bgc_printf(BV_ERROR, "Error in m_resp() from bgc()\n");
				ok=0;
			}

			bgc_printf(BV_DIAG, "%d\t%d\tdone maint resp\n",simyr,yday);

			/* begin canopy bio-physical process simulation */
			/* do canopy ET calculations whenever there is leaf area
			displayed, since there may be intercepted water on the 
			canopy that needs to be dealt with */
			if (ok && cs.leafc && metv.dayl)
			{
				/* conductance and evapo-transpiration */
				if (ok && canopy_et(&metv, &epc, &epv, &wf, 1))
				{
					bgc_printf(BV_ERROR, "Error in canopy_et() from bgc()\n");
					ok=0;
				}
				
				bgc_printf(BV_DIAG, "%d\t%d\tdone canopy_et\n",simyr,yday);

			}
			/* do photosynthesis only when it is part of the current
			growth season, as defined by the remdays_curgrowth flag.  This
			keeps the occurrence of new growth consistent with the treatment
			of litterfall and allocation */
			if (ok && cs.leafc && phen.remdays_curgrowth && metv.dayl)
			{
				if (ok && total_photosynthesis(&metv, &epc, &epv, &cf, &psn_sun, &psn_shade))
				{
					bgc_printf(BV_ERROR, "Error in total_photosynthesis() from bgc()\n");
					ok=0;
				}
				
			} /* end of photosynthesis calculations */
			else
			{
				epv.assim_sun = epv.assim_shade = 0.0;
			}

			if (mode == MODE_MODEL)
			{
				/* nitrogen deposition and fixation */
				nf.ndep_to_sminn = daily_ndep;
				nf.nfix_to_sminn = daily_nfix;
			}
			else if (mode == MODE_SPINUP)
			{
				/* nitrogen deposition and fixation */
				nf.ndep_to_sminn = sitec.ndep/365.0;
				nf.nfix_to_sminn = sitec.nfix/365.0;
			}

			/* calculate outflow */
			if (ok && outflow(&sitec, &ws, &wf))
			{
				bgc_printf(BV_ERROR, "Error in outflow() from bgc.c\n");
				ok=0;
			}
			
			bgc_printf(BV_DIAG, "%d\t%d\tdone outflow\n",simyr,yday);

			/* daily litter and soil decomp and nitrogen fluxes */
			if (ok && decomp(metv.tsoil,&epc,&epv,&sitec,&cs,&cf,&ns,&nf,&nt))
			{
				bgc_printf(BV_ERROR, "Error in decomp() from bgc.c\n");
				ok=0;
			}
			
			bgc_printf(BV_DIAG, "%d\t%d\tdone decomp\n",simyr,yday);

			/* Daily allocation gets called whether or not this is a
			current growth day, because the competition between decomp
			immobilization fluxes and plant growth N demand is resolved
			here.  On days with no growth, no allocation occurs, but
			immobilization fluxes are updated normally */
			if (mode == MODE_MODEL)
			{
				if (ok && daily_allocation(&cf,&cs,&nf,&ns,&epc,&epv,&nt,1.0,MODE_MODEL))
				{
					bgc_printf(BV_ERROR, "Error in daily_allocation() from bgc.c\n");
					ok=0;
				}
			}
			else if (mode == MODE_SPINUP)
			{
				/* spinup control */
				/* in the rising limb, use the spinup allocation code
				that supplements N supply */
				if (!steady1 && rising && metcycle == 0)
				{
					if (ok && daily_allocation(&cf,&cs,&nf,&ns,&epc,&epv,&nt,naddfrac,MODE_SPINUP))
					{
						bgc_printf(BV_ERROR, "Error in daily_allocation() from bgc.c\n");
						ok=0;
					}
				}
				else
				{
					if (ok && daily_allocation(&cf,&cs,&nf,&ns,&epc,&epv,&nt,1.0,MODE_MODEL))
					{
						bgc_printf(BV_ERROR, "Error in daily_allocation() from bgc.c\n");
						ok=0;
					}
				}
			}
			
			bgc_printf(BV_DIAG, "%d\t%d\tdone daily_allocation\n",simyr,yday);

			/* reassess the annual turnover rates for livewood --> deadwood,
			and for evergreen leaf and fine root litterfall. This happens
			once each year, on the annual_alloc day (the last litterfall day) */
			if (ok && annual_alloc)
			{
				if (ok && annual_rates(&epc,&epv))
				{
					bgc_printf(BV_ERROR, "Error in annual_rates() from bgc()\n");
					ok=0;
				}
				
				bgc_printf(BV_DIAG, "%d\t%d\tdone annual rates\n",simyr,yday);
			} 


			/* daily growth respiration */
			if (ok && growth_resp(&epc, &cf))
			{
				bgc_printf(BV_ERROR, "Error in daily_growth_resp() from bgc.c\n");
				ok=0;
			}
			
			bgc_printf(BV_DIAG, "%d\t%d\tdone growth_resp\n",simyr,yday);

			/* daily update of the water state variables */
			if (ok && daily_water_state_update(&wf, &ws))
			{
				bgc_printf(BV_ERROR, "Error in daily_water_state_update() from bgc()\n");
				ok=0;
			}
			
			bgc_printf(BV_DIAG, "%d\t%d\tdone water state update\n",simyr,yday);

			/* daily update of carbon state variables */
			if (ok && daily_carbon_state_update(&cf, &cs, annual_alloc,
				epc.woody, epc.evergreen))
			{
				bgc_printf(BV_ERROR, "Error in daily_carbon_state_update() from bgc()\n");
				ok=0;
			}

			bgc_printf(BV_DIAG, "%d\t%d\tdone carbon state update\n",simyr,yday);

			/* daily update of nitrogen state variables */
			if (ok && daily_nitrogen_state_update(&nf, &ns, annual_alloc,
				epc.woody, epc.evergreen))
			{
				bgc_printf(BV_ERROR, "Error in daily_nitrogen_state_update() from bgc()\n");
				ok=0;
			}
			
			bgc_printf(BV_DIAG, "%d\t%d\tdone nitrogen state update\n",simyr,yday);

			/* calculate N leaching loss.  This is a special state variable
			update routine, done after the other fluxes and states are
			reconciled in order to avoid negative sminn under heavy leaching
			potential */
			if (ok && nleaching(&ns, &nf, &ws, &wf))
			{
				bgc_printf(BV_ERROR, "Error in nleaching() from bgc()\n");
				ok=0;
			}
			
			bgc_printf(BV_DIAG, "%d\t%d\tdone nitrogen leaching\n",simyr,yday);

			/* calculate daily mortality fluxes and update state variables */
			/* this is done last, with a special state update procedure, to
			insure that pools don't go negative due to mortality fluxes
			conflicting with other proportional fluxes */
			if (ok && mortality(&epc,&cs,&cf,&ns,&nf))
			{
				bgc_printf(BV_ERROR, "Error in mortality() from bgc()\n");
				ok=0;
			}
			
			bgc_printf(BV_DIAG, "%d\t%d\tdone mortality\n",simyr,yday);

			/* test for water balance */
			if (ok && check_water_balance(&ws, first_balance))
			{
				bgc_printf(BV_ERROR, "Error in check_water_balance() from bgc()\n");
				bgc_printf(BV_ERROR, "%d\n",metday);
				ok=0;
			}
			
			bgc_printf(BV_DIAG, "%d\t%d\tdone water balance\n",simyr,yday);

			/* test for carbon balance */
			if (ok && check_carbon_balance(&cs, first_balance))
			{
				bgc_printf(BV_ERROR, "Error in check_carbon_balance() from bgc()\n");
				bgc_printf(BV_ERROR, "%d\n",metday);
				ok=0;
			}
			
			bgc_printf(BV_DIAG, "%d\t%d\tdone carbon balance\n",simyr,yday);

			/* test for nitrogen balance */
			if (ok && check_nitrogen_balance(&ns, first_balance))
			{
				bgc_printf(BV_ERROR, "Error in check_nitrogen_balance() from bgc()\n");
				bgc_printf(BV_ERROR, "%d\n",metday);
				ok=0;
			}
			
			bgc_printf(BV_DIAG, "%d\t%d\tdone nitrogen balance\n",simyr,yday);

			/* calculate carbon summary variables */
			if (ok && csummary(&cf, &cs, &summary))
			{
				bgc_printf(BV_ERROR, "Error in csummary() from bgc()\n");
				ok=0;
			} 
			
			bgc_printf(BV_DIAG, "%d\t%d\tdone carbon summary\n",simyr,yday);

			/* calculate water summary variables */
			if (ok && wsummary(&ws,&wf,&summary))
			{
				printf("Error in wsummary() from bgc()\n");
				ok=0;
			}

			bgc_printf(BV_DIAG, "%d\t%d\tdone water summary\n", simyr,yday);

			/* DAILY OUTPUT HANDLING */
			/* fill the daily output array if daily output is requested,
			or if the monthly or annual average of daily output variables
			have been requested */
			bgc_printf(BV_DIAG, "Number of daily outputs: %d\n", ctrl.ndayout);
			if (ok && dayout)
			{
				/* fill the daily output array */
				for (outv=0 ; outv<ctrl.ndayout ; outv++)
				{
					bgc_printf(BV_DIAG, "Outv: %d, ", outv);
					bgc_printf(BV_DIAG, "DayCode: %d, ", ctrl.daycodes[outv]);
					bgc_printf(BV_DIAG, "Output: %f\n", *output_map[ctrl.daycodes[outv]]);
					dayarr[outv] = (float) *output_map[ctrl.daycodes[outv]];
				}
			}
			/* only write daily outputs if requested */
			if (ok && ctrl.dodaily)
			{
				/* write the daily output array to daily output file */
				if (fwrite(dayarr, sizeof(float), ctrl.ndayout, bgcout->dayout.ptr)
					!= (size_t)ctrl.ndayout)
				{
					bgc_printf(BV_ERROR, "Error writing to %s: simyear = %d, simday = %d\n",
						bgcout->dayout.name,simyr,yday);
					ok=0;
				}
				
				bgc_printf(BV_DIAG, "%d\t%d\tdone daily output\n",simyr,yday);
				if(ok && bgcout->bgc_ascii)
				{	
				
					output_ascii(dayarr,ctrl.ndayout,bgcout->dayoutascii.ptr);
				
				}
				
			}
			/*******************/
			/* MONTHLY OUTPUTS */
			/*******************/
			
			/* MONTHLY AVERAGE OF DAILY OUTPUT VARIABLES */
			if (ctrl.domonavg)
			{
				/* update the monthly average array */
				for (outv=0 ; outv<ctrl.ndayout ; outv++)
				{
					monavgarr[outv] += dayarr[outv];

					switch (ctrl.daycodes[outv])
					{
						/* Leaf area index */
						case 545:   
							if(dayarr[outv] > monmaxlai) monmaxlai = dayarr[outv]; 
							break;
					}
				}
				
				/* if this is the last day of the current month, output... */
				if (yday == endday[curmonth])
				{
					/* finish the averages */
					for (outv=0 ; outv<ctrl.ndayout ; outv++)
					{
						if (summary_sanity == SANE)
						{
							switch (ctrl.daycodes[outv])
							{
								/* Leaf area index */
								/* Maximum monthly */
								case 545:
									monavgarr[outv] = monmaxlai; 
									break;
								/* Snow water */
								case 21:
									monavgarr[outv] = dayarr[outv] - eomsnoww; 
									eomsnoww = dayarr[outv]; 
									break;
								/* Soil water content */
								case 20:
									monavgarr[outv] = dayarr[outv] - eomsoilw;
									eomsoilw = dayarr[outv];
									break;
								default:
									monavgarr[outv] /= (float)mondays[curmonth];
									break;
							}
						}
						else 
						{
							monavgarr[outv] /= (float)mondays[curmonth];
						}
					}
					
					/* write to file */
					if (fwrite(monavgarr, sizeof(float), ctrl.ndayout, bgcout->monavgout.ptr)
						!= (size_t)ctrl.ndayout)
					{
						bgc_printf(BV_ERROR, "Error writing to %s: simyear = %d, simday = %d\n",
							bgcout->monavgout.name,simyr,yday);
						ok=0;
					}
					
					if(ok && bgcout->bgc_ascii)
					{
						output_ascii(monavgarr,ctrl.ndayout, bgcout->monoutascii.ptr);
						
					}
					
					/* reset monthly average variables for next month */
					for (outv=0 ; outv<ctrl.ndayout ; outv++)
					{
						monavgarr[outv] = 0.0;
						monmaxlai = 0.0;
						monmaxsnoww = 0.0;
					}
					
					/* increment current month counter */
					curmonth++;
					
					bgc_printf(BV_DIAG, "%d\t%d\tdone monavg output\n",simyr,yday);
				
				}
			}
			
			/* ANNUAL AVERAGE OF DAILY OUTPUT VARIABLES */
			if (ctrl.doannavg)
			{
				/* update the annual average array */
				for (outv=0 ; outv<ctrl.ndayout ; outv++)
				{
					annavgarr[outv] += dayarr[outv];
					switch (ctrl.daycodes[outv])
					{
						/* Leaf area index */
						case 545:
							if(dayarr[outv] > annmaxplai) annmaxplai = dayarr[outv];
							break;
					}
				}
				
				/* if this is the last day of the year, output... */
				if (yday == 364)
				{
					/* finish averages */
					for (outv=0 ; outv<ctrl.ndayout ; outv++)
					{
						if (summary_sanity == SANE)
						{
							switch (ctrl.daycodes[outv])
							{
								/* Leaf area index*/ 
								case 545:
									annavgarr[outv] = (float)annmaxplai;
									break;
								default: 
									annavgarr[outv] /= 365.0;
									break;
							}
						}
						else
						{
							annavgarr[outv] /= 365.0;
						}
					}
					
					/* write to file */
					if (fwrite(annavgarr, sizeof(float), ctrl.ndayout, bgcout->annavgout.ptr)
						!= (size_t)ctrl.ndayout)
					{
						bgc_printf(BV_ERROR, "Error writing to %s: simyear = %d, simday = %d\n",
							bgcout->annavgout.name,simyr,yday);
						ok=0;
					}
					
					/* reset annual average variables for next month */
					for (outv=0 ; outv<ctrl.ndayout ; outv++)
					{
						annavgarr[outv] = 0.0;
						annmaxplai = 0.0;
					}
					
					bgc_printf(BV_DIAG, "%d\t%d\tdone annavg output\n",simyr,yday);
				
				}
			}
			
			if (mode == MODE_MODEL)
			{
				/* very simple annual summary variables for text file output */
				if (epv.proj_lai > (double)annmaxlai) annmaxlai = (float)epv.proj_lai;
				annet += wf.canopyw_evap + wf.snoww_subl + wf.soilw_evap +
					wf.soilw_trans;
				annoutflow += wf.soilw_outflow;
				annnpp += summary.daily_npp * 1000.0;
				annnbp += summary.daily_nee * 1000.0;
				annprcp += metv.prcp;
				anntavg += metv.tavg/365.0;
			}
			else if (mode == MODE_SPINUP)
			{
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
				bgc_printf(BV_ERROR, "Error writing to %s: simyear = %d, simday = %d\n",
					bgcout->annout.name,simyr,yday);
				ok=0;
			}
			
			if(ok && bgcout->bgc_ascii)
			{
			
				output_ascii(annarr,ctrl.nannout,bgcout->annoutascii.ptr);
				
			}
			bgc_printf(BV_DIAG, "%d\t%d\tdone annual output\n",simyr,yday);
		}
		
		if (mode == MODE_MODEL && bgcout->bgc_ascii)
		{
			/* write the simple annual text output */
			fprintf(bgcout->anntext.ptr,"%6d%10.1f%10.1f%10.1f%10.1f%10.1f%10.1f%10.1f\n",
				ctrl.simstartyear+simyr,annprcp,anntavg,annmaxlai,annet,annoutflow,annnpp,annnbp);
		}
			
		metyr++;

		if (mode == MODE_SPINUP)
		{
			/* spinup control */
			spinyears++;
		}

	}   /* end of annual model loop */

	if (mode == MODE_SPINUP)
	{
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

			bgc_printf(BV_DIAG, "spinyears = %d rising = %d steady1 = %d\n",spinyears,
				rising,steady1);
			bgc_printf(BV_DIAG, "metcycle = %d tally1 = %lf tally2 = %lf pdif = %lf\n\n",
				metcycle,tally1,tally2,t1);
			if (steady1) bgc_printf(BV_DIAG, "SWITCH\n\n");

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
				bgc_printf(BV_DIAG, "\nSWITCH BACK\n");

				steady1 = 0;
				rising = 1;
			}

			bgc_printf(BV_DIAG, "spinyears = %d rising = %d steady2 = %d\n",spinyears,
				rising,steady2);
			bgc_printf(BV_DIAG, "metcycle = %d tally1 = %lf tally2 = %lf pdif = %lf\n\n",
				metcycle,tally1,tally2,t1);

			metcycle = 0;
		}
		else
		{
			bgc_printf(BV_DIAG, "spinyears = %d rising = %d steady1 = %d\n",spinyears,
				rising,steady1);
			bgc_printf(BV_DIAG, "metcycle = %d tally1 = %lf tally2 = %lf pdif = %lf\n",
				metcycle,tally1,tally2,t1);

			metcycle++;
		}
	}

	/* end of do block, test for steady state */
	} while (mode == MODE_SPINUP && (!(steady1 && steady2) && (spinyears < ctrl.maxspinyears ||
		metcycle != 0)) );

	/* mode == MODE_SPINUP only */
	if (mode == MODE_SPINUP)
	{
		/* save some information on the end status of spinup */
		tally1b /= (double)nblock * 365.0;
		tally2b /= (double)nblock * 365.0;
		bgcout->spinup_resid_trend = (tally2b-tally1b)/(double)nblock;
		bgcout->spinup_years = spinyears;
	}
	
	/* RESTART OUTPUT HANDLING */
	/* if write_restart flag is set, copy data to the output restart struct */
	/* Removed 'write_restart' restriction to ensure that restart data are */
	/* available for spin and go operation.  WMJ 3/16/2005 */
	if (ok)
	{
		if (restart_output(&ctrl, &ws, &cs, &ns, &epv, metyr, 
			&(bgcout->restart_output)))
		{
			bgc_printf(BV_ERROR, "Error in call to restart_output() from bgc()\n");
			ok=0;
		}
		
		bgc_printf(BV_DIAG, "%d\t%d\tdone restart output\n",simyr,yday);
	}

	/* free phenology memory */
	if (ok && free_phenmem(&phenarr))
	{
		bgc_printf(BV_ERROR, "Error in free_phenmem() from bgc()\n");
		ok=0;
	}

	bgc_printf(BV_DIAG, "%d\t%d\tdone free phenmem\n",simyr,yday);
	
	/* free memory for local output arrays */
	
	if (dayout) free(dayarr);
	if (ctrl.domonavg) free(monavgarr);
	if (ctrl.doannavg) free(annavgarr);
	if (ctrl.doannual) free(annarr);
	free(output_map);
	
	/* print timing info if error */
	if (!ok)
	{
		bgc_printf(BV_ERROR, "ERROR at year %d\n",simyr-1);
		bgc_printf(BV_ERROR, "ERROR at yday %d\n",yday-1);
	}
	
	/* return error status */	
	return (!ok);
}
