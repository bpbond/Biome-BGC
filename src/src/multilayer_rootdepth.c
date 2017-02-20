/* 
multilayer_rootdepth.c
Hidy 2011 - calculation of changing rooting depth based on empirical function and state update of rootzone sminn content (sminn_RZ)
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.3
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
#include "bgc_func.h"
#include "bgc_constants.h"

int multilayer_rootdepth(const control_struct* ctrl, const epconst_struct* epc, const siteconst_struct* sitec, 
						 phenology_struct* phen, planting_struct* PLT, harvesting_struct* HRV, 
						 epvar_struct* epv, nstate_struct* ns)
{


	int ok=1;
	int layer, yday, ny;

	double onday, offday, plant_day, matur_day, rootdepthmin, RLprop_sum1, RLprop_sum2, sminn_RZ, maturity_coeff;


	/* initalizing internal variables */

	onday=offday=plant_day=matur_day=rootdepthmin=RLprop_sum1=RLprop_sum2=sminn_RZ=maturity_coeff=0;

	maturity_coeff = epc->maturity_coeff;
	yday           = ctrl->yday;
	onday          = phen->onday;
	offday         = phen->offday;
	rootdepthmin   = CRIT_PREC;

	if(PLT->PLT_flag == 2)
	{
		ny = ctrl->simyr;
	}
	else ny=0;
	
	
	
	/* ***************************************************************************************************** */	
	/* 1. Calculating planting date and maturity date (Campbell and Diaz) based on empirical function 
	      and taking into consideration the day of planting and ploughing */

	/* after harvest - effect of harvest, no effect of planting */

	if (HRV->HRV_flag)
	{
		if (HRV->mgmd >= 0)
		{
			HRV->afterHRV = 1;
			PLT->afterPLT = 0;
		}
	}

	
	/* after planting - effect of planting, no effect of harvest */
	if (PLT->mgmd >= 0) 
	{
		PLT->afterPLT = 1;
		HRV->afterHRV = 0;
	}		

	/* no management in spinup - management setting in spinup INI files refers to transient run */
	if (ctrl->spinup || (ctrl->simyears == 0 && ctrl->yday == 0))
	{
		HRV->afterHRV = 0;
		PLT->afterPLT = 0;
	}

	/* after planting, but before ploughing the onday is the day of the last planting day */
	if (PLT->PLT_flag)
		plant_day = PLT->PLTdays_array[0][ny];
	else
		plant_day = onday;
			
	matur_day = onday + maturity_coeff * (offday - onday);


	/* ***************************************************************************************************** */	
	/* 2. Calculating rooting depth in case of non-wwody ecosystems (based on Campbell and Diaz, 1988) 
	      actual rooting depth determines the rootzone depth (epv->n_rootlayers) */
	
	if (!epc->woody)
	{
		if (yday < offday) 
		{
			if (yday < plant_day || HRV->afterHRV == 1)
				epv->rooting_depth = rootdepthmin;
			
			else
				epv->rooting_depth = epc->max_rootzone_depth * (1./(1 + 44.2 * exp(-8.5*((yday - plant_day)/(matur_day - plant_day)))));
		}
		else 
		{
			if (HRV->afterHRV == 1)
				epv->rooting_depth = rootdepthmin;
			
			else
				epv->rooting_depth = epc->max_rootzone_depth - (yday - offday)/(NDAY_OF_YEAR - offday) * epc->max_rootzone_depth;
			
		}
	}
	else epv->rooting_depth = epc->max_rootzone_depth;

	/* ***************************************************************************************************** */	
	/* 3. Calculating the number of the soil layers in which root can be found. It determines the rootzone depth (epv->n_rootlayers) */
	
	if (epv->rooting_depth > 0)
	{
		if (epv->rooting_depth > sitec->soillayer_depth[0])
		{
			if (epv->rooting_depth > sitec->soillayer_depth[1])
			{	
				if (epv->rooting_depth > sitec->soillayer_depth[2])
				{
					if (epv->rooting_depth > sitec->soillayer_depth[3])
					{
						if (epv->rooting_depth > sitec->soillayer_depth[4])
						{
							if (epv->rooting_depth > sitec->soillayer_depth[5])
							{
								epv->n_rootlayers = 7;
							}
							else 
							{
								epv->n_rootlayers = 6;
							}
						}
						else 
						{
							epv->n_rootlayers = 5;
						}
					}
					else
					{
						epv->n_rootlayers = 4;
					}
				}
				else 
				{
					epv->n_rootlayers = 3;
				}	
			}
			else 
			{
				epv->n_rootlayers = 2;
			}
		}
		else
		{
			epv->n_rootlayers = 1;
		}
	}
	else 
	{
		epv->n_rootlayers = 0;
		printf("Error in multilayer_rootdepth: maximum of rooting depth is 0\n");
		ok=0;
	}
	/* ***************************************************************************************************** */	
	/* 3. Calculating the number of the soil layers in which root can be found. It determines the rootzone depth (epv->n_rootlayers) */
	
	if (epc->max_rootzone_depth > 0)
	{
		if (epc->max_rootzone_depth > sitec->soillayer_depth[0])
		{
			if (epc->max_rootzone_depth > sitec->soillayer_depth[1])
			{	
				if (epc->max_rootzone_depth > sitec->soillayer_depth[2])
				{
					if (epc->max_rootzone_depth > sitec->soillayer_depth[3])
					{
						if (epc->max_rootzone_depth > sitec->soillayer_depth[4])
						{
							if (epv->rooting_depth > sitec->soillayer_depth[5])
							{
								epv->n_maxrootlayers = 7;
							}
							else 
							{
								epv->n_maxrootlayers = 6;
							}
						}
						else 
						{
							epv->n_maxrootlayers = 5;
						}
					}
					else
					{
						epv->n_maxrootlayers = 4;
					}
				}
				else 
				{
					epv->n_maxrootlayers = 3;
				}	
			}
			else 
			{
				epv->n_maxrootlayers = 2;
			}
		}
		else
		{
			epv->n_maxrootlayers = 1;
		}
	}
	else 
	{
		epv->n_maxrootlayers = 0;
		printf("Error in multilayer_rootdepth: maximum of rooting depth is 0\n");
		ok=0;
	}
	/* ***************************************************************************************************** */	
	/* 4. Calculating the distribution of the root in the soil layers based on empirical function (Jarvis, 1989)*/
	
	/* initalization */
	for (layer =0; layer < N_SOILLAYERS; layer++) epv->rootlength_prop[layer]     = 0;   
	
	/* calculation in active soil layer from 2 active soil layers */
	for (layer =0; layer < epv->n_rootlayers; layer++)
	{
		epv->rootlength_prop[layer]   = epc->rootdistrib_param * (sitec->soillayer_thickness[layer] / epv->rooting_depth) * 
 											  exp(-epc->rootdistrib_param * (sitec->soillayer_midpoint[layer] / epv->rooting_depth));
		RLprop_sum1 += epv->rootlength_prop[layer];
	}

	/* correction */
	for (layer =0; layer < N_SOILLAYERS; layer++)
	{
		if (RLprop_sum1 > 0)
			epv->rootlength_prop[layer] = epv->rootlength_prop[layer] / RLprop_sum1;
		else 
            epv->rootlength_prop[0] = 1;

		RLprop_sum2                 += epv->rootlength_prop[layer];

	}
	
	/* control */
	if ((1. - RLprop_sum2 > 1e-8))
	{
		printf("Error in multilayer_rootdepth: sum of soillayer_RZportion is not equal to 1.0\n");
	    ok=0;
	}


	/* ***************************************************************************************************** */	
	/* 5. Calculating the soil mineral N content of rooting zone taking into account changing rooting depth 
		  N elimitated/added to rootzone Ncontent because of the decrease/increase of rootzone depth */
	
	sminn_RZ = 0;
	if (epv->n_rootlayers == 1)
	{
		sminn_RZ = ns->sminn[0];
	}
	else
	{
		for (layer = 0; layer < epv->n_rootlayers-1; layer++)
		{
			sminn_RZ	+= ns->sminn[layer];
		}	
		sminn_RZ	+= ns->sminn[epv->n_rootlayers-1] * (epv->rooting_depth - sitec->soillayer_depth[layer-1]) / sitec->soillayer_thickness[layer];
	}
	ns->sminn_RZ	  = sminn_RZ;

	
	



	return(!ok);
}

