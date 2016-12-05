/* 
fertilizing.c
do fertilization  - increase the mineral soil nitrogen (sminn)

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.2
Copyright 2016, D. Hidy [dori.hidy@gmail.com]
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

int fertilizing(const control_struct* ctrl, fertilizing_struct* FRZ, 
				cstate_struct* cs, nstate_struct*ns, cflux_struct* cf, nflux_struct* nf)
{

	/* fertilizing parameters .*/
	int ok=1;
	int ny, mgmd;

	double FRZ_to_sminn_act,FRZ_to_litrn_act,FRZ_to_litrc_act, N2Oflux_fertil;
	
	N2Oflux_fertil=FRZ_to_sminn_act=FRZ_to_litrn_act=FRZ_to_litrc_act=0;

	/* yearly varied or constant management parameters */
	if(FRZ->FRZ_flag == 2)
	{
		ny = ctrl->simyr;
	}
	else ny=0;

	
	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                                    CALCULATING FLUXES 
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
	
	/* on management days fertilizer is put on the field */

	mgmd=FRZ->mgmd;

	if (mgmd >=0) 
	{
	

		FRZ->FRZ_pool_act += FRZ->fertilizer_array[mgmd][ny]  / 10000.;		/* kgN/ha -> kgN/m2 */

		FRZ->DC_act = FRZ->dissolv_coeff_array[mgmd][ny]; 
		FRZ->UC_act         = FRZ->utiliz_coeff_array[mgmd][ny] /100;		/* from % to number */
		FRZ->NH3content_act = FRZ->NH3content_array[mgmd][ny] / 100.;		/* from % to number */
		FRZ->Ccontent_act   = FRZ->Ccontent_array[mgmd][ny] / 100.;			/* from % to number */
		FRZ->Ncontent_act   = FRZ->Ncontent_array[mgmd][ny] / 100.;			/* from % to number */

		FRZ->flab_act    = FRZ->litr_flab_array[mgmd][ny] / 100.;			/* from % to number */
		FRZ->fucel_act   = FRZ->litr_fucel_array[mgmd][ny] / 100.;		/* from % to number */
		FRZ->fscel_act   = FRZ->litr_fscel_array[mgmd][ny] / 100.;		/* from % to number */
		FRZ->flig_act    = FRZ->litr_flig_array[mgmd][ny] / 100.;			/* from % to number */
		FRZ->EFf_N2O_act = FRZ->EFfert_N2O[mgmd][ny] /NDAY_OF_YEAR;
		
	}

	/* on and after fertilizing day, ammonium and nitrate enters into the soil */			
	if (FRZ->FRZ_pool_act> 0.0)
	{
		/* not all the amount of the nitrogen from fertilization is available on the given fertilization day to plants ->
		dissolv_coeff define the ratio */

		/* nitrate content of fertilizer can be uptaken by plant directly -> get to the sminn pool */
		FRZ_to_sminn_act = FRZ->DC_act * FRZ->FRZ_pool_act * FRZ->Ncontent_act;
		/* ammonium content of fertilizer have to be nitrificatied before be uptaken by plant  -> get to the litrn pool */
		FRZ_to_litrn_act = FRZ->DC_act * FRZ->FRZ_pool_act * FRZ->NH3content_act;
		/* carbon content of fertilizer turn to the litter pool */
		FRZ_to_litrc_act = FRZ->DC_act * FRZ->FRZ_pool_act * FRZ->Ccontent_act;
		
		FRZ->FRZ_pool_act = FRZ->FRZ_pool_act - FRZ->FRZ_pool_act * FRZ->DC_act;
	

		/* if N from fertilization is available (in FRZ_pool_act) a given ratio of its N content (defined by useful part)
			get into the soil mineral nitrogen pool, the rest disappers from the system (slipping away...) */
		if (FRZ_to_sminn_act > CRIT_PREC)
		{
			nf->FRZ_to_sminn = FRZ_to_sminn_act * FRZ->UC_act;
			
			nf->FRZ_to_litr1n = FRZ_to_litrn_act * FRZ->flab_act  * FRZ->UC_act;
			nf->FRZ_to_litr2n = FRZ_to_litrn_act * FRZ->fucel_act * FRZ->UC_act;
			nf->FRZ_to_litr3n = FRZ_to_litrn_act * FRZ->fscel_act * FRZ->UC_act;
			nf->FRZ_to_litr4n = FRZ_to_litrn_act * FRZ->flig_act  * FRZ->UC_act;

			cf->FRZ_to_litr1c = FRZ_to_litrc_act * FRZ->flab_act  * FRZ->UC_act;
			cf->FRZ_to_litr2c = FRZ_to_litrc_act * FRZ->fucel_act * FRZ->UC_act;
			cf->FRZ_to_litr3c = FRZ_to_litrc_act * FRZ->fscel_act * FRZ->UC_act;
			cf->FRZ_to_litr4c = FRZ_to_litrc_act * FRZ->flig_act  * FRZ->UC_act;	
			
		}

		/* if the nitrogen from fertilization has been consumed already, the fertilization has no more effect .*/ 
		else
		{
			nf->FRZ_to_sminn = 0;
			
			nf->FRZ_to_litr1n = 0;
			nf->FRZ_to_litr2n = 0;
			nf->FRZ_to_litr3n = 0;
			nf->FRZ_to_litr4n = 0;

			cf->FRZ_to_litr1c = 0;
			cf->FRZ_to_litr2c = 0;
			cf->FRZ_to_litr3c = 0;
			cf->FRZ_to_litr4c = 0;	
			
		}
	}
	else
	{
		nf->FRZ_to_sminn = 0;
			
		nf->FRZ_to_litr1n = 0;
		nf->FRZ_to_litr2n = 0;
		nf->FRZ_to_litr3n = 0;
		nf->FRZ_to_litr4n = 0;

		cf->FRZ_to_litr1c = 0;
		cf->FRZ_to_litr2c = 0;
		cf->FRZ_to_litr3c = 0;
		cf->FRZ_to_litr4c = 0;		
	}

	/* !!!!!!!! N2O emissions (kgN to mgN)!!!!!!!!!!*/ 
	N2Oflux_fertil   = FRZ->FRZ_pool_act * 1e+6 * FRZ->EFf_N2O_act;
	nf->N2O_flux_FRZ = N2Oflux_fertil;
	

	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                                    STATE UPDATE 
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/ 

	/* 1. carbon */	
	cs->litr1c += cf->FRZ_to_litr1c;
	cs->litr2c += cf->FRZ_to_litr2c;
	cs->litr3c += cf->FRZ_to_litr3c;
	cs->litr4c += cf->FRZ_to_litr4c;
	
	cs->FRZsrc += cf->FRZ_to_litr1c + cf->FRZ_to_litr2c + cf->FRZ_to_litr3c + cf->FRZ_to_litr4c;

	/* 2. nitrogen */
	ns->litr1n += nf->FRZ_to_litr1n;
	ns->litr2n += nf->FRZ_to_litr2n;
	ns->litr3n += nf->FRZ_to_litr3n;
	ns->litr4n += nf->FRZ_to_litr4n;

	ns->sminn[0]	  += nf->FRZ_to_sminn;
	
	ns->FRZsrc += nf->FRZ_to_sminn + nf->FRZ_to_litr1n + nf->FRZ_to_litr2n + nf->FRZ_to_litr3n + nf->FRZ_to_litr4n;


   return (!ok);
}
	