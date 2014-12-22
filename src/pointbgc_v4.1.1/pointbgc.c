/*
pointbgc.c
front-end to BIOME-BGC for single-point, single-biome simulations
Uses BIOME-BGC function library v4.1

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.1.1
Copyright 2000, Peter E. Thornton
Numerical Terradynamics Simulation Group (NTSG)
School of Forestry, University of Montana
Missoula, MT 59812
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <time.h>

#include "ini.h"              /* general file structure and I/O prototypes */
#include "bgc_struct.h"       /* data structures for bgc() */
#include "pointbgc_struct.h"   /* data structures for point driver */
#include "pointbgc_func.h"     /* function prototypes for point driver */
#include "bgc_io.h"           /* bgc() interface definition */
#include "bgc_epclist.h"      /* array structure for epc-by-vegtype */

void main(int argc, char *argv[])
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
	
	/* system time variables */
	struct tm *tm_ptr;
	time_t lt;
	
	/* get the system time at start of simulation */
	lt = time(NULL);
	tm_ptr = localtime(&lt);
	strcpy(point.systime,asctime(tm_ptr));
	
	/* initialize the bgcin state variable structures before filling with
	values from ini file */
	if (presim_state_init(&bgcin.ws, &bgcin.cs, &bgcin.ns, &bgcin.cinit))
	{
		printf("Error in call to presim_state_init() from pointbgc()\n");
		exit(1);
	}

	/******************************
	**                           **
	**  BEGIN READING INIT FILE  **
	**                           **
	******************************/

	/* read the name of the main init file from the command line
	and store as init.name */
	if (argc != 2)
	{
		printf("usage: <executable name>  <initialization file name>\n");
		exit(1);
	}
	strcpy(init.name, argv[1]);
	
	/* open the main init file for ascii read and check for errors */
	if (file_open(&init,'i'))
	{
		printf("Error opening init file, pointbgc.c\n");
		exit(1);
	}

	/* read the header string from the init file */
	if (fgets(point.header, 100, init.ptr)==NULL)
	{
		printf("Error reading header string: pointbgc.c\n");
		exit(1);
	}

	/* open met file, discard header lines */
	if (met_init(init, &point))
	{
		printf("Error in call to met_init() from pointbgc.c... Exiting\n");
		exit(1);
	}

	/* read restart control parameters */
	if (restart_init(init, &restart))
	{
		printf("Error in call to restart_init() from pointbgc.c... Exiting\n");
		exit(1);
	}

	/* read simulation timing control parameters */
	if (time_init(init, &(bgcin.ctrl)))
	{
		printf("Error in call to epclist_init() from pointbgc.c... Exiting\n");
		exit(1);
	}
	
	/* read scalar climate change parameters */
	if (scc_init(init, &scc))
	{
		printf("Error in call to scc_init() from pointbgc.c... Exiting\n");
		exit(1);
	}
	
	/* read CO2 control parameters */
	if (co2_init(init, &(bgcin.co2), bgcin.ctrl.simyears))
	{
		printf("Error in call to co2_init() from pointbgc.c... Exiting\n");
		exit(1);
	}
	
	/* read site constants */
	if (sitec_init(init, &bgcin.sitec))
	{
		printf("Error in call to sitec_init() from pointbgc.c... Exiting\n");
		exit(1);
	}
	
	/* read ramped nitrogen deposition block */
	if (ramp_ndep_init(init, &bgcin.ramp_ndep))
	{
		printf("Error in call to ramp_ndep_init() from pointbgc.c... Exiting\n");
		exit(1);
	}
	
	/* read ecophysiological constants */
	if (epc_init(init, &bgcin.epc))
	{
		printf("Error in call to epc_init() from pointbgc.c... Exiting\n");
		exit(1);
	}

	/* initialize water state structure */
	if (wstate_init(init, &bgcin.sitec, &bgcin.ws))
	{
		printf("Error in call to wstate_init() from pointbgc.c... Exiting\n");
		exit(1);
	}

	/* initialize carbon and nitrogen state structures */
	if (cnstate_init(init, &bgcin.epc, &bgcin.cs, &bgcin.cinit,
		&bgcin.ns))
	{
		printf("Error in call to cstate_init() from pointbgc.c... Exiting\n");
		exit(1);
	}

	/* read the output control information */
	if (output_init(init, &output))
	{
		printf("Error in call to output_init() from pointbgc.c... Exiting\n");
		exit(1);
	}
	
	/* read final line out of init file to test for proper file structure */
	if (end_init(init))
	{
		printf("Error in call to end_init() from pointbgc.c... exiting\n");
		exit(1);
	}
	fclose(init.ptr);

	/* read meteorology file, build metarr arrays, compute running avgs */
	if (metarr_init(point.metf, &bgcin.metarr, &scc, bgcin.ctrl.metyears))
	{
		printf("Error in call to metarr_init() from pointbgc.c... Exiting\n");
		exit(1);
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
	bgcout.anntext = output.anntext;
	
	
	/* if using ramped Ndep, copy preindustrial Ndep into ramp_ndep struct */
	if (bgcin.ramp_ndep.doramp)
	{
		bgcin.ramp_ndep.preind_ndep = bgcin.sitec.ndep;
	}
	
	/* if using an input restart file, read a record */
	if (restart.read_restart)
	{
		fread(&(bgcin.restart_input),sizeof(restart_data_struct),1,
			restart.in_restart.ptr);
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
		if (spinup_bgc(&bgcin, &bgcout))
		{
			printf("Error in call to bgc()\n");
			exit(1);
		}
		printf("SPINUP: residual trend  = %.6lf\n",bgcout.spinup_resid_trend);
		printf("SPINUP: number of years = %d\n",bgcout.spinup_years);
	}
	else
	{
		if (bgc(&bgcin, &bgcout))
		{
			printf("Error in call to bgc()\n");
			exit(1);
		}
	}
		

	/* if using an output restart file, write a record */
	if (restart.write_restart)
	{
		fwrite(&(bgcout.restart_output),sizeof(restart_data_struct),1,
			restart.out_restart.ptr);
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
	free(output.anncodes);
	free(output.daycodes);
	
	/* close files */
	if (restart.read_restart) fclose(restart.in_restart.ptr);
	if (restart.write_restart) fclose(restart.out_restart.ptr);
	if (output.dodaily) fclose(output.dayout.ptr);
	if (output.domonavg) fclose(output.monavgout.ptr);
	if (output.doannavg) fclose(output.annavgout.ptr);
	if (output.doannual) fclose(output.annout.ptr);
	fclose(output.anntext.ptr);
	
} /* end of main */
	
