 /*
tipping.c
Tipping model for INFILT simulation()

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
BBGC MuSo v4
Copyright 2000, Peter E. Thornton
Numerical Terradynamics Simulation Group
Copyright 2014, D. Hidy (dori.hidy@gmail.com)
Hungarian Science Academy
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
	double INFILT, diff;


	double drain_coeff = 0.5; //ratio of the drained of water of a given day

	double vwc, soilw_sat0, pondw_to_soilw;
	double HOLD, DRAIN, DRMX, ESWi0, ESWi1, THETi0, THETi1, inner, DBAR, GRAD, FLOW, conduct, conduct_sat;

	double m_to_cm, dz0, dz1, n_sec_in_hour, soilw_sat1;

	double DRN[N_SOILLAYERS]; /* drainage rate throug soil layer (cm/day) */


	m_to_cm   = 100;
	n_sec_in_hour = n_sec_in_day/n_hour_in_day;
	

	/* --------------------------------------------------------------------------------------------------------*/
	/* 1. POND WATER INFILT: in case of pont water possible water flux from pond to the first layer */

	if (ws->pond_water > 0)
	{
		soilw_sat0     = sitec->vwc_sat[0] * sitec->soillayer_thickness[0] * water_density;
		if ((soilw_sat0 - ws->soilw[0]) < ws->pond_water)
			pondw_to_soilw = soilw_sat0 - ws->soilw[0];
		else
			pondw_to_soilw = ws->pond_water;

		ws->pond_water -= pondw_to_soilw;
		ws->soilw[0]   += pondw_to_soilw;
		epv->vwc[0]    = ws->soilw[0] / sitec->soillayer_thickness[0] / water_density;
	
	}

	/* --------------------------------------------------------------------------------------------------------*/
	/* 2. WATER DRAIN */

	/* -------------------------------------------------------*/
	/* 2.1. rainy days */

	if (wf->prcp_to_soilw > 0 || wf->snoww_to_soilw > 0)
	{

		INFILT = 0;

		/* 2.1.1. BEGIN LOOP: layer */
		for  (layer=0 ; layer<N_SOILLAYERS-1 ; layer++) 
		{
	
			vwc = epv->vwc[layer];
			dz0 = sitec->soillayer_thickness[layer]  * m_to_cm;
	
			/* saturated hydraulic conductivity in actual layer (cm/h = m/s * 100 * sec/hour) */
			conduct_sat = sitec->hydr_conduct_sat[layer] 
				                * m_to_cm * n_sec_in_hour;

			/* hydraulic conductivity in actual layer (cm/day = m/s * 100 * n_sec_in_day) */
			conduct = sitec->hydr_conduct_sat[layer] * pow(epv->vwc[layer]/sitec->vwc_sat[layer], 2*(sitec->soil_b[layer])+3)
								* m_to_cm * n_sec_in_day;

			/* soil water conductitvity constans [1/day] */
			drain_coeff = 0.1122 * pow(conduct,0.339);

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
				if (conduct_sat > 0.0 && DRN[layer] > conduct_sat * n_hour_in_day) 
				{
					DRN[layer] = conduct_sat * n_hour_in_day;
					DRAIN      = DRN[layer] + HOLD - INFILT;
				}

				/* state update temporal varialbe */
				vwc = vwc + (INFILT - DRN[layer])/dz0;

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
             
				   if (conduct_sat > 0.0 && DRN[layer] > conduct_sat * n_hour_in_day)
				   {
					  DRN[layer] = conduct_sat * n_hour_in_day;
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
	
			/* saturated hydraulic conductivity in actual layer (cm/h = m/s * 100 * sec/hour) */
			conduct_sat = sitec->hydr_conduct_sat[layer] 
				                * m_to_cm * n_sec_in_hour;

			/* hydraulic conductivity in actual layer (cm/day = m/s * 100 * n_sec_in_day) */
			conduct = sitec->hydr_conduct_sat[layer] * pow(epv->vwc[layer]/sitec->vwc_sat[layer], 2*(sitec->soil_b[layer])+3)
								* m_to_cm * n_sec_in_day;

			/* soil water conductitvity constans [1/day] */
			drain_coeff = 0.1122 * pow(conduct,0.339);


			if (vwc > sitec->vwc_fc[layer])
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
			
		   if (conduct_sat > 0.0 && DRN[layer] > conduct_sat * n_hour_in_day)
		   {
			  DRN[layer] = conduct_sat * n_hour_in_day;
		   }

			/* water flux: cm/day to kg/(m2*day) */
			wf->soilw_percolated[layer] = (DRN[layer] / m_to_cm) * water_density;
 
		} /* END LOOP: VWC_sat flow */

	
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

			THETi0 = (epv->vwc[layer]   - sitec->vwc_wp[layer]);
			THETi1 = (epv->vwc[layer+1] - sitec->vwc_wp[layer+1]);

			inner  = (THETi0 * dz0 + THETi1 * dz1) /((dz0+dz1));
			DBAR   = 0.88 * exp(35.4 * 0.5 * inner);
			DBAR   = MIN(DBAR, 100);

			inner  = (ESWi0 * dz0+ ESWi1 * dz0)/((dz0+dz1));
			GRAD   = (THETi1/ESWi1 - THETi0/ESWi0) * inner;

			FLOW   = DBAR * GRAD/((dz0+dz1)*0.5);

			wf->soilw_diffused[layer] = -1 * (FLOW / m_to_cm) * water_density;

		}	

	}
	else
	{
		wf->soilw_diffused[layer] = 0;
	}

	/* BOUNDARY LAYER IS SPECIAL */
	wf->soilw_percolated[N_SOILLAYERS-1] = 0;
	wf->soilw_diffused[N_SOILLAYERS-1]   = 0;

	/* ----------------------------------------------------------------------------------------------------------*/
	/* STATE UPDATE */

	for (layer=N_SOILLAYERS-2; layer>=0; layer--)
	{	
		if ((epv->vwc[layer+1] - sitec->vwc_sat[layer+1]) > -0.01) 
		{
			wf->soilw_percolated[layer] = 0;
		}
	
		soilw_sat1 = sitec->vwc_sat[layer+1]* sitec->soillayer_thickness[layer+1]   * water_density;
		diff = wf->soilw_percolated[layer] + ws->soilw[layer+1] - soilw_sat1;
		if (diff > 0)
		{
			wf->soilw_percolated[layer] -= diff;
			if (wf->soilw_percolated[layer] < 0)
			{
				wf->soilw_diffused[layer] += wf->soilw_percolated[layer];
				wf->soilw_percolated[layer] = 0;
			}
		}
		ws->soilw[layer]   -= (wf->soilw_percolated[layer] + wf->soilw_diffused[layer]);
		ws->soilw[layer+1] += (wf->soilw_percolated[layer] + wf->soilw_diffused[layer]);

		epv->vwc[layer]    = ws->soilw[layer]  /sitec->soillayer_thickness[layer]  /water_density;
		epv->vwc[layer+1]  = ws->soilw[layer+1]/sitec->soillayer_thickness[layer+1]/water_density;

	}
	
	/* --------------------------------------------------------------------------------------------------------*/
	/* 1. POND WATER INFILT: in case of pont water possible water flux from pond to the first layer */

	
	soilw_sat0 = sitec->vwc_sat[0]* sitec->soillayer_thickness[0]   * water_density;
	diff = soilw_sat0 - ws->soilw[0];
	if (diff < 0)
	{
		ws->pond_water -= diff;
		ws->soilw[0]   =  soilw_sat0;
		epv->vwc[0]    =  ws->soilw[0]  /sitec->soillayer_thickness[0]  /water_density;
				
	}
		
	if (ws->pond_water > 0)
	{
		soilw_sat0     = sitec->vwc_sat[0] * sitec->soillayer_thickness[0] * water_density;
		if ((soilw_sat0 - ws->soilw[0]) < ws->pond_water)
			pondw_to_soilw = soilw_sat0 - ws->soilw[0];
		else
			pondw_to_soilw = ws->pond_water;

		ws->pond_water -= pondw_to_soilw;
		ws->soilw[0]   += pondw_to_soilw;
		epv->vwc[0]    = ws->soilw[0] / sitec->soillayer_thickness[0] / water_density;
	
	}

	return (!ok);

}
