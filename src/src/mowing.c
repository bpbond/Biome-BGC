/* 
mowing.c
do mowing  - decrease the plant material (leafc, leafn, canopy water)

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Copyright 2020, D. Hidy [dori.hidy@gmail.com]
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

int mowing(const control_struct* ctrl, const epconst_struct* epc, const mowing_struct* MOW, const epvar_struct* epv, 
		   cstate_struct* cs, nstate_struct* ns, wstate_struct* ws, cflux_struct* cf, nflux_struct* nf, wflux_struct* wf)
{

	/* mowing parameters */
	double LAI_limit, outc, outn, inc, inn, MOW_to_transpC, MOW_to_transpN;
	double remained_prop;						/* remained proportion of plabnt material is calculated from transport coefficient */

	/* local parameters */
	double befgrass_LAI;						/* value of LAI before mowing */
	double MOWcoeff;							/* coefficient determining the decrease of plant material caused by mowing */
	int errorCode=0;
	int md, year,ap;

	year = ctrl->simstartyear + ctrl->simyr;
	md = MOW->mgmdMOW-1;

	/* actual phenological phase */
	ap = (int) epv->n_actphen-1; 

	LAI_limit=MOW_to_transpC=MOW_to_transpN=outc=outn=inc=inn=0;
	
	MOWcoeff  = 0.0;

	/**********************************************************************************************/
	/* I. CALCULATING: MOWcoeff */
	
	/* 1. mowing calculation based on LAI_limit: we assume that due to the mowing LAI (leafc) reduces to the value of LAI_limit  */

	befgrass_LAI = cs->leafc * epc->avg_proj_sla[ap];

	/* 2. mowing type: fixday or condMGM */
	if (MOW->condMOW_flag == 0)
	{	
		if (MOW->MOW_num && md >= 0)
		{
			if (year == MOW->MOWyear_array[md] && ctrl->month == MOW->MOWmonth_array[md] && ctrl->day == MOW->MOWday_array[md]) 
			{
				remained_prop = (100 - MOW->transportMOW_array[md])/100.;
				LAI_limit = MOW->LAI_limit_array[md];
			}
			else 
			{
				remained_prop = 0;
				LAI_limit = befgrass_LAI;
			}
		}
	}
	else
	{
		/* mowing if LAI greater than a given value (fixLAIbef_condMOW) */
		if (cs->leafc * epc->avg_proj_sla[ap] > MOW->fixLAIbef_condMOW)
		{
			remained_prop = (100 - MOW->transpCOEFF_condMOW)/100.;
			LAI_limit = MOW->fixLAIaft_condMOW;
			if (ctrl->onscreen && (ctrl->spinup == 0 || ctrl->simyr == 0)) printf("conditional MOWING on %i%s%i\n", ctrl->month, "/", ctrl->day);
		}
		else
		{
			remained_prop = 0;
			LAI_limit = befgrass_LAI;
		}
	}

	/* 3. effect of mowing: MOWcoeff */
	if ((MOW->condMOW_flag || (MOW->MOW_num && md >= 0)) && befgrass_LAI > LAI_limit)
	{	
		MOWcoeff = (1- LAI_limit/befgrass_LAI);
	}


	if (MOWcoeff > 0)
	{
		/**********************************************************************************************/
		/* II. CALCULATING FLUXES */

		/*----------------------------------------------------------*/
		/* 1. daily loss due to MOW */ 

		/* 1.1. actual and transfer plant pools*/
		if (epc->leaf_cn)
		{
			cf->leafc_to_MOW              = cs->leafc * MOWcoeff;
			cf->leafc_transfer_to_MOW     = 0; //cs->leafc_transfer * MOWcoeff * 0.1;
			cf->leafc_storage_to_MOW      = 0; //cs->leafc_storage * MOWcoeff * 0.1;

			nf->leafn_to_MOW              = cf->leafc_to_MOW          / epc->leaf_cn;
			nf->leafn_transfer_to_MOW     = 0; //cf->leafc_transfer_to_MOW / epc->leaf_cn;
			nf->leafn_storage_to_MOW      = 0; //cf->leafc_storage_to_MOW  / epc->leaf_cn;
		}
	
		if (epc->fruit_cn)
		{
			cf->fruitc_to_MOW              = cs->fruitc * MOWcoeff;
			cf->fruitc_transfer_to_MOW     = 0; //cs->fruitc_transfer * MOWcoeff * 0.1;
			cf->fruitc_storage_to_MOW      = 0; //cs->fruitc_storage * MOWcoeff * 0.1;

			nf->fruitn_to_MOW              = cf->fruitc_to_MOW          / epc->fruit_cn;
			nf->fruitn_transfer_to_MOW     = 0; //cf->fruitc_transfer_to_MOW / epc->fruit_cn;
			nf->fruitn_storage_to_MOW      = 0; //cf->fruitc_storage_to_MOW  / epc->fruit_cn;
		}

		if (epc->softstem_cn)
		{
			cf->softstemc_to_MOW              = cs->softstemc * MOWcoeff;
			cf->softstemc_transfer_to_MOW     = 0; //cs->softstemc_transfer * MOWcoeff * 0.1;
			cf->softstemc_storage_to_MOW      = 0; //cs->softstemc_storage * MOWcoeff * 0.1;

			nf->softstemn_to_MOW              = cf->softstemc_to_MOW          / epc->softstem_cn;
			nf->softstemn_transfer_to_MOW     = 0; //cf->softstemc_transfer_to_MOW / epc->softstem_cn;
			nf->softstemn_storage_to_MOW      = 0; //cf->softstemc_storage_to_MOW  / epc->softstem_cn;
		}
	
		cf->gresp_transfer_to_MOW     = 0; //cs->gresp_transfer * MOWcoeff;
		cf->gresp_storage_to_MOW      = 0; //cs->gresp_storage * MOWcoeff;

		nf->retransn_to_MOW           = 0; //ns->retransn * MOWcoeff ;

		/* 1.2.  standing dead biome */
		cf->STDBc_leaf_to_MOW     = cs->STDBc_leaf     * MOWcoeff;
		cf->STDBc_fruit_to_MOW    = cs->STDBc_fruit    * MOWcoeff; 
		cf->STDBc_softstem_to_MOW = cs->STDBc_softstem * MOWcoeff;
		cf->STDBc_nsc_to_MOW = cs->STDBc_nsc * MOWcoeff;

		nf->STDBn_leaf_to_MOW     = ns->STDBn_leaf     * MOWcoeff; 
		nf->STDBn_fruit_to_MOW    = ns->STDBn_fruit    * MOWcoeff; 
		nf->STDBn_softstem_to_MOW = ns->STDBn_softstem * MOWcoeff;
		nf->STDBn_nsc_to_MOW      = ns->STDBn_nsc * MOWcoeff;



		/* 1.3 water */
		wf->canopyw_to_MOW = ws->canopyw * MOWcoeff;


		/*----------------------------------------------------------*/
		/* 2. transport: part of the plant material is transported (MOW_to_transpC and MOW_to_transpN; transp_coeff = 1-remained_prop),*/

		MOW_to_transpC = (cf->leafc_to_MOW + cf->fruitc_to_MOW + cf->softstemc_to_MOW + 
						  cf->STDBc_leaf_to_MOW + cf->STDBc_fruit_to_MOW + cf->STDBc_softstem_to_MOW)  * (1-remained_prop);
		MOW_to_transpN = (nf->leafn_to_MOW + nf->fruitn_to_MOW + nf->softstemn_to_MOW + 
						  nf->STDBn_leaf_to_MOW + nf->STDBn_fruit_to_MOW + nf->STDBn_softstem_to_MOW)  * (1-remained_prop);


		/*----------------------------------------------------------*/
		/* 3. cut-down biomass: the rest remains at the site (MOW_to_litrc_strg, MOW_to_litrn_strg)*/
	
		cf->MOW_to_CTDBc_leaf     = (cf->leafc_to_MOW     + cf->STDBc_leaf_to_MOW) * remained_prop;

		cf->MOW_to_CTDBc_fruit    = (cf->fruitc_to_MOW    + cf->STDBc_fruit_to_MOW)  * remained_prop;

		cf->MOW_to_CTDBc_softstem = (cf->softstemc_to_MOW + cf->STDBc_softstem_to_MOW)  * remained_prop;

		cf->MOW_to_CTDBc_nsc      = (cf->leafc_transfer_to_MOW     + cf->leafc_storage_to_MOW + 
									 cf->fruitc_transfer_to_MOW    + cf->fruitc_storage_to_MOW + 
									 cf->softstemc_transfer_to_MOW + cf->softstemc_storage_to_MOW + 
									 cf->gresp_storage_to_MOW      + cf->gresp_transfer_to_MOW +
									 cf->STDBc_nsc_to_MOW);


		nf->MOW_to_CTDBn_leaf     = (nf->leafn_to_MOW     + nf->STDBn_leaf_to_MOW) * remained_prop;

		nf->MOW_to_CTDBn_fruit    = (nf->fruitn_to_MOW    + nf->STDBn_fruit_to_MOW)  * remained_prop;

		nf->MOW_to_CTDBn_softstem = (nf->softstemn_to_MOW + nf->STDBn_softstem_to_MOW) * remained_prop;

		nf->MOW_to_CTDBn_nsc      = (nf->leafn_transfer_to_MOW     + nf->leafn_storage_to_MOW + 
									 nf->fruitn_transfer_to_MOW    + nf->fruitn_storage_to_MOW + 
									 nf->softstemn_transfer_to_MOW + nf->softstemn_storage_to_MOW + 
									 nf->retransn_to_MOW +
									 nf->STDBn_nsc_to_MOW);

		/**********************************************************************************************/
		/* III. STATE UPDATE */

		/* 1. OUT */
		/* 1.1.actual and transfer plant pools */	
		cs->leafc				-= cf->leafc_to_MOW;
		cs->fruitc				-= cf->fruitc_to_MOW;
		cs->softstemc			-= cf->softstemc_to_MOW;

		cs->leafc_storage		-= cf->leafc_storage_to_MOW;
		cs->fruitc_storage		-= cf->fruitc_storage_to_MOW;
		cs->softstemc_storage	-= cf->softstemc_storage_to_MOW;

		cs->leafc_transfer		-= cf->leafc_transfer_to_MOW;
		cs->fruitc_transfer		-= cf->fruitc_transfer_to_MOW;
		cs->softstemc_transfer	-= cf->softstemc_transfer_to_MOW;

		cs->gresp_transfer      -= cf->gresp_transfer_to_MOW;
		cs->gresp_storage       -= cf->gresp_storage_to_MOW;


		ns->leafn				-= nf->leafn_to_MOW;
		ns->fruitn				-= nf->fruitn_to_MOW;
		ns->softstemn			-= nf->softstemn_to_MOW;

		ns->leafn_storage		-= nf->leafn_storage_to_MOW;
		ns->fruitn_storage		-= nf->fruitn_storage_to_MOW;
		ns->softstemn_storage	-= nf->softstemn_storage_to_MOW;

		ns->leafn_transfer		-= nf->leafn_transfer_to_MOW;
		ns->fruitn_transfer		-= nf->fruitn_transfer_to_MOW;
		ns->softstemn_transfer	-= nf->softstemn_transfer_to_MOW;

		ns->retransn            -= nf->retransn_to_MOW;


		/* 1.2. dead standing biomass */
		cs->STDBc_leaf     -= cf->STDBc_leaf_to_MOW;
		cs->STDBc_fruit    -= cf->STDBc_fruit_to_MOW;
		cs->STDBc_softstem -= cf->STDBc_softstem_to_MOW;
		cs->STDBc_nsc -= cf->STDBc_nsc_to_MOW ;

		ns->STDBn_leaf     -= nf->STDBn_leaf_to_MOW;
		ns->STDBn_fruit    -= nf->STDBn_fruit_to_MOW;
		ns->STDBn_softstem -= nf->STDBn_softstem_to_MOW;
		ns->STDBn_nsc      -= nf->STDBn_nsc_to_MOW ;


		/* 1.3. water */
		ws->canopyw_MOWsnk += wf->canopyw_to_MOW;
		ws->canopyw        -= wf->canopyw_to_MOW;

	
		/* 2. TRANSPORT*/
		cs->MOW_transportC  += MOW_to_transpC;
		ns->MOW_transportN  += MOW_to_transpN;
	
	
		/* 3. IN: cut-down biome  */
		cs->CTDBc_leaf     += cf->MOW_to_CTDBc_leaf;
		cs->CTDBc_fruit    += cf->MOW_to_CTDBc_fruit;
		cs->CTDBc_softstem += cf->MOW_to_CTDBc_softstem;
		cs->CTDBc_nsc      += cf->MOW_to_CTDBc_nsc;

		ns->CTDBn_leaf     += nf->MOW_to_CTDBn_leaf;
		ns->CTDBn_fruit    += nf->MOW_to_CTDBn_fruit;
		ns->CTDBn_softstem += nf->MOW_to_CTDBn_softstem;
		ns->CTDBn_nsc      += nf->MOW_to_CTDBn_nsc;
	

		/**********************************************************************************************/
		/* IV. CONTROL */

		outc = cf->leafc_to_MOW + cf->leafc_transfer_to_MOW + cf->leafc_storage_to_MOW +
			   cf->fruitc_to_MOW + cf->fruitc_transfer_to_MOW + cf->fruitc_storage_to_MOW +
			   cf->softstemc_to_MOW + cf->softstemc_transfer_to_MOW + cf->softstemc_storage_to_MOW +
			   cf->gresp_storage_to_MOW + cf->gresp_transfer_to_MOW + 
			   cf->STDBc_leaf_to_MOW + cf->STDBc_fruit_to_MOW + cf->STDBc_softstem_to_MOW + cf->STDBc_nsc_to_MOW;

		outn = nf->leafn_to_MOW + nf->leafn_transfer_to_MOW + nf->leafn_storage_to_MOW +
			   nf->fruitn_to_MOW + nf->fruitn_transfer_to_MOW + nf->fruitn_storage_to_MOW +
			   nf->softstemn_to_MOW + nf->softstemn_transfer_to_MOW + nf->softstemn_storage_to_MOW +
			   nf->retransn_to_MOW + 
			   nf->STDBn_leaf_to_MOW + nf->STDBn_fruit_to_MOW + nf->STDBn_softstem_to_MOW + nf->STDBn_nsc_to_MOW;

		inc = cf->MOW_to_CTDBc_leaf + cf->MOW_to_CTDBc_fruit  + cf->MOW_to_CTDBc_softstem + cf->MOW_to_CTDBc_nsc;
		inn = nf->MOW_to_CTDBn_leaf + nf->MOW_to_CTDBn_fruit  + nf->MOW_to_CTDBn_softstem + nf->MOW_to_CTDBn_nsc;

		if (fabs(inc + MOW_to_transpC - outc) > CRIT_PREC || fabs(inn + MOW_to_transpN - outn) > CRIT_PREC )
		{
			printf("\n");
			printf("BALANCE ERROR in mowing calculation in mowing.c\n");
			errorCode=1;
		}


}

  return (errorCode);
}	

