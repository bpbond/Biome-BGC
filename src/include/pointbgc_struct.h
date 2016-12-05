/* 
pointbgc_struct.h
for use with pointbgc front-end to BBGC MuSo v4 library

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.2
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2016, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
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
	file control_file;	   /* Hidy 2012 - file to write out control variables */
	file log_file;		   /* Hidy 2015 - main information about model run */
} output_struct;


/* GSI arrays - by Hidy 2012.*/
typedef struct
{
	int GSI_flag;					/* flag for doing runoff or not */
	double snowcover_limit;			/* critical amount of snow (above: no vegetation period) */
	double heatsum_limit1;			/* lower limit of heatsum to calculate heatsum index */
    double heatsum_limit2;			/* upper limit of heatsum to calculate heatsum index */
	double tmin_limit1;				/* lower limit of tmin to calculate tmin index */
	double tmin_limit2;				/* upper limit of tmin to calculate tmin index  */
	double vpd_limit1;				/* lower limit of tmin to calculate vpd index  */
	double vpd_limit2;				/* upper limit of tmin to calculate vpd index  */
	double dayl_limit1;				/* lower limit of dayl to calculate vpd index  */
	double dayl_limit2;				/* upper limit of dayl to calculate vpd index  */
	int n_moving_avg;				/* moving average (calculated from indicatiors to avoid the effects of single extreme events)*/
	double GSI_limit_SGS;			/* when GSI index fisrt time greater that limit -> start of the growing season */
	double GSI_limit_EGS;			/* when GSI index fisrt time less that limit -> end of the growing season */
	file GSI_file;					/* file to write out th estimated start and the end of vegper */
} GSI_struct;

