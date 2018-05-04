/*
daily_allocation.c  
daily allocation of carbon and nitrogen, as well as the final reconciliation
of N immobilization by microbes (see decomp.c)

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2018, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

Modified:
by Hidy 2018
instead of separete spinup_daily_allocation, common daily_allocation rutin is for both normal and spinup phase
In the spinup phase exactly enough mineral N is added at each time step to satisfy the total demand of
plant and microbe.

Modified:
by James Trembath and Galina Churkina
20/07/2001:  Changes were made in the logic of the retranslocation of nitrogen, from 
V4.1.1 back to those of V4.1. The underlying reasons for this change are 
the polarised logics between the two versions. V4.1 considered a recipient 
based flow, the deployment of retranslocated leaf nitrogen in the spring is
based on the requirement of the plant, access by the plant to this pool was
ungoverned. That is to say that the plant could access all the nitrogen in
one day if it so required. This often resulted in an initial modelled NEE
spike in the early growing season, something that is seen in the EUROFLUX
data. V4.1.1 can be considered as a donor based flow, in that the deployment
of the retranslocated leaf nitrogen in the spring is a fractionation of the
initial size of the translocated pool at the end of the previous growing
season, not a translation in any way of plant demand. In test simulations,
this induced the long-term build up of nitrogen, inaccessible to the plant,
conceptually turning the retranslocated pool into a permanent nitrogen sink.
The translation of this logic to the overall modelled NEE flux was to induce
a mid-season crash in productivity as a result of nitrogen deficiency, something
unseen in the EUROFLUX data.


by Peter Thornton
4/17/2000 (PET): Comparisons with eddy flux data showed that the 
deployment of retranslocated leaf N for new growth was too rapid early in
the growing season under N-limited conditions, causing an early spike in
growth rate which was never observed in the eddy flux data.  The original
mechanism for deployment of retranslocated N allowed access to the entire
retranslocated N pool on each day. This has now been changed so that only
a fraction of the total retranslocated N pool is available for deployment
as new growth on each day. The macro DAYSNDEPLOY defines the
number of days over which the existing retrans N pool can be used up, and
on each day ns.retransn/DAYSNDEPLOY defines the maximum amount of N available
from the retrans N pool for new growth. 

4/17/2000 (PET): Part of the problem identified through comparisons with eddy 
flux data was related to the treatment of daily allocation in the face of
a cpool deficit. The original logic was to apply all available C each day
to the cpool until the deficit was eliminated, resulting in an abrupt shift
mid-spring when shifting over to new growth (no growth respiration attached to
C allocated to reduce cpool deficit).  The new approach that avoids this
abrupt shift is to apply C from the daily available pool at a rate that 
would alleviate the existing deficit if continued at a constant rate over
N days. As the deficit grows, this rate also increases, since N is a constant.
The result is that a dynamic equilibrium is established between withdrawals
and additions to the cpool, with small daily allocations throughout the 
growing season to make up for the small daily expenditures for maintenance
respiration through the winter. The new parameter (N) is called 
DAYSCRECOVER, and for now it is included as a macro definition in this file.

4/17/2000 (PET): Another part of the early-season spike observed under N
limitation is due to the accumulation of excess soil mineral N. The original
algorithm didn't have any explicit controls on the accumulation of soil
mineral N except leaching, fire, and denitrification tied to the rate of
decomposition processes. However, excess mineral N in real soils is quickly
lost to the atmosphere, and the new logic reflects this fact. On each day,
a specified proportion of the excess mineral N (not required for plant growth
or microbial growth) is lost to an atmospheric sink. This proportion is
defined below as BULK_DENITRIF_PROPORTION. This is a gross approximation
of the real processes, but it has the desired effect of eliminating
the accumulation of soil mineral N.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "bgc_struct.h"
#include "bgc_func.h"
#include "bgc_constants.h"

int daily_allocation(const epconst_struct* epc, const metvar_struct* metv, cstate_struct*cs,  nstate_struct* ns, 
	                 cflux_struct* cf, nflux_struct* nf, epvar_struct* epv, ntemp_struct* nt, double naddfrac)
{
	int ok=1;
    double day_gpp;     /* daily gross production */
	double day_mresp;   /* daily total maintenance respiration */
	double avail_c;     /* total C available for new production */
	double f1;          /* RATIO   new leaf C      : new total C   */
	double f2;          /* RATIO   new fine root C : new total C   */
	double f3;          /* RATIO   new fruit C     : new total C  */
	double f4;          /* RATIO   new softstem C  : new total C */
	double f5;          /* RATIO   new live woody stem C: new total C */
	double f6;          /* RATIO   new dead woody stem C: new total C */
	double f7;          /* RATIO   new live coarse root C: new total C */
	double f8;          /* RATIO   new dead coarse root C: new total C */
	double g1;          /* RATIO   C respired for growth : C grown  */ 

	int woody;
	double c_allometry, n_allometry;
	double plant_ndemand, sum_plant_nsupply, plant_remaining_ndemand,potential_immob_rootzone,sminn_rootzone,layer_prop;
	double plant_nalloc, plant_calloc;
    double excess_c, pnow_Tcoeff;
	int n_limitation, layer;
	double cn_l1,cn_l2,cn_l4,cn_s1,cn_s2,cn_s3,cn_s4;
	double rfl1s1, rfl2s2, rfl4s3, rfs1s2, rfs2s3, rfs3s4;
	double daily_net_nmin, actual_immob;
	double fpi, dif,sumdif;

	double pnow = 0;			/* proportion of growth displayed on current day */ 

	/* actual phenological phase */
	int ap = (int) epv->n_actphen-1; 

	excess_c=potential_immob_rootzone=sminn_rootzone=daily_net_nmin=sumdif=f1=f2=f3=f4=f5=f6=f7=f8=0;
	woody = epc->woody;

	cn_l1=cn_l2=cn_l4=cn_s1=cn_s2=cn_s3=cn_s4=0;

	/* respiration fractions for fluxes between compartments */
	rfl1s1 = epc->rfl1s1; //0.39;
	rfl2s2 = epc->rfl2s2; //0.55;
	rfl4s3 = epc->rfl4s3; //0.29;
	rfs1s2 = epc->rfs1s2; //0.28;
	rfs2s3 = epc->rfs2s3; //0.46;
	rfs3s4 = epc->rfs3s4; //0.55;

	/* Assess the carbon availability on the basis of this day's
	gross production and maintenance respiration costs */
	day_gpp = cf->psnsun_to_cpool + cf->psnshade_to_cpool;
	
	day_mresp = cf->leaf_day_mr + cf->leaf_night_mr + cf->froot_mr + cf->fruit_mr + cf->softstem_mr +
			         cf->livestem_mr + cf->livecroot_mr;
	avail_c = day_gpp - day_mresp;

	
	/* no allocation when the daily C balance is negative */
	if (avail_c < 0.0) avail_c = 0.0;

	/* test for cpool deficit */
	if (cs->cpool < 0.0 && fabs(cs->cpool) > CRIT_PREC)
	{

		printf("\n");
		printf("ERROR: negative cpool in daily_allocation\n");
		ok=0;
	} /* end if negative cpool */
	
	/* assign local values for the allocation control parameters */
	if (ap >= 0)
	{
		f1 = epc->alloc_leafc[ap];
		f2 = epc->alloc_frootc[ap];
		f3 = epc->alloc_fruitc[ap];
		f4 = epc->alloc_softstemc[ap];
		f5 = epc->alloc_livestemc[ap];
		f6 = epc->alloc_deadstemc[ap];
		f7 = epc->alloc_livecrootc[ap];
		f8 = epc->alloc_deadcrootc[ap];

		/* proportion of growth displayed on current day is the function of air temperature */ 
		pnow_Tcoeff = 1;
		if (epc->pnow_minT != DATA_GAP)
		{
			/* if less than min or greater than max -> 0 */
			if (metv->tday < epc->pnow_minT || metv->tday >= epc->pnow_maxT)
				pnow_Tcoeff = 0;
			else
			{
				/*  between optimal temperature -> 1, below/above linearly decreasing */
				if (metv->tday < epc->pnow_opt1T)
					pnow_Tcoeff = (metv->tday - epc->pnow_minT) / (epc->pnow_opt1T - epc->pnow_minT);
				else
				{
					if (metv->tday < epc->pnow_opt2T)
						pnow_Tcoeff = 1;
					else
						pnow_Tcoeff = (epc->pnow_maxT - metv->tday) / (epc->pnow_maxT - epc->pnow_opt2T);
				}
			
			}
		}
		
		pnow = epc->curgrowth_prop[ap] * pnow_Tcoeff;

		/* control */
		if (pnow_Tcoeff < 0 || pnow_Tcoeff > 1 || pnow < 0 || pnow > 1)
		{
			printf("\n");
			printf("FATAL ERROR in pnow calculation (daily_allocation.c)\n");
			ok=0;
		}
	}
		

	epv->pnow = pnow;
	g1 = epc->GR_ratio;

	/* given the available C, use constant allometric relationships to
	determine how much N is required to meet this potential growth
	demand */

	c_allometry = (1.0+g1);
	n_allometry = (f1/epc->leaf_cn + f2/epc->froot_cn);
    
	if (epc->fruit_cn > 0)    n_allometry +=  f3/epc->fruit_cn;
	if (epc->softstem_cn > 0) n_allometry +=  f4/epc->softstem_cn;
	
	if (woody) n_allometry += (f5/epc->livewood_cn + f6/epc->deadwood_cn + f7/epc->livewood_cn + f8/epc->deadwood_cn);



	/* MULTILAYER SOIL: calculation of soil in rootzone */ 
	for (layer=0; layer < epv->n_rootlayers; layer++)
	{
		potential_immob_rootzone += nt->potential_immob[layer];
	    sminn_rootzone           += ns->sminNH4[layer] * epc->NH4_mobilen_prop + ns->sminNO3[layer] * epc->NO3_mobilen_prop;
	}

	/* now compare the combined decomposition immobilization and plant
	growth N demands against the avsminNH4_totalailable soil mineral N pool. */

	
	plant_ndemand   = avail_c * (n_allometry / c_allometry);

	ns->sum_ndemand     = plant_ndemand + potential_immob_rootzone;

	/* ONLY IN SPINUP PHASE: add N to sminn to meet demand: naddfrac scales N additions from 1.0 to 0.0 */
	if (naddfrac && sminn_rootzone)
	{
		/* The NH4+ pool is buffered to represent an exchangeable pool and a pool in solution that is available for plants, immobilization, nitrification, and leaching */
		if (ns->sum_ndemand > sminn_rootzone)
		{
			for (layer = 0; layer < epv->n_rootlayers; layer++)
			{
				layer_prop = (ns->sminNH4[layer] * epc->NH4_mobilen_prop + ns->sminNO3[layer] * epc->NO3_mobilen_prop) /sminn_rootzone;
				
				dif = (ns->sum_ndemand - sminn_rootzone) * layer_prop;
				sumdif += dif;
				if (dif >= 0)
				{
					ns->sminNH4[layer]  += dif * naddfrac;
					ns->SPINUPsrc	    += dif * naddfrac;
				}
				else
				{
					printf("\n");
					printf("FATAL ERROR in sminn calculation (daily_allocation.c)\n");
					ok=0;
				}
			}
			sminn_rootzone += sumdif * naddfrac;
		}
	}

	if ((fabs(ns->sum_ndemand - sminn_rootzone) > 0) && (fabs(ns->sum_ndemand - sminn_rootzone) < CRIT_PREC)) 
	{
		sminn_rootzone = ns->sum_ndemand;
	}
	/* N availability is not limiting immobilization or plant uptake, and both can proceed at their potential rates */

	if (ns->sum_ndemand <= sminn_rootzone)
	{
		n_limitation = 0;
		actual_immob = potential_immob_rootzone;

		/* Determine the split between retranslocation N and soil mineral N to meet the plant demand */
		sum_plant_nsupply = ns->retransn + sminn_rootzone;
	
		if (sum_plant_nsupply)
		{
			nf->retransn_to_npool = plant_ndemand * (ns->retransn/sum_plant_nsupply);
		}
		else
		{
			nf->retransn_to_npool = 0.0;
		}
		nf->sminn_to_npool = plant_ndemand - nf->retransn_to_npool;

		plant_nalloc       = nf->retransn_to_npool + nf->sminn_to_npool;
		plant_calloc       = avail_c;
	
	}
	else
	{
		/* N availability can not satisfy the sum of immobiliation and plant growth demands, 
		so these two demands compete for available soil mineral N */

		actual_immob = sminn_rootzone * potential_immob_rootzone/ns->sum_ndemand;
		if (potential_immob_rootzone)
		{
			fpi = actual_immob/potential_immob_rootzone;
		}
		else
		{
			fpi = 0.0;
		}
		nf->sminn_to_npool      = sminn_rootzone - actual_immob;
		if (fabs(nf->sminn_to_npool) < CRIT_PREC && nf->sminn_to_npool != 0) nf->sminn_to_npool = 0;
	
		plant_remaining_ndemand = plant_ndemand - nf->sminn_to_npool;

		/* the demand not satisfied by uptake from soil mineral N is now sought from the retranslocated N pool */
		if (plant_remaining_ndemand <= ns->retransn)
		{
			/* there is enough N available in retranslocation pool to
			satisfy the remaining plant N demand */
			nf->retransn_to_npool   = plant_remaining_ndemand;
			plant_nalloc            = nf->retransn_to_npool + nf->sminn_to_npool;
			plant_calloc            = avail_c;

			/* flag for nitrogen limitation type */
			n_limitation = 1;	
		}
		else
		{
			/* there is not enough retranslocation N left to satisfy the
			entire demand. In this case, all remaing retranslocation N is
			used, and the remaining unsatisfied N demand is translated
			back to a C excess, which is deducted proportionally from
			the sun and shade photosynthesis source terms */
			nf->retransn_to_npool    = ns->retransn;
			plant_nalloc             = nf->retransn_to_npool + nf->sminn_to_npool;
			plant_calloc             = plant_nalloc * (c_allometry / n_allometry);
		
			excess_c                 = avail_c - plant_calloc;
			if (day_gpp > 0)
			{
				cf->psnsun_to_cpool   -= excess_c * cf->psnsun_to_cpool/day_gpp;
				cf->psnshade_to_cpool -= excess_c * cf->psnshade_to_cpool/day_gpp;

				day_gpp = cf->psnsun_to_cpool + cf->psnshade_to_cpool;
			}
			else
			{
				printf("\n");
				printf("ERROR: Negative GPP value (daily_allocation.c)\n");
				ok = 0;
			}
			
			n_limitation = 2;		
		}
	}

	epv->excess_c = excess_c;


	/* calculate the amount of new leaf C dictated by these allocation
	decisions, and figure the daily fluxes of C and N to current
	growth and storage pools */
	/* pnow is the proportion of this day's growth that is displayed now,
	the remainder going into storage for display next year through the
	transfer pools */
	/* daily C fluxes out of cpool and into new growth or storage */
	
	if (plant_calloc)
	{
		cf->cpool_to_leafc              = f1 * pnow       * (plant_calloc/c_allometry);
		cf->cpool_to_leafc_storage      = f1 * (1.0-pnow) * (plant_calloc/c_allometry);
		cf->cpool_to_frootc             = f2 * pnow       * (plant_calloc/c_allometry);
		cf->cpool_to_frootc_storage     = f2 * (1.0-pnow) * (plant_calloc/c_allometry);
		cf->cpool_to_fruitc             = f3 * pnow       * (plant_calloc/c_allometry);
		cf->cpool_to_fruitc_storage     = f3 * (1.0-pnow) * (plant_calloc/c_allometry);
		cf->cpool_to_softstemc          = f4 * pnow       * (plant_calloc/c_allometry);
		cf->cpool_to_softstemc_storage  = f4 * (1.0-pnow) * (plant_calloc/c_allometry);

		
		nf->npool_to_leafn              = f1 * pnow       * (1./epc->leaf_cn)     * (plant_calloc/c_allometry);
		nf->npool_to_leafn_storage      = f1 * (1.0-pnow) * (1./epc->leaf_cn)     * (plant_calloc/c_allometry);
		nf->npool_to_frootn             = f2 * pnow       * (1./epc->froot_cn)    * (plant_calloc/c_allometry);
		nf->npool_to_frootn_storage     = f2 * (1.0-pnow) * (1./epc->froot_cn)    * (plant_calloc/c_allometry);
		nf->npool_to_fruitn             = f3 * pnow       * (1./epc->fruit_cn)    * (plant_calloc/c_allometry);
		nf->npool_to_fruitn_storage     = f3 * (1.0-pnow) * (1./epc->fruit_cn)    * (plant_calloc/c_allometry);
		nf->npool_to_softstemn          = f4 * pnow       * (1./epc->softstem_cn) * (plant_calloc/c_allometry);
		nf->npool_to_softstemn_storage  = f4 * (1.0-pnow) * (1./epc->softstem_cn) * (plant_calloc/c_allometry);

		if (epc->woody)
		{
			cf->cpool_to_livestemc          = f5 * pnow       * (plant_calloc/c_allometry);
			cf->cpool_to_livestemc_storage  = f5 * (1.0-pnow) * (plant_calloc/c_allometry);
			cf->cpool_to_deadstemc          = f6 * pnow       * (plant_calloc/c_allometry);
			cf->cpool_to_deadstemc_storage  = f6 * (1.0-pnow) * (plant_calloc/c_allometry);
			cf->cpool_to_livecrootc         = f7 * pnow       * (plant_calloc/c_allometry);
			cf->cpool_to_livecrootc_storage = f7 * (1.0-pnow) * (plant_calloc/c_allometry);
			cf->cpool_to_deadcrootc         = f8 * pnow       * (plant_calloc/c_allometry);
			cf->cpool_to_deadcrootc_storage = f8 * (1.0-pnow) * (plant_calloc/c_allometry);

			nf->npool_to_livestemn          = f5 * pnow       * (1./epc->livewood_cn) * (plant_calloc/c_allometry);
			nf->npool_to_livestemn_storage  = f5 * (1.0-pnow) * (1./epc->livewood_cn) * (plant_calloc/c_allometry);
			nf->npool_to_deadstemn          = f6 * pnow       * (1./epc->deadwood_cn) * (plant_calloc/c_allometry);
			nf->npool_to_deadstemn_storage  = f6 * (1.0-pnow) * (1./epc->deadwood_cn) * (plant_calloc/c_allometry);
			nf->npool_to_livecrootn         = f7 * pnow       * (1./epc->livewood_cn) * (plant_calloc/c_allometry);
			nf->npool_to_livecrootn_storage = f7 * (1.0-pnow) * (1./epc->livewood_cn) * (plant_calloc/c_allometry);
			nf->npool_to_deadcrootn         = f8 * pnow       * (1./epc->deadwood_cn) * (plant_calloc/c_allometry);
			nf->npool_to_deadcrootn_storage = f8 * (1.0-pnow) * (1./epc->deadwood_cn) * (plant_calloc/c_allometry);
		}

	}
	else
	{
		cf->cpool_to_leafc              = 0;
		cf->cpool_to_leafc_storage      = 0;
		cf->cpool_to_frootc             = 0;
		cf->cpool_to_frootc_storage     = 0;
		cf->cpool_to_fruitc             = 0;
		cf->cpool_to_fruitc_storage     = 0;
		cf->cpool_to_softstemc          = 0;
		cf->cpool_to_softstemc_storage  = 0;
		cf->cpool_to_livestemc          = 0;
		cf->cpool_to_livestemc_storage  = 0;
		cf->cpool_to_deadstemc          = 0;
		cf->cpool_to_deadstemc_storage  = 0;
		cf->cpool_to_livecrootc         = 0;
		cf->cpool_to_livecrootc_storage = 0;
		cf->cpool_to_deadcrootc         = 0;
		cf->cpool_to_deadcrootc_storage = 0;

		nf->npool_to_leafn              = 0;
		nf->npool_to_leafn_storage      = 0;
		nf->npool_to_frootn             = 0;
		nf->npool_to_frootn_storage     = 0;
		nf->npool_to_fruitn             = 0;
		nf->npool_to_fruitn_storage     = 0;
		nf->npool_to_softstemn          = 0;
		nf->npool_to_softstemn_storage  = 0;
		nf->npool_to_livestemn          = 0;
		nf->npool_to_livestemn_storage  = 0;
		nf->npool_to_deadstemn          = 0;
		nf->npool_to_deadstemn_storage  = 0;
		nf->npool_to_livecrootn         = 0;
		nf->npool_to_livecrootn_storage = 0;
		nf->npool_to_deadcrootn         = 0;
		nf->npool_to_deadcrootn_storage = 0;
	
	}
	
	epv->plant_calloc = plant_calloc; 
	epv->plant_nalloc = plant_nalloc; 
	
	/* calculate the amount of carbon that needs to go into growth respiration storage to satisfy all of the storage growth demands. 
	Note that in version 4.1, this function has been changed to allow for the fraction of growth respiration that is released at the
	time of fixation, versus the remaining fraction that is stored forrelease at the time of display. Note that all the growth respiration
	fluxes that get released on a given day are calculated in growth_resp(), but that the storage of C for growth resp during display of 
	transferred growth is assigned here. (GRPNOW: proportion of growth resp to release at fixation ) */
	
	cf->cpool_to_gresp_storage = (cf->cpool_to_leafc_storage + cf->cpool_to_frootc_storage + cf->cpool_to_fruitc_storage + cf->cpool_to_softstemc_storage +
                                  cf->cpool_to_livestemc_storage + cf->cpool_to_deadstemc_storage +
						          cf->cpool_to_livecrootc_storage + cf->cpool_to_deadcrootc_storage) * g1 * (1.0-GRPNOW);
	

	
	/* now use the N limitation information to assess the final decomposition fluxes. Mineralizing fluxes (pmnf* < 0.0) occur at the potential rate
	regardless of the competing N demands between microbial processes and plant uptake, but immobilizing fluxes are reduced when soil mineral N is limiting */

	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		/* calculate litter and soil compartment C:N ratios */
		if (ns->litr1n[layer] > 0.0) cn_l1 = cs->litr1c[layer]/ns->litr1n[layer];
		if (ns->litr2n[layer] > 0.0) cn_l2 = cs->litr2c[layer]/ns->litr2n[layer];
		if (ns->litr4n[layer] > 0.0) cn_l4 = cs->litr4c[layer]/ns->litr4n[layer];

		if (ns->soil1n[layer] > 0.0) cn_s1 = cs->soil1c[layer]/ns->soil1n[layer];
		if (ns->soil2n[layer] > 0.0) cn_s2 = cs->soil2c[layer]/ns->soil2n[layer];
		if (ns->soil3n[layer] > 0.0) cn_s3 = cs->soil3c[layer]/ns->soil3n[layer];
		if (ns->soil4n[layer] > 0.0) cn_s4 = cs->soil4c[layer]/ns->soil4n[layer];

	

		/* labile litter fluxes */
		if (cs->litr1c[layer] > 0.0)
		{
			if (n_limitation && nt->pmnf_l1s1[layer] > 0.0)
			{
				nt->plitr1c_loss[layer] *= fpi;
				nt->pmnf_l1s1[layer]    *= fpi;
			}
			cf->litr1_hr[layer]         = rfl1s1 * nt->plitr1c_loss[layer];
			cf->litr1c_to_soil1c[layer] = (1.0 - rfl1s1) * nt->plitr1c_loss[layer];

		
			if (ns->litr1n[layer] > 0.0) 
				nf->litr1n_to_soil1n[layer] = nt->plitr1c_loss[layer] / cn_l1;
			else 
				nf->litr1n_to_soil1n[layer] = 0.0;

			nf->sminn_to_soil1n_l1[layer] = nt->pmnf_l1s1[layer];
			daily_net_nmin               -= nt->pmnf_l1s1[layer];
		}

		/* cellulose litter fluxes */
		if (cs->litr2c[layer] > 0.0)
		{
			if (n_limitation && nt->pmnf_l2s2[layer] > 0.0)
			{
				nt->plitr2c_loss[layer] *= fpi;
				nt->pmnf_l2s2[layer]    *= fpi;
			}
			cf->litr2_hr[layer]         = rfl2s2 * nt->plitr2c_loss[layer];
			cf->litr2c_to_soil2c[layer] = (1.0 - rfl2s2) * nt->plitr2c_loss[layer];

			if (ns->litr2n[layer] > 0.0) 
				nf->litr2n_to_soil2n[layer] = nt->plitr2c_loss[layer] / cn_l2;
			else 
				nf->litr2n_to_soil2n[layer] = 0.0;

			nf->sminn_to_soil2n_l2[layer] = nt->pmnf_l2s2[layer];
			daily_net_nmin               -= nt->pmnf_l2s2[layer];
		}

		/* release of shielded cellulose litter, tied to the decay rate of
		lignin litter */
		if (cs->litr3c[layer] > 0.0)
		{
			if (n_limitation && nt->pmnf_l4s3[layer] > 0.0)
			{
				cf->litr3c_to_litr2c[layer] = nt->kl4[layer] * cs->litr3c[layer] * fpi;
				nf->litr3n_to_litr2n[layer] = nt->kl4[layer] * ns->litr3n[layer] * fpi;
			}
			else
			{
				cf->litr3c_to_litr2c[layer] = nt->kl4[layer] * cs->litr3c[layer];
				nf->litr3n_to_litr2n[layer] = nt->kl4[layer] * ns->litr3n[layer];
			}
		}

		/* lignin litter fluxes */
		if (cs->litr4c[layer] > 0.0)
		{
			if (n_limitation && nt->pmnf_l4s3[layer] > 0.0)
			{
				nt->plitr4c_loss[layer] *= fpi;
				nt->pmnf_l4s3[layer] *= fpi;
			}
			cf->litr4_hr[layer]         = rfl4s3 * nt->plitr4c_loss[layer];
			cf->litr4c_to_soil3c[layer] = (1.0 - rfl4s3) * nt->plitr4c_loss[layer];

			if (ns->litr4n[layer] > 0.0) 
				nf->litr4n_to_soil3n[layer] = nt->plitr4c_loss[layer] / cn_l4;
			else 
				nf->litr4n_to_soil3n[layer] = 0.0;

			nf->sminn_to_soil3n_l4[layer] = nt->pmnf_l4s3[layer];
			daily_net_nmin               -= nt->pmnf_l4s3[layer];
		}
		
		/* fast microbial recycling pool */
		if (cs->soil1c[layer] > 0.0)
		{
			if (n_limitation && nt->pmnf_s1s2[layer] > 0.0)
			{
				nt->psoil1c_loss[layer] *= fpi;
				nt->pmnf_s1s2[layer] *= fpi;
			}
			cf->soil1_hr[layer]				= rfs1s2 * nt->psoil1c_loss[layer];
			cf->soil1c_to_soil2c[layer]		= (1.0 - rfs1s2) * nt->psoil1c_loss[layer];
			nf->soil1n_to_soil2n[layer]		= nt->psoil1c_loss[layer] / cn_s1;
			nf->sminn_to_soil2n_s1[layer]   = nt->pmnf_s1s2[layer];
			daily_net_nmin                 -= nt->pmnf_s1s2[layer];
		}
		
		/* medium microbial recycling pool */
		if (cs->soil2c[layer] > 0.0)
		{
			if (n_limitation && nt->pmnf_s2s3[layer] > 0.0)
			{
				nt->psoil2c_loss[layer] *= fpi;
				nt->pmnf_s2s3[layer] *= fpi;
			}
			cf->soil2_hr[layer]           = rfs2s3 * nt->psoil2c_loss[layer];
			cf->soil2c_to_soil3c[layer]   = (1.0 - rfs2s3) * nt->psoil2c_loss[layer];
			nf->soil2n_to_soil3n[layer]   = nt->psoil2c_loss[layer] / cn_s2;
			nf->sminn_to_soil3n_s2[layer] = nt->pmnf_s2s3[layer];
			daily_net_nmin               -= nt->pmnf_s2s3[layer];
		}

		/* slow microbial recycling pool */
		if (cs->soil3c[layer] > 0.0)
		{
			if (n_limitation && nt->pmnf_s3s4[layer] > 0.0)
			{
				nt->psoil3c_loss[layer] *= fpi;
				nt->pmnf_s3s4[layer] *= fpi;
			}
			cf->soil3_hr[layer]           = rfs3s4 * nt->psoil3c_loss[layer];
			cf->soil3c_to_soil4c[layer]   = (1.0 - rfs3s4) * nt->psoil3c_loss[layer];
			nf->soil3n_to_soil4n[layer]   = nt->psoil3c_loss[layer] / cn_s3;
			nf->sminn_to_soil4n_s3[layer] = nt->pmnf_s3s4[layer];
			daily_net_nmin               -= nt->pmnf_s3s4[layer];
		}
		
		/* recalcitrant SOM pool (rf = 1.0, always mineralizing) */
		if (cs->soil4c[layer] > 0.0)
		{
			cf->soil4_hr[layer]           = nt->psoil4c_loss[layer];
			nf->soil4n_to_sminNH4[layer]  = nt->psoil4c_loss[layer] / cn_s4;
			daily_net_nmin                += nf->soil4n_to_sminNH4[layer];
		}
		
		/* store the day's net N mineralization */
		epv->daily_net_nmin[layer]     = daily_net_nmin;
		epv->daily_gross_nimmob[layer] = actual_immob;
		epv->n_limitation[layer]       = n_limitation;

	}
		
	return (!ok);
}

