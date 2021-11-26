/* 
planting.c
planting  - planting seeds in soil - increase transfer pools

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
#include <time.h>
#include <math.h>
#include "ini.h"     
#include "pointbgc_struct.h"
#include "bgc_struct.h"
#include "pointbgc_func.h"
#include "bgc_constants.h"

int planting(control_struct* ctrl, const siteconst_struct* sitec, const planting_struct* PLT, epconst_struct* epc, 
	         epvar_struct* epv, phenology_struct* phen, cstate_struct* cs, nstate_struct*ns, cflux_struct* cf, nflux_struct* nf)
{


	/* planting parameters */	   

	double seed_quantity,seed_Ccontent;					
	double g_to_kg;
	double prop_leaf, prop_froot, prop_fruit, prop_softstem;
	double propLAYER0, propLAYER1, propLAYER2;
	double total_allocation = epc->alloc_leafc[epc->n_germ_phenophase-1]+epc->alloc_frootc[epc->n_germ_phenophase-1]+
		                      epc->alloc_softstemc[epc->n_germ_phenophase-1]+epc->alloc_fruitc[epc->n_germ_phenophase-1];
	
	int errorCode=0;
	int flag_layerIMP, layer, EPCfromINI; 
	file epc_file;
	int md, year;

	year = ctrl->simstartyear + ctrl->simyr;
	md = PLT->mgmdPLT-1;
	
	flag_layerIMP = 0;
	layer = 1;
	seed_quantity=prop_softstem=0;
	g_to_kg=0.001;

	

	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                                    CALCULATING FLUXES 
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

	
	/* 2. we assume that the transfer pools contain the palnt material of seeds. Therefore planting increase the transfer pools */ 
	if (PLT->PLT_num && md >= 0)
	{
		if (year == PLT->PLTyear_array[md] && ctrl->month == PLT->PLTmonth_array[md] && ctrl->day == PLT->PLTday_array[md]) 
		{
			/* 2.0 new EPC in case of annual changing planting information */
			strcpy(epc_file.name, PLT->filename_array[md]);
			
			/* read ecophysiological constants */
			EPCfromINI = 0;
			if (!errorCode && epc_init(epc_file, epc, ctrl, EPCfromINI))
			{
				printf("ERROR in EPC file reading (from PLANTING file)\n");
				errorCode=1;
			}

			if (epc->n_emerg_phenophase == 0 || epc->n_germ_phenophase == 0)
			{
				printf("\n");
				printf("ERROR in setting of emergence and/or germination phenophase. Please, check EPC files\n");
				errorCode=1; 
			} 

			/* plant type determination based on EPC file header */
			if (!errorCode && planttype_determination(ctrl, phen))
			{
				printf("\n");
				printf("ERROR in planttype_determination() in planting.c\n");
				errorCode=1; 
			} 

			seed_quantity = PLT->n_seedlings_array[md] * (PLT->weight_1000seed_array[md] * g_to_kg / 1000);	/* n/m2 * (g/1000n) -> kg seed/m2 */
			seed_Ccontent = PLT->seed_carbon_array[md]/100;														/* change unit: % to number */

			epv->germ_depth = PLT->germ_depth_array[md]; 

			cs->fruitC_HRV          = 0;
			cs->vegC_HRV          = 0;

			/* 2.2 germination depth and layer */
			while (!flag_layerIMP)
			{
				if (epv->germ_depth < sitec->soillayer_depth[0])
				{
					epv->germ_layer = 0;
					flag_layerIMP = 1;
				}

				if (epv->germ_depth >= sitec->soillayer_depth[layer-1] && epv->germ_depth < sitec->soillayer_depth[layer])
				{
					epv->germ_layer = layer;
					flag_layerIMP = 1;
				}
				else
					layer += 1;
			}
	
		}

	}

	if (seed_quantity && epc->n_germ_phenophase)
	{
		/* 2.3 Allocation is calculated based on  EPC alloc.params */
		prop_leaf     = epc->alloc_leafc[epc->n_germ_phenophase-1]    / total_allocation;											
		prop_froot    = epc->alloc_frootc[epc->n_germ_phenophase-1]   / total_allocation;	
		prop_fruit    = epc->alloc_fruitc[epc->n_germ_phenophase-1]   / total_allocation;	
		prop_softstem = epc->alloc_softstemc[epc->n_germ_phenophase-1]/ total_allocation;	

		if (epc->leaf_cn)
		{
			cf->leafc_transfer_from_PLT  = (seed_quantity * prop_leaf)  * seed_Ccontent;
			nf->leafn_transfer_from_PLT  =  cf->leafc_transfer_from_PLT  / epc->leaf_cn;
			cf->STDBc_leaf_to_PLT        = cs->STDBc_leaf;
			nf->STDBn_leaf_to_PLT        = ns->STDBn_leaf;
		}
		
		
		if (epc->froot_cn)
		{
			cf->frootc_transfer_from_PLT = (seed_quantity * prop_froot) * seed_Ccontent;
			nf->frootn_transfer_from_PLT =  cf->frootc_transfer_from_PLT / epc->froot_cn;
			cf->STDBc_froot_to_PLT        = cs->STDBc_froot;
			nf->STDBn_froot_to_PLT        = ns->STDBn_froot;
		}
		
		
		if (epc->fruit_cn)
		{
			cf->fruitc_transfer_from_PLT = (seed_quantity * prop_fruit) * seed_Ccontent;
			nf->fruitn_transfer_from_PLT =  cf->fruitc_transfer_from_PLT / epc->fruit_cn;
			cf->STDBc_fruit_to_PLT        = cs->STDBc_fruit;
			nf->STDBn_fruit_to_PLT        = ns->STDBn_fruit;
		}
		
		
		if (epc->softstem_cn)
		{
			cf->softstemc_transfer_from_PLT = (seed_quantity * prop_softstem) * seed_Ccontent;
			nf->softstemn_transfer_from_PLT =  cf->softstemc_transfer_from_PLT / epc->softstem_cn;
			cf->STDBc_softstem_to_PLT        = cs->STDBc_softstem;
			nf->STDBn_softstem_to_PLT        = ns->STDBn_softstem;
		}
		
			
		/* 2.4 STATE UPDATE */ 
		/* carbon */
		cs->leafc_transfer += cf->leafc_transfer_from_PLT;
		cs->PLTsrc_C += cf->leafc_transfer_from_PLT;
		cs->frootc_transfer += cf->frootc_transfer_from_PLT;
		cs->PLTsrc_C += cf->frootc_transfer_from_PLT;
		cs->fruitc_transfer += cf->fruitc_transfer_from_PLT;
		cs->PLTsrc_C += cf->fruitc_transfer_from_PLT;
		cs->softstemc_transfer += cf->softstemc_transfer_from_PLT;
		cs->PLTsrc_C += cf->softstemc_transfer_from_PLT;
		
		cs->STDBc_leaf     -=cf->STDBc_leaf_to_PLT;
		cs->STDBc_froot    -=cf->STDBc_froot_to_PLT;
		cs->STDBc_fruit    -=cf->STDBc_fruit_to_PLT;
		cs->STDBc_softstem -=cf->STDBc_softstem_to_PLT;


		/* nitrogen */
		ns->leafn_transfer += nf->leafn_transfer_from_PLT;
		ns->PLTsrc_N += nf->leafn_transfer_from_PLT;
		ns->frootn_transfer += nf->frootn_transfer_from_PLT;
		ns->PLTsrc_N += nf->frootn_transfer_from_PLT;
		ns->fruitn_transfer += nf->fruitn_transfer_from_PLT;
		ns->PLTsrc_N += nf->fruitn_transfer_from_PLT;
		ns->softstemn_transfer += nf->softstemn_transfer_from_PLT;
		ns->PLTsrc_N += nf->softstemn_transfer_from_PLT;

		ns->STDBn_leaf     -=nf->STDBn_leaf_to_PLT;
		ns->STDBn_froot    -=nf->STDBn_froot_to_PLT;
		ns->STDBn_fruit    -=nf->STDBn_fruit_to_PLT;
		ns->STDBn_softstem -=nf->STDBn_softstem_to_PLT;

		/* 2.5. mortality fluxes turn into litter pools: 	aboveground biomass into the top soil layer, belowground biomass divided between soil layers based on their root content */
	
		/* new feature: litter turns into the first AND the second soil layer */
		propLAYER0 = sitec->soillayer_thickness[0]/sitec->soillayer_depth[2];
		propLAYER1 = sitec->soillayer_thickness[1]/sitec->soillayer_depth[2];
		propLAYER2 = sitec->soillayer_thickness[2]/sitec->soillayer_depth[2];


		/* 9.1 aboveground biomass into the top soil layer */

		cs->litr1c[0] += (cf->STDBc_leaf_to_PLT * epc->leaflitr_flab  + cf->STDBc_fruit_to_PLT * epc->fruitlitr_flab  + 
						  cf->STDBc_softstem_to_PLT * epc->softstemlitr_flab) * propLAYER0;
		cs->litr2c[0] += (cf->STDBc_leaf_to_PLT * epc->leaflitr_fucel + cf->STDBc_fruit_to_PLT * epc->fruitlitr_fucel + 
						  cf->STDBc_softstem_to_PLT * epc->softstemlitr_fucel) * propLAYER0;
		cs->litr3c[0] += (cf->STDBc_leaf_to_PLT * epc->leaflitr_fscel + cf->STDBc_fruit_to_PLT * epc->fruitlitr_fscel + 
						  cf->STDBc_softstem_to_PLT * epc->softstemlitr_fscel) * propLAYER0;
		cs->litr4c[0] += (cf->STDBc_leaf_to_PLT * epc->leaflitr_flig  + cf->STDBc_fruit_to_PLT * epc->fruitlitr_flig  + 
						  cf->STDBc_softstem_to_PLT * epc->softstemlitr_flig) * propLAYER0;

		ns->litr1n[0] += (nf->STDBn_leaf_to_PLT * epc->leaflitr_flab  + nf->STDBn_fruit_to_PLT * epc->fruitlitr_flab  + 
						  nf->STDBn_softstem_to_PLT * epc->softstemlitr_flab) * propLAYER0;
		ns->litr2n[0] += (nf->STDBn_leaf_to_PLT * epc->leaflitr_fucel + nf->STDBn_fruit_to_PLT * epc->fruitlitr_fucel + 
						  nf->STDBn_softstem_to_PLT * epc->softstemlitr_fucel) * propLAYER0;
		ns->litr3n[0] += (nf->STDBn_leaf_to_PLT * epc->leaflitr_fscel + nf->STDBn_fruit_to_PLT * epc->fruitlitr_fscel + 
						  nf->STDBn_softstem_to_PLT * epc->softstemlitr_fscel) * propLAYER0;
		ns->litr4n[0] += (nf->STDBn_leaf_to_PLT * epc->leaflitr_flig  + nf->STDBn_fruit_to_PLT * epc->fruitlitr_flig  + 
						  nf->STDBn_softstem_to_PLT * epc->softstemlitr_flig) * propLAYER0;

		cs->litr1c[1] += (cf->STDBc_leaf_to_PLT * epc->leaflitr_flab  + cf->STDBc_fruit_to_PLT * epc->fruitlitr_flab  + 
						  cf->STDBc_softstem_to_PLT * epc->softstemlitr_flab) * propLAYER1;
		cs->litr2c[1] += (cf->STDBc_leaf_to_PLT * epc->leaflitr_fucel + cf->STDBc_fruit_to_PLT * epc->fruitlitr_fucel + 
						  cf->STDBc_softstem_to_PLT * epc->softstemlitr_fucel) * propLAYER1;
		cs->litr3c[1] += (cf->STDBc_leaf_to_PLT * epc->leaflitr_fscel + cf->STDBc_fruit_to_PLT * epc->fruitlitr_fscel + 
						  cf->STDBc_softstem_to_PLT * epc->softstemlitr_fscel) * propLAYER1;
		cs->litr4c[1] += (cf->STDBc_leaf_to_PLT * epc->leaflitr_flig  + cf->STDBc_fruit_to_PLT * epc->fruitlitr_flig  + 
						  cf->STDBc_softstem_to_PLT * epc->softstemlitr_flig) * propLAYER1;

		ns->litr1n[1] += (nf->STDBn_leaf_to_PLT * epc->leaflitr_flab  + nf->STDBn_fruit_to_PLT * epc->fruitlitr_flab  + 
						  nf->STDBn_softstem_to_PLT * epc->softstemlitr_flab) * propLAYER1;
		ns->litr2n[1] += (nf->STDBn_leaf_to_PLT * epc->leaflitr_fucel + nf->STDBn_fruit_to_PLT * epc->fruitlitr_fucel + 
						  nf->STDBn_softstem_to_PLT * epc->softstemlitr_fucel) * propLAYER1;
		ns->litr3n[1] += (nf->STDBn_leaf_to_PLT * epc->leaflitr_fscel + nf->STDBn_fruit_to_PLT * epc->fruitlitr_fscel + 
						  nf->STDBn_softstem_to_PLT * epc->softstemlitr_fscel) * propLAYER1;
		ns->litr4n[1] += (nf->STDBn_leaf_to_PLT * epc->leaflitr_flig  + nf->STDBn_fruit_to_PLT * epc->fruitlitr_flig  + 
						  nf->STDBn_softstem_to_PLT * epc->softstemlitr_flig) * propLAYER1;
	
		cs->litr1c[2] += (cf->STDBc_leaf_to_PLT * epc->leaflitr_flab  + cf->STDBc_fruit_to_PLT * epc->fruitlitr_flab  + 
						  cf->STDBc_softstem_to_PLT * epc->softstemlitr_flab) * propLAYER2;
		cs->litr2c[2] += (cf->STDBc_leaf_to_PLT * epc->leaflitr_fucel + cf->STDBc_fruit_to_PLT * epc->fruitlitr_fucel + 
						  cf->STDBc_softstem_to_PLT * epc->softstemlitr_fucel) * propLAYER2;
		cs->litr3c[2] += (cf->STDBc_leaf_to_PLT * epc->leaflitr_fscel + cf->STDBc_fruit_to_PLT * epc->fruitlitr_fscel + 
						  cf->STDBc_softstem_to_PLT * epc->softstemlitr_fscel) * propLAYER2;
		cs->litr4c[2] += (cf->STDBc_leaf_to_PLT * epc->leaflitr_flig  + cf->STDBc_fruit_to_PLT * epc->fruitlitr_flig  + 
						  cf->STDBc_softstem_to_PLT * epc->softstemlitr_flig) * propLAYER2;

		ns->litr1n[2] += (nf->STDBn_leaf_to_PLT * epc->leaflitr_flab  + nf->STDBn_fruit_to_PLT * epc->fruitlitr_flab  + 
						  nf->STDBn_softstem_to_PLT * epc->softstemlitr_flab) * propLAYER2;
		ns->litr2n[2] += (nf->STDBn_leaf_to_PLT * epc->leaflitr_fucel + nf->STDBn_fruit_to_PLT * epc->fruitlitr_fucel + 
						  nf->STDBn_softstem_to_PLT * epc->softstemlitr_fucel) * propLAYER2;
		ns->litr3n[2] += (nf->STDBn_leaf_to_PLT * epc->leaflitr_fscel + nf->STDBn_fruit_to_PLT * epc->fruitlitr_fscel + 
						  nf->STDBn_softstem_to_PLT * epc->softstemlitr_fscel) * propLAYER2;
		ns->litr4n[2] += (nf->STDBn_leaf_to_PLT * epc->leaflitr_flig  + nf->STDBn_fruit_to_PLT * epc->fruitlitr_flig  + 
						  nf->STDBn_softstem_to_PLT * epc->softstemlitr_flig) * propLAYER2;

		/* 9.2 	belowground biomass divided between soil layers based on their root content */ 
	
		if (epv->rootdepth > CRIT_PREC)
		{
			for (layer=0; layer < N_SOILLAYERS; layer++)
			{
				cs->litr1c[layer]  += cf->STDBc_froot_to_PLT * epc->frootlitr_flab  * epv->rootlength_prop[layer];
				cs->litr2c[layer]  += cf->STDBc_froot_to_PLT * epc->frootlitr_fucel * epv->rootlength_prop[layer];
				cs->litr3c[layer]  += cf->STDBc_froot_to_PLT * epc->frootlitr_fscel * epv->rootlength_prop[layer];
				cs->litr4c[layer]  += cf->STDBc_froot_to_PLT * epc->frootlitr_flig  * epv->rootlength_prop[layer];

				ns->litr1n[layer]  += nf->STDBn_froot_to_PLT * epc->frootlitr_flab  * epv->rootlength_prop[layer];
				ns->litr2n[layer]  += nf->STDBn_froot_to_PLT * epc->frootlitr_fucel * epv->rootlength_prop[layer];
				ns->litr3n[layer]  += nf->STDBn_froot_to_PLT * epc->frootlitr_fscel * epv->rootlength_prop[layer];
				ns->litr4n[layer]  += nf->STDBn_froot_to_PLT * epc->frootlitr_flig  * epv->rootlength_prop[layer];
			}
		}
		else
		{
			for (layer=0; layer < N_SOILLAYERS; layer++)
			{
				cs->litr1c[layer]  += cf->STDBc_froot_to_PLT * epc->frootlitr_flab  * epv->rootlengthLandD_prop[layer];
				cs->litr2c[layer]  += cf->STDBc_froot_to_PLT * epc->frootlitr_fucel * epv->rootlengthLandD_prop[layer];
				cs->litr3c[layer]  += cf->STDBc_froot_to_PLT * epc->frootlitr_fscel * epv->rootlengthLandD_prop[layer];
				cs->litr4c[layer]  += cf->STDBc_froot_to_PLT * epc->frootlitr_flig  * epv->rootlengthLandD_prop[layer];

				ns->litr1n[layer]  += nf->STDBn_froot_to_PLT * epc->frootlitr_flab  * epv->rootlengthLandD_prop[layer];
				ns->litr2n[layer]  += nf->STDBn_froot_to_PLT * epc->frootlitr_fucel * epv->rootlengthLandD_prop[layer];
				ns->litr3n[layer]  += nf->STDBn_froot_to_PLT * epc->frootlitr_fscel * epv->rootlengthLandD_prop[layer];
				ns->litr4n[layer]  += nf->STDBn_froot_to_PLT * epc->frootlitr_flig  * epv->rootlengthLandD_prop[layer];
			}
		}
	}
	




   return (errorCode);
}
	

int planttype_determination(control_struct* ctrl, phenology_struct* phen)
{
	int errorCode = 0;
	char pt1[] = "maize";
	char pt2[] = "winter_wheat";
	char pt3[] = "winter_barley";
	char pt4[] = "sunflower";
	char pt5[] = "canola";
	char pt6[] = "spring_wheat";
	char pt7[] = "spring_barley";
	char pt8[] = "alfalfa";
	char pt9[] = "sugarbeet";
	char pt10[] = "bean";
	char pt11[] = "pea";
	char pt12[] = "potato";
	char pt13[] = "oat";
	char pt14[] = "rye";
	char pt15[] = "sugarcane";
	char pt16[] = "rice";
	char pt17[] = "cotton";
	char pt18[] = "crop_notSpec";
	char pt19[] = "vegetable_notSpec";
	char pt20[] = "c3_grass";
	char pt21[] = "c4_grass";
	char pt22[] = "shrub";
	char pt23[] = "deciduous_forest";
	char pt24[] = "evergreen_forest";
	char pt25[] = "mustard";
	
	phen->planttype =-1;

	/* 	bare soil/fallow:0  */

	if (!errorCode && !strcmp(ctrl->planttypeName, pt1)) phen->planttype = 1;

	if (!errorCode && !strcmp(ctrl->planttypeName, pt2)) phen->planttype = 2;

	if (!errorCode && !strcmp(ctrl->planttypeName, pt3)) phen->planttype = 3;

	if (!errorCode && !strcmp(ctrl->planttypeName, pt4)) phen->planttype = 4;

	if (!errorCode && !strcmp(ctrl->planttypeName, pt5)) phen->planttype = 5;

	if (!errorCode && !strcmp(ctrl->planttypeName, pt6)) phen->planttype = 6;

	if (!errorCode && !strcmp(ctrl->planttypeName, pt7)) phen->planttype = 7;

	if (!errorCode && !strcmp(ctrl->planttypeName, pt8)) phen->planttype = 8;

	if (!errorCode && !strcmp(ctrl->planttypeName, pt9)) phen->planttype = 9;

	if (!errorCode && !strcmp(ctrl->planttypeName, pt10)) phen->planttype = 10;

	if (!errorCode && !strcmp(ctrl->planttypeName, pt11)) phen->planttype = 11;

	if (!errorCode && !strcmp(ctrl->planttypeName, pt12)) phen->planttype = 12;

	if (!errorCode && !strcmp(ctrl->planttypeName, pt13)) phen->planttype = 13;

	if (!errorCode && !strcmp(ctrl->planttypeName, pt14)) phen->planttype = 14;

	if (!errorCode && !strcmp(ctrl->planttypeName, pt15)) phen->planttype = 15;

	if (!errorCode && !strcmp(ctrl->planttypeName, pt16)) phen->planttype = 16;

	if (!errorCode && !strcmp(ctrl->planttypeName, pt17)) phen->planttype = 17;

	if (!errorCode && !strcmp(ctrl->planttypeName, pt18)) phen->planttype = 18;

	if (!errorCode && !strcmp(ctrl->planttypeName, pt19)) phen->planttype = 19;

	if (!errorCode && !strcmp(ctrl->planttypeName, pt20)) phen->planttype = 20;

	if (!errorCode && !strcmp(ctrl->planttypeName, pt21)) phen->planttype = 21;

	if (!errorCode && !strcmp(ctrl->planttypeName, pt22)) phen->planttype = 22;

	if (!errorCode && !strcmp(ctrl->planttypeName, pt23)) phen->planttype = 23;

	if (!errorCode && !strcmp(ctrl->planttypeName, pt24)) phen->planttype = 24;

	if (!errorCode && !strcmp(ctrl->planttypeName, pt25)) phen->planttype = 25;

	return (errorCode);
}