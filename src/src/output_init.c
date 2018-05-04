/* 
output_init.c
Reads output control information from initialization file

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2018, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Modified:
4/17/2000 (PET): Added annual total precipitation and annual average air
temperature to the formatted annual ascii output file.
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

int output_init(file init, output_struct* output)
{
	int ok = 1;
	int i;
	char key1[] = "OUTPUT_CONTROL";
	char key2[] = "DAILY_OUTPUT";
	char key3[] = "ANNUAL_OUTPUT";
	char keyword[80];


	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** OUTPUT_CONTROL                                                  ** 
	**                                                                 **
	********************************************************************/
	
	/* scan for the output file block keyword, exit if not next */
	if (ok && scan_value(init, keyword, 's'))
	{
		printf("Error reading keyword for output file data\n");
		ok=0;
	}
	if (ok && strcmp(keyword, key1))
	{
		printf("Expecting keyword --> %s in file %s\n",key1,init.name);
		ok=0;
	}
	
	/* get the output filename prefix */
	if (ok && scan_value(init,output->outprefix,'s'))
	{
		printf("Error reading outfile prefix: output_init(), output_init.c\n");
		ok=0;
	}
	
	/* scan flags for daily output */
	if (ok && scan_value(init, &output->dodaily, 'i'))
	{
		printf("Error reading daily output flag: output_init()\n");
		ok=0;
	}
	/* scan flag for monthly average output (operates on daily variables) */
	if (ok && scan_value(init, &output->domonavg, 'i'))
	{
		printf("Error reading monthly average output flag: output_init()\n");
		ok=0;
	}
	/* scan flag for annual average output (operates on daily variables) */
	if (ok && scan_value(init, &output->doannavg, 'i'))
	{
		printf("Error reading annual average output flag: output_init()\n");
		ok=0;
	}
	/* scan flag for annual output */
	if (ok && scan_value(init, &output->doannual, 'i'))
	{
		printf("Error reading annual output flag: output_init()\n");
		ok=0;
	}
	/* scan flag for on-screen progress indicator */
	if (ok && scan_value(init, &output->onscreen, 'i'))
	{
		printf("Error reading on-screen indicator flag: output_init()\n");
		ok=0;
	}
	

	/* open outfiles if specified */
	strcpy(output->log_file.name,output->outprefix);
	strcat(output->log_file.name,".log");
	if (file_open(&(output->log_file),'o'))
	{
		printf("Error opening log_file (%s) in output_init()\n",output->log_file.name);
		ok=0;
	}


	/* open outfiles if specified */
	if (ok && output->dodaily)
	{
		strcpy(output->dayout.name,output->outprefix);
		strcat(output->dayout.name,".dayout");
		
		/* flag = 1 -> binary; flag = 2 -> ascii */
		if (output->dodaily == 1)
		{
			if (file_open(&(output->dayout),'w'))
			{
				printf("Error opening daily outfile (%s) in output_init()\n",output->dayout.name);
				ok=0;
			}
		}
		else
		{
			if (file_open(&(output->dayout),'o'))
			{
				printf("Error opening daily outfile (%s) in output_init()\n",output->dayout.name);
				ok=0;
			}
		}
	}
	if (ok && output->domonavg)
	{
		strcpy(output->monavgout.name,output->outprefix);
		strcat(output->monavgout.name,".monavgout");

		/* flag = 1 -> binary; flag = 2 -> ascii */
		if (output->domonavg == 1)
		{
			if (file_open(&(output->monavgout),'w'))
			{
				printf("Error opening monthly average outfile (%s) in output_init()\n",output->monavgout.name);
				ok=0;
			}
		}
		else
		{
			if (file_open(&(output->monavgout),'o'))
			{
				printf("Error opening monthly average outfile (%s) in output_init()\n",output->monavgout.name);
				ok=0;
			}
		}
	}
	if (ok && output->doannavg)
	{
		strcpy(output->annavgout.name,output->outprefix);
		strcat(output->annavgout.name,".annavgout");

		/* flag = 1 -> binary; flag = 2 -> ascii */
		if (output->domonavg == 1)
		{
			if (file_open(&(output->annavgout),'w'))
			{
				printf("Error opening annual average outfile (%s) in output_init()\n",output->annavgout.name);
				ok=0;
			}
		}
		else
		{
			if (file_open(&(output->annavgout),'o'))
			{
				printf("Error opening annual average outfile (%s) in output_init()\n",output->annavgout.name);
				ok=0;
			}
		}
	}
	if (ok && output->doannual)
	{
		strcpy(output->annout.name,output->outprefix);
		strcat(output->annout.name,".annout");

		/* flag = 1 -> binary; flag = 2 -> ascii */
		if (output->domonavg == 1)
		{
			if (file_open(&(output->annout),'w'))
			{
				printf("Error opening annual outfile (%s) in output_init()\n",output->annout.name);
				ok=0;
			}
		}
		else
		{
			if (file_open(&(output->annout),'o'))
			{
				printf("Error opening annual  outfile (%s) in output_init()\n",output->annout.name);
				ok=0;
			}
		}
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
		printf("Error reading keyword for output file data\n");
		ok=0;
	}
	if (ok && strcmp(keyword, key2))
	{
		printf("Expecting keyword --> %s in file %s\n",key2,init.name);
		ok=0;
	}
	
	/* read the number of daily output variables */
	if (ok && scan_value(init, &output->ndayout, 'i'))
	{
		printf("Error reading number of daily outputs: output_init()\n");
		ok=0;
	}
	/* allocate space for the daily output variable indices */
	if (ok)
	{
		output->daycodes = (int*) malloc(output->ndayout * sizeof(int));
		output->daynames = (char**) malloc(output->ndayout * sizeof(char*));
		if 	(!output->daycodes || !output->daynames)
		{
			printf("Error allocating for daycodes/daynames array: output_init()\n");
			ok=0;
		}
	}
	/* begin loop to read in the daily output variable indices */
	for (i=0 ; ok && i<output->ndayout ; i++)
	{
		if (ok && scan_array(init, &(output->daycodes[i]), 'i', 0))
		{
			printf("Error reading daily output index #%d: output_init()\n",i);
			ok=0;
		}
		output->daynames[i] = (char*) malloc(20 * sizeof(char));
		if (ok && scan_array(init, output->daynames[i], 's', 1))
		{
			printf("Error reading daily output names #%d: output_init()\n",i);
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
		printf("Error reading keyword for output file data\n");
		ok=0;
	}
	if (ok && strcmp(keyword, key3))
	{
		printf("Expecting keyword --> %s in file %s\n",key3,init.name);
		ok=0;
	}
	
	/* read the number of annual output variables */
	if (ok && scan_value(init, &output->nannout, 'i'))
	{
		printf("Error reading number of annual outputs: output_init()\n");
		ok=0;
	}
	/* allocate space for the annual output variable indices */
	if (ok)
	{
		output->anncodes = (int*) malloc(output->nannout * sizeof(int));
		output->annnames = (char**) malloc(output->nannout * sizeof(char*));
		if 	(!output->anncodes || !output->annnames)
		{
			printf("Error allocating for anncodes or annnames array: output_init()\n");
			ok=0;
		}
	}
	
	/* begin loop to read in the annual output variable indices */
	for (i=0 ; ok && i<output->nannout ; i++)
	{
		if (ok && scan_array(init, &(output->anncodes[i]), 'i', 0))
		{
			printf("Error reading annual output index #%d: output_init()\n",i);
			ok=0;
		}
		output->annnames[i] = (char*) malloc(20 * sizeof(char));
		if (ok && scan_array(init, output->annnames[i], 's',1))
		{
			printf("Error reading annual output name #%d: output_init()\n",i);
			ok=0;
		}
	}


	if (!ok)
	{
		fprintf(output->log_file.ptr, "ERROR in reading output section of INI file\n");
		fprintf(output->log_file.ptr, "SIMULATION STATUS [0 - failure; 1 - success]\n");
		fprintf(output->log_file.ptr, "0\n");
	}
	
	return (!ok);
}
