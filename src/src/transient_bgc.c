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
	int errorCode=0;
	int nyears;
    

	/* iofiles and program control variables */
	control_struct     ctrl;

	/* meteorological variables */
	metarr_struct       metarr;
	metvar_struct       metv;
	co2control_struct   co2;
	ndep_control_struct ndep;

	
	/* state and flux variables for water, carbon, and nitrogen */
	wstate_struct      ws;
	wflux_struct       wf;
	cinit_struct       cinit;
	cstate_struct      cs;
	cflux_struct       cf;
	nstate_struct      ns;
	nflux_struct       nf;
	/* primary ecophysiological variables */
	epvar_struct       epv;

	/* management variables */
	planting_struct     PLT;
	thinning_struct     THN;
	mowing_struct       MOW;
	grazing_struct      GRZ;
	harvesting_struct   HRV;
	ploughing_struct    PLG;
	fertilizing_struct  FRZ;
	irrigating_struct   IRG;
	groundwater_struct  gws;
	
	/* site physical constants */
	siteconst_struct   sitec;

	/* soil proportion variables */
	soilprop_struct   sprop;

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


	/* local storage for daily and annual output variables and output mapping (array of pointers to double)  */
	double* dayarr=0;
	double* monavgarr=0;
	double* annavgarr=0;
	double* annarr=0;
	double** output_map=0;

	

	/* miscelaneous variables for program control in main */
	int simyr = 0;
	int yday  = 0;
	int first_balance;
	int annual_alloc;
	double daily_ndep; 
	double tair_annavg;
	double nmetdays;
	int i;
	double CbalanceERR = 0;
	double NbalanceERR = 0;
	double WbalanceERR = 0;
	double CNratioERR = 0;
	
   /* variables used for monthly average output */
	int curmonth;
	int mondays[nMONTHS_OF_YEAR] = {31,28,31,30,31,30,31,31,30,31,30,31};
	int endday[nMONTHS_OF_YEAR] = {30,58,89,119,150,180,211,242,272,303,333,364};
	

	/* copy the input structures into local structures */
	ws = bgcin->ws;
	cinit = bgcin->cinit;
	cs = bgcin->cs;
	ns = bgcin->ns;

	sitec = bgcin->sitec;
	sprop = bgcin->sprop;
	epc = bgcin->epc;
	gws = bgcin->gws;

	PLT = bgcin->PLT; 		/* planting variables */
	THN = bgcin->THN; 		/* thinning variables */
	MOW = bgcin->MOW; 		/* mowing variables */
	GRZ = bgcin->GRZ; 		/* grazing variables */
	HRV = bgcin->HRV;		/* harvesting variables */
	PLG = bgcin->PLG;		/* harvesting variables */
	FRZ = bgcin->FRZ;		/* fertilizing variables */
	IRG = bgcin->IRG; 		/* irrigating variables */



	/* note that the following three structures have dynamic memory elements, and so the notion of copying the input structure to a local structure
	value-by-value is not the same as above. In this case, the array pointersare being copied, so the local members use the same memory that was
	allocated in the calling function. Note also that bgc() does not modify the contents of these structures. */
	ctrl = bgcin->ctrl;
	metarr = bgcin->metarr;
	co2 = bgcin->co2;
	ndep = bgcin->ndep;
	

#ifdef DEBUG
	printf("done copy input\n");
#endif


	/* writing log file */
	fprintf(bgcout->log_file.ptr, "---------------- \n");
	fprintf(bgcout->log_file.ptr, "transient phase\n");
	fprintf(bgcout->log_file.ptr, "---------------- \n");

	/* sign transient run */
	ctrl.spinup = 2;

	
	/* in case of natural ecosystem, nyears = number of meteorological year, in case agricultural system: nyears = number of plantings */
	if (!PLT.PLT_num)
		nyears = ctrl.simyears;
	else
		nyears = PLT.PLT_num + ctrl.simyears;
	
	/* allocate memory for local output arrays */
	if (!errorCode && (ctrl.dodaily || ctrl.domonavg || ctrl.doannavg)) 
	{
		dayarr = (double*) malloc(ctrl.ndayout * sizeof(double));
		if (!dayarr)
		{
			printf("ERROR allocating for local daily output array in bgc()\n");
			errorCode=3010;
		}
		for (i=0 ; i<ctrl.ndayout ; i++) dayarr[i] = 0.0;
	}

	if (!errorCode && ctrl.domonavg) 
	{
		monavgarr = (double*) malloc(ctrl.ndayout * sizeof(double));
		if (!monavgarr)
		{
			printf("ERROR allocating for monthly average output array in bgc()\n");
			errorCode=3020;
		}

		for (i=0 ; i<ctrl.ndayout ; i++) monavgarr[i] = 0.0;
	}
	if (!errorCode && ctrl.doannavg) 
	{
		annavgarr = (double*) malloc(ctrl.ndayout * sizeof(double));
		if (!annavgarr)
		{
			printf("ERROR allocating for annual average output array in bgc()\n");
			errorCode=3030;
		}
		for (i=0 ; i<ctrl.ndayout ; i++) annavgarr[i] = 0.0;
	}
	if (!errorCode && ctrl.doannual)
	{
		annarr = (double*) malloc(ctrl.nannout * sizeof(double));
		if (!annarr)
		{
			printf("ERROR allocating for local annual output array in bgc()\n");
			errorCode=3040;
		}
		for (i=0 ; i<ctrl.nannout ; i++) annarr[i] = 0.0;

	}
	

	/* allocate space for the output map pointers */
	if (!errorCode) 
	{
		output_map = (double**) malloc(NMAP * sizeof(double*));
		if (!output_map)
		{
			printf("ERROR allocating for output map in output_map_init()\n");
			errorCode=3050;
		}
	}
	
	/* allocate space for the onday_arr and offday_arr: first column - year, second column: day*/
	if (!errorCode)
	{
		phenarr.onday_arr  = (int**) malloc(nyears*sizeof(int*));  
        phenarr.offday_arr = (int**) malloc(nyears*sizeof(int*));  
			
		for (i = 0; i<nyears; i++)
		{
			phenarr.onday_arr[i]  = (int*) malloc(2*sizeof(int));  
			phenarr.offday_arr[i] = (int*) malloc(2*sizeof(int));  
			phenarr.onday_arr[i]  = (int*) malloc(2*sizeof(int));  
			phenarr.offday_arr[i] = (int*) malloc(2*sizeof(int));  
		}


		if (!phenarr.onday_arr || !phenarr.onday_arr)
		{
			printf("ERROR allocating for onday_arr/offday_arr, prephenology()\n");
			errorCode=3060;
		}
	}

	
	
	/* initialize the output mapping array */
	if (!errorCode && output_map_init(output_map,&phen,&metv,&ws,&wf,&cs,&cf,&ns,&nf,&sprop,&epv,&psn_sun,&psn_shade,&summary))
	{
		printf("ERROR in call to output_map_init() from transient_bgc.c\n");
		errorCode=4010;
	}
	
#ifdef DEBUG
	printf("done initialize outmap\n");
#endif
	
	
	/* atmospheric pressure (Pa) as a function of elevation (m) */
	if (!errorCode && atm_pres(sitec.elev, &metv.pa))
	{
		printf("ERROR in atm_pres() from transient_bgc.c\n");
		errorCode=4020;
	}
	
#ifdef DEBUG
	printf("done atm_pres\n");
#endif


    /* calculate GSI to deterime onday and offday 	*/	
	if (ctrl.GSI_flag && !PLT.PLT_num)
	{
		
		if (!errorCode && GSI_calculation(&metarr, &sitec, &epc, &phenarr, &ctrl))
		{
			printf("ERROR in call to GSI_calculation(), from transient_bgc.c\n");
			errorCode=4030;
		}

		#ifdef DEBUG
			printf("done GSI_calculation\n");
		#endif
	}


	/* calculate conductance limitation factors 	*/	
	if (!errorCode && conduct_limit_factors(bgcout->log_file, &ctrl, &sprop, &epc, &epv))
	{
		printf("ERROR in call to conduct_limit_factors(), from transient_bgc.c\n");
		errorCode=4040;
	}

#ifdef DEBUG
	printf("done conduct_limit_factors\n");
#endif

	/* determine phenological signals */
	if (!errorCode && prephenology(bgcout->log_file, &epc, &metarr, &PLT, &HRV, &ctrl, &phenarr))
	{
		printf("ERROR in call to prephenology(), from transient_bgc.c\n");
		errorCode=4050;
	}
	
#ifdef DEBUG
	printf("done prephenology\n");
#endif
	
	/* calculate the annual average air temperature for use in soil temperature corrections.  */
	tair_annavg = 0.0;
	nmetdays = ctrl.simyears * nDAYS_OF_YEAR;
	for (i=0 ; i<nmetdays ; i++)
	{
		tair_annavg += metarr.tavg[i];
	}
	tair_annavg /= (double)nmetdays;



	/* initialize leaf C and N pools depending on phenology signals for the first metday */
	if (!errorCode && firstday(&ctrl, &sprop, &epc, &PLT, &sitec, &cinit, &phen, &epv, &cs, &ns, &metv,&psn_sun,&psn_shade))
	{
		printf("ERROR in call to firstday(), from transient_bgc.c\n");
		errorCode=4070;
	}
#ifdef DEBUG
	printf("done firstday\n");
#endif


	/* zero water, carbon, and nitrogen source and sink variables */
	if (!errorCode && zero_srcsnk(&cs,&ns,&ws,&summary))
	{
		printf("ERROR in call to zero_srcsnk(), from transient_bgc.c\n");
		errorCode=4080;
	}

#ifdef DEBUG
	printf("done zero_srcsnk\n");
#endif

		
	
	/* initialize the indicator for first day of current simulation, so that the checks for mass balance can have two days for comparison */
	first_balance = 1;
	
	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
	/* 1. BEGIN OF THE ANNUAL LOOP */
	for (simyr=0 ; !errorCode && simyr<ctrl.simyears ; simyr++)
	{

		/* set current month to 0 (january) at the beginning of each year */
		curmonth = 0;

		/* counters into control sturct */ 
		ctrl.simyr = simyr;
		ctrl.spinyears = 0;


		/* output to screen to indicate start of simulation year */
		if (ctrl.onscreen) printf("Year: %6d\n",ctrl.simstartyear+simyr);

		/* set the max lai, maturity and flowering variables, for annual diagnostic output */
		epv.annmax_lai = 0.0;
		epv.annmax_rootDepth = 0.0;
		epv.annmax_plantHeight = 0.0;

		/* atmospheric CO2 handling */
		if (!(co2.varco2))
		{
			/*constant CO2 */
			metv.co2  = co2.co2ppm;
		}
		else
		{	
			/* CO2 from file */
			metv.co2 = co2.co2ppm_array[simyr];
		}

		 /* atmospheric Ndep handling */
		if (!(ndep.varndep))
		{
			/*constant Ndep */
			daily_ndep = ndep.ndep / nDAYS_OF_YEAR;
		}
		else
		{	
			/* Ndep from file */
			daily_ndep = ndep.Ndep_array[simyr] / nDAYS_OF_YEAR;
		}
		
	
		/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
		/* 2. BEGIN OF THE DAILY LOOP */
		for (yday=0 ; !errorCode && yday<nDAYS_OF_YEAR ; yday++)
		{

#ifdef DEBUG
			printf("year %d\tyday %d\n",simyr,yday);
#endif


			/* set the day index for meteorological and phenological arrays */
			ctrl.yday   = yday;
			ctrl.metday	= simyr*nDAYS_OF_YEAR + yday;		

			/* set fluxes to zero */
			if (!errorCode && make_zero_flux_struct(&ctrl,&wf, &cf, &nf))
			{
				printf("ERROR in call to make_zero_flux_struct() from transient_bgc.c\n");
				errorCode=5010;
			}

#ifdef DEBUG
	printf("done make_zero_flux\n");
#endif
			/* initalizing annmax variables */
			if (yday == 0)
			{
				epv.annmax_leafc = 0;
				epv.annmax_frootc = 0;
				epv.annmax_fruitc = 0;
				epv.annmax_softstemc = 0;
				epv.annmax_livestemc = 0;
				epv.annmax_livecrootc = 0;
			}

		    /* nitrogen deposition and fixation */
			nf.ndep_to_sminnTOTAL = daily_ndep;
			nf.nfix_to_sminnTOTAL = epc.nfix / nDAYS_OF_YEAR;


			/* calculating actual onday and offday */
			if (!errorCode && dayphen(&ctrl, &epc, &phenarr, &PLT, &phen))
			{
				printf("ERROR in dayphen from transient_bgc.c\n");
				errorCode=5020;
			}
			

			/* setting MANAGEMENT DAYS based on input data */
			if (!errorCode && management(&ctrl, &FRZ, &GRZ, &HRV, &MOW, &PLT, &PLG, &THN, &IRG, &gws))
			{
				printf("ERROR in management days() from transient_bgc.c\n");
				errorCode=5030;
			}


			/* determining soil hydrological parameters */
 			if (!errorCode && multilayer_hydrolparams(&sitec, &sprop, &ws, &epv))
			{
				printf("ERROR in multilayer_hydrolparams() from transient_bgc.c\n");
				errorCode=5040;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_hydrolparams\n",simyr,yday);
#endif

			/* daily meteorological variables from metarrays */
			if (!errorCode && daymet(&ctrl, &metarr, &epc, &sitec, &metv, &tair_annavg, ws.snoww))
			{
				printf("ERROR in daymet() from transient_bgc.c\n");
				errorCode=5050;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone daymet\n",simyr,yday);
#endif
			
			/* phenophases calculation */
			if (!errorCode && phenphase(bgcout->log_file, &ctrl, &epc, &sprop, &PLT, &phen, &metv, &epv, &cs))
			{
				printf("ERROR in phenphase() from transient_bgc.c\n");
				errorCode=5060;
			}

#ifdef DEBUG
				printf("%d\t%d\tdone phenphase\n",simyr,yday);
#endif
			/* soil temperature calculations */
			if (!errorCode && multilayer_tsoil(&epc, &sitec, &sprop, &epv, yday, ws.snoww, &metv))
			{
				printf("ERROR in multilayer_tsoil() from transient_bgc.c\n");
				errorCode=5070;
			}
#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_tsoil\n",simyr,yday);
#endif

			/* mulch calculations */
			if (!errorCode && mulch(&sitec, &sprop, &metv, &epv, &cs))
			{
				printf("ERROR in mulch() from transient_bgc.c\n");
				errorCode=5470;
			}
#ifdef DEBUG
			printf("%d\t%d\tdone mulch\n",simyr,yday);
#endif


			/* phenology calculation */
			if (!errorCode && phenology(&ctrl, &epc, &cs, &ns, &phen, &metv, &epv, &cf, &nf))
			{
				printf("ERROR in phenology() from transient_bgc.c\n");
				errorCode=5090;
			}
#ifdef DEBUG
			printf("%d\t%d\tdone phenology\n",simyr,yday);
#endif


		
			/* calculate leaf area index, sun and shade fractions, and specific leaf area for sun and shade canopy fractions, then calculate canopy radiation interception and transmission */          
			if (!errorCode && radtrans(&ctrl, &phen, &cs, &epc, &sitec, &metv, &epv))
			{
				printf("ERROR in radtrans() from transient_bgc.c\n");
				errorCode=5100;
			}
			
			
			/* update the ann max LAI for annual diagnostic output */
			if (epv.proj_lai > epv.annmax_lai)             epv.annmax_lai = epv.proj_lai;
			if (epv.rootdepth > epv.annmax_rootDepth)      epv.annmax_rootDepth = epv.rootdepth;
			if (epv.plant_height > epv.annmax_plantHeight) epv.annmax_plantHeight = epv.plant_height;
			
#ifdef DEBUG
			printf("%d\t%d\tdone radtrans\n",simyr,yday);
#endif
			

			/* precip routing (when there is precip) */
			if (!errorCode && metv.prcp && prcp_route(&metv, epc.int_coef, epv.all_lai,	&wf))
			{
				printf("ERROR in prcp_route() from transient_bgc.c\n");
				errorCode=5100;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone prcp_route\n",simyr,yday);
#endif

	
			/* snowmelt (when there is a snowpack) */
			if (!errorCode && ws.snoww && snowmelt(&metv, &wf, ws.snoww))
			{
				printf("ERROR in snowmelt() from transient_bgc.c\n");
				errorCode=5110;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone snowmelt\n",simyr,yday);
#endif

		
			  /* potential bare-soil evaporation */
			if (!errorCode && potential_evap(&sprop, &metv, &wf))
			{
				printf("ERROR in potential_evap() from transient_bgc.c\n");
				errorCode=5120;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone bare_soil evap\n",simyr,yday);
#endif
		  	/* conductance calculation */
			if (!errorCode && conduct_calc(&ctrl, &metv, &epc, &epv, simyr))
			{
				printf("ERROR in conduct_calc() from transient_bgc.c\n");
				errorCode=5130;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone conduct_calc\n",simyr,yday);
#endif
		

		
			/* begin canopy bio-physical process simulation */
			/* do canopy ET calculations whenever there is leaf areadisplayed, since there may be intercepted water on the canopy that needs to be dealt with */
			if (!errorCode && epv.n_actphen > epc.n_emerg_phenophase && metv.dayl)
			{
				/* evapo-transpiration */
				if (!errorCode && cs.leafc && canopy_et(&epc, &metv, &epv, &wf))
				{
					printf("ERROR in canopy_et() from transient_bgc.c\n");
					errorCode=5140;
				}
				
#ifdef DEBUG
				printf("%d\t%d\tdone canopy_et\n",simyr,yday);
#endif

			}

			/* daily maintenance respiration */
			if (!errorCode && maint_resp(&PLT, &cs, &ns, &epc, &metv, &epv, &cf))
			{
				printf("ERROR in m_resp() from transient_bgc.c\n");
				errorCode=5150;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone maint resp\n",simyr,yday);
#endif


			/* do photosynthesis calculation */
			if (!errorCode && cs.leafc && photosynthesis(&epc, &metv, &cs, &ws, &phen, &epv, &psn_sun, &psn_shade, &cf))
			{
				printf("ERROR in photosynthesis() from transient_bgc.c\n");
				errorCode=5160;
			}
				
#ifdef DEBUG
				printf("%d\t%d\tdone photosynthesis\n",simyr,yday);
#endif

			/* daily litter and soil decomp and nitrogen fluxes */
			if (!errorCode && decomp(&metv,&epc,&sprop,&sitec,&cs,&ns,&epv,&cf,&nf,&nt))
			{
				printf("ERROR in decomp() from bgc.c\n");
				errorCode=5170;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone decomp\n",simyr,yday);
#endif

	

	        /* Daily allocation gets called whether or not this is a current growth day, because the competition between decomp immobilization fluxes 
			and plant growth N demand is resolved here.  On days with no growth, no allocation occurs, but immobilization fluxes are updated normally */

			if (!errorCode && daily_allocation(&epc,&sitec,&sprop,&metv,&cs,&ns,&cf,&nf,&epv,&nt,0))
			{
				printf("ERROR in daily_allocation() from bgc.c\n");
				errorCode=5180;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone daily_allocation\n",simyr,yday);
#endif

                        /* heat stress during flowering can affect daily allocation of fruit */
			if (epc.n_flowHS_phenophase != DATA_GAP)
			{
				if (!errorCode && flowering_heatstress(&epc, &metv, &epv, &cf, &nf))
				{
					printf("ERROR in flowering_heatstress() from transient_bgc.c\n");
					errorCode=5190;
				}
			}

			/* reassess the annual turnover rates for livewood --> deadwood, and for evergreen leaf and fine root litterfall. 
			This happens once each year, on the annual_alloc day (the last litterfall day - test for annual allocation day) */
			
			if (phen.remdays_litfall == 1) annual_alloc = 1;
			else annual_alloc = 0;

			if (!errorCode && annual_alloc)
			{
				if (!errorCode && annual_rates(&epc,&epv))
				{
					printf("ERROR in annual_rates() from transient_bgc.c\n");
					errorCode=5200;
				}
				
#ifdef DEBUG
				printf("%d\t%d\tdone annual rates\n",simyr,yday);
#endif
			} 


			/* daily growth respiration */
			if (!errorCode && growth_resp(&epc, &cf))
			{
				printf("ERROR in daily_growth_resp() from bgc.c\n");
				errorCode=5210;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone growth_resp\n",simyr,yday);
#endif
			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
			/* 3. WATER CALCULATIONS WITH STATE UPDATE */
			
			

	        /* IRRIGATING separately from other management routines */
			if (!errorCode && irrigating(&ctrl, &IRG, &epv, &ws, &wf))
			{
				printf("ERROR in irrigating() from transient_bgc.c\n");
				errorCode=5220;
			}
	

#ifdef DEBUG
			printf("%d\t%d\tdone groundwater\n",simyr,yday);
#endif	
         
			/* multilayer soil hydrology: percolation calculation based on PRCP, RUNOFF, EVAP, TRANS */
			if (!errorCode && multilayer_hydrolprocess(&ctrl,  &sitec, &sprop, &epc, &epv, &ws, &wf, &gws))
			{
				printf("ERROR in multilayer_hydrolprocess() from transient_bgc.c\n");
				errorCode=5240;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_hydrolprocess\n",simyr,yday);
#endif	
		
                     
			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
			/* 4. STATE UPDATE */

			/* daily update of the water state variables */
			if (!errorCode && daily_water_state_update(&epc, &wf, &ws))
			{
				printf("ERROR in daily_water_state_update() from transient_bgc.c\n");
				errorCode=5250;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone water state update\n",simyr,yday);
#endif
		
	
	        /* daily update of carbon and nitrogen state variables */
			if (!errorCode && daily_CN_state_update(&sitec, &epc, &ctrl, &epv, &cf, &nf, &cs, &ns, annual_alloc, epc.evergreen))
			{
				printf("ERROR in daily_CN_state_update() from transient_bgc.c\n");
				errorCode=5260;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone CN state update\n",simyr,yday);
#endif

		       
			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
			/* 5. MORTALITY AND NITROGEN FLUXES CALCULATION WITH OWN STATE UPDATE: 
			to insure that pools don't go negative due to mortality/leaching fluxes conflicting with other proportional fluxes  */

			/* calculate daily senescence mortality fluxes and update state variables */
			if (!errorCode && senescence(&sitec, &epc, &GRZ, &metv, &ctrl, &cs, &cf, &ns, &nf, &epv))
			{
				printf("ERROR in senescence() from transient_bgc.c\n");
				errorCode=5270;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone senescence\n",simyr,yday);
#endif
		     /* calculate daily mortality fluxes  and update state variables */
			if (!errorCode && mortality(&ctrl, &sitec, &epc, &epv, &cs, &cf, &ns, &nf, simyr))
			{
				printf("ERROR in mortality() from transient_bgc.c\n");
				errorCode=5280;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone mortality\n",simyr,yday);
#endif
		
			
			/* calculate the change of soil mineralized N in multilayer soil */ 
			if (!errorCode && multilayer_sminn(&ctrl, &metv, &sprop, &sitec, &cf, &epv, &ns, &nf))
			{
				printf("ERROR in multilayer_sminn() from transient_bgc.c\n");
				errorCode=5290;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_sminn\n",simyr,yday);
#endif

			/* calculate the leaching of N, DOC and DON from multilayer soil */
			if (!errorCode && multilayer_leaching(&sprop, &epv, &ctrl, &cs, &cf, &ns, &nf, &ws, &wf))
			{
				printf("ERROR in multilayer_leaching() from transient_bgc.c\n");
				errorCode=5300;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_leaching\n",simyr,yday);
#endif

			
			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
			/* 6. MANAGEMENT FLUXES */
			
			/* PLANTING */
			if (!errorCode && planting(&ctrl, &sitec, &PLT, &epc, &epv, &phen, &cs, &ns, &cf, &nf))
			{
				printf("ERROR in planting() from transient_bgc.c\n");
				errorCode=5310;
			}

		   	/* THINNIG  */
			if (!errorCode && thinning(&ctrl, &epc, &THN, &cs, &ns, &ws, &cf, &nf, &wf))
			{
				printf("ERROR in thinning() from transient_bgc.c\n");
				errorCode=5320;
			}

			/* MOWING  */
			if (!errorCode && mowing(&ctrl, &epc, &MOW, &epv, &cs, &ns, &ws, &cf, &nf, &wf))
			{
				printf("ERROR in mowing() from transient_bgc.c\n");
				errorCode=5330;
			}

			/* GRAZING  */
			if (!errorCode && grazing(&ctrl, &epc, &sitec, &GRZ, &epv, &cs, &ns, &ws, &cf, &nf, &wf))
			{
				printf("ERROR in grazing() from transient_bgc.c\n");
				errorCode=5340;
			}
	
		   	/* HARVESTING  */
			if (!errorCode && harvesting(bgcout->econout_file, &ctrl, &phen, &epc, &HRV, &IRG, &epv, &cs, &ns, &ws, &cf, &nf, &wf))
			{
				printf("ERROR in harvesting() from transient_bgc.c\n");
				errorCode=5350;
			}
 
			/* PLOUGHING */
 			if (!errorCode && ploughing(&ctrl, &epc, &sitec, &sprop, &metv, &epv, &PLG, &cs, &ns, &ws, &cf, &nf, &wf))
			{
				printf("ERROR in ploughing() from transient_bgc.c\n");
				errorCode=5360;
			}
		 
			/* FERTILIZING  */
	    	if (!errorCode && fertilizing(&ctrl, &sitec, &sprop, &FRZ, &epv, &cs, &ns, &ws, &cf, &nf, &wf))
			{
				printf("ERROR in fertilizing() from transient_bgc.c\n");
				errorCode=5370;
			}	
				
			/* cut-down plant material (due to management) */
			if (!errorCode && cutdown2litter(&sitec, &epc, &epv, &cs, &cf, &ns, &nf))
			{
				printf("ERROR in cutdown2litter() from transient_bgc.c\n");
				errorCode=5390;
			}

			/* calculating rooting depth, n_rootlayers, n_maxrootlayers, rootlength_prop */
 			 if (!errorCode && multilayer_rootdepth(&epc, &sprop, &cs, &sitec, &epv))
			 {
				printf("ERROR in multilayer_rootdepth() from transient_bgc.c\n");
				errorCode=5400;
			 }
#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_rootdepth\n",simyr,yday);
#endif
			
			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
			/* 7. ERROR CHECKING AND SUMMARY VARIABLES  */
			
			/* test for very low state variable values and force them to 0.0 to avoid rounding and floating point overflow errors */
			if (!errorCode && precision_control(&ws, &cs, &ns))
			{
				printf("ERROR in call to precision_control() from transient_bgc.c\n");
				errorCode=5410;
			} 
				
#ifdef DEBUG
				printf("%d\t%d\tdone precision_control\n",simyr,yday);
#endif

			/* test for water balance*/
			if (!errorCode && check_water_balance(&ws, first_balance))
			{
				printf("ERROR in check_water_balance() from transient_bgc.c\n");
				errorCode=5420;
			}
			 
#ifdef DEBUG
			printf("%d\t%d\tdone water balance\n",simyr,yday);
#endif

	
            /* test for carbon balance */
			if (!errorCode  && check_carbon_balance(&cs, first_balance))
			{
				printf("ERROR in check_carbon_balance() from transient_bgc.c\n");
				errorCode=5430;
			}
		
			
#ifdef DEBUG
			printf("%d\t%d\tdone carbon balance\n",simyr,yday); 
#endif

			/* test for nitrogen balance */
			if (!errorCode && check_nitrogen_balance(&ns, first_balance))
			{
				printf("ERROR in check_nitrogen_balance() from transient_bgc.c\n");
				errorCode=5440;
			}
			
			
#ifdef DEBUG
			printf("%d\t%d\tdone nitrogen balance\n",simyr,yday);
#endif
		

			/* calculate summary variables */
			if (!errorCode && cnw_summary(yday, &epc, &sitec, &sprop, &metv, &cs, &cf, &ns, &nf, &wf, &epv, &summary))
			{
				printf("ERROR in cnw_summary() from transient_bgc.c\n");
				errorCode=5450;
			}
	
	
#ifdef DEBUG
			printf("%d\t%d\tdone carbon summary\n",simyr,yday);
#endif

	        /* output handling */
			if (!errorCode && output_handling(mondays[curmonth], endday[curmonth], &ctrl, output_map, dayarr, monavgarr, annavgarr, annarr, 
				                            bgcout->dayoutT, bgcout->monavgoutT, bgcout->annavgoutT, bgcout->annoutT))
			{
				printf("ERROR in output_handling() from transient_bgc.c\n");
				errorCode=5460;
			}
	
	
#ifdef DEBUG
			printf("%d\t%d\tdoneoutput_handling\n",simyr,yday); 
#endif

			/* at the end of first day of simulation, turn off the first_balance switch */
			if (first_balance) first_balance = 0;

			/*  if no dormant period (e.g. evergreen): last day is the dormant day */
			if (phen.offday - phen.onday == 364 && phen.offday == phen.yday_total) 
			{
				epv.n_actphen = 0;
				phen.onday = -1;
				phen.offday = -1;
				phen.remdays_litfall =-1;
			}

			/* if this is the last day of the current month: increment current month counter */
			if (yday == endday[curmonth]) curmonth++;
		
	}

}
	bgcin->ws = ws;
	bgcin->cs = cs;
	bgcin->ns = ns;

	
	/********************************************************************************************************* */
	/* writing log file */
    /********************************************************************************************************* */

	/* most important informations - onscreen */
	if (ctrl.onscreen) 
	{
		printf("\n");
		printf("INFORMATION FROM THE LAST SIMULATION YEAR.\n");
	    printf("Maximum projected LAI [m2/m2]:                 %12.1f\n",epv.annmax_lai);
		printf("Total litter and cwdc carbon content [kgC/m2]: %12.1f\n",summary.litrCwdC_total);
		printf("Total soil carbon content [kgC/m2]:            %12.1f\n",summary.soilC_total);
	}

	if (cs.CbalanceERR != 0) CbalanceERR = log10(cs.CbalanceERR);
	if (ns.NbalanceERR != 0) NbalanceERR = log10(ns.NbalanceERR);
	if (ws.WbalanceERR != 0) WbalanceERR = log10(ws.WbalanceERR);
	if (cs.CNratioERR  != 0) CNratioERR  = log10(cs.CNratioERR);

	fprintf(bgcout->log_file.ptr, "SOME IMPORTANT ANNUAL OUTPUTS FROM LAST SIMULATION YEAR\n");
    fprintf(bgcout->log_file.ptr, "Cumulative sum of GPP [gC/m2/year]:                      %12.1f\n",summary.cum_gpp*1000);
	fprintf(bgcout->log_file.ptr, "Cumulative sum of NEE [gC/m2/year]:                      %12.1f\n",summary.cum_nee*1000);
	fprintf(bgcout->log_file.ptr, "Cumulative sum of ET  [kgH2O/m2/year]:                   %12.1f\n",summary.cum_ET);
	fprintf(bgcout->log_file.ptr, "Cumulative sum of soilEvap [kgH2O/m2/year]:              %12.1f\n",summary.cum_evap);
	fprintf(bgcout->log_file.ptr, "Cumulative sum of TRANSP   [kgH2O/m2/year]:              %12.1f\n",summary.cum_transp);
	fprintf(bgcout->log_file.ptr, "Cumulative sum of N2O flux [gN/m2/year]:                 %12.2f\n",summary.cum_n2o*1000);
	fprintf(bgcout->log_file.ptr, "Maximum projected LAI [m2/m2]:                           %12.2f\n",epv.annmax_lai);
	fprintf(bgcout->log_file.ptr, "Soil carbon content (in 0-30 cm soil layer) [%%]:         %12.2f\n",summary.SOM_C_top30);
	fprintf(bgcout->log_file.ptr, "Total soil carbon content [kgC/m2/year]:                 %12.2f\n",summary.soilC_total);
	fprintf(bgcout->log_file.ptr, "Total litter and cwdc carbon content [kgC/m2]:           %12.2f\n",summary.litrCwdC_total);
	fprintf(bgcout->log_file.ptr, "Averaged available soil ammonium content (0-30 cm) [ppm]:%12.2f\n",summary.NH4_top30avail);
	fprintf(bgcout->log_file.ptr, "Averaged available soil nitrate content (0-30 cm) [ppm]: %12.2f\n",summary.NO3_top30avail);
	fprintf(bgcout->log_file.ptr, "Averaged soil water content  [m3/m3]:                    %12.2f\n",epv.VWC_avg);
	fprintf(bgcout->log_file.ptr, " \n");
	fprintf(bgcout->log_file.ptr, "10-base logarithm of the maximum carbon balance diff.:   %12.1f\n",CbalanceERR);
	fprintf(bgcout->log_file.ptr, "10-base logarithm of the maximum nitrogen balance diff.: %12.1f\n",NbalanceERR);
	fprintf(bgcout->log_file.ptr, "10-base logarithm of the maximum water balance diff.:    %12.1f\n",WbalanceERR);
	fprintf(bgcout->log_file.ptr, "10-base logarithm of the C-N calc. numbering error:      %12.1f\n",CNratioERR);
	fprintf(bgcout->log_file.ptr, " \n");

	fprintf(bgcout->log_file.ptr, "WARNINGS \n");
	if (!ctrl.limittransp_flag && !ctrl.limitevap_flag && !ctrl.limitleach_flag && !ctrl.limitleach_flag && !ctrl.limitdiffus_flag &&
		!ctrl.limitSNSC_flag && !ctrl.limitMR_flag && !ctrl.notransp_flag && !ctrl.noMR_flag && !ctrl.pond_flag&& !ctrl.grazingW_flag &&
		!ctrl.condMOWerr_flag && !ctrl.condIRGerr_flag && !ctrl.condIRGerr_flag && !ctrl.prephen1_flag && !ctrl.prephen2_flag && 
		!ctrl.bareground_flag && !ctrl.vegper_flag)
	{
		fprintf(bgcout->log_file.ptr, "no WARNINGS\n");
	}
	else
	{
		if (ctrl.limittransp_flag)
		{
			fprintf(bgcout->log_file.ptr, "Limited transpiration due to dry soil\n");
			ctrl.limittransp_flag = -1;
		}

		if (ctrl.limitevap_flag)
		{
			fprintf(bgcout->log_file.ptr, "Limited evaporation due to dry soil\n");
			ctrl.limitevap_flag = -1;
		}

		if (ctrl.limitleach_flag)
		{
			fprintf(bgcout->log_file.ptr, "Limited leaching\n");
			ctrl.limitleach_flag = -1;
		}

		if (ctrl.limitdiffus_flag)
		{
			fprintf(bgcout->log_file.ptr, "Limited diffusing\n");
			ctrl.limitdiffus_flag = -1;
		}

		if (ctrl.limitSNSC_flag)
		{
			fprintf(bgcout->log_file.ptr, "Limited genetically programmed leaf senescence\n");
			ctrl.limitSNSC_flag = -1;
		}

		if (ctrl.limitMR_flag)
		{
			fprintf(bgcout->log_file.ptr, "Limited maintanance respiration \n");
			ctrl.limitMR_flag = -1;
		}

		if (ctrl.notransp_flag)
		{
			fprintf(bgcout->log_file.ptr, "No transpiration (full limitation) due to dry soil\n");
			ctrl.notransp_flag = -1;
		}

		if (ctrl.noMR_flag)
		{
			fprintf(bgcout->log_file.ptr, "No woody maintanance respiration  (full limitation)\n");
			ctrl.noMR_flag = -1;
		}

		if (ctrl.pond_flag)
		{
			fprintf(bgcout->log_file.ptr, "Pond water on soil surface\n");
			ctrl.notransp_flag = -1;
		}
		
		if (ctrl.grazingW_flag)
		{
			fprintf(bgcout->log_file.ptr, "Not enough grass for grazing\n");
			ctrl.grazingW_flag = -1;
		}

		if (ctrl.condMOWerr_flag)
		{
			fprintf(bgcout->log_file.ptr, "If conditional MOWING flag is on, no MOWING is possible\n");
			ctrl.condMOWerr_flag = -1;
		}

		if (ctrl.condIRGerr_flag)
		{
			fprintf(bgcout->log_file.ptr, "If conditional IRRIGATING flag is on, no IRRIGATING is possible\n");
			ctrl.condIRGerr_flag = -1;
		}

	
		if (ctrl.prephen1_flag)
		{
			fprintf(bgcout->log_file.ptr, "In case of planting model-defined phenology is not possible (firstday:planting, lastday:harvesting)\n");
			ctrl.prephen1_flag = -1;
		}

		if (ctrl.prephen2_flag)
		{
			fprintf(bgcout->log_file.ptr, "In case of user-defined phenology GSI calculation is not possible\n");
			ctrl.prephen2_flag = -1;
		}

		if (ctrl.bareground_flag)
		{
			fprintf(bgcout->log_file.ptr, "User-defined bareground run (onday and offday set to -9999 in EPC)\n");
			ctrl.bareground_flag = -1;
		}

		if (ctrl.vegper_flag)
		{
			fprintf(bgcout->log_file.ptr, "Vegetation period has not ended until the last day of year, the offday is equal to the last day of year\n");
			ctrl.vegper_flag = -1;
		}


		
	}
	/********************************************************************************************************* */

	
	/* free memory for local output arrays */
	
    if ((errorCode == 0 || errorCode > 301) && ctrl.dodaily) free(dayarr);
	if ((errorCode == 0 || errorCode > 302) && ctrl.domonavg) free(monavgarr);
	if ((errorCode == 0 || errorCode > 303) && ctrl.doannavg) free(annavgarr);
	if ((errorCode == 0 || errorCode > 304) && ctrl.doannual) free(annarr); 
	if ((errorCode == 0 || errorCode > 305)) free(output_map);
    if (((errorCode == 0 || errorCode > 306) && !ctrl.GSI_flag) || ((errorCode == 0 || errorCode > 405) && ctrl.GSI_flag)) 
	{
		free(phenarr.onday_arr);
		free(phenarr.offday_arr);
		if (ctrl.GSI_flag)
		{
			free(phenarr.tmin_index);
			free(phenarr.vpd_index);
			free(phenarr.heatsum_index);
			free(phenarr.heatsum);
			free(phenarr.dayl_index);
			free(phenarr.gsi_indexAVG);
		}
	}
	
	/* print timing info if error */
	if (errorCode)
	{
		printf("ERROR at year %d\n",simyr-1);
		printf("ERROR at yday %d\n",yday-1);
	}
	


    /* return error status */	
	return (errorCode);
}
