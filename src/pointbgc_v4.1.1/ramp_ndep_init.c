/* 
ramp_ndep_init.c
Initialize the ramped N deposition parameters for bgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.1.1
Copyright 2000, Peter E. Thornton
Numerical Terradynamics Simulation Group (NTSG)
School of Forestry, University of Montana
Missoula, MT 59812
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "ini.h"
#include "bgc_struct.h"
#include "pointbgc_struct.h"
#include "pointbgc_func.h"

int ramp_ndep_init(file init, ramp_ndep_struct* ramp_ndep)
{
	int ok = 1;
	char key1[] = "RAMP_NDEP";
	char keyword[80];

	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** RAMP_NDEP                                                     ** 
	**                                                                 **
	********************************************************************/

	/* scan for the climate change block keyword, exit if not next */
	if (ok && scan_value(init, keyword, 's'))
	{
		printf("Error reading keyword, ramp_ndep_init()\n");
		ok=0;
	}
	if (ok && strcmp(keyword,key1))
	{
		printf("Expecting keyword --> %s in %s\n",key1,init.name);
		ok=0;
	}

	/* begin reading ramp_ndep information */
	if (ok && scan_value(init, &ramp_ndep->doramp, 'i'))
	{
		printf("Error reading ramp Ndep flag: ramp_ndep_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &ramp_ndep->ind_year, 'i'))
	{
		printf("Error reading year of reference for industrial Ndep: ramp_ndep_init()\n");
		ok=0;
	}
    if (scan_value(init,&ramp_ndep->ind_ndep,'d')) 
	{
		printf("Error reading industrial Ndep value: ramp_ndep_init()\n");
		ok=0;
	}
	
	return (!ok);
}
