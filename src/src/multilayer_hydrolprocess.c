/* 
multilayer_hydrolprocess.c
calculation of soil water content layer by layer taking into account soil hydrological processes 
(precipitation, evaporation, runoff, percolation, diffusion)

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.4
Copyright 2017, D. Hidy [dori.hidy@gmail.com]
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

int multilayer_hydrolprocess(const control_struct* ctrl, const siteconst_struct* sitec, const epconst_struct* epc, 
							 epvar_struct* epv, wstate_struct* ws, wflux_struct* wf)
{
	/* given a list of site constants and the soil water mass (kg/m2),
	this function returns the soil water potential (MPa)
	inputs:


	For further discussion see:
	Cosby, B.J., G.M. Hornberger, R.B. Clapp, and T.R. Ginn, 1984.     

	Balsamo et al 2009 - 
	A Revised Hydrology for the ECMWF Model - Verification from Field Site to Water Storage IFS - JHydromet.pdf

	Chen and Dudhia 2001 - 
	Coupling an Advanced Land Surface-Hydrology Model with the PMM5 Modeling System Part I - MonWRev.pdf*/
	

	/* internal variables */
	double prcp, evap_diff;
	double soilw_hw0, soilw_before;  /* (kgH2O/m2/min) */
	int layer;
	double coeff_soiltype, coeff_soilmoist, RCN, coeff_runoff;
	int ok=1;
	soilw_before=0;

	/* *****************************/
	/* 1. PRECIPITATION AND RUNOFF*/

	/* when the precipitation at the surface exceeds the max. infiltration rate, the excess water is put into surfacerunoff (Balsamo et al. 20008; Eq.(7)) */
	

	prcp = (wf->prcp_to_soilw + wf->snoww_to_soilw + wf->IRG_to_prcp + ws->pond_water);


	/* if the precipitation is greater than critical amount a fixed part of prcp is lost due to runoff (based on Campbell and Diaz, 1988) */

	RCN = sitec->RCN;
	coeff_soiltype  = 254*(100 / RCN - 1);

	coeff_soilmoist = 0.15 * ((sitec->vwc_sat[0] - epv->vwc[0]) / (sitec->vwc_sat[0]  - sitec->vwc_hw[0]));

	coeff_runoff = coeff_soiltype * coeff_soilmoist;

	if (prcp > coeff_runoff)
	{
		wf->prcp_to_runoff = pow(prcp - coeff_runoff, 2) / (prcp + (1 - coeff_soilmoist)*coeff_soiltype);

	}
	else
	{
		wf->prcp_to_runoff = 0;
	}


	/* ********************************/
	/* 2. PERCOLATION  AND DIFFUSION  */
	if (epc->SHCM_flag == 0)
	{
		if (ok && richards(sitec, epc, epv, ws, wf))
		{
			printf("Error in richards() from bgc()\n");
			ok=0; 
		} 
		#ifdef DEBUG
					printf("%d\t%d\tdone richards\n",simyr,yday);
		#endif	
	}
	else
	{
		if (ok && tipping(sitec, epc, epv, ws, wf))
		{
			printf("Error in tipping() from bgc()\n");
			ok=0;
		} 
		#ifdef DEBUG
					printf("%d\t%d\tdone tipping\n",simyr,yday);
		#endif	
	}

	/* ---------------------------------------------------------*/	

	/* ********************************************/
	/* 3. EVAPORATION */
	

	/* actual soil water content at theoretical lower limit of water content: hygroscopic water content */
	soilw_hw0 = sitec->vwc_hw[0] * sitec->soillayer_thickness[0] * water_density;

	/* evap_diff: control parameter to avoid negative soil water content (due to overestimated evaporation + dry soil) */
	evap_diff = ws->soilw[0] - wf->soilw_evap - soilw_hw0;

	/* theoretical lower limit of water content: hygroscopic water content. */
	if (evap_diff < 0)
	{
		wf->soilw_evap += evap_diff;
	}
	
	ws->soilw[0] -= wf->soilw_evap;
	epv->vwc[0]  = ws->soilw[0] / water_density / sitec->soillayer_thickness[0];


	/* ********************************/
	/* BOTTOM LAYER IS SPECIAL: percolated water is net loss for the system, water content does not change */
	
	
	if (sitec->gwd_act == DATA_GAP || ( sitec->gwd_act != DATA_GAP && sitec->gwd_act > sitec->soillayer_depth[N_SOILLAYERS-1]))
	{
		soilw_before              = ws->soilw[N_SOILLAYERS-1];
		epv->vwc[N_SOILLAYERS-1]  = sitec->vwc_fc[N_SOILLAYERS-1];
		ws->soilw[N_SOILLAYERS-1] = sitec->vwc_fc[N_SOILLAYERS-1] * (sitec->soillayer_thickness[N_SOILLAYERS-1]) * 1000.0;
		ws->deeppercolation_snk += (soilw_before - ws->soilw[N_SOILLAYERS-1]);
	}

	
	/* ********************************/
	/* 5. POND WATER EVAPORATION: water stored on surface which can not infiltrated because of saturation */
	if (ws->pond_water > 0)
	{
		if (ctrl->onscreen && ctrl->spinup == 0) printf("INFORMATION: pond water on soil surface on yday: %3i\n", ctrl->yday);
		if (wf->pot_evap < ws->pond_water)
			wf->pondw_evap = wf->pot_evap;
		else 
			wf->pondw_evap = ws->pond_water;
	}


	/* ********************************/
	/* 6. CONTROL - unrealistic VWC content (higher than saturation value or less then hygroscopic) */

	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		if (epv->vwc[layer] < sitec->vwc_hw[layer])       
		{
			if (sitec->vwc_hw[layer] - epv->vwc[layer] < 1e-3)
			{
				wf->soilw_percolated[layer] -= (sitec->vwc_hw[layer] - epv->vwc[layer]);
				ws->deeppercolation_snk -= (sitec->vwc_hw[layer] - epv->vwc[layer]);
				epv->vwc[layer] = sitec->vwc_hw[layer];
				ws->soilw[layer] = epv->vwc[layer] * water_density * sitec->soillayer_thickness[layer];
			}
			else
			{
				printf("FATAL ERROR in soil water content calculation (multilayer_hydrolprocess.c)\n");
				ok=0;	
			}

		}
		
		if (epv->vwc[layer] > sitec->vwc_sat[layer])       
		{
			if (epv->vwc[layer] - sitec->vwc_sat[layer] < 1e-2)
			{
				wf->soilw_percolated[layer] += (epv->vwc[layer] - sitec->vwc_sat[layer]);
				ws->deeppercolation_snk += (epv->vwc[layer] - sitec->vwc_sat[layer]);
				epv->vwc[layer] = sitec->vwc_sat[layer];
				ws->soilw[layer] = epv->vwc[layer] * water_density * sitec->soillayer_thickness[layer];
			}
			else
			{
				printf("FATAL ERROR in soil water content calculation (multilayer_hydrolprocess)\n");
				ok=0;	
			}

		}
	}

	return (!ok);
}