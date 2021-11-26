/* 
phenology.c
daily phenology fluxes

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2020, D. Hidy [dori.hidy@gmail.com]
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
#include "bgc_func.h"
#include "bgc_constants.h"

int phenology(const control_struct* ctrl, const epconst_struct* epc, const cstate_struct* cs, const nstate_struct* ns, 
	          phenology_struct* phen, metvar_struct* metv,epvar_struct* epv, cflux_struct* cf, nflux_struct* nf)

{
	int errorCode=0;
	double ndays;
	double leaflitfallc, frootlitfallc;
	double livestemtovrc, livestemtovrn;
	double livecroottovrc, livecroottovrn;
	double drate;

	double fruitlitfallc=0;
	double softstemlitfallc=0;
	

	

	/* phenological control for EVERGREENS */
	if (epc->evergreen)
	{
		/* transfer growth fluxes */
		/* check for days left in transfer growth period */
		ndays = phen->remdays_transfer;
		if (ndays > 0)
		{
			/* calculate rates required to empty each transfer compartment by the end of transfer period, at approximately a constant rate of transfer */
			if (epc->leaf_cn)
			{
				cf->leafc_transfer_to_leafc         = cs->leafc_transfer / ndays;
				nf->leafn_transfer_to_leafn         = cf->leafc_transfer_to_leafc / epc->leaf_cn;
			}

			if (epc->froot_cn)
			{
				cf->frootc_transfer_to_frootc       = cs->frootc_transfer / ndays;
				nf->frootn_transfer_to_frootn       = cf->frootc_transfer_to_frootc / epc->froot_cn;
			}
	        
			if (epc->fruit_cn)
			{
				cf->fruitc_transfer_to_fruitc       = cs->fruitc_transfer / ndays;
				nf->fruitn_transfer_to_fruitn       = cf->fruitc_transfer_to_fruitc       / epc->fruit_cn;
			}

			if (epc->softstem_cn)
			{
				cf->softstemc_transfer_to_softstemc = cs->softstemc_transfer / ndays;
				nf->softstemn_transfer_to_softstemn = cf->softstemc_transfer_to_softstemc / epc->softstem_cn;
			}
			
			if (epc->livewood_cn)
			{
				cf->livestemc_transfer_to_livestemc   = cs->livestemc_transfer / ndays;
				cf->livecrootc_transfer_to_livecrootc = cs->livecrootc_transfer / ndays;
				nf->livestemn_transfer_to_livestemn   = cf->livestemc_transfer_to_livestemc   / epc->livewood_cn;
				nf->livecrootn_transfer_to_livecrootn = cf->livecrootc_transfer_to_livecrootc / epc->livewood_cn;
			}

			if (epc->deadwood_cn)
			{
				cf->deadstemc_transfer_to_deadstemc   = cs->deadstemc_transfer / ndays;
				cf->deadcrootc_transfer_to_deadcrootc = cs->deadcrootc_transfer / ndays;
				nf->deadstemn_transfer_to_deadstemn   = cf->deadstemc_transfer_to_deadstemc   / epc->deadwood_cn;
				nf->deadcrootn_transfer_to_deadcrootn = cf->deadcrootc_transfer_to_deadcrootc / epc->deadwood_cn;
			}
		}
	

		/* litterfall happens everyday, at a rate determined each year on the annual allocation day.  To prevent litterfall from driving
		pools negative in the case of a very high mortality, fluxes are checked and set to zero when the pools get too small. */

		/* leaf litterfall */
		leaflitfallc = epv->day_leafc_litfall_increment;
		if (leaflitfallc > cs->leafc) leaflitfallc = cs->leafc;
		if (!errorCode && leaf_litfall(epc,leaflitfallc,cf,nf))
		{
			printf("\n");
			printf("ERROR in call to leaf_litfall() from phenology()\n");
			errorCode=1;
		}
		
	
		/* fine root litterfall */
		frootlitfallc = epv->day_frootc_litfall_increment;
		if (frootlitfallc > cs->frootc) frootlitfallc = cs->frootc;
		if (!errorCode && froot_litfall(epc,frootlitfallc,cf,nf))
		{
			printf("\n");
			printf("ERROR in call to froot_litfall() from phenology()\n");
			errorCode=1;
		}

		/* fruit litterfall */
		fruitlitfallc = epv->day_fruitc_litfall_increment;
		if (fruitlitfallc > cs->fruitc) fruitlitfallc = cs->fruitc;
		if (!errorCode && fruit_litfall(epc,fruitlitfallc,cf,nf))
		{
			printf("\n");
			printf("ERROR in call to fruit_litfall() from phenology()\n");
			errorCode=1;
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
				nf->livestemn_to_retransn  = livestemtovrn - nf->livestemn_to_deadstemn;
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
			if (!errorCode && softstem_litfall(epc,softstemlitfallc,cf,nf))
			{
				printf("\n");
				printf("ERROR in call to softstem_litfall() from phenology()\n");
				errorCode=1;
			}
		}
		
	} /* end if evergreen */
	else
	{
		/* deciduous */
		
		/* transfer growth fluxes based on GDD or EPC parameter */
		/* check for days left in transfer growth period */
		
		if (epc->transferGDD_flag)
		{
			if (!errorCode && transfer_fromGDD(epc, cs, ns, phen, metv, epv, cf, nf))
			{
				printf("ERROR: transfer_fromGDD() for sitec_init \n");
				errorCode=1;
			}	
		}
		else
		{
			ndays = phen->remdays_transfer;
			if (ndays > 0)
			{
				/* transfer rate is defined to be a linearly decreasing function that reaches zero on the last day of the transfer period */
				if (epc->leaf_cn)
				{
					cf->leafc_transfer_to_leafc         = 2*cs->leafc_transfer / ndays;
					nf->leafn_transfer_to_leafn         = cf->leafc_transfer_to_leafc / epc->leaf_cn;
				}

				if (epc->froot_cn)
				{
					cf->frootc_transfer_to_frootc       = 2*cs->frootc_transfer / ndays;
					nf->frootn_transfer_to_frootn       = cf->frootc_transfer_to_frootc / epc->froot_cn;
				}
	        
				if (epc->fruit_cn)
				{
					cf->fruitc_transfer_to_fruitc       = 2*cs->fruitc_transfer / ndays;
					nf->fruitn_transfer_to_fruitn       = cf->fruitc_transfer_to_fruitc       / epc->fruit_cn;
				}

				if (epc->softstem_cn)
				{
					cf->softstemc_transfer_to_softstemc = 2*cs->softstemc_transfer / ndays;
					nf->softstemn_transfer_to_softstemn = cf->softstemc_transfer_to_softstemc / epc->softstem_cn;
				}
			
				if (epc->livewood_cn)
				{
					cf->livestemc_transfer_to_livestemc   = 2*cs->livestemc_transfer / ndays;
					cf->livecrootc_transfer_to_livecrootc = 2*cs->livecrootc_transfer / ndays;
					nf->livestemn_transfer_to_livestemn   = cf->livestemc_transfer_to_livestemc   / epc->livewood_cn;
					nf->livecrootn_transfer_to_livecrootn = cf->livecrootc_transfer_to_livecrootc / epc->livewood_cn;
				}

				if (epc->deadwood_cn)
				{
					cf->deadstemc_transfer_to_deadstemc   = 2*cs->deadstemc_transfer / ndays;
					cf->deadcrootc_transfer_to_deadcrootc = 2*cs->deadcrootc_transfer / ndays;
					nf->deadstemn_transfer_to_deadstemn   = cf->deadstemc_transfer_to_deadstemc   / epc->deadwood_cn;
					nf->deadcrootn_transfer_to_deadcrootn = cf->deadcrootc_transfer_to_deadcrootc / epc->deadwood_cn;
				}
			
				if (cf->leafc_transfer_to_leafc > cs->leafc_transfer)               cf->leafc_transfer_to_leafc = cs->leafc_transfer;
				if (cf->frootc_transfer_to_frootc > cs->frootc_transfer)             cf->frootc_transfer_to_frootc = cs->frootc_transfer;
				if (cf->fruitc_transfer_to_fruitc > cs->fruitc_transfer)             cf->fruitc_transfer_to_fruitc = cs->fruitc_transfer;
				if (cf->softstemc_transfer_to_softstemc > cs->softstemc_transfer)    cf->softstemc_transfer_to_softstemc = cs->softstemc_transfer;
				if (cf->livestemc_transfer_to_livestemc > cs->livestemc_transfer)    cf->livestemc_transfer_to_livestemc = cs->livestemc_transfer;
				if (cf->deadstemc_transfer_to_deadstemc > cs->deadstemc_transfer)    cf->deadstemc_transfer_to_deadstemc = cs->deadstemc_transfer;
				if (cf->livecrootc_transfer_to_livecrootc > cs->livecrootc_transfer) cf->livecrootc_transfer_to_livecrootc = cs->livecrootc_transfer;
				if (cf->deadcrootc_transfer_to_deadcrootc > cs->deadcrootc_transfer) cf->deadcrootc_transfer_to_deadcrootc = cs->deadcrootc_transfer;

				if (nf->leafn_transfer_to_leafn > ns->leafn_transfer)                nf->leafn_transfer_to_leafn = ns->leafn_transfer;
				if (nf->frootn_transfer_to_frootn > ns->frootn_transfer)             nf->frootn_transfer_to_frootn = ns->frootn_transfer;
				if (nf->fruitn_transfer_to_fruitn > ns->fruitn_transfer)             nf->fruitn_transfer_to_fruitn = ns->fruitn_transfer;
				if (nf->softstemn_transfer_to_softstemn > ns->softstemn_transfer)    nf->softstemn_transfer_to_softstemn = ns->softstemn_transfer;
				if (nf->livestemn_transfer_to_livestemn > ns->livestemn_transfer)    nf->livestemn_transfer_to_livestemn = ns->livestemn_transfer;
				if (nf->deadstemn_transfer_to_deadstemn > ns->deadstemn_transfer)    nf->deadstemn_transfer_to_deadstemn = ns->deadstemn_transfer;
				if (nf->livecrootn_transfer_to_livecrootn > ns->livecrootn_transfer) nf->livecrootn_transfer_to_livecrootn = ns->livecrootn_transfer;
				if (nf->deadcrootn_transfer_to_deadcrootn > ns->deadcrootn_transfer) nf->deadcrootn_transfer_to_deadcrootn = ns->deadcrootn_transfer;
		
				/* calculating transfer ratio */
				if (cs->leafc_transfer) epv->transfer_ratio = cf->leafc_transfer_to_leafc / cs->leafc_transfer;
					
			}
		}


		
		/* litterfall */
		/* defined such that all live material is removed by the end of the litterfall period, with a linearly ramping removal rate. assumes that
		the initial rate on the first day of litterfall is 0.0. */
		ndays = phen->remdays_litfall;
		if (ndays != 0)
		{
			if (ndays == -1.0)
			{
				/* SPECIAL DAY AFTER litterfall, special case to gaurantee that pools go to 0.0 */
				leaflitfallc     = cs->leafc;
				frootlitfallc    = cs->frootc;
				fruitlitfallc    = cs->fruitc;
				softstemlitfallc = cs->softstemc;
			}
			else
			{
				/* otherwise, assess litterfall  rates as described above */
				leaflitfallc = epv->day_leafc_litfall_increment;
				drate = 2.0*(cs->leafc - leaflitfallc*ndays)/(ndays*ndays);
				epv->day_leafc_litfall_increment += drate;
				
				frootlitfallc = epv->day_frootc_litfall_increment;
				drate = 2.0*(cs->frootc - frootlitfallc*ndays)/(ndays*ndays);
				epv->day_frootc_litfall_increment += drate;

				fruitlitfallc = epv->day_fruitc_litfall_increment;
				drate = 2.0*(cs->fruitc - fruitlitfallc*ndays)/(ndays*ndays);
				epv->day_fruitc_litfall_increment += drate;

				softstemlitfallc = epv->day_softstemc_litfall_increment;
				drate = 2.0*(cs->softstemc - softstemlitfallc*ndays)/(ndays*ndays);
				epv->day_softstemc_litfall_increment += drate;
				
			}
			
			/* leaf litterfall */
			if (leaflitfallc > cs->leafc) leaflitfallc = cs->leafc;
			if (!errorCode && leaflitfallc && leaf_litfall(epc,leaflitfallc,cf,nf))
			{
				printf("\n");
				printf("ERROR in call to leaf_litfall() from phenology()\n");
				errorCode=1;
			}
	
			/* fine root litterfall */
			if (frootlitfallc > cs->frootc) frootlitfallc = cs->frootc;
			if (!errorCode && frootlitfallc && froot_litfall(epc,frootlitfallc,cf,nf))
			{
				printf("\n");
				printf("ERROR in call to froot_litfall() from phenology()\n");
				errorCode=1;
			}

			/* fruit litterfall */
			if (fruitlitfallc > cs->fruitc) fruitlitfallc = cs->fruitc;
			if (!errorCode && fruitlitfallc && fruit_litfall(epc,fruitlitfallc,cf,nf))
			{
				printf("\n");
				printf("ERROR in call to fruit_litfall() from phenology()\n");
				errorCode=1;
			}

			/* sofstem litterfall */
			if (softstemlitfallc > cs->softstemc) softstemlitfallc = cs->softstemc;
			if (!errorCode && softstemlitfallc && softstem_litfall(epc,softstemlitfallc,cf,nf))
			{
				printf("\n");
				printf("ERROR in call to softstem_litfall() from phenology()\n");
				errorCode=1;
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
		
	} /* end if deciduous */


	
	/* for all types, find annual maximum leafc */
	if (epv->annmax_leafc < cs->leafc)           epv->annmax_leafc = cs->leafc;
	if (epv->annmax_frootc < cs->frootc)         epv->annmax_frootc = cs->frootc;
	if (epv->annmax_fruitc < cs->fruitc)         epv->annmax_fruitc = cs->fruitc;
	if (epv->annmax_softstemc < cs->softstemc)   epv->annmax_softstemc = cs->softstemc;
	if (epv->annmax_livestemc < cs->livestemc)   epv->annmax_livestemc = cs->livestemc;
	if (epv->annmax_livecrootc < cs->livecrootc) epv->annmax_livecrootc = cs->livecrootc;
	
	return (errorCode);
}

int leaf_litfall(const epconst_struct* epc, double litfallc, cflux_struct* cf, nflux_struct* nf)
{
	int errorCode=0;
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
	
	if (!errorCode)
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
	
	return (errorCode);
}

int fruit_litfall(const epconst_struct* epc, double litfallc, cflux_struct* cf, nflux_struct* nf)
{
	int errorCode=0;
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
	
	if (!errorCode)
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
	
	return (errorCode);
}

int froot_litfall(const epconst_struct* epc, double litfallc, cflux_struct* cf, nflux_struct* nf)
{
	int errorCode=0;
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
	
	if (!errorCode)
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
	
	return (errorCode);
}

int softstem_litfall(const epconst_struct* epc, double litfallc, cflux_struct* cf, nflux_struct* nf)
{
	int errorCode=0;
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
	
	if (!errorCode)
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
	
	return (errorCode);
}


int transfer_fromGDD(const epconst_struct* epc, const cstate_struct* cs, const nstate_struct* ns, 
	                 phenology_struct *phen, metvar_struct *metv, epvar_struct* epv, cflux_struct* cf, nflux_struct* nf)

{
	int errorCode=0;
	
	/* phenological stages */
	int n_actphen          = (int) epv->n_actphen;

	/* **********************************************************************************/
	/* Growing degree day calculation for phenological calculation */

	/* transfer period */
	if (n_actphen == epc->n_emerg_phenophase)
	{
		/* determining the first and last day of transfer period */
		if (epc->n_emerg_phenophase > 1)
		{
			phen->GDD_emergSTART = phen->GDD_crit[epc->n_emerg_phenophase-2];
			phen->GDD_emergEND   = phen->GDD_crit[epc->n_emerg_phenophase-2] + epc->phenophase_length[n_actphen-1];
		}
		else
		{
			phen->GDD_emergSTART = 0;
			phen->GDD_emergEND   = epc->phenophase_length[0];
		}

		
		epv->transfer_ratio    = (metv->GDD_wMOD-phen->GDD_emergSTART)/(phen->GDD_emergEND-phen->GDD_emergSTART);
		
		if (epv->transfer_ratio < 0 || epv->transfer_ratio > 1)
		{
			printf("\n");
			printf("ERROR in transfer_ratio calculation() in phenology()\n");
			errorCode=1;
			
		}
		

		/* transfer rate is defined to be a linearly decreasing function that reaches zero on the last day of the transfer period */
		cf->leafc_transfer_to_leafc   = cs->leafc_transfer * epv->transfer_ratio;
		nf->leafn_transfer_to_leafn   = cf->leafc_transfer_to_leafc / epc->leaf_cn;
		
		cf->frootc_transfer_to_frootc = cs->frootc_transfer * epv->transfer_ratio;
		nf->frootn_transfer_to_frootn = cf->frootc_transfer_to_frootc / epc->froot_cn;
		
		cf->fruitc_transfer_to_fruitc = cs->fruitc_transfer * epv->transfer_ratio;
		nf->fruitn_transfer_to_fruitn = cf->fruitc_transfer_to_fruitc / epc->fruit_cn;

		cf->softstemc_transfer_to_softstemc = cs->softstemc_transfer * epv->transfer_ratio;
		nf->softstemn_transfer_to_softstemn = cf->softstemc_transfer_to_softstemc / epc->softstem_cn;

		if (cf->leafc_transfer_to_leafc > cs->leafc_transfer) cf->leafc_transfer_to_leafc = cs->leafc_transfer;
		if (nf->leafn_transfer_to_leafn > ns->leafn_transfer) nf->leafn_transfer_to_leafn = ns->leafn_transfer;
		if (cf->frootc_transfer_to_frootc > cs->frootc_transfer) cf->frootc_transfer_to_frootc = cs->frootc_transfer;
		if (nf->frootn_transfer_to_frootn > ns->frootn_transfer) nf->frootn_transfer_to_frootn = ns->frootn_transfer;
		if (cf->fruitc_transfer_to_fruitc > cs->fruitc_transfer) cf->fruitc_transfer_to_fruitc = cs->fruitc_transfer;
		if (nf->fruitn_transfer_to_fruitn > ns->fruitn_transfer) nf->fruitn_transfer_to_fruitn = ns->fruitn_transfer;
		if (cf->softstemc_transfer_to_softstemc > cs->softstemc_transfer) cf->softstemc_transfer_to_softstemc = cs->softstemc_transfer;
		if (nf->softstemn_transfer_to_softstemn > ns->softstemn_transfer) nf->softstemn_transfer_to_softstemn = ns->softstemn_transfer;
		
	}
	else
	{
		/* first day after EMERGNECE period */
		if (metv->GDD_wMOD == phen->GDD_crit[epc->n_emerg_phenophase-1])
		{
			epv->transfer_ratio = 1;
			cf->leafc_transfer_to_leafc = cs->leafc_transfer * epv->transfer_ratio;
			nf->leafn_transfer_to_leafn = ns->leafn_transfer * epv->transfer_ratio;
			cf->frootc_transfer_to_frootc = cs->frootc_transfer * epv->transfer_ratio;
			nf->frootn_transfer_to_frootn = ns->frootn_transfer * epv->transfer_ratio;

			if (cf->leafc_transfer_to_leafc > cs->leafc_transfer) cf->leafc_transfer_to_leafc = cs->leafc_transfer;
			if (nf->leafn_transfer_to_leafn > ns->leafn_transfer) nf->leafn_transfer_to_leafn = ns->leafn_transfer;
			if (cf->frootc_transfer_to_frootc > cs->frootc_transfer) cf->frootc_transfer_to_frootc = cs->frootc_transfer;
			if (nf->frootn_transfer_to_frootn > ns->frootn_transfer) nf->frootn_transfer_to_frootn = ns->frootn_transfer;

			cf->softstemc_transfer_to_softstemc = cs->softstemc_transfer * epv->transfer_ratio;
			nf->softstemn_transfer_to_softstemn = ns->softstemn_transfer * epv->transfer_ratio;
			if (cf->softstemc_transfer_to_softstemc > cs->softstemc_transfer) cf->softstemc_transfer_to_softstemc = cs->softstemc_transfer;
			if (nf->softstemn_transfer_to_softstemn > ns->softstemn_transfer) nf->softstemn_transfer_to_softstemn = ns->softstemn_transfer;
			
		}
		else
			epv->transfer_ratio = 0;
		
	}
	


	

	return (errorCode);
}
