/* 
richards.c
calculation of soil water content layer by layer taking into account soil hydrological processes 
(percolation, diffusion)  using inamic discretization level in order to taking account the change of the diffusivity and conductivity depending on soil moisture 
(after rain event the soil moisture rises suddenly causing high diffusivity and conductivity. If we assumed that these diffusivity 
 and conductivity values were valid all the day we would overestimate the velocity diffusion and percolation process) 

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
BBGC MuSo v4
Copyright 2014, D. Hidy (dori.hidy@gmail.com)
Hungarian Academy of Sciences
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "bgc_struct.h"
#include "bgc_constants.h"
#include "bgc_func.h"
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))   

int richards(const siteconst_struct* sitec, const epconst_struct* epc, epvar_struct* epv, wstate_struct* ws, wflux_struct* wf)
{

	
	/* internal variables */
	int layer;
	double pondw_to_soilw;


	/* diffusion and percolation calculation */

	double hydr_diffus0, hydr_diffus1;	  /* hydrological diffusion coefficient (m2/s) */
	double hydr_conduct[N_SOILLAYERS],hydr_diffus[N_SOILLAYERS];  /* hydrological conduction coefficient (m/s) */

	double diffus, percol, wflux, wflux_limit, soilw_sat, vwc_diff_max;
	double localvalue,  exponent_discretlevel;
	double soilw_sat0, dz0, dz1;  /* (kgH2O/m2/min) */
	double soilw0, soilw1, vwc0, vwc1, vwc_diff0, vwc_diff1;
	int ok, n_second, step, n_sec, discretlevel;

	/* INITALIZATION */
	n_second=step=0;
	ok=n_sec=1;
	discretlevel = epc->discretlevel_Richards + 3; /* discretization level: change in VWC 1% -> timestep: second */


	/* ********************************/	
	/*  CALCULATE PROCESSES WITH DYNAMIC TIME STEP (except of bottom layer) */

	while (n_second < n_sec_in_day)
	{
	
		vwc_diff_max=0;
	
		/* ********************************/	
		/* CALCULATE PROCESSES LAYER TO LAYER (except of bottom layer) */
		for (layer=0 ; layer < N_SOILLAYERS-1 ; layer++)
		{
		
			/* 1. INITALITAZION - actual soil water content at theoretical lower and upper limit of water content:  hygroscopic water content and saturation */		
			dz0 = sitec->soillayer_thickness[layer];
			dz1 = sitec->soillayer_thickness[layer+1];
		
			/* -----------------------------*/
			/* 2. POND WATER INFILTRATION: in case of pont water possible water flux from pond to the first layer */
			if (layer == 0 && ws->pond_water > 0)
			{
				soilw_sat0 = sitec->vwc_sat[layer]   * dz0   * water_density;
				if ((soilw_sat0 - ws->soilw[layer]) < ws->pond_water)
					pondw_to_soilw = soilw_sat0 - ws->soilw[layer];
				else
					pondw_to_soilw = ws->pond_water;

				ws->pond_water -= pondw_to_soilw;
				ws->soilw[0]   += pondw_to_soilw;
				epv->vwc[0]    = ws->soilw[layer] / dz0 / water_density;
			}

			/* -----------------------------*/
			/* 3. PERCOLATION */

			/* conductivity coefficient - theoretical upper limit: saturation value */
			hydr_conduct[layer] = sitec->hydr_conduct_sat[layer] * pow(epv->vwc[layer]/sitec->vwc_sat[layer], 2*(sitec->soil_b[layer])+3);
 

			/* percolation flux - ten minute amount */
			percol = hydr_conduct[layer] * water_density;

	
			/* -----------------------------*/
			/* 4. DIFFUSION */
			
			/* diffusivity coefficient */	
			
	       	hydr_diffus0 = (((sitec->soil_b[layer] * sitec->hydr_conduct_sat[layer] * (-100*sitec->psi_sat[layer]))) / 
				             sitec->vwc_sat[layer]) * pow(epv->vwc[layer]/sitec->vwc_sat[layer], sitec->soil_b[layer]+2);
			
			hydr_diffus1 = (((sitec->soil_b[layer+1] * sitec->hydr_conduct_sat[layer+1] * (-100*sitec->psi_sat[layer+1]))) / 
				             sitec->vwc_sat[layer+1]) * pow(epv->vwc[layer+1]/sitec->vwc_sat[layer+1], sitec->soil_b[layer+1]+2);

			hydr_diffus[layer] = (hydr_diffus0 * dz0 + hydr_diffus1 * dz1) /(dz0 + dz1);
	
			/* diffusion flux  */
			diffus = (((epv->vwc[layer] - epv->vwc[layer+1]) / (sitec->soillayer_midpoint[layer+1]-sitec->soillayer_midpoint[layer])) 
						* hydr_diffus[layer]) 	* water_density; 


			/* -----------------------------*/
			/* 5. LIMITATION OF WATER FLUX: SATURATION 
				 (downward movement: if lower layer, upward movement if upper layer is saturated -> no wflux */

		
			if (sitec->vwc_sat[layer+1] - epv->vwc[layer+1] < 1e-3) 
			{
				percol = 0;
				if (diffus > 0) diffus = 0;
			}
		
			if (sitec->vwc_sat[layer] - epv->vwc[layer]   < 1e-3) 
			{	
				if (diffus < 0) diffus = 0;
			}

			wflux       = (percol + diffus)  * n_sec;
			wflux_limit = wflux;
	

			if (wflux > 0)
			{
				soilw_sat = sitec->vwc_sat[layer+1] * water_density * dz1;
				if (ws->soilw[layer+1] + wflux > soilw_sat)
				{
					wflux_limit = soilw_sat - ws->soilw[layer+1];
					percol = wflux_limit / n_sec - diffus;
					wflux  = wflux_limit;

				}
			
			}
			else
			{
				soilw_sat = sitec->vwc_sat[layer] * water_density * dz0;
				if (ws->soilw[layer] - wflux > soilw_sat)
				{
					wflux_limit  = ws->soilw[layer] - soilw_sat;
					diffus = wflux_limit / n_sec - percol;
					wflux  = wflux_limit;

				}
			}

		
			/* -----------------------------*/
			/* 5. FINDING THE MAXIMAL CHANGE to determine discretization step */
		
		
			soilw0 = ws->soilw[layer]   - (percol + diffus);
			vwc0   = soilw0 / dz0 / water_density;
			vwc_diff0 = fabs(epv->vwc[layer]   - vwc0);

			/* bottom layer is special: i+1 layer is he boundary layer of which water content does not change */
			if (layer < N_SOILLAYERS-2)
			{
				soilw1 = ws->soilw[layer+1] + (percol + diffus);
				vwc1   = soilw1 / dz1 / water_density;
				vwc_diff1 = fabs(epv->vwc[layer+1] - vwc1);
		
				if (vwc_diff_max < vwc_diff0 || vwc_diff_max < vwc_diff1) vwc_diff_max = MAX(vwc_diff0, vwc_diff1);			
			}
			else
			{
				if (vwc_diff_max < vwc_diff0) vwc_diff_max = vwc_diff0;
			}
		
			/* -----------------------------*/
			/* 6. UPDATING STATE VARIABLES    */

	
			ws->soilw[layer]    -= wflux;
			epv->vwc[layer]   = ws->soilw[layer]   / dz0 / water_density;

			/* 6.1 bottom layer is special:: i+1 layer is he boundary layer of which water content does not change */
			if (layer < N_SOILLAYERS-2)
			{
				ws->soilw[layer+1]  += wflux;
				epv->vwc[layer+1] = ws->soilw[layer+1] / dz1 / water_density;
			}
			

			wf->soilw_percolated[layer] += percol * n_sec;
			wf->soilw_diffused[layer]   += diffus * n_sec;



		}


		/* if the maximal change of VWC is not equal to 0 (greater than) -> discretlevel is the function of local value of the change */
		if (vwc_diff_max > 0)
		{
			localvalue=floor(log10(vwc_diff_max));
			if (localvalue + discretlevel < 0)
				exponent_discretlevel = fabs(localvalue + discretlevel);
			else
				exponent_discretlevel = 0;

			n_sec = (int) pow(10, exponent_discretlevel);
		}
		else
		{
			n_sec = n_sec_in_day;
		}


		/* hydr_conduct and hydr_diffus at the beginning of the calculation  */
		if (step == 0)
		{	for (layer = 0; layer < N_SOILLAYERS; layer++)
			{
				epv->hydr_conduct_S[layer] = hydr_conduct[layer];
				epv->hydr_diffus_S[layer] = hydr_diffus[layer];
			}
		}

		if (n_second + n_sec >= n_sec_in_day) 
		{
			n_sec = n_sec_in_day - n_second;
			/* hydr_conduct and hydr_diffus at the end of the calculation  */
			for (layer = 0; layer < N_SOILLAYERS; layer++)
			{
				epv->hydr_conduct_E[layer] = hydr_conduct[layer];
				epv->hydr_diffus_E[layer] = hydr_diffus[layer];
			}
		}
		else
		{
		
			step=step+1;
		}
		
		n_second += n_sec;


	

	}



	return (!ok);
}
