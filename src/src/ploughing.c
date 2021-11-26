 /* 
ploughing.c
do ploughing  - decrease the plant material (leafc, leafn, canopy water)

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

int ploughing(const control_struct* ctrl, const epconst_struct* epc, siteconst_struct* sitec, soilprop_struct* sprop, metvar_struct* metv,  epvar_struct* epv,
			  ploughing_struct* PLG, cstate_struct* cs, nstate_struct* ns, wstate_struct* ws, cflux_struct* cf, nflux_struct* nf, wflux_struct* wf)
{

	/* ploughing parameters */
	int PLGlayer, layer;
	double PLGdepth, PLGcoeff, sminNH4_SUM, sminNO3_SUM, soilw_SUM, tsoil_SUM, sand_SUM, silt_SUM;	 
	double litr1c_SUM, litr2c_SUM, litr3c_SUM, litr4c_SUM, litr1n_SUM, litr2n_SUM, litr3n_SUM, litr4n_SUM;
	double soil1c_SUM, soil2c_SUM, soil3c_SUM, soil4c_SUM, soil1n_SUM, soil2n_SUM, soil3n_SUM, soil4n_SUM;
	int md, year;
	
	int errorCode = 0;

	year = ctrl->simstartyear + ctrl->simyr;
	md = PLG->mgmdPLG-1;

	errorCode=0;
	PLGdepth=0;
	PLGcoeff=sminNH4_SUM=sminNO3_SUM=soilw_SUM=tsoil_SUM=sand_SUM=silt_SUM=0;	 
	litr1c_SUM=litr2c_SUM=litr3c_SUM=litr4c_SUM=litr1n_SUM=litr2n_SUM=litr3n_SUM=litr4n_SUM=0;
	soil1c_SUM=soil2c_SUM=soil3c_SUM=soil4c_SUM=soil1n_SUM=soil2n_SUM=soil3n_SUM=soil4n_SUM=0;

	PLGcoeff = 0;

	/**********************************************************************************************/
	/* I. CALCULATING PLGcoeff AND PLGdepth */

	if (PLG->PLG_num && md >= 0)
	{
		if (year == PLG->PLGyear_array[md] && ctrl->month == PLG->PLGmonth_array[md] && ctrl->day == PLG->PLGday_array[md])
		{
			/* decrease of plant material caused by ploughing: difference between plant material before and after harvesting */
			PLGcoeff      = 1.0;  
		
			/* ploughing layer from depth */
			layer = 1;
			PLGlayer = 0;
			PLGdepth = PLG->PLGdepths_array[md];

			if (PLGdepth > sitec->soillayer_depth[0])
			{
				while (PLGlayer== 0 && layer < N_SOILLAYERS)
				{
					if ((PLGdepth > sitec->soillayer_depth[layer-1]) && (PLGdepth <= sitec->soillayer_depth[layer])) layer += 1;
					PLGlayer  = layer;
				}
				if (PLGlayer == 0)
				{
					printf("ERROR in ploughing depth calculation (ploughing.c)\n");
					errorCode=1;
				}
			}
		}
	}
	/**********************************************************************************************/
	/* II. UNIFORM DISTRIBUTION OF SMINN, VWC litterC, litterN and soilC and soilN AFTER PLOUGHING */


	if (PLGcoeff > 0)
	{
		for (layer = 0; layer<PLGlayer; layer++)
		{
			tsoil_SUM += metv->tsoil[layer] * sitec->soillayer_thickness[layer] / sitec->soillayer_depth[PLGlayer-1];

			soilw_SUM     += ws->soilw[layer];
			sminNH4_SUM   += ns->sminNH4[layer];
			sminNO3_SUM   += ns->sminNO3[layer];
			sand_SUM      += sprop->sand[layer];
			silt_SUM      += sprop->silt[layer];
			litr1c_SUM += cs->litr1c[layer];
			litr2c_SUM += cs->litr2c[layer];
			litr3c_SUM += cs->litr3c[layer];
			litr4c_SUM += cs->litr4c[layer];
			litr1n_SUM += ns->litr1n[layer];  
			litr2n_SUM += ns->litr2n[layer]; 
			litr3n_SUM += ns->litr3n[layer]; 
			litr4n_SUM += ns->litr4n[layer]; 
			soil1c_SUM += cs->soil1c[layer];
			soil2c_SUM += cs->soil2c[layer];
			soil3c_SUM += cs->soil3c[layer];
			soil4c_SUM += cs->soil4c[layer];
			soil1n_SUM += ns->soil1n[layer];
			soil2n_SUM += ns->soil2n[layer];
			soil3n_SUM += ns->soil3n[layer];
			soil4n_SUM += ns->soil4n[layer];

		}


		for (layer = 0; layer<PLGlayer; layer++)
		{
			metv->tsoil[layer] = tsoil_SUM;

			ws->soilw[layer]   = soilw_SUM * sitec->soillayer_thickness[layer] / sitec->soillayer_depth[PLGlayer-1];
			epv->VWC[layer]    = ws->soilw[layer] / (water_density * sitec->soillayer_thickness[layer]);

			sprop->sand[layer] = sand_SUM/PLGdepth;
			sprop->silt[layer] = silt_SUM/PLGdepth;
			sprop->clay[layer] = 100-sprop->sand[layer]-sprop->silt[layer];

			ns->sminNH4[layer]   = sminNH4_SUM * sitec->soillayer_thickness[layer] / sitec->soillayer_depth[PLGlayer-1];
			ns->sminNO3[layer]   = sminNO3_SUM * sitec->soillayer_thickness[layer] / sitec->soillayer_depth[PLGlayer-1];
			cs->litr1c[layer]  = litr1c_SUM * sitec->soillayer_thickness[layer] / sitec->soillayer_depth[PLGlayer-1];
			cs->litr2c[layer]  = litr2c_SUM * sitec->soillayer_thickness[layer] / sitec->soillayer_depth[PLGlayer-1];
			cs->litr3c[layer]  = litr3c_SUM * sitec->soillayer_thickness[layer] / sitec->soillayer_depth[PLGlayer-1];
			cs->litr4c[layer]  = litr4c_SUM * sitec->soillayer_thickness[layer] / sitec->soillayer_depth[PLGlayer-1];
			ns->litr1n[layer]  = litr1n_SUM * sitec->soillayer_thickness[layer] / sitec->soillayer_depth[PLGlayer-1];
			ns->litr2n[layer]  = litr2n_SUM * sitec->soillayer_thickness[layer] / sitec->soillayer_depth[PLGlayer-1];
			ns->litr3n[layer]  = litr3n_SUM * sitec->soillayer_thickness[layer] / sitec->soillayer_depth[PLGlayer-1];
			ns->litr4n[layer]  = litr4n_SUM * sitec->soillayer_thickness[layer] / sitec->soillayer_depth[PLGlayer-1];
			cs->soil1c[layer]  = soil1c_SUM * sitec->soillayer_thickness[layer] / sitec->soillayer_depth[PLGlayer-1];
			cs->soil2c[layer]  = soil2c_SUM * sitec->soillayer_thickness[layer] / sitec->soillayer_depth[PLGlayer-1];
			cs->soil3c[layer]  = soil3c_SUM * sitec->soillayer_thickness[layer] / sitec->soillayer_depth[PLGlayer-1];
			cs->soil4c[layer]  = soil4c_SUM * sitec->soillayer_thickness[layer] / sitec->soillayer_depth[PLGlayer-1];
			ns->soil1n[layer]  = soil1n_SUM * sitec->soillayer_thickness[layer] / sitec->soillayer_depth[PLGlayer-1];
			ns->soil2n[layer]  = soil2n_SUM * sitec->soillayer_thickness[layer] / sitec->soillayer_depth[PLGlayer-1];
			ns->soil3n[layer]  = soil3n_SUM * sitec->soillayer_thickness[layer] / sitec->soillayer_depth[PLGlayer-1];
			ns->soil4n[layer]  = soil4n_SUM * sitec->soillayer_thickness[layer] / sitec->soillayer_depth[PLGlayer-1]; 
		}

		/* update TSOIL values */
		metv->tsoil_surface_pre = metv->tsoil[0];
		metv->tsoil_surface     = metv->tsoil[0];


		/* estimating aboveground litter and cwdc */
		cs->litrc_above = 0;



		/**********************************************************************************************/
		/* III. CALCULATING FLUXES */
	
	
		/* 1. leaf, froot, fruit, sofstem, gresp*/

		if (epc->leaf_cn)
		{
			cf->leafc_to_PLG				= cs->leafc * PLGcoeff;
			cf->leafc_storage_to_PLG		= cs->leafc_storage * PLGcoeff;
			cf->leafc_transfer_to_PLG		= cs->leafc_transfer * PLGcoeff;
		
			nf->leafn_to_PLG				= ns->leafn * PLGcoeff;
			nf->leafn_storage_to_PLG		= ns->leafn_storage * PLGcoeff;
			nf->leafn_transfer_to_PLG		= ns->leafn_transfer * PLGcoeff;
		}

		if (epc->froot_cn)
		{
			cf->frootc_to_PLG				= cs->frootc * PLGcoeff;
			cf->frootc_storage_to_PLG		= cs->frootc_storage * PLGcoeff;
			cf->frootc_transfer_to_PLG		= cs->frootc_transfer * PLGcoeff;

			nf->frootn_to_PLG				= ns->frootn * PLGcoeff;
			nf->frootn_storage_to_PLG		= ns->frootn_storage * PLGcoeff;
			nf->frootn_transfer_to_PLG		= ns->frootn_transfer * PLGcoeff;
		}

		if (epc->fruit_cn)
		{
			cf->fruitc_to_PLG				= cs->fruitc * PLGcoeff;
			cf->fruitc_storage_to_PLG		= cs->fruitc_storage * PLGcoeff;
			cf->fruitc_transfer_to_PLG		= cs->fruitc_transfer * PLGcoeff;

			nf->fruitn_to_PLG			    = ns->fruitn * PLGcoeff;
			nf->fruitn_storage_to_PLG		= ns->fruitn_storage * PLGcoeff;
			nf->fruitn_transfer_to_PLG		= ns->fruitn_transfer * PLGcoeff;
		}

		if (epc->softstem_cn)
		{
			cf->softstemc_to_PLG				= cs->softstemc * PLGcoeff;
			cf->softstemc_storage_to_PLG		= cs->softstemc_storage * PLGcoeff;
			cf->softstemc_transfer_to_PLG		= cs->softstemc_transfer * PLGcoeff;

			nf->softstemn_to_PLG				= ns->softstemn * PLGcoeff;
			nf->softstemn_storage_to_PLG		= ns->softstemn_storage * PLGcoeff;
			nf->softstemn_transfer_to_PLG		= ns->softstemn_transfer * PLGcoeff;
		}

		cf->gresp_storage_to_PLG		= cs->gresp_storage * PLGcoeff;
		cf->gresp_transfer_to_PLG		= cs->gresp_transfer * PLGcoeff;

		nf->retransn_to_PLG              = ns->retransn * PLGcoeff;

		/* 2. standing dead biome to cut-down belowground materail: PLGcoeff part of aboveground and whole belowground */

		cf->STDBc_leaf_to_PLG	         = cs->STDBc_leaf     * PLGcoeff;
		cf->STDBc_froot_to_PLG	         = cs->STDBc_froot    * PLGcoeff;
		cf->STDBc_fruit_to_PLG	         = cs->STDBc_fruit    * PLGcoeff;
		cf->STDBc_softstem_to_PLG        = cs->STDBc_softstem * PLGcoeff;
		cf->STDBc_nsc_to_PLG             = cs->STDBc_nsc      * PLGcoeff;

		nf->STDBn_leaf_to_PLG	         = ns->STDBn_leaf     * PLGcoeff;
		nf->STDBn_froot_to_PLG	         = ns->STDBn_froot    * PLGcoeff;
		nf->STDBn_fruit_to_PLG	         = ns->STDBn_fruit    * PLGcoeff;
		nf->STDBn_softstem_to_PLG        = ns->STDBn_softstem * PLGcoeff;
		nf->STDBn_nsc_to_PLG             = ns->STDBn_nsc * PLGcoeff;

		 /* 3. cut-down dead biome: abovegound to belowground  */
		cf->CTDBc_leaf_to_PLG	         = cs->CTDBc_leaf     * PLGcoeff;
		cf->CTDBc_fruit_to_PLG	         = cs->CTDBc_fruit    * PLGcoeff;
		cf->CTDBc_softstem_to_PLG        = cs->CTDBc_softstem * PLGcoeff;

		nf->CTDBn_leaf_to_PLG	         = ns->CTDBn_leaf     * PLGcoeff;
		nf->CTDBn_fruit_to_PLG	         = ns->CTDBn_fruit    * PLGcoeff;
		nf->CTDBn_softstem_to_PLG        = ns->CTDBn_softstem * PLGcoeff;

		/* 4. WATER */ 
		wf->canopyw_to_PLG              = ws->canopyw * PLGcoeff;


		/**********************************************************************************************/
		/* III. STATE UPDATE */

		/* 1. OUT */
		/* 1.1. leaf, froot, fruit, sofstem, gresp*/
		cs->leafc				-= cf->leafc_to_PLG;
		cs->leafc_transfer		-= cf->leafc_transfer_to_PLG;
		cs->leafc_storage		-= cf->leafc_storage_to_PLG;
		cs->gresp_transfer		-= cf->gresp_transfer_to_PLG;
		cs->gresp_storage		-= cf->gresp_storage_to_PLG;
		cs->frootc				-= cf->frootc_to_PLG;
		cs->frootc_transfer		-= cf->frootc_transfer_to_PLG;
		cs->frootc_storage		-= cf->frootc_storage_to_PLG;
		cs->fruitc				-= cf->fruitc_to_PLG;
		cs->fruitc_transfer		-= cf->fruitc_transfer_to_PLG;
		cs->fruitc_storage		-= cf->fruitc_storage_to_PLG;
		cs->softstemc			-= cf->softstemc_to_PLG;
		cs->softstemc_transfer  -= cf->softstemc_transfer_to_PLG;
		cs->softstemc_storage   -= cf->softstemc_storage_to_PLG;

		ns->leafn				-= nf->leafn_to_PLG;
		ns->leafn_transfer		-= nf->leafn_transfer_to_PLG;
		ns->leafn_storage		-= nf->leafn_storage_to_PLG;
		ns->frootn				-= nf->frootn_to_PLG;
		ns->frootn_transfer		-= nf->frootn_transfer_to_PLG;
		ns->frootn_storage		-= nf->frootn_storage_to_PLG;
		ns->fruitn				-= nf->fruitn_to_PLG;
		ns->fruitn_transfer		-= nf->fruitn_transfer_to_PLG;
		ns->fruitn_storage		-= nf->fruitn_storage_to_PLG;
		ns->softstemn			-= nf->softstemn_to_PLG;
		ns->softstemn_transfer  -= nf->softstemn_transfer_to_PLG;
		ns->softstemn_storage	-= nf->softstemn_storage_to_PLG;
		ns->retransn			-= nf->retransn_to_PLG;
   

		/* 1.2. standing dead biome */
		cs->STDBc_leaf     -= cf->STDBc_leaf_to_PLG;
		cs->STDBc_froot    -= cf->STDBc_froot_to_PLG;
		cs->STDBc_fruit    -= cf->STDBc_fruit_to_PLG;
		cs->STDBc_softstem -= cf->STDBc_softstem_to_PLG;
		cs->STDBc_nsc      -= cf->STDBc_nsc_to_PLG;

		ns->STDBn_leaf     -= nf->STDBn_leaf_to_PLG;
		ns->STDBn_froot    -= nf->STDBn_froot_to_PLG;
		ns->STDBn_fruit    -= nf->STDBn_fruit_to_PLG;
		ns->STDBn_softstem -= nf->STDBn_softstem_to_PLG;
		ns->STDBn_nsc      -= nf->STDBn_nsc_to_PLG;

		/* 1.3. cut-down dead biome: aboveground to belowground */
		cs->CTDBc_leaf     -= cf->CTDBc_leaf_to_PLG;
		cs->CTDBc_fruit    -= cf->CTDBc_fruit_to_PLG;
		cs->CTDBc_softstem -= cf->CTDBc_softstem_to_PLG;

		ns->CTDBn_leaf     -= nf->CTDBn_leaf_to_PLG;
		ns->CTDBn_fruit    -= nf->CTDBn_fruit_to_PLG;
		ns->CTDBn_softstem -= nf->CTDBn_softstem_to_PLG;
	
	
		/* 1.4 water*/
		ws->canopyw        -= wf->canopyw_to_PLG;

		/*--------------------------------------------------------------------*/
		/* 2. IN: fluxes to belowground cut-down biomass */
	 
		cs->CTDBc_froot += cf->leafc_to_PLG     + cf->STDBc_leaf_to_PLG     + cf->CTDBc_leaf_to_PLG +    
						   cf->fruitc_to_PLG    + cf->STDBc_fruit_to_PLG    + cf->CTDBc_fruit_to_PLG +   
						   cf->softstemc_to_PLG + cf->STDBc_softstem_to_PLG + cf->CTDBc_softstem_to_PLG +
						   cf->frootc_to_PLG    + cf->STDBc_froot_to_PLG;

		ns->CTDBn_froot += nf->leafn_to_PLG     + nf->STDBn_leaf_to_PLG     + nf->CTDBn_leaf_to_PLG + 
						      nf->fruitn_to_PLG    + nf->STDBn_fruit_to_PLG    + nf->CTDBn_fruit_to_PLG + 
						      nf->softstemn_to_PLG + nf->STDBn_softstem_to_PLG + nf->CTDBn_softstem_to_PLG +
							  nf->frootn_to_PLG    + nf->STDBn_froot_to_PLG;

		cs->CTDBc_nsc   += cf->leafc_storage_to_PLG     + cf->leafc_transfer_to_PLG  + 
			                  cf->frootc_storage_to_PLG    + cf->frootc_transfer_to_PLG +
						      cf->fruitc_storage_to_PLG    + cf->fruitc_transfer_to_PLG + 
							  cf->softstemc_storage_to_PLG + cf->softstemc_transfer_to_PLG +
						      cf->gresp_storage_to_PLG     + cf->gresp_transfer_to_PLG +
							  cf->STDBc_nsc_to_PLG;


		ns->CTDBn_nsc   += nf->leafn_storage_to_PLG     + nf->leafn_transfer_to_PLG  + 
			                  nf->frootn_storage_to_PLG    + nf->frootn_transfer_to_PLG +
						      nf->fruitn_storage_to_PLG    + nf->fruitn_transfer_to_PLG + 
							  nf->softstemn_storage_to_PLG + nf->softstemn_transfer_to_PLG +
						      nf->retransn_to_PLG +
							  nf->STDBn_nsc_to_PLG;
	}

   return (errorCode);
}
	