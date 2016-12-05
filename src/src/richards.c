/* 
richards.c
calculation of soil water content layer by layer taking into account soil hydrological processes 
(percolation, diffusion)  using inamic discretization level in order to taking account the change of the diffusivity and conductivity depending on soil moisture 
(after rain event the soil moisture rises suddenly causing high diffusivity and conductivity. If we assumed that these diffusivity 
 and conductivity values were valid all the day we would overestimate the velocity diffusion and percolation process) 

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.2
Copyright 2016, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
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
	double INFILT, DRAIN;


	/* diffusion and percolation calculation */

	double hydr_diffus0, hydr_diffus1;	  /* hydrological diffusion coefficient (m2/s) */
	double hydr_conduct[N_SOILLAYERS],hydr_diffus[N_SOILLAYERS];  /* hydrological conduction coefficient (m/s) */

	double diffus, percol, wflux,  vwc_diff_max;
	double localvalue,  exponent_discretlevel;
	double soilw0, soilw1, vwc0, vwc1, vwc_diff0, vwc_diff1, diff0, diff1, limit;
	double soilw_sat0, soilw_sat1, soilw_hw0, soilw_fc0,  soilw_hw1, soilw_fc1, dz0, dz1;
	double soilw_act[N_SOILLAYERS], vwc_act[N_SOILLAYERS];
	int ok, n_second, step, n_sec, discretlevel;

	/* INITALIZATION */
	n_second=step=0;
	ok=n_sec=1;
	discretlevel = epc->discretlevel_Richards + 3; /* discretization level: change in VWC 1% -> timestep: second */



	/* --------------------------------------------------------------------------------------------------------*/
	/* INITALIZATION */

	for (layer=0 ; layer < N_SOILLAYERS ; layer++)
	{
		soilw_act[layer] = ws->soilw[layer];
		vwc_act[layer]   = epv->vwc[layer];
	}
	
	/* ********************************/
	/* 1. WATER DRAIN */


	wf->pondw_to_soilw = ws->pond_water;
	INFILT = (wf->canopyw_to_soilw + wf->prcp_to_soilw + wf->snoww_to_soilw + wf->IRG_to_prcp + wf->pondw_to_soilw - wf->prcp_to_runoff);
	  

	/* ********************************/	
	/* 2. CALCULATE PROCESSES WITH DYNAMIC TIME STEP (except of bottom layer) */

	while (n_second < n_sec_in_day)
	{

		DRAIN = INFILT;
		soilw_sat0 = sitec->vwc_sat[0]  * sitec->soillayer_thickness[0] * water_density;
		if (DRAIN > soilw_sat0 - soilw_act[0]) 
		{
			DRAIN = soilw_sat0 - soilw_act[0];
		}
		soilw_act[0] += DRAIN;
		vwc_act[0] = soilw_act[0] / sitec->soillayer_thickness[0] / water_density;
		INFILT -= DRAIN;
		
		
		vwc_diff_max=0;
	
		/* ********************************/	
		/* CALCULATE PROCESSES LAYER TO LAYER (except of bottom layer) */
		for (layer=0 ; layer < N_SOILLAYERS-1 ; layer++)
		{

			/* -----------------------------*/
			/* 2.1. INITALITAZION - actual soil water content at theoretical lower and upper limit of water content:  hygroscopic water content and saturation */		
		
			dz0 = sitec->soillayer_thickness[layer];
			dz1 = sitec->soillayer_thickness[layer+1];
			soilw_hw0  = sitec->vwc_hw[layer]   * dz0 * water_density; 
			soilw_fc0  = sitec->vwc_fc[layer]   * dz0 * water_density;
			soilw_sat0 = sitec->vwc_sat[layer]  * dz0 * water_density;
			soilw_hw1  = sitec->vwc_hw[layer+1] * dz1 * water_density; 
			soilw_fc1  = sitec->vwc_fc[layer+1] * dz1 * water_density;
			soilw_sat1 = sitec->vwc_sat[layer+1]* dz1 * water_density;

			/* -----------------------------*/
			/* 2.2. PERCOLATION */

			/* conductivity coefficient - theoretical upper limit: saturation value */
			hydr_conduct[layer] = sitec->hydr_conduct_sat[layer] * pow(vwc_act[layer]/sitec->vwc_sat[layer], 2*(sitec->soil_b[layer])+3);
 

			/* percolation flux - ten minute amount */
			percol = hydr_conduct[layer] * water_density;

	
	
			/* -----------------------------*/
			/* 2.3. DIFFUSION */
		
			/* diffusivity coefficient 	*/
			
	       	hydr_diffus0 = (((sitec->soil_b[layer] * sitec->hydr_conduct_sat[layer] * (-100*sitec->psi_sat[layer]))) / 
				             sitec->vwc_sat[layer]) * pow(vwc_act[layer]/sitec->vwc_sat[layer], sitec->soil_b[layer]+2);
			
			hydr_diffus1 = (((sitec->soil_b[layer+1] * sitec->hydr_conduct_sat[layer+1] * (-100*sitec->psi_sat[layer+1]))) / 
				             sitec->vwc_sat[layer+1]) * pow(vwc_act[layer+1]/sitec->vwc_sat[layer+1], sitec->soil_b[layer+1]+2);

			hydr_diffus[layer] = (hydr_diffus0 * dz0 + hydr_diffus1 * dz1) /(dz0 + dz1);
	
		
			diffus = (vwc_act[layer] - vwc_act[layer+1]) 	* hydr_diffus[layer] 	* water_density;

		

			/* -----------------------------*/
			/* 2.4. LIMITATION OF WATER FLUX: SATURATION */
			
			wflux       = (percol + diffus)  * n_sec;
			
		
			if (wflux > 0)
			{
				if (soilw_act[layer] > soilw_fc0)
					diff0 = soilw_act[layer] - soilw_fc0;
				else
					diff0 = soilw_act[layer] - soilw_hw0;

				diff1 = soilw_sat1 - soilw_act[layer+1];
				limit = MIN(diff0, diff1);

				if (wflux > limit)
				{
					wflux = limit;
					percol = wflux / n_sec;
					diffus = 0;
				}
			}
			else
			{
				if (soilw_act[layer+1] > soilw_fc1)
					diff1 = soilw_act[layer+1] - soilw_fc1;
				else
					diff1 = soilw_act[layer+1] - soilw_hw1;

				diff0 = soilw_sat0 - soilw_act[layer];

				limit = MIN(diff0, diff1);

				if (fabs(wflux) > limit)
				{
					wflux = -1*limit;
					diffus = wflux / n_sec;
					percol = 0;
				}
			}

		
			/* -----------------------------*/
			/* 2.5. FINDING THE MAXIMAL CHANGE to determine discretization step */
		
		
			soilw0 = soilw_act[layer]   - (percol + diffus);
			vwc0   = soilw0 / dz0 / water_density;
			vwc_diff0 = fabs(vwc_act[layer]   - vwc0);

			/* bottom layer is special: i+1 layer is he boundary layer of which water content does not change */
			if (layer < N_SOILLAYERS-2)
			{
				soilw1 = soilw_act[layer+1] + (percol + diffus);
				vwc1   = soilw1 / dz1 / water_density;
				vwc_diff1 = fabs(vwc_act[layer+1] - vwc1);
		
				if (vwc_diff_max < vwc_diff0 || vwc_diff_max < vwc_diff1) vwc_diff_max = MAX(vwc_diff0, vwc_diff1);			
			}
			else
			{
				if (vwc_diff_max < vwc_diff0) vwc_diff_max = vwc_diff0;
			}
		
			/* -----------------------------*/
			/* 2.6. UPDATING STATE VARIABLES    */

	
			soilw_act[layer]    -= wflux;
			vwc_act[layer]   = soilw_act[layer]   / dz0 / water_density;

			soilw_act[layer+1]  += wflux;
			vwc_act[layer+1] = soilw_act[layer+1] / dz1 / water_density;
			
			wf->soilw_percolated[layer] += percol * n_sec;
			wf->soilw_diffused[layer]   += diffus * n_sec;

		}

		/* ********************************/	
		/* 3. CALCULATION OF DISCRETE LEVEL: if the maximal change of VWC is not equal to 0 (greater than) -> discretlevel is the function of local value of the change */
	
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

		/* ********************************/
		/* 4. CALCULATION OF CONDUCTANCE AND DIFFUSION PARAMETERS */
		
		/* 4.1 hydr_conduct and hydr_diffus at the beginning of the calculation  */
		if (step == 0)
		{	for (layer = 0; layer < N_SOILLAYERS; layer++)
			{
				epv->hydr_conduct_S[layer] = hydr_conduct[layer];
				epv->hydr_diffus_S[layer] = hydr_diffus[layer];
			}
		}

		/* 4.2. hydr_conduct and hydr_diffus at the end of the calculation  */
		if (n_second + n_sec >= n_sec_in_day) 
		{
			n_sec = n_sec_in_day - n_second;
			
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

	/* ********************************/
	/* 5. UPDATE STATE VARIBLES */

	for (layer=0 ; layer < N_SOILLAYERS-1; layer++)
	{
		ws->soilw[layer] = soilw_act[layer];
		epv->vwc[layer]  = vwc_act[layer];
	}
	/* 5.1 bottom layer is special:: i+1 layer is he boundary layer of which water content does not change */
	wf->soilw_percolated[N_SOILLAYERS-1] = soilw_act[N_SOILLAYERS-1] - ws->soilw[N_SOILLAYERS-1];

	return (!ok);
}
