/* 
time_init.c
Initialize the simulation timing control parameters for bgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "pointbgc.h"

int time_init(file init, control_struct *ctrl)
{
	int ok = 1;
	char key1[] = "TIME_DEFINE";
	char keyword[80];
	extern signed char cli_mode;

	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** TIME_DEFINE                                                     ** 
	**                                                                 **
	********************************************************************/

	/* scan for the control block keyword, exit if not next */
	if (ok && scan_value(init, keyword, 's'))
	{
		bgc_printf(BV_ERROR, "Error reading keyword for control data\n");
		ok=0;
	}
	if (ok && strcmp(keyword, key1))
	{
		bgc_printf(BV_ERROR, "Expecting keyword --> %s in file %s\n",key1,init.name);
		ok=0;
	}

	/* read the number of years of data in met files */
	if (ok && scan_value(init, &ctrl->metyears, 'i'))
	{
		bgc_printf(BV_ERROR, "Error reading number of met years: time_init()\n");
		ok=0;
	}
	
	/* read the number of simulation years */
	if (ok && scan_value(init, &ctrl->simyears, 'i'))
	{
		bgc_printf(BV_ERROR, "Error reading simyears: time_init(), time_init.c\n");
		ok=0;
	}
	
	/* read integer value for the first year of the simulation */
	if (ok && scan_value(init, &ctrl->simstartyear, 'i'))
	{
		bgc_printf(BV_ERROR, "Error reading simstartyear: time_init(), time_init.c\n");
		ok=0;
	}
	
	/* read flag controling whether or not this is a spinup simulation */
	if (ok && scan_value(init, &ctrl->spinup, 'i'))
	{
		bgc_printf(BV_ERROR, "Error reading spinup flag: time_init(), time_init.c\n");
		ok=0;
	}
	
	if (cli_mode == MODE_MODEL)
		ctrl->spinup = 0;
	if (cli_mode == MODE_SPINUP || cli_mode == MODE_SPINNGO)
		ctrl->spinup = 1;
	
	/* read maximum allowable simulation years for spinup simulation */
	if (ok && scan_value(init, &ctrl->maxspinyears, 'i'))
	{
		bgc_printf(BV_ERROR, "Error reading max spinup years: time_init(), time_init.c\n");
		ok=0;
	}
	
	return (!ok);
}
