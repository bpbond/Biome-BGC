/* 
grazing.c
grazing  - decrease the plant material (leafc, leafn, canopy water) and increase N-content (mature)

method: Vuichard et al, 2007
NOTE: LSU: livestock unit = unit used to compare or aggregate different species and it is equivalnet to the liveweight of an average cattle (1 adult cattle = 1 LSU)

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

int grazing(control_struct* ctrl, const epconst_struct* epc, const siteconst_struct* sitec, grazing_struct* GRZ, epvar_struct* epv,
			cstate_struct* cs, nstate_struct* ns, wstate_struct* ws, cflux_struct* cf, nflux_struct* nf, wflux_struct* wf)
{


	/* grazing parameters */
	double DMintake, stocking_rate,weight_LSU;			

	/* local variables */
	double EFman_N2O, Nexrate, EFman_CH4, EFfer_CH4;
	double DMintake2excr, DM_Ccontent, EXCR_Ccontent_array, EXCR_Ncontent_array;				
    double excr2litter;				        /* proportion of excrement return to litter */
	double GRZcoeff;						/* coefficient determining decrease of plant material caused by grazing  */
	double befgrazing_leafc = 0;			/* value of leafc before grazing */
	double aftergrazing_leafc = 0;			/* value of leafc before grazing */
	double daily_excr_prod = 0;				/* daily excrement production */	
	double daily_C_loss = 0;				/* daily carbon loss due to grazing */
	double Cplus_from_excrement = 0;		/* daily carbon plus from excrement */
	double Nplus_from_excrement = 0;        /* daily nitrogen plus from excrement */

	double propLAYER0, propLAYER1, propLAYER2;
	
	int GRZyday_start, GRZyday_end;
	
	int errorCode=0;
	int md, year;

	/* fraction of total annual nitrogen excretion for pasture management system*/
	double MS_N2O = 0.015;

	year = ctrl->simstartyear + ctrl->simyr;
	md = GRZ->mgmdGRZ-1;

	GRZcoeff  = 0.0;


	/**********************************************************************************************/
	/* I. CALCULATING GRZcoeff */

	if (GRZ->GRZ_num && md >= 0)
	{
		GRZyday_start = date_to_doy(GRZ->GRZstart_month_array[md], GRZ->GRZstart_day_array[md]);
		GRZyday_end   = date_to_doy(GRZ->GRZend_month_array[md], GRZ->GRZend_day_array[md]);

	
		if (year == GRZ->GRZstart_year_array[md] && ctrl->yday >= GRZyday_start && ctrl->yday <= GRZyday_end) 
		{
		
			GRZ->trampleff_act  = GRZ->trampling_effect[md];
			DMintake            = GRZ->DMintake_array[md];						 /*  unit: kgDM/LSU (DM:dry matter)*/
			stocking_rate       = GRZ->stocking_rate_array[md]/10000;			/*  unit: LSU/ha -> new unit: LSU/m2 */
			weight_LSU			= GRZ->weight_LSU[md];

			DMintake2excr       = GRZ->DMintake2excr_array[md] / 100.;	     /* from proporiton(%) to ratio(number) */
			DM_Ccontent         = GRZ->DM_Ccontent_array[md] / 100.;			/* from proporiton(%) to ratio(number) */
			EXCR_Ccontent_array = GRZ->EXCR_Ccontent_array[md] / 100.;			 /* from proporiton(%) to ratio(number) */
			EXCR_Ncontent_array = GRZ->EXCR_Ncontent_array[md] / 100.;			 /* from proporiton(%) to ratio(number) */
			excr2litter         = GRZ->excr2litter_array[md] / 100;

			Nexrate   = GRZ->Nexrate[md];
			EFman_N2O = GRZ->EFman_N2O[md];
			EFman_CH4 = GRZ->EFman_CH4[md]/nDAYS_OF_YEAR;;
			EFfer_CH4 = GRZ->EFfer_CH4[md]/nDAYS_OF_YEAR;;

		
			if (epv->proj_lai)
			{
				/* daily total ingested carbon per m2 from daily ingested drymatter and carbon content of drymatter and stocking rate
								[kgC/m2 = kgDM/LSU * (kgC/kgDM) * (LSU/m2)] */	
				daily_C_loss = (DMintake * DM_Ccontent) * stocking_rate;	
	
		
				/* effect of grazing: decrease of leafc and increase of soilc and soiln (manure)*/
				befgrazing_leafc = cs->leafc;
		
				if (befgrazing_leafc - daily_C_loss > 0)
				{
					aftergrazing_leafc = befgrazing_leafc - daily_C_loss;
					GRZcoeff  = 1-aftergrazing_leafc/befgrazing_leafc;
				}
				else
				{
					GRZcoeff  = 1.0;
					aftergrazing_leafc = 0;
					daily_C_loss = befgrazing_leafc;

				}	
				daily_excr_prod = (daily_C_loss/DM_Ccontent) * DMintake2excr;/* kg manure/m2/day -> kgC/m2/day */
			}
			else
			{
				/* grazingW_flag: flag of WARNING writing in log file (only at first time) */
				if (!ctrl->grazingW_flag) ctrl->grazingW_flag = 1;
			}
		}
	}


	/**********************************************************************************************/
	/* II. CALCULATING FLUXES */

	/*----------------------------------------------------------*/
	/* 1. OUT: daily loss due to GRZ */ 

	/* 1.1. actual and transfer plant pools*/
	if (GRZcoeff)
	{
		if (epc->leaf_cn)
		{
			cf->leafc_to_GRZ              = cs->leafc * GRZcoeff;
			cf->leafc_transfer_to_GRZ     = 0; //cs->leafc_transfer * GRZcoeff;
			cf->leafc_storage_to_GRZ      = 0; //cs->leafc_storage * GRZcoeff;

			nf->leafn_to_GRZ              = cf->leafc_to_GRZ          / epc->leaf_cn;
			nf->leafn_transfer_to_GRZ     = 0; //cf->leafc_transfer_to_GRZ / epc->leaf_cn;
			nf->leafn_storage_to_GRZ      = 0; //cf->leafc_storage_to_GRZ  / epc->leaf_cn;
		}
	
		if (epc->fruit_cn)
		{
			cf->fruitc_to_GRZ              = cs->fruitc * GRZcoeff;
			cf->fruitc_transfer_to_GRZ     = 0; //cs->fruitc_transfer * GRZcoeff;
			cf->fruitc_storage_to_GRZ      = 0; //cs->fruitc_storage * GRZcoeff;

			nf->fruitn_to_GRZ              = cf->fruitc_to_GRZ          / epc->fruit_cn;
			nf->fruitn_transfer_to_GRZ     = 0; //cf->fruitc_transfer_to_GRZ / epc->fruit_cn;
			nf->fruitn_storage_to_GRZ      = 0; //cf->fruitc_storage_to_GRZ  / epc->fruit_cn;
		}

		if (epc->softstem_cn)
		{
			cf->softstemc_to_GRZ              = cs->softstemc * GRZcoeff;
			cf->softstemc_transfer_to_GRZ     = 0; //cs->softstemc_transfer * GRZcoeff;
			cf->softstemc_storage_to_GRZ      = 0; //cs->softstemc_storage * GRZcoeff;

			nf->softstemn_to_GRZ              = cf->softstemc_to_GRZ          / epc->softstem_cn;
			nf->softstemn_transfer_to_GRZ     = 0; //cf->softstemc_transfer_to_GRZ / epc->softstem_cn;
			nf->softstemn_storage_to_GRZ      = 0; //cf->softstemc_storage_to_GRZ  / epc->softstem_cn;
		}
	
		cf->gresp_transfer_to_GRZ     = 0; //cs->gresp_transfer * GRZcoeff;
		cf->gresp_storage_to_GRZ      = 0; //cs->gresp_storage * GRZcoeff;

		nf->retransn_to_GRZ           = 0; //ns->retransn * GRZcoeff ;

	
		/* 1.2.  standing dead biome */
		cf->STDBc_leaf_to_GRZ     = cs->STDBc_leaf     * GRZcoeff;
		cf->STDBc_fruit_to_GRZ    = cs->STDBc_fruit    * GRZcoeff;
		cf->STDBc_softstem_to_GRZ = cs->STDBc_softstem * GRZcoeff;
		cf->STDBc_nsc_to_GRZ = cs->STDBc_nsc      * GRZcoeff;

		nf->STDBn_leaf_to_GRZ     = ns->STDBn_leaf     * GRZcoeff;
		nf->STDBn_fruit_to_GRZ    = ns->STDBn_fruit    * GRZcoeff;
		nf->STDBn_softstem_to_GRZ = ns->STDBn_softstem * GRZcoeff;
		nf->STDBn_nsc_to_GRZ      = ns->STDBn_nsc      * GRZcoeff;


		/* 1.3 WATER */
		wf->canopyw_to_GRZ = ws->canopyw * GRZcoeff;

		/*----------------------------------------------------------*/
		/* 2. IN: MANURE PRODUCTION (plant material goes into the litter pool) - gain to ecosystem is loss for "atmosphere" - negatice sign*/	

		/* daily manure production per m2 (return to the litter) from daily total ingested dry matter and litter_return_ratio and its C and N content
						[kgMANURE = (kgDM/LSU) * (LSU/m2) * (%)] */
		Cplus_from_excrement = daily_excr_prod * EXCR_Ccontent_array;
		Nplus_from_excrement = daily_excr_prod * EXCR_Ncontent_array;

		cf->GRZ_to_litr1c = (Cplus_from_excrement) * epc->leaflitr_flab * excr2litter;
		cf->GRZ_to_litr2c = (Cplus_from_excrement) * epc->leaflitr_fucel * excr2litter;
		cf->GRZ_to_litr3c = (Cplus_from_excrement) * epc->leaflitr_fscel * excr2litter;
		cf->GRZ_to_litr4c = (Cplus_from_excrement) * epc->leaflitr_flig * excr2litter;

		nf->GRZ_to_litr1n = (Nplus_from_excrement) * epc->leaflitr_flab * excr2litter;  
		nf->GRZ_to_litr2n = (Nplus_from_excrement) * epc->leaflitr_fucel * excr2litter; 
		nf->GRZ_to_litr3n = (Nplus_from_excrement) * epc->leaflitr_fscel * excr2litter; 
		nf->GRZ_to_litr4n = (Nplus_from_excrement) * epc->leaflitr_flig * excr2litter;

	
		/*----------------------------------------------------------*/
		/* 3. CH4 and N2O emissions */ 

		/*  dimension: db animal/m2 * kgN/(kg animal * day) * kg animal/db animal = kgN/m2/day */
		nf->N2O_flux_GRZ     = stocking_rate * Nexrate * weight_LSU/1000 *  EFman_N2O * MS_N2O;

		/* dimension: kgCH4/head/day * head / m2 * (C/CH4) * mg/kg = gC/m2/day */
		cf->CH4_flux_MANURE  = EFman_CH4  * stocking_rate * 12./16.;  
		cf->CH4_flux_ANIMAL  = EFfer_CH4  * stocking_rate * 12./16.;  


		/**********************************************************************************************/
		/* III. STATE UPDATE */

		/* 1.actual and transfer plant pools */	

		cs->leafc				-= cf->leafc_to_GRZ;
		cs->fruitc				-= cf->fruitc_to_GRZ;
		cs->softstemc			-= cf->softstemc_to_GRZ;

		cs->leafc_storage		-= cf->leafc_storage_to_GRZ;
		cs->fruitc_storage		-= cf->fruitc_storage_to_GRZ;
		cs->softstemc_storage	-= cf->softstemc_storage_to_GRZ;

		cs->leafc_transfer		-= cf->leafc_transfer_to_GRZ;
		cs->fruitc_transfer		-= cf->fruitc_transfer_to_GRZ;
		cs->softstemc_transfer	-= cf->softstemc_transfer_to_GRZ;

		cs->gresp_transfer      -= cf->gresp_transfer_to_GRZ;
		cs->gresp_storage       -= cf->gresp_storage_to_GRZ;

		cs->GRZsnk_C			+= cf->leafc_to_GRZ          + cf->fruitc_to_GRZ          + cf->softstemc_to_GRZ +
									cf->leafc_storage_to_GRZ  + cf->fruitc_storage_to_GRZ  + cf->softstemc_storage_to_GRZ +
									cf->leafc_transfer_to_GRZ + cf->fruitc_transfer_to_GRZ + cf->softstemc_transfer_to_GRZ +
									cf->gresp_transfer_to_GRZ + cf->gresp_storage_to_GRZ;

		ns->leafn				-= nf->leafn_to_GRZ;
		ns->fruitn				-= nf->fruitn_to_GRZ;
		ns->softstemn			-= nf->softstemn_to_GRZ;

		ns->leafn_storage		-= nf->leafn_storage_to_GRZ;
		ns->fruitn_storage		-= nf->fruitn_storage_to_GRZ;
		ns->softstemn_storage	-= nf->softstemn_storage_to_GRZ;

		ns->leafn_transfer		-= nf->leafn_transfer_to_GRZ;
		ns->fruitn_transfer		-= nf->fruitn_transfer_to_GRZ;
		ns->softstemn_transfer	-= nf->softstemn_transfer_to_GRZ;

		ns->retransn            -= nf->retransn_to_GRZ;

		ns->GRZsnk_N			+= nf->leafn_to_GRZ          + nf->fruitn_to_GRZ          + nf->softstemn_to_GRZ +
									nf->leafn_storage_to_GRZ  + nf->fruitn_storage_to_GRZ  + nf->softstemn_storage_to_GRZ +
									nf->leafn_transfer_to_GRZ + nf->fruitn_transfer_to_GRZ + nf->softstemn_transfer_to_GRZ +
									nf->retransn_to_GRZ;

		/* 2. dead standing biomass */
		cs->STDBc_leaf     -= cf->STDBc_leaf_to_GRZ;
		cs->STDBc_fruit    -= cf->STDBc_fruit_to_GRZ;
		cs->STDBc_softstem -= cf->STDBc_softstem_to_GRZ;
		cs->STDBc_nsc -= cf->STDBc_nsc_to_GRZ;

		cs->GRZsnk_C       += (cf->STDBc_leaf_to_GRZ + cf->STDBc_fruit_to_GRZ + cf->STDBc_softstem_to_GRZ + cf->STDBc_nsc_to_GRZ);

		ns->STDBn_leaf     -= nf->STDBn_leaf_to_GRZ;
		ns->STDBn_fruit    -= nf->STDBn_fruit_to_GRZ;
		ns->STDBn_softstem -= nf->STDBn_softstem_to_GRZ;
		ns->STDBn_nsc      -= nf->STDBn_nsc_to_GRZ;

		ns->GRZsnk_N       += (nf->STDBn_leaf_to_GRZ + nf->STDBn_fruit_to_GRZ + nf->STDBn_softstem_to_GRZ + nf->STDBn_nsc_to_GRZ);
	
	
		/* 3. water */
		ws->canopyw_GRZsnk += wf->canopyw_to_GRZ;
		ws->canopyw        -= wf->canopyw_to_GRZ;

	
		/* 4. aboveground biomass into top soil layers */

		/* new feature: litter turns into the first AND the second soil layer */
		propLAYER0 = sitec->soillayer_thickness[0]/sitec->soillayer_depth[2];
		propLAYER1 = sitec->soillayer_thickness[1]/sitec->soillayer_depth[2];
		propLAYER2 = sitec->soillayer_thickness[2]/sitec->soillayer_depth[2];

		cs->litr1c[0] += cf->GRZ_to_litr1c * propLAYER0;
		cs->litr2c[0] += cf->GRZ_to_litr2c * propLAYER0;
		cs->litr3c[0] += cf->GRZ_to_litr3c * propLAYER0;
		cs->litr4c[0] += cf->GRZ_to_litr4c * propLAYER0;

		ns->litr1n[0] += nf->GRZ_to_litr1n * propLAYER0;
		ns->litr2n[0] += nf->GRZ_to_litr2n * propLAYER0;
		ns->litr3n[0] += nf->GRZ_to_litr3n * propLAYER0;
		ns->litr4n[0] += nf->GRZ_to_litr4n * propLAYER0;

		cs->litr1c[1] += cf->GRZ_to_litr1c * propLAYER1;
		cs->litr2c[1] += cf->GRZ_to_litr2c * propLAYER1;
		cs->litr3c[1] += cf->GRZ_to_litr3c * propLAYER1;
		cs->litr4c[1] += cf->GRZ_to_litr4c * propLAYER1;

		ns->litr1n[1] += nf->GRZ_to_litr1n * propLAYER1;
		ns->litr2n[1] += nf->GRZ_to_litr2n * propLAYER1;
		ns->litr3n[1] += nf->GRZ_to_litr3n * propLAYER1;
		ns->litr4n[1] += nf->GRZ_to_litr4n * propLAYER1;

		cs->litr1c[2] += cf->GRZ_to_litr1c * propLAYER2;
		cs->litr2c[2] += cf->GRZ_to_litr2c * propLAYER2;
		cs->litr3c[2] += cf->GRZ_to_litr3c * propLAYER2;
		cs->litr4c[2] += cf->GRZ_to_litr4c * propLAYER2;

		ns->litr1n[2] += nf->GRZ_to_litr1n * propLAYER2;
		ns->litr2n[2] += nf->GRZ_to_litr2n * propLAYER2;
		ns->litr3n[2] += nf->GRZ_to_litr3n * propLAYER2;
		ns->litr4n[2] += nf->GRZ_to_litr4n * propLAYER2;

		/* estimating aboveground litter and cwdc */
		cs->litrc_above += cf->GRZ_to_litr1c + cf->GRZ_to_litr2c + cf->GRZ_to_litr3c + cf->GRZ_to_litr4c;

	
		cs->GRZsrc_C += cf->GRZ_to_litr1c + cf->GRZ_to_litr2c + cf->GRZ_to_litr3c + cf->GRZ_to_litr4c;
		ns->GRZsrc_N += nf->GRZ_to_litr1n + nf->GRZ_to_litr2n + nf->GRZ_to_litr3n + nf->GRZ_to_litr4n;

	}


	
   return (errorCode);
}
	