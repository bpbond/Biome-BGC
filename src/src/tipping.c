 /*
tipping.c
Tipping model for INFILT simulation()

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Copyright 2020, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "ini.h"
#include "bgc_struct.h"
#include "bgc_func.h"
#include "bgc_constants.h"
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

int tipping(siteconst_struct* sitec, soilprop_struct* sprop, const epconst_struct* epc, epvar_struct* epv, wstate_struct* ws, wflux_struct* wf)
{

	int errorCode=0;
	int layer=0;
	int ll=0;
	double INFILT;
	double conduct_act;

	double soilw, VWC, soilw_sat1, soilw1,VWCequilib,diffus_limit;
	double EXCESS, HOLD, DRAIN, DRMX, ESWi0, ESWi1, THETi0, THETi1, inner, DBAR, GRAD, FLOW, conduct_sat;

	double m_to_cm, mm_to_cm, dz0, dz1;

	double DRN[N_SOILLAYERS]; /* drainage rate throug soil layer (cm/day) */
	

	m_to_cm   = 100;
	mm_to_cm  = 0.1;



	INFILT = wf->infiltPOT * mm_to_cm;

	/* -------------------------------------------------------*/
	/* 1.1. rainy days */
	

	if (INFILT > 0)
	{

		/* 1.1.1. BEGIN LOOP: layer */
		for  (layer=0 ; layer<N_SOILLAYERS-1; layer++) 
		{
	
			VWC = epv->VWC[layer];
			dz0 = sitec->soillayer_thickness[layer]  * m_to_cm;
	
			/* saturated hydraulic conductivity in actual layer (cm/day = m/s * 100 * sec/day) */
			conduct_sat = sprop->hydrCONDUCTsat[layer] * m_to_cm * nSEC_IN_DAY;
			conduct_act = conduct_sat * pow(VWC/sprop->VWCsat[layer], 2*(sprop->soilB[layer]+3));
	
			/* [cm = m3/m3 * cm */
			HOLD = (sprop->VWCsat[layer] - VWC) * dz0;
			
		
			/* 1.1.2.  IF: INFILT > HOLD */
			if (INFILT > 0.0 && INFILT > HOLD)
			{
				/* drainage from soil profile [cm = m3/m3 * cm ] */
				if (sprop->drain_coeff_mes[layer] != DATA_GAP)
					DRAIN = sprop->drain_coeff[layer] * (sprop->VWCsat[layer] - sprop->VWCfc[layer]) * dz0;
				else
					DRAIN = MIN(sprop->VWCsat[layer] - sprop->VWCfc[layer], conduct_act)  * dz0;

				/* drainage rate throug soil layer (cm/day) */
				DRN[layer] = INFILT - HOLD + DRAIN;

				/* drainage is limited: cm/h * h/day */
				if ((DRN[layer] - conduct_sat) > 0.0) 
				{
					DRN[layer] = conduct_sat;
					DRAIN      = DRN[layer] + HOLD - INFILT;
				}
			
				/* state update temporal varialbe */
				VWC = VWC + (INFILT - DRN[layer])/dz0;

				/* above saturation - */
				if (VWC >= sprop->VWCsat[layer])
				{
					
					EXCESS=(VWC - sprop->VWCsat[layer])*dz0;
					VWC=sprop->VWCsat[layer];
					
					/* if there is excess water, redistribute it in layers above */
					if (EXCESS > 0)
					{
						for (ll=layer-1; ll>=0; ll--)
						{
							dz1						= sitec->soillayer_thickness[ll] * mm_to_cm;
							soilw_sat1				= sprop->VWCsat[ll] * sitec->soillayer_thickness[ll] * water_density * mm_to_cm;
							soilw1					= epv->VWC[ll]       * sitec->soillayer_thickness[ll] * water_density * mm_to_cm;
							HOLD					= MIN(soilw_sat1-soilw1, EXCESS);
							ws->soilw[ll]			+= HOLD/mm_to_cm;
							epv->VWC[ll]			= ws->soilw[ll]/sitec->soillayer_thickness[ll]/water_density;
							DRN[ll]					= MAX(DRN[ll]-EXCESS, 0.0);
							EXCESS					= EXCESS - HOLD;

						}
						/* if too much pondwater -> runoff */
						wf->soilw_to_pondw += EXCESS / mm_to_cm;;
					}		
				}

				INFILT = DRN[layer];
			
			} /* END IF: INFILT > HOLD */
			else
			{ /* 1.1.3. BEGIN ELSE: INFILT < HOLD */

				VWC = VWC + INFILT/dz0;

				
				/* BEGIN IF-ELSE: VWC > FC */
				if (VWC >= sprop->VWCfc[layer])
				{

				   DRAIN = (VWC - sprop->VWCfc[layer]) * sprop->drain_coeff[layer] * dz0;
				  
				   if (sprop->drain_coeff_mes[layer] != DATA_GAP)
					   DRAIN = sprop->drain_coeff[layer] * (VWC - sprop->VWCfc[layer]) * dz0;
				   else
					   DRAIN = MIN(VWC - sprop->VWCfc[layer], conduct_act)  * dz0;
             
				   DRN[layer] = DRAIN;
             
				 
				   /* drainage is limited */
				   if ((DRN[layer] - conduct_sat) > 0.0) 
				   {
					  DRN[layer] = conduct_sat;
					  DRAIN = DRN[layer];
				   }
			
             
				   VWC  = VWC - DRAIN / dz0;
				   INFILT = DRAIN;
				}
				else
				{
				   INFILT = 0.0;
				   DRN[layer]   = 0.0;

				} /* END IF-ELSE: VWC > FC */

			} /* END ELSE: INFILT < HOLD */

			/* water flux: cm/day to kg/(m2*day) */
			wf->soilw_percolated[layer] = (DRN[layer] / m_to_cm) * water_density;

		
			/* state update: with new VWC calcualte soilw */
			soilw = VWC * sitec->soillayer_thickness[layer] * water_density;

			epv->VWC[layer]=VWC;
			ws->soilw[layer]=soilw;
			
			
 
		} /* END FOR (layer) */

		
	}
	/* -------------------------------------------------------*/
	else /* 1.2. rainless days */
	{
		
		/* BEGIN LOOP: VWCsat flow */
		for (layer=0; layer<N_SOILLAYERS-1; layer++)   
		{

			VWC = epv->VWC[layer];
			dz0 = sitec->soillayer_thickness[layer]  * m_to_cm;

			/* saturated hydraulic conductivity in actual layer (cm/day = m/s * 100 * sec/day) */
			conduct_sat = sprop->hydrCONDUCTsat[layer] * m_to_cm * nSEC_IN_DAY;
			conduct_act = conduct_sat * pow(VWC/sprop->VWCsat[layer], 2*(sprop->soilB[layer]+3));
	
			/* saturated hydraulic conductivity in actual layer (cm/day = m/s * 100 * sec/day) */
			conduct_sat = sprop->hydrCONDUCTsat[layer] * m_to_cm * nSEC_IN_DAY;
			

			if (VWC  > sprop->VWCfc[layer])
			{
				DRMX = (VWC-sprop->VWCfc[layer]) * sprop->drain_coeff[layer] * dz0;
				DRMX = MAX(0.0,DRMX);

				if (sprop->drain_coeff_mes[layer] != DATA_GAP)
					DRMX = MAX((VWC-sprop->VWCfc[layer]) * sprop->drain_coeff[layer] * dz0,0);
				else
					DRMX = MIN(VWC - sprop->VWCfc[layer], conduct_act)  * dz0;
			}
			else
				DRMX = 0;

			/* BEGIN IF-ELSE: layer == 0 */
			if (layer == 0)
			{
				DRN[layer] = DRMX;
			}
			else
			{

				if (epv->VWC[layer] < sprop->VWCfc[layer])
					HOLD = (sprop->VWCfc[layer] - epv->VWC[layer]) * dz0;
				else
					HOLD = 0.0;
				
				DRN[layer] = MAX(DRN[layer-1] + DRMX - HOLD,0.0);
			
		
			} 	/* BEGIN IF-ELSE: layer == 0 */
			
		   if ((DRN[layer] - conduct_sat) > 0.0) 
		   {
			  DRN[layer] = conduct_sat;
		   }
		  
 
		} /* END LOOP: VWCsat flow */

		for (layer=N_SOILLAYERS-2; layer >= 0; layer--)
		{
			
				VWC = epv->VWC[layer];
				dz0 = sitec->soillayer_thickness[layer]  * m_to_cm;


				if (layer > 0)
				{
					VWC = epv->VWC[layer] + (DRN[layer-1]-DRN[layer])/ dz0 ;
					if (VWC > sprop->VWCsat[layer])
					{
						DRN[layer-1] = (sprop->VWCsat[layer] - epv->VWC[layer])*dz0 +DRN[layer];
						VWC          = sprop->VWCsat[layer];
					}
				}
				else
				{
					VWC = epv->VWC[layer] - (DRN[layer]/ dz0);
				}

			
                 /* water flux: cm/day to kg/(m2*day) */
			     wf->soilw_percolated[layer] = (DRN[layer] / m_to_cm) * water_density;
				/* state update: with new VWC calcualte soilw */
				epv->VWC[layer]  = VWC;
				ws->soilw[layer] = epv->VWC[layer] * sitec->soillayer_thickness[layer] * water_density;

			
		}
	}



	/* --------------------------------------------------------------------------------------------------------*/
	/* 2. UPWARD WATER MOVEMENT - based on 4M method  */


	if (epc->SHCM_flag == 0)
	{
		for (layer=0; layer<N_SOILLAYERS-2; layer++)
		{
			dz0 = sitec->soillayer_thickness[layer]  * m_to_cm;
			dz1 = sitec->soillayer_thickness[layer+1]* m_to_cm;

			ESWi0 = (sprop->VWCfc[layer]   - sprop->VWCwp[layer]);
			ESWi1 = (sprop->VWCfc[layer+1] - sprop->VWCwp[layer+1]);

			THETi0 = MIN(epv->VWC[layer]   - sprop->VWCwp[layer],   ESWi0);
			THETi1 = MIN(epv->VWC[layer+1] - sprop->VWCwp[layer+1], ESWi1);

			THETi0 = MAX(THETi0,   0);
			THETi1 = MAX(THETi1,   0);

			inner  = (THETi0 * dz0 + THETi1 * dz1) /(dz0+dz1);
	
			DBAR   = sprop->p1diffus_tipping * exp(sprop->p2diffus_tipping * inner);
			DBAR   = MIN(DBAR, sprop->p3diffus_tipping);

			inner  = (ESWi0 * dz0+ ESWi1 * dz1)/((dz0+dz1));
			GRAD   = (THETi1/ESWi1 - THETi0/ESWi0) * inner;
			FLOW   = DBAR * GRAD/(dz0+dz1) * 0.5;

			if (fabs(FLOW) > 0)
				wf->soilw_diffused[layer] = -1*(FLOW / m_to_cm) * water_density;
			else
				wf->soilw_diffused[layer] = 0;
	
			/* tipping diffusion limitation */
			VWCequilib = (epv->VWC[layer] * sitec->soillayer_thickness[layer] + epv->VWC[layer+1] * sitec->soillayer_thickness[layer+1]) /
				         (sitec->soillayer_thickness[layer]+sitec->soillayer_thickness[layer+1]);
			
			diffus_limit = (VWCequilib - epv->VWC[layer+1]) * sitec->soillayer_thickness[layer+1] * water_density * sprop->drain_coeff[layer+1];

			if (fabs(wf->soilw_diffused[layer]) > CRIT_PREC && fabs(wf->soilw_diffused[layer]) > fabs(diffus_limit))
			{
				wf->soilw_diffused[layer] = diffus_limit;
			}
			
			
			ws->soilw[layer]   -= wf->soilw_diffused[layer];
			ws->soilw[layer+1] += wf->soilw_diffused[layer];
			epv->VWC[layer]    =  ws->soilw[layer]   / sitec->soillayer_thickness[layer]   / water_density;
			epv->VWC[layer+1]  =  ws->soilw[layer+1] / sitec->soillayer_thickness[layer+1] / water_density;
		
		}	
	
	}
	else
	{
		wf->soilw_diffused[layer] = 0;
	}

	

	/* ********************************/
	/* 5. BOTTOM LAYER IS SPECIAL 	*/
	ws->soilw[N_SOILLAYERS-1] += wf->soilw_percolated[N_SOILLAYERS-2] + wf->soilw_diffused[N_SOILLAYERS-2];
	epv->VWC[N_SOILLAYERS-1]    =  ws->soilw[N_SOILLAYERS-1]   / sitec->soillayer_thickness[N_SOILLAYERS-1]   / water_density;

	return (errorCode);

}
