#ifndef POINTBGC_STRUCT_H
#define POINTBGC_STRUCT_H
/* 
pointbgc_struct.h
for use with pointbgc front-end to BIOME-BGC library

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#ifdef __cplusplus
extern "C"
{
#endif

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
	file dayoutascii;	/* ASCII daily output file */
	file monoutascii;	/* ASCII monthly output file */
	file annoutascii;	/* ASCII annual output file */
	unsigned char bgc_ascii;	
} output_struct;

#ifdef __cplusplus
}
#endif

#endif
