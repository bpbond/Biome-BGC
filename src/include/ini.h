/*
ini.h
header file for file I/O handling and initialization file reading
for use with BBGC MuSo v4 library

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
BBGC MuSo v4
Copyright 2000, Peter E. Thornton
Copyright 2014, D. Hidy
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

/* structure definition for filename handling */
typedef struct
{
	char name[128];
	FILE *ptr;
} file;

/* function prototypes */
int file_open (file *target, char mode);
int scan_value (file ini, void *var, char mode);
int scan_array (file ini, void *var, char mode, int nl);
int scan_open (file ini,file *target,char mode);
