/* 
senescence.c
calculation of daily senescence mortality fluxes (due to drought/water stress)
Senescence mortality: these fluxes all enter litter sinks due to  low VWC during a long period

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Copyright 2020, D. Hidy [dori.hidy@gmail.com]
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

int senescence(const siteconst_struct *sitec, const epconst_struct* epc, const grazing_struct* GRZ, const metvar_struct* metv, 
			   control_struct* ctrl, cstate_struct* cs, cflux_struct* cf,nstate_struct* ns, nflux_struct* nf, epvar_struct* epv)
{
	int errorCode=0;
	int layer;

	/* mortality parameters */
	double SNSCmort_abovebiom, SNSCmort_belowbiom, SNSCmort_nsc;
	double mort_SNSC_to_litter = 0;
	double propLAYER0, propLAYER1, propLAYER2;
	double STDB_CN;
	
	SNSCmort_abovebiom=SNSCmort_belowbiom=SNSCmort_nsc=STDB_CN=0;
	
	/* initalizing N flux to retranslocation due to senescence */
	nf->SNSC_to_retrans = 0;
	
	/*  SMSI - multiplication of m_SWCstress, m_extremT and m_SWCstressLENGTH */
	/* calculating EXTREM temperature effect */
	
	if (metv->tmax < epc->SNSC_extremT1)
		epv->m_extremT = 1;
	else
	{
		/*  above critical temperature -> max, below linearly decreasing */
		if (metv->tmax < epc->SNSC_extremT2)
			epv->m_extremT = 1 - (metv->tmax - epc->SNSC_extremT1) / (epc->SNSC_extremT2 - epc->SNSC_extremT1);
		else
			epv->m_extremT = 0;
			
	}
	/* control */
	if (epv->m_extremT < 0 || epv->m_extremT > 1)
	{
		printf("\n");
		printf("FATAL ERROR in extremT_effect calculation (senescence.c)\n");
		errorCode=1;
	}

	if ((cs->leafc > 0 && ns->leafn > 0) || (cs->frootc > 0 && ns->frootn > 0) || (cs->softstemc > 0 && ns->softstemn > 0))
	{	
		epv->SMSI = 1 - epv->m_SWCstress * epv->m_extremT * epv->m_SWCstressLENGTH;

		/* control */
		if (epv->SMSI < 0)
		{
			if (fabs(epv->SMSI) > CRIT_PREC)
			{
				printf("\n");
				printf("FATAL ERROR in SMSI calculation (senescence.c)\n");
				errorCode=1;
			}
			else
				epv->SMSI = 0;
		}

		SNSCmort_abovebiom = (epc->SNSCmort_abovebiom_max * epv->SMSI);
		if (SNSCmort_abovebiom > 1) SNSCmort_abovebiom = 1;
		
		SNSCmort_belowbiom = (epc->SNSCmort_belowbiom_max * epv->SMSI); 
		if (SNSCmort_belowbiom > 1) SNSCmort_belowbiom = 1;

		SNSCmort_nsc = (epc->SNSCmort_nsc_max * epv->SMSI); 
		if (SNSCmort_nsc > 1) SNSCmort_nsc = 1;

		/* control */
		if (SNSCmort_abovebiom > 1  || SNSCmort_belowbiom > 1 || SNSCmort_nsc > 1)
		{
			printf("\n");
			printf("FATAL ERROR in senescence mortality calculation (senescence.c)\n");
			errorCode=1;
		}
	
	}	
	else
		epv->SMSI = 0;



	/* 1.4. determine the actual turnover rate of wilted standing biomass to litter parameter */
	if (cf->leafc_to_GRZ > 0)
	{
		mort_SNSC_to_litter = GRZ->trampleff_act * epc->mort_SNSC_to_litter;
		if (mort_SNSC_to_litter > 1) mort_SNSC_to_litter = 1;
	}
	else mort_SNSC_to_litter = epc->mort_SNSC_to_litter;


	/****************************************************************************************/
	/* 2. genetically programmed senescence */

	if (!errorCode && genprog_senescence(epc, metv, epv, cf, nf))
	{
		printf("\n");
		printf("ERROR in call to genprog_senescence() from senescence()\n");
		errorCode=1;
	}


	/****************************************************************************************/
	/* 3.  mortality fluxes:leaf, fine root, fruit, softstem, gresp, retrans */

	if (SNSCmort_belowbiom > 0 || SNSCmort_belowbiom > 0 || SNSCmort_nsc > 0)
	{
		/* in order to save the C:N ratio: N-fluxes are calculated from C-fluxes using C:N ratio parameters */
		if (epc->leaf_cn)
		{
			STDB_CN = epc->leaflitr_cn;
			if (STDB_CN <= 0) errorCode=1;

			cf->m_leafc_to_SNSC				= SNSCmort_abovebiom * cs->leafc;  
			cf->m_leafc_storage_to_SNSC		= SNSCmort_nsc * cs->leafc_storage;
			cf->m_leafc_transfer_to_SNSC	= SNSCmort_nsc * cs->leafc_transfer;
	
			nf->m_leafn_to_SNSC				= cf->m_leafc_to_SNSC / epc->leaf_cn;  	
			nf->m_leafn_storage_to_SNSC		= cf->m_leafc_storage_to_SNSC / epc->leaf_cn; 
			nf->m_leafn_transfer_to_SNSC	= cf->m_leafc_transfer_to_SNSC / epc->leaf_cn; 

			nf->leafSNSC_to_retrans          = nf->m_leafn_to_SNSC - cf->m_leafc_to_SNSC/STDB_CN;
			nf->leaf_transferSNSC_to_retrans = nf->m_leafn_transfer_to_SNSC - cf->m_leafc_transfer_to_SNSC/STDB_CN;
			nf->leaf_storageSNSC_to_retrans  = nf->m_leafn_storage_to_SNSC - cf->m_leafc_storage_to_SNSC/STDB_CN;
			
			nf->SNSC_to_retrans             += nf->leafSNSC_to_retrans + nf->leaf_transferSNSC_to_retrans + nf->leaf_storageSNSC_to_retrans;
			
		}

		if (epc->froot_cn)
		{
			STDB_CN = epc->froot_cn * epc->leaflitr_cn/epc->leaf_cn;
			if (STDB_CN <= 0) errorCode=1;

		/*	cf->m_frootc_to_SNSC			= SNSCmort_belowbiom * cs->frootc;	 
			cf->m_frootc_storage_to_SNSC	= SNSCmort_nsc * cs->frootc_storage;	
			cf->m_frootc_transfer_to_SNSC	= SNSCmort_nsc * cs->frootc_transfer;
	
			nf->m_frootn_to_SNSC			= cf->m_frootc_to_SNSC / epc->froot_cn;
			nf->m_frootn_storage_to_SNSC	= cf->m_frootc_storage_to_SNSC / epc->froot_cn;
			nf->m_frootn_transfer_to_SNSC	= cf->m_frootc_transfer_to_SNSC / epc->froot_cn;

			nf->frootSNSC_to_retrans          = nf->m_frootn_to_SNSC - cf->m_frootc_to_SNSC/STDB_CN;
			nf->froot_transferSNSC_to_retrans = nf->m_frootn_transfer_to_SNSC - cf->m_frootc_transfer_to_SNSC/STDB_CN;
			nf->froot_storageSNSC_to_retrans  = nf->m_frootn_storage_to_SNSC - cf->m_frootc_storage_to_SNSC/STDB_CN;	*/

			nf->SNSC_to_retrans             += nf->frootSNSC_to_retrans + nf->froot_transferSNSC_to_retrans + nf->froot_storageSNSC_to_retrans;

		}
	
		if (epc->softstem_cn)
		{
			STDB_CN = epc->softstem_cn * epc->leaflitr_cn/epc->leaf_cn;
			if (STDB_CN <= 0) errorCode=1;

		/*	cf->m_softstemc_to_SNSC			= SNSCmort_abovebiom * cs->softstemc;	 
			cf->m_softstemc_storage_to_SNSC	= SNSCmort_nsc * cs->softstemc_storage;	
			cf->m_softstemc_transfer_to_SNSC= SNSCmort_nsc * cs->softstemc_transfer;

			nf->m_softstemn_to_SNSC			= cf->m_softstemc_to_SNSC / epc->softstem_cn;
			nf->m_softstemn_storage_to_SNSC	= cf->m_softstemc_storage_to_SNSC / epc->softstem_cn;	
			nf->m_softstemn_transfer_to_SNSC= cf->m_softstemc_transfer_to_SNSC / epc->softstem_cn;

			nf->softstemSNSC_to_retrans          = nf->m_softstemn_to_SNSC - cf->m_softstemc_to_SNSC/STDB_CN;
			nf->softstem_transferSNSC_to_retrans = nf->m_softstemn_transfer_to_SNSC - cf->m_softstemc_transfer_to_SNSC/STDB_CN;
			nf->softstem_storageSNSC_to_retrans  = nf->m_softstemn_storage_to_SNSC - cf->m_softstemc_storage_to_SNSC/STDB_CN;	*/

			nf->SNSC_to_retrans             += nf->softstemSNSC_to_retrans + nf->softstem_transferSNSC_to_retrans + nf->softstem_storageSNSC_to_retrans;
		}

	
		cf->m_fruitc_to_SNSC			= 0; 
		cf->m_fruitc_storage_to_SNSC	= 0;	
		cf->m_fruitc_transfer_to_SNSC	= 0;
	
		nf->m_fruitn_to_SNSC			= 0; 
		nf->m_fruitn_storage_to_SNSC	= 0;	
		nf->m_fruitn_transfer_to_SNSC	= 0;

		cf->m_gresp_storage_to_SNSC		= SNSCmort_nsc * cs->gresp_storage;
		cf->m_gresp_transfer_to_SNSC	= SNSCmort_nsc * cs->gresp_transfer;

		nf->m_retransn_to_SNSC			= SNSCmort_nsc * ns->retransn;	
	}

	/****************************************************************************************/
	/* 4. update state variables - decreasing state variables */
	
	if (SNSCmort_belowbiom > 0 || SNSCmort_belowbiom > 0 || SNSCmort_nsc > 0)
	{
		cs->leafc			   -= cf->m_leafc_to_SNSC;
		cs->leafc_storage	   -= cf->m_leafc_storage_to_SNSC;
		cs->leafc_transfer	   -= cf->m_leafc_transfer_to_SNSC;
	
		cs->frootc			   -= cf->m_frootc_to_SNSC;
		cs->frootc_storage	   -= cf->m_frootc_storage_to_SNSC; 
		cs->frootc_transfer    -= cf->m_frootc_transfer_to_SNSC; 
	
		cs->fruitc			   -= cf->m_fruitc_to_SNSC; 
		cs->fruitc_storage	   -= cf->m_fruitc_storage_to_SNSC; 
		cs->fruitc_transfer    -= cf->m_fruitc_transfer_to_SNSC; 

		cs->softstemc		   -= cf->m_softstemc_to_SNSC; 
		cs->softstemc_storage  -= cf->m_softstemc_storage_to_SNSC; 
		cs->softstemc_transfer -= cf->m_softstemc_transfer_to_SNSC; 

		cs->gresp_storage	   -= cf->m_gresp_storage_to_SNSC;
		cs->gresp_transfer	   -= cf->m_gresp_transfer_to_SNSC;

		ns->leafn				-= nf->m_leafn_to_SNSC;
		ns->leafn_storage		-= nf->m_leafn_storage_to_SNSC;
		ns->leafn_transfer		-= nf->m_leafn_transfer_to_SNSC;

		ns->frootn				-= nf->m_frootn_to_SNSC;
		ns->frootn_storage		-= nf->m_frootn_storage_to_SNSC;
		ns->frootn_transfer     -= nf->m_frootn_transfer_to_SNSC;

		ns->fruitn				-= nf->m_fruitn_to_SNSC;
		ns->fruitn_storage		-= nf->m_fruitn_storage_to_SNSC;
		ns->fruitn_transfer     -= nf->m_fruitn_transfer_to_SNSC;

		ns->softstemn			-= nf->m_softstemn_to_SNSC;
		ns->softstemn_storage   -= nf->m_softstemn_storage_to_SNSC;
		ns->softstemn_transfer  -= nf->m_softstemn_transfer_to_SNSC;

		ns->retransn			-= nf->m_retransn_to_SNSC;	

	}


	/****************************************************************************************/
	/* 5. update state variables with genetically programmed leaf senescence */

	if (cf->m_leafc_to_SNSCgenprog != 0)
	{
		if (cf->m_leafc_to_SNSCgenprog > cs->leafc || nf->m_leafn_to_SNSCgenprog > ns->leafn)
		{
			cf->m_leafc_to_SNSCgenprog = cs->leafc;
			nf->m_leafn_to_SNSCgenprog = ns->leafn;
			nf->leafSNSCgenprog_to_retrans = nf->m_leafn_to_SNSCgenprog - cf->m_leafc_to_SNSCgenprog/epc->leaflitr_cn;

			cs->leafc = 0;
			ns->leafn = 0;

			/* limitSNSC_flag: flag of WARNING writing in log file (only at first time) */
			if (!ctrl->limitSNSC_flag) ctrl->limitSNSC_flag = 1;
		}
		else
		{
			cs->leafc           -= cf->m_leafc_to_SNSCgenprog; 
			ns->leafn           -= nf->m_leafn_to_SNSCgenprog; 
		}

	
		nf->SNSC_to_retrans += nf->leafSNSCgenprog_to_retrans;

		/* control */
		if ((ns->leafn == 0 && cs->leafc !=0) || (ns->leafn != 0 && cs->leafc ==0))
		{
			printf("ERROR: in genetically programmed leaf senescence calculation in senescence.c\n");
			errorCode=1;
		}
	}
	/****************************************************************************************/
	/* 6. plant material losses because of senescence */
	
	cf->m_vegc_to_SNSC  += cf->m_leafc_to_SNSC     + cf->m_leafc_storage_to_SNSC     + cf->m_leafc_transfer_to_SNSC  +   cf->m_leafc_to_SNSCgenprog + 
						   cf->m_frootc_to_SNSC    + cf->m_frootc_storage_to_SNSC    + cf->m_frootc_transfer_to_SNSC +  
						   cf->m_fruitc_to_SNSC    + cf->m_fruitc_storage_to_SNSC    + cf->m_fruitc_transfer_to_SNSC +
						   cf->m_softstemc_to_SNSC + cf->m_softstemc_storage_to_SNSC + cf->m_softstemc_transfer_to_SNSC + 
						   cf->m_gresp_storage_to_SNSC  + cf->m_gresp_transfer_to_SNSC;
	cs->SNSCsnk_C       += cf->m_vegc_to_SNSC; 

	nf->m_vegn_to_SNSC += nf->m_leafn_to_SNSC     + nf->m_leafn_storage_to_SNSC     + nf->m_leafn_transfer_to_SNSC     + nf->m_leafn_to_SNSCgenprog + 
		                  nf->m_frootn_to_SNSC    + nf->m_frootn_storage_to_SNSC    + nf->m_frootn_transfer_to_SNSC    +
						  nf->m_fruitn_to_SNSC    + nf->m_fruitn_storage_to_SNSC    + nf->m_fruitn_transfer_to_SNSC    +  
						  nf->m_softstemn_to_SNSC + nf->m_softstemn_storage_to_SNSC + nf->m_softstemn_transfer_to_SNSC +
					      nf->m_retransn_to_SNSC;

	ns->SNSCsnk_N       += nf->m_vegn_to_SNSC;

    /****************************************************************************************/
	/* 7. update litter variables - increasing litter storage state variables AND retranslocation pool
	MULTILAYER SOIL: separate above- and belowground litr1 */
	

	cs->STDBc_leaf     += cf->m_leafc_to_SNSC + cf->m_leafc_to_SNSCgenprog; 											
	cs->STDBc_froot    += cf->m_frootc_to_SNSC;
	cs->STDBc_fruit    += cf->m_fruitc_to_SNSC; 
	cs->STDBc_softstem += cf->m_softstemc_to_SNSC; 
	cs->STDBc_nsc      += cf->m_leafc_storage_to_SNSC  + cf->m_leafc_transfer_to_SNSC  + cf->m_frootc_storage_to_SNSC    + cf->m_frootc_transfer_to_SNSC +
		                  cf->m_fruitc_storage_to_SNSC + cf->m_fruitc_transfer_to_SNSC + cf->m_softstemc_storage_to_SNSC + cf->m_softstemc_transfer_to_SNSC +
				          cf->m_gresp_storage_to_SNSC + cf->m_gresp_transfer_to_SNSC;

	ns->STDBn_leaf     += nf->m_leafn_to_SNSC + nf->m_leafn_to_SNSCgenprog - (nf->leafSNSCgenprog_to_retrans + nf->leafSNSC_to_retrans); 											
	ns->STDBn_froot    += nf->m_frootn_to_SNSC - nf->frootSNSC_to_retrans;
	ns->STDBn_fruit    += nf->m_fruitn_to_SNSC - nf->fruitSNSC_to_retrans; 
	ns->STDBn_softstem += nf->m_softstemn_to_SNSC  - nf->softstemSNSC_to_retrans; 
	ns->STDBn_nsc      += nf->m_leafn_storage_to_SNSC  + nf->m_leafn_transfer_to_SNSC  + nf->m_frootn_storage_to_SNSC    + nf->m_frootn_transfer_to_SNSC +
		                  nf->m_fruitn_storage_to_SNSC + nf->m_fruitn_transfer_to_SNSC + nf->m_softstemn_storage_to_SNSC + nf->m_softstemn_transfer_to_SNSC +
				          nf->m_retransn_to_SNSC - 
					     (nf->leaf_transferSNSC_to_retrans + nf->leaf_storageSNSC_to_retrans + nf->froot_transferSNSC_to_retrans + nf->froot_storageSNSC_to_retrans +
						  nf->fruit_transferSNSC_to_retrans + nf->fruit_storageSNSC_to_retrans + nf->softstem_transferSNSC_to_retrans + nf->softstem_storageSNSC_to_retrans);

	ns->retransn            += nf->SNSC_to_retrans;
	 /****************************************************************************************/
	/* 8. mortality fluxes into litter pools */


	cf->STDBc_leaf_to_litr     = cs->STDBc_leaf     * mort_SNSC_to_litter;
	cf->STDBc_froot_to_litr    = cs->STDBc_froot    * mort_SNSC_to_litter;                                                         
	cf->STDBc_fruit_to_litr    = cs->STDBc_fruit    * mort_SNSC_to_litter;
	cf->STDBc_softstem_to_litr = cs->STDBc_softstem * mort_SNSC_to_litter;
	cf->STDBc_nsc_to_litr      = cs->STDBc_nsc      * mort_SNSC_to_litter;

	nf->STDBn_leaf_to_litr     = ns->STDBn_leaf     * mort_SNSC_to_litter;
	nf->STDBn_froot_to_litr    = ns->STDBn_froot    * mort_SNSC_to_litter;                                                         
	nf->STDBn_fruit_to_litr    = ns->STDBn_fruit    * mort_SNSC_to_litter;
	nf->STDBn_softstem_to_litr = ns->STDBn_softstem * mort_SNSC_to_litter;
	nf->STDBn_nsc_to_litr      = ns->STDBn_nsc      * mort_SNSC_to_litter;

	cf->STDBc_to_litr = cf->STDBc_leaf_to_litr + cf->STDBc_froot_to_litr + cf->STDBc_fruit_to_litr + cf->STDBc_softstem_to_litr + cf->STDBc_nsc_to_litr;

	nf->STDBn_to_litr = nf->STDBn_leaf_to_litr + nf->STDBn_froot_to_litr + nf->STDBn_fruit_to_litr + nf->STDBn_softstem_to_litr + nf->STDBn_nsc_to_litr;

	/****************************************************************************************/
	/* 9. mortality fluxes turn into litter pools: 	aboveground biomass into the top soil layer, belowground biomass divided between soil layers based on their root content */
	
	/* new feature: litter turns into the first AND the second soil layer */
	propLAYER0 = sitec->soillayer_thickness[0]/sitec->soillayer_depth[2];
	propLAYER1 = sitec->soillayer_thickness[1]/sitec->soillayer_depth[2];
	propLAYER2 = sitec->soillayer_thickness[2]/sitec->soillayer_depth[2];



	/* 9.1 aboveground biomass into the top soil layer */

	cs->litr1c[0] += (cf->STDBc_leaf_to_litr * epc->leaflitr_flab  + cf->STDBc_fruit_to_litr * epc->fruitlitr_flab  + 
		              cf->STDBc_softstem_to_litr * epc->softstemlitr_flab + cf->STDBc_nsc_to_litr) * propLAYER0;
	cs->litr2c[0] += (cf->STDBc_leaf_to_litr * epc->leaflitr_fucel + cf->STDBc_fruit_to_litr * epc->fruitlitr_fucel + 
		              cf->STDBc_softstem_to_litr * epc->softstemlitr_fucel) * propLAYER0;
	cs->litr3c[0] += (cf->STDBc_leaf_to_litr * epc->leaflitr_fscel + cf->STDBc_fruit_to_litr * epc->fruitlitr_fscel + 
		              cf->STDBc_softstem_to_litr * epc->softstemlitr_fscel) * propLAYER0;
	cs->litr4c[0] += (cf->STDBc_leaf_to_litr * epc->leaflitr_flig  + cf->STDBc_fruit_to_litr * epc->fruitlitr_flig  + 
		              cf->STDBc_softstem_to_litr * epc->softstemlitr_flig) * propLAYER0;

	ns->litr1n[0] += (nf->STDBn_leaf_to_litr * epc->leaflitr_flab  + nf->STDBn_fruit_to_litr * epc->fruitlitr_flab  + 
		              nf->STDBn_softstem_to_litr * epc->softstemlitr_flab + nf->STDBn_nsc_to_litr) * propLAYER0;
	ns->litr2n[0] += (nf->STDBn_leaf_to_litr * epc->leaflitr_fucel + nf->STDBn_fruit_to_litr * epc->fruitlitr_fucel + 
		              nf->STDBn_softstem_to_litr * epc->softstemlitr_fucel) * propLAYER0;
	ns->litr3n[0] += (nf->STDBn_leaf_to_litr * epc->leaflitr_fscel + nf->STDBn_fruit_to_litr * epc->fruitlitr_fscel + 
		              nf->STDBn_softstem_to_litr * epc->softstemlitr_fscel) * propLAYER0;
	ns->litr4n[0] += (nf->STDBn_leaf_to_litr * epc->leaflitr_flig  + nf->STDBn_fruit_to_litr * epc->fruitlitr_flig  + 
		              nf->STDBn_softstem_to_litr * epc->softstemlitr_flig) * propLAYER0;

	cs->litr1c[1] += (cf->STDBc_leaf_to_litr * epc->leaflitr_flab  + cf->STDBc_fruit_to_litr * epc->fruitlitr_flab  + 
		              cf->STDBc_softstem_to_litr * epc->softstemlitr_flab + cf->STDBc_nsc_to_litr) * propLAYER1;
	cs->litr2c[1] += (cf->STDBc_leaf_to_litr * epc->leaflitr_fucel + cf->STDBc_fruit_to_litr * epc->fruitlitr_fucel + 
		              cf->STDBc_softstem_to_litr * epc->softstemlitr_fucel) * propLAYER1;
	cs->litr3c[1] += (cf->STDBc_leaf_to_litr * epc->leaflitr_fscel + cf->STDBc_fruit_to_litr * epc->fruitlitr_fscel + 
		              cf->STDBc_softstem_to_litr * epc->softstemlitr_fscel) * propLAYER1;
	cs->litr4c[1] += (cf->STDBc_leaf_to_litr * epc->leaflitr_flig  + cf->STDBc_fruit_to_litr * epc->fruitlitr_flig  + 
		              cf->STDBc_softstem_to_litr * epc->softstemlitr_flig) * propLAYER1;

	ns->litr1n[1] += (nf->STDBn_leaf_to_litr * epc->leaflitr_flab  + nf->STDBn_fruit_to_litr * epc->fruitlitr_flab  + 
		              nf->STDBn_softstem_to_litr * epc->softstemlitr_flab + nf->STDBn_nsc_to_litr) * propLAYER1;
	ns->litr2n[1] += (nf->STDBn_leaf_to_litr * epc->leaflitr_fucel + nf->STDBn_fruit_to_litr * epc->fruitlitr_fucel + 
		              nf->STDBn_softstem_to_litr * epc->softstemlitr_fucel) * propLAYER1;
	ns->litr3n[1] += (nf->STDBn_leaf_to_litr * epc->leaflitr_fscel + nf->STDBn_fruit_to_litr * epc->fruitlitr_fscel + 
		              nf->STDBn_softstem_to_litr * epc->softstemlitr_fscel) * propLAYER1;
	ns->litr4n[1] += (nf->STDBn_leaf_to_litr * epc->leaflitr_flig  + nf->STDBn_fruit_to_litr * epc->fruitlitr_flig  + 
		              nf->STDBn_softstem_to_litr * epc->softstemlitr_flig) * propLAYER1;
	
	cs->litr1c[2] += (cf->STDBc_leaf_to_litr * epc->leaflitr_flab  + cf->STDBc_fruit_to_litr * epc->fruitlitr_flab  + 
		              cf->STDBc_softstem_to_litr * epc->softstemlitr_flab + cf->STDBc_nsc_to_litr) * propLAYER2;
	cs->litr2c[2] += (cf->STDBc_leaf_to_litr * epc->leaflitr_fucel + cf->STDBc_fruit_to_litr * epc->fruitlitr_fucel + 
		              cf->STDBc_softstem_to_litr * epc->softstemlitr_fucel) * propLAYER2;
	cs->litr3c[2] += (cf->STDBc_leaf_to_litr * epc->leaflitr_fscel + cf->STDBc_fruit_to_litr * epc->fruitlitr_fscel + 
		              cf->STDBc_softstem_to_litr * epc->softstemlitr_fscel) * propLAYER2;
	cs->litr4c[2] += (cf->STDBc_leaf_to_litr * epc->leaflitr_flig  + cf->STDBc_fruit_to_litr * epc->fruitlitr_flig  + 
		              cf->STDBc_softstem_to_litr * epc->softstemlitr_flig) * propLAYER2;

	ns->litr1n[2] += (nf->STDBn_leaf_to_litr * epc->leaflitr_flab  + nf->STDBn_fruit_to_litr * epc->fruitlitr_flab  + 
		              nf->STDBn_softstem_to_litr * epc->softstemlitr_flab + nf->STDBn_nsc_to_litr) * propLAYER2;
	ns->litr2n[2] += (nf->STDBn_leaf_to_litr * epc->leaflitr_fucel + nf->STDBn_fruit_to_litr * epc->fruitlitr_fucel + 
		              nf->STDBn_softstem_to_litr * epc->softstemlitr_fucel) * propLAYER2;
	ns->litr3n[2] += (nf->STDBn_leaf_to_litr * epc->leaflitr_fscel + nf->STDBn_fruit_to_litr * epc->fruitlitr_fscel + 
		              nf->STDBn_softstem_to_litr * epc->softstemlitr_fscel) * propLAYER2;
	ns->litr4n[2] += (nf->STDBn_leaf_to_litr * epc->leaflitr_flig  + nf->STDBn_fruit_to_litr * epc->fruitlitr_flig  + 
		              nf->STDBn_softstem_to_litr * epc->softstemlitr_flig) * propLAYER2;

	/* 9.2 	belowground biomass divided between soil layers based on their root content */ 
	
	if (epv->rootdepth > CRIT_PREC)
	{
		for (layer=0; layer < N_SOILLAYERS; layer++)
		{
			cs->litr1c[layer]  += cf->STDBc_froot_to_litr * epc->frootlitr_flab  * epv->rootlength_prop[layer];
			cs->litr2c[layer]  += cf->STDBc_froot_to_litr * epc->frootlitr_fucel * epv->rootlength_prop[layer];
			cs->litr3c[layer]  += cf->STDBc_froot_to_litr * epc->frootlitr_fscel * epv->rootlength_prop[layer];
			cs->litr4c[layer]  += cf->STDBc_froot_to_litr * epc->frootlitr_flig  * epv->rootlength_prop[layer];

			ns->litr1n[layer]  += nf->STDBn_froot_to_litr * epc->frootlitr_flab  * epv->rootlength_prop[layer];
			ns->litr2n[layer]  += nf->STDBn_froot_to_litr * epc->frootlitr_fucel * epv->rootlength_prop[layer];
			ns->litr3n[layer]  += nf->STDBn_froot_to_litr * epc->frootlitr_fscel * epv->rootlength_prop[layer];
			ns->litr4n[layer]  += nf->STDBn_froot_to_litr * epc->frootlitr_flig  * epv->rootlength_prop[layer];
		}
	}
	else
	{
		for (layer=0; layer < N_SOILLAYERS; layer++)
		{
			cs->litr1c[layer]  += cf->STDBc_froot_to_litr * epc->frootlitr_flab  * epv->rootlengthLandD_prop[layer];
			cs->litr2c[layer]  += cf->STDBc_froot_to_litr * epc->frootlitr_fucel * epv->rootlengthLandD_prop[layer];
			cs->litr3c[layer]  += cf->STDBc_froot_to_litr * epc->frootlitr_fscel * epv->rootlengthLandD_prop[layer];
			cs->litr4c[layer]  += cf->STDBc_froot_to_litr * epc->frootlitr_flig  * epv->rootlengthLandD_prop[layer];

			ns->litr1n[layer]  += nf->STDBn_froot_to_litr * epc->frootlitr_flab  * epv->rootlengthLandD_prop[layer];
			ns->litr2n[layer]  += nf->STDBn_froot_to_litr * epc->frootlitr_fucel * epv->rootlengthLandD_prop[layer];
			ns->litr3n[layer]  += nf->STDBn_froot_to_litr * epc->frootlitr_fscel * epv->rootlengthLandD_prop[layer];
			ns->litr4n[layer]  += nf->STDBn_froot_to_litr * epc->frootlitr_flig  * epv->rootlengthLandD_prop[layer];
		}
	}
	
	/* estimating aboveground litter and cwdc */
	cs->litrc_above +=  cf->STDBc_leaf_to_litr + cf->STDBc_fruit_to_litr + cf->STDBc_softstem_to_litr + cf->STDBc_nsc_to_litr;

	/****************************************************************************************/
	/* 10. decreasing of temporary pool */
	
	cs->STDBc_leaf     -= cf->STDBc_leaf_to_litr;
	cs->STDBc_froot    -= cf->STDBc_froot_to_litr;
	cs->STDBc_fruit    -= cf->STDBc_fruit_to_litr;
	cs->STDBc_softstem -= cf->STDBc_softstem_to_litr;
	cs->STDBc_nsc      -= cf->STDBc_nsc_to_litr;

	ns->STDBn_leaf     -= ns->STDBn_leaf     * mort_SNSC_to_litter;
	ns->STDBn_froot    -= ns->STDBn_froot    * mort_SNSC_to_litter;
	ns->STDBn_fruit    -= ns->STDBn_fruit    * mort_SNSC_to_litter;
	ns->STDBn_softstem -= ns->STDBn_softstem * mort_SNSC_to_litter;
	ns->STDBn_nsc      -= ns->STDBn_nsc      * mort_SNSC_to_litter;


	/************************************************************/
	/* 11. precision control */

   if ((cs->STDBc_leaf != 0 && fabs(cs->STDBc_leaf) < CRIT_PREC) || (ns->STDBn_leaf != 0 && fabs(ns->STDBn_leaf) < CRIT_PREC))
	{
		cs->FIREsnk_C += cs->STDBc_leaf;
		ns->FIREsnk_N += ns->STDBn_leaf; 
		cs->STDBc_leaf = 0;
		ns->STDBn_leaf = 0;
	}

	if ((cs->STDBc_fruit != 0 && fabs(cs->STDBc_fruit) < CRIT_PREC) || (ns->STDBn_fruit != 0 && fabs(ns->STDBn_fruit) < CRIT_PREC))
	{
		cs->FIREsnk_C += cs->STDBc_fruit;
		ns->FIREsnk_N += ns->STDBn_fruit; 
		cs->STDBc_fruit = 0;
		ns->STDBn_fruit = 0;
	}

	if ((cs->STDBc_softstem != 0 && fabs(cs->STDBc_softstem) < CRIT_PREC) || (ns->STDBn_softstem != 0 && fabs(ns->STDBn_softstem) < CRIT_PREC))
	{
		cs->FIREsnk_C += cs->STDBc_softstem;
		ns->FIREsnk_N += ns->STDBn_softstem; 
		cs->STDBc_softstem = 0;
		ns->STDBn_softstem = 0;
	}

	if ((cs->STDBc_froot != 0 && fabs(cs->STDBc_froot) < CRIT_PREC) || (ns->STDBn_froot != 0 && fabs(ns->STDBn_froot) < CRIT_PREC))
	{
		cs->FIREsnk_C += cs->STDBc_froot;
		ns->FIREsnk_N += ns->STDBn_froot; 
		cs->STDBc_froot = 0;
		ns->STDBn_froot = 0;
	}

	if ((cs->STDBc_nsc != 0 && fabs(cs->STDBc_nsc) < CRIT_PREC) || (ns->STDBn_nsc != 0 && fabs(ns->STDBn_nsc) < CRIT_PREC))
	{
		cs->FIREsnk_C += cs->STDBc_nsc;
		ns->FIREsnk_N += ns->STDBn_nsc; 
		cs->STDBc_nsc = 0;
		ns->STDBn_nsc = 0;
	}
	
	
	return (errorCode);
}

int genprog_senescence(const epconst_struct* epc, const metvar_struct* metv, epvar_struct* epv, cflux_struct* cf, nflux_struct* nf)
{

	int vd, leafday;
	double plantlifetime, m_leafc_to_SNSCgenprog, m_leafn_to_SNSCgenprog;
	int errorCode = 0;

	plantlifetime = m_leafc_to_SNSCgenprog =  m_leafn_to_SNSCgenprog= 0;

	leafday  = (int) epv->leafday;
	
	
	/* phenological phases: 1: seedling->germination, 2: germination->emergence, 3: leaf increasing */
	if (epv->n_actphen >= epc->n_emerg_phenophase && epv->n_actphen > 0)		
	{
		leafday += 1;

		if (metv->tavg - epc->base_temp > 0)
			epv->thermal_time = (metv->tavg - epc->base_temp);
		else
			epv->thermal_time = 0;

		epv->cpool_to_leafcARRAY[leafday] = cf->cpool_to_leafc;
		epv->npool_to_leafnARRAY[leafday] = nf->npool_to_leafn;
		epv->gpSNSC_phenARRAY[leafday]    = (int) epv->n_actphen;
	

		for (vd=0; vd < leafday; vd++)
		{
			plantlifetime = epc->max_plantlifetime[epv->gpSNSC_phenARRAY[vd]-1];
			epv->thermal_timeARRAY[vd] += epv->thermal_time;
			if (epv->thermal_timeARRAY[vd] > plantlifetime)
			{
				m_leafc_to_SNSCgenprog += epv->cpool_to_leafcARRAY[vd];
				m_leafn_to_SNSCgenprog += epv->npool_to_leafnARRAY[vd];
				epv->cpool_to_leafcARRAY[vd] = 0;
				epv->npool_to_leafnARRAY[vd] = 0;

				if (vd > epv->leafday_lastmort) epv->leafday_lastmort = vd;
			}
		}


		if (leafday >= nDAYS_OF_YEAR*2)
		{
			printf("\n");
			printf("ERROR in leafday calculation() in senescence.c: vegetation period must less than 730 ydays()\n");
			errorCode=1;
			
		}

	}	
	else
	{
		leafday = -1;
		epv->thermal_time = 0;

		for (vd=0 ; vd < 2*nDAYS_OF_YEAR; vd++)
		{
			epv->cpool_to_leafcARRAY[vd] = 0;
			epv->npool_to_leafnARRAY[vd] = 0;
			epv->gpSNSC_phenARRAY[vd] = 0;
			epv->thermal_timeARRAY[vd] = 0;
		}
	}

	cf->m_leafc_to_SNSCgenprog = m_leafc_to_SNSCgenprog;
	nf->m_leafn_to_SNSCgenprog = m_leafn_to_SNSCgenprog;
	nf->leafSNSCgenprog_to_retrans = nf->m_leafn_to_SNSCgenprog - m_leafc_to_SNSCgenprog/epc->leaflitr_cn;



	epv->leafday = (double) leafday;
	

	

	return (errorCode);
}


