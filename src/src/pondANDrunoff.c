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


int pondANDrunoff(siteconst_struct* sitec, soilprop_struct* sprop, epvar_struct* epv, wstate_struct* ws, wflux_struct* wf)
{

	
	/* internal variables */
	int errorCode, layer, flagEXTRA;
	double RCN, coeff_soiltype, coeff_soilmoist, coeff_runoff, INFILT;
	double soilw_dist, soilwEXTRA;

	 errorCode=layer=flagEXTRA=0;

	/* sum of potential infitration */
	INFILT = (wf->prcp_to_soilw + wf->snoww_to_soilw + wf->canopyw_to_soilw + wf->IRG_to_prcp);

	/* calculation of the amount of water which can still fits into the soil */ 
	
	soilw_dist = (sprop->hydrCONDUCTsat[0]*sitec->soillayer_thickness[0]/sitec->soillayer_depth[1] + 
		          sprop->hydrCONDUCTsat[1]*sitec->soillayer_thickness[1]/sitec->soillayer_depth[1]) * nSEC_IN_DAY;

	soilwEXTRA = sprop->VWCsat[0] * sitec->soillayer_thickness[0] * water_density - epv->VWC[0];
	while (flagEXTRA == 0 && layer < N_SOILLAYERS-1)
	{
		if (soilw_dist > sitec->soillayer_depth[layer])
			soilwEXTRA += sprop->VWCsat[layer+1] * sitec->soillayer_thickness[layer+1] * water_density - epv->VWC[layer+1];
		else
			flagEXTRA = 1;

		layer += 1;
	}
	

	/* --------------------------------------------------------------------------------------------------------*/
	/* 1. POND WATER */

	/* if there is pond water in the area */ 
	if (ws->pondw)
	{
	
		/* PRCP to pond water (limitaion: pondmax) */
		if (ws->pondw + INFILT < sprop->pondmax)
		{
			wf->prcp_to_pondw = INFILT;
		}
		else
		{
			wf->prcp_to_pondw = sprop->pondmax - ws->pondw;
			wf->prcp_to_runoff = ws->pondw + INFILT - sprop->pondmax;
		}

		ws->pondw += wf->prcp_to_pondw;


		/* PRCP to pond water (limitaion: pondmax) */
		if (ws->pondw > soilwEXTRA)
			wf->pondw_to_soilw = soilwEXTRA;
		else
			wf->pondw_to_soilw = ws->pondw;
		
		wf->infiltPOT = wf->pondw_to_soilw;
		ws->pondw -= wf->pondw_to_soilw;


	}
	else
	{
		if (INFILT > 0)
		{
			RCN = sprop->RCN;
			/* if runoff is assumed */
			if (RCN > 0)
			{
				coeff_soiltype  = 254*(100 / RCN - 1);

				coeff_soilmoist = 0.15 * ((sprop->VWCsat[0] - epv->VWC[0]) / (sprop->VWCsat[0]  - sprop->VWChw[0]));

				coeff_runoff = coeff_soiltype * coeff_soilmoist;

				if (INFILT > coeff_runoff)
					wf->prcp_to_runoff = pow(INFILT - coeff_runoff, 2) / (INFILT + (1 - coeff_soilmoist)*coeff_soiltype);
				else
					wf->prcp_to_runoff = 0;

				/* pond water filling: empty space in pond water ->runoff 	 */
				if (sprop->pondmax - ws->pondw > 0 && wf->prcp_to_runoff > 0)
				{
					if (sprop->pondmax - ws->pondw > wf->prcp_to_runoff)
					{
						wf->prcp_to_pondw  = wf->prcp_to_runoff;
						wf->prcp_to_runoff = 0;
					}
					else
					{
						wf->prcp_to_pondw   = sprop->pondmax - ws->pondw;
						wf->prcp_to_runoff -= sprop->pondmax - ws->pondw;			
					}
			
				}
				/* Dunnian runoff */
				if (INFILT - wf->prcp_to_runoff - wf->prcp_to_pondw > soilwEXTRA)
				{
					wf->prcp_to_runoff += INFILT - wf->prcp_to_runoff - wf->prcp_to_pondw - soilwEXTRA;
				}
			}
			/* if NO runoff is assumed */
			else
			{
				/* if empty space in soil is greater than infiltration -> pond water formation (and runoff - if pond water is small) */
				if (INFILT > soilwEXTRA)
				{
					if (sprop->pondmax - ws->pondw > INFILT - soilwEXTRA)
					{
						wf->prcp_to_pondw  = INFILT - soilwEXTRA;
					}
					else
					{
						wf->prcp_to_pondw   = sprop->pondmax - ws->pondw;		
						wf->prcp_to_runoff = INFILT - wf->prcp_to_pondw - soilwEXTRA;
					}
				}

			}

			
			ws->pondw += wf->prcp_to_pondw;
		}


		wf->infiltPOT = (wf->prcp_to_soilw + wf->snoww_to_soilw + wf->canopyw_to_soilw + wf->IRG_to_prcp + wf->pondw_to_soilw - wf->prcp_to_runoff - wf->prcp_to_pondw);
	}


	


	return (errorCode);
}
