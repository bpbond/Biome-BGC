#ifndef INI_H
#define INI_H

/*
ini.h
header file for file I/O handling and initialization file reading
for use with BIOME-BGC library

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#ifdef __cplusplus
extern "C"
{
#endif

#define FLT_COND_TOL 1e-10  /* This constant is used in if conditions    *
                             * where floating point values are compared  */


/* structure definition for filename handling */
typedef struct
{
	char name[128];
	FILE *ptr;
} file;

/* function prototypes */
int file_open (file *target, char mode);
int scan_value (file ini, void *var, char mode);
int scan_open (file ini,file *target,char mode);

#ifdef __cplusplus
}
#endif

#endif
