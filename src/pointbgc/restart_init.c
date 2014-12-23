/* 
restart_init.c
Initialize the simulation restart parameters

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "pointbgc.h"

int restart_init(file init, restart_ctrl_struct* restart)
{
	int ok = 1;
	char key1[] = "RESTART";
	char keyword[256];
	char junk[256];
	extern signed char cli_mode;


	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** RESTART                                                         ** 
	**                                                                 **
	********************************************************************/
	
	/* scan for the restart block keyword, exit if not next */
	if (ok && scan_value(init, keyword, 's'))
	{
		bgc_printf(BV_ERROR, "Error reading keyword for restart data\n");
		ok=0;
	}
	if (ok && strcmp(keyword, key1))
	{
		bgc_printf(BV_ERROR, "Expecting keyword --> %s in file %s\n",key1,init.name);
		ok=0;
	}
	
	/* check for input restart file */
	if (ok && scan_value(init, &restart->read_restart, 'i'))
	{
		bgc_printf(BV_ERROR, "Error reading input restart flag\n");
		ok=0;
	}
	
	if (cli_mode == MODE_SPINUP || cli_mode == MODE_SPINNGO)
		restart->read_restart = 0;
	if (cli_mode == MODE_MODEL)
		restart->read_restart = 1;
		
	/* check for output restart file */
	if (ok && scan_value(init, &restart->write_restart, 'i'))
	{
		bgc_printf(BV_ERROR, "Error reading output restart flag\n");
		ok=0;
	}
	
	if (cli_mode == MODE_SPINUP)
		restart->write_restart = 1;
	if (cli_mode == MODE_MODEL || cli_mode == MODE_SPINNGO)
		restart->write_restart = 0;
		
	/* flag for metyear handling */
	if (ok && scan_value(init, &restart->keep_metyr, 'i'))
	{
		bgc_printf(BV_ERROR, "Error reading keep_metyr flag\n");
		ok=0;
	}
	/* if using an input restart file, open it, otherwise discard the next line of the ini file */
	if (ok && restart->read_restart)
	{
    	if (scan_open(init,&(restart->in_restart),'r')) 
		{
			bgc_printf(BV_ERROR, "Error opening input restart file\n");
			ok=0;
		}
	}
	else
	{
		if (scan_value(init, junk, 's'))
		{
			bgc_printf(BV_ERROR, "Error scanning input restart filename\n");
			ok=0;
		}
	}
	/* if using an output restart file, open it, otherwise
	discard the next line of the ini file */
	if (ok && restart->write_restart)
	{
    	if (scan_open(init,&(restart->out_restart),'w')) 
		{
			bgc_printf(BV_ERROR, "Error opening output restart file\n");
			ok=0;
		}
	}
	else
	{
		if (scan_value(init, junk, 's'))
		{
			bgc_printf(BV_ERROR, "Error scanning output restart filename\n");
			ok=0;
		}
	}
	
	return (!ok);
}
