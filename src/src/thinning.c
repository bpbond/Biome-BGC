 /* 
thinning.c
do thinning  - decrease the plant material (leafc, leafn, canopy water, frootc, frootn, stemc, stemn, crootc, crootn)

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0.
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

int thinning(const control_struct* ctrl, const epconst_struct* epc, const thinning_struct* THN,
	        cstate_struct* cs, nstate_struct* ns, wstate_struct* ws, cflux_struct* cf, nflux_struct* nf, wflux_struct* wf)
{

	/* thinning parameters */
	double STDBc_to_THN, STDBn_to_THN;
	double THN_to_transpC, THN_to_transpN, inc, inn, outc, outn;			
	double THNcoeff_w, THNcoeff_nw;
	double remprop_w, remprop_nw;

	int ok=1;
	int ny;
	int mgmd = THN->mgmd;


	/* yearly varied or constant management parameters */
	if(THN->THN_flag == 2)
	{
		ny = ctrl->simyr;
	}
	else ny=0;

	/**********************************************************************************************/
	/* I. CALCULATING THNcoeff */

	/* thinning if flag=1 */
	if (mgmd >= 0)
	{	
		/* coefficient determining decrease of plant material caused by thinning  */
		THNcoeff_w  = THN->thinningrate_w_array[mgmd][ny]; 			
		THNcoeff_nw = THN->thinningrate_nw_array[mgmd][ny]; ;
		
		/* coefficient determining the remaining proportion of plant material  */
		remprop_w  = (100 - THN->transpcoeff_w_array[mgmd][ny])/100.;
		remprop_nw = (100 - THN->transpcoeff_nw_array[mgmd][ny])/100.;
	


		/**********************************************************************************************/
		/* II. CALCULATING FLUXES */

		/* 1. OUT: as results of the thinning the carbon, nitrogen and water content of the leaf/root/stem decreases - harvested fruit is transported from the site  */


		/* 1.1: leaf, root, fruit, stem */
		if (epc->leaf_cn)
		{
			cf->leafc_to_THN          = cs->leafc * THNcoeff_nw;
			cf->leafc_transfer_to_THN = 0; //cs->leafc_transfer * THNcoeff_nw; 
			cf->leafc_storage_to_THN  = 0; //cs->leafc_storage * THNcoeff_nw; 

			nf->leafn_to_THN           = cf->leafc_to_THN          / epc->leaf_cn;
			nf->leafn_transfer_to_THN  = 0; //cf->leafc_transfer_to_THN / epc->leaf_cn; 
			nf->leafn_storage_to_THN   = 0; //cf->leafc_storage_to_THN  / epc->leaf_cn;  
		}

		if (epc->fruit_cn)
		{
			cf->fruitc_to_THN          = cs->fruitc * THNcoeff_nw;
			cf->fruitc_transfer_to_THN = 0; //cs->fruitc_transfer * THNcoeff_nw; 
			cf->fruitc_storage_to_THN  = 0; //cs->fruitc_storage * THNcoeff_nw; 

			nf->fruitn_to_THN           = cf->fruitc_to_THN          / epc->fruit_cn;
			nf->fruitn_transfer_to_THN  = 0; //cf->fruitc_transfer_to_THN / epc->fruit_cn; 
			nf->fruitn_storage_to_THN   = 0; //cf->fruitc_storage_to_THN  / epc->fruit_cn;  
		}

		if (epc->livewood_cn)
		{
			cf->livestemc_to_THN          = cs->livestemc * THNcoeff_w;
			cf->livestemc_transfer_to_THN = 0; //cs->livestemc_transfer * THNcoeff_w; 
			cf->livestemc_storage_to_THN  = 0; //cs->livestemc_storage * THNcoeff_w; 

			nf->livestemn_to_THN           = cf->livestemc_to_THN          / epc->livewood_cn;
			nf->livestemn_transfer_to_THN  = 0; //cf->livestemc_transfer_to_THN / epc->livewood_cn; 
			nf->livestemn_storage_to_THN   = 0; //cf->livestemc_storage_to_THN  / epc->livewood_cn;  
		}

		if (epc->deadwood_cn)
		{
			cf->deadstemc_to_THN          = cs->deadstemc * THNcoeff_w;
			cf->deadstemc_transfer_to_THN = 0; //cs->deadstemc_transfer * THNcoeff_w; 
			cf->deadstemc_storage_to_THN  = 0; //cs->deadstemc_storage * THNcoeff_w; 

			nf->deadstemn_to_THN           = cf->deadstemc_to_THN          / epc->deadwood_cn;
			nf->deadstemn_transfer_to_THN  = 0; //cf->deadstemc_transfer_to_THN / epc->deadwood_cn; 
			nf->deadstemn_storage_to_THN   = 0; //cf->deadstemc_storage_to_THN  / epc->deadwood_cn;  
		}
	
		cf->gresp_storage_to_THN      = 0; //cs->gresp_storage * THNcoeff_nw; 
		cf->gresp_transfer_to_THN     = 0; //cs->gresp_transfer * THNcoeff_nw; 

		nf->retransn_to_THN           = 0; //ns->retransn * THNcoeff_nw;

		/* 1.2 standing dead biome */

		cf->STDBc_leaf_to_THN     = cs->STDBc_leaf     * THNcoeff_nw; 
		cf->STDBc_fruit_to_THN    = cs->STDBc_fruit    * THNcoeff_nw; 
		cf->STDBc_transfer_to_THN = cs->STDBc_transfer * THNcoeff_nw;

		STDBc_to_THN = cf->STDBc_leaf_to_THN + cf->STDBc_fruit_to_THN + cf->STDBc_transfer_to_THN;

		nf->STDBn_leaf_to_THN     = ns->STDBn_leaf     * THNcoeff_nw; 
		nf->STDBn_fruit_to_THN    = ns->STDBn_fruit    * THNcoeff_nw; 
		nf->STDBn_transfer_to_THN = ns->STDBn_transfer * THNcoeff_nw;

		STDBn_to_THN = nf->STDBn_leaf_to_THN + nf->STDBn_fruit_to_THN + nf->STDBn_transfer_to_THN;


		/*-----------------------------------------*/
		/* 1.3. water */
	
		wf->canopyw_to_THN = ws->canopyw * THNcoeff_nw;



		/*-----------------------------------------------------------------------------------*/
		/* 2. TRANSPORT: part of the plant material is transported  (transp_coeff part of leaf, fruit and stem, but no transfer pools!)*/	
	

		THN_to_transpC = (cf->leafc_to_THN        + cf->fruitc_to_THN  + 
			              cf->STDBc_leaf_to_THN   + cf->STDBc_fruit_to_THN)             * (1-remprop_nw) +
						 (cf->livestemc_to_THN    + cf->deadstemc_to_THN )             * (1-remprop_w);

		THN_to_transpN = (nf->leafn_to_THN        + nf->fruitn_to_THN  + 
			              nf->STDBn_leaf_to_THN   + nf->STDBn_fruit_to_THN)             * (1-remprop_nw) +
						 (nf->livestemn_to_THN    + nf->deadstemn_to_THN )             * (1-remprop_w);
	

		/*-----------------------------------------------------------------------------------*/
		/* 3. IN: the rest remains at the site (THN_to_litr_strg, THN_to_cwd_strg -  cwd_strg: temporary cwd storage pool which contains the cut-down part of coarse root and stem  */
	
		cf->THN_to_CTDBc_leaf     = (cf->leafc_to_THN + cf->STDBc_leaf_to_THN) * remprop_nw;

		cf->THN_to_CTDBc_fruit    = (cf->fruitc_to_THN + cf->STDBc_fruit_to_THN) * remprop_nw;

		cf->THN_to_CTDBc_transfer = cf->leafc_transfer_to_THN     + cf->leafc_storage_to_THN + 
									cf->fruitc_transfer_to_THN    + cf->fruitc_storage_to_THN + 
									cf->gresp_storage_to_THN      + cf->gresp_transfer_to_THN  + 
									cf->STDBc_transfer_to_THN     +
									cf->livestemc_transfer_to_THN + cf->livestemc_storage_to_THN + 
									cf->deadstemc_transfer_to_THN + cf->deadstemc_storage_to_THN; 


		cf->THN_to_CTDBc_cstem    = (cf->livestemc_to_THN + cf->deadstemc_to_THN) * remprop_w;			     

	
		nf->THN_to_CTDBn_leaf     = (nf->leafn_to_THN + nf->STDBn_leaf_to_THN) * remprop_nw;

		nf->THN_to_CTDBn_fruit    = (nf->fruitn_to_THN + nf->STDBn_fruit_to_THN) * remprop_nw;


		nf->THN_to_CTDBn_transfer = nf->leafn_transfer_to_THN     + nf->leafn_storage_to_THN + 
									nf->fruitn_transfer_to_THN    + nf->fruitn_storage_to_THN + 
									nf->retransn_to_THN +
									nf->livestemn_transfer_to_THN + nf->livestemn_storage_to_THN + 
									nf->deadstemn_transfer_to_THN + nf->deadstemn_storage_to_THN; 

		nf->THN_to_CTDBn_cstem    = (nf->livestemn_to_THN + nf->deadstemn_to_THN) * remprop_w;			     


		/**********************************************************************************************/
		/* III. STATE UPDATE */

		/* OUT */
		/* 1.1. leaf, root, fruit, stem */
		cs->leafc          -= cf->leafc_to_THN;
		cs->leafc_transfer -= cf->leafc_transfer_to_THN;
		cs->leafc_storage  -= cf->leafc_storage_to_THN;
		cs->fruitc          -= cf->fruitc_to_THN;
		cs->fruitc_transfer -= cf->fruitc_transfer_to_THN;
		cs->fruitc_storage  -= cf->fruitc_storage_to_THN;
	
		cs->gresp_storage   -= cf->gresp_storage_to_THN;
		cs->gresp_transfer  -= cf->gresp_transfer_to_THN;
	
		cs->livestemc           -= cf->livestemc_to_THN;
		cs->livestemc_transfer  -= cf->livestemc_transfer_to_THN;
		cs->livestemc_storage   -= cf->livestemc_storage_to_THN;
		cs->deadstemc           -= cf->deadstemc_to_THN;
		cs->deadstemc_transfer  -= cf->deadstemc_transfer_to_THN;
		cs->deadstemc_storage   -= cf->deadstemc_storage_to_THN;

		ns->leafn				-= nf->leafn_to_THN;
		ns->leafn_transfer		-= nf->leafn_transfer_to_THN;
		ns->leafn_storage		-= nf->leafn_storage_to_THN;	
		ns->fruitn				-= nf->fruitn_to_THN;
		ns->fruitn_transfer		-= nf->fruitn_transfer_to_THN;
		ns->fruitn_storage		-= nf->fruitn_storage_to_THN;	
	
		ns->retransn        -= nf->retransn_to_THN;
	
		ns->livestemn			-= nf->livestemn_to_THN;
		ns->livestemn_transfer  -= nf->livestemn_transfer_to_THN;
		ns->livestemn_storage	-= nf->livestemn_storage_to_THN;	
		ns->deadstemn			-= nf->deadstemn_to_THN;
		ns->deadstemn_transfer  -= nf->deadstemn_transfer_to_THN;
		ns->deadstemn_storage	-= nf->deadstemn_storage_to_THN;	
	


		/* 1.2. standing dead biome */
	
		cs->STDBc_leaf     -= cf->STDBc_leaf_to_THN; 
		cs->STDBc_fruit    -= cf->STDBc_fruit_to_THN; 
		cs->STDBc_transfer -= cf->STDBc_transfer_to_THN;
	
		ns->STDBn_leaf     -= nf->STDBn_leaf_to_THN; 
		ns->STDBn_fruit    -= nf->STDBn_fruit_to_THN; 
		ns->STDBn_transfer -= nf->STDBn_transfer_to_THN;



		/* 1.3. water */
		ws->canopyw_THNsnk += wf->canopyw_to_THN;
		ws->canopyw -= wf->canopyw_to_THN;

		/* 2. TRANSPORT */
		cs->THN_transportC  += THN_to_transpC;
		ns->THN_transportN  += THN_to_transpN;

		/* 3. IN: cut-down biome */
		cs->CTDBc_leaf     += cf->THN_to_CTDBc_leaf;
		cs->CTDBc_fruit    += cf->THN_to_CTDBc_fruit;
		cs->CTDBc_transfer += cf->THN_to_CTDBc_transfer;
		cs->CTDBc_cstem    += cf->THN_to_CTDBc_cstem;

		ns->CTDBn_leaf     += nf->THN_to_CTDBn_leaf;
		ns->CTDBn_fruit    += nf->THN_to_CTDBn_fruit;
		ns->CTDBn_transfer += nf->THN_to_CTDBn_transfer;
		ns->CTDBn_cstem    += nf->THN_to_CTDBn_cstem;
	

		/**********************************************************************************************/
		/* IV. CONTROL */

		outc = cf->leafc_to_THN + cf->leafc_transfer_to_THN + cf->leafc_storage_to_THN +
			   cf->fruitc_to_THN + cf->fruitc_transfer_to_THN + cf->fruitc_storage_to_THN +
			   cf->livestemc_to_THN + cf->livestemc_transfer_to_THN + cf->livestemc_storage_to_THN +
			   cf->deadstemc_to_THN + cf->deadstemc_transfer_to_THN + cf->deadstemc_storage_to_THN +
			   cf->gresp_storage_to_THN + cf->gresp_transfer_to_THN + 
			   STDBc_to_THN;

		outn = nf->leafn_to_THN + nf->leafn_transfer_to_THN + nf->leafn_storage_to_THN +
			   nf->fruitn_to_THN + nf->fruitn_transfer_to_THN + nf->fruitn_storage_to_THN +
			   nf->livestemn_to_THN + nf->livestemn_transfer_to_THN + nf->livestemn_storage_to_THN +
			   nf->deadstemn_to_THN + nf->deadstemn_transfer_to_THN + nf->deadstemn_storage_to_THN +
			   nf->retransn_to_THN + 
			   STDBn_to_THN;

		inc = cf->THN_to_CTDBc_leaf + cf->THN_to_CTDBc_fruit  + cf->THN_to_CTDBc_transfer + cf->THN_to_CTDBc_cstem;
		inn = nf->THN_to_CTDBn_leaf + nf->THN_to_CTDBn_fruit  + nf->THN_to_CTDBn_transfer + nf->THN_to_CTDBn_cstem;

		if (fabs(inc + THN_to_transpC - outc) > CRIT_PREC || fabs(inn + THN_to_transpN - outn) > CRIT_PREC )
		{
			printf("BALANCE ERROR in thinning calculation in thinning.c\n");
			ok=0;
		}
	}
   return (!ok);
}
	