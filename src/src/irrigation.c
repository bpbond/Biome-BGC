/* 
irrigation.c
irrigation  - irrigation seeds in soil - increase transfer pools

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
BBGC MuSo v4
Copyright 2014, D. Hidy (dori.hidy@gmail.com)
Hungarian Academy of Sciences
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

int irrigation(const control_struct* ctrl, irrigation_struct* IRG, wstate_struct* ws, wflux_struct* wf)
{
	/* irrigation parameters Hidy 2015.*/	   
	
	int ok=1;
	int ny;
	int mgmd = IRG->mgmd;

	/* yearly varied or constant management parameters */
	if(IRG->IRG_flag == 2)
	{
		ny = ctrl->simyr;
	}
	else ny=0;

	/* ------ CALCULATING FLUXES ------*/
	
	if (mgmd >=0)
 		wf->IRG_to_prcp=IRG->IRGquantity_array[mgmd][ny];
		
	else
		wf->IRG_to_prcp=0;
	
	
	/* ------- STATE UPDATE ----------*/ 
	ws->IRGsrc += wf->IRG_to_prcp;


   return (!ok);
}
	