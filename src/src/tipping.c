 /*
tipping.c
Tipping model for INFILT simulation()

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.2
Copyright 2016, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "bgc_struct.h"
#include "bgc_func.h"
#include "bgc_constants.h"
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

int tipping(const siteconst_struct* sitec, const epconst_struct* epc, epvar_struct* epv, wstate_struct* ws, wflux_struct* wf)
{

	int ok=1;
	int layer=0;
	int ll=0;
	double INFILT, drain_coeff; /* ratio of the drained of water of a given day */

	double soilw, vwc, vwc_sat, soilw_sat1, soilw1;
	double EXCESS, HOLD, DRAIN, DRMX, ESWi0, ESWi1, THETi0, THETi1, inner, DBAR, GRAD, FLOW, conduct, conduct_sat;

	double m_to_cm, mm_to_cm, dz0, dz1, diff;

	double DRN[N_SOILLAYERS]; /* drainage rate throug soil layer (cm/day) */

	double pondmax;

	pondmax=20; /* INI paramter in MuSo v5 */

	diff=INFILT=drain_coeff=soilw=vwc=vwc_sat=soilw_sat1=soilw1=EXCESS=HOLD=DRAIN=DRMX=ESWi0=ESWi1=THETi0=THETi1=inner=0;
	DBAR=GRAD=FLOW=conduct=conduct_sat=m_to_cm=mm_to_cm=dz0=dz1=diff=0;
	
	m_to_cm   = 100;
	mm_to_cm  = 0.1;

	/* --------------------------------------------------------------------------------------------------------*/
	/* 2. WATER DRAIN */


	wf->pondw_to_soilw = ws->pond_water;

	INFILT = (wf->canopyw_to_soilw + wf->prcp_to_soilw + wf->snoww_to_soilw + wf->IRG_to_prcp + wf->pondw_to_soilw - wf->prcp_to_runoff) * mm_to_cm;



	/* -------------------------------------------------------*/
	/* 2.1. rainy days */
	

	if (INFILT > 0)
	{

		/* 2.1.1. BEGIN LOOP: layer */
		for  (layer=0 ; layer<N_SOILLAYERS-1; layer++) 
		{
	
	
			vwc = epv->vwc[layer];
			vwc_sat = sitec->vwc_sat[layer];
			dz0 = sitec->soillayer_thickness[layer]  * m_to_cm;
	
			/* saturated hydraulic conductivity in actual layer (cm/day = m/s * 100 * sec/day): zero - if bottom layer of lower layer is saturated */
			if ((sitec->vwc_sat[layer+1] - epv->vwc[layer+1]) > CRIT_PREC)
			{
				conduct_sat = sitec->hydr_conduct_sat[layer] * m_to_cm * n_sec_in_day;
			}
			else
			{
				conduct_sat = 0;
			}

			/* soil water conductitvity constans [1/day] */
			drain_coeff = 0.1122 * pow(conduct_sat,0.339);

			/* hydraulic conductivity in actual layer (cm/day = m/s * 100 * n_sec_in_day) */
			conduct = conduct_sat * pow(epv->vwc[layer]/sitec->vwc_sat[layer], 2*(sitec->soil_b[layer])+3);
	
			/* [cm = m3/m3 * cm */
			HOLD = (sitec->vwc_sat[layer] - vwc) * dz0;
			
		
			/* 2.1.2.  IF: INFILT > HOLD */
			if (INFILT > 0.0 && INFILT > HOLD)
			{
				/* drainage from soil profile [cm = m3/m3 * cm ] */
				DRAIN = drain_coeff * (sitec->vwc_sat[layer] - sitec->vwc_fc[layer]) * dz0;

				/* drainage rate throug soil layer (cm/day) */
				DRN[layer] = INFILT - HOLD + DRAIN;

				/* drainage is limited: cm/h * h/day */
				if ((DRN[layer] - conduct_sat) > 0.0) 
				{
					DRN[layer] = conduct_sat;
					DRAIN      = DRN[layer] + HOLD - INFILT;
				}
			
				/* state update temporal varialbe */
				vwc = vwc + (INFILT - DRN[layer])/dz0;

				/* above saturation - */
				if (vwc >= vwc_sat)
				{
					
					EXCESS=(vwc - vwc_sat)*dz0;
					vwc=vwc_sat;
					
					/* if there is excess water, redistribute it in layers above */
					if (EXCESS > 0)
					{
						for (ll=layer-1; ll>=0; ll--)
						{
							dz1						= sitec->soillayer_thickness[ll] * mm_to_cm;
							soilw_sat1				= sitec->vwc_sat[ll] * sitec->soillayer_thickness[ll] * water_density * mm_to_cm;
							soilw1					= epv->vwc[ll]       * sitec->soillayer_thickness[ll] * water_density * mm_to_cm;
							HOLD					= MIN(soilw_sat1-soilw1, EXCESS);
							ws->soilw[ll]			+= HOLD/mm_to_cm;
							epv->vwc[ll]			= ws->soilw[ll]/sitec->soillayer_thickness[ll]/water_density;
							DRN[ll]					= MAX(DRN[ll]-EXCESS, 0.0);
							EXCESS					= EXCESS - HOLD;

						}
						/* if too much pondwater -> runoff */
						wf->soilw_to_pondw = EXCESS / mm_to_cm;;
					}
			
				
					
				}

				INFILT = DRN[layer];
			
			} /* END IF: INFILT > HOLD */
			else
			{ /* 2.1.3. BEGIN ELSE: INFILT < HOLD */

				vwc = vwc + INFILT/dz0;

				
				/* BEGIN IF-ELSE: VWC > FC */
				if (vwc >= sitec->vwc_fc[layer])
				{

				   DRAIN = (vwc - sitec->vwc_fc[layer]) * drain_coeff * dz0;
             
				   DRN[layer] = DRAIN;
             
				 
				   /* drainage is limited */
				   if ((DRN[layer] - conduct_sat) > 0.0) 
				   {
					  DRN[layer] = conduct_sat;
					  DRAIN = DRN[layer];
				   }
			
             
				   vwc  = vwc - DRAIN / dz0;
				   INFILT = DRAIN;
				}
				else
				{
				   INFILT = 0.0;
				   DRN[layer]   = 0.0;

				} /* END IF-ELSE: VWC > FC */

			} /* END ELSE: INFILT < HOLD */

		
		
			/* state update: with new vwc calcualte soilw */
			soilw = vwc * sitec->soillayer_thickness[layer] * water_density;

			epv->vwc[layer]=vwc;
			ws->soilw[layer]=soilw;
			/* water flux: cm/day to kg/(m2*day) */
			wf->soilw_percolated[layer] = (DRN[layer] / m_to_cm) * water_density;
			
 
		} /* END FOR (layer) */

		
	}
	/* -------------------------------------------------------*/
	else /* 2.2. rainless days */
	{
		/* BEGIN LOOP: VWC_sat flow */
		for (layer=0; layer<N_SOILLAYERS-1; layer++)   
		{

			vwc = epv->vwc[layer];
			dz0 = sitec->soillayer_thickness[layer]  * m_to_cm;
	
			/* saturated hydraulic conductivity in actual layer (cm/day = m/s * 100 * sec/day): zero - if bottom layer of lower layer is saturated */
			if ((sitec->vwc_sat[layer+1] - epv->vwc[layer+1]) < CRIT_PREC)
			{
				conduct_sat = sitec->hydr_conduct_sat[layer] * m_to_cm * n_sec_in_day;
			}
			else
			{
				conduct_sat = 0;
			}

			/* soil water conductitvity constans [1/day] */
			drain_coeff = 0.1122 * pow(conduct_sat,0.339);

			/* hydraulic conductivity in actual layer (cm/day = m/s * 100 * n_sec_in_day) */
			conduct = conduct_sat * pow(epv->vwc[layer]/sitec->vwc_sat[layer], 2*(sitec->soil_b[layer])+3);


			if ((vwc - sitec->vwc_fc[layer]) > CRIT_PREC)
			{
				DRMX = (vwc-sitec->vwc_fc[layer]) * drain_coeff * dz0;
				DRMX = MAX(0.0,DRMX);
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

				if (epv->vwc[layer] < sitec->vwc_fc[layer])
					HOLD = (sitec->vwc_fc[layer] - epv->vwc[layer]) * dz0;
				else
					HOLD = 0.0;
				
				DRN[layer] = MAX(DRN[layer-1] + DRMX - HOLD,0.0);
			
		
			} 	/* BEGIN IF-ELSE: layer == 0 */
			
		   if ((DRN[layer] - conduct_sat) > 0.0) 
		   {
			  DRN[layer] = conduct_sat;
		   }

			/* water flux: cm/day to kg/(m2*day) */
			wf->soilw_percolated[layer] = (DRN[layer] / m_to_cm) * water_density;

			/* state update: with new vwc calcualte soilw */
			soilw = vwc * sitec->soillayer_thickness[layer] * water_density;
			epv->vwc[layer]=vwc;
			ws->soilw[layer]=soilw;
			
			
 
		} /* END LOOP: VWC_sat flow */

	
	}

	ws->pond_water    += wf->prcp_to_pondw + wf->soilw_to_pondw;
	ws->pond_water    -= wf->pondw_to_soilw;
	ws->pondwater_src += wf->prcp_to_pondw;
	if (ws->pond_water > pondmax)
	{
		wf->prcp_to_runoff += (ws->pond_water - pondmax);
		ws->pond_water = pondmax;
	}


	/* --------------------------------------------------------------------------------------------------------*/
	/* 3. UPWARD WATER MOVEMENT - based on 4M method  */

	if (epc->SHCM_flag != 2)
	{
		for (layer=0; layer<N_SOILLAYERS-2; layer++)
		{
		
			dz0 = sitec->soillayer_thickness[layer]  * m_to_cm;
			dz1 = sitec->soillayer_thickness[layer+1]* m_to_cm;

			ESWi0 = (sitec->vwc_fc[layer]   - sitec->vwc_wp[layer]);
			ESWi1 = (sitec->vwc_fc[layer+1] - sitec->vwc_wp[layer+1]);

			THETi0 = MIN(epv->vwc[layer]   - sitec->vwc_wp[layer],   ESWi0);
			THETi1 = MIN(epv->vwc[layer+1] - sitec->vwc_wp[layer+1], ESWi1);

			
			THETi0 = MAX(THETi0,   0);
			THETi1 = MAX(THETi1,   0);

			inner  = (THETi0 * dz0 + THETi1 * dz1) /(dz0+dz1);
	
			DBAR   = 0.88 * exp(35.4 * 0.5 * inner);
			DBAR   = MIN(DBAR, 100);

			inner  = (ESWi0 * dz0+ ESWi1 * dz1)/((dz0+dz1));
			GRAD   = (THETi1/ESWi1 - THETi0/ESWi0) * inner;
			FLOW   = DBAR * GRAD/(dz0+dz1) * 0.5;

			if (fabs(FLOW) > 0)
				wf->soilw_diffused[layer] = -1*(FLOW / m_to_cm) * water_density;
			else
				wf->soilw_diffused[layer] = 0;
		
			ws->soilw[layer]   -= wf->soilw_diffused[layer];
			ws->soilw[layer+1] += wf->soilw_diffused[layer];
			epv->vwc[layer]    =  ws->soilw[layer]   / sitec->soillayer_thickness[layer]   / water_density;
			epv->vwc[layer+1]  =  ws->soilw[layer+1] / sitec->soillayer_thickness[layer+1] / water_density;
		
		
	

		}	
	
	}
	else
	{
		wf->soilw_diffused[layer] = 0;
	}

	

	
	/* -----------------------------*/
	/* 5. BOUNDARY LAYER IS SPECIAL */
	wf->soilw_percolated[N_SOILLAYERS-1] = 0;
	wf->soilw_diffused[N_SOILLAYERS-1]   = 0;

//	wf->prcp_to_runoff += INFILT / mm_to_cm;
	wf->soilw_percolated[N_SOILLAYERS-1] = INFILT / mm_to_cm;



	return (!ok);

}
