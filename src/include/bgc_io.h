/*
bgc_io.h
header file to hold the structures used to communicate with bgc()
also contains the function prototype for bgc()

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.4
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2017, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/


/* structure for passing input parameters to bgc() */
typedef struct
{
	restart_data_struct restart_input;  /* input restart data */
	control_struct ctrl;		/* bgc control variables */
	ndep_control_struct ndep;	/* Ndep information */
	co2control_struct co2;		/* CO2 concentration information */
	metarr_struct metarr;   /* meteorological data array */
	wstate_struct ws;       /* water state variables */
	cinit_struct cinit;     /* first-year values for leafc and stemc */
	cstate_struct cs;       /* carbon state variables */
	nstate_struct ns;       /* nitrogen state variables */
	siteconst_struct sitec; /* site constants */
	epconst_struct epc;     /* ecophysiological constants */
	/* by Hidy 2012.*/
	planting_struct PLT;			/* parameters for planting */
	thinning_struct THN;			/* parameters for thinning */
    mowing_struct MOW;				/* parameters for mowing */
	grazing_struct GRZ;				/* parameters for grazing */
	harvesting_struct HRV;			/* parameters for harvesting */
	ploughing_struct PLG;			/* parameters for ploughing */
	fertilizing_struct FRZ;			/* parameters for fertilizing */
	irrigation_struct IRG;			/* parameters for irrigation */
	GSI_struct GSI;					/* parameters for calculating growing season */
} bgcin_struct;

/* structure for output handling from bgc() */
typedef struct
{
	restart_data_struct restart_output; /* output restart data */
	file dayout;            /* file containing daily output */
	file monavgout;         /* file containing monthly average output */
	file annavgout;         /* file containing annual average output */
	file annout;            /* file containing annual output */
	file anntext;           /* file containing simple annual text output */
	double spinup_resid_trend; /* kgC/m2/yr remaining trend after spinup */
	int spinup_years;       /* number of years before reaching steady-state */
	file control_file;
	file log_file;
} bgcout_struct;

/* function prototypes for calling bgc */
int bgc(bgcin_struct* bgcin, bgcout_struct* bgcout);
int spinup_bgc(bgcin_struct* bgcin, bgcout_struct* bgcout);
/* Hidy 2014 - transient run */
int transient_bgc(bgcin_struct* bgcin, bgcout_struct* bgcout);
