/* 
irrigation.c
irrigation  - irrigation seeds in soil - increase transfer pools

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

int irrigation(const control_struct* ctrl, const irrigation_struct* IRG, wflux_struct* wf)
{
	/* irrigation parameters */	   
	
	int ok=1;
	int ny;
	int mgmd = IRG->mgmd;

	/* yearly varied or constant management parameters */
	if(IRG->IRG_flag == 2)
	{
		ny = ctrl->simyr;
	}
	else ny=0;

	/*  CALCULATING FLUXES: amount of water (kgH2O) * utiliz_coeff (%) * (%_to_prop) */
	
	if (mgmd >=0)
	{
		/* control */
		if (IRG->IRGquantity_array[mgmd][ny] == DATA_GAP      || IRG->utiliz_coeff_array[mgmd][ny] == DATA_GAP)
		{
			printf("ERROR in irrigation parameters in INI or management file (irrigation.c)\n");
			ok=0;
		}
 		wf->IRG_to_prcp=IRG->IRGquantity_array[mgmd][ny] * (IRG->utiliz_coeff_array[mgmd][ny] / 100.);
	}
		
	else
		wf->IRG_to_prcp=0;
	

   return (!ok);
}
	