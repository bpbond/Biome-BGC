/* 
smooth.c
functions that perform smoothing on vectors

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group (NTSG)
School of Forestry, University of Montana
Missoula, MT 59812
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include "misc_func.h"

int run_avg(const double *input, double *output, int n, int w, int w_flag)
{
    /* calculates running averages
    n = length of input and output arrays
    w = width of averaging window
    w_flag : 1=linearly weighted window, 0=constant weighted window
    */

    register int i,j;
    int errorCode=0;
    int *wt = 0;
    double total, sum;

    if (w>n)
    {
        printf("ERROR: averaging window longer than input array\n");
        errorCode=1;
    }

    if (!errorCode)
    {
		wt = (int*) malloc(w * sizeof(int));
		if (!wt)
		{
    		printf("Allocation error in boxcar_smooth... Exiting\n");
    		errorCode=1;
		}
    }

    if (!errorCode)
    {
        if (w_flag)
            for (i=0 ; i<w ; i++)
                wt[i] = i+1;
        else
            for (i=0 ; i<w ; i++)
                wt[i] = 1;

        for (i=0 ; i<n ; i++)
        {
            total = 0.0;
            sum = 0.0;
            if (i<(w-1))
            {
                for (j=w-i-1 ; j<w ; j++)
                {
                    total += (double)wt[j] * input[i-w+j+1];
                    sum += (double)wt[j];
                }
            }
            else
            {
                for (j=0 ; j<w ; j++)
                {
                    total += (double)wt[j] * input[i-w+j+1];
                    sum += (double)wt[j];
                }
            }
            output[i] = total/sum;
            
        } /* end for i=nelements */
        
        free(wt);
    }
    return (errorCode);
}

/* boxcar_smooth() performs a windowed smoothing on the input array, returns
result in output array. Both arrays must be doubles. n=array length,
w = windowing width, w_flag (0=flat boxcar, 1=ramped boxcar, e.g. [1 2 3 2 1])
*/

int boxcar_smooth(double* input, double* output, int n, int w, int w_flag)
{
	int errorCode=0;
    int tail = 0;
	int i,j;
    int* wt = 0;
    double total,sum;

    if (!errorCode && (w > n/2))
    {
        printf("Boxcar window longer than 1/2 array length...\n");
        printf("Resize window and try again\n");
        errorCode=1;
    }

    /* establish the lengths of the boxcar tails */
    if (!errorCode)
    {
	    if (!(w % 2))
	        w += 1;
	    tail = w/2;
	}
	
	 if (!errorCode)
    {
		wt = (int*) malloc(w * sizeof(int));
		if (!wt)
		{
    		printf("Allocation error in boxcar_smooth... Exiting\n");
    		errorCode=1;
		}
    }

    
    /* when w_flag != 0, use linear ramp to weight tails, 
    otherwise use constant weight */
	if (!errorCode)
	{
	    if (w_flag)
	    {
	        for (i=0 ; i<tail ; i++)
	            wt[i] = i+1;
	        for (i=0 ; i<= tail ; i++)
	            wt[i+tail] = tail + 1 - i;
	    }
	    else
	        for (i=0 ; i<w ; i++)
	            wt[i] = 1;

	   
	    for (i=0 ; i<n ; i++)
	    {
	        total = 0.0;
	        sum = 0.0;
	        if (i < tail)
	       	{
	            for (j=tail - i ; j<w ; j++)
	            {
	                total += input[i+j-tail] * wt[j];
	                sum += (double) wt[j];
	            }
	        }
	        else if ((i >= tail) && (i < n-tail))
	       	{
	            for (j=0 ; j<w ; j++)
	            {
	                total += input[i+j-tail] * wt[j];
	                sum += (double) wt[j];
	            }
	        }
	        else if (i >= n-tail)
	        {
	            for (j=0 ; j<tail+n-i ; j++)
	            {
	                total += input[i+j-tail] * wt[j];
	                sum += (double) wt[j];
	            }
	        }
	        output[i] = total/sum;
	        
	    } /* end for i=nelements */
	    
		free(wt);
		
	} /* end if ok */
	
	return (errorCode);
}   


