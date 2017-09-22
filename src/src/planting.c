/* 
planting.c
planting  - planting seeds in soil - increase transfer pools

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.1
Copyright 2017, D. Hidy [dori.hidy@gmail.com]
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

int planting(const control_struct* ctrl,const epconst_struct* epc, planting_struct* PLT, cflux_struct* cf, nflux_struct* nf, 
			 cstate_struct* cs, nstate_struct* ns)
{
	/* planting parameters Hidy 2012.*/	   

	double seed_quantity,seed_Ccontent;					
	double utiliz_coeff;
	double prop_leaf, prop_froot, prop_fruit, prop_softstem;	
	
	int ok=1;
	int ny;
	int mgmd = PLT->mgmd;

	/* yearly varied or constant management parameters */
	if(PLT->PLT_flag == 2)
	{
		ny = ctrl->simyr;
	}
	else ny=0;

	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                                    CALCULATING FLUXES 
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

		
	/* we assume that the transfer pools contain the palnt material of seeds. Therefore planting increase the transfer pools */ 
	if (mgmd >= 0) 
	{	
		seed_quantity = PLT->seed_quantity_array[mgmd][ny]/1000;				 /* change unit: kg seed/ha -> kg seed/m2 */
		seed_Ccontent = PLT->seed_carbon_array[mgmd][ny]/100;			     /* change unit: % to number */
		utiliz_coeff  = PLT->utiliz_coeff_array[mgmd][ny]/100;	         /* change unit: % to number */
	
		/* allocation is calculated based on leafC - EPC alloc.params: unit is leafC content */
		prop_leaf     = 1.0                       /(epc->alloc_frootc_leafc + epc->alloc_fruitc_leafc + epc->alloc_softstemc_leafc + 1.);											
		prop_froot    = epc->alloc_frootc_leafc   /(epc->alloc_frootc_leafc + epc->alloc_fruitc_leafc + epc->alloc_softstemc_leafc + 1.);
		prop_fruit    = epc->alloc_fruitc_leafc   /(epc->alloc_frootc_leafc + epc->alloc_fruitc_leafc + epc->alloc_softstemc_leafc + 1.);
		prop_softstem = epc->alloc_softstemc_leafc/(epc->alloc_frootc_leafc + epc->alloc_fruitc_leafc + epc->alloc_softstemc_leafc + 1.);

		cf->leafc_transfer_from_PLT  = (seed_quantity * utiliz_coeff * prop_leaf)  * seed_Ccontent;
		nf->leafn_transfer_from_PLT  =  cf->leafc_transfer_from_PLT  / epc->leaf_cn;
		cf->frootc_transfer_from_PLT = (seed_quantity * utiliz_coeff * prop_froot) * seed_Ccontent;
		nf->frootn_transfer_from_PLT =  cf->frootc_transfer_from_PLT / epc->froot_cn;
		/* fruit simulation - Hidy 2013 */
		cf->fruitc_transfer_from_PLT = (seed_quantity * utiliz_coeff * prop_fruit) * seed_Ccontent;
		nf->fruitn_transfer_from_PLT =  cf->fruitc_transfer_from_PLT / epc->fruit_cn;
		/* softstem simulation - Hidy 2013 */
		cf->softstemc_transfer_from_PLT = (seed_quantity * utiliz_coeff * prop_softstem) * seed_Ccontent;
		nf->softstemn_transfer_from_PLT =  cf->softstemc_transfer_from_PLT / epc->softstem_cn;

	}
	else
	{
		cf->leafc_transfer_from_PLT  = 0.;
		cf->frootc_transfer_from_PLT = 0.;
		nf->leafn_transfer_from_PLT  = 0.;
		nf->frootn_transfer_from_PLT = 0.;
		cf->fruitc_transfer_from_PLT = 0.;
		nf->fruitn_transfer_from_PLT = 0.;
		cf->softstemc_transfer_from_PLT = 0.;
		nf->softstemn_transfer_from_PLT = 0.;
	}

	
	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                                    STATE UPDATE 
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/ 

	/* 1. carbon */
	cs->leafc_transfer += cf->leafc_transfer_from_PLT;
	cs->PLTsrc += cf->leafc_transfer_from_PLT;
	cs->frootc_transfer += cf->frootc_transfer_from_PLT;
	cs->PLTsrc += cf->frootc_transfer_from_PLT;
	/* fruit simulation - Hidy 2013 */
	cs->fruitc_transfer += cf->fruitc_transfer_from_PLT;
	cs->PLTsrc += cf->fruitc_transfer_from_PLT;
	/* softstem simulation - Hidy 2013 */
	cs->softstemc_transfer += cf->softstemc_transfer_from_PLT;
	cs->PLTsrc += cf->softstemc_transfer_from_PLT;

	/* 2. nitrogen */
	ns->leafn_transfer += nf->leafn_transfer_from_PLT;
	ns->PLTsrc += nf->leafn_transfer_from_PLT;
	ns->frootn_transfer += nf->frootn_transfer_from_PLT;
	ns->PLTsrc += nf->frootn_transfer_from_PLT;
	/* fruit simulation - Hidy 2013 */
	ns->fruitn_transfer += nf->fruitn_transfer_from_PLT;
	ns->PLTsrc += nf->fruitn_transfer_from_PLT;
	/* softstem simulation - Hidy 2013 */
	ns->softstemn_transfer += nf->softstemn_transfer_from_PLT;
	ns->PLTsrc += nf->softstemn_transfer_from_PLT;


   return (!ok);
}
	