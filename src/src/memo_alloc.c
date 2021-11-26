/*
prephenology.c
Initialize phenology arrays, called prior to annual loop in bgc()

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.0.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2019, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include "ini.h"
#include "bgc_struct.h"
#include "pointbgc_struct.h"
#include "pointbgc_func.h"
#include "bgc_func.h"     
#include "bgc_constants.h"
#include "misc_func.h"

int memo_alloc(const planting_struct* PLT, const control_struct* ctrl, phenarray_struct* phenarr,  
	          double* dayarr, double* monavgarr, double* annavgarr, double* annarr)

{
	int errflag=0;
	int nyears,i;



	/* in case of natural ecosystem, nyears = number of meteorological year, in case agricultural system: nyears = number of plantings */
	if (!PLT->PLT_num)
		nyears = ctrl->simyears;
	else
		nyears = PLT->PLT_num + ctrl->simyears;
	
	/* allocate memory for local output arrays */
	if (!errflag && (ctrl->dodaily || ctrl->domonavg || ctrl->doannavg)) 
	{
		dayarr = (double*) malloc(ctrl->ndayout * sizeof(double));
		if (!dayarr)
		{
			printf("ERROR allocating for local daily output array in bgc()\n");
			errflag=301;
		}
		for (i=0 ; i<ctrl->ndayout ; i++) dayarr[i] = 0.0;
	}

	if (!errflag && ctrl->domonavg) 
	{
		monavgarr = (double*) malloc(ctrl->ndayout * sizeof(double));
		if (!monavgarr)
		{
			printf("ERROR allocating for monthly average output array in bgc()\n");
			errflag=302;
		}

		for (i=0 ; i<ctrl->ndayout ; i++) monavgarr[i] = 0.0;
	}
	if (!errflag && ctrl->doannavg) 
	{
		annavgarr = (double*) malloc(ctrl->ndayout * sizeof(double));
		if (!annavgarr)
		{
			printf("ERROR allocating for annual average output array in bgc()\n");
			errflag=303;
		}
		for (i=0 ; i<ctrl->ndayout ; i++) annavgarr[i] = 0.0;
	}
	if (!errflag && ctrl->doannual)
	{
		annarr = (double*) malloc(ctrl->nannout * sizeof(double));
		if (!annarr)
		{
			printf("ERROR allocating for local annual output array in bgc()\n");
			errflag=304;
		}
		for (i=0 ; i<ctrl->nannout ; i++) annarr[i] = 0.0;

	}
	

	
	/* allocate space for the onday_arr and offday_arr: first column - year, second column: day*/
	if (!errflag)
	{
		phenarr->onday_arr  = (int**) malloc(nyears*sizeof(int*));  
        phenarr->offday_arr = (int**) malloc(nyears*sizeof(int*));  
			
		for (i = 0; i<nyears; i++)
		{
			phenarr->onday_arr[i]  = (int*) malloc(2*sizeof(int));  
			phenarr->offday_arr[i] = (int*) malloc(2*sizeof(int));  
			phenarr->onday_arr[i]  = (int*) malloc(2*sizeof(int));  
			phenarr->offday_arr[i] = (int*) malloc(2*sizeof(int));  
		}


		if (!phenarr->onday_arr || !phenarr->onday_arr)
		{
			printf("ERROR allocating for onday_arr/offday_arr, prephenology()\n");
			errflag=306;
		}
	}
	
	

	return (errflag);
}

