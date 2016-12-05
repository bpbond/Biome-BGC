/* 
phenology.c
daily phenology fluxes

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.2
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

int phenology(const control_struct* ctrl, const epconst_struct* epc, const phenology_struct* phen, epvar_struct* epv, 
			  cstate_struct* cs, cflux_struct* cf, nstate_struct* ns, nflux_struct* nf)
{
	int ok=1;
	double ndays, coeff;
	double leaflitfallc, frootlitfallc;
	double livestemtovrc, livestemtovrn;
	double livecroottovrc, livecroottovrn;
	double drate;
	double fruitlitfallc = 0;	/* fruit simulation - Hidy 2013. */
	double softstemlitfallc = 0;	/* softstem simulation - Hidy 2013. */
	
	/* phenological control for EVERGREENS */
	if (epc->evergreen)
	{
		/* transfer growth fluxes */
		/* check for days left in transfer growth period */
		ndays = phen->remdays_transfer;
		if (ndays > 0)
		{
			/* calculate rates required to empty each transfer
			compartment by the end of transfer period, at approximately a
			constant rate of transfer */
			cf->leafc_transfer_to_leafc = cs->leafc_transfer / ndays;
			nf->leafn_transfer_to_leafn = ns->leafn_transfer / ndays;
			cf->frootc_transfer_to_frootc = cs->frootc_transfer / ndays;
			nf->frootn_transfer_to_frootn = ns->frootn_transfer / ndays;
		
			
			/* TREE-specific and NON-WOODY SPECIFIC fluxes */
			if (epc->woody)
			{
				cf->livestemc_transfer_to_livestemc = cs->livestemc_transfer / ndays;
				nf->livestemn_transfer_to_livestemn = ns->livestemn_transfer / ndays;
				cf->deadstemc_transfer_to_deadstemc = cs->deadstemc_transfer / ndays;
				nf->deadstemn_transfer_to_deadstemn = ns->deadstemn_transfer / ndays;
				cf->livecrootc_transfer_to_livecrootc = cs->livecrootc_transfer / ndays;
				nf->livecrootn_transfer_to_livecrootn = ns->livecrootn_transfer / ndays;
				cf->deadcrootc_transfer_to_deadcrootc = cs->deadcrootc_transfer / ndays;
				nf->deadcrootn_transfer_to_deadcrootn = ns->deadcrootn_transfer / ndays;
			}
			else
			{
				cf->softstemc_transfer_to_softstemc = cs->softstemc_transfer / ndays;
				nf->softstemn_transfer_to_softstemn = ns->softstemn_transfer / ndays;
			}
		}
	

		/* litterfall happens everyday, at a rate determined each year
		on the annual allocation day.  To prevent litterfall from driving
		pools negative in the case of a very high mortality, fluxes are
		checked and set to zero when the pools get too small. */

		/* leaf litterfall */
		leaflitfallc = epv->day_leafc_litfall_increment;
		if (leaflitfallc > cs->leafc) leaflitfallc = cs->leafc;
		if (ok && leaf_litfall(epc,leaflitfallc,cf,nf))
		{
			printf("Error in call to leaf_litfall() from phenology()\n");
			ok=0;
		}
		
	
		/* fine root litterfall */
		frootlitfallc = epv->day_frootc_litfall_increment;
		if (frootlitfallc > cs->frootc) frootlitfallc = cs->frootc;
		if (ok && froot_litfall(epc,frootlitfallc,cf,nf))
		{
			printf("Error in call to froot_litfall() from phenology()\n");
			ok=0;
		}
		

	
		/* TREE-specific and NON-WOODY SPECIFIC fluxes */
		if (epc->woody)
		{ 	/* turnover of live wood to dead wood also happens every day, at a
			rate determined once each year, using the annual maximum livewoody
			compartment masses and the specified livewood turnover rate */
			
			/* turnover from live stem wood to dead stem wood */
			livestemtovrc = epv->day_livestemc_turnover_increment;
			livestemtovrn = livestemtovrc / epc->livewood_cn;
			if (livestemtovrc > cs->livestemc) livestemtovrc = cs->livestemc;
			if (livestemtovrn > ns->livestemn) livestemtovrn = ns->livestemn;
			if (livestemtovrc && livestemtovrn)
			{
				cf->livestemc_to_deadstemc = livestemtovrc;
				nf->livestemn_to_deadstemn = livestemtovrc / epc->deadwood_cn;
				nf->livestemn_to_retransn = livestemtovrn - nf->livestemn_to_deadstemn;
			}

			/* turnover from live coarse root wood to dead coarse root wood */
			livecroottovrc = epv->day_livecrootc_turnover_increment;
			livecroottovrn = livecroottovrc / epc->livewood_cn;
			if (livecroottovrc > cs->livecrootc) livecroottovrc = cs->livecrootc;
			if (livecroottovrn > ns->livecrootn) livecroottovrn = ns->livecrootn;
			if (livecroottovrc && livecroottovrn)
			{
				cf->livecrootc_to_deadcrootc = livecroottovrc;
				nf->livecrootn_to_deadcrootn = livecroottovrc / epc->deadwood_cn;
				nf->livecrootn_to_retransn = livecroottovrn - nf->livecrootn_to_deadcrootn;
			}
		}
		else
		{
			/* softstem litterfall */
			softstemlitfallc = epv->day_softstemc_litfall_increment;
			if (softstemlitfallc > cs->softstemc) softstemlitfallc = cs->softstemc;
			if (ok && softstem_litfall(epc,softstemlitfallc,cf,nf))
			{
				printf("Error in call to softstem_litfall() from phenology()\n");
				ok=0;
			}
		}
		
	} /* end if evergreen */
	else
	{
		/* deciduous */
		/* transfer growth fluxes */
		/* check for days left in transfer growth period */



		ndays = phen->remdays_transfer;
		if (ndays > 0)
		{
			/* transfer rate is defined to be a linearly decreasing
			function that reaches zero on the last day of the transfer
			period */
			cf->leafc_transfer_to_leafc = 2.0*cs->leafc_transfer / ndays;
			nf->leafn_transfer_to_leafn = 2.0*ns->leafn_transfer / ndays;
			cf->frootc_transfer_to_frootc = 2.0*cs->frootc_transfer / ndays;
			nf->frootn_transfer_to_frootn = 2.0*ns->frootn_transfer / ndays;
		
			
			/* TREE-specific and NON-WOODY SPECIFIC fluxes */
			if (epc->woody)
			{
				cf->livestemc_transfer_to_livestemc = 2.0*cs->livestemc_transfer / ndays;
				nf->livestemn_transfer_to_livestemn = 2.0*ns->livestemn_transfer / ndays;
				cf->deadstemc_transfer_to_deadstemc = 2.0*cs->deadstemc_transfer / ndays;
				nf->deadstemn_transfer_to_deadstemn = 2.0*ns->deadstemn_transfer / ndays;
				cf->livecrootc_transfer_to_livecrootc = 2.0*cs->livecrootc_transfer / ndays;
				nf->livecrootn_transfer_to_livecrootn = 2.0*ns->livecrootn_transfer / ndays;
				cf->deadcrootc_transfer_to_deadcrootc = 2.0*cs->deadcrootc_transfer / ndays;
				nf->deadcrootn_transfer_to_deadcrootn = 2.0*ns->deadcrootn_transfer / ndays;
			}
			else
			{ 	/* SOFT STEM SIMULATION of non-woody biomes - Hidy 2015 */
				cf->softstemc_transfer_to_softstemc = 2.0*cs->softstemc_transfer / ndays;
				nf->softstemn_transfer_to_softstemn = 2.0*ns->softstemn_transfer / ndays;
				if (cf->softstemc_transfer_to_softstemc > cs->softstemc_transfer) cf->softstemc_transfer_to_softstemc = cs->softstemc_transfer;
				if (nf->softstemn_transfer_to_softstemn > ns->softstemn_transfer) nf->softstemn_transfer_to_softstemn = ns->softstemn_transfer;
			
			}
		}
	
		
		/* litterfall */
		/* defined such that all live material is removed by the end of the
		litterfall period, with a linearly ramping removal rate. assumes that
		the initial rate on the first day of litterfall is 0.0. */
		ndays = phen->remdays_litfall;
		if (ndays > 0)
		{
			if (ndays == 1.0)
			{
				/* last day of litterfall, special case to gaurantee
				that pools go to 0.0 */
				leaflitfallc = cs->leafc;
				frootlitfallc = cs->frootc;
				/* softstem simulation - Hidy 2015. */
				softstemlitfallc = cs->softstemc;
			}
			else
			{
				/* otherwise, assess litterfall 
				rates as described above */
				leaflitfallc = epv->day_leafc_litfall_increment;
				drate = 2.0*(cs->leafc - leaflitfallc*ndays)/(ndays*ndays);
				epv->day_leafc_litfall_increment += drate;
				
				frootlitfallc = epv->day_frootc_litfall_increment;
				drate = 2.0*(cs->frootc - frootlitfallc*ndays)/(ndays*ndays);
				epv->day_frootc_litfall_increment += drate;

				softstemlitfallc = epv->day_softstemc_litfall_increment;
				drate = 2.0*(cs->softstemc - softstemlitfallc*ndays)/(ndays*ndays);
				epv->day_softstemc_litfall_increment += drate;
				
		
			}
			/* leaf litterfall */
			if (leaflitfallc > cs->leafc) leaflitfallc = cs->leafc;
			if (ok && leaflitfallc && leaf_litfall(epc,leaflitfallc,cf,nf))
			{
				printf("Error in call to leaf_litfall() from phenology()\n");
				ok=0;
			}
	
			/* fine root litterfall */
			if (frootlitfallc > cs->frootc) frootlitfallc = cs->frootc;
			if (ok && frootlitfallc && froot_litfall(epc,frootlitfallc,cf,nf))
			{
				printf("Error in call to froot_litfall() from phenology()\n");
				ok=0;
			}
		} /* end if deciduous litterfall day */
		
	
		/* TREE-specific and NON-WOODY SPECIFIC fluxes */
		if (epc->woody)
		{	/* turnover of livewood to deadwood happens each day, just as for
			evergreen types, at a rate determined from the annual maximum
			livewood mass and the specified turnover rate */
			/* turnover from live stem wood to dead stem wood */
			livestemtovrc = epv->day_livestemc_turnover_increment;
			livestemtovrn = livestemtovrc / epc->livewood_cn;
			if (livestemtovrc > cs->livestemc) livestemtovrc = cs->livestemc;
			if (livestemtovrn > ns->livestemn) livestemtovrn = ns->livestemn;
			if (livestemtovrc && livestemtovrn)
			{
				cf->livestemc_to_deadstemc = livestemtovrc;
				nf->livestemn_to_deadstemn = livestemtovrc / epc->deadwood_cn;
				nf->livestemn_to_retransn = livestemtovrn - nf->livestemn_to_deadstemn;
			}

			/* turnover from live coarse root wood to dead coarse root wood */
			livecroottovrc = epv->day_livecrootc_turnover_increment;
			livecroottovrn = livecroottovrc / epc->livewood_cn;
			if (livecroottovrc > cs->livecrootc) livecroottovrc = cs->livecrootc;
			if (livecroottovrn > ns->livecrootn) livecroottovrn = ns->livecrootn;
			if (livecroottovrc && livecroottovrn)
			{
				cf->livecrootc_to_deadcrootc = livecroottovrc;
				nf->livecrootn_to_deadcrootn = livecroottovrc / epc->deadwood_cn;
				nf->livecrootn_to_retransn = livecroottovrn - nf->livecrootn_to_deadcrootn;
			}
		}
		else
		{ /* soft stem simulation Hidy 2015 */
			/* softstem litterfall */
			if (softstemlitfallc > cs->softstemc) softstemlitfallc = cs->softstemc;
			if (ok && softstemlitfallc && softstem_litfall(epc,softstemlitfallc,cf,nf))
			{
				printf("Error in call to softstem_litfall() from phenology()\n");
				ok=0;
			}
		} 
		
	} /* end if deciduous */

		
	/* fruit simulation - Hidy 2013. */
	if (epc->alloc_fruitc_leafc)
	{
		ndays = phen->offday - ctrl->yday;
		if (epv->flowering && ndays > 0)
		{
			
			if (epc->evergreen)
				coeff = 1;
			else
				coeff = 2;
			cf->fruitc_transfer_to_fruitc = coeff*cs->fruitc_transfer / ndays;
			nf->fruitn_transfer_to_fruitn = coeff*ns->fruitn_transfer / ndays;
			if (cf->fruitc_transfer_to_fruitc > cs->fruitc_transfer) cf->fruitc_transfer_to_fruitc = cs->fruitc_transfer;
			if (nf->fruitn_transfer_to_fruitn > ns->fruitn_transfer) nf->fruitn_transfer_to_fruitn = ns->fruitn_transfer;

			fruitlitfallc = epv->day_fruitc_litfall_increment;
            
			if (!epc->evergreen)
			{
				drate = 2.0*(cs->fruitc - fruitlitfallc*ndays)/(ndays*ndays);
				epv->day_fruitc_litfall_increment += drate;
			}

            if (fruitlitfallc > cs->fruitc || ndays == 1) fruitlitfallc = cs->fruitc;
		}
		else
		{
			cf->fruitc_transfer_to_fruitc = 0;
			nf->fruitn_transfer_to_fruitn = 0;
			fruitlitfallc = cs->fruitc;
		
		}
		if (fruitlitfallc > cs->fruitc) fruitlitfallc = cs->fruitc;
		if (ok && fruitlitfallc && fruit_litfall(epc,fruitlitfallc,cf,nf))
		{
			printf("Error in call to fruit_litfall() from phenology()\n");
			ok=0;
		}
	
	}
	else
	{
		cf->fruitc_transfer_to_fruitc = 0;
		nf->fruitn_transfer_to_fruitn = 0;
	}

	
	/* TREE-specific and NON-WOODY SPECIFIC fluxes */
	if (epc->woody)
	{	/* for woody types, find annual maximum value for live stemc and live crootc
		calculation of livewood turnover rates */
		if (epv->annmax_livestemc < cs->livestemc) epv->annmax_livestemc = cs->livestemc;
		if (epv->annmax_livecrootc < cs->livecrootc) epv->annmax_livecrootc = cs->livecrootc;
	}
	else
	{
		/* softstem simulation - Hidy 2013. */
		if (epv->annmax_softstemc < cs->softstemc) epv->annmax_softstemc = cs->softstemc;
	
	}
	
	/* for all types, find annual maximum leafc */
	if (epv->annmax_leafc < cs->leafc) epv->annmax_leafc = cs->leafc;
	if (epv->annmax_frootc < cs->frootc) epv->annmax_frootc = cs->frootc;
	/* fruit simulation - Hidy 2013. */
	if (epv->annmax_fruitc < cs->fruitc) epv->annmax_fruitc = cs->fruitc;
	/* softstem simulation - Hidy 2013. */
	if (epv->annmax_softstemc < cs->softstemc) epv->annmax_softstemc = cs->softstemc;
	
	return (!ok);
}

int leaf_litfall(const epconst_struct* epc, double litfallc,
cflux_struct* cf, nflux_struct* nf)
{
	int ok=1;
	double c1,c2,c3,c4;
	double n1,n2,n3,n4;
	double nretrans;
	double avg_cn;
	double litfalln;
	
	avg_cn = epc->leaf_cn;
	litfalln = litfallc / epc->leaflitr_cn;
	
	c1 = litfallc * epc->leaflitr_flab;
	n1 = litfalln * epc->leaflitr_flab;
	c2 = litfallc * epc->leaflitr_fucel;
	n2 = litfalln * epc->leaflitr_fucel; 
	c3 = litfallc * epc->leaflitr_fscel;
	n3 = litfalln * epc->leaflitr_fscel; 
	c4 = litfallc * epc->leaflitr_flig;
	n4 = litfalln * epc->leaflitr_flig; 
	nretrans = (litfallc/avg_cn) - (litfalln);
	
	if (ok)
	{
		/* set fluxes in daily flux structure */
		cf->leafc_to_litr1c = c1;
		cf->leafc_to_litr2c = c2;
		cf->leafc_to_litr3c = c3;
		cf->leafc_to_litr4c = c4;
		nf->leafn_to_litr1n = n1;
		nf->leafn_to_litr2n = n2;
		nf->leafn_to_litr3n = n3;
		nf->leafn_to_litr4n = n4;
		nf->leafn_to_retransn = nretrans;
	}
	
	return (!ok);
}

int fruit_litfall(const epconst_struct* epc, double litfallc, 
cflux_struct* cf, nflux_struct* nf)
{
	int ok=1;
	double c1,c2,c3,c4;
	double n1,n2,n3,n4;
	double avg_cn;
	
	avg_cn = epc->fruit_cn;
	
	c1 = litfallc * epc->fruitlitr_flab;
	n1 = c1 / avg_cn;
	c2 = litfallc * epc->fruitlitr_fucel;
	n2 = c2 / avg_cn;
	c3 = litfallc * epc->fruitlitr_fscel;
	n3 = c3 / avg_cn;
	c4 = litfallc * epc->fruitlitr_flig;
	n4 = c4 / avg_cn;
	
	if (ok)
	{
		/* set fluxes in daily flux structure */
		cf->fruitc_to_litr1c = c1;
		cf->fruitc_to_litr2c = c2;
		cf->fruitc_to_litr3c = c3;
		cf->fruitc_to_litr4c = c4;
		nf->fruitn_to_litr1n = n1;
		nf->fruitn_to_litr2n = n2;
		nf->fruitn_to_litr3n = n3;
		nf->fruitn_to_litr4n = n4;
	}
	
	return (!ok);
}

int froot_litfall(const epconst_struct* epc, double litfallc, 
cflux_struct* cf, nflux_struct* nf)
{
	int ok=1;
	double c1,c2,c3,c4;
	double n1,n2,n3,n4;
	double avg_cn;
	
	avg_cn = epc->froot_cn;
	
	c1 = litfallc * epc->frootlitr_flab;
	n1 = c1 / avg_cn;
	c2 = litfallc * epc->frootlitr_fucel;
	n2 = c2 / avg_cn;
	c3 = litfallc * epc->frootlitr_fscel;
	n3 = c3 / avg_cn;
	c4 = litfallc * epc->frootlitr_flig;
	n4 = c4 / avg_cn;
	
	if (ok)
	{
		/* set fluxes in daily flux structure */
		cf->frootc_to_litr1c = c1;
		cf->frootc_to_litr2c = c2;
		cf->frootc_to_litr3c = c3;
		cf->frootc_to_litr4c = c4;
		nf->frootn_to_litr1n = n1;
		nf->frootn_to_litr2n = n2;
		nf->frootn_to_litr3n = n3;
		nf->frootn_to_litr4n = n4;
	}
	
	return (!ok);
}

int softstem_litfall(const epconst_struct* epc, double litfallc, 
cflux_struct* cf, nflux_struct* nf)
{
	int ok=1;
	double c1,c2,c3,c4;
	double n1,n2,n3,n4;
	double avg_cn;
	
	avg_cn = epc->softstem_cn;
	
	c1 = litfallc * epc->softstemlitr_flab;
	n1 = c1 / avg_cn;
	c2 = litfallc * epc->softstemlitr_fucel;
	n2 = c2 / avg_cn;
	c3 = litfallc * epc->softstemlitr_fscel;
	n3 = c3 / avg_cn;
	c4 = litfallc * epc->softstemlitr_flig;
	n4 = c4 / avg_cn;
	
	if (ok)
	{
		/* set fluxes in daily flux structure */
		cf->softstemc_to_litr1c = c1;
		cf->softstemc_to_litr2c = c2;
		cf->softstemc_to_litr3c = c3;
		cf->softstemc_to_litr4c = c4;
 		nf->softstemn_to_litr1n = n1;
		nf->softstemn_to_litr2n = n2;
		nf->softstemn_to_litr3n = n3;
		nf->softstemn_to_litr4n = n4;
	}
	
	return (!ok);
}


