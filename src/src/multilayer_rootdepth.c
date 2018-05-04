/* 
multilayer_rootdepth.c
calculation of changing rooting depth based on empirical function 
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0
Copyright 2018, D. Hidy [dori.hidy@gmail.com]
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

int multilayer_rootdepth(const control_struct* ctrl, const phenology_struct* phen, const epconst_struct* epc, const siteconst_struct* sitec, const cstate_struct* cs, epvar_struct* epv)
{

	int ok=1;
	int layer;

	double RLprop_sum1, RLprop_sum2;


	/* initalizing internal variables */

	RLprop_sum1=RLprop_sum2=0.0;


	/* ***************************************************************************************************** */	
	/* 1. Calculating rooting depth in case of non-wwody ecosystems (based on Campbell and Diaz, 1988) 
	      actual rooting depth determines the rootzone depth (epv->n_rootlayers) */
	
	if (cs->frootc) 
		if (cs->frootc < epc->rootlenght_par1)
			epv->rooting_depth = epc->max_rootzone_depth * pow(cs->frootc / epc->rootlenght_par1, epc->rootlenght_par2);
		else
			epv->rooting_depth = epc->max_rootzone_depth;
	else
		epv->rooting_depth = CRIT_PREC;

	if (epc->woody) epv->rooting_depth = epc->max_rootzone_depth;

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
								if (epv->rooting_depth > sitec->soillayer_depth[6])
								{
									if (epv->rooting_depth > sitec->soillayer_depth[7])
									{
										if (epv->rooting_depth > sitec->soillayer_depth[8])
										{	
											epv->n_rootlayers = 10;		
										}
										else 
										{
											epv->n_rootlayers = 9;
										}
									}
									else 
									{
										epv->n_rootlayers = 8;
									}
								}
								else 
								{
									epv->n_rootlayers = 7;
								}
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
		printf("\n");
		printf("ERROR in multilayer_rootdepth: rooting depth is 0\n");
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
							if (epc->max_rootzone_depth > sitec->soillayer_depth[5])
							{
								if (epc->max_rootzone_depth > sitec->soillayer_depth[6])
								{
									if (epc->max_rootzone_depth > sitec->soillayer_depth[7])
									{
										if (epc->max_rootzone_depth > sitec->soillayer_depth[8])
										{
												epv->n_maxrootlayers = 10;
										}
										else 
										{
											epv->n_maxrootlayers = 9;
										}
									}
									else 
									{
										epv->n_maxrootlayers = 8;
									}
								}
								else 
								{
									epv->n_maxrootlayers = 7;
								}
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
		printf("\n");
		printf("ERROR in multilayer_rootdepth: maximum of rooting depth is 0\n");
		ok=0;
	}
	/* ***************************************************************************************************** */	
	/* 4. Calculating the distribution of the root in the soil layers based on empirical function (Jarvis, 1989)*/
	


	/* calculation in active soil layer from 2 active soil layers */
	for (layer =0; layer < N_SOILLAYERS; layer++)
	{
		if (layer < epv->n_rootlayers)
		{
			epv->rootlength_prop[layer]   = epc->rootdistrib_param * (sitec->soillayer_thickness[layer] / epv->rooting_depth) * 
 												  exp(-epc->rootdistrib_param * (sitec->soillayer_midpoint[layer] / epv->rooting_depth));
			RLprop_sum1 += epv->rootlength_prop[layer];
		}
		else
			epv->rootlength_prop[layer]   = 0;
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
	if ((fabs(1. - RLprop_sum2) > 1e-8))
	{
		printf("\n");
		printf("ERROR in multilayer_rootdepth: sum of soillayer_RZportion is not equal to 1.0\n");
	    ok=0;
	}

	/* ***************************************************************************************************** */	
	/* 5. calculation of plant height (based on 4M)*/

	if (epc->woody)
	{
		epv->plant_height = epc->max_plant_height*(1-exp((-5/epc->max_stem_weight)*(cs->livestemc+cs->deadstemc)));
	}
	else
	{
		if (ctrl->PLT_flag)
		{
			epv->plant_height = pow((cs->softstemc/0.08),0.5);
		}
		else
		{
			epv->plant_height = 0.12*epv->proj_lai + 0.15;
		
		}
	}
	if (epv->plant_height > epc->max_plant_height) epv->plant_height = epc->max_plant_height;


	return(!ok);
}

