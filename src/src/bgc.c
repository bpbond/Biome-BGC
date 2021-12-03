/*
bgc.c
Core BGC model logic

Includes in-line output handling routines that write to daily and annual
output files. This is the only library module that has external
I/O connections, and so it is the only module that includes bgc_io.h.

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2020, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Modified:
4/17/2000 (PET): Added annual average temperature and annual total precipitation
to the simple annual text output file.
Modified:
13/07/2000: Added input of Ndep from file. Changes are made by Galina Churkina. 
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
/* #define DEBUG   set this to see function roll-call on-screen */



int bgc(bgcin_struct* bgcin, bgcout_struct* bgcout)
{
	
	/* variable declarations */
	int errorCode=0;
	int nyears;


	/* iofiles and program control variables */
	control_struct     ctrl;

	/* meteorological variables */
	metarr_struct			metarr;
	metvar_struct			metv;
	co2control_struct		co2;
	ndep_control_struct		ndep;

	
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

		
	/* ecophysiological constants */
	epconst_struct     epc;

	/* phenological data */
	phenarray_struct   phenarr;
    phenology_struct   phen;


	/* photosynthesis structures */
	psn_struct         psn_sun, psn_shade;
	
	/* temporary nitrogen variables for decomposition and allocation */
	ntemp_struct       nt;
	
	/* summary variable structure */
	summary_struct     summary;

	/* miscelaneous variables for program control in main */
	int simyr = 0;
	int yday  = 0;
	int i     = 0;
	int first_balance;
	int annual_alloc;

	double daily_ndep; 
	double tair_annavg;
	double nmetdays;

	double CbalanceERR = 0;
	double NbalanceERR = 0;
	double WbalanceERR = 0;
	double CNratioERR = 0;

	/* local storage for daily and annual output variables and output mapping (array of pointers to double)  */
	double* dayarr=0;
	double* monavgarr=0;
	double* annavgarr=0;
	double* annarr=0;
	double** output_map=0;

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
 


	/********************************************************************************************************* */
	/* writing logfile */
	fprintf(bgcout->log_file.ptr, "NORMAL RUN\n");
	fprintf(bgcout->log_file.ptr, " \n");

	fprintf(bgcout->log_file.ptr, "VEGETATION TYPE\n");
	if (epc.woody == 0)
		fprintf(bgcout->log_file.ptr, "biome type           - NON-WOODY\n");
	else
		fprintf(bgcout->log_file.ptr, "biome type           - WOODY\n");
	
	if (epc.evergreen == 0)
		fprintf(bgcout->log_file.ptr, "woody type           - DECIDUOUS\n");
	else
		fprintf(bgcout->log_file.ptr, "woody type           - EVERGREEN\n");

	if (epc.c3_flag == 0)
		fprintf(bgcout->log_file.ptr, "photosyn.type        - C4 PSN\n");
	else
		fprintf(bgcout->log_file.ptr, "photosyn.type        - C3 PSN\n");
	fprintf(bgcout->log_file.ptr, " \n");

	fprintf(bgcout->log_file.ptr, "CALCULATION METHODS\n");
	if (epc.SHCM_flag == 0 || epc.SHCM_flag == 2)
	{
		if (epc.SHCM_flag == 0)
			fprintf(bgcout->log_file.ptr, "hydrology            - tipping (with diffusion)\n");
		else
			fprintf(bgcout->log_file.ptr, "hydrology            - tipping (without diffusion)\n");
	}
	else
	{
		fprintf(bgcout->log_file.ptr, "hydrology            - Richards\n");
	}


	if (epc.STCM_flag == 0)
		fprintf(bgcout->log_file.ptr, "temperature          - MuSo\n");
	else
		fprintf(bgcout->log_file.ptr, "temperature          - DSSAT\n");

	if (epc.photosynt_flag == 0)
		fprintf(bgcout->log_file.ptr, "photosynthesis       - Farquhar\n");
	else
		fprintf(bgcout->log_file.ptr, "photosynthesis       - DSSAT\n");

	if (epc.evapotransp_flag == 0)
		fprintf(bgcout->log_file.ptr, "evapotranspiration   - Penman-Monteith\n");
	else
		fprintf(bgcout->log_file.ptr, "evapotranspiration   - Priestley-Taylor\n");

	if (epc.radiation_flag == 0)
		fprintf(bgcout->log_file.ptr, "radiation            - based on SWabs\n");
	else
		fprintf(bgcout->log_file.ptr, "radiation            - based on Rn\n");

	if (epc.soilstress_flag == 0)
		fprintf(bgcout->log_file.ptr, "soilstress           - based on VWC\n");
	else
		fprintf(bgcout->log_file.ptr, "soilstress           - based on transp.demand\n");

	if (epc.transferGDD_flag == 0)
		fprintf(bgcout->log_file.ptr, "transfer period      - EPC\n");
	else
		fprintf(bgcout->log_file.ptr, "transfer period      - GDD\n");

	if (epc.q10depend_flag == 0)
		fprintf(bgcout->log_file.ptr, "q10 value            - constant\n");
	else
		fprintf(bgcout->log_file.ptr, "q10 value            - temperature dependent\n");

	if (epc.phtsyn_acclim_flag == 0)
		fprintf(bgcout->log_file.ptr, "photosyn. acclim.    - no\n");
	else
		fprintf(bgcout->log_file.ptr, "photosyn. acclim.    - yes\n");

	if (epc.resp_acclim_flag == 0)
		fprintf(bgcout->log_file.ptr, "respiration acclim.  - no\n");
	else
		fprintf(bgcout->log_file.ptr, "respiration acclim.  - yes\n");

	if (epc.CO2conduct_flag == 0)
		fprintf(bgcout->log_file.ptr, "CO2 conduct. effect  - no effect\n");
	else
		fprintf(bgcout->log_file.ptr, "CO2 conduct. effect  - reduction\n");

	if (sprop.Tp1_decomp == DATA_GAP)
		fprintf(bgcout->log_file.ptr, "Decomposition Tresp. - Lloyd and Taylor\n");
	else
		fprintf(bgcout->log_file.ptr, "Decomposition Tresp. - Bell function\n");

	if (sprop.Tp1_nitrif == DATA_GAP)
		fprintf(bgcout->log_file.ptr, "Nitrification Tresp. - Lloyd and Taylor\n");
	else
		fprintf(bgcout->log_file.ptr, "Nitrification Tresp. - Bell function\n");

	fprintf(bgcout->log_file.ptr, " \n");

	fprintf(bgcout->log_file.ptr, "DATA SOURCES\n");

	if (epc.phenology_flag == 0) 
	{
		if (PLT.PLT_num != 0)
		{
			fprintf(bgcout->log_file.ptr, "SGS data - planting date from planting file\n");
		}
		else
		{
			if (ctrl.varSGS_flag == 0) 
				fprintf(bgcout->log_file.ptr, "SGS data - user-defined from EPC file\n");
			else
			{
				fprintf(bgcout->log_file.ptr, "SGS data - user-defined from annual varying SGS file\n");
				if (ctrl.onscreen) printf("INFORMATION: reading onday_normal.txt: annual varying SGS data\n");
			}
		}
		if (HRV.HRV_num)
		{
			fprintf(bgcout->log_file.ptr, "EGS data - harvesting date from harvesting file\n");
		}
		else
		{
			if (ctrl.varEGS_flag == 0) 
				fprintf(bgcout->log_file.ptr, "EGS data - user-defined from EPC file\n");
			else
			{
				fprintf(bgcout->log_file.ptr, "EGS data - user-defined from annual varying EGS file\n");
				if (ctrl.onscreen) printf("INFORMATION: reading onday_normal.txt: annual varying SGS data\n");
			}
		}
	}
	else
	{
		if (ctrl.GSI_flag == 0) 
		{
			if (PLT.PLT_num != 0) 
				fprintf(bgcout->log_file.ptr, "SGS data - planting date from planting file\n");
			else
				fprintf(bgcout->log_file.ptr, "SGS data - model estimation (with original method)\n");
			
			if (HRV.HRV_num)
				fprintf(bgcout->log_file.ptr, "EGS data - harvesting date from harvesting file\n");
			else
				fprintf(bgcout->log_file.ptr, "EGS data - model estimation (with original method)\n");
		}
		else
		{
			if (PLT.PLT_num != 0) 
				fprintf(bgcout->log_file.ptr, "SGS data - planting date from planting file\n");
			else
				fprintf(bgcout->log_file.ptr, "SGS data - model estimation (with GSI method)\n");
			
			if (HRV.HRV_num)
				fprintf(bgcout->log_file.ptr, "EGS data - harvesting date from harvesting file\n");
			else
				fprintf(bgcout->log_file.ptr, "EGS data - model estimation (with GSI method)\n");
		}
	}

	if (ctrl.oldSOIfile_flag == 0) 
		fprintf(bgcout->log_file.ptr, "SOI data - new SOI file (number of lines: 92)\n");
	else
		fprintf(bgcout->log_file.ptr, "SOI data - old SOI file  (number of lines: 64) and extraSOIparameters.txt\n");

	if (ctrl.varWPM_flag == 0) 
		fprintf(bgcout->log_file.ptr, "WPM data - constant \n");
	else
	{
		fprintf(bgcout->log_file.ptr, "WPM data - annual varying\n");
		if (ctrl.onscreen) printf("INFORMATION: reading mortality_normal.txt - annual varying WPM data\n");
	}

	if (ctrl.varMSC_flag == 0) 
		fprintf(bgcout->log_file.ptr, "MSC data - constant\n");
	else
	{
		fprintf(bgcout->log_file.ptr, "MSC data - annual varying\n");
		if (ctrl.onscreen) printf("INFORMATION: reading conductance_normal.txt: annual varying MSC data\n");
	}

	if (PLT.PLT_num || THN.THN_num || MOW.MOW_num || GRZ.GRZ_num || HRV.HRV_num || PLG.PLG_num || FRZ.FRZ_num || IRG.IRG_num || 
		ctrl.condIRG_flag || ctrl.condMOW_flag)
		fprintf(bgcout->log_file.ptr, "management  - YES\n");
	else
		fprintf(bgcout->log_file.ptr, "management  - NO\n");

	if (gws.GWD_num == 0) 
		fprintf(bgcout->log_file.ptr, "groundwater - NO\n");
	else
	{
		fprintf(bgcout->log_file.ptr, "groundwater - YES\n");
		if (ctrl.onscreen) printf("INFORMATION: reading groundwater_normal.txt - daily GWD data\n");
	}

	fprintf(bgcout->log_file.ptr, " \n");

	fprintf(bgcout->log_file.ptr, "CRITICAL VWC (m3/m3) AND PSI (MPa) VALUES OF TOP SOIL LAYER \n");
	fprintf(bgcout->log_file.ptr, "saturation:                    %12.3f%12.4f\n",sprop.VWCsat[0],sprop.PSIsat[0]);
	fprintf(bgcout->log_file.ptr, "field capacity:                %12.3f%12.4f\n",sprop.VWCfc[0], sprop.PSIfc[0]);
	fprintf(bgcout->log_file.ptr, "wilting point:                 %12.3f%12.4f\n",sprop.VWCwp[0], sprop.PSIwp[0]);
	fprintf(bgcout->log_file.ptr, "hygroscopic water:             %12.3f%12.4f\n",sprop.VWChw[0], sprop.PSIhw);
	fprintf(bgcout->log_file.ptr, "bulk density:                  %12.3f\n",sprop.BD[0]);
	fprintf(bgcout->log_file.ptr, "Clapp-Hornberger b parameter:  %12.3f\n",sprop.soilB[0]);
	fprintf(bgcout->log_file.ptr, " \n");

	if (HRV.HRV_num) fprintf(bgcout->econout_file.ptr, "year planttype primaryProd[tC/ha] secondaryProd[tC/ha] condIRGamunt condIRGtype\n");
	/********************************************************************************************************* */
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/* MEMORY ALLOCATION */
	
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
			errorCode=301;
		}
		for (i=0 ; i<ctrl.ndayout ; i++) dayarr[i] = 0.0;
	}

	if (!errorCode && ctrl.domonavg) 
	{
		monavgarr = (double*) malloc(ctrl.ndayout * sizeof(double));
		if (!monavgarr)
		{
			printf("ERROR allocating for monthly average output array in bgc()\n");
			errorCode=302;
		}

		for (i=0 ; i<ctrl.ndayout ; i++) monavgarr[i] = 0.0;
	}
	if (!errorCode && ctrl.doannavg) 
	{
		annavgarr = (double*) malloc(ctrl.ndayout * sizeof(double));
		if (!annavgarr)
		{
			printf("ERROR allocating for annual average output array in bgc()\n");
			errorCode=303;
		}
		for (i=0 ; i<ctrl.ndayout ; i++) annavgarr[i] = 0.0;
	}
	if (!errorCode && ctrl.doannual)
	{
		annarr = (double*) malloc(ctrl.nannout * sizeof(double));
		if (!annarr)
		{
			printf("ERROR allocating for local annual output array in bgc()\n");
			errorCode=304;
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
			errorCode=305;
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
			errorCode=306;
		}
	}


	
	
	/* initialize the output mapping array */
	if (!errorCode && output_map_init(output_map,&phen,&metv,&ws,&wf,&cs,&cf,&ns,&nf,&sprop,&epv,&psn_sun,&psn_shade,&summary))
	{
		printf("ERROR in call to output_map_init() from bgc.c\n");
		errorCode=401;
	}
	
#ifdef DEBUG
	printf("done initialize outmap\n");
#endif
	
	
	/* atmospheric pressure (Pa) as a function of elevation (m) */
	if (!errorCode && atm_pres(sitec.elev, &metv.pa))
	{
		printf("ERROR in atm_pres() from bgc.c\n");
		errorCode=402;
	}
	
#ifdef DEBUG
	printf("done atm_pres\n");
#endif


    /* calculate GSI to deterime onday and offday 	*/	
	if (ctrl.GSI_flag && !PLT.PLT_num)
	{
		
		if (!errorCode && GSI_calculation(&metarr, &sitec, &epc, &phenarr, &ctrl))
		{
			printf("ERROR in call to GSI_calculation(), from bgc.c\n");
			errorCode=403;
		}

		#ifdef DEBUG
			printf("done GSI_calculation\n");
		#endif
	}

	/* calculate conductance limitation factors */	
	if (!errorCode && conduct_limit_factors(bgcout->log_file, &ctrl, &sprop, &epc, &epv))
	{
		printf("ERROR in call to conduct_limit_factors(), from bgc.c\n");
		errorCode=404;
	}

#ifdef DEBUG
	printf("done conduct_limit_factors\n");
#endif


	/* determine phenological signals */
 	if (!errorCode && prephenology(bgcout->log_file, &epc, &metarr, &PLT, &HRV, &ctrl, &phenarr))
	{
		printf("ERROR in call to prephenology(), from bgc.c\n");
		errorCode=405;
	}
	
#ifdef DEBUG
	printf("done prephenology\n");
#endif
	
	/* calculate the annual average air temperature for use in soil temperature corrections */
	tair_annavg = 0.0;
	nmetdays = ctrl.simyears * nDAYS_OF_YEAR;
	for (i=0 ; i<nmetdays ; i++)
	{
		tair_annavg += metarr.tavg[i];
	}
	tair_annavg /= (double)nmetdays;

	
	/* if this simulation is using a restart file for its initialconditions, then copy restart info into structures */
	if (!errorCode && ctrl.read_restart)
	{
		if (!errorCode && restart_input(&ctrl, &epc, &sprop, &sitec, &ws, &cs, &ns, &epv, &(bgcin->restart_input)))
		{
			printf("ERROR in call to restart_input() from bgc.c\n");
			errorCode=406;
		}
		
#ifdef DEBUG
		printf("done restart_input\n");
#endif
	
	}
	

	/* initialize epvar, leaf C and N pools depending on phenology signals for the first metday */
	if (!errorCode && firstday(&ctrl, &sprop, &epc, &PLT,  &sitec, &cinit, &phen, &epv, &cs, &ns, &metv,&psn_sun,&psn_shade))
	{
		printf("ERROR in call to firstday(), from bgc.c\n");
		errorCode=407;
	}
#ifdef DEBUG
		printf("done firstday\n");
#endif		

	/* zero water, carbon, and nitrogen source and sink variables */
	if (!errorCode && zero_srcsnk(&cs,&ns,&ws,&summary))
	{
		printf("ERROR in call to zero_srcsnk(), from bgc.c\n");
		errorCode=408;
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


		/* set the max lai, maturity and flowering variables, for annual diagnostic output */
		epv.annmax_lai = 0.0;
		epv.annmax_rootDepth = 0.0;
		epv.annmax_plantHeight = 0.0;
	
		/* atmospheric CO2 handling */
		if (!(co2.varco2))
		{
			/* constant CO2 */
			metv.co2 = co2.co2ppm;
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
		
		if (ctrl.onscreen) printf("-------------------\n");
		if (ctrl.onscreen) printf("Year: %d\t\n",ctrl.simstartyear+simyr);

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
			if (!errorCode && make_zero_flux_struct(&ctrl, &wf, &cf, &nf))
			{
				printf("ERROR in call to make_zero_flux_struct() from bgc.c\n");
				errorCode=501;
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

			/* set the day index for meteorological and phenological arrays */
			ctrl.metday = simyr*nDAYS_OF_YEAR + yday;
			

			/* nitrogen deposition and fixation */
			nf.ndep_to_sminnTOTAL = daily_ndep;
			nf.nfix_to_sminnTOTAL = epc.nfix / nDAYS_OF_YEAR;


			/* calculating actual onday and offday */
			if (!errorCode && dayphen(&ctrl, &epc, &phenarr, &PLT, &phen))
			{
				printf("ERROR in dayphen() from bgc.c\n");
				errorCode=502;
			}

			/* setting MANAGEMENT DAYS based on input data */
			if (!errorCode && management(&ctrl, &FRZ, &GRZ, &HRV, &MOW, &PLT, &PLG, &THN, &IRG, &gws))
			{
				printf("ERROR in management days() from bgc.c\n");
				errorCode=503;
			}

		
			/* determining soil hydrological parameters  */
 			if (!errorCode && multilayer_hydrolparams(&sitec, &sprop, &ws, &epv))
			{
				printf("ERROR in multilayer_hydrolparams() from bgc.c\n");
				errorCode=504;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_hydrolparams\n",simyr,yday);
#endif
		
		

			/* daily meteorological variables from metarrays */
			if (!errorCode && daymet(&ctrl, &metarr, &epc, &sitec, &metv, &tair_annavg, ws.snoww))
			{
				printf("ERROR in daymet() from bgc.c\n");
				errorCode=505;
			}

			
#ifdef DEBUG
			printf("%d\t%d\tdone daymet\n",simyr,yday);
#endif


			/* phenophases calculation */
			if (!errorCode && phenphase(bgcout->log_file, &ctrl, &epc, &sprop, &PLT, &phen, &metv, &epv, &cs))
			{
				printf("ERROR in phenphase() from bgc.c\n");
				errorCode=506;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone phenphase\n",simyr,yday);
#endif
			
			
			/* soil temperature calculations */
			if (!errorCode && multilayer_tsoil(&epc, &sitec, &sprop, &epv, yday, ws.snoww, &metv))
			{
				printf("ERROR in multilayer_tsoil() from bgc.c\n");
				errorCode=507;
			}
#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_tsoil\n",simyr,yday);
#endif

			/* mulch calculations */
			if (!errorCode && mulch(&sitec, &sprop, &metv, &epv, &cs))
			{
				printf("ERROR in mulch() from bgc.c\n");
				errorCode=547;
			}
#ifdef DEBUG
			printf("%d\t%d\tdone mulch\n",simyr,yday);
#endif


			/* phenology calculation */
			if (!errorCode && phenology(&ctrl, &epc, &cs, &ns, &phen, &metv, &epv, &cf, &nf))
			{
				printf("ERROR in phenology() from bgc.c\n");
				errorCode=508;
			}
#ifdef DEBUG
				printf("%d\t%d\tdone phenology\n",simyr,yday);
#endif

			
			/* calculate leaf area index, sun and shade fractions, and specific leaf area for sun and shade canopy fractions, then calculate canopy radiation interception and transmission */          
			if (!errorCode && radtrans(&ctrl, &phen, &cs, &epc, &sitec, &metv, &epv))
			{
				printf("ERROR in radtrans() from bgc.c\n");
				errorCode=509;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone radtrans\n",simyr,yday);
#endif
			
			
			/* update the annmax LAI/rootingDepth/plantHeight for annual diagnostic output */
			if (epv.proj_lai > epv.annmax_lai)             epv.annmax_lai = epv.proj_lai;
			if (epv.rootdepth > epv.annmax_rootDepth)      epv.annmax_rootDepth = epv.rootdepth;
			if (epv.plant_height > epv.annmax_plantHeight) epv.annmax_plantHeight = epv.plant_height;


			
			/* precip routing (when there is precip) */
			if (!errorCode && metv.prcp && prcp_route(&metv, epc.int_coef, epv.all_lai,	&wf))
			{
				printf("ERROR in prcp_route() from bgc.c\n");
				errorCode=510;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone prcp_route\n",simyr,yday);
#endif
		

			/* snowmelt (when there is a snowpack) */
			if (!errorCode && ws.snoww && snowmelt(&metv, &wf, ws.snoww))
			{
				printf("ERROR in snowmelt() from bgc.c\n");
				errorCode=511;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone snowmelt\n",simyr,yday);
#endif
		
	
	
		/* potential bare-soil evaporation */
		if (!errorCode && potential_evap(&sprop, &metv, &wf))
		{
			printf("ERROR in potential_evap() from bgc.c\n");
			errorCode=512;
		}

#ifdef DEBUG
			printf("%d\t%d\tdone bare_soil evap\n",simyr,yday);
#endif

		/* conductance calculation */
		if (!errorCode && conduct_calc(&ctrl, &metv, &epc, &epv, simyr))
		{
			printf("ERROR in conduct_calc() from bgc.c\n");
			errorCode=513;
		}
			
		#ifdef DEBUG
					printf("%d\t%d\tdone conduct_calc\n",simyr,yday);
		#endif

		/* begin canopy bio-physical process simulation */
		/* do canopy ET calculations whenever there is leaf area displayed, since there may be intercepted water on the canopy that needs to be dealt with */
		if (!errorCode && epv.n_actphen > epc.n_emerg_phenophase && metv.dayl)
		{
	
			/* evapo-transpiration */
			if (!errorCode && cs.leafc && canopy_et(&epc, &metv, &epv, &wf))
			{
				printf("ERROR in canopy_et() from bgc.c\n");
				errorCode=514;
			}
				
#ifdef DEBUG
				printf("%d\t%d\tdone canopy_et\n",simyr,yday);
#endif
			}
			
			
			/* daily maintenance respiration */
			if (!errorCode && maint_resp(&PLT, &cs, &ns, &epc, &metv, &epv, &cf))
			{
				printf("ERROR in m_resp() from bgc.c\n");
				errorCode=515;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone maint resp\n",simyr,yday);
#endif

			

			/* photosynthesis calculation */
			if (!errorCode && cs.leafc && photosynthesis(&epc, &metv, &cs, &ws, &phen, &epv, &psn_sun, &psn_shade, &cf))
			{
				printf("ERROR in photosynthesis() from bgc.c\n");
				errorCode=516;
			}
				
#ifdef DEBUG
			printf("%d\t%d\tdone photosynthesis\n",simyr,yday);
#endif
			
			
			/* daily litter and soil decomp and nitrogen fluxes */
			if (!errorCode && decomp(&metv,&epc, &sprop, &sitec,&cs,&ns,&epv,&cf,&nf,&nt))
			{
				printf("ERROR in decomp() from bgc.c\n");
				errorCode=517;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone decomp\n",simyr,yday);
#endif

		

			/* Daily allocation gets called whether or not this is a current growth day, because the competition between decomp immobilization fluxes 
			and plant growth N demand is resolved here.  On days with no growth, no allocation occurs, but immobilization fluxes are updated normally */

			if (!errorCode && daily_allocation(&epc,&sitec,&sprop,&metv,&cs,&ns,&cf,&nf,&epv,&nt,0))
			{
				printf("ERROR in daily_allocation() from bgc.c\n");
				errorCode=518;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone daily_allocation\n",simyr,yday);
#endif
		
			/* heat stress during flowering can affect daily allocation of fruit */
			if (epc.n_flowHS_phenophase > 0)
			{
				if (!errorCode && flowering_heatstress(&epc, &metv, &epv, &cf, &nf))
				{
					printf("ERROR in flowering_heatstress() from bgc.c\n");
					errorCode=519;
				}
			}

			/* reassess the annual turnover rates for livewood --> deadwood, and for evergreen leaf and fine root litterfall. 
			This happens once each year, on the annual_alloc day (the last litterfall day - test for annual allocation day) */
			
			if (phen.remdays_litfall == 1) 
				annual_alloc = 1;
			else 
				annual_alloc = 0;
		

			/* litterfall rates once a year */
			if (!errorCode && annual_alloc)
			{
				if (!errorCode && annual_rates(&epc,&epv))
				{
					printf("ERROR in annual_rates() from bgc.c\n");
					errorCode=520;
				}
				
#ifdef DEBUG
				printf("%d\t%d\tdone annual rates\n",simyr,yday);
#endif
			} 


			/* daily growth respiration */
			if (!errorCode && growth_resp(&epc, &cf))
			{
				printf("ERROR in daily_growth_resp() from bgc.c\n");
				errorCode=521;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone growth_resp\n",simyr,yday);
#endif

			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
			/* 3. WATER CALCULATIONS WITH STATE UPDATE */

			/* IRRIGATING separately from other management routines*/
			if (!errorCode && irrigating(&ctrl, &IRG, &epv, &ws, &wf))
			{
				printf("ERROR in irrigating() from bgc.c\n");
				errorCode=522;
			}

			
	    	/* multilayer soil hydrology: percolation calculation based on PRCP, RUNOFF, EVAP, TRANSP */
			if (!errorCode && multilayer_hydrolprocess(&ctrl, &sitec, &sprop, &epc,  &epv, &ws, &wf, &gws))
			{
				printf("ERROR in multilayer_hydrolprocess() from bgc.c\n");
				errorCode=524;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_hydrolprocess\n",simyr,yday);
#endif	
		
		
			
 
			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
			/* 4. STATE UPDATE */

			/* daily update of the water state variables */
			if (!errorCode && daily_water_state_update(&epc, &wf, &ws))
			{
				printf("ERROR in daily_water_state_update() from bgc.c\n");
				errorCode=525;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone water state update\n",simyr,yday);
#endif

		
			

			/* daily update of carbon and nitrogen state variables */
			if (!errorCode && daily_CN_state_update(&sitec, &epc, &ctrl, &epv, &cf, &nf, &cs, &ns, annual_alloc, epc.evergreen))
			{
				printf("ERROR in daily_CN_state_update() from bgc.c\n");
				errorCode=526;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone CN state update\n",simyr,yday);
#endif

		       
			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
			/* 5. MORTALITY AND NITROGEN FLUXES CALCULATION WITH OWN STATE UPDATE: 
			to insure that pools don't go negative due to mortality/leaching fluxes conflicting with other proportional fluxes */

		
			/* calculate daily senescence mortality fluxes and update state variables */
			if (!errorCode && senescence(&sitec, &epc, &GRZ, &metv, &ctrl, &cs, &cf, &ns, &nf, &epv))
			{
				printf("ERROR in senescence() from bgc.c\n");
				errorCode=527;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone senescence\n",simyr,yday);
#endif
		    /* calculate daily mortality fluxes  and update state variables */
			if (!errorCode && mortality(&ctrl, &sitec, &epc, &epv, &cs, &cf, &ns, &nf, simyr))
			{
				printf("ERROR in mortality() from bgc.c\n");
				errorCode=528;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone mortality\n",simyr,yday);
#endif

			
			/* calculate the change of soil mineralized N in multilayer soil */ 
			if (!errorCode && multilayer_sminn(&ctrl, &metv,&sprop, &sitec, &cf, &epv, &ns, &nf))
			{
				printf("ERROR in multilayer_sminn() from bgc.c\n");
				errorCode=529;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_sminn\n",simyr,yday);
#endif

			/* calculate the leaching of N, DOC and DON from multilayer soil */
			if (!errorCode && multilayer_leaching(&sprop, &epv, &ctrl, &cs, &cf, &ns, &nf, &ws, &wf))
			{
				printf("ERROR in multilayer_leaching() from bgc.c\n");
				errorCode=530;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_leaching\n",simyr,yday);
#endif


		
	
			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
			/* 6. MANAGEMENT FLUXES */
			
			/* PLANTING */
			if (!errorCode && planting(&ctrl, &sitec, &PLT, &epc, &epv, &phen, &cs, &ns, &cf, &nf))
			{
				printf("ERROR in planting() from bgc.c\n");
				errorCode=531;
			}


		   	/* THINNIG  */
			if (!errorCode && thinning(&ctrl, &epc, &THN, &cs, &ns, &ws, &cf, &nf, &wf))
			{
				printf("ERROR in thinning() from bgc.c\n");
				errorCode=532;
			}

			/* MOWING  */
			if (!errorCode && mowing(&ctrl, &epc, &MOW, &epv, &cs, &ns, &ws, &cf, &nf, &wf))
			{
				printf("ERROR in mowing() from bgc.c\n");
				errorCode=533;
			}

			/* GRAZING  */
			if (!errorCode && grazing(&ctrl, &epc, &sitec, &GRZ, &epv, &cs, &ns, &ws, &cf, &nf, &wf))
			{
				printf("ERROR in grazing() from bgc.c\n");
				errorCode=534;
			}
	
		   	/* HARVESTING  */
			if (!errorCode && harvesting(bgcout->econout_file, &ctrl, &phen, &epc, &HRV, &IRG, &epv, &cs, &ns, &ws, &cf, &nf, &wf))
			{
				printf("ERROR in harvesting() from bgc.c\n");
				errorCode=535;
			}
 
			/* PLOUGHING */
 			if (!errorCode && ploughing(&ctrl, &epc, &sitec, &sprop, &metv, &epv, &PLG, &cs, &ns, &ws, &cf, &nf, &wf))
			{
				printf("ERROR in ploughing() from bgc.c\n");
				errorCode=536;
			}
		 
			/* FERTILIZING  */
	    	if (!errorCode && fertilizing(&ctrl, &sitec, &sprop, &FRZ, &epv, &cs, &ns, &ws, &cf, &nf, &wf))
			{
				printf("ERROR in fertilizing() from bgc.c\n");
				errorCode=537;
			}	
				
			/* cut-down plant material (due to management) */
			if (!errorCode && cutdown2litter(&sitec, &epc, &epv, &cs, &cf, &ns, &nf))
			{
				printf("ERROR in cutdown2litter() from bgc.c\n");
				errorCode=539;
			}


			/* calculating rooting depth, n_rootlayers, n_maxrootlayers, rootlength_prop */
 			 if (!errorCode && multilayer_rootdepth(&epc, &sprop, &cs, &sitec, &epv))
			 {
				printf("ERROR in multilayer_rootdepth() from bgc.c\n");
				errorCode=540;
			 }
#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_rootdepth\n",simyr,yday);
#endif
		
			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
			/* 7. ERROR CHECKING AND SUMMARY VARIABLES  */
			
			/* test for very low state variable values and force them to 0.0 to avoid rounding and floating point overflow errors */
			if (!errorCode && precision_control(&ws, &cs, &ns))
			{
				printf("ERROR in call to precision_control() from bgc.c\n");
				errorCode=541;
			} 
				
#ifdef DEBUG
				printf("%d\t%d\tdone precision_control\n",simyr,yday);
#endif

			/* test for water balance*/
 			if (!errorCode && check_water_balance(&ws, first_balance))
			{
				printf("ERROR in check_water_balance() from bgc.c\n");
				errorCode=542;
			}
			 
#ifdef DEBUG
			printf("%d\t%d\tdone water balance\n",simyr,yday);
#endif
			
	
	        /* test for carbon balance */
			if (!errorCode && check_carbon_balance(&cs, first_balance))
			{
				printf("ERROR in check_carbon_balance() from bgc.c\n");
				errorCode=543;
			}
	
		
			
#ifdef DEBUG
			printf("%d\t%d\tdone carbon balance\n",simyr,yday); 
#endif


			/* test for nitrogen balance */
			if (!errorCode && check_nitrogen_balance(&ns, first_balance))
			{
				printf("ERROR in check_nitrogen_balance() from bgc.c\n");
				errorCode=544;
			}
			
			
#ifdef DEBUG
			printf("%d\t%d\tdone nitrogen balance\n",simyr,yday);
#endif
	

			/* calculate summary variables */
			if (!errorCode && cnw_summary(yday, &epc, &sitec, &sprop, &metv, &cs, &cf, &ns, &nf, &wf, &epv, &summary))
			{
				printf("ERROR in cnw_summary() from bgc.c\n");
				errorCode=545;
			}
	
	
#ifdef DEBUG
			printf("%d\t%d\tdone carbon summary\n",simyr,yday); 
#endif

		
			/* output handling */
			if (!errorCode && output_handling(mondays[curmonth], endday[curmonth], &ctrl, output_map, dayarr, monavgarr, annavgarr, annarr, 
				                            bgcout->dayout, bgcout->monavgout, bgcout->annavgout, bgcout->annout))
			{
				printf("ERROR in output_handling() from bgc.c\n");
				errorCode=546;
			}

	
#ifdef DEBUG
			printf("%d\t%d\tdoneoutput_handling\n",simyr,yday); 
#endif
			/*  if no dormant period (e.g. evergreen): last day is the dormant day */
			if (phen.offday - phen.onday == 364 && phen.offday == phen.yday_total) 
			{
				epv.n_actphen = 0;
				phen.onday = -1;
				phen.offday = -1;
				phen.remdays_litfall =-1;
			}

		
			/* at the end of first day of simulation, turn off the first_balance switch */
			if (first_balance) first_balance = 0;

			/* if this is the last day of the current month: increment current month counter */
			if (yday == endday[curmonth]) curmonth++;

		}   /* end of daily model loop */
		

	}   /* end of annual model loop */


	/********************************************************************************************************* */
	/* wrinting log file */
	
	/* most important informations - onscreen */
	if (ctrl.onscreen) 
	{
		printf("\n");
		printf("INFORMATION FROM THE LAST SIMULATION YEAR\n");
		printf("Maximum projected LAI [m2/m2]:                 %12.1f\n",epv.annmax_lai);
		printf("Total litter and cwdc carbon content [kgC/m2]: %12.1f\n",summary.litrCwdC_total);
		printf("Total soil carbon content [kgC/m2]:            %12.1f\n",summary.soilC_total);
	}

	if (cs.CbalanceERR != 0) CbalanceERR = log10(cs.CbalanceERR);
	if (ns.NbalanceERR != 0) NbalanceERR = log10(ns.NbalanceERR);
	if (ws.WbalanceERR != 0) WbalanceERR = log10(ws.WbalanceERR);
	if (cs.CNratioERR  != 0) CNratioERR  = log10(cs.CNratioERR);

	fprintf(bgcout->log_file.ptr, " \n");
	fprintf(bgcout->log_file.ptr, " \n");
	fprintf(bgcout->log_file.ptr, "SOME IMPORTANT ANNUAL OUTPUTS FROM LAST SIMULATION YEAR\n");
    fprintf(bgcout->log_file.ptr, "Cumulative sum of GPP [gC/m2/year]:                      %12.1f\n",summary.cum_gpp*1000);
	fprintf(bgcout->log_file.ptr, "Cumulative sum of NEE [gC/m2/year]:                      %12.1f\n",summary.cum_nee*1000);
	fprintf(bgcout->log_file.ptr, "Cumulative sum of ET  [kgH2O/m2/year]:                   %12.1f\n",summary.cum_ET);
	fprintf(bgcout->log_file.ptr, "Cumulative sum of soilEvap [kgH2O/m2/year):              %12.1f\n",summary.cum_evap);
	fprintf(bgcout->log_file.ptr, "Cumulative sum of TRANSP   [kgH2O/m2/year):              %12.1f\n",summary.cum_transp);
	fprintf(bgcout->log_file.ptr, "Cumulative sum of N2O flux [gN/m2/year]:                 %12.2f\n",summary.cum_n2o*1000);
	fprintf(bgcout->log_file.ptr, "Maximum projected LAI [m2/m2]:                           %12.2f\n",epv.annmax_lai);
	fprintf(bgcout->log_file.ptr, "Soil carbon content (in 0-30 cm soil layer) [%%]:         %12.2f\n",summary.SOM_C_top30);
	fprintf(bgcout->log_file.ptr, "Total soil carbon content [kgC/m2/year]:                 %12.2f\n",summary.soilC_total);
	fprintf(bgcout->log_file.ptr, "Total litter and cwdc carbon content [kgC/m2/year]:      %12.2f\n",summary.litrCwdC_total);
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
		!ctrl.limitSNSC_flag && !ctrl.limitMR_flag && !ctrl.notransp_flag && !ctrl.noMR_flag && !ctrl.pond_flag && !ctrl.grazingW_flag && 
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

	
	/* 8. RESTART OUTPUT HANDLING */
	/* if write_restart flag is set, copy data to the output restart struct */
	if (!errorCode && ctrl.write_restart)
	{
		if (restart_output( &ws, &cs, &ns, &epv, &(bgcout->restart_output)))
		{
			printf("ERROR in call to restart_output() from bgc.c\n");
			errorCode=600;
		}
		
#ifdef DEBUG
		printf("%d\t%d\tdone restart output\n",simyr,yday);
#endif
	}

	
	/* free memory for local output arrays  */
	
    if ((errorCode == 0 || errorCode > 301) && ctrl.dodaily) free(dayarr);
	if ((errorCode == 0 || errorCode > 302) && ctrl.domonavg) free(monavgarr);
	if ((errorCode == 0 || errorCode > 303) && ctrl.doannavg) free(annavgarr);
	if ((errorCode == 0 || errorCode > 304) && ctrl.doannual) free(annarr); 
	if ((errorCode == 0 || errorCode > 305)) free(output_map);
	if (((errorCode == 0 ||errorCode > 306) && !ctrl.GSI_flag) || ((errorCode == 0 || errorCode > 405) && ctrl.GSI_flag)) 
	{
		free(phenarr.onday_arr);
		free(phenarr.offday_arr);
		if (ctrl.GSI_flag)
		{
			free(phenarr.tmin_index);
			free(phenarr.vpd_index);
			free(phenarr.heatsum_index);
			free(phenarr.dayl_index);
			free(phenarr.gsi_indexAVG);
			free(phenarr.heatsum);

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
