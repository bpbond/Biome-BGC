/* 
output_ctrl.c
Reads output control information from initialization file

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information

Revisions from version 4.1.2:
This file used to be output_init.c. The ini file scanning and output
file initialization has been split into two files. Also, spin-n-go
functionality has been added

Revisions from version 4.1.1:
Fixed error in ascii output file that incorrectly gave the 
units for annual precipitation as cm/year - the real units are mm/yr.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "pointbgc.h"

int output_ctrl(file init, output_struct* output)
{
	int ok = 1;
	int i;
	char key1[] = "OUTPUT_CONTROL";
	char key2[] = "DAILY_OUTPUT";
	char key3[] = "ANNUAL_OUTPUT";
	char keyword[80];
	extern signed char cli_mode;

	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** OUTPUT_CONTROL                                                  ** 
	**                                                                 **
	********************************************************************/
	
	/* scan for the output file block keyword, exit if not next */
	if (ok && scan_value(init, keyword, 's'))
	{
		bgc_printf(BV_ERROR, "Error reading keyword for output file data\n");
		ok=0;
	}
	if (ok && strcmp(keyword, key1))
	{
		bgc_printf(BV_ERROR, "Expecting keyword --> %s in file %s\n",key1,init.name);
		ok=0;
	}
	
	/* get the output filename prefix */
	if (ok && scan_value(init,output->outprefix,'s'))
	{
		bgc_printf(BV_ERROR, "Error reading outfile prefix: output_ctrl(), output_ctrl.c\n");
		ok=0;
	}
	/* scan flags for daily output */
	if (ok && scan_value(init, &output->dodaily, 'i'))
	{
		bgc_printf(BV_ERROR, "Error reading daily output flag: output_ctrl()\n");
		ok=0;
	}
	
	if (cli_mode == MODE_SPINUP || cli_mode == MODE_SPINNGO)
		output->dodaily = 0;
	if (cli_mode == MODE_MODEL)
		output->dodaily = 1;
	
	/* scan flag for monthly average output (operates on daily variables) */
	if (ok && scan_value(init, &output->domonavg, 'i'))
	{
		bgc_printf(BV_ERROR, "Error reading monthly average output flag: output_ctrl()\n");
		ok=0;
	}
	
	if (cli_mode == MODE_SPINUP || cli_mode == MODE_SPINNGO)
		output->domonavg = 0;
	if (cli_mode == MODE_MODEL)
		output->domonavg = 1;
		
	/* scan flag for annual average output (operates on daily variables) */
	if (ok && scan_value(init, &output->doannavg, 'i'))
	{
		bgc_printf(BV_ERROR, "Error reading annual average output flag: output_ctrl()\n");
		ok=0;
	}
	
	if (cli_mode == MODE_SPINUP || cli_mode == MODE_SPINNGO)
		output->doannavg = 0;
	if (cli_mode == MODE_MODEL)
		output->doannavg = 1;
	
	/* scan flag for annual output */
	if (ok && scan_value(init, &output->doannual, 'i'))
	{
		bgc_printf(BV_ERROR, "Error reading annual output flag: output_ctrl()\n");
		ok=0;
	}
	
	if (cli_mode == MODE_SPINUP || cli_mode == MODE_SPINNGO)
		output->doannual = 0;
	if (cli_mode == MODE_MODEL)
		output->doannual = 1;
	
	/* scan flag for on-screen progress indicator */
	if (ok && scan_value(init, &output->onscreen, 'i'))
	{
		bgc_printf(BV_ERROR, "Error reading on-screen indicator flag: output_ctrl()\n");
		ok=0;
	}
	
	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** DAILY_OUTPUT                                                    ** 
	**                                                                 **
	********************************************************************/
	
	/* scan for the output file block keyword, exit if not next */
	if (ok && scan_value(init, keyword, 's'))
	{
		bgc_printf(BV_ERROR, "Error reading keyword for output file data\n");
		ok=0;
	}
	if (ok && strcmp(keyword, key2))
	{
		bgc_printf(BV_ERROR, "Expecting keyword --> %s in file %s\n",key2,init.name);
		ok=0;
	}
	
	/* read the number of daily output variables */
	if (ok && scan_value(init, &output->ndayout, 'i'))
	{
		bgc_printf(BV_ERROR, "Error reading number of daily outputs: output_ctrl()\n");
		ok=0;
	}
	/* allocate space for the daily output variable indices */
	if (ok && output->ndayout && !(output->daycodes = (int*) malloc(output->ndayout * sizeof(int))))
	{
		bgc_printf(BV_ERROR, "Error allocating for daycodes array: output_ctrl()\n");
		ok=0;
	}
	/* begin loop to read in the daily output variable indices */
	for (i=0 ; ok && i<output->ndayout ; i++)
	{
		if (ok && scan_value(init, &(output->daycodes[i]), 'i'))
		{
			bgc_printf(BV_ERROR, "Error reading daily output #%d: output_ctrl()\n",i);
			ok=0;
		}
	}
	
	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** ANNUAL_OUTPUT                                                   ** 
	**                                                                 **
	********************************************************************/
	
	/* scan for the output file block keyword, exit if not next */
	if (ok && scan_value(init, keyword, 's'))
	{
		bgc_printf(BV_ERROR, "Error reading keyword for output file data\n");
		ok=0;
	}
	if (ok && strcmp(keyword, key3))
	{
		bgc_printf(BV_ERROR, "Expecting keyword --> %s in file %s\n",key3,init.name);
		ok=0;
	}
	
	/* read the number of annual output variables */
	if (ok && scan_value(init, &output->nannout, 'i'))
	{
		bgc_printf(BV_ERROR, "Error reading number of annual outputs: output_ctrl()\n");
		ok=0;
	}
	
	if (output->nannout == 0 && output->ndayout == 0 && (cli_mode == MODE_MODEL || cli_mode == MODE_SPINNGO))
	{
		bgc_printf(BV_ERROR, "ERROR! You are trying to run the model with no output variables. Please add some output variables to your ini file.\n");
		ok=0;
	}
	/* allocate space for the annual output variable indices */
	if (ok && output->nannout && !(output->anncodes = (int*) malloc(output->nannout * sizeof(int))))
	{
		bgc_printf(BV_ERROR, "Error allocating for anncodes array: output_ctrl()\n");
		ok=0;
	}
	/* begin loop to read in the annual output variable indices */
	for (i=0 ; ok && i<output->nannout ; i++)
	{
		if (ok && scan_value(init, &(output->anncodes[i]), 'i'))
		{
			bgc_printf(BV_ERROR, "Error reading annual output #%d: output_ctrl()\n",i);
			ok=0;
		}
	}
	
	return (!ok);
}
