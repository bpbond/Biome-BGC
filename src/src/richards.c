/* 
richards.c
calculation of soil water content layer by layer taking into account soil hydrological processes 
(percolation, diffusion)  using inamic discretization level in order to taking account the change of the diffusivity and conductivity depending on soil moisture 
(after rain event the soil moisture rises suddenly causing high diffusivity and conductivity. If we assumed that these diffusivity 
 and conductivity values were valid all the day we would overestimate the velocity diffusion and percolation process) 

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Copyright 2020, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "ini.h"
#include "bgc_struct.h"
#include "bgc_constants.h"
#include "bgc_func.h"
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))   

int richards(siteconst_struct* sitec, soilprop_struct* sprop, const epconst_struct* epc, epvar_struct* epv, wstate_struct* ws, wflux_struct* wf)
{

	
	/* internal variables */
	int errorCode;
	double INFILT,INFILT_sum, INFILT_act,INFILT_ctrl,EVAP,EVAP_act,EVAP_ctrl,TRANSP_ctrl;
	double transpDEM[N_SOILLAYERS],transpDEM_act[N_SOILLAYERS];
	double VWCequilib,diffus_limit;

	/* diffusion and percolation calculation */
	double D0, D1, Dact;	        /* hydrological diffusion coefficient (m2/s) */

	int layer, n_second, n_sec, discretlevel;

	double CRIT_PRECwater;
	double VWCdiff_max, VWCdiff, diff;
	double localvalue,  exponent_discretlevel;
	double soilw0, VWC0, VWC1, soilw_sat0;
	double Ksat0, Ksat1, Kact;

	double pondw_act, infilt_limit,outflux;
	double pondw_to_runoff,soilw_to_pondw, infilt_to_soilw, infilt_to_pondw, pondw_evap, soilw_evap, pondw_to_soilw;

	double dz0, dz1;
	double transp[N_SOILLAYERS],diffus[N_SOILLAYERS], percol[N_SOILLAYERS];
	
	errorCode = 0;
	CRIT_PRECwater=1e-8;
	
	INFILT=EVAP=INFILT_ctrl=EVAP_act=EVAP_ctrl=TRANSP_ctrl=0;
	outflux=pondw_to_runoff=pondw_to_soilw=soilw_to_pondw=infilt_to_soilw=infilt_to_pondw=pondw_evap=soilw_evap=pondw_act=0;
	for (layer=0 ; layer < N_SOILLAYERS; layer++)
	{
		transp[layer]=0;
		diffus[layer]=0;
		percol[layer]=0;
	}
	ws->timestepRichards=0;

	/* --------------------------------------------------------------------------------------------------------*/
	/* I. INITALIZATION */
	
	n_second=0;
	n_sec=1;
	discretlevel = epc->discretlevel_Richards + 3; /* discretization level: change in VWC 1% -> timestep: second */
	
	INFILT_sum = (wf->prcp_to_soilw + wf->snoww_to_soilw + wf->canopyw_to_soilw + wf->IRG_to_prcp);
	INFILT     = INFILT_sum;
	INFILT_act = INFILT/nSEC_IN_DAY;
	
	pondw_act = ws->pondw;

	EVAP         = wf->soilw_evapPOT;
	EVAP_act     = EVAP/nSEC_IN_DAY;

	for (layer=0 ; layer < N_SOILLAYERS; layer++) 
	{
		wf->soilw_transpDEMAND[layer]  = wf->soilw_transpDEMAND_SUM * epv->rootlength_prop[layer]; 
		transpDEM[layer]               = wf->soilw_transpDEMAND[layer];
		transpDEM_act[layer]           = transpDEM[layer] /nSEC_IN_DAY;
	}
	


	/* --------------------------------------------------------------------------------------------------------*/	
	/* II. CALCULATE PROCESSES WITH DYNAMIC TIME STEP LAYER TO LAYER (except of bottom layer) */

	while (n_second < nSEC_IN_DAY)
	{
	
		VWCdiff_max=0;
		
		for (layer=0 ; layer < N_SOILLAYERS-1; layer++) transp[layer] = 0;

	
		/* ----------------------------------------*/
		/* 1. CALCULATE PROCESSES  LAYER TO LAYER */
		for (layer=0 ; layer < N_SOILLAYERS-1; layer++)
		{	
			/* -------------------*/
			/* 1.0. INITALIZATION */
			dz0        = sitec->soillayer_thickness[layer];
			dz1        = sitec->soillayer_thickness[layer+1];
			soilw0     = ws->soilw[layer];
			VWC0       = epv->VWC[layer];
			VWC1       = epv->VWC[layer+1];
			
			soilw_sat0 = sprop->VWCsat[layer] * dz0 * water_density;
		
			Ksat0 = sprop->hydrCONDUCTsat[layer];
			Ksat1 = sprop->hydrCONDUCTsat[layer+1];

		
			/* ------------------- */
			/* 1.1. INFILTRATION */
  			if (layer == 0) 
			{
				infilt_limit = Ksat0 * water_density * n_sec;

 				if (pondw_act == 0)
				{
					if ((soilw0 + INFILT_act - soilw_sat0) > CRIT_PRECwater)			
					{
						infilt_to_soilw     = soilw_sat0 - soilw0;
						infilt_to_pondw     = INFILT_act - infilt_to_soilw;
					}
					else
					{
						infilt_to_soilw     = INFILT_act;
						infilt_to_pondw     = 0;
					}
					pondw_to_soilw = 0; 
					if (infilt_to_soilw > infilt_limit) infilt_to_soilw = infilt_limit;
				}
				else
				{
					infilt_to_pondw = INFILT_act;
					infilt_to_soilw = 0;
						
					pondw_to_soilw = MIN(pondw_act, infilt_limit);
				
				}
			}

			/* -------------------*/
			/* 1.2. EVAPORATION */
  			if (layer == 0) 
			{
				if (EVAP_act > pondw_act) /* potential evaporation > pondw -> both pondw evaporation */
				{
					pondw_evap = pondw_act;
					soilw_evap = EVAP_act - pondw_evap;
				}
				else /* potential evaporation < pondw -> only pondw is evaporated */
				{
					pondw_evap = EVAP_act;
					soilw_evap = 0;
				}

			}

			/* -------------------*/
			/* 1.3. TRANSPIRATION */

			/* transpiration based on rootlenght proportion */
			transp[layer] = transpDEM_act[layer]; 

			/* -------------------*/
			/* 1.4. PERCOLATION */

			/* conductivity coefficient - theoretical upper limit: saturation value */
			Kact = Ksat0 * pow(VWC0/sprop->VWCsat[layer], 2*(sprop->soilB[layer])+3);

			/* percolation flux */
			percol[layer] = Kact * water_density * n_sec; // kg/m2

			/* -------------------*/
			/* 1.5. DIFFUSION */
		
			/* diffusivity coefficient 	*/
			D0 = (((sprop->soilB[layer]    * Ksat0   * (-100*sprop->PSIsat[layer])))   / VWC0) * pow(VWC0/sprop->VWCsat[layer],  sprop->soilB[layer] +2);
			
			D1 = (((sprop->soilB[layer+1]  * Ksat1   * (-100*sprop->PSIsat[layer+1])))   / VWC1) * pow(VWC1/sprop->VWCsat[layer+1],  sprop->soilB[layer+1] +2);

			Dact = (D0 * dz0/(dz0+dz1) + D1 * dz1/(dz0+dz1));
	
		
			/* diffusion flux  */
			if (fabs(VWC0 - VWC1) > CRIT_PRECwater) 
				diffus[layer]  = (VWC0 - VWC1)/((dz0+dz1)/2) 	* Dact  * water_density * n_sec;
			else
				diffus[layer]  = 0;

			/* limit of diffusion: balanced soil moisture  */
			VWCequilib = (VWC0 * dz0 + VWC1 * dz1) / (dz0+dz1);
			diffus_limit = (VWCequilib - VWC1) * dz1 * water_density;
			if (fabs(diffus[layer]) > CRIT_PRECwater && fabs(diffus[layer]) > fabs(diffus_limit)) diffus[layer]  = diffus_limit;


			
		}


		/* ----------------------------------------*/
		/* 2. UPDATING */

	
		/* -------------------*/
		/* 2.2 SOIL WATER CONTENT */
		for (layer=0 ; layer < N_SOILLAYERS; layer++)
		{
			
			if (layer == 0)
				ws->soilw[layer] = ws->soilw[layer] + infilt_to_soilw + pondw_to_soilw - soilw_evap - transp[layer] - percol[layer] - diffus[layer];
			else
				ws->soilw[layer] = ws->soilw[layer] - transp[layer] + percol[layer-1] + diffus[layer-1] - percol[layer] - diffus[layer];


			diff             = sprop->VWChw[layer] * sitec->soillayer_thickness[layer] * water_density - ws->soilw[layer];
		
			/* control to avoid negative SWC pool */
			if (diff >	0)
			{
				/*  top soil layer: limitation of evaporation AND transpiration */
				if (layer == 0)
				{
					if (soilw_evap+transp[layer] > diff)
					{
						soilw_evap       -= soilw_evap*(diff/(soilw_evap+transp[layer]));
						transp[layer]    -= transp[layer]*(diff/(soilw_evap+transp[layer]));
						ws->soilw[layer] = sprop->VWChw[layer] * sitec->soillayer_thickness[layer] * water_density;
					}
					else
					{
						if (diff - (soilw_evap+transp[layer]) < CRIT_PREC)
						{
							soilw_evap       = 0;
							transp[layer]    = 0;
							ws->soilw[layer] = sprop->VWChw[layer] * sitec->soillayer_thickness[layer] * water_density;
						}
						else
						{
							printf("\n");
							printf("ERROR: negative soil water content\n");
							errorCode=1;
						}
					}
				}
				/* except of top soil layer: limitation of transpiration */
				else
				{
					if (transp[layer]  > diff)
					{
						transp[layer]    -= diff;
						ws->soilw[layer] = sprop->VWChw[layer] * sitec->soillayer_thickness[layer] * water_density;
					}
					else
					{
						if (diff - transp[layer] < CRIT_PREC)
						{
							transp[layer]    = 0;
							ws->soilw[layer] = sprop->VWChw[layer] * sitec->soillayer_thickness[layer] * water_density;
						}
						else
						{
							printf("\n");
							printf("ERROR: negative soil water content\n");
							errorCode=1;
						}
					}
				}
			}

			
			
			
			VWC0 = epv->VWC[layer];

		
			epv->VWC[layer] = ws->soilw[layer] / (water_density * sitec->soillayer_thickness[layer]);
	
		
			wf->soilw_transp[layer]     += transp[layer];
			wf->soilw_percolated[layer] += percol[layer];
			wf->soilw_diffused[layer]   += diffus[layer];

			transpDEM[layer]-= transp[layer];			
			TRANSP_ctrl     += transp[layer];
		

			/* -------------------*/
			/* 2.3. FINDING THE MAXIMAL CHANGE to determine discretization step */
			
			VWCdiff = fabs(VWC0 - epv->VWC[layer]);
			
			if (VWCdiff_max < VWCdiff) VWCdiff_max = VWCdiff;		
		
		
		}

		/* -------------------*/
		/* 2.1 POND WATER */

		/* control to avoid SWC greater than saturation */

		diff             = ws->soilw[0] - sprop->VWCsat[0] * sitec->soillayer_thickness[0] * water_density;
			
		if (diff >	0)
		{
			soilw_to_pondw += diff;
			ws->soilw[0] = sprop->VWCsat[0] * sitec->soillayer_thickness[0] * water_density;
			epv->VWC[0] = ws->soilw[0] / (water_density * sitec->soillayer_thickness[0]);
		}
		pondw_act          += infilt_to_pondw + soilw_to_pondw - pondw_to_soilw - pondw_evap;
		
		if (pondw_act < 0)
		{
			diff            = -1*pondw_act;
			pondw_evap     -= diff * pondw_evap    /(pondw_evap+pondw_to_soilw);
			pondw_to_soilw -= diff * pondw_to_soilw/(pondw_evap+pondw_to_soilw);
			pondw_act       = 0;
		}
		if (pondw_act > sprop->pondmax)
		{
			pondw_to_runoff = pondw_act - sprop->pondmax;
			pondw_act = sprop->pondmax;
		}

		/* 2.2 pondw and FLUX VARIABLES */
			
		wf->infilt_to_pondw += infilt_to_pondw;
		wf->infilt_to_soilw += infilt_to_soilw;
		wf->soilw_to_pondw  += soilw_to_pondw;
		wf->soilw_evap      += soilw_evap;
		wf->pondw_to_soilw  += pondw_to_soilw;
		wf->pondw_evap      += pondw_evap;
		wf->pondw_to_soilw  += pondw_to_soilw;
		wf->pondw_to_runoff += pondw_to_runoff;

		INFILT          -= (infilt_to_soilw + infilt_to_pondw);
		INFILT_ctrl     += (infilt_to_soilw + infilt_to_pondw);

		EVAP           -= (pondw_evap + soilw_evap);
		EVAP_ctrl      += (pondw_evap + soilw_evap);


			
		n_second += n_sec;

		/* ----------------------------------------*/
		/* 3. CALCULATION OF DISCRETE LEVEL: if the maximal change of VWC is not equal to 0 (greater than) -> discretlevel is the function of local value of the change */
		VWCdiff_max = VWCdiff_max/n_sec;
		
		if (epc->discretlevel_Richards < 5)
		{
			if (VWCdiff_max > CRIT_PRECwater)
			{
				localvalue=floor(log10(VWCdiff_max));
				if (localvalue + discretlevel < 0)
					exponent_discretlevel = fabs(localvalue + discretlevel);
				else
					exponent_discretlevel = 0;

				n_sec = (int) pow(10, exponent_discretlevel);
				if (n_sec + n_second > nSEC_IN_DAY) 
					n_sec = nSEC_IN_DAY - n_second;
			}
			else
			{
				n_sec = nSEC_IN_DAY - n_second;
			}
		}
		else
			n_sec = 1;

		VWCdiff_max = 0;

	
		if (n_second < nSEC_IN_DAY)
		{
			INFILT_act = INFILT/(nSEC_IN_DAY-n_second) * n_sec; 
			EVAP_act = EVAP/(nSEC_IN_DAY-n_second) * n_sec;
			for (layer=0 ; layer < N_SOILLAYERS; layer++) transpDEM_act[layer] = transpDEM[layer]/(nSEC_IN_DAY-n_second) * n_sec;
		}
		else
		{
			INFILT_act = 0;
			EVAP_act = 0;
			for (layer=0 ; layer < N_SOILLAYERS; layer++) transpDEM_act[layer] = 0;

			/* pond water formation from the non-infiltrated water */
			if (INFILT_sum - INFILT_ctrl > 0)
			{
				infilt_to_soilw      = INFILT_sum - INFILT_ctrl;
				wf->infilt_to_soilw += infilt_to_soilw;
				pondw_act           += infilt_to_soilw;
				INFILT_ctrl         += infilt_to_soilw;
			}
			
			/* if pond water is greatedr than a maximum height -> runoff */
			if (pondw_act > sprop->pondmax)
			{
				pondw_to_runoff      = pondw_act - sprop->pondmax;
				wf->pondw_to_runoff += pondw_to_runoff;
				pondw_act            = sprop->pondmax;
			}

			/* precision control */
			if (pondw_act > 0 && pondw_act < CRIT_PRECwater)
			{
				wf->pondw_evap += pondw_act;
				pondw_act       = 0;
			}
		}

		ws->timestepRichards=ws->timestepRichards+1;	
	
	}
	
	
	ws->pondw = pondw_act;

	/* --------------------------------------------------------------------------------------------------------*/	
	/* III. CONTROLS OF CALCULATION /

	/* ----------------------------------------*/
	/* 1. control of soil evaporation  */
	if (wf->soilw_evap > wf->soilw_evapPOT)
	{
		diff = wf->soilw_evap - wf->soilw_evapPOT;
		if (diff < CRIT_PRECwater)
		{
			wf->soilw_evap -= diff;
			EVAP_ctrl      -= diff;
			ws->soilw[0]   += diff;
			epv->VWC[0]     = ws->soilw[0] / (water_density * sitec->soillayer_thickness[0]);
		}
		else
		{
			printf("\n");
			printf("WARNING: balance in evaporation calculation\n");
		}
	}

	if (fabs(wf->soilw_evap + wf->pondw_evap - EVAP_ctrl) > CRIT_PRECwater)
	{
	//	printf("\n");
		printf("WARNING: balance problem in evaporation calculation\n");
	}

	/* ----------------------------------------*/
	/* 2. control of transpiration */

	for (layer=0 ; layer < N_SOILLAYERS-1; layer++)
	{
		wf->soilw_transp_SUM          += wf->soilw_transp[layer];
		wf->soilw_transpDEMAND[layer]  = wf->soilw_transpDEMAND_SUM * epv->rootlength_prop[layer]; 
	}

	if (wf->soilw_transp_SUM > wf->soilw_transpDEMAND_SUM)
	{
		diff = wf->soilw_transp_SUM - wf->soilw_transpDEMAND_SUM;
		if (diff < CRIT_PRECwater)
		{
			for (layer=0 ; layer < N_SOILLAYERS-1; layer++) 
			{
				wf->soilw_transp[layer] *= (wf->soilw_transpDEMAND_SUM - diff)/wf->soilw_transpDEMAND_SUM;
				TRANSP_ctrl      -= diff;
				ws->soilw[layer] += diff;
				epv->VWC[layer]   = ws->soilw[layer] / (water_density * sitec->soillayer_thickness[layer]);
			}
		}
		else
		{
			printf("\n");
			printf("ERROR: in transpiration calculation\n");
			errorCode=1;
		}
	}

	if (fabs(wf->soilw_transp_SUM - TRANSP_ctrl) > CRIT_PRECwater)
	{
		//printf("\n");
		printf("WARNING: balance problem in transpiration calculation\n");
	}

	wf->infiltPOT = (wf->prcp_to_soilw + wf->snoww_to_soilw + wf->canopyw_to_soilw + wf->IRG_to_prcp + wf->pondw_to_soilw - wf->prcp_to_runoff - wf->prcp_to_pondw);


	/* ----------------------------------------*/
	/* 2. control of infiltration */
	if (fabs(INFILT_sum - INFILT_ctrl) > CRIT_PRECwater)
	{
		//printf("\n");
		printf("WARNING: balance problem in infiltration calculation\n");
	}


	/* BOTTOM LAYER IS SPECIAL */
	diff = ws->soilw[N_SOILLAYERS-1] - sprop->VWCfc[N_SOILLAYERS-1]* sitec->soillayer_thickness[N_SOILLAYERS-1] * water_density;
	if (diff > 0)
	{
		wf->soilw_percolated[N_SOILLAYERS-1]  = diff;
		ws->soilw[N_SOILLAYERS-1] -= diff;
		epv->VWC[N_SOILLAYERS-1] = ws->soilw[N_SOILLAYERS-1] / (water_density * sitec->soillayer_thickness[layer]);
	}


	return (errorCode);
}

