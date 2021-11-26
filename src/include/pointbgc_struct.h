/* 
pointbgc_struct.h
for use with pointbgc front-end to BBGC MuSo v4 library

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2020, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/
#define N_MGMDAYS 7		/*  number of type of management events in a single year */

/* point simulation control parameters */
typedef struct
{
    char header[100];      /* header string, written to all output files */
	char systime[100];     /* system time at start of simulation */ 
	file metf;             /* met data file (ASCII) *//* simulation restart control variables */
	int  nday_lastsimyear; /* (int) number of simdays in last simyear (truncated year: < 365) */
} point_struct;

typedef struct
{
	int read_restart;      /* flag to read restart file */
	int write_restart;     /* flag to write restart file */
	file in_restart;       /* input restart file */
	file out_restart;      /* output restart file */
} restart_ctrl_struct;

/* a structure to hold scalar climate change scenario information */
typedef struct
{
    double s_tmax;          /* (deg C)   daily maximum air temperature */
    double s_tmin;          /* (deg C)   daily minimum air temperature */
    double s_prcp;          /* (scalar)  precipitation multiplier */
    double s_vpd;           /* (scalar)  vpd multiplier */
    double s_swavgfd;       /* (scalar)  shortwave multiplier */
} climchange_struct;

typedef struct
{
	int onscreen;				/* flag for on-screen progress indicator */
	char outprefix[100];		/* output filename prefix */
	int dodaily; 				/* flag for daily output */
	int domonavg;				/* flag for monthly average output */
	int doannavg;				/* flag for annual average output */
	int doannual;				/* flag for annual output */
	int ndayout;				/* number of custom daily outputs */
	int nannout;				/* number of custom annual outputs */
	int* daycodes;				/* array of indices for daily outputs */
	char** daynames;			/* array of names for daily outputs */
	int* anncodes;				/* array of indices for annual outputs */
	char** annnames;			/* array of names for daily outputs */
    file dayout;				/* daily output file */
	file monavgout;				/* monthly average output file */
	file annavgout;				/* annual average output file */
    file annout;				/* annual output file */
	file dayoutT;				/* transient daily output file */
	file monavgoutT;			/* transient monthly average output file */
	file annavgoutT;			/* transient annual average output file */
    file annoutT;				/* transient annual output file */
	file log_file;				/* main information about model run */
	file econout_file;			/* economical information  */
} output_struct;



