/*
transient_bgc.c
Core BGC model logic

Includes in-line output handling routines get the bgcin struct of spinup run
with contans CO2 and Ndep data without management and calculate ws, cs, ns
stuctures with increasing CO2 and Ndep value (with management) in order to
terminate the disruption between normal and spiup run
This run has no output and it is optional 
(spinup_ini: CO2_CONTROL block varCO2 flag=1)

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.4
Copyright 2017, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <time.h>

#include "ini.h"    
#include "bgc_struct.h"     
#include "bgc_constants.h"
#include "bgc_func.h"
#include "pointbgc_struct.h"   /* data structures for point driver */
#include "bgc_io.h"
#include "pointbgc_func.h"
//#define DEBUG



int transient_bgc(bgcin_struct* bgcin, bgcout_struct* bgcout)
{
	
	/* variable declarations */
	int ok=1;
    

	/* iofiles and program control variables */
	control_struct     ctrl;

	/* meteorological variables */
	metarr_struct       metarr;
	metvar_struct       metv;
	co2control_struct   co2;
	ndep_control_struct ndep;

	
	/* state and flux variables for water, carbon, and nitrogen */
	wstate_struct      ws;
	wflux_struct       wf, zero_wf;
	cstate_struct      cs;
	cflux_struct       cf, zero_cf;
	nstate_struct      ns;
	nflux_struct       nf, zero_nf;
	cinit_struct       cinit;

	/* primary ecophysiological variables */
	epvar_struct       epv;

	/* planting variables - by Hidy 2008*/
	planting_struct       PLT;

	/* thinning variables - by Hidy 2008*/
	thinning_struct     THN;

	/* mowing variables - by Hidy 2008*/
	mowing_struct       MOW;

	/* grazing variables - by Hidy 2009*/
	grazing_struct       GRZ;

	/* harvesting variables - by Hidy 2008*/
	harvesting_struct       HRV;

	/* ploughing variables - by Hidy 2012*/
	ploughing_struct      PLG;

	/* fertilizing variables - by Hidy 2008*/
	fertilizing_struct       FRZ;

	/* irrigation variables - by Hidy 2015*/
	irrigation_struct       IRG;

	/* GSI variables - by Hidy 2009. */
	GSI_struct      GSI;

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
	int dayout;
	double daily_ndep; 
	double tair_annavg;
	double nmetdays;
	int i;

	

	/* copy the input structures into local structures */
	ws = bgcin->ws; 
	cs = bgcin->cs;
	ns = bgcin->ns;
	cinit = bgcin->cinit;
	sitec = bgcin->sitec;
	epc = bgcin->epc;
	PLT = bgcin->PLT; 		/* planting variables - Hidy 2008.*/
	THN = bgcin->THN; 		/* thinning variables - Hidy 2008.*/
	MOW = bgcin->MOW; 		/* mowing variables - Hidy 2008.*/
	GRZ = bgcin->GRZ; 		/* grazing variables - Hidy 2008.*/
	HRV = bgcin->HRV;		/* harvesting variables - Hidy 2008.*/
	PLG = bgcin->PLG;		/* harvesting variables - Hidy 2008.*/
	FRZ = bgcin->FRZ;		/* fertilizing variables - Hidy 2008.*/
	IRG = bgcin->IRG;
	GSI = bgcin->GSI;		/* GSI variables - Hidy 2009. */



	/* note that the following three structures have dynamic memory elements,
	and so the notion of copying the input structure to a local structure
	value-by-value is not the same as above. In this case, the array pointers
	are being copied, so the local members use the same memory that was
	allocated in the calling function. Note also that bgc() does not modify
	the contents of these structures. */
	ctrl = bgcin->ctrl;
	metarr = bgcin->metarr;
	co2 = bgcin->co2;
	ndep = bgcin->ndep;
	

#ifdef DEBUG
	printf("done copy input\n");
#endif


	/* Hidy 2015 - writing log file */
	fprintf(bgcout->log_file.ptr, "---------------- \n");
	fprintf(bgcout->log_file.ptr, "transient phase\n");
	fprintf(bgcout->log_file.ptr, "---------------- \n");

	/* sign transient run */
	ctrl.spinup = 2;

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


    /* Hidy 2009. - calculate GSI to deterime onday and offday 	*/	
	if (ctrl.GSI_flag)
	{
		
		if (ok && GSI_calculation(&metarr, &ctrl, &sitec, &epc, &GSI, &phenarr))
		{
			printf("Error in call to GSI_calculation(), from bgc()\n");
			ok=0;
		}

		#ifdef DEBUG
			printf("done GSI_calculation\n");
		#endif
	}


	/* Hidy 2012. - calculate conductance limitation factors 	*/	
	if (ok && conduct_limit_factors(bgcout->log_file, &ctrl, &sitec, &epc, &epv))
	{
		printf("Error in call to conduct_limit_factors(), from bgc()\n");
		ok=0;
	}

#ifdef DEBUG
	printf("done conduct_limit_factors\n");
#endif

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
	conjunction with soil temperature testing done with Mike White.  */
	tair_annavg = 0.0;
	nmetdays = ctrl.metyears * NDAY_OF_YEAR;
	for (i=0 ; i<nmetdays ; i++)
	{
		tair_annavg += metarr.tavg[i];
	}
	tair_annavg /= (double)nmetdays;


		/* set the initial rates of litterfall and live wood turnover */
	if (epc.evergreen)
	{
		/* leaf and fineroot litterfall rates */
		epv.day_leafc_litfall_increment = cinit.max_leafc * epc.leaf_turnover / NDAY_OF_YEAR;
		epv.day_frootc_litfall_increment = cinit.max_leafc * epc.froot_turnover / NDAY_OF_YEAR;
		/* fruit simulation - Hidy 2013. */
		epv.day_fruitc_litfall_increment = cinit.max_leafc * epc.fruit_turnover / NDAY_OF_YEAR;
		/* softstem simulation - Hidy 2013. */
		epv.day_softstemc_litfall_increment = cinit.max_leafc * epc.softstem_turnover / NDAY_OF_YEAR;
	}
	else
	{
		/* deciduous: reset the litterfall rates to 0.0 for the start of the
		next litterfall season */
		epv.day_leafc_litfall_increment = 0.0;
		epv.day_frootc_litfall_increment = 0.0;
		/* fruit simulation - Hidy 2013. */
		epv.day_fruitc_litfall_increment = 0.0;
		/* softstem simulation - Hidy 2013. */
		epv.day_softstemc_litfall_increment = 0.0;
	}
	/* all types can use annmax leafc and frootc */
	epv.annmax_leafc = 0.0;
	epv.annmax_frootc = 0.0;
	/* fruit simulation - Hidy 2013. */
	epv.annmax_fruitc = 0.0;
	epv.annmax_softstemc = 0.0;
	epv.annmax_livestemc = 0.0;
	epv.annmax_livecrootc = 0.0;

	epv.day_livestemc_turnover_increment = cs.livestemc * epc.livewood_turnover / NDAY_OF_YEAR;
	epv.day_livecrootc_turnover_increment = cs.livecrootc * epc.livewood_turnover / NDAY_OF_YEAR;

	/* initial value for metyr */
	metyr = 0;

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
	
	/* !!!!!!!!!!!!!!!!!!!!!!! */
	/* BEGIN OF THE ANNUAL LOOP */

	for (simyr=0 ; ok && simyr<ctrl.simyears ; simyr++)
	{

		/* output to screen to indicate start of simulation year */
		if (ctrl.onscreen) printf("Year: %6d\n",ctrl.simstartyear+simyr);

		/* set the max lai, maturity and flowering variables, for annual diagnostic output */
		epv.ytd_maxplai = 0.0;
		epv.maturity    = 0;
		epv.flowering   = 0;
	

		/* atmospheric CO2 handling */
		if (!(co2.varco2))
		{
			/*constant Ndep */
			metv.co2  = co2.co2ppm;
		}
		else
		{	
			/* Ndep from file */
			metv.co2 = co2.co2ppm_array[simyr];
		}

		 /* atmospheric Ndep handling */
		if (!(ndep.varndep))
		{
			/*constant Ndep */
			daily_ndep = ndep.ndep / NDAY_OF_YEAR;
		}
		else
		{	
			/* Ndep from file */
			daily_ndep = ndep.ndep_array[simyr] / NDAY_OF_YEAR;
		}
		
	
		
		/* !!!!!!!!!!!!!!!!!!!!!!! */
		/* BEGIN OF THE DAILY LOOP */
		for (yday=0 ; ok && yday<NDAY_OF_YEAR ; yday++)
		{

#ifdef DEBUG
			printf("year %d\tyday %d\n",simyr,yday);
#endif

	
		
			/* Hidy 2010. - counters into control sturct */ 
			ctrl.simyr = simyr;
			ctrl.yday = yday;
			ctrl.spinyears = 0;
				
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

			/* MANAGEMENT DAYS - Hidy 2013. */
			if (ok && management(&ctrl, &FRZ, &GRZ, &HRV, &MOW, &PLT, &PLG, &THN, &IRG))
			{
				printf("Error in management days() from bgc()\n");
				ok=0;
			}


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


			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Hidy 2011 - MULTILAYER SOIL!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

			/* rooting depth */
 			 if (ok && multilayer_rootdepth(&ctrl, &epc, &sitec, &phen,  &PLT, &HRV, &epv, &ns))
			 {
				printf("Error in multilayer_rootdepth() from bgc()\n");
				ok=0;
			 }
#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_rootdepth\n",simyr,yday);
#endif


			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
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
			       /* original */
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

			/* !!!!!!!!!!!!!!!!!!!!!! TRANSPIRATION AND SOILPSI IN MULTILAYER SOIL!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
		
			/* Hidy 2010 - calculate the part-transpiration from total transpiration */
			if (ok && multilayer_transpiration(&ctrl, &sitec, &epv, &ws, &wf))
			{
				printf("Error in multilayer_transpiration() from bgc()\n");
				ok=0;
			}
#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_transpiration\n",simyr,yday);
#endif
			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
		
			/* nitrogen deposition and fixation */
			nf.ndep_to_sminn = daily_ndep;
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
			if (ok && daily_allocation(&epc,&sitec,&cf,&cs,&nf,&ns,&epv,&nt))
			{
				printf("Error in daily_allocation() from bgc.c\n");
				ok=0;
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

         /* ground water calculation */
			if (ok && groundwater(&ctrl, &sitec, &epv, &ws, &wf))
			{
				printf("Error in groundwater() from bgc()\n");
				ok=0;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone groundwater\n",simyr,yday);
#endif	
		
			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  MULTILAYER SOIL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
			/* Hidy 2013 - multilayer soil hydrology: percolation calculation based on PRCP, RUNOFF, EVAP, TRANS */
			
	               /* IRRIGATION - Hidy 2015. */
			if (ok && irrigation(&ctrl, &IRG, &ws, &wf))
			{
				printf("Error in irrigation() from bgc()\n");
				ok=0;
			}

			if (ok && multilayer_hydrolprocess(&ctrl, &sitec, &epc, &epv, &ws, &wf))
			{
				printf("Error in multilayer_hydrolprocess() from bgc()\n");
				ok=0;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_hydrolprocess\n",simyr,yday);
#endif	
		
               
			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

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



			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  MANAGEMENT SUBMODULES !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

			
			/* PLANTING - Hidy 2009. */
			if (ok && planting(&ctrl, &epc, &PLT, &cf, &nf, &cs, &ns))
			{
				printf("Error in planting() from bgc()\n");
				ok=0;
			}

		   	/* THINNIG - Hidy 2012. */
			if (ok && thinning(&ctrl, &epc, &THN, &cf, &nf, &wf, &cs, &ns, &ws))
			{
				printf("Error in thinning() from bgc()\n");
				ok=0;
			}

			/* MOWING - Hidy 2008. */
			if (ok && mowing(&ctrl, &epc, &MOW, &cf, &nf, &wf, &cs, &ns, &ws))
			{
				printf("Error in mowing() from bgc()\n");
				ok=0;
			}

			 /* GRAZING - Hidy 2009. */
			if (ok && grazing(&ctrl, &epc, &GRZ, &cf, &nf, &wf, &cs, &ns, &ws))
			{
				printf("Error in grazing() from bgc()\n");
				ok=0;
			}
	
		   	/* HARVESTING - Hidy 2012. */
			if (ok && harvesting(&ctrl, &epc, &HRV, &cf, &nf, &wf, &cs, &ns, &ws))
			{
				printf("Error in harvesting() from bgc()\n");
				ok=0;
			}
 
			/* PLOUGHING - Hidy 2012. */
			if (ok && ploughing(&ctrl, &epc, &sitec, &metv,  &epv, &PLG, &cf, &nf, &wf, &cs, &ns, &ws))
			{
				printf("Error in ploughing() from bgc()\n");
				ok=0;
			}
		 
			/* FERTILIZING -  Hidy 2008 */
	    	if (ok && fertilizing(&ctrl, &FRZ, &cs, &ns, &cf, &nf))
			{
				printf("Error in fertilizing() from bgc()\n");
				ok=0;
			}	

			cs.CTDBc =  cs.litr1c_strg_HRV + cs.litr1c_strg_MOW + cs.litr1c_strg_THN + 
			cs.litr2c_strg_HRV + cs.litr2c_strg_MOW + cs.litr2c_strg_THN + 
			cs.litr3c_strg_HRV + cs.litr3c_strg_MOW + cs.litr3c_strg_THN + 
			cs.litr4c_strg_HRV + cs.litr4c_strg_MOW + cs.litr4c_strg_THN;
			
		

			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

	

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

			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  MULTILAYER SOIL !!!!!!!!!!!!!!!!!!!!!!!!! */
			/* Hidy 2011 - calculate the change of soil mineralized N in multilayer soil.  
                        This is a special state variable update routine, done after the other fluxes and states are
                        reconciled in order to avoid negative sminn (nleaching is included) */

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


			/* test for water balance*/
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
			if (ok  && check_carbon_balance(&cs, first_balance))
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

	          
		
	}
}
	bgcin->ws = ws;
	bgcin->cs = cs;
	bgcin->ns = ns;

	/********************************************************************************************************* */
	/* Hidy 2015 - writing log file */
	fprintf(bgcout->log_file.ptr, "Some important annual outputs\n");
	fprintf(bgcout->log_file.ptr, "Mean annual GPP (gC/m2/year):                           %12.1f\n",summary.cum_gpp/ctrl.simyears*1000);
	fprintf(bgcout->log_file.ptr, "Mean annual NEE (gC/m2/year):                           %12.1f\n",summary.cum_nee/ctrl.simyears*1000);
	fprintf(bgcout->log_file.ptr, "Maximum projected LAI (m2/m2):                          %12.2f\n",epv.ytd_maxplai);
	fprintf(bgcout->log_file.ptr, "Recalcitrant SOM carbon content (kgC/m2):               %12.1f\n",cs.soil4c);
	fprintf(bgcout->log_file.ptr, "Total soil carbon content (kgC/m2):                     %12.1f\n",summary.soilc);
	fprintf(bgcout->log_file.ptr, "Total soil mineralized nitrogen content (gN/m2):        %12.2f\n",summary.sminn*1000);
	fprintf(bgcout->log_file.ptr, "Mean annual SWC in rootzone (m3/m3):                    %12.2f\n",summary.vwc_annavg/(ctrl.simyears*NDAY_OF_YEAR));
	fprintf(bgcout->log_file.ptr, " \n");	
	/********************************************************************************************************* */



	
	/* print timing info if error */
	if (!ok)
	{
		printf("ERROR at year %d\n",simyr-1);
		printf("ERROR at yday %d\n",yday-1);
	}
	


	return (!ok);
}
