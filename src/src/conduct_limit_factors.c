/* 
conduct_limit_factors.c
calculate the limitation factors of conductance
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.3
Copyright 2016, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "ini.h"
#include "bgc_struct.h"
#include "pointbgc_struct.h"
#include "pointbgc_func.h"
#include "bgc_constants.h"


int conduct_limit_factors(file logfile, const control_struct* ctrl,const siteconst_struct* sitec, const epconst_struct* epc, 
						  epvar_struct* epv)
{
	int ok = 1;
	int layer;
	double soil_b, psi_sat,psi_fc, vwc_sat,vwc_fc,vwc_wp; 
	double psi_crit1, psi_crit2, vwc_crit1, vwc_crit2, vwc_ratio_crit1, vwc_ratio_crit2;


	/* -------------------------------------------*/
	/* control: using vwc_ratio OR psi to calculate conductance limitation  */
	if ((epc->relVWC_crit1  != (double) DATA_GAP && epc->PSI_crit1  != (double) DATA_GAP) ||
		(epc->relVWC_crit2 != (double) DATA_GAP && epc->PSI_crit2 != (double) DATA_GAP))
	{
		printf("ERROR: critical relSWC and PSI data are set simultaneously in EPC file, please check it!\n");
		ok=0;

	}

	/* calculations layer by layer (due to different soil properties) */
	for (layer=0; layer < N_SOILLAYERS; layer++)
	{
		
		soil_b  = sitec->soil_b[layer];
		psi_sat = sitec->psi_sat[layer]; 
		psi_fc  = sitec->psi_fc[layer];
		vwc_sat = sitec->vwc_sat[layer]; 
		vwc_fc  = sitec->vwc_fc[layer]; 
		vwc_wp  = sitec->vwc_wp[layer];


		/* -------------------------------------------*/
		/* if no relSWC data are available: the crit1 and crit2 VWC value is calculated from relPSI
		   if no relPSI data are available: the crit1 and crit2 VWC value is calculated from relSWC  */
		
		if (epc->relVWC_crit1 == (double) DATA_GAP)
		{
			if (epc->PSI_crit1 == (double) DATA_GAP)
			{
				psi_crit1 = psi_fc;  
				vwc_crit1 = vwc_fc;
			}
			else
			{
				psi_crit1 = epc->PSI_crit1;  
				vwc_crit1 = vwc_sat * (log(soil_b) / log(psi_crit1/psi_sat));

				/* CONTROL */
				if (epc->PSI_crit1 < psi_sat)
				{
					if (vwc_crit1 > vwc_sat) vwc_crit1 = vwc_sat;
				}
				else
				{
					printf("ERROR: PSI_crit1 data (EPC file) is greater than saturation value\n");
					ok=0;
				}
				
			}
		}
		else
		{
			vwc_crit1 = vwc_fc * epc->relVWC_crit1;
			psi_crit1 = psi_fc * pow((vwc_crit1/vwc_fc), -1*soil_b);
			/* CONTROL */
			if (vwc_crit1 > vwc_sat)
			{
				printf("ERROR: VWC_crit1 data (calculated from relVWC_crit1 in EPC file) is greater than saturation value\n");
				ok=0;
			}
			
		}
		


		if (epc->relVWC_crit2 == (double) DATA_GAP)
		{
			if (epc->PSI_crit2 == (double) DATA_GAP)
			{
				psi_crit2 = psi_sat;  
				vwc_crit2 = vwc_sat;
			}
			else
			{  
				psi_crit2 = epc->PSI_crit2;  
				vwc_crit2 = vwc_sat * (log(soil_b) / log(psi_crit2/psi_sat));

				/* CONTROL */
				if (epc->PSI_crit2 < psi_sat)
				{
					if (vwc_crit2 > vwc_sat) vwc_crit2 = vwc_sat;
				}
				else
				{
					printf("ERROR: PSI_crit2 data (EPC file) is greater than saturation value\n");
					ok=0;
				}

			}
		}
		else
		{
			vwc_crit2 = vwc_sat * epc->relVWC_crit2;
			psi_crit2 = psi_sat * pow((vwc_crit2/vwc_sat), -1*soil_b);
		
		}

		/* CONTROL */
		if (vwc_crit2 > vwc_sat)
		{
			printf("ERROR: VWC_crit2 data (calculated from relVWC_crit2 in EPC file) is greater than saturation value\n");
			ok=0;
		}

		/* CONTROL */
		if (ok && vwc_crit2 < vwc_crit1)
		{
			printf("ERROR: VWC_crit1 data is greater then VWC_crit2 data (calculated from relVWC_crit2 in EPC file)\n");
			ok=0;
		}
			

		vwc_ratio_crit1  = (vwc_crit1 - vwc_wp)/(vwc_sat - vwc_wp);
		vwc_ratio_crit2  = (vwc_crit2 - vwc_wp)/(vwc_sat - vwc_wp);


		epv->psi_crit1[layer]		= psi_crit1;
		epv->psi_crit2[layer]		= psi_crit2;
		epv->vwc_crit1[layer]		= vwc_crit1;
		epv->vwc_crit2[layer]		= vwc_crit2;
		epv->vwc_ratio_crit1[layer] = vwc_ratio_crit1;
		epv->vwc_ratio_crit2[layer] = vwc_ratio_crit2;

	}
	if (ctrl->spinup < 2) fprintf(logfile.ptr, "Limitation values of SWC (m3/m3) and PSI (MPa) in top soil layer\n");  
	if (ctrl->spinup < 2) fprintf(logfile.ptr, "SWC (limit1 and limit2):  %12.3f %12.3f\n", epv->vwc_crit1[0], epv->vwc_crit2[0]);
	if (ctrl->spinup < 2) fprintf(logfile.ptr, "PSI (limit1 and limit2):  %12.4f %12.4f\n", epv->psi_crit1[0], epv->psi_crit2[0]);
	if (ctrl->spinup < 2) fprintf(logfile.ptr, " \n");
		
	return (!ok);
}
