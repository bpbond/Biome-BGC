/* 
restart_init.c
Initialize the simulation restart parameters

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

int restart_init(file init, restart_ctrl_struct* restart)
{
	int ok = 1;
	char key1[] = "RESTART";
	char keyword[80];
	char junk[80];

	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** RESTART                                                         ** 
	**                                                                 **
	********************************************************************/
	
	/* scan for the restart block keyword, exit if not next */
	if (ok && scan_value(init, keyword, 's'))
	{
		printf("Error reading keyword for restart data\n");
		ok=0;
	}
	if (ok && strcmp(keyword, key1))
	{
		printf("Expecting keyword --> %s in file %s\n",key1,init.name);
		ok=0;
	}
	
	/* check for input restart file */
	if (ok && scan_value(init, &restart->read_restart, 'i'))
	{
		printf("Error reading input restart flag\n");
		ok=0;
	}
	/* check for output restart file */
	if (ok && scan_value(init, &restart->write_restart, 'i'))
	{
		printf("Error reading output restart flag\n");
		ok=0;
	}
	/* flag for metyear handling */
	if (ok && scan_value(init, &restart->keep_metyr, 'i'))
	{
		printf("Error reading keep_metyr flag\n");
		ok=0;
	}
	/* if using an input restart file, open it, otherwise
	discard the next line of the ini file */
	if (ok && restart->read_restart)
	{
    	if (scan_open(init,&(restart->in_restart),'r')) 
		{
			printf("Error opening input restart file\n");
			ok=0;
		}
	}
	else
	{
		if (scan_value(init, junk, 's'))
		{
			printf("Error scanning input restart filename\n");
			ok=0;
		}
	}
	/* if using an output restart file, open it, otherwise
	discard the next line of the ini file */
	if (ok && restart->write_restart)
	{
    	if (scan_open(init,&(restart->out_restart),'w')) 
		{
			printf("Error opening output restart file\n");
			ok=0;
		}
	}
	else
	{
		if (scan_value(init, junk, 's'))
		{
			printf("Error scanning output restart filename\n");
			ok=0;
		}
	}
	
	return (!ok);
}
