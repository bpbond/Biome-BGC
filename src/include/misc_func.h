/*
misc_func.h
function prototypes for miscellaneous functions used in bgc routines
for use with pointbgc front-end to BBGC MuSo v4 library

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
BBGC MuSo v4
Copyright 2000, Peter E. Thornton
Copyright 2014, D. Hidy
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/
/* function prototypes for smoothing functions */
int run_avg(const double *input, double *output, int n, int w, int w_flag);
int boxcar_smooth(double* input, double* output, int n, int w, int w_flag);
