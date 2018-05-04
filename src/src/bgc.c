/*
bgc.c
Core BGC model logic

Includes in-line output handling routines that write to daily and annual
output files. This is the only library module that has external
I/O connections, and so it is the only module that includes bgc_io.h.

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2018, D. Hidy [dori.hidy@gmail.com]
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
	wflux_struct       wf;
	cinit_struct       cinit;
	cstate_struct      cs;
	cflux_struct       cf;
	nstate_struct      ns;
	nflux_struct       nf;


	/* primary ecophysiological variables */
	epvar_struct       epv;

	/* planting variables */
	planting_struct       PLT;

	/* thinning variables */
	thinning_struct     THN;

	/* mowing variables */
	mowing_struct       MOW;

	/* grazing variables */
	grazing_struct       GRZ;

	/* harvesting variables */
	harvesting_struct       HRV;

	/* ploughing variables */
	ploughing_struct      PLG;

	/* fertilizing variables */
	fertilizing_struct       FRZ;

	/* irrigation variables */
	irrigation_struct       IRG;

	/* GSI variables */
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
	double *dayarr = 0;
	double *monavgarr = 0;
	double *annavgarr = 0;
	double *annarr = 0;



	/* miscelaneous variables for program control in main */
	int simyr = 0;
	int yday  = 0;
	int metyr = 0;
	int metday= 0;
	int i     = 0;
	int first_balance;
	int annual_alloc;
    int outv;
	double daily_ndep; 
	double tair_annavg;
	double nmetdays;

	double CbalanceERR = 0;
	double NbalanceERR = 0;
	double WbalanceERR = 0;
	double CNerror = 0;

    /* variables used for monthly average output */
	int curmonth;
	int mondays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	int endday[12] = {30,58,89,119,150,180,211,242,272,303,333,364};
	

	/* copy the input structures into local structures */
	ws = bgcin->ws;
	cinit = bgcin->cinit;
	cs = bgcin->cs;
	ns = bgcin->ns;
	sitec = bgcin->sitec;
	epc = bgcin->epc;
	PLT = bgcin->PLT; 		/* planting variables */
	THN = bgcin->THN; 		/* thinning variables */
	MOW = bgcin->MOW; 		/* mowing variables */
	GRZ = bgcin->GRZ; 		/* grazing variables */
	HRV = bgcin->HRV;		/* harvesting variables */
	PLG = bgcin->PLG;		/* harvesting variables */
	FRZ = bgcin->FRZ;		/* fertilizing variables */
	IRG = bgcin->IRG; 		/* irrigation variables */
	GSI = bgcin->GSI;		/* GSI variables  */


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
		if (PLT.PLT_flag)
		{
			fprintf(bgcout->log_file.ptr, "SGS data - planting date from planting file\n");
		}
		else
		{
			if (ctrl.varSGS_flag == 0) 
				fprintf(bgcout->log_file.ptr, "SGS data - constant\n");
			else
			{
				fprintf(bgcout->log_file.ptr, "SGS data - annual varying\n");
				printf("INFORMATION: reading onday_normal.txt: annual varying SGS data\n");
			}
		}
		if (HRV.HRV_flag)
		{
			fprintf(bgcout->log_file.ptr, "EGS data - harvesting date from harvesting file\n");
		}
		else
		{
			if (ctrl.varEGS_flag == 0) 
				fprintf(bgcout->log_file.ptr, "EGS data - constant\n");
			else
			{
				fprintf(bgcout->log_file.ptr, "EGS data - annual varying\n");
				printf("INFORMATION: reading onday_normal.txt: annual varying SGS data\n");
			}
		}
	}
	else
	{
		if (ctrl.GSI_flag == 0) 
		{
			if (PLT.PLT_flag) 
				fprintf(bgcout->log_file.ptr, "SGS data - planting date from planting file\n");
			else
				fprintf(bgcout->log_file.ptr, "SGS data - original method\n");
			
			if (HRV.HRV_flag)
				fprintf(bgcout->log_file.ptr, "EGS data - harvesting date from harvesting file\n");
			else
				fprintf(bgcout->log_file.ptr, "EGS data - original method\n");
		}
		else
		{
			if (PLT.PLT_flag) 
				fprintf(bgcout->log_file.ptr, "SGS data - planting date from planting file\n");
			else
				fprintf(bgcout->log_file.ptr, "SGS data - GSI method\n");
			
			if (HRV.HRV_flag)
				fprintf(bgcout->log_file.ptr, "EGS data - harvesting date from harvesting file\n");
			else
				fprintf(bgcout->log_file.ptr, "EGS data - GSI method\n");
		}
	}

	if (ctrl.varWPM_flag == 0) 
		fprintf(bgcout->log_file.ptr, "WPM data - constant \n");
	else
	{
		fprintf(bgcout->log_file.ptr, "WPM data - annual varying\n");
		printf("INFORMATION: reading mortality_normal.txt - annual varying WPM data\n");
	}

	if (ctrl.varMSC_flag == 0) 
		fprintf(bgcout->log_file.ptr, "MSC data - constant\n");
	else
	{
		fprintf(bgcout->log_file.ptr, "MSC data - annual varying\n");
		printf("INFORMATION: reading conductance_normal.txt: annual varying MSC data\n");
	}

	if (ctrl.FRZ_flag || ctrl.GRZ_flag || ctrl.MOW_flag || ctrl.THN_flag || ctrl.PLG_flag || ctrl.PLT_flag || ctrl.IRG_flag || ctrl.HRV_flag)
		fprintf(bgcout->log_file.ptr, "management  - YES\n");
	else
		fprintf(bgcout->log_file.ptr, "management  - NO\n");

	if (ctrl.GWD_flag == 0) 
		fprintf(bgcout->log_file.ptr, "groundwater - NO\n");
	else
	{
		fprintf(bgcout->log_file.ptr, "groundwater - YES\n");
		printf("INFORMATION: reading groundwater_normal.txt - annual varying GWD data\n");
	}

	fprintf(bgcout->log_file.ptr, " \n");
	/********************************************************************************************************* */
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	
	/* allocate memory for local output arrays */
	if (ok && (ctrl.dodaily || ctrl.domonavg || ctrl.doannavg)) 
	{
		dayarr = (double*) malloc(ctrl.ndayout * sizeof(double));
		if (!dayarr)
		{
			printf("Error allocating for local daily output array in bgc()\n");
			ok=0;
		}
	}

	if (ok && ctrl.domonavg) 
	{
		monavgarr = (double*) malloc(ctrl.ndayout * sizeof(double));
		if (!monavgarr)
		{
			printf("Error allocating for monthly average output array in bgc()\n");
			ok=0;
		}
	}
	if (ok && ctrl.doannavg) 
	{
		annavgarr = (double*) malloc(ctrl.ndayout * sizeof(double));
		if (!annavgarr)
		{
			printf("Error allocating for annual average output array in bgc()\n");
			ok=0;
		}
	}
	if (ok && ctrl.doannual)
	{
		annarr = (double*) malloc(ctrl.nannout * sizeof(double));
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
	if (ok && output_map_init(output_map,&phen,&metv,&ws,&wf,&cs,&cf,&ns,&nf,&epv,&psn_sun,&psn_shade,&summary))
	{
		printf("Error in call to output_map_init() from bgc()\n");
		ok=0;
	}
	
#ifdef DEBUG
	printf("done initialize outmap\n");
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


    /* calculate GSI to deterime onday and offday 	*/	
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

	/* calculate conductance limitation factors */	
	if (ok && conduct_limit_factors(bgcout->log_file, &ctrl, &sitec, &epc, &epv))
	{
		printf("Error in call to conduct_limit_factors(), from bgc()\n");
		ok=0;
	}

#ifdef DEBUG
	printf("done conduct_limit_factors\n");
#endif


	/* determine phenological signals */
 	if (ok && prephenology(bgcout->log_file, &PLT, &HRV, &epc, &sitec, &metarr, &ctrl, &phenarr))
	{
		printf("Error in call to prephenology(), from bgc()\n");
		ok=0;
	}
	
#ifdef DEBUG
	printf("done prephenology\n");
#endif
	
	/* calculate the annual average air temperature for use in soil temperature corrections */
	tair_annavg = 0.0;
	nmetdays = ctrl.metyears * NDAYS_OF_YEAR;
	for (i=0 ; i<nmetdays ; i++)
	{
		tair_annavg += metarr.tavg[i];
	}
	tair_annavg /= (double)nmetdays;

	
	/* if this simulation is using a restart file for its initialconditions, then copy restart info into structures */
	if (ok && ctrl.read_restart)
	{
		if (ok && restart_input(&epc, &ws, &cs, &ns, &epv, &(bgcin->restart_input)))
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
		/* initial value for metyr */
		metyr = 0;
		

	}

	/* initialize leaf C and N pools depending on phenology signals for the first metday */
	if (ok && firstday(&ctrl, &sitec, &epc, &cinit, &phen, &epv, &cs, &ns, &metv))
	{
		printf("Error in call to firstday(), from bgc()\n");
		ok=0;
	}
#ifdef DEBUG
		printf("done firstday\n");
#endif		

	/* zero water, carbon, and nitrogen source and sink variables */
	if (ok && zero_srcsnk(&cs,&ns,&ws,&summary))
	{
		printf("Error in call to zero_srcsnk(), from bgc()\n");
		ok=0;
	}

#ifdef DEBUG
	printf("done zero_srcsnk\n");
#endif
	

	
	/* initialize the indicator for first day of current simulation, so that the checks for mass balance can have two days for comparison */
	first_balance = 1;
		
	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
	/* 1. BEGIN OF THE ANNUAL LOOP */

	for (simyr=0 ; ok && simyr<ctrl.simyears ; simyr++)
	{

		/* set current month to 0 (january) at the beginning of each year */
		curmonth = 0;

		/* test whether metyr needs to be reset */
		if (metyr == ctrl.metyears)
		{
			if (ctrl.onscreen) printf("Resetting met data for cyclic input\n");
			metyr = 0;
		}

		/* output to screen to indicate start of simulation year */
        if (ctrl.onscreen) printf("Year: %6d\n",ctrl.simstartyear+simyr);

		/* set the max lai, maturity and flowering variables, for annual diagnostic output */
		epv.ytd_maxplai = 0.0;
	
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
			daily_ndep = ndep.ndep / NDAYS_OF_YEAR;
		}
		else
		{	
			/* Ndep from file */
			daily_ndep = ndep.ndep_array[simyr] / NDAYS_OF_YEAR;
		
		}
		
		if (ctrl.onscreen) printf("-------------------\n");
		if (ctrl.onscreen) printf("Year: %d\t\n",ctrl.simstartyear+simyr);

		/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
		/* 2. BEGIN OF THE DAILY LOOP */
		
		for (yday=0 ; ok && yday<NDAYS_OF_YEAR ; yday++)
		{

#ifdef DEBUG
			printf("year %d\tyday %d\n",simyr,yday);
#endif

			
			/* counters into control sturct */ 
			ctrl.simyr = simyr;
			ctrl.metyr = metyr;
			ctrl.yday  = yday;
			ctrl.spinyears = 0;

			/* set fluxes to zero */
			if (ok && make_zero_flux_struct(&wf, &cf, &nf))
			{
				printf("Error in call to make_zero_flux_struct() from bgc()\n");
				ok=0;
			}

#ifdef DEBUG
	printf("done make_zero_flux\n");
#endif
	
			
			/* set the day index for meteorological and phenological arrays */
			metday = metyr*NDAYS_OF_YEAR + yday;
			
			
			/* nitrogen deposition and fixation */
			nf.ndep_to_sminn = daily_ndep;
			nf.nfix_to_sminn = epc.nfix / NDAYS_OF_YEAR;


			/* actual onday and offday */
			if (ok && dayphen(&ctrl, &epc, &phenarr, &phen))
			{
				printf("Error in dayphen from bgc()\n");
				ok=0;
			}
		
	
			/* setting MANAGEMENT DAYS based on input data */
			if (ok && management(&ctrl, &FRZ, &GRZ, &HRV, &MOW, &PLT, &PLG, &THN, &IRG))
			{
				printf("Error in management days() from bgc()\n");
				ok=0;
			}

			/* determining soil hydrological parameters  */
 			if (ok && multilayer_hydrolparams(&sitec, &ws, &epv))
			{
				printf("Error in multilayer_hydrolparams() from bgc()\n");
				ok=0;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_hydrolparams\n",simyr,yday);
#endif


			/* daily meteorological variables from metarrays */
			if (ok && daymet(&metarr, &sitec, &metv, &tair_annavg, ws.snoww, metday))
			{
				printf("Error in daymet() from bgc()\n");
				ok=0;
			}

			
#ifdef DEBUG
			printf("%d\t%d\tdone daymet\n",simyr,yday);
#endif


			/* phenophases calculation */
			if (ok && phenphase(&ctrl, &epc, &sitec, &phen, &metv, &epv, &cs))
			{
				printf("Error in phenphase() from bgc()\n");
				ok=0;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone phenphase\n",simyr,yday);
#endif

			
			/* soil temperature calculations */
			if (ok && multilayer_tsoil(&epc, &sitec, &epv, yday, ws.snoww, &metv))
			{
				printf("Error in multilayer_tsoil() from bgc()\n");
				ok=0;
			}
#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_tsoil\n",simyr,yday);
#endif


			/* phenology calculation */
			if (ok && phenology(&ctrl, &epc, &cs, &ns, &phen, &metv, &epv, &cf, &nf))
			{
				printf("Error in phenology() from bgc()\n");
				ok=0;
			}
#ifdef DEBUG
				printf("%d\t%d\tdone phenology\n",simyr,yday);
#endif

			

			/* calculating rooting depth, n_rootlayers, n_maxrootlayers, rootlength_prop */
 			 if (ok && multilayer_rootdepth(&ctrl, &phen, &epc, &sitec, &cs, &epv))
			 {
				printf("Error in multilayer_rootdepth() from bgc()\n");
				ok=0;
			 }
#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_rootdepth\n",simyr,yday);
#endif
		
			
			/* calculate leaf area index, sun and shade fractions, and specific leaf area for sun and shade canopy fractions, then calculate canopy radiation interception and transmission */
                        
			if (ok && radtrans(&ctrl, &phen, &cs, &epc, &sitec, &metv, &epv))
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
		
		
		     /* conductance calculation */
			if (ok && conduct_calc(&ctrl, &metv, &epc, &sitec, &epv, simyr))
			{
				printf("Error in conduct_calc() from bgc()\n");
				ok=0;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone conduct_calc\n",simyr,yday);
#endif


		
			/* bare-soil evaporation */
			if (ok && baresoil_evap(&epc, &metv, &wf, &epv.dsr))
			{
				printf("Error in baresoil_evap() from bgc()\n");
				ok=0;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone bare_soil evap\n",simyr,yday);
#endif


			/* begin canopy bio-physical process simulation */
			/* do canopy ET calculations whenever there is leaf area displayed, since there may be intercepted water on the canopy that needs to be dealt with */
			if (ok && epv.n_actphen >= epc.n_emerg_phenophase && cs.leafc && metv.dayl)
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
			
			/* daily maintenance respiration */
			if (ok && maint_resp(&cs, &ns, &epc, &metv, &epv, &cf))
			{
				printf("Error in m_resp() from bgc()\n");
				ok=0;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone maint resp\n",simyr,yday);
#endif


	
			/* do photosynthesis only when it is part of the current growth season, as defined by the remdays_curgrowth flag.  
			This keeps the occurrence of new growth consistent with the treatment of litterfall and allocation */
			if (ok && epv.n_actphen >= epc.n_emerg_phenophase && cs.leafc && phen.remdays_curgrowth && metv.dayl && ws.snoww <= GSI.snowcover_limit)
			{		
				if (ok && photosynthesis(&epc, &metv, &epv, &psn_sun, &psn_shade, &cf))
				{
					printf("Error in photosynthesis() from bgc()\n");
					ok=0;
				}
				
#ifdef DEBUG
				printf("%d\t%d\tdone photosynthesis\n",simyr,yday);
#endif
			} 
			


			/* daily litter and soil decomp and nitrogen fluxes */
			if (ok && decomp(&metv,&epc,&sitec,&cs,&ns,&epv,&cf,&nf,&nt))
			{
				printf("Error in decomp() from bgc.c\n");
				ok=0;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone decomp\n",simyr,yday);
#endif

			/* Daily allocation gets called whether or not this is a current growth day, because the competition between decomp immobilization fluxes 
			and plant growth N demand is resolved here.  On days with no growth, no allocation occurs, but immobilization fluxes are updated normally */

			if (ok && daily_allocation(&epc,&metv,&cs,&ns,&cf,&nf,&epv,&nt,0))
			{
				printf("Error in daily_allocation() from bgc.c\n");
				ok=0;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone daily_allocation\n",simyr,yday);
#endif
		
			/* heat stress during flowering can affect daily allocation of fruit */
			if (epc.n_flowHS_phenophase != DATA_GAP)
			{
				if (ok && flowering_heatstress(&epc, &metv, &epv, &cf, &nf))
				{
					printf("Error in flowering_heatstress() from bgc()\n");
					ok=0;
				}
			}

			/* reassess the annual turnover rates for livewood --> deadwood, and for evergreen leaf and fine root litterfall. 
			This happens once each year, on the annual_alloc day (the last litterfall day - test for annual allocation day) */
			
			if (phen.remdays_litfall == 1) 
				annual_alloc = 1;
			else 
				annual_alloc = 0;
		

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

			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
			/* 3. WATER CALCULATIONS WITH STATE UPDATE */

			/* calculate the part-transpiration from total transpiration */
			if (ok && multilayer_transpiration(&ctrl, &sitec, &epv, &ws, &wf))
			{
				printf("Error in multilayer_transpiration() from bgc()\n");
				ok=0;
			}
#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_transpiration\n",simyr,yday);
#endif
		

			/* IRRIGATION separately from other management routines*/
			if (ok && irrigation(&ctrl, &IRG, &wf))
			{
				printf("Error in irrigation() from bgc()\n");
				ok=0;
			}
		
		    /* ground water calculation */
			if (ok && groundwater(&sitec, &ctrl, &epv, &ws, &wf))
			{
				printf("Error in groundwater() from bgc()\n");
				ok=0;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone groundwater\n",simyr,yday);
#endif	
			
	
			/* multilayer soil hydrology: percolation calculation based on PRCP, RUNOFF, EVAP, TRANS */
			if (ok && multilayer_hydrolprocess(&ctrl, &sitec, &epc, &epv, &ws, &wf))
			{
				printf("Error in multilayer_hydrolprocess() from bgc()\n");
				ok=0;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_hydrolprocess\n",simyr,yday);
#endif	
		
 
			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
			/* 4. STATE UPDATE */

			/* daily update of the water state variables */
			if (ok && daily_water_state_update(&wf, &ws))
			{
				printf("Error in daily_water_state_update() from bgc()\n");
				ok=0;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone water state update\n",simyr,yday);
#endif


			/* daily update of carbon and nitrogen state variables */
			if (ok && daily_CN_state_update(&epc, &ctrl, &epv, &cf, &nf, &cs, &ns, annual_alloc, epc.woody, epc.evergreen))
			{
				printf("Error in daily_CN_state_update() from bgc()\n");
				ok=0;
			}

#ifdef DEBUG
			printf("%d\t%d\tdone CN state update\n",simyr,yday);
#endif

			
		       
			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
			/* 5. MORTALITY AND NITROGEN FLUXES CALCULATION WITH OWN STATE UPDATE: to insure that pools don't go negative due to mortality/leaching fluxes conflicting with other proportional fluxes

			/* calculate daily senescence mortality fluxes and update state variables */
			if (ok && senescence(yday, &epc, &GRZ, &metv, &phen, &cs, &cf, &ns, &nf, &epv))
			{
				printf("Error in senescence() from bgc()\n");
				ok=0;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone senescence\n",simyr,yday);
#endif
		    /* calculate daily mortality fluxes  and update state variables */
			if (ok && mortality(&ctrl, &epc, &epv, &cs, &cf, &ns, &nf, simyr))
			{
				printf("Error in mortality() from bgc()\n");
				ok=0;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone mortality\n",simyr,yday);
#endif

			
			/* calculate the change of soil mineralized N in multilayer soil */ 
			if (ok && multilayer_sminn(&epc, &epv, &sitec, &cf, &ns, &nf))
			{
				printf("Error in multilayer_sminn() from bgc()\n");
				ok=0;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_sminn\n",simyr,yday);
#endif

			/* calculate the leaching of N, DOC and DON from multilayer soil */
			if (ok && multilayer_leaching(&epc, &epv, &cs, &cf, &ns, &nf, &ws, &wf))
			{
				printf("Error in multilayer_leaching() from bgc()\n");
				ok=0;
			}
			
#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_leaching\n",simyr,yday);
#endif



			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
			/* 6. MANAGEMENT FLUXES */
			
			/* PLANTING */
			if (ok && planting(&ctrl, &epc, &sitec, &PLT, &epv, &phen, &cs, &ns, &cf, &nf))
			{
				printf("Error in planting() from bgc()\n");
				ok=0;
			}


		   	/* THINNIG  */
			if (ok && thinning(&ctrl, &epc, &THN, &cs, &ns, &ws, &cf, &nf, &wf))
			{
				printf("Error in thinning() from bgc()\n");
				ok=0;
			}

			/* MOWING  */
			if (ok && mowing(&ctrl, &epc, &MOW, &epv, &cs, &ns, &ws, &cf, &nf, &wf))
			{
				printf("Error in mowing() from bgc()\n");
				ok=0;
			}

			/* GRAZING  */
			if (ok && grazing(&ctrl, &epc, &GRZ, &cs, &ns, &ws, &cf, &nf, &wf))
			{
				printf("Error in grazing() from bgc()\n");
				ok=0;
			}
	
		   	/* HARVESTING  */
			if (ok && harvesting(&ctrl, &epc, &HRV, &epv, &cs, &ns, &ws, &cf, &nf, &wf))
			{
				printf("Error in harvesting() from bgc()\n");
				ok=0;
			}
 
			/* PLOUGHING */
 			if (ok && ploughing(&ctrl, &epc, &sitec, &metv, &epv, &PLG, &cs, &ns, &ws, &cf, &nf, &wf))
			{
				printf("Error in ploughing() from bgc()\n");
				ok=0;
			}
		 
			/* FERTILIZING  */
	    	if (ok && fertilizing(&ctrl, &sitec, &FRZ, &epv, &cs, &ns, &ws, &cf, &nf, &wf))
			{
				printf("Error in fertilizing() from bgc()\n");
				ok=0;
			}	
				
			/* cut-down plant material (due to management) */
			if (ok && cutdown2litter(&epc, &epv, &cs, &cf, &ns, &nf))
			{
				printf("Error in cutdown2litter() from bgc()\n");
				ok=0;
			}

			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
			/* 7. ERROR CHECKING AND SUMMARY VARIABLES  */
			
			/* test for very low state variable values and force them to 0.0 to avoid rounding and floating point overflow errors */
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
				ok=0;
			}
			 
#ifdef DEBUG
			printf("%d\t%d\tdone water balance\n",simyr,yday);
#endif

	
	        /* test for carbon balance */
			if (ok  && check_carbon_balance(&cs, first_balance))
			{
				printf("Error in check_carbon_balance() from bgc()\n");
				ok=0;
			}
	
		
			
#ifdef DEBUG
			printf("%d\t%d\tdone carbon balance\n",simyr,yday); 
#endif


			/* test for nitrogen balance */
			if (ok && check_nitrogen_balance(&ns, first_balance))
			{
				printf("Error in check_nitrogen_balance() from bgc()\n");
				ok=0;
			}
			
			
#ifdef DEBUG
			printf("%d\t%d\tdone nitrogen balance\n",simyr,yday);
#endif
	

			/* calculate summary variables */
			if (ok && cnw_summary(yday, &sitec, &metv, &cs, &cf, &ns, &nf, &wf, &epv, &summary))
			{
				printf("Error in cnw_summary() from bgc()\n");
				ok=0;
			}
	
	
#ifdef DEBUG
			printf("%d\t%d\tdone carbon summary\n",simyr,yday); 
#endif


			/* output handling */
			if (ok && output_handling(mondays[curmonth], endday[curmonth], &ctrl, output_map, dayarr, monavgarr, annavgarr, annarr, 
				bgcout->dayout, bgcout->monavgout, bgcout->annavgout, bgcout->annout))
			{
				printf("Error in output_handling() from bgc()\n");
				ok=0;
			}
	
	
#ifdef DEBUG
			printf("%d\t%d\tdoneoutput_handling\n",simyr,yday); 
#endif
		
		
			/* at the end of first day of simulation, turn off the first_balance switch */
			if (first_balance) first_balance = 0;

			/* if this is the last day of the current month: increment current month counter */
			if (yday == endday[curmonth]) curmonth++;

		}   /* end of daily model loop */
		

	
		metyr++;

	}   /* end of annual model loop */


	/********************************************************************************************************* */
	/* wrinting log file */

	if (cs.balanceERR != 0) CbalanceERR = log10(cs.balanceERR);
	if (ns.balanceERR != 0) NbalanceERR = log10(ns.balanceERR);
	if (ws.balanceERR != 0) WbalanceERR = log10(ws.balanceERR);
	if (ctrl.CNerror  != 0) CNerror     = log10(ctrl.CNerror);

	fprintf(bgcout->log_file.ptr, "Some important annual outputs from last simulation year\n");
	fprintf(bgcout->log_file.ptr, "Cumulative sum of  GPP (gC/m2/year):                    %12.1f\n",summary.cum_gpp*1000);
	fprintf(bgcout->log_file.ptr, "Cumulative sum of  NEE (gC/m2/year):                    %12.1f\n",summary.cum_nee*1000);
	fprintf(bgcout->log_file.ptr, "Cumulative sum of  ET  (kgH2O/m2/year):                 %12.1f\n",summary.cum_ET);
	fprintf(bgcout->log_file.ptr, "Cumulative sum of N2O flux (gN/m2/year):                %12.1f\n",summary.cum_n2o*1000);
	fprintf(bgcout->log_file.ptr, "Maximum projected LAI (m2/m2):                          %12.2f\n",epv.ytd_maxplai);
    fprintf(bgcout->log_file.ptr, "Humus carbon content (0-10 cm) (gC/kg soil):            %12.1f\n",summary.humusC_top10);
	fprintf(bgcout->log_file.ptr, "SOM carbon content (0-10 cm) (gC/kg soil):              %12.1f\n",summary.SOM_C_top10);
	fprintf(bgcout->log_file.ptr, "SOM nitrogen content (0-10 cm) (gN/kg soil):            %12.1f\n",summary.SOM_N_top10);
	fprintf(bgcout->log_file.ptr, "Soil ammonium content (0-10 cm) (mgN/kg soil):          %12.1f\n",summary.NH4_top10);
	fprintf(bgcout->log_file.ptr, "Soil nitrate content (0-10 cm) (mgN/kg soil):           %12.1f\n",summary.NO3_top10);
	fprintf(bgcout->log_file.ptr, "SWC on last simulation day (0-4 m) (m3/m3):             %12.3f\n",epv.vwc_avg);
	fprintf(bgcout->log_file.ptr, " \n");
	fprintf(bgcout->log_file.ptr, "10-base logarithm of the maximum carbon balance diff.:   %12.1f\n",CbalanceERR);
	fprintf(bgcout->log_file.ptr, "10-base logarithm of the maximum nitrogen balance diff.: %12.1f\n",NbalanceERR);
	fprintf(bgcout->log_file.ptr, "10-base logarithm of the maximum water balance diff.:    %12.1f\n",WbalanceERR);
	fprintf(bgcout->log_file.ptr, "10-base logarithm of the C-N calc. numbering error:      %12.1f\n",CNerror);
	fprintf(bgcout->log_file.ptr, " \n");
	/********************************************************************************************************* */

	
	/* 8. RESTART OUTPUT HANDLING */
	/* if write_restart flag is set, copy data to the output restart struct */
	if (ok && ctrl.write_restart)
	{
		if (restart_output( &ws, &cs, &ns, &epv, &(bgcout->restart_output)))
		{
			printf("Error in call to restart_output() from bgc()\n");
			ok=0;
		}
		
#ifdef DEBUG
		printf("%d\t%d\tdone restart output\n",simyr,yday);
#endif
	}

	
	/* free memory for local output arrays */
	free(phenarr.onday_arr);
	free(phenarr.offday_arr);
    if (ctrl.dodaily) free(dayarr);
	if (ctrl.domonavg) free(monavgarr);
	if (ctrl.doannavg) free(annavgarr);
	if (ctrl.doannual)free(annarr);
	

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
