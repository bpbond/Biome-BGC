/* 
harvesting.c
do harvesting  - decrease the plant material (leafc, leafn, canopy water)

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0
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

int harvesting(const control_struct* ctrl, const epconst_struct* epc, const harvesting_struct* HRV, 
	           epvar_struct* epv, cstate_struct* cs, nstate_struct* ns, wstate_struct* ws, cflux_struct* cf, nflux_struct* nf, wflux_struct* wf)
{

	/* harvesting parameters */			                                    /* remained proportion of plant  */
	double snag, befharv_stem;						                        /* value of LAI before harvesting */
	double HRVcoeff_leaf, HRVcoeff_fruit, HRVcoeff_softstem, remained_prop;	/* decrease of plant material caused by harvest: difference between plant material before and after harvesting */
	double outc, outn, inc, inn, HRV_to_transpC, HRV_to_transpN;
	
	/* local parameters */
	int ok = 1;
	int ny;
	
	int mgmd = HRV->mgmd;
	

	/* yearly varied or constant management parameters */
	if(HRV->HRV_flag == 2)
		ny = ctrl->simyr;
	else 
		ny=0;


	HRV_to_transpC=HRV_to_transpN=outc=outn=inc=inn=HRVcoeff_leaf=HRVcoeff_fruit=HRVcoeff_softstem=remained_prop=0;
	
	/**********************************************************************************************/
	/* I. CALCULATING HRVcoeff */

	/* harvesting if gapflag=1  */
	if (mgmd >= 0)
	{ 
		/* control */
		if (HRV->transport_coeff_array[mgmd][ny] == DATA_GAP      || HRV->snag_array[mgmd][ny] == DATA_GAP)
		{
			printf("ERROR in harvesting parameters in INI or management file (harvesting.c)\n");
			ok=0;
		}

		remained_prop = (100 - HRV->transport_coeff_array[mgmd][ny])/100.; /* remained prop. of plant mat.is calculated from transport coeff. */
		snag = HRV->snag_array[mgmd][ny];
		epv->rooting_depth = CRIT_PREC;

		
	
		/* if before harvesting the value of the LAI is less than snag (limit value) - > no harvesting
	   if harvest: plant material decreases as the rate of "harvest effect", which is th ratio of LAI before harvest and LAI snag */
			
		befharv_stem = cs->softstemc;
		if (befharv_stem > snag && befharv_stem > 0)
		{	
			HRVcoeff_leaf  = 1.0;
			HRVcoeff_fruit = 1.0;
			HRVcoeff_softstem = 1. - (snag/befharv_stem);
		
		}
		else
		{
			HRVcoeff_leaf = 1.0;
			HRVcoeff_fruit = 1.0;
			HRVcoeff_softstem = 1.0;
			if (ctrl->onscreen) printf("sofstem carbon content is less than snag set in INI file\n");
		}
	

		/**********************************************************************************************/
		/* II. CALCULATING FLUXES */
	
		/*----------------------------------------------------------*/
		/* 1. OUT: daily loss due to HRV */ 

		/* 1.1. actual and transfer plant pools*/
		if (epc->leaf_cn)
		{
			cf->leafc_to_HRV              = cs->leafc          * HRVcoeff_leaf;
			cf->leafc_storage_to_HRV      = 0; // cs->leafc_storage  * HRVcoeff_leaf;
			cf->leafc_transfer_to_HRV     = 0; // cs->leafc_transfer * HRVcoeff_leaf;

			nf->leafn_to_HRV              = cf->leafc_to_HRV          / epc->leaf_cn;
			nf->leafn_storage_to_HRV      = 0; // cf->leafc_storage_to_HRV  / epc->leaf_cn;
			nf->leafn_transfer_to_HRV     = 0; // cf->leafc_transfer_to_HRV / epc->leaf_cn;
		}

		if (epc->fruit_cn)
		{
			cf->fruitc_to_HRV              = cs->fruitc          * HRVcoeff_fruit;
			cf->fruitc_storage_to_HRV      = 0; // cs->fruitc_storage  * HRVcoeff_fruit;
			cf->fruitc_transfer_to_HRV     = 0; // cs->fruitc_transfer * HRVcoeff_fruit;

			nf->fruitn_to_HRV              = cf->fruitc_to_HRV          / epc->fruit_cn;
			nf->fruitn_storage_to_HRV      = 0; // cf->fruitc_storage_to_HRV  / epc->fruit_cn;
			nf->fruitn_transfer_to_HRV     = 0; // cf->fruitc_transfer_to_HRV / epc->fruit_cn;
		}

		if (epc->softstem_cn)
		{
			cf->softstemc_to_HRV              = cs->softstemc          * HRVcoeff_softstem;
			cf->softstemc_storage_to_HRV      = 0; // cs->softstemc_storage  * HRVcoeff_softstem;
			cf->softstemc_transfer_to_HRV     = 0; // cs->softstemc_transfer * HRVcoeff_softstem;

			nf->softstemn_to_HRV              = cf->softstemc_to_HRV          / epc->softstem_cn;
			nf->softstemn_storage_to_HRV      = 0; // cf->softstemc_storage_to_HRV  / epc->softstem_cn;
			nf->softstemn_transfer_to_HRV     = 0; // cf->softstemc_transfer_to_HRV / epc->softstem_cn;
		}

		cf->gresp_storage_to_HRV      = 0; // cs->gresp_storage  * HRVcoeff_leaf;
		cf->gresp_transfer_to_HRV     = 0; // cs->gresp_transfer * HRVcoeff_leaf;
	
		nf->retransn_to_HRV           = 0; // ns->retransn * HRVcoeff_leaf;

		/* 1.2 standing dead biome */
		cf->STDBc_leaf_to_HRV     = cs->STDBc_leaf     * HRVcoeff_leaf; 
		cf->STDBc_fruit_to_HRV    = cs->STDBc_fruit    * HRVcoeff_fruit; 
		cf->STDBc_softstem_to_HRV = cs->STDBc_softstem * HRVcoeff_softstem;
		cf->STDBc_transfer_to_HRV = cs->STDBc_transfer * HRVcoeff_softstem;
		

		nf->STDBn_leaf_to_HRV     = ns->STDBn_leaf     * HRVcoeff_leaf; 
		nf->STDBn_fruit_to_HRV    = ns->STDBn_fruit    * HRVcoeff_fruit; 
		nf->STDBn_softstem_to_HRV = ns->STDBn_softstem * HRVcoeff_softstem;
		nf->STDBn_transfer_to_HRV = ns->STDBn_transfer * HRVcoeff_softstem;

		
		/* 1.3 Water */
		wf->canopyw_to_HRV   = ws->canopyw * HRVcoeff_leaf;


 		/*----------------------------------------------------------*/
		/* 2. TRANSPORT: part of the plant material is transported (all of the fruit, transp_coeff part of leaf and softstem, but no transfer pools!)*/

		HRV_to_transpC = (cf->leafc_to_HRV  + cf->softstemc_to_HRV + cf->STDBc_leaf_to_HRV + cf->STDBc_softstem_to_HRV) * (1-remained_prop) +
						  cf->fruitc_to_HRV + cf->STDBc_fruit_to_HRV;
		HRV_to_transpN = (nf->leafn_to_HRV  + nf->softstemn_to_HRV + nf->STDBn_leaf_to_HRV + nf->STDBn_softstem_to_HRV) * (1-remained_prop) +
						  nf->fruitn_to_HRV + nf->STDBn_fruit_to_HRV;

		/*----------------------------------------------------------*/
		/* 3. IN: cut-down biomass - the rest remains at the site */

		cf->HRV_to_CTDBc_leaf     = (cf->leafc_to_HRV     + cf->STDBc_leaf_to_HRV) * remained_prop;

		cf->HRV_to_CTDBc_fruit    = 0;

		cf->HRV_to_CTDBc_softstem = (cf->softstemc_to_HRV + cf->STDBc_softstem_to_HRV) * remained_prop;

		cf->HRV_to_CTDBc_transfer = (cf->leafc_transfer_to_HRV     + cf->leafc_storage_to_HRV + 
									 cf->fruitc_transfer_to_HRV    + cf->fruitc_storage_to_HRV + 
									 cf->softstemc_transfer_to_HRV + cf->softstemc_storage_to_HRV + 
									 cf->gresp_storage_to_HRV      + cf->gresp_transfer_to_HRV +
									 cf->STDBc_transfer_to_HRV);


		nf->HRV_to_CTDBn_leaf     = (nf->leafn_to_HRV     + nf->STDBn_leaf_to_HRV)  * remained_prop;

		nf->HRV_to_CTDBn_fruit    = 0;

		nf->HRV_to_CTDBn_softstem = (nf->softstemn_to_HRV + nf->STDBn_softstem_to_HRV) * remained_prop;

		nf->HRV_to_CTDBn_transfer = (nf->leafn_transfer_to_HRV     + nf->leafn_storage_to_HRV + 
									 nf->fruitn_transfer_to_HRV    + nf->fruitn_storage_to_HRV + 
									 nf->softstemn_transfer_to_HRV + nf->softstemn_storage_to_HRV + 
									 nf->retransn_to_HRV +
									 nf->STDBn_transfer_to_HRV);

  


		/**********************************************************************************************/
		/* III. STATE UPDATE */

		/* 1. OUT */
		/* 1.1.leaf, fruit, softstem, gresp */

		cs->leafc				-= cf->leafc_to_HRV;
		cs->leafc_transfer		-= cf->leafc_transfer_to_HRV;
		cs->leafc_storage		-= cf->leafc_storage_to_HRV;
		cs->fruitc				-= cf->fruitc_to_HRV;
		cs->fruitc_transfer		-= cf->fruitc_transfer_to_HRV;
		cs->fruitc_storage		-= cf->fruitc_storage_to_HRV;
		cs->softstemc			-= cf->softstemc_to_HRV;
		cs->softstemc_transfer	-= cf->softstemc_transfer_to_HRV;
		cs->softstemc_storage	-= cf->softstemc_storage_to_HRV;
		cs->gresp_transfer		-= cf->gresp_transfer_to_HRV;
		cs->gresp_storage		-= cf->gresp_storage_to_HRV;

		ns->leafn				-= nf->leafn_to_HRV;
		ns->leafn_transfer		-= nf->leafn_transfer_to_HRV;
		ns->leafn_storage		-= nf->leafn_storage_to_HRV;
		ns->fruitn				-= nf->fruitn_to_HRV;
		ns->fruitn_transfer		-= nf->fruitn_transfer_to_HRV;
		ns->fruitn_storage		-= nf->fruitn_storage_to_HRV;
		ns->softstemn			-= nf->softstemn_to_HRV;
		ns->softstemn_transfer	-= nf->softstemn_transfer_to_HRV;
		ns->softstemn_storage	-= nf->softstemn_storage_to_HRV;
		ns->retransn			-= nf->retransn_to_HRV;

	

		/* 1.2 dead standing biomass */
		cs->STDBc_leaf     -= cf->STDBc_leaf_to_HRV;
		cs->STDBc_fruit    -= cf->STDBc_fruit_to_HRV;
		cs->STDBc_softstem -= cf->STDBc_softstem_to_HRV;
		cs->STDBc_transfer -= cf->STDBc_transfer_to_HRV;

		ns->STDBn_leaf     -= nf->STDBn_leaf_to_HRV;
		ns->STDBn_fruit    -= nf->STDBn_fruit_to_HRV;
		ns->STDBn_softstem -= nf->STDBn_softstem_to_HRV;
		ns->STDBn_transfer -= nf->STDBn_transfer_to_HRV;


		/* 1.3. water */
		ws->canopyw_HRVsnk += wf->canopyw_to_HRV;
		ws->canopyw -= wf->canopyw_to_HRV;

		/* 2. TRANSPORT*/
		cs->HRV_transportC  += HRV_to_transpC;
		ns->HRV_transportN  += HRV_to_transpN;

		/* 3. IN: cut-down biome  */
		cs->CTDBc_leaf     += cf->HRV_to_CTDBc_leaf;
		cs->CTDBc_fruit    += cf->HRV_to_CTDBc_fruit;
		cs->CTDBc_softstem += cf->HRV_to_CTDBc_softstem;
		cs->CTDBc_transfer += cf->HRV_to_CTDBc_transfer;

		ns->CTDBn_leaf     += nf->HRV_to_CTDBn_leaf;
		ns->CTDBn_fruit    += nf->HRV_to_CTDBn_fruit;
		ns->CTDBn_softstem += nf->HRV_to_CTDBn_softstem;
		ns->CTDBn_transfer += nf->HRV_to_CTDBn_transfer;

		/* 4. after harvest, remaining softstem and froot and transfer pools transferred to standing dead biomass */
		
		if (cs->leafc_storage || ns->leafn_storage)
		{
			cf->HRV_leafc_storage_to_SNSC      = cs->leafc_storage;
			cs->leafc_storage                 -= cf->HRV_leafc_storage_to_SNSC;
			cs->STDBc_transfer                += cf->HRV_leafc_storage_to_SNSC;
			cs->SNSCsnk                       += cf->HRV_leafc_storage_to_SNSC;

			nf->HRV_leafn_storage_to_SNSC      = ns->leafn_storage;
			ns->leafn_storage                 -= nf->HRV_leafn_storage_to_SNSC;
			ns->STDBn_transfer                += nf->HRV_leafn_storage_to_SNSC;
			ns->SNSCsnk                       += nf->HRV_leafn_storage_to_SNSC;
		}

		if (cs->fruitc_transfer || ns->fruitn_transfer)
		{
			cf->HRV_fruitc_transfer_to_SNSC    = cs->fruitc_transfer;
			cs->fruitc_transfer               -= cf->HRV_fruitc_transfer_to_SNSC;
			cs->STDBc_transfer                += cf->HRV_fruitc_transfer_to_SNSC;
			cs->SNSCsnk                       += cf->HRV_fruitc_transfer_to_SNSC;

			nf->HRV_fruitn_transfer_to_SNSC    = ns->fruitn_transfer;
			ns->fruitn_transfer               -= nf->HRV_fruitn_transfer_to_SNSC;
			ns->STDBn_transfer                += nf->HRV_fruitn_transfer_to_SNSC;
			ns->SNSCsnk                       += nf->HRV_fruitn_transfer_to_SNSC;
		}
		
		if (cs->fruitc_storage || ns->fruitn_storage)
		{
			cf->HRV_fruitc_storage_to_SNSC     = cs->fruitc_storage;
			cs->fruitc_storage                -= cf->HRV_fruitc_storage_to_SNSC;
			cs->STDBc_transfer                += cf->HRV_fruitc_storage_to_SNSC;
			cs->SNSCsnk                       += cf->HRV_fruitc_storage_to_SNSC;

			nf->HRV_fruitn_storage_to_SNSC     = ns->fruitn_storage;
			ns->fruitn_storage                -= nf->HRV_fruitn_storage_to_SNSC;
			ns->STDBn_transfer                += nf->HRV_fruitn_storage_to_SNSC;
			ns->SNSCsnk                       += nf->HRV_fruitn_storage_to_SNSC;
		}

		if (cs->softstemc_transfer || ns->softstemn_transfer)
		{
			cf->HRV_softstemc_transfer_to_SNSC	 = cs->softstemc_transfer;
			cs->softstemc_transfer				-= cf->HRV_softstemc_transfer_to_SNSC;
			cs->STDBc_transfer                  += cf->HRV_softstemc_transfer_to_SNSC;
			cs->SNSCsnk                         += cf->HRV_softstemc_transfer_to_SNSC;

			nf->HRV_softstemn_transfer_to_SNSC   = ns->softstemn_transfer;
			ns->softstemn_transfer              -= nf->HRV_softstemn_transfer_to_SNSC;
			ns->STDBn_transfer                  += nf->HRV_softstemn_transfer_to_SNSC;
			ns->SNSCsnk                         += nf->HRV_softstemn_transfer_to_SNSC;
		}
		
		if (cs->softstemc || ns->softstemn)
		{
			cf->HRV_softstemc_to_SNSC  = cs->softstemc;
			cs->softstemc             -= cf->HRV_softstemc_to_SNSC;
			cs->STDBc_softstem        += cf->HRV_softstemc_to_SNSC;
			cs->SNSCsnk               += cf->HRV_softstemc_to_SNSC;

			nf->HRV_softstemn_to_SNSC  = ns->softstemn;
			ns->softstemn             -= nf->HRV_softstemn_to_SNSC;
			ns->STDBn_softstem        += nf->HRV_softstemn_to_SNSC;
			ns->SNSCsnk               += nf->HRV_softstemn_to_SNSC;
		}

		if (cs->softstemc_storage || ns->softstemn_storage)
		{
			cf->HRV_softstemc_storage_to_SNSC  = cs->softstemc_storage;
			cs->softstemc_storage             -= cf->HRV_softstemc_storage_to_SNSC;
			cs->STDBc_transfer                += cf->HRV_softstemc_storage_to_SNSC;
			cs->SNSCsnk                       += cf->HRV_softstemc_storage_to_SNSC;

			nf->HRV_softstemn_storage_to_SNSC  = ns->softstemn_storage;
			ns->softstemn_storage             -= nf->HRV_softstemn_storage_to_SNSC;
			ns->STDBn_transfer                += nf->HRV_softstemn_storage_to_SNSC;
			ns->SNSCsnk                       += nf->HRV_softstemn_storage_to_SNSC;
		}

		if (cs->softstemc_transfer || ns->softstemn_transfer)
		{
			cf->HRV_softstemc_transfer_to_SNSC = cs->softstemc_transfer;
			cs->softstemc_transfer            -= cf->HRV_softstemc_transfer_to_SNSC;
			cs->STDBc_transfer                += cf->HRV_softstemc_transfer_to_SNSC;
			cs->SNSCsnk                       += cf->HRV_softstemc_transfer_to_SNSC;

			nf->HRV_softstemn_transfer_to_SNSC = ns->softstemn_transfer;
			ns->softstemn_transfer            -= nf->HRV_softstemn_transfer_to_SNSC;
			ns->STDBn_transfer                += nf->HRV_softstemn_transfer_to_SNSC;
			ns->SNSCsnk                       += nf->HRV_softstemn_transfer_to_SNSC;
		}

		if (cs->frootc || ns->frootn)
		{
			cf->HRV_frootc_to_SNSC  = cs->frootc;
			cs->frootc             -= cf->HRV_frootc_to_SNSC;
			cs->STDBc_froot        += cf->HRV_frootc_to_SNSC;
			cs->SNSCsnk            += cf->HRV_frootc_to_SNSC;

			nf->HRV_frootn_to_SNSC  = ns->frootn;
			ns->frootn             -= nf->HRV_frootn_to_SNSC;
			ns->STDBn_froot        += nf->HRV_frootn_to_SNSC;
			ns->SNSCsnk            += nf->HRV_frootn_to_SNSC;
		}

		if (cs->frootc_storage || ns->frootn_storage)
		{
			cf->HRV_frootc_storage_to_SNSC  = cs->frootc_storage;
			cs->frootc_storage             -= cf->HRV_frootc_storage_to_SNSC;
			cs->STDBc_transfer             += cf->HRV_frootc_storage_to_SNSC;
			cs->SNSCsnk                    += cf->HRV_frootc_storage_to_SNSC;

			nf->HRV_frootn_storage_to_SNSC  = ns->frootn_storage;
			ns->frootn_storage             -= nf->HRV_frootn_storage_to_SNSC;
			ns->STDBn_transfer             += nf->HRV_frootn_storage_to_SNSC;
			ns->SNSCsnk                    += nf->HRV_frootn_storage_to_SNSC;
		}

		if (cs->frootc_transfer || ns->frootn_transfer)
		{
			cf->HRV_frootc_transfer_to_SNSC  = cs->frootc_transfer;
			cs->frootc_transfer             -= cf->HRV_frootc_transfer_to_SNSC;
			cs->STDBc_transfer              += cf->HRV_frootc_transfer_to_SNSC;
			cs->SNSCsnk                     += cf->HRV_frootc_transfer_to_SNSC;

			nf->HRV_frootn_transfer_to_SNSC  = ns->frootn_transfer;
			ns->frootn_transfer             -= nf->HRV_frootn_transfer_to_SNSC;
			ns->STDBn_transfer              += nf->HRV_frootn_transfer_to_SNSC;
			ns->SNSCsnk                     += nf->HRV_frootn_transfer_to_SNSC;
		}

		if (cs->gresp_storage)
		{
			cf->HRV_gresp_storage_to_SNSC  = cs->gresp_storage;
			cs->gresp_storage             -= cf->HRV_gresp_storage_to_SNSC;
			cs->STDBc_transfer            += cf->HRV_gresp_storage_to_SNSC;
			cs->SNSCsnk                   += cf->HRV_gresp_storage_to_SNSC;
		}

		if (cs->gresp_transfer)
		{
			cf->HRV_gresp_transfer_to_SNSC  = cs->gresp_transfer;
			cs->gresp_transfer             -= cf->HRV_gresp_transfer_to_SNSC;
			cs->STDBc_transfer			   += cf->HRV_gresp_transfer_to_SNSC;
			cs->SNSCsnk					   += cf->HRV_gresp_transfer_to_SNSC;
		}

		if (ns->retransn)
		{
			nf->HRV_retransn_to_SNSC        = ns->retransn;
			ns->retransn                   -= nf->HRV_retransn_to_SNSC;
			ns->STDBn_transfer			   += nf->HRV_retransn_to_SNSC;
			ns->SNSCsnk					   += nf->HRV_retransn_to_SNSC;
		}



		/**********************************************************************************************/
		/* IV. CONTROL */

		outc = cf->leafc_to_HRV         + cf->leafc_transfer_to_HRV     + cf->leafc_storage_to_HRV +
			   cf->fruitc_to_HRV        + cf->fruitc_transfer_to_HRV    + cf->fruitc_storage_to_HRV +
			   cf->softstemc_to_HRV     + cf->softstemc_transfer_to_HRV + cf->softstemc_storage_to_HRV +
			   cf->gresp_storage_to_HRV + cf->gresp_transfer_to_HRV + 
			   cf->STDBc_leaf_to_HRV + cf->STDBc_fruit_to_HRV + cf->STDBc_softstem_to_HRV + cf->STDBc_transfer_to_HRV;


		outn = nf->leafn_to_HRV         + nf->leafn_transfer_to_HRV     + nf->leafn_storage_to_HRV +
			   nf->fruitn_to_HRV        + nf->fruitn_transfer_to_HRV    + nf->fruitn_storage_to_HRV +
			   nf->softstemn_to_HRV     + nf->softstemn_transfer_to_HRV + nf->softstemn_storage_to_HRV +
			   nf->retransn_to_HRV + 
			   nf->STDBn_leaf_to_HRV + nf->STDBn_fruit_to_HRV + nf->STDBn_softstem_to_HRV + nf->STDBn_transfer_to_HRV;

		inc = cf->HRV_to_CTDBc_leaf + cf->HRV_to_CTDBc_fruit  + cf->HRV_to_CTDBc_softstem + cf->HRV_to_CTDBc_transfer;

		inn = nf->HRV_to_CTDBn_leaf + nf->HRV_to_CTDBn_fruit  + nf->HRV_to_CTDBn_softstem + nf->HRV_to_CTDBn_transfer;


		if (fabs(inc + HRV_to_transpC - outc) > CRIT_PREC || fabs(inn + HRV_to_transpN - outn) > CRIT_PREC )
		{
 			printf("BALANCE ERROR in harvest calculation in harvesting.c\n");
			ok=0;
		}	

	}

	

   return (!ok);
}
	