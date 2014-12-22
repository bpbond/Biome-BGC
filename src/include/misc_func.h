/*
misc_func.h
function prototypes for miscellaneous functions used in bgc routines
for use with pointbgc front-end to BIOME-BGC library v4.1

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.1.1
Copyright 2000, Peter E. Thornton
Numerical Terradynamics Simulation Group (NTSG)
School of Forestry, University of Montana
Missoula, MT 59812
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/
/* function prototypes for smoothing functions */
int run_avg(const double *input, double *output, int n, int w, int w_flag);
int boxcar_smooth(double* input, double* output, int n, int w, int w_flag);
