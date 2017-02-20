/* 
growth_resp.c
daily growth respiration rates

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.3
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2016, D. Hidy [dori.hidy@gmail.com]
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

int growth_resp(epconst_struct* epc, cflux_struct* cf)
{
	int ok=1;
	double g1;   /* RATIO   C respired for growth : C grown  */ 
	double g2;   /* proportion of growth resp to release at fixation */

	g1 = epc->GR_ratio;
	g2 = GRPNOW;
	
	/* leaf and fine root growth respiration for both trees and grass */	
	cf->cpool_leaf_gr     = cf->cpool_to_leafc * g1;
	cf->cpool_froot_gr    = cf->cpool_to_frootc * g1;
	cf->cpool_leaf_storage_gr  = cf->cpool_to_leafc_storage * g1 * g2;
	cf->cpool_froot_storage_gr = cf->cpool_to_frootc_storage * g1 * g2; 
	cf->transfer_leaf_gr  = cf->leafc_transfer_to_leafc * g1 * (1.0-g2);
	cf->transfer_froot_gr = cf->frootc_transfer_to_frootc * g1 * (1.0-g2);
	/* fruit simulation - Hidy 2013. */
	cf->cpool_fruit_gr          = cf->cpool_to_fruitc * g1;
	cf->cpool_fruit_storage_gr  = cf->cpool_to_fruitc_storage * g1 * g2;
	cf->transfer_fruit_gr       = cf->fruitc_transfer_to_fruitc * g1 * (1.0-g2);
	


	/* TREE-specific and NON-WOODY SPECIFIC fluxes */
	if (epc->woody)
	{
		/* woody tissue growth respiration only for trees */
		cf->cpool_livestem_gr     = cf->cpool_to_livestemc * g1;
		cf->cpool_deadstem_gr     = cf->cpool_to_deadstemc * g1;
		cf->cpool_livecroot_gr    = cf->cpool_to_livecrootc * g1;
		cf->cpool_deadcroot_gr    = cf->cpool_to_deadcrootc * g1;
		cf->cpool_livestem_storage_gr  = cf->cpool_to_livestemc_storage * g1 * g2;
		cf->cpool_deadstem_storage_gr  = cf->cpool_to_deadstemc_storage * g1 * g2;
		cf->cpool_livecroot_storage_gr = cf->cpool_to_livecrootc_storage * g1 * g2;
		cf->cpool_deadcroot_storage_gr = cf->cpool_to_deadcrootc_storage * g1 * g2;
		cf->transfer_livestem_gr  = cf->livestemc_transfer_to_livestemc * g1 * (1.0-g2);
		cf->transfer_deadstem_gr  = cf->deadstemc_transfer_to_deadstemc * g1 * (1.0-g2);
		cf->transfer_livecroot_gr = cf->livecrootc_transfer_to_livecrootc * g1 * (1.0-g2);
		cf->transfer_deadcroot_gr = cf->deadcrootc_transfer_to_deadcrootc * g1 * (1.0-g2);
	}
	else
	{
		/* SOFT STEM SIMULATION of non-woody biomes - Hidy 2015 */
		cf->cpool_softstem_gr     = cf->cpool_to_softstemc * g1;
		cf->cpool_softstem_storage_gr  = cf->cpool_to_softstemc_storage * g1 * g2;
		cf->transfer_softstem_gr = cf->softstemc_transfer_to_softstemc * g1 * (1.0-g2);
	}
	
	return (!ok);
}

