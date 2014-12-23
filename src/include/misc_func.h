#ifndef MISC_FUNC_H
#define MISC_FUNC_H
/*
misc_func.h
function prototypes for miscellaneous functions used in bgc routines
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

/* function prototypes for smoothing functions */
int run_avg(const double *input, double *output, int n, int w, int w_flag);
int boxcar_smooth(double* input, double* output, int n, int w, int w_flag);

#ifdef __cplusplus
}
#endif

#endif
