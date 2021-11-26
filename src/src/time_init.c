/* 
time_init.c
Initialize the simulation timing control parameters for bgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
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
	int errorCode=0;
	char key1[] = "TIME_DEFINE";
	char keyword[STRINGSIZE];

	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** TIME_DEFINE                                                     ** 
	**                                                                 **
	********************************************************************/

	/* scan for the control block keyword, exit if not next */
	if (!errorCode && scan_value(init, keyword, 's'))
	{
		printf("ERROR reading keyword for control data\n");
		errorCode=204;
	}
	if (!errorCode && strcmp(keyword, key1))
	{
		printf("Expecting keyword --> %s in file %s\n",key1,init.name);
		errorCode=204;
	}

	
	/* read the number of simulation years */
	if (!errorCode && scan_value(init, &ctrl->simyears, 'i'))
	{
		printf("ERROR reading simyears: time_init.c\n");
		errorCode=20402;
	}

	
	/* read integer value for the first year of the simulation */
	if (!errorCode && scan_value(init, &ctrl->simstartyear, 'i'))
	{
		printf("ERROR reading simstartyear: time_init.c\n");
		errorCode=20403;
	}
	
	/* read flag controling whether or not this is a spinup simulation */
	if (!errorCode && scan_value(init, &ctrl->spinup, 'i'))
	{
		printf("ERROR reading spinup flag: time_init.c\n");
		errorCode=20404;
	}
	
	/* read maximum allowable simulation cycles for spinup simulation */
	if (!errorCode && scan_value(init, &ctrl->maxspincycles, 'i'))
	{
		printf("ERROR reading max spinup cycles: time_init.c\n");
		errorCode=20405;
	}

	
	return (errorCode);
}
