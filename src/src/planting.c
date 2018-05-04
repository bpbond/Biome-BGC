/* 
planting.c
planting  - planting seeds in soil - increase transfer pools

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0.
Copyright 2018, D. Hidy [dori.hidy@gmail.com]
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

int planting(control_struct* ctrl, const epconst_struct* epc, const siteconst_struct* sitec, const planting_struct* PLT, 
	         epvar_struct* epv, phenology_struct* phen, cstate_struct* cs, nstate_struct* ns, cflux_struct* cf, nflux_struct* nf)
{


	/* planting parameters */	   

	double seed_quantity,seed_Ccontent;					
	double utiliz_coeff, g_to_kg;
	double prop_leaf, prop_froot, prop_fruit, prop_softstem;
	
	double total_allocation = epc->alloc_leafc[epc->n_germ_phenophase-1]+epc->alloc_frootc[epc->n_germ_phenophase-1]+
		                      epc->alloc_softstemc[epc->n_germ_phenophase-1]+epc->alloc_fruitc[epc->n_germ_phenophase-1];
	
	int ok=1;
	int ny;
	int flag_layerIMP, layer; 
	int mgmd = PLT->mgmd;
	
	flag_layerIMP = 0;
	layer = 1;
	seed_quantity=utiliz_coeff=prop_softstem=0;
	g_to_kg=0.001;

	/* yearly varied or constant management parameters */
	if(PLT->PLT_flag == 2)
		ny = ctrl->simyr;
	else 
		ny=0;
	
	


	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                                    CALCULATING FLUXES 
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

		
	/* 1. if planting: transfer pools are deplenished (set to zero) on the first simulation day */
	if (ctrl->yday == 0 && ctrl->PLT_flag != 0 && epv->n_actphen == 0)
	{
		cs->HRV_transportC += cs->leafc_transfer;
		cs->HRV_transportC += cs->frootc_transfer;
		cs->HRV_transportC += cs->fruitc_transfer;
		cs->HRV_transportC += cs->softstemc_transfer;

		cs->leafc_transfer = 0;
		cs->frootc_transfer = 0;
		cs->fruitc_transfer = 0;
		cs->softstemc_transfer = 0;

		ns->HRV_transportN += ns->leafn_transfer;
		ns->HRV_transportN += ns->frootn_transfer;
		ns->HRV_transportN += ns->fruitn_transfer;
		ns->HRV_transportN += ns->softstemn_transfer;

		ns->leafn_transfer = 0;
		ns->frootn_transfer = 0;
		ns->fruitn_transfer = 0;
		ns->softstemn_transfer = 0;
	}
	
	/* 2. we assume that the transfer pools contain the palnt material of seeds. Therefore planting increase the transfer pools */ 
	if (mgmd >= 0) 
	{
		/* 2.1 control */
		if (PLT->emerg_rate_array[mgmd][ny] == DATA_GAP  || PLT->germ_depth_array[mgmd][ny] == DATA_GAP   ||  PLT->n_seedlings_array[mgmd][ny] == DATA_GAP ||
			PLT->seed_carbon_array[mgmd][ny] == DATA_GAP  || PLT->weight_1000seed_array[mgmd][ny] == DATA_GAP)
		{
				printf("ERROR in planting parameters in INI or management file (planting.c)\n");
				ok=0;
		}
		
		seed_quantity = PLT->n_seedlings_array[mgmd][ny] * (PLT->weight_1000seed_array[mgmd][ny] * g_to_kg / 1000);	/* n/m2 * (g/1000n) -> kg seed/m2 */
		seed_Ccontent = PLT->seed_carbon_array[mgmd][ny]/100;														/* change unit: % to number */
		utiliz_coeff  = PLT->emerg_rate_array[mgmd][ny]/100;														/* change unit: % to number */

		

		/* 2.2 germination depth and layer */
		ctrl->germ_depth = PLT->germ_depth_array[0][ny];
		while (!flag_layerIMP)
		{
			if (ctrl->germ_depth >= sitec->soillayer_depth[layer-1] && ctrl->germ_depth < sitec->soillayer_depth[layer])
			{
				ctrl->germ_layer = layer;
				flag_layerIMP = 1;
			}
			else
				layer += 1;
		}
	
	
		/* 2.3 Allocation is calculated based on  EPC alloc.params */
		prop_leaf     = epc->alloc_leafc[epc->n_germ_phenophase-1]    / total_allocation;											
		prop_froot    = epc->alloc_frootc[epc->n_germ_phenophase-1]   / total_allocation;	
		prop_fruit    = epc->alloc_fruitc[epc->n_germ_phenophase-1]   / total_allocation;	
		prop_softstem = epc->alloc_softstemc[epc->n_germ_phenophase-1]/ total_allocation;	

		if (epc->leaf_cn)
		{
			cf->leafc_transfer_from_PLT  = (seed_quantity * utiliz_coeff * prop_leaf)  * seed_Ccontent;
			nf->leafn_transfer_from_PLT  =  cf->leafc_transfer_from_PLT  / epc->leaf_cn;
		}
		
		
		if (epc->froot_cn)
		{
			cf->frootc_transfer_from_PLT = (seed_quantity * utiliz_coeff * prop_froot) * seed_Ccontent;
			nf->frootn_transfer_from_PLT =  cf->frootc_transfer_from_PLT / epc->froot_cn;
		}
		
		
		if (epc->fruit_cn)
		{
			cf->fruitc_transfer_from_PLT = (seed_quantity * utiliz_coeff * prop_fruit) * seed_Ccontent;
			nf->fruitn_transfer_from_PLT =  cf->fruitc_transfer_from_PLT / epc->fruit_cn;
		}
		
		
		if (epc->softstem_cn)
		{
			cf->softstemc_transfer_from_PLT = (seed_quantity * utiliz_coeff * prop_softstem) * seed_Ccontent;
			nf->softstemn_transfer_from_PLT =  cf->softstemc_transfer_from_PLT / epc->softstem_cn;
		}
		

			
		/* 2.4 STATE UPDATE */ 
		/* carbon */
		cs->leafc_transfer += cf->leafc_transfer_from_PLT;
		cs->PLTsrc += cf->leafc_transfer_from_PLT;
		cs->frootc_transfer += cf->frootc_transfer_from_PLT;
		cs->PLTsrc += cf->frootc_transfer_from_PLT;
		cs->fruitc_transfer += cf->fruitc_transfer_from_PLT;
		cs->PLTsrc += cf->fruitc_transfer_from_PLT;
		cs->softstemc_transfer += cf->softstemc_transfer_from_PLT;
		cs->PLTsrc += cf->softstemc_transfer_from_PLT;

		/* nitrogen */
		ns->leafn_transfer += nf->leafn_transfer_from_PLT;
		ns->PLTsrc += nf->leafn_transfer_from_PLT;
		ns->frootn_transfer += nf->frootn_transfer_from_PLT;
		ns->PLTsrc += nf->frootn_transfer_from_PLT;
		ns->fruitn_transfer += nf->fruitn_transfer_from_PLT;
		ns->PLTsrc += nf->fruitn_transfer_from_PLT;
		ns->softstemn_transfer += nf->softstemn_transfer_from_PLT;
		ns->PLTsrc += nf->softstemn_transfer_from_PLT;


	}
	




   return (!ok);
}
	