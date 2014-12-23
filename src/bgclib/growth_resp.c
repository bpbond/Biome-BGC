/* 
growth_resp.c
daily growth respiration rates

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "bgc.h"

int growth_resp(epconst_struct* epc, cflux_struct* cf)
{
	int ok=1;
	double g1;   /* RATIO   C respired for growth : C grown  */ 
	double g2;   /* proportion of growth resp to release at fixation */

	g1 = GRPERC;
	g2 = GRPNOW;
	
	/* leaf and fine root growth respiration for both trees and grass */	
	cf->cpool_leaf_gr     = cf->cpool_to_leafc * g1;
	cf->cpool_froot_gr    = cf->cpool_to_frootc * g1;
	cf->cpool_leaf_storage_gr  = cf->cpool_to_leafc_storage * g1 * g2;
	cf->cpool_froot_storage_gr = cf->cpool_to_frootc_storage * g1 * g2; 
	cf->transfer_leaf_gr  = cf->leafc_transfer_to_leafc * g1 * (1.0-g2);
	cf->transfer_froot_gr = cf->frootc_transfer_to_frootc * g1 * (1.0-g2);
	
	/* woody tissue growth respiration only for trees */
	if (epc->woody)
	{
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
	
	return (!ok);
}

