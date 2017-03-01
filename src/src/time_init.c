/* 
time_init.c
Initialize the simulation timing control parameters for bgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.4
Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group (NTSG)
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

int time_init(file init, control_struct *ctrl)
{
	int ok = 1;
	char key1[] = "TIME_DEFINE";
	char keyword[80];

	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** TIME_DEFINE                                                     ** 
	**                                                                 **
	********************************************************************/

	/* scan for the control block keyword, exit if not next */
	if (ok && scan_value(init, keyword, 's'))
	{
		printf("Error reading keyword for control data\n");
		ok=0;
	}
	if (ok && strcmp(keyword, key1))
	{
		printf("Expecting keyword --> %s in file %s\n",key1,init.name);
		ok=0;
	}

	/* read the number of years of data in met files */
	if (ok && scan_value(init, &ctrl->metyears, 'i'))
	{
		printf("Error reading number of met years: time_init()\n");
		ok=0;
	}
	
	/* read the number of simulation years */
	if (ok && scan_value(init, &ctrl->simyears, 'i'))
	{
		printf("Error reading simyears: time_init(), time_init.c\n");
		ok=0;
	}
	
	/* read integer value for the first year of the simulation */
	if (ok && scan_value(init, &ctrl->simstartyear, 'i'))
	{
		printf("Error reading simstartyear: time_init(), time_init.c\n");
		ok=0;
	}
	
	/* read flag controling whether or not this is a spinup simulation */
	if (ok && scan_value(init, &ctrl->spinup, 'i'))
	{
		printf("Error reading spinup flag: time_init(), time_init.c\n");
		ok=0;
	}
	
	/* read maximum allowable simulation years for spinup simulation */
	if (ok && scan_value(init, &ctrl->maxspinyears, 'i'))
	{
		printf("Error reading max spinup years: time_init(), time_init.c\n");
		ok=0;
	}

	
	return (!ok);
}
