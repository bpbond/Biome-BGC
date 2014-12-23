/* 
smooth.c
functions that perform smoothing on vectors

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "bgc.h"

int run_avg(const double *input, double *output, int n, int w, int w_flag)
{
    /* calculates running averages
    n = length of input and output arrays
    w = width of averaging window
    w_flag : 1=linearly weighted window, 0=constant weighted window
    */

    register int i,j;
    int ok = 1;
    int *wt;
    double total, sum;

    if (w>n)
    {
        bgc_printf(BV_ERROR, "Error: averaging window longer than input array\n");
        ok=0;
    }

    if (ok && (!(wt = (int*) malloc(w * sizeof(int)))))
    {
    	bgc_printf(BV_ERROR, "Allocation error in boxcar_smooth... Exiting\n");
    	ok=0;
    }

    if (ok)
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
    return (!ok);
}

/* boxcar_smooth() performs a windowed smoothing on the input array, returns
result in output array. Both arrays must be doubles. n=array length,
w = windowing width, w_flag (0=flat boxcar, 1=ramped boxcar, e.g. [1 2 3 2 1])
*/

int boxcar_smooth(double* input, double* output, int n, int w, int w_flag)
{
	int ok=1;
    int tail,i,j;
    int* wt;
    double total,sum;

    if (ok && (w > n/2))
    {
        bgc_printf(BV_ERROR, "Boxcar window longer than 1/2 array length...\n");
        bgc_printf(BV_ERROR, "Resize window and try again\n");
        ok=0;
    }

    /* establish the lengths of the boxcar tails */
    if (ok)
    {
	    if (!(w % 2))
	        w += 1;
	    tail = w/2;
	}
	
    if (ok && (!(wt = (int*) malloc(w * sizeof(int)))))
    {
    	bgc_printf(BV_ERROR, "Allocation error in boxcar_smooth... Exiting\n");
    	ok=0;
    }
    
    /* when w_flag != 0, use linear ramp to weight tails, 
    otherwise use constant weight */
	if (ok)
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
	
	return (!ok);
}   


