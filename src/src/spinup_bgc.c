/*
spinup_bgc.c
Core BGC model logic, modified for spinup runs.  Instead of running for
a fixed number of years, as in a normal simulation, a spinup run proceeds
until steady-state is reached for the soil carbon, or until a specified
maximum number of years is reached.

Includes in-line output handling routines that write to daily and annual
output files. 

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.2
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2016, D. Hidy [dori.hidy@gmail.com]
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
#include "pointbgc_struct.h"   
  
#include "bgc_struct.h"     
#include "bgc_constants.h"
#include "bgc_func.h"
#include "bgc_io.h"
#include "pointbgc_func.h" 
/* #define DEBUG  set this to see function roll-call on-screen */
/* #define DEBUG_SPINUP set this to see the spinup details on-screen */

int spinup_bgc(bgcin_struct* bgcin, bgcout_struct* bgcout)
{
	
	/* variable declarations */
	int ok=1;

	/* iofiles and program control variables */
	control_struct     ctrl;

	/* meteorological variables */
	metarr_struct			metarr;
	metvar_struct			metv;
	co2control_struct		co2;
	ndep_control_struct		ndep;
	
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

	planting_struct	    PLT;
	grazing_struct		GRZ;
	harvesting_struct	HRV;

	/* GSI variables - by Hidy 2009. */
	GSI_struct      GSI;

	/* output mapping (array of pointers to double) */
	double **output_map = 0;
	
	/* local storage for daily and annual output variables */
	float *dayarr = 0;
	float *monavgarr = 0;
	float *annavgarr = 0;
	float *annarr = 0;

	/* miscelaneous variables for program control in main */
	int simyr = 0;
	int yday  = 0;
	int metyr = 0; 
	int metday= 0; 
	int first_balance;
	int annual_alloc;
	int outv;
    int i, nmetdays;
	double tair_annavg;
	int dayout;
	
	
	/* variables used for monthly average output */
	int curmonth;
	int mondays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	int endday[12] = {30,58,89,119,150,180,211,242,272,303,333,364};
	
	/* spinup control */
	int ntimesmet, nblock;
	int steady1, steady2, rising, metcycle, spinyears;
	double t1      = 0;
	double tally1  = 0;
	double tally1b = 0;
	double tally2  = 0;
	double tally2b = 0;
	double naddfrac;
	
	/* copy the input structures into local structures */
	ws = bgcin->ws;
	cinit = bgcin->cinit;
	cs = bgcin->cs;
	ns = bgcin->ns;
	sitec = bgcin->sitec;
	epc = bgcin->epc;

	PLT = bgcin->PLT;
	GRZ = bgcin->GRZ;
	HRV = bgcin->HRV;

	/* Hidy 2012.*/
	GSI = bgcin->GSI;		/* GSI variables - by Hidy 2009*/


	/* note that the following three structures have dynamic memory elements,
	and so the notion of copying the input structure to a local structure
	value-by-value is not the same as above. In this case, the array pointers
	are being copied, so the local members use the same memory that was
	allocated in the calling function. Note also that bgc() does not modify
	the contents of these structures. */
	ctrl	= bgcin->ctrl;
	metarr	= bgcin->metarr;
	co2		= bgcin->co2;
	ndep	= bgcin->ndep;
#ifdef DEBUG
	printf("done copy input\n");
#endif

 

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (ctrl.onscreen)
	{
		if (ctrl.GSI_flag)
		{
			file_open (&GSI.GSI_file, 'w');				/* file of GSI parameters - Hidy 2009.*/
		}
	
		file_open (&bgcout->control_file, 'o');		/* file of BBGC variables to control the simulation - Hidy 2009.*/
		fprintf(bgcout->control_file.ptr, "simyr yday tsoil0 tsoil1 tsoil2 GDD vwc0 vwc1 vwc2 SMSI STDBc CTDBc sminn soilc litr_aboveground litr_belowground leafc fruitc cumNPP abgC GPP TER evapotransp\n");

	}


	/********************************************************************************************************* */
	/* Hidy 2015 - writing log file */
	fprintf(bgcout->log_file.ptr, "SPINUP RUN\n");
	fprintf(bgcout->log_file.ptr, " \n");

	fprintf(bgcout->log_file.ptr, "Critical SWC (m3/m3) and PSI (MPa) values of top soil layer \n");
	fprintf(bgcout->log_file.ptr, "saturation:                    %12.3f%12.4f\n",sitec.vwc_sat[0],sitec.psi_sat[0]);
	fprintf(bgcout->log_file.ptr, "field capacity:                %12.3f%12.4f\n",sitec.vwc_fc[0], sitec.psi_fc[0]);
	fprintf(bgcout->log_file.ptr, "wilting point:                 %12.3f%12.4f\n",sitec.vwc_wp[0], sitec.psi_wp[0]);
	fprintf(bgcout->log_file.ptr, "hygroscopic water:             %12.3f%12.4f\n",sitec.vwc_hw[0], sitec.psi_hw);
	fprintf(bgcout->log_file.ptr, "bulk density:                  %12.3f\n",sitec.BD[0]);
	fprintf(bgcout->log_file.ptr, "Clapp-Hornberger b parameter:  %12.3f\n",sitec.soil_b[0]);
	fprintf(bgcout->log_file.ptr, " \n");

	fprintf(bgcout->log_file.ptr, "Soil calculation methods\n");
	if (epc.SHCM_flag == 0)
		fprintf(bgcout->log_file.ptr, "hydrology   - MuSo\n");
	else
		fprintf(bgcout->log_file.ptr, "hydrology   - DSSAT\n");

	if (epc.STCM_flag == 0)
		fprintf(bgcout->log_file.ptr, "temperature - MuSo\n");
	else
		fprintf(bgcout->log_file.ptr, "temperature - DSSAT\n");
	fprintf(bgcout->log_file.ptr, " \n");


	fprintf(bgcout->log_file.ptr, "Data sources\n");
	if (epc.phenology_flag == 0) 
	{
		if (ctrl.varSGS_flag == 0) 
			fprintf(bgcout->log_file.ptr, "SGS data - constant\n");
		else
		{
			fprintf(bgcout->log_file.ptr, "SGS data - annual varying (in transient phase)\n");
			printf("INFORMATION: reading onday_transient.txt: annual varying SGS data in transient phase\n");
		}

		if (ctrl.varEGS_flag == 0) 
			fprintf(bgcout->log_file.ptr, "EGS data - constant\n");
		else
		{
			fprintf(bgcout->log_file.ptr, "EGS data - annual varying (in transient phase)\n");
			printf("INFORMATION: reading offday_transient.txt: annual varying EGS data in transient phase\n");
		}
	}
	else
	{
		if (ctrl.GSI_flag == 0) 
		{
			fprintf(bgcout->log_file.ptr, "SGS data - original method\n");
			fprintf(bgcout->log_file.ptr, "EGS data - original method\n");
		}
		else
		{
			fprintf(bgcout->log_file.ptr, "SGS data - GSI method\n");
			fprintf(bgcout->log_file.ptr, "EGS data - GSI method\n");
		}
	}

	if (ctrl.varWPM_flag == 0) 
		fprintf(bgcout->log_file.ptr, "WPM data - constant \n");
	else
	{
		fprintf(bgcout->log_file.ptr, "WPM data- annual varying (in transient phase)\n");
		printf("INFORMATION: reading mortality_transient.txt: annual varying WPM data in transient phase\n");
	}

	if (ctrl.varMSC_flag == 0) 
		fprintf(bgcout->log_file.ptr, "MSC data - constant\n");
	else
	{
		fprintf(bgcout->log_file.ptr, "MSC data - annual varying (in transient phase)\n");
		printf("INFORMATION: reading conductance_transient.txt: annual varying MSC data in transient phase\n");
	}

	if (ctrl.FRZ_flag || ctrl.GRZ_flag || ctrl.MOW_flag || ctrl.THN_flag || ctrl.PLG_flag || ctrl.PLT_flag || ctrl.IRG_flag || ctrl.HRV_flag)
		fprintf(bgcout->log_file.ptr, "management  - YES (in transient phase)\n");
	else
		fprintf(bgcout->log_file.ptr, "management  - NO\n");

	if (ctrl.GWD_flag == 0) 
		fprintf(bgcout->log_file.ptr, "groundwater - NO\n");
	else
	{
		fprintf(bgcout->log_file.ptr, "groundwater - YES\n");
		printf("INFORMATION: reading groundwater_spinup.txt: annual varying GWD data\n");
	}

	fprintf(bgcout->log_file.ptr, " \n");
	/********************************************************************************************************* */

	/* local variable that signals the need for daily output array */
	dayout = (ctrl.dodaily || ctrl.domonavg || ctrl.doannavg);
	
	/* allocate memory for local output arrays */
	if (ok && dayout) 
	{
		dayarr = (float*) malloc(ctrl.ndayout * sizeof(float));
		if (!dayarr)
		{
			printf("Error allocating for local daily output array in bgc()\n");
			ok=0;
		}
	}
	if (ok && ctrl.domonavg) 
	{
		monavgarr = (float*) malloc(ctrl.ndayout * sizeof(float));
		if (!monavgarr)
		{
			printf("Error allocating for monthly average output array in bgc()\n");
			ok=0;
		}
	}
	if (ok && ctrl.doannavg) 
	{
		annavgarr = (float*) malloc(ctrl.ndayout * sizeof(float));
		if (!annavgarr)
		{
			printf("Error allocating for annual average output array in bgc()\n");
			ok=0;
		}
	}
	if (ok && ctrl.doannual)
	{
		annarr = (float*) malloc(ctrl.nannout * sizeof(float));
		if (!annarr)
		{
			printf("Error allocating for local annual output array in bgc()\n");
			ok=0;
		}
	}
	/* allocate space for the output map pointers */
	if (ok) 
	{
		output_map = (double**) malloc(NMAP * sizeof(double*));
		if (!output_map)
		{
			printf("Error allocating for output map in output_map_init()\n");
			ok=0;
		}
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
	
	/* initialize the output mapping array*/ 
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
		
     

	/* Hidy 2009. - calculate GSI to deterime onday and offday 	*/
	if (ctrl.GSI_flag)
	{
		if (ok && GSI_calculation(&metarr, &ctrl, &sitec, &epc, &GSI, &phenarr))
		{
			printf("Error in call to GSI_calculation(), from bgc()\n");
			ok=0;
		}
	}



#ifdef DEBUG
	printf("done GSI_calculation\n");
#endif

	/* Hidy 2012. - calculate conductance limitation factors 	*/	
	if (ok && conduct_limit_factors(bgcout->log_file, &ctrl, &sitec, &epc, &epv))
	{
		printf("Error in call to conduct_limit_factors(), from bgc()\n");
		ok=0;
	}

#ifdef DEBUG
	printf("done conduct_limit_factors\n");
#endif

	/********************************************************************************************************* */
	fprintf(bgcout->log_file.ptr, "---------------- \n");
	fprintf(bgcout->log_file.ptr, "spinup phase\n");
	fprintf(bgcout->log_file.ptr, "---------------- \n");
	/********************************************************************************************************* */

	/* determine phenological signals */		
	if (ok && prephenology(bgcout->log_file, &ctrl, &epc, &sitec, &metarr, &phenarr))
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
	tair_annavg = 0.0;
	nmetdays = ctrl.metyears * NDAY_OF_YEAR;
 	for (i=0 ; i<nmetdays ; i++)
	{
		tair_annavg += metarr.tavg[i];
	}
	tair_annavg /= (double)nmetdays;


	/* if this simulation is using a restart file for its initial
	conditions, then copy restart info into structures */
	if (ok && ctrl.read_restart)
	{
		if (ok && restart_input(&ctrl, &epc, &ws, &cs, &ns, &epv, &metyr,
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
		if (ok && firstday(&sitec, &epc, &cinit, &epv, &phenarr, &cs, &ns, &metv))
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
			
			naddfrac = (1.0 - ((double)simyr/(double)nblock));
		
			
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
			if (spinyears == 0 && ctrl.onscreen) 
			{
				printf("-----------------------------------------\n");
				printf("Start of spinup simulation. Please, wait!\n");
			}
			if (ctrl.onscreen) printf("*");
			
			/* set the max lai, maturity and flowering variables, for annual diagnostic output */
			epv.ytd_maxplai = 0.0;
			epv.maturity    = 0;
			epv.flowering   = 0;

			/* atmospheric concentration of CO2 (ppm) */
			metv.co2 = co2.co2ppm;
	
	
			/* begin the daily model loop */
			for (yday=0 ; ok && yday<NDAY_OF_YEAR ; yday++)
			{
#ifdef DEBUG
				printf("year %d\tyday %d\n",simyr,yday);
#endif
				/* Hidy 2010. - counters into control sturct */ 
				ctrl.simyr = simyr;
				ctrl.yday = yday;
				ctrl.spinyears = spinyears;


							
				/* Test for very low state variable values and force them
				to 0.0 to avoid rounding and floating point overflow errors */
				if (ok && precision_control(&ws, &cs, &ns))
				{
					printf("Error in call to precision_control() from bgc()\n");
					ok=0;
				} 

				/* set the day index for meteorological and phenological arrays */
				metday = metyr*NDAY_OF_YEAR + yday;

				/* zero all the daily flux variables */
				wf = zero_wf;
				cf = zero_cf;
				nf = zero_nf;

	               /* soil hydrological parameters: psi and vwc  */
 			if (ok && multilayer_hydrolparams(&sitec, &ws, &epv, &metv))
			{
				printf("Error in multilayer_hydrolparams() from bgc()\n");
				ok=0;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_hydrolparams\n",simyr,yday);
#endif
				/* daily meteorological variables from metarrays */
				if (ok && daymet(&ctrl, &metarr, &sitec, &epc, &PLT, &HRV,&ws, &epv, &metv, &tair_annavg, metday))
				{
					printf("Error in daymet() from bgc()\n");
					ok=0;
				}

#ifdef DEBUG
				printf("%d\t%d\tdone daymet\n",simyr,yday);
#endif

		

			/* soil temperature calculations */
				if (ok && multilayer_tsoil(yday, &epc, &sitec, &ws, &metv, &epv))
				{
					printf("Error in multilayer_tsoil() from bgc()\n");
					ok=0;
				}

#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_tsoil\n",simyr,yday);
#endif


				/* daily phenological variables from phenarrays */
				if (ok && dayphen(&phenarr, &phen, metday, metyr))
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
				if (ok && phenology(&ctrl, &epc, &phen, &epv, &cs, &cf, &ns, &nf))
				{
					printf("Error in phenology() from bgc()\n");
					ok=0;
				}

#ifdef DEBUG
				printf("%d\t%d\tdone phenology\n",simyr,yday);
#endif
			
			

				/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Hidy 2011 - MULTILAYER SOIL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
				/* rooting depth */

 				 if (ok && multilayer_rootdepth(&ctrl, &epc, &sitec, &phen, &PLT, &HRV, &epv, &ns))
				 {
					printf("Error in multilayer_rootdepth() from bgc()\n");
					ok=0;
				 }

#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_rootdepth\n",simyr,yday);
			
#endif
				/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
			
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
				if (ok && metv.prcp && prcp_route(&metv, epc.int_coef, epv.all_lai,	&wf))
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
				if (ok && baresoil_evap(&metv, &wf, &epv.dsr))
				{
					printf("Error in baresoil_evap() from bgc()\n");
					ok=0;
				}

#ifdef DEBUG
				printf("%d\t%d\tdone bare_soil evap\n",simyr,yday);
#endif

				/* conductance - Hidy 2011 */
				if (ok && conduct_calc(&ctrl, &metv, &epc, &sitec, &epv, simyr))
				{
					printf("Error in conduct_calc() from bgc()\n");
					ok=0;
				}
			
#ifdef DEBUG
			printf("%d\t%d\tdone conduct_calc\n",simyr,yday);
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

					/* evapo-transpiration */
					if (ok && canopy_et(&epc, &metv, &epv, &wf))
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
				if (ok && cs.leafc && phen.remdays_curgrowth && metv.dayl && ws.snoww <= GSI.snowcover_limit)
				{
					/* SUNLIT canopy fraction photosynthesis */
					/* set the input variables */
					psn_sun.c3 = epc.c3_flag;
					psn_sun.co2 = metv.co2;
					psn_sun.pa = metv.pa;
					psn_sun.t = metv.tday;
					psn_sun.lnc = 1.0 / (epv.sun_proj_sla * epc.leaf_cn);
					psn_sun.flnr = epc.flnr;
                                        psn_sun.flnp = epc.flnp;
					psn_sun.ppfd = metv.ppfd_per_plaisun;
					/* convert conductance from m/s --> umol/m2/s/Pa, and correct
					for CO2 vs. water vapor */
					psn_sun.g = epv.gl_t_wv_sun * 1e6/(1.6*R*(metv.tday+273.15));
					psn_sun.dlmr = epv.dlmr_area_sun;
					
					if (ok && photosynthesis(&epc, &metv, &psn_sun))
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
					cf.psnsun_to_cpool = (epv.assim_sun + epv.dlmr_area_sun) * epv.plaisun * metv.dayl * 12.011e-9; 
						

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
					if (ok && photosynthesis(&epc, &metv, &psn_shade))
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
					cf.psnshade_to_cpool = (epv.assim_shade + epv.dlmr_area_shade) * epv.plaishade * metv.dayl * 12.011e-9; 
						

				} /* end of photosynthesis calculations */
				else
				{
					epv.assim_sun = epv.assim_shade = 0.0;
					/*new 29.5.02 */
					psn_sun.Ci=psn_shade.Ci=0.0;
					psn_sun.lnc=psn_shade.lnc=0.0;
					psn_sun.g=psn_shade.g=0.0;
					psn_sun.dlmr=psn_shade.dlmr=0.0;
					psn_sun.Vmax=psn_shade.Vmax=0.0;
					psn_sun.Jmax=psn_shade.Jmax=0.0;
					psn_sun.A=psn_shade.A=0.0;
					epv.m_ppfd_sun=epv.m_ppfd_shade=0.0;
					/* end new 28.5.02*/
				}
				
			/* Hidy 2010 - calculation water stress days */
 			if (ok && waterstress_days(yday, &phen, &epv, &epc))
			{
				printf("Error in waterstress_days() from bgc()\n");
				ok=0;
			}
#ifdef DEBUG
			printf("%d\t%d\tdone waterstress_days\n",simyr,yday);
#endif	
			
			/* !!!!!!!!!!!!!!!!!!!!!!  TRANSPIRATION AND SOILPSI IN MULTILAYER SOIL!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */		        
			/* Hidy 2010 - calculate the part-transpiration from total transpiration */
			if (ok && multilayer_transpiration(&ctrl, &sitec, &epv, &ws, &wf))
			{
				printf("Error in multilayer_transpiration() from bgc()\n");
				ok=0;
			}
#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_transpiration\n",simyr,yday);
#endif		

            /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
			
			/* nitrogen deposition and fixation */
			nf.ndep_to_sminn = ndep.ndep / NDAY_OF_YEAR;
			nf.nfix_to_sminn = epc.nfix / NDAY_OF_YEAR;

	

			/* daily litter and soil decomp and nitrogen fluxes */
			if (ok && decomp(&metv,&epc,&epv,&sitec,&cs,&cf,&ns,&nf,&nt))
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
			if ((!steady1 && rising && metcycle == 0))
			{
				if (ok && spinup_daily_allocation(&epc,&sitec,&cf,&cs,&nf,&ns,&epv,&nt,naddfrac))
				{
					printf("Error in daily_allocation() from bgc.c\n");
					ok=0;
				}

			}
			else
			{
				if (ok && daily_allocation(&epc,&sitec,&cf,&cs,&nf,&ns,&epv,&nt))
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

	
			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  MULTILAYER SOIL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */	
			/* Hidy 2013 - multilayer soil hydrology: percolation calculation based on PRCP, RUNOFF, EVAP, TRANS */
     		if (ok && multilayer_hydrolprocess(&ctrl, &sitec, &epc, &epv, &ws, &wf))
			{ 
				printf("Error in multilayer_hydrolprocess() from bgc()\n");
				ok=0;
			}

#ifdef DEBUG
		printf("%d\t%d\tdone multilayer_hydrolprocess\n",simyr,yday);
#endif

			/* ground water calculation */
			if (ok && groundwater(&ctrl, &sitec, &epv, &ws, &wf))
			{
				printf("Error in groundwater() from bgc()\n");
				ok=0;
			}
		

#ifdef DEBUG
			printf("%d\t%d\tdone groundwater\n",simyr,yday);
#endif	

			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
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
			if (ok && daily_carbon_state_update(&cf, &cs, annual_alloc, epc.woody, epc.evergreen))
			{
				printf("Error in daily_carbon_state_update() from bgc()\n");
				ok=0;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone carbon state update\n",simyr,yday);
#endif
	
			/* daily update of nitrogen state variables */
			if (ok && daily_nitrogen_state_update(&epc, &nf, &ns, annual_alloc, epc.woody, epc.evergreen))
			{
				printf("Error in daily_nitrogen_state_update() from bgc()\n");
				ok=0;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone nitrogen state update\n",simyr,yday);
#endif


            /* calculate daily mortality fluxes and update state variables */
			/* this is done last, with a special state update procedure, to
			insure that pools don't go negative due to mortality fluxes
			conflicting with other proportional fluxes */
			if (ok && mortality(&ctrl, &epc, &cs, &cf, &ns, &nf, simyr))
			{
				printf("Error in mortality() from bgc()\n");
				ok=0;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone mortality\n",simyr,yday);
#endif


			/* Hidy 2013 - calculate daily senescence mortality fluxes and update state variables */
			if (ok && senescence(&epc, &GRZ, &cs, &cf, &ns, &nf, &epv))
			{
				printf("Error in senescence() from bgc()\n");
				ok=0;
			}
					
#ifdef DEBUG
		printf("%d\t%d\tdone senescence\n",simyr,yday);
#endif



			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  MULTILAYER SOIL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */	
			/* Hidy 2011 - calculate the change of soil mineralized N in multilayer soil.  
			This is a special state variable update routine, done after the other fluxes and states are reconciled 
			in order to avoid negative sminn (nleaching is included) */
			if (ok && multilayer_sminn(&epc, &sitec, &epv, &ns, &nf, &ws, &wf))
			{
				printf("Error in multilayer_sminn() from bgc()\n");
				ok=0;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_sminn\n",simyr,yday);
#endif
		

			/* Hidy 2013 - test again for very low state variable values and force them
			to 0.0 to avoid rounding and floating point overflow errors */
			if (ok && precision_control(&ws, &cs, &ns))
			{
				printf("Error in call to precision_control() from bgc()\n");
				ok=0;
			} 
			
#ifdef DEBUG
			printf("%d\t%d\tdone precision_control\n",simyr,yday);
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
			

			/* test for nitrogen balance -  modified by Hidy 2008 */
			if (ok && check_nitrogen_balance(&ns, first_balance))
			{
				printf("Error in check_nitrogen_balance() from bgc()\n");
				printf("%d\n",metday);
				ok=0;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone nitrogen balance\n",simyr,yday);
#endif

			
            /* calculate summary variables */
			if (ok && cnw_summary(yday, &cs, &cf, &ns, &nf, &wf, &epv, &summary))
			{
				printf("Error in cnw_summary() from bgc()\n");
				ok=0;
			} 
#ifdef DEBUG
			printf("%d\t%d\tdone carbon summary\n",simyr,yday);
#endif

	

				/* INTERNAL VARIALBE CONTROL - Hidy 2013 */
			if (ctrl.onscreen && (ctrl.spinyears == 0 || ctrl.spinyears == 1 ||  ctrl.spinyears == 100 ||
			    ctrl.spinyears == 600 || ctrl.spinyears == 1000 || ctrl.spinyears == 2000))
			{
					fprintf(bgcout->control_file.ptr, "%i %i %14.8f %14.8f %14.8f %14.8f %14.8f %14.8f %14.8f %14.8f %14.8f %14.8f %14.8f %14.8f %14.8f %14.8f %14.8f %14.8f %14.8f %14.8f %14.8f %14.8f %14.8f\n",
		            ctrl.simyr,yday,metv.tsoil[0], metv.tsoil[1], metv.tsoil[2], metv.GDD,
					epv.vwc[0], epv.vwc[1], epv.vwc[2],
					epv.m_soilstress, 
					cs.STDBc, cs.CTDBc,
					(ns.sminn[0]+ns.sminn[1]+ns.sminn[2]+ns.sminn[3]+ns.sminn[4]+ns.sminn[5]+ns.sminn[6]), 
				    summary.soilc, cs.litr_aboveground, cs.litr_belowground, 
					cs.leafc, cs.fruitc, summary.cum_npp_ann, summary.abgc, 
					summary.daily_gpp, summary.daily_tr, wf.evapotransp); 

			}

	

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
							annavgarr[outv] /= NDAY_OF_YEAR;
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
			tally1 /= (double)nblock * NDAY_OF_YEAR;
			tally2 /= (double)nblock * NDAY_OF_YEAR;
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
			tally1 /= (double)nblock * NDAY_OF_YEAR;
			tally2 /= (double)nblock * NDAY_OF_YEAR;
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
	

	/********************************************************************************************************* */
	/* Hidy 2015 - writing log file */
	fprintf(bgcout->log_file.ptr, "Some important annual outputs\n");
	fprintf(bgcout->log_file.ptr, "Mean annual GPP (gC/m2/year):                           %12.1f\n",summary.cum_gpp/ctrl.spinyears*1000);
	fprintf(bgcout->log_file.ptr, "Mean annual NEE (gC/m2/year):                           %12.1f\n",summary.cum_nee/ctrl.spinyears*1000);
	fprintf(bgcout->log_file.ptr, "Maximum projected LAI (m2/m2):                          %12.2f\n",epv.ytd_maxplai);
	fprintf(bgcout->log_file.ptr, "Recalcitrant SOM carbon content (kgC/m2):               %12.1f\n",cs.soil4c);
	fprintf(bgcout->log_file.ptr, "Total soil carbon content (kgC/m2):                     %12.1f\n",summary.soilc);
	fprintf(bgcout->log_file.ptr, "Total soil mineralized nitrogen content (gN/m2):        %12.2f\n",summary.sminn*1000);
	fprintf(bgcout->log_file.ptr, "Mean annual SWC in rootzone (m3/m3):                    %12.2f\n",summary.vwc_annavg/(ctrl.spinyears*NDAY_OF_YEAR));
	fprintf(bgcout->log_file.ptr, " \n");
 	fprintf(bgcout->log_file.ptr, "Mean annual N-plus (spinup_daily_allocation) (gN/year): %12.2f\n",summary.cum_nplus/ctrl.spinyears*1000);
    fprintf(bgcout->log_file.ptr, " \n");

	fprintf(bgcout->log_file.ptr,"spinyears = %d \n",spinyears);
	fprintf(bgcout->log_file.ptr, " \n");
	
	/********************************************************************************************************* */

	/* save some information on the end status of spinup */
	tally1b /= (double)nblock * NDAY_OF_YEAR;
	tally2b /= (double)nblock * NDAY_OF_YEAR;
	bgcout->spinup_resid_trend = (tally2b-tally1b)/(double)nblock;
	bgcout->spinup_years = spinyears;
	
	if (ctrl.onscreen) printf("\n");
	if (ctrl.onscreen) printf("SPINUP: residual trend   = %.6lf\n",bgcout->spinup_resid_trend);
 	if (ctrl.onscreen) printf("SPINUP: number of years  = %d\n",bgcout->spinup_years);

	/********************************************************************************************************* */
	 /* !!!!!!!!!!! close control file - Hidy 2009.!!!!!!!!!!!!*/

	/* free phenology memory */
	if (ok && free_phenmem(&phenarr))
	{
		printf("Error in free_phenmem() from bgc()\n");
		ok=0;
	}
	
	if (ctrl.onscreen)
	{
		if (ctrl.GSI_flag)
		{
			fclose (GSI.GSI_file.ptr);
		}
		fclose (bgcout->control_file.ptr);
	}
   
	/*----------------------------------------------------------*/
	/* TRANSIENT RUN between spinup and normal run - Hidy 2014 */
  	if (ok && (co2.varco2 || ndep.varndep))
	{
	
		bgcin->ws = ws;
		bgcin->cs = cs;
		bgcin->ns = ns;

		if (ctrl.onscreen) 
		{
			printf("-----------------------------------------\n");
			printf("Start of transient run.\n");
		}

		if (transient_bgc(bgcin, bgcout))
		{
			printf("Error in call to bgc()\n");
			exit(1);
		}

		ws = bgcin->ws;
		cs = bgcin->cs;
		ns = bgcin->ns;
	}



	/*----------------------------------------------------------*/

	/* RESTART OUTPUT HANDLING */
	/* if write_restart flag is set, copy data to the output restart struct */
	if (ok && ctrl.write_restart)
	{
		if (restart_output(&ws, &cs, &ns, &epv, metyr, &(bgcout->restart_output)))
		{
			printf("Error in call to restart_output() from bgc()\n");
			ok=0;
		}
		
#ifdef DEBUG
		printf("%d\t%d\tdone restart output\n",simyr,yday);
#endif
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
