/* 
scc_init.c
Initialize the scalar climate change parameters for bgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
BBGC MuSo v4
Copyright 2000, Peter E. Thornton
Numerical Terradynamics Simulation Group
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
#include "bgc_struct.h"
#include "pointbgc_struct.h"
#include "pointbgc_func.h"

int scc_init(file init, climchange_struct* scc)
{
	int ok = 1;
	char key1[] = "CLIM_CHANGE";
	char keyword[80];

	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** CLIM_CHANGE                                                     ** 
	**                                                                 **
	********************************************************************/

	/* scan for the climate change block keyword, exit if not next */
	if (ok && scan_value(init, keyword, 's'))
	{
		printf("Error reading keyword, scc_init()\n");
		ok=0;
	}
	if (ok && strcmp(keyword,key1))
	{
		printf("Expecting keyword --> %s in %s\n",key1,init.name);
		ok=0;
	}

	/* begin reading climate change data */
	if (ok && scan_value(init, &scc->s_tmax, 'd'))
	{
		printf("Error reading scalar for tmax, scc_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &scc->s_tmin, 'd'))
	{
		printf("Error reading scalar for tmin, scc_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &scc->s_prcp, 'd'))
	{
		printf("Error reading scalar for prcp, scc_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &scc->s_vpd, 'd'))
	{
		printf("Error reading scalar for vpd, scc_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &scc->s_swavgfd, 'd'))
	{
		printf("Error reading scalar for swavgfd, scc_init()\n");
		ok=0;
	}
	
	/* some error checking on scalar climate change values */
	if (scc->s_prcp < 0.0)
	{
		printf("Error in scc_init(): prcp scalar must be positive\n");
		ok=0;
	}
	if (scc->s_vpd < 0.0)
	{
		printf("Error in scc_init(): vpd scalar must be positive\n");
		ok=0;
	}
	if (scc->s_swavgfd < 0.0)
	{
		printf("Error in scc_init(): swavgfd scalar must be positive\n");
		ok=0;
	}

	return (!ok);
}
