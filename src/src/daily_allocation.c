/*
daily_allocation.c  
daily allocation of carbon and nitrogen, as well as the final reconciliation
of N immobilization by microbes (see decomp.c)

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.4
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2017, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

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

int daily_allocation(const epconst_struct* epc, const siteconst_struct* sitec, 
					 cflux_struct* cf, cstate_struct* cs,nflux_struct* nf, nstate_struct* ns, epvar_struct* epv, ntemp_struct* nt)
{
	int ok=1;
    double day_gpp;     /* daily gross production */
	double day_mresp;   /* daily total maintenance respiration */
	double avail_c;     /* total C available for new production */
	double f1;          /* RATIO   new fine root C : new leaf C     */
	double f2;          /* RATIO   new fruit C : new leaf C - fruit simulation (Hidy 2013.) */
	double f3;          /* RATIO   new softstem C : new leaf C - softstem simulation (Hidy 2015.) */
	double f4;          /* RATIO   new coarse root C : new stem C   */
	double f5;          /* RATIO   new stem C : new leaf C          */
	double f6;          /* RATIO   new live wood C : new wood C     */
	double g1;          /* RATIO   C respired for growth : C grown  */ 
	double g2;          /* proportion of growth resp to release at fixation */
	double cnl;         /* RATIO   leaf C:N */  
	double cnf;         /* RATIO   fruit C:N - fruit simulation (Hidy 2013.) */
	double cnss;        /* RATIO   softstem C:N - softstem simulation (Hidy 2013.) */
	double cnfr;        /* RATIO   fine root C:N */
	double cnlw;        /* RATIO   live wood C:N */
	double cndw;        /* RATIO   dead wood C:N */
	double nlc;         /* actual new leaf C, minimum of C and N limits   */      
	double pnow;        /* proportion of growth displayed on current day */ 
	double gresp_storage;  
	int woody;
	double c_allometry, n_allometry;
	double plant_ndemand, sum_ndemand;
	double actual_immob;
	double sum_plant_nsupply;
	double plant_nalloc, plant_calloc;
	double plant_remaining_ndemand;
    double excess_c;
	int nlimit;
	double cn_l1,cn_l2,cn_l4,cn_s1,cn_s2,cn_s3,cn_s4;
	double rfl1s1, rfl2s2, rfl4s3, rfs1s2, rfs2s3, rfs3s4;
	double daily_net_nmin, bulk_denitrif_prop;
	double fpi = 0.0;
	double excessn = 0;

    
     
    int n_limitation = 0;   /* nitrogen limitation flag Hidy 2009. */

 	cn_l1 = cn_l2 = cn_l4 = cn_s1 = cn_s2 = cn_s3 = cn_s4 = 0;
	
	woody = epc->woody;

	
	/* Assess the carbon availability on the basis of this day's
	gross production and maintenance respiration costs */
	day_gpp = cf->psnsun_to_cpool + cf->psnshade_to_cpool;
	
     /* TREE-SPECIFIC and NON_WOODY specifix fluxes - Hidy 2015 */
     if (woody)
	{
		day_mresp = cf->leaf_day_mr + cf->leaf_night_mr + cf->froot_mr + cf->fruit_mr +  /* fruit simulation (Hidy 2013.) */

			cf->livestem_mr + cf->livecroot_mr;
	}
	else
	{
		day_mresp = cf->leaf_day_mr + cf->leaf_night_mr + cf->froot_mr + 
                       cf->fruit_mr + cf->softstem_mr;  /* fruit and softstem simulation (Hidy 2013.) */
	}
	avail_c = day_gpp - day_mresp;
	
	/* no allocation when the daily C balance is negative */
	if (avail_c < 0.0) avail_c = 0.0;

	/* test for cpool deficit */
	if (cs->cpool < 0.0)
	{
		/* running a deficit in cpool, so the first priority
		is to let today's available C accumulate in cpool.  The actual
		accumulation in the cpool is resolved in day_carbon_state(). */
		if (-cs->cpool < avail_c)
		{
			/* cpool deficit is less than the available
			carbon for the day, so aleviate cpool deficit
			and use the rest of the available carbon for 
			new growth and storage. Remember that fluxes in
			and out of the cpool are reconciled at the end
			of the daily loop, so for now, just keep track
			of the amount of daily GPP-MR that is not needed
			to restore a negative cpool. */
			avail_c += cs->cpool; 
		}
		else
		{
			/* cpool deficit is >= available C, so all of the
			daily GPP, if any, is used to alleviate negative cpool */
			avail_c = 0.0;
		}
	} /* end if negative cpool */
	
	/* assign local values for the allocation control parameters */
	f1 = epc->alloc_frootc_leafc;
	/* fruit simulation */
	f2 = epc->alloc_fruitc_leafc;
	/* sofstem simulation */
	f3 = epc->alloc_softstemc_leafc;
	f4 = epc->alloc_crootc_stemc;
	f5 = epc->alloc_newstemc_newleafc; 
	f6 = epc->alloc_newlivewoodc_newwoodc;


	g1 = epc->GR_ratio;
	g2 = GRPNOW;
	cnl = epc->leaf_cn;
	cnf = epc->fruit_cn;
	cnss = epc->softstem_cn;
	cnfr = epc->froot_cn;
	cnlw = epc->livewood_cn;
	cndw = epc->deadwood_cn;
	pnow = epc->alloc_prop_curgrowth;
	
	/* given the available C, use constant allometric relationships to
	determine how much N is required to meet this potential growth
	demand */
	/* TREE-specific and NON-WOODY SPECIFIC fluxes */
	if (woody)
	{
		c_allometry = ((1.0+g1)*(1.0 + f1 + f5*(1.0+f4)));
		n_allometry = (1.0/cnl + f1/cnfr + f2/cnf +(f5*f6*(1.0+f4))/cnlw + 
			(f5*(1.0-f6)*(1.0+f4))/cndw);
	}
	else
	{
		c_allometry = (1.0 + g1 + f1 + f1*g1 + f3 + f3*g1);
		n_allometry = (1.0/cnl + f1/cnfr + f3/cnss);
	}

	if (epv->flowering)
	{
		c_allometry += f2 + f2 * g1;
		n_allometry += f2 / cnf;
		

	}
	plant_ndemand = avail_c * (n_allometry / c_allometry);


	/* now compare the combined decomposition immobilization and plant
	growth N demands against the available soil mineral N pool. */
	sum_ndemand = plant_ndemand + nt->potential_immob;

	ns->sum_ndemand = sum_ndemand;
/* Hidy 2013 - NO spinup control: add N to sminn to meet demand */
	/* naddfrac scales N additions from 1.0 to 0.0 */
/*	if (sum_ndemand > ns->sminn_RZ)
	{
		dif = sum_ndemand - ns->sminn[0];
		ns->sminn[0] += dif * naddfrac;
		ns->sminn_RZ += dif * naddfrac;
		ns->ndep_src += dif * naddfrac;
		nf->nplus    = dif;
	}
	else
		nf->nplus    = 0;
		*/
	if (sum_ndemand <= ns->sminn_RZ)
	{
		/* N availability is not limiting immobilization or plant
		uptake, and both can proceed at their potential rates */
		n_limitation = 0;		// Hidy 2009.

		actual_immob = nt->potential_immob;
		nlimit = 0;
		
		/* Determine the split between retranslocation N and soil mineral
		N to meet the plant demand */
		sum_plant_nsupply = ns->retransn + ns->sminn_RZ;
		if (sum_plant_nsupply)
		{
			nf->retransn_to_npool = plant_ndemand *
				(ns->retransn/sum_plant_nsupply);
		}
		else
		{
			nf->retransn_to_npool = 0.0;
		}
		nf->sminn_to_npool = plant_ndemand - nf->retransn_to_npool;
		plant_nalloc = nf->retransn_to_npool + nf->sminn_to_npool;
		plant_calloc = avail_c;

		/* under conditions of excess N, some proportion of excess N is
		assumed to be lost to denitrification, in addition to the constant
		proportion lost in the decomposition pathways. */
		excessn = ns->sminn_RZ - sum_ndemand;

		if (epv->vwc[0] > 0.95 * sitec->vwc_sat[0]) 
			bulk_denitrif_prop = epc->bulkN_denitrif_prop_WET;
		else
			bulk_denitrif_prop = epc->bulkN_denitrif_prop_DRY;

		nf->sminn_to_denitrif = excessn * bulk_denitrif_prop;
	}
	else
	{
		/* N availability can not satisfy the sum of immobiliation and
		plant growth demands, so these two demands compete for available
		soil mineral N */
		nlimit = 1;
		actual_immob = ns->sminn_RZ * (nt->potential_immob/sum_ndemand);
		if (nt->potential_immob)
		{
			fpi = actual_immob/nt->potential_immob;
		}
		else
		{
			fpi = 0.0;
		}
		nf->sminn_to_npool = ns->sminn_RZ - actual_immob;
		plant_remaining_ndemand = plant_ndemand - nf->sminn_to_npool;
		/* the demand not satisfied by uptake from soil mineral N is
		now sought from the retranslocated N pool */
		if (plant_remaining_ndemand <= ns->retransn)
		{
			/* there is enough N available in retranslocation pool to
			satisfy the remaining plant N demand */
			nf->retransn_to_npool = plant_remaining_ndemand;
			plant_nalloc = nf->retransn_to_npool + nf->sminn_to_npool;
			plant_calloc = avail_c;

			/* Hidy 2010. -  flag for nitrogen limitation type */
			n_limitation = 1;	
		}
		else
		{
			/* there is not enough retranslocation N left to satisfy the
			entire demand. In this case, all remaing retranslocation N is
			used, and the remaining unsatisfied N demand is translated
			back to a C excess, which is deducted proportionally from
			the sun and shade photosynthesis source terms */
			nf->retransn_to_npool = ns->retransn;
			plant_nalloc = nf->retransn_to_npool + nf->sminn_to_npool;
			plant_calloc = plant_nalloc * (c_allometry / n_allometry);
			excess_c = avail_c - plant_calloc;
			if (day_gpp > 0)
			{
				cf->psnsun_to_cpool -= excess_c * cf->psnsun_to_cpool/day_gpp;
				cf->psnshade_to_cpool -= excess_c * cf->psnshade_to_cpool/day_gpp;
			}
			else
			{
				printf("FATAL ERROR: Negative GPP value (daily_allocation.c)\n");
				ok = 1;
			}
			
			n_limitation = 2;		// Hidy 2009.
		}
	}


	
	/* calculate the amount of new leaf C dictated by these allocation
	decisions, and figure the daily fluxes of C and N to current
	growth and storage pools */
	/* pnow is the proportion of this day's growth that is displayed now,
	the remainder going into storage for display next year through the
	transfer pools */

	/* actual new leaf C, minimum of C and N limits   */
	nlc = plant_calloc / c_allometry; 

	/* fruit simulation - Hidy 2013.: after flowering date the priority is to allucating fruit */
	if (plant_calloc > 0 && epv->flowering)
	{
		cf->cpool_to_fruitc             = nlc * f2 * pnow;
		cf->cpool_to_fruitc_storage     = nlc * f2 * (1.0-pnow);
		nf->npool_to_fruitn             = (nlc * f2 / cnf) * pnow;
		nf->npool_to_fruitn_storage     = (nlc * f2 / cnf) * (1.0-pnow);
		plant_calloc                   -= cf->cpool_to_fruitc;
	    plant_calloc                   -= cf->cpool_to_fruitc_storage;
		plant_nalloc                   -= nf->npool_to_fruitn;
	    plant_nalloc                   -= nf->npool_to_fruitn_storage;

	
	}
	else
	{
	    cf->cpool_to_fruitc = 0.0;
        cf->cpool_to_fruitc_storage = 0.0;
        nf->npool_to_fruitn = 0.0;
        nf->npool_to_fruitn_storage = 0.0;
	}



	/* daily C fluxes out of cpool and into new growth or storage */
	cf->cpool_to_leafc              = nlc * pnow;
	cf->cpool_to_leafc_storage      = nlc * (1.0-pnow);
	cf->cpool_to_frootc             = nlc * f1 * pnow;
	cf->cpool_to_frootc_storage     = nlc * f1 * (1.0-pnow);


	/* TREE-specific and NON-WOODY SPECIFIC fluxes */
	if (woody)
	{
		cf->cpool_to_livestemc          = nlc * f5 * f6 * pnow;
		cf->cpool_to_livestemc_storage  = nlc * f5 * f6 * (1.0-pnow);
		cf->cpool_to_deadstemc          = nlc * f5 * (1.0-f6) * pnow;
		cf->cpool_to_deadstemc_storage  = nlc * f5 * (1.0-f6) * (1.0-pnow);
		cf->cpool_to_livecrootc         = nlc * f4 * f5 * f6 * pnow;
		cf->cpool_to_livecrootc_storage = nlc * f4 * f5 * f6 * (1.0-pnow);
		cf->cpool_to_deadcrootc         = nlc * f4 * f5 * (1.0-f6) * pnow;
		cf->cpool_to_deadcrootc_storage = nlc * f4 * f5 * (1.0-f6) * (1.0-pnow);
	}
	else
	{
		/* SOFT STEM SIMULATION of non-woody biomes - Hidy 2015 */
		cf->cpool_to_softstemc             = nlc * f3 * pnow;
		cf->cpool_to_softstemc_storage     = nlc * f3 * (1.0-pnow);
	}
	/* daily N fluxes out of npool and into new growth or storage */
	nf->npool_to_leafn              = (nlc / cnl) * pnow;
	nf->npool_to_leafn_storage      = (nlc / cnl) * (1.0-pnow);
	nf->npool_to_frootn             = (nlc * f1 / cnfr) * pnow;
	nf->npool_to_frootn_storage     = (nlc * f1 / cnfr) * (1.0-pnow);


	 /* TREE-specific and NON-WOODY SPECIFIC fluxes */
	if (woody)
	{
		nf->npool_to_livestemn          = (nlc * f5 * f6 / cnlw) * pnow;
		nf->npool_to_livestemn_storage  = (nlc * f5 * f6 / cnlw) * (1.0-pnow);
		nf->npool_to_deadstemn          = (nlc * f5 * (1.0-f6) / cndw) * pnow;
		nf->npool_to_deadstemn_storage  = (nlc * f5 * (1.0-f6) / cndw) * (1.0-pnow);
		nf->npool_to_livecrootn         = (nlc * f4 * f5 * f6 / cnlw) * pnow;
		nf->npool_to_livecrootn_storage = (nlc * f4 * f5 * f6 / cnlw) * (1.0-pnow);
		nf->npool_to_deadcrootn         = (nlc * f4 * f5 * (1.0-f6) / cndw) * pnow;
		nf->npool_to_deadcrootn_storage = (nlc * f4 * f5 * (1.0-f6) / cndw) * (1.0-pnow);
	}
	else
	{
		/* SOFT STEM SIMULATION of non-woody biomes - Hidy 2015 */
		nf->npool_to_softstemn             = (nlc * f3 / cnss) * pnow;
		nf->npool_to_softstemn_storage     = (nlc * f3 / cnss) * (1.0-pnow);
    
	}
	
	/* calculate the amount of carbon that needs to go into growth
	respiration storage to satisfy all of the storage growth demands. 
	Note that in version 4.1, this function has been changed to 
	allow for the fraction of growth respiration that is released at the
	time of fixation, versus the remaining fraction that is stored for
	release at the time of display. Note that all the growth respiration
	fluxes that get released on a given day are calculated in growth_resp(),
	but that the storage of C for growth resp during display of transferred
	growth is assigned here. */
	
	
	/* TREE-specific and NON-WOODY SPECIFIC fluxes */
	if (woody)
	{
		gresp_storage = (cf->cpool_to_leafc_storage + cf->cpool_to_frootc_storage 
						+ cf->cpool_to_fruitc_storage +  /* fruit simulation - Hidy 2013.) */
						+ cf->cpool_to_livestemc_storage + cf->cpool_to_deadstemc_storage
						+ cf->cpool_to_livecrootc_storage + cf->cpool_to_deadcrootc_storage)
						* g1 * (1.0-g2);
	}
	else
	{   /* SOFT STEM SIMULATION of non-woody biomes - Hidy 2015 */
		gresp_storage = (cf->cpool_to_leafc_storage + cf->cpool_to_frootc_storage 
						+ cf->cpool_to_fruitc_storage +     /* fruit simulation - Hidy 2013.) */
						+ cf->cpool_to_softstemc_storage)  /* softstem simulation - Hidy 2015.) */
						* g1 * (1.0-g2);
	}
	
	cf->cpool_to_gresp_storage = gresp_storage;	

	/* now use the N limitation information to assess the final decomposition
	fluxes. Mineralizing fluxes (pmnf* < 0.0) occur at the potential rate
	regardless of the competing N demands between microbial processes and
	plant uptake, but immobilizing fluxes are reduced when soil mineral
	N is limiting */
	/* calculate litter and soil compartment C:N ratios */
	if (ns->litr1n > 0.0) cn_l1 = cs->litr1c/ns->litr1n;
	if (ns->litr2n > 0.0) cn_l2 = cs->litr2c/ns->litr2n;
	if (ns->litr4n > 0.0) cn_l4 = cs->litr4c/ns->litr4n;
	cn_s1 = SOIL1_CN;
	cn_s2 = SOIL2_CN;
	cn_s3 = SOIL3_CN;
	cn_s4 = SOIL4_CN;
	/* respiration fractions for fluxes between compartments */
	rfl1s1 = 0.39;
	rfl2s2 = 0.55;
	rfl4s3 = 0.29;
	rfs1s2 = 0.28;
	rfs2s3 = 0.46;
	rfs3s4 = 0.55;
	
	daily_net_nmin = 0.0;
	/* labile litter fluxes */
	if (cs->litr1c > 0.0)
	{
		if (nlimit && nt->pmnf_l1s1 > 0.0)
		{
			nt->plitr1c_loss *= fpi;
			nt->pmnf_l1s1 *= fpi;
		}
		cf->litr1_hr = rfl1s1 * nt->plitr1c_loss;
		cf->litr1c_to_soil1c = (1.0 - rfl1s1) * nt->plitr1c_loss;

	
		if (ns->litr1n > 0.0) nf->litr1n_to_soil1n = nt->plitr1c_loss / cn_l1;
		else nf->litr1n_to_soil1n = 0.0;
		nf->sminn_to_soil1n_l1 = nt->pmnf_l1s1;
		daily_net_nmin -= nt->pmnf_l1s1;
	}

	/* cellulose litter fluxes */
	if (cs->litr2c > 0.0)
	{
		if (nlimit && nt->pmnf_l2s2 > 0.0)
		{
			nt->plitr2c_loss *= fpi;
			nt->pmnf_l2s2 *= fpi;
		}
		cf->litr2_hr = rfl2s2 * nt->plitr2c_loss;
		cf->litr2c_to_soil2c = (1.0 - rfl2s2) * nt->plitr2c_loss;
		if (ns->litr2n > 0.0) nf->litr2n_to_soil2n = nt->plitr2c_loss / cn_l2;
		else nf->litr2n_to_soil2n = 0.0;
		nf->sminn_to_soil2n_l2 = nt->pmnf_l2s2;
		daily_net_nmin -= nt->pmnf_l2s2;
	}

	/* release of shielded cellulose litter, tied to the decay rate of
	lignin litter */
	if (cs->litr3c > 0.0)
	{
		if (nlimit && nt->pmnf_l4s3 > 0.0)
		{
			cf->litr3c_to_litr2c = nt->kl4 * cs->litr3c * fpi;
			nf->litr3n_to_litr2n = nt->kl4 * ns->litr3n * fpi;
		}
		else
		{
			cf->litr3c_to_litr2c = nt->kl4 * cs->litr3c;
			nf->litr3n_to_litr2n = nt->kl4 * ns->litr3n;
		}
	}

	/* lignin litter fluxes */
	if (cs->litr4c > 0.0)
	{
		if (nlimit && nt->pmnf_l4s3 > 0.0)
		{
			nt->plitr4c_loss *= fpi;
			nt->pmnf_l4s3 *= fpi;
		}
		cf->litr4_hr = rfl4s3 * nt->plitr4c_loss;
		cf->litr4c_to_soil3c = (1.0 - rfl4s3) * nt->plitr4c_loss;
		if (ns->litr4n > 0.0) nf->litr4n_to_soil3n = nt->plitr4c_loss / cn_l4;
		else nf->litr4n_to_soil3n = 0.0;
		nf->sminn_to_soil3n_l4 = nt->pmnf_l4s3;
		daily_net_nmin -= nt->pmnf_l4s3;
	}
	
	/* fast microbial recycling pool */
	if (cs->soil1c > 0.0)
	{
		if (nlimit && nt->pmnf_s1s2 > 0.0)
		{
			nt->psoil1c_loss *= fpi;
			nt->pmnf_s1s2 *= fpi;
		}
		cf->soil1_hr = rfs1s2 * nt->psoil1c_loss;
		cf->soil1c_to_soil2c = (1.0 - rfs1s2) * nt->psoil1c_loss;
		nf->soil1n_to_soil2n = nt->psoil1c_loss / cn_s1;
		nf->sminn_to_soil2n_s1 = nt->pmnf_s1s2;
		daily_net_nmin -= nt->pmnf_s1s2;
	}
	
	/* medium microbial recycling pool */
	if (cs->soil2c > 0.0)
	{
		if (nlimit && nt->pmnf_s2s3 > 0.0)
		{
			nt->psoil2c_loss *= fpi;
			nt->pmnf_s2s3 *= fpi;
		}
		cf->soil2_hr = rfs2s3 * nt->psoil2c_loss;
		cf->soil2c_to_soil3c = (1.0 - rfs2s3) * nt->psoil2c_loss;
		nf->soil2n_to_soil3n = nt->psoil2c_loss / cn_s2;
		nf->sminn_to_soil3n_s2 = nt->pmnf_s2s3;
		daily_net_nmin -= nt->pmnf_s2s3;
	}

	/* slow microbial recycling pool */
	if (cs->soil3c > 0.0)
	{
		if (nlimit && nt->pmnf_s3s4 > 0.0)
		{
			nt->psoil3c_loss *= fpi;
			nt->pmnf_s3s4 *= fpi;
		}
		cf->soil3_hr = rfs3s4 * nt->psoil3c_loss;
		cf->soil3c_to_soil4c = (1.0 - rfs3s4) * nt->psoil3c_loss;
		nf->soil3n_to_soil4n = nt->psoil3c_loss / cn_s3;
		nf->sminn_to_soil4n_s3 = nt->pmnf_s3s4;
		daily_net_nmin -= nt->pmnf_s3s4;
	}
	
	/* recalcitrant SOM pool (rf = 1.0, always mineralizing) */
	if (cs->soil4c > 0.0)
	{
		cf->soil4_hr = nt->psoil4c_loss;
		nf->soil4n_to_sminn = nt->psoil4c_loss / cn_s4;
		daily_net_nmin += nf->soil4n_to_sminn;
	}
	
	/* store the day's net N mineralization */
	epv->daily_net_nmin = daily_net_nmin;
	epv->daily_gross_nimmob = actual_immob;

	epv->n_limitation = n_limitation; // Hidy 2009.
	
	return (!ok);
}

