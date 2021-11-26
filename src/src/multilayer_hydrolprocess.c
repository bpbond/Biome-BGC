/* 
multilayer_hydrolprocess.c
calculation of soil water content layer by layer taking into account soil hydrological processes 
(precipitation, evaporation, runoff, percolation, diffusion)

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

int multilayer_hydrolprocess(control_struct* ctrl, siteconst_struct* sitec, soilprop_struct* sprop, const epconst_struct* epc, 
	                         epvar_struct* epv, wstate_struct* ws, wflux_struct* wf, groundwater_struct* gws)
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
	double VWC_avg, VWC_RZ, PSI_RZ, soilw_RZ, weight, weight_SUM, ratio, hydrCONDUCTsat_avg;
	double soilw_hw, soilw_wp, soilw_sat, transp_diff, transp_diff_SUM, soilw_trans_ctrl, soilw_before;
	double VWCsat_RZ, VWCfc_RZ, VWCwp_RZ, VWChw_RZ, soilw_RZ_avail;
	int layer;
	int errorCode=0;
	soilw_sat=soilw_before=soilw_hw=transp_diff=transp_diff_SUM=soilw_trans_ctrl=VWC_avg=VWC_RZ=PSI_RZ=soilw_RZ=weight=weight_SUM=ratio=soilw_wp=soilw_RZ_avail=0;
	VWCsat_RZ=VWCfc_RZ=VWCwp_RZ=VWChw_RZ=hydrCONDUCTsat_avg=0.0;
	


	if (epc->SHCM_flag == 1)
	{
		if (!errorCode && richards(sitec, sprop, epc, epv, ws, wf))
		{
			printf("\n");
			printf("ERROR in richards() from multilayer_hydrolprocess.c()\n");
			errorCode=1; 
		} 
		#ifdef DEBUG
					printf("%d\t%d\tdone richards\n",simyr,yday);
		#endif	

		if (ws->pondw > 0)
		{
			/* pond_flag: flag of WARNING writing (only at first time) */
			if (!ctrl->pond_flag ) ctrl->pond_flag = 1;
		}
	}
	else
	{
	
		/* *****************************/
		/* 1. INFILTRATION, POND AND RUNOFF*/
		/* when the precipitation at the surface exceeds the max. infiltration rate, the excess water is put into surfacerunoff (Balsamo et al. 20008; Eq.(7)) */
		/* if the precipitation is greater than critical amount a fixed part of infiltPOT is lost due to runoff (based on Campbell and Diaz, 1988) */
		if (!errorCode && pondANDrunoff(sitec,sprop, epv,ws, wf))
		{
			printf("\n");
			printf("ERROR in runoff() from multilayer_hydrolprocess.c()\n");
			errorCode=1; 
		} 
	
		
		/* ********************************************/
		/* 2. EVAPORATION */
		/* calculation of actual evaporation from potential evaporation */
		if (!errorCode && potEVAP_to_actEVAP(ctrl, sitec, sprop, epv, ws, wf))
		{
			printf("ERROR in potEVAP_to_actEVAP() from multilayer_hydrolprocess.c()\n");
			errorCode=1;
		}

		/* ********************************/
		/* 3. PERCOLATION  AND DIFFUSION  */

		
		if (!errorCode && tipping(sitec, sprop, epc, epv, ws, wf))
		{
			printf("\n");
			printf("ERROR in tipping() from multilayer_hydrolprocess.c()\n");
			errorCode=1;
		} 
		#ifdef DEBUG
					printf("%d\t%d\tdone tipping\n",simyr,yday);
		#endif	

	
		/* ********************************************/
		/* 4. TRANSPIRATION */
		/* calculate the part-transpiration from total transpiration */
		if (!errorCode && multilayer_transpiration(ctrl, sitec, sprop, epv, ws, wf))
		{
			printf("ERROR in multilayer_transpiration() from multilayer_hydrolprocess.c()\n");
			errorCode=1;
		}
		#ifdef DEBUG
			printf("%d\t%d\tdone multilayer_transpiration\n",simyr,yday);
		#endif

			
		/* ********************************************/
		/* 5. groundwater calculation */
	
		if (!errorCode && groundwater(ctrl, sitec, sprop, epv, ws, wf, gws))
		{
			printf("ERROR in groundwater() from bgc.c\n");
			errorCode=523;
		}

	#ifdef DEBUG
				printf("%d\t%d\tdone groundwater\n",simyr,yday);
	#endif	

		/* water from GW to top soil layer to evaporation limitation calculation (value from previous day) */
		wf->soilw_from_GW0 = wf->soilw_from_GW[0];

		/* -----------------------------*/
		/*  6. POND WATER from soil */
		if (ws->pondw + wf->soilw_to_pondw + wf->GW_to_pondw  > sprop->pondmax)
		{
			wf->prcp_to_runoff += ws->pondw + wf->GW_to_pondw + wf->soilw_to_pondw  - sprop->pondmax;
			ws->pondw = sprop->pondmax;
			wf->soilw_to_pondw = sprop->pondmax - ws->pondw;
		}
		else
			ws->pondw += wf->soilw_to_pondw + wf->GW_to_pondw;

	}

	

	/* evaportanspiration calculation */	
	
	wf->evapotransp = wf->canopyw_evap + wf->soilw_evap + wf->soilw_transp_SUM + wf->snoww_subl + wf->pondw_evap;
	wf->PET         = wf->soilw_evapPOT + wf->soilw_transPOT;


	

	/* ********************************/
	/* 6. BOTTOM LAYER IS SPECIAL: percolated water is net loss for the system, water content does not change 	*/
	
	if (sprop->GWD == DATA_GAP || (sprop->GWlayer == DATA_GAP))
	{
		soilw_before              = ws->soilw[N_SOILLAYERS-1];
		epv->VWC[N_SOILLAYERS-1]  = sprop->VWCfc[N_SOILLAYERS-1];
		ws->soilw[N_SOILLAYERS-1] = sprop->VWCfc[N_SOILLAYERS-1] * (sitec->soillayer_thickness[N_SOILLAYERS-1]) * water_density;
		if (soilw_before > ws->soilw[N_SOILLAYERS-1])
			wf->soilw_percolated[N_SOILLAYERS-1] += soilw_before - ws->soilw[N_SOILLAYERS-1];
		else
			wf->soilw_diffused[N_SOILLAYERS-1] += soilw_before - ws->soilw[N_SOILLAYERS-1];		
	}
	else
	{
		soilw_sat= sprop->VWCsat[N_SOILLAYERS-1] * (sitec->soillayer_thickness[N_SOILLAYERS-1]) * water_density;
		if (ws->soilw[N_SOILLAYERS-1] > soilw_sat)
		{
			wf->GW_recharge[N_SOILLAYERS-1] = ws->soilw[N_SOILLAYERS-1] - soilw_sat;
			ws->soilw[N_SOILLAYERS-1] = soilw_sat;
			epv->VWC[N_SOILLAYERS-1]  = sprop->VWCsat[N_SOILLAYERS-1];

		}
	}

	
	/* ********************************************/
	/* 7. Soilstress calculation based on VWC or transpiration demand-possibitiy */
	
	if (!errorCode && soilstress_calculation(sprop, epc, epv, ws, wf))
	{
		printf("\n");
		printf("ERROR in soilstress_calculation() from multilayer_hydrolprocess.c()\n");
		errorCode=1; 
	} 

	/* ********************************/
	/* 8. CONTROL and calculating averages - unrealistic VWC content (higher than saturation value or less then hygroscopic) */

	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		if (epv->VWC[layer] < sprop->VWChw[layer])       
		{
			if (sprop->VWChw[layer] - epv->VWC[layer] < 1e-3)
			{
				wf->soilw_percolated[layer] -= (sprop->VWChw[layer] - epv->VWC[layer]);
				epv->VWC[layer] = sprop->VWChw[layer];
				ws->soilw[layer] = epv->VWC[layer] * water_density * sitec->soillayer_thickness[layer];
			}
			else
			{
				printf("\n");
				printf("ERROR in soil water content calculation (multilayer_hydrolprocess.c)\n");
				errorCode=1;	
			}

		}
		
		if (epv->VWC[layer] > sprop->VWCsat[layer])       
		{
			if (sprop->GWlayer == layer)
			{
				wf->GW_recharge[layer] = epv->VWC[layer] - sprop->VWCsat[layer];
				epv->VWC[layer] = sprop->VWCsat[layer];
				ws->soilw[layer] = epv->VWC[layer] * water_density * sitec->soillayer_thickness[layer];
			}
			else
			{
				if (epv->VWC[layer] - sprop->VWCsat[layer] < 1e-2)
				{
					wf->soilw_percolated[layer] += (epv->VWC[layer] - sprop->VWCsat[layer]);
					epv->VWC[layer] = sprop->VWCsat[layer];
					ws->soilw[layer] = epv->VWC[layer] * water_density * sitec->soillayer_thickness[layer];
				}
				else
				{
					printf("\n");
					printf("ERROR in soil water content calculation (multilayer_hydrolprocess)\n");
					errorCode=1;	
				}
			}

		}
		VWC_avg	            += epv->VWC[layer]    * (sitec->soillayer_thickness[layer] / sitec->soillayer_depth[N_SOILLAYERS-1]);
		hydrCONDUCTsat_avg	+= sprop->hydrCONDUCTsat[layer]   * (sitec->soillayer_thickness[layer] / sitec->soillayer_depth[N_SOILLAYERS-1]);
		
		
		/* calculation of rootzone variables - weight of the last layer depends on the depth of the root */

		if (epv->n_rootlayers) 
		{
			weight_SUM += epv->rootlength_prop[layer];

			VWC_RZ			+= epv->VWC[layer]      * epv->rootlength_prop[layer];
			PSI_RZ			+= epv->PSI[layer]      * epv->rootlength_prop[layer];
			VWCsat_RZ		+= sprop->VWCsat[layer] * epv->rootlength_prop[layer];
			VWCfc_RZ		+= sprop->VWCfc[layer]  * epv->rootlength_prop[layer];
			VWCwp_RZ		+= sprop->VWCwp[layer]  * epv->rootlength_prop[layer];
			VWChw_RZ		+= sprop->VWChw[layer]  * epv->rootlength_prop[layer];
			soilw_RZ        += ws->soilw[layer]     * epv->rootlength_prop[layer];
			soilw_RZ_avail  += (sprop->VWCwp[layer] * sitec->soillayer_thickness[layer] * water_density) * epv->rootlength_prop[layer];
		}
		else
		{
			VWC_RZ			= 0;
			PSI_RZ			= 0;
			VWCsat_RZ		= 0;
			VWCfc_RZ		= 0;
			VWCwp_RZ		= 0;
			VWChw_RZ		= 0;
			soilw_RZ        = 0;
			soilw_RZ_avail  = 0;
		}
	}

	if (epv->rootdepth && fabs(1-weight_SUM) > CRIT_PREC)
	{
		printf("ERROR in calculation of rootzone variables (multilayer_hydrolprocess.c) \n");
		errorCode=1;
	}
	epv->VWCsat_RZ = VWCsat_RZ;
	epv->VWCfc_RZ  = VWCfc_RZ;
	epv->VWCwp_RZ  = VWCwp_RZ;
	epv->VWChw_RZ  = VWChw_RZ;
	
	epv->hydrCONDUCTsat_avg = hydrCONDUCTsat_avg;
	epv->VWC_avg = VWC_avg;
	epv->VWC_RZ  = VWC_RZ;
	epv->PSI_RZ  = PSI_RZ;
	ws->soilw_RZ  = soilw_RZ;
	ws->soilw_RZ_avail  = soilw_RZ_avail;
	

	
	return (errorCode);
}