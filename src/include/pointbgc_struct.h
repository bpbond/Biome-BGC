/* 
pointbgc_struct.h
for use with pointbgc front-end to BIOME-BGC library v4.0

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.1.1
Copyright 2000, Peter E. Thornton
Numerical Terradynamics Simulation Group (NTSG)
School of Forestry, University of Montana
Missoula, MT 59812
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

/* point simulation control parameters */
typedef struct
{
    char header[100];      /* header string, written to all output files */
	char systime[100];     /* system time at start of simulation */ 
	file metf;             /* met data file (ASCII) *//* simulation restart control variables */
} point_struct;

typedef struct
{
	int read_restart;      /* flag to read restart file */
	int write_restart;     /* flag to write restart file */
	int keep_metyr;        /* (flag) 1=retain restart metyr, 0=reset metyr */
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
	int onscreen;          /* flag for on-screen progress indicator */
	char outprefix[100];   /* output filename prefix */
	int dodaily; 	       /* flag for daily output */
	int domonavg;          /* flag for monthly average output */
	int doannavg;          /* flag for annual average output */
	int doannual;		   /* flag for annual output */
	int ndayout;           /* number of custom daily outputs */
	int nannout;           /* number of custom annual outputs */
	int* daycodes;         /* array of indices for daily outputs */
	int* anncodes;         /* array of indices for annual outputs */
    file dayout;           /* daily output file */
	file monavgout;        /* monthly average output file */
	file annavgout;        /* annual average output file */
    file annout;           /* annual output file */
	file anntext;          /* simple annual text output */
} output_struct;

