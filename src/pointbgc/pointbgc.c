/*
pointbgc.c
front-end to BIOME-BGC for single-point, single-biome simulations
Uses BIOME-BGC function library

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "pointbgc.h"

char *argv_zero = NULL;
signed char cli_mode = MODE_INI;

int main(int argc, char *argv[])
{
	/* bgc input and output structures */
	bgcin_struct bgcin;
	bgcout_struct bgcout;

	/* local control information */
	point_struct point;
	restart_ctrl_struct restart;
	climchange_struct scc;
	output_struct output;
	
	/* initialization file */
	file init;
	file ndep_file;

	/* system time variables */
	struct tm *tm_ptr;
	time_t lt;

	extern signed char summary_sanity;

	int c; /* for getopt cli argument processing */
	extern signed char bgc_verbosity;
	extern int optind, opterr;
	unsigned char bgc_ascii = 0;
	extern char *optarg;
	extern signed char cli_mode; /* What cli requested mode to run in.*/
	int readndepfile = 0;		/* Flag to tell the program to read an external NDEP file passed using getpopt -n */
	
	bgcin.ndepctrl.varndep = 0;
	/* Store command name for use by bgc_print_usage() */
	argv_zero = (char *)malloc(strlen(argv[0])+1);
	strncpy(argv_zero, argv[0], strlen(argv[0])+1);

	/* Process command line arguments */
	opterr = 0;
	while((c = getopt(argc, argv, "pVsl:v:ugmn:a")) != -1)
	{
		switch(c)
		{
			case 'V':
				bgc_printf(BV_ERROR, "BiomeBGC version %s (built %s %s by %s on %s)\n", VERS, __DATE__, __TIME__, USER, HOST);
				exit(EXIT_SUCCESS);
				break;
			case 's':
				bgc_verbosity = BV_SILENT;
				break;
			case 'v':
				bgc_verbosity = bgc_verbosity_decode(optarg);
				break;
			case 'l':
				bgc_logfile_setup(optarg);
				bgc_printf(BV_DIAG, "Using logfile for output.\n");
				break;
			case 'p':
				summary_sanity = SANE;
				break;
			case 'u':
				cli_mode = MODE_SPINUP;
				break;
			case 'm':
				cli_mode = MODE_MODEL;
				break;
			case 'g':
				cli_mode = MODE_SPINNGO;
				break;
			case 'a':
				bgc_ascii = 1;
				break;
			case 'n':  /* Nitrogen deposition file */
				strcpy(ndep_file.name,optarg);
				bgc_printf(BV_DIAG,"Using annual NDEP file: %s\n",ndep_file.name);
				readndepfile = 1;
				bgcin.ndepctrl.varndep = 1;
				break;
				
			case '?':
				break;
			default:
				break;
			}
	}

	bgc_printf(BV_DIAG, "Verbosity Level Set To: %d\n", bgc_verbosity);
	
	if (summary_sanity == SANE)
		bgc_printf(BV_WARN, "Summary outputs will be calculated more sanely. See USAGE.TXT for details\n");

	if (cli_mode != MODE_INI)
	{
		bgc_printf(BV_WARN, "Overridding ini mode. ");
		if (cli_mode == MODE_SPINUP)
			bgc_printf(BV_WARN, "Running in Spinup Mode.\n");
		if (cli_mode == MODE_MODEL)
			bgc_printf(BV_WARN, "Running in Model mode.\n");
		if (cli_mode == MODE_SPINNGO)
			bgc_printf(BV_WARN, "Running in Spin-and-Go mode.\nThe spinup and model will both be run.\n");
	}
		
	bgc_printf(BV_DIAG, "Done processing CLI arguments.\n");

	/* get the system time at start of simulation */
	lt = time(NULL);
	tm_ptr = localtime(&lt);
	strcpy(point.systime,asctime(tm_ptr));
	/* Andrew tried this, you shouldn't. localtime returns a global extern. */
	/* free(tm_ptr); */
	output.anncodes = NULL;
	output.daycodes = NULL;
	output.bgc_ascii = bgc_ascii;
	
	/* initialize the bgcin state variable structures before filling with
	values from ini file */
	if (presim_state_init(&bgcin.ws, &bgcin.cs, &bgcin.ns, &bgcin.cinit))
	{
		bgc_printf(BV_ERROR, "Error in call to presim_state_init() from pointbgc()\n");
		exit(EXIT_FAILURE);
	}

	/******************************
	**                           **
	**  BEGIN READING INIT FILE  **
	**                           **
	******************************/

	/* read the name of the main init file from the command line
	and store as init.name */
	if (optind >= argc )
	{
		bgc_print_usage();
		exit(EXIT_FAILURE);
	}
	strcpy(init.name, argv[optind]);
	
	/* open the main init file for ascii read and check for errors */
	if (file_open(&init,'i'))
	{
		bgc_printf(BV_ERROR, "Error opening init file, pointbgc.c\n");
		exit(EXIT_FAILURE);
	}

	/* read the header string from the init file */
	if (fgets(point.header, 100, init.ptr)==NULL)
	{
		bgc_printf(BV_ERROR, "Error reading header string: pointbgc.c\n");
		exit(EXIT_FAILURE);
	}

	/* open met file, discard header lines */
	if (met_init(init, &point))
	{
		bgc_printf(BV_ERROR, "Error in call to met_init() from pointbgc.c... Exiting\n");
		exit(EXIT_FAILURE);
	}

	/* read restart control parameters */
	if (restart_init(init, &restart))
	{
		bgc_printf(BV_ERROR, "Error in call to restart_init() from pointbgc.c... Exiting\n");
		exit(EXIT_FAILURE);
	}

	/* read simulation timing control parameters */
	if (time_init(init, &(bgcin.ctrl)))
	{
		bgc_printf(BV_ERROR, "Error in call to epclist_init() from pointbgc.c... Exiting\n");
		exit(EXIT_FAILURE);
	}
	
	/* read scalar climate change parameters */
	if (scc_init(init, &scc))
	{
		bgc_printf(BV_ERROR, "Error in call to scc_init() from pointbgc.c... Exiting\n");
		exit(EXIT_FAILURE);
	}
	
	/* read CO2 control parameters */
	if (co2_init(init, &(bgcin.co2), bgcin.ctrl.simyears))
	{
		bgc_printf(BV_ERROR, "Error in call to co2_init() from pointbgc.c... Exiting\n");
		exit(EXIT_FAILURE);
	}
	if(readndepfile)
	{
		if (ndep_init(ndep_file, &(bgcin.ndepctrl)))
		{
			bgc_printf(BV_ERROR, "Error in call to ndep_init() from pointbgc.c... Exiting\n");
			exit(EXIT_FAILURE);
		}
	}
	/* read site constants */
	if (sitec_init(init, &bgcin.sitec))
	{
		bgc_printf(BV_ERROR, "Error in call to sitec_init() from pointbgc.c... Exiting\n");
		exit(EXIT_FAILURE);
	}
	
	/* read ramped nitrogen deposition block */
	if (ramp_ndep_init(init, &bgcin.ramp_ndep))
	{
		bgc_printf(BV_ERROR, "Error in call to ramp_ndep_init() from pointbgc.c... Exiting\n");
		exit(EXIT_FAILURE);
	}
	
	/* read ecophysiological constants */
	if (epc_init(init, &bgcin.epc))
	{
		bgc_printf(BV_ERROR, "Error in call to epc_init() from pointbgc.c... Exiting\n");
		exit(EXIT_FAILURE);
	}

	/* initialize water state structure */
	if (wstate_init(init, &bgcin.sitec, &bgcin.ws))
	{
		bgc_printf(BV_ERROR, "Error in call to wstate_init() from pointbgc.c... Exiting\n");
		exit(EXIT_FAILURE);
	}

	/* initialize carbon and nitrogen state structures */
	if (cnstate_init(init, &bgcin.epc, &bgcin.cs, &bgcin.cinit,
		&bgcin.ns))
	{
		bgc_printf(BV_ERROR, "Error in call to cstate_init() from pointbgc.c... Exiting\n");
		exit(EXIT_FAILURE);
	}

	/* read the output control information */
	if (output_ctrl(init, &output))
	{
		bgc_printf(BV_ERROR, "Error in call to output_ctrl() from pointbgc.c... Exiting\n");
		exit(EXIT_FAILURE);
	}
	
	/* initialize output files. Does nothing in spinup mode*/
	if (output_init(&output))
	{
		bgc_printf(BV_ERROR, "Error in call to output_init() from pointbgc.c... Exiting\n");
		exit(EXIT_FAILURE);
	}
	
	/* read final line out of init file to test for proper file structure */
	if (end_init(init))
	{
		bgc_printf(BV_ERROR, "Error in call to end_init() from pointbgc.c... exiting\n");
		exit(EXIT_FAILURE);
	}
	fclose(init.ptr);

	/* read meteorology file, build metarr arrays, compute running avgs */
	if (metarr_init(point.metf, &bgcin.metarr, &scc, bgcin.ctrl.metyears))
	{
		bgc_printf(BV_ERROR, "Error in call to metarr_init() from pointbgc.c... Exiting\n");
		exit(EXIT_FAILURE);
	}
	fclose(point.metf.ptr);

	/* copy some of the info from input structure to bgc simulation control
	structure */
	bgcin.ctrl.onscreen = output.onscreen;
	bgcin.ctrl.dodaily = output.dodaily;
	bgcin.ctrl.domonavg = output.domonavg;
	bgcin.ctrl.doannavg = output.doannavg;
	bgcin.ctrl.doannual = output.doannual;
	bgcin.ctrl.ndayout = output.ndayout;
	bgcin.ctrl.nannout = output.nannout;
	bgcin.ctrl.daycodes = output.daycodes;
	bgcin.ctrl.anncodes = output.anncodes;
	bgcin.ctrl.read_restart = restart.read_restart;
	bgcin.ctrl.write_restart = restart.write_restart;
	bgcin.ctrl.keep_metyr = restart.keep_metyr;
	
	/* copy the output file structures into bgcout */
	if (output.dodaily) bgcout.dayout = output.dayout;
	if (output.domonavg) bgcout.monavgout = output.monavgout;
	if (output.doannavg) bgcout.annavgout = output.annavgout;
	if (output.doannual) bgcout.annout = output.annout;
	if (output.bgc_ascii && output.dodaily) bgcout.dayoutascii = output.dayoutascii;
	if (output.bgc_ascii && output.domonavg) bgcout.monoutascii = output.monoutascii;
	if (output.bgc_ascii && output.doannual) bgcout.annoutascii = output.annoutascii;
	bgcout.anntext = output.anntext;
	bgcout.bgc_ascii = bgc_ascii;
	
	/* if using ramped Ndep, copy preindustrial Ndep into ramp_ndep struct */
	if (bgcin.ramp_ndep.doramp)
	{
		bgcin.ramp_ndep.preind_ndep = bgcin.sitec.ndep;
	}
	
	/* if using an input restart file, read a record */
	if (restart.read_restart)
	{
		/* 02/06/04
		 * The if statement gaurds against core dump on bad restart file.
		 * If spinup exits with error then the norm trys to use the restart,
		 * that has nothing in it, a seg fault occurs. Amac */
		if( fread(&(bgcin.restart_input),sizeof(restart_data_struct),1,restart.in_restart.ptr) == 0)
		{
			bgc_printf(BV_ERROR, "Error reading restart file! 0 bytes read. Aborting..\n");
			exit(EXIT_FAILURE);
		}
	}

	/*********************
	**                  **
	**  CALL BIOME-BGC  **
	**                  **
	*********************/

	/* all initialization complete, call model */
	/* either call the spinup code or the normal simulation code */
	if (bgcin.ctrl.spinup)
	{
		if (bgc(&bgcin, &bgcout,MODE_SPINUP))
		{
			bgc_printf(BV_ERROR, "Error in call to bgc()\n");
			exit(EXIT_FAILURE);
		}
		bgc_printf(BV_PROGRESS, "SPINUP: residual trend  = %.6lf\n",bgcout.spinup_resid_trend);
		bgc_printf(BV_PROGRESS, "SPINUP: number of years = %d\n",bgcout.spinup_years);
	}
	else
	{
		if (bgc(&bgcin, &bgcout, MODE_MODEL))
		{
			bgc_printf(BV_ERROR, "Error in call to bgc()\n");
			exit(EXIT_FAILURE);
		}
	}
		

	/* if using an output restart file, write a record */
	if (restart.write_restart)
	{
		fwrite(&(bgcout.restart_output),sizeof(restart_data_struct),1,
			restart.out_restart.ptr);
	}
	
	/* Now do the Model part of Spin & Go. */
	if (cli_mode == MODE_SPINNGO)
	{
		bgc_printf(BV_PROGRESS, "Finished Spinup for Spin 'n Go. Now starting Model run ('Go' part of Spin'n Go)\n");
			
		bgc_printf(BV_PROGRESS, "Assigned bgcout struct to bgcin for spinngo model run\n");
		
		bgcin.ctrl.spinup = 0;
		output.doannavg = 1;
		output.doannual = 1;
		output.dodaily = 1;
		output.domonavg = 1;
		
		if (output_init(&output))
		{
			bgc_printf(BV_ERROR, "Error in call to output_init() from pointbgc.c... Exiting\n");
			exit(EXIT_FAILURE);
		}
		
		/* copy some of the info from input structure to bgc simulation control structure */
		bgcin.ctrl.dodaily = output.dodaily;
		bgcin.ctrl.domonavg = output.domonavg;
		bgcin.ctrl.doannavg = output.doannavg;
		bgcin.ctrl.doannual = output.doannual;
	
		/* copy the output file structures into bgcout */
		if (output.dodaily) bgcout.dayout = output.dayout;
		if (output.domonavg) bgcout.monavgout = output.monavgout;
		if (output.doannavg) bgcout.annavgout = output.annavgout;
		if (output.doannual) bgcout.annout = output.annout;
		if (output.bgc_ascii && output.dodaily) bgcout.dayoutascii = output.dayoutascii;
		if (output.bgc_ascii && output.domonavg) bgcout.monoutascii = output.monoutascii;
		if (output.bgc_ascii && output.doannual) bgcout.annoutascii = output.annoutascii;
		if (output.bgc_ascii && output.doannual) bgcout.anntext = output.anntext;
		
		/* initialize output files. Does nothing in spinup mode*/
		
		
		bgcin.ctrl.read_restart = 1;
		bgcin.restart_input = bgcout.restart_output;
		
		if (bgc(&bgcin, &bgcout, MODE_MODEL))
		{
			bgc_printf(BV_ERROR, "Error in call to bgc()\n");
			exit(EXIT_FAILURE);
		}
		restart.read_restart = 0;
		bgcin.ctrl.read_restart = 0;

		bgc_printf(BV_WARN, "Finished the bgc() Model call in spinngo\n");
		
	}

	/* post-processing output handling, if any, goes here */
	
	/* free memory */
	free(bgcin.metarr.tmax);
	free(bgcin.metarr.tmin);
	free(bgcin.metarr.prcp);
	free(bgcin.metarr.vpd);
	free(bgcin.metarr.tavg);
	free(bgcin.metarr.tavg_ra);
	free(bgcin.metarr.swavgfd);
	free(bgcin.metarr.par);
	free(bgcin.metarr.dayl);
	if (bgcin.co2.varco2) free(bgcin.co2.co2ppm_array);
	if (bgcin.ndepctrl.varndep) free(bgcin.ndepctrl.ndepyear_array);
	if (bgcin.ndepctrl.varndep) free(bgcin.ndepctrl.ndep_array);
	if (output.anncodes != NULL) free(output.anncodes);
	if (output.daycodes != NULL) free(output.daycodes);
	
	/* close files */
	if (restart.read_restart) fclose(restart.in_restart.ptr);
	if (restart.write_restart) {
		if (fclose(restart.out_restart.ptr) != 0)
		{
			bgc_printf(BV_WARN, "Warning, error closing restart file after write: %s\n", strerror(errno));
		}
	}
	if (output.dodaily) fclose(output.dayout.ptr);
	if (output.domonavg) fclose(output.monavgout.ptr);
	if (output.doannavg) fclose(output.annavgout.ptr);
	if (output.doannual) fclose(output.annout.ptr);
	/* Close the ASCII output files */
	if (output.bgc_ascii && output.dodaily) fclose(output.dayoutascii.ptr);
	if (output.bgc_ascii && output.domonavg) fclose(output.monoutascii.ptr);
	if (output.bgc_ascii && output.doannual) fclose(output.annoutascii.ptr);
	
	if ( output.bgc_ascii && output.doannual && (fclose(output.anntext.ptr) != 0))
	{
		bgc_printf(BV_WARN, "Warning, error closing ascii annual output file: %s\n", strerror(errno));
	}

	bgc_logfile_finish();
	free(argv_zero);
	return EXIT_SUCCESS;
} /* end of main */
	
