/*
ini.h
header file for file I/O handling and initialization file reading
for use with BBGC MuSo v4 library

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2020, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/
#include "bgc_constants.h"

/* structure definition for filename handling */
typedef struct
{
	char name[FILENAMESIZE];
	FILE *ptr;
} file;

/* function prototypes */
int file_open (file *target, char mode, int errormessage);
int scan_value (file ini, void *var, char mode);
int scan_array (file ini, void *var, char mode, int nl, int errormessage);
int scan_open (file ini,file *target,char mode, int errormessage);
