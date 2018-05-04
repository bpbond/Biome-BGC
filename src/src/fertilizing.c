/* 
fertilizing.c
do fertilization  - increase the mineral soil nitrogen (sminn)

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

int fertilizing(const control_struct* ctrl, const siteconst_struct* sitec, fertilizing_struct* FRZ, epvar_struct* epv, 
				cstate_struct* cs, nstate_struct* ns, wstate_struct* ws, cflux_struct* cf, nflux_struct* nf, wflux_struct* wf)
{


	/* fertilizing parameters .*/
	int ok=1;
	int ny, mgmd, layer;

	double FRZdepth;					    /* (m) actual  depth of fertilization */
	double fertilizer_DM;                   /* (kg/m2) dry matter content of fertilizer  */
	double fertilizer_WC;                   /* (kg/m2) water content of fertilizer  */
	double flab;                            /* (%) labile fraction of fertilizer */
	double fucel;                           /* (%) unshielded cellulose fraction of fertilizer */
	double fscel;                           /* (%) shielded cellulose fraction of fertilizer */
	double flig;                            /* (%) lignin fraction of fertilizer */
	double EFf_N2O;                         /* (kgN2O-N:kgN) fertilization emission factor for direct N2O emissions from synthetic fertililers */
	double FRZ_to_litrc, FRZ_to_litrn;		/* fertilizing carbon and nitrogen fluxes */
	int FRZlayer;                               /* (DIM) number of fertilization layer */
	
		
	double ratio, ratioSUM,ha_to_m2;

	FRZ_to_litrc=FRZ_to_litrn=ratio=ratioSUM=0;
	ha_to_m2 = 1./10000;

	/* yearly varied or constant management parameters */
	if(FRZ->FRZ_flag == 2)
	{
		ny = ctrl->simyr;
	}
	else ny=0;

	
	/* On management days fertilizer is put on the field */

	mgmd=FRZ->mgmd;

	if (mgmd >=0) 
	{
		/* 1. control */
		if (FRZ->FRZdepth_array[mgmd][ny] == DATA_GAP    || FRZ->fertilizer_array[mgmd][ny] == DATA_GAP || FRZ->DM_array[mgmd][ny] == DATA_GAP || 
			FRZ->NO3content_array[mgmd][ny] == DATA_GAP  || FRZ->NH4content_array[mgmd][ny] == DATA_GAP || FRZ->orgCcontent_array[mgmd][ny] == DATA_GAP ||
			FRZ->orgNcontent_array[mgmd][ny] == DATA_GAP || FRZ->litr_flab_array[mgmd][ny] == DATA_GAP  || FRZ->litr_fucel_array[mgmd][ny] == DATA_GAP || 
			FRZ->litr_fscel_array[mgmd][ny] == DATA_GAP  || FRZ->litr_flig_array[mgmd][ny] == DATA_GAP  || FRZ->EFfert_N2O[mgmd][ny] == DATA_GAP)
		{
				printf("ERROR in fertilizing parameters in INI or management file (fertilizing.c)\n");
				ok=0;
		}


		/* 2. input parameters in actual year from array */
		FRZdepth   = FRZ->FRZdepth_array[mgmd][ny]; 

		flab		= FRZ->litr_flab_array[mgmd][ny] / 100.;		/* from % to number */
		fucel		= FRZ->litr_fucel_array[mgmd][ny] / 100.;		/* from % to number */
		fscel		= FRZ->litr_fscel_array[mgmd][ny] / 100.;		/* from % to number */
		flig		= FRZ->litr_flig_array[mgmd][ny] / 100.;		/* from % to number */
		EFf_N2O	    = FRZ->EFfert_N2O[mgmd][ny] /NDAYS_OF_YEAR;

		/* DM and WC content of fertilizer: kg/m2 = kg fertilizer/ha * ha/m2 * (% to prop.) */
		fertilizer_DM = FRZ->fertilizer_array[mgmd][ny]  * ha_to_m2 * (FRZ->DM_array[mgmd][ny] / 100.);
		fertilizer_WC = FRZ->fertilizer_array[mgmd][ny]  * ha_to_m2 * (1 - FRZ->DM_array[mgmd][ny] / 100.);

		/* 3. on fertilizing day a fixed amount of ammonium/nitrate/organic nitrogen/organic carbon/water enters into the soil */

		
		nf->FRZ_to_sminNH4 = fertilizer_DM * FRZ->NH4content_array[mgmd][ny];
		
		nf->FRZ_to_sminNO3 = fertilizer_DM * FRZ->NO3content_array[mgmd][ny];
		
		FRZ_to_litrn = fertilizer_DM * FRZ->orgNcontent_array[mgmd][ny];
		nf->FRZ_to_litr1n  = FRZ_to_litrn * flab;
		nf->FRZ_to_litr2n  = FRZ_to_litrn * fucel;
		nf->FRZ_to_litr3n  = FRZ_to_litrn * fscel;
		nf->FRZ_to_litr4n  = FRZ_to_litrn * flig;	
	
		FRZ_to_litrc = fertilizer_DM * FRZ->orgCcontent_array[mgmd][ny];
		cf->FRZ_to_litr1c  = FRZ_to_litrc * flab;
		cf->FRZ_to_litr2c  = FRZ_to_litrc * fucel;
		cf->FRZ_to_litr3c  = FRZ_to_litrc * fscel;
		cf->FRZ_to_litr4c  = FRZ_to_litrc * flig;	

		wf->FRZ_to_soilw   = fertilizer_WC;
		
	
		/* 4. fertilizing layer from depth */
		layer = 1;
		FRZlayer = 0;
		if (FRZdepth > sitec->soillayer_depth[0])
		{
			while (FRZlayer == 0 && layer < N_SOILLAYERS)
			{
				if ((FRZdepth > sitec->soillayer_depth[layer-1]) && (FRZdepth <= sitec->soillayer_depth[layer])) FRZlayer = layer;
				layer += 1;
			}
			if (FRZlayer == 0)
			{
				printf("Error in fertilizing depth calculation (fertilizing.c)\n");
				ok=0;
			}
		}


	

		/* 5. N2O emissions (kgN2O-N:kgN) */ 
		nf->N2O_flux_FRZ   = (nf->FRZ_to_sminNH4 + nf->FRZ_to_sminNO3 + FRZ_to_litrn) * EFf_N2O;

	

	    /* 6. STATE UPDATE */
		for (layer = 0; layer <= FRZlayer; layer++)
		{
			if (FRZlayer == 0)
				ratio = 1;
			else	
			{
				if (sitec->soillayer_depth[layer] < FRZdepth)
					ratio = sitec->soillayer_thickness[layer] / FRZdepth;
				else
					ratio = (FRZdepth - sitec->soillayer_depth[layer-1]) / FRZdepth;
			}

			ratioSUM += ratio;

			cs->litr1c[layer] += cf->FRZ_to_litr1c * ratio;
			cs->litr2c[layer] += cf->FRZ_to_litr2c * ratio;
			cs->litr3c[layer] += cf->FRZ_to_litr3c * ratio;
			cs->litr4c[layer] += cf->FRZ_to_litr4c * ratio;

			ns->litr1n[layer] += nf->FRZ_to_litr1n * ratio;
			ns->litr2n[layer] += nf->FRZ_to_litr2n * ratio;
			ns->litr3n[layer] += nf->FRZ_to_litr3n * ratio;
			ns->litr4n[layer] += nf->FRZ_to_litr4n * ratio;

			ns->sminNH4[layer]  += nf->FRZ_to_sminNH4 * ratio;
			ns->sminNO3[layer]  += nf->FRZ_to_sminNO3 * ratio;

			ws->soilw[layer]  += wf->FRZ_to_soilw * ratio;
			epv->vwc[layer]   = ws->soilw[layer] / (water_density * sitec->soillayer_thickness[layer]);
		}

		/* control */
		if (fabs(ratioSUM - 1) > CRIT_PREC)
		{
			printf("Error in fertilizing ratio calculation (fertilizing.c)\n");
			ok=0;
		}
		cs->FRZsrc += FRZ_to_litrc;
		ns->FRZsrc += nf->FRZ_to_sminNH4 + nf->FRZ_to_sminNO3 + FRZ_to_litrn;
		ws->FRZsrc += wf->FRZ_to_soilw;
		

	} /* endif mgmd */


   return (!ok);
}
	