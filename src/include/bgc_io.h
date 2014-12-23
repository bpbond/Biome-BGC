#ifndef BGC_IO_H
#define BGC_IO_H
/*
bgc_io.h
header file to hold the structures used to communicate with bgc()
also contains the function prototype for bgc()

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#ifdef __cplusplus
extern "C"
{
#endif

/* structure for passing input parameters to bgc() */
typedef struct
{
	restart_data_struct restart_input;  /* input restart data */
	control_struct ctrl;    /* bgc control variables */
	ramp_ndep_struct ramp_ndep;  /* ramped Ndep information */
	co2control_struct co2;  /* CO2 concentration information */
	ndepcontrol_struct ndepctrl;	/* Nitrogen deposition control structure */
	metarr_struct metarr;   /* meteorological data array */
	wstate_struct ws;       /* water state variables */
	cinit_struct cinit;     /* first-year values for leafc and stemc */
	cstate_struct cs;       /* carbon state variables */
	nstate_struct ns;       /* nitrogen state variables */
	siteconst_struct sitec; /* site constants */
	epconst_struct epc;     /* ecophysiological constants */
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
	file dayoutascii;	/* file containing daily ascii output */
	file monoutascii;	/* file containing monthly ascii output */
	file annoutascii;	/* file containing annual ascii output */
	
	double spinup_resid_trend; /* kgC/m2/yr remaining trend after spinup */
	int spinup_years;       /* number of years before reaching steady-state */
	unsigned char bgc_ascii;	/* ASCII output flag */
} bgcout_struct;

/* function prototypes for calling bgc */
int bgc(bgcin_struct* bgcin, bgcout_struct* bgcout, int mode);

/* Verbosity sensitive printf for BiomeBGC.*/
#ifdef __USE_ISOC99
int _bgc_printf(const char *file, int line, signed char verbosity, const char *format, ...);
#define bgc_printf(...) _bgc_printf(__FILE__, __LINE__, __VA_ARGS__)
#else
int bgc_printf(signed char verbosity, const char *format, ...);
#endif
signed char bgc_verbosity_decode(char *keyword);
void bgc_print_usage(void);
int bgc_logfile_setup(char *logfile);
int bgc_logfile_finish(void);

/* Verbosity keywords. BV: Bgc Verbosity */
#define BV_SILENT -1
#define BV_ERROR 0
#define BV_WARN 1
#define BV_PROGRESS 2
#define BV_DETAIL 3
#define BV_DIAG 4

#ifdef __cplusplus
}
#endif

#endif
