/* 
phenology.c
daily phenology fluxes

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "bgc.h"

int phenology(const epconst_struct* epc, const phenology_struct* phen,
epvar_struct* epv, cstate_struct* cs, cflux_struct* cf, nstate_struct* ns,
nflux_struct* nf)
{
	int ok=1;
	double ndays;
	double leaflitfallc, frootlitfallc;
	double livestemtovrc, livestemtovrn;
	double livecroottovrc, livecroottovrn;
	double drate;
	
	/* phenological control for EVERGREENS */
	if (epc->evergreen)
	{
		/* transfer growth fluxes */
		/* check for days left in transfer growth period */
		/* AAN - yes, this is an assignment */
		if ((ndays = phen->remdays_transfer))
		{
			/* calculate rates required to empty each transfer
			compartment by the end of transfer period, at approximately a
			constant rate of transfer */
			cf->leafc_transfer_to_leafc = cs->leafc_transfer / ndays;
			nf->leafn_transfer_to_leafn = ns->leafn_transfer / ndays;
			cf->frootc_transfer_to_frootc = cs->frootc_transfer / ndays;
			nf->frootn_transfer_to_frootn = ns->frootn_transfer / ndays;
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
			bgc_printf(BV_ERROR, "Error in call to leaf_litfall() from phenology()\n");
			ok=0;
		}

		/* fine root litterfall */
		frootlitfallc = epv->day_frootc_litfall_increment;
		if (frootlitfallc > cs->frootc) frootlitfallc = cs->frootc;
		if (ok && froot_litfall(epc,frootlitfallc,cf,nf))
		{
			bgc_printf(BV_ERROR, "Error in call to froot_litfall() from phenology()\n");
			ok=0;
		}
		
		/* turnover of live wood to dead wood also happens every day, at a
		rate determined once each year, using the annual maximum livewoody
		compartment masses and the specified livewood turnover rate */
		if (epc->woody)
		{
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
		
	} /* end if evergreen */
	else
	{
		/* deciduous */
		/* transfer growth fluxes */
		/* check for days left in transfer growth period */
		/* AAN - yes, this is an assignment */
		if ((ndays = phen->remdays_transfer))
		{
			/* transfer rate is defined to be a linearly decreasing
			function that reaches zero on the last day of the transfer
			period */
			cf->leafc_transfer_to_leafc = 2.0*cs->leafc_transfer / ndays;
			nf->leafn_transfer_to_leafn = 2.0*ns->leafn_transfer / ndays;
			cf->frootc_transfer_to_frootc = 2.0*cs->frootc_transfer / ndays;
			nf->frootn_transfer_to_frootn = 2.0*ns->frootn_transfer / ndays;
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
		}
		
		/* litterfall */
		/* defined such that all live material is removed by the end of the
		litterfall period, with a linearly ramping removal rate. assumes that
		the initial rate on the first day of litterfall is 0.0. */
		/* AAN - yes, this is an assignment */
		if ((ndays = phen->remdays_litfall))
		{
			if (ndays == 1.0)
			{
				/* last day of litterfall, special case to gaurantee
				that pools go to 0.0 */
				leaflitfallc = cs->leafc;
				frootlitfallc = cs->frootc;
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
			}
			/* leaf litterfall */
			if (leaflitfallc > cs->leafc) leaflitfallc = cs->leafc;
			if (ok && leaflitfallc && leaf_litfall(epc,leaflitfallc,cf,nf))
			{
				bgc_printf(BV_ERROR, "Error in call to leaf_litfall() from phenology()\n");
				ok=0;
			}
			/* fine root litterfall */
			if (frootlitfallc > cs->frootc) frootlitfallc = cs->frootc;
			if (ok && frootlitfallc && froot_litfall(epc,frootlitfallc,cf,nf))
			{
				bgc_printf(BV_ERROR, "Error in call to froot_litfall() from phenology()\n");
				ok=0;
			}
		} /* end if deciduous litterfall day */
		
		/* turnover of livewood to deadwood happens each day, just as for
		evergreen types, at a rate determined from the annual maximum
		livewood mass and the specified turnover rate */
		if (epc->woody)
		{
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
	
	/* for woody types, find annual maximum value for live stemc and live crootc
	calculation of livewood turnover rates */
	if (epc->woody)
	{
		if (epv->annmax_livestemc < cs->livestemc) epv->annmax_livestemc = cs->livestemc;
		if (epv->annmax_livecrootc < cs->livecrootc) epv->annmax_livecrootc = cs->livecrootc;
	}	
	
	/* for all types, find annual maximum leafc */
	if (epv->annmax_leafc < cs->leafc) epv->annmax_leafc = cs->leafc;
	if (epv->annmax_frootc < cs->frootc) epv->annmax_frootc = cs->frootc;
	
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

