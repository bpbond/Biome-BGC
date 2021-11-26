/* 
output_init.c
Reads output control information from initialization file

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2020, D. Hidy [dori.hidy@gmail.com]
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

int output_init(file init, int transient, harvesting_struct* HRV, output_struct* output)
{
	int errorCode=0;
	int i;
	char key1[]  = "OUTPUT_CONTROL";
	char key2[] = "DAILY_OUTPUT";
	char key3[] = "ANNUAL_OUTPUT";
	
	char keyword[STRINGSIZE];

	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** OUTPUT_CONTROL                                                  ** 
	**                                                                 **
	********************************************************************/
	
	/* scan for the output file block keyword, exit if not next */
	if (!errorCode && scan_value(init, keyword, 's'))
	{
		printf("ERROR reading keyword for output file data\n");
		errorCode=216;
	}
	if (!errorCode && strcmp(keyword, key1))
	{
		printf("Expecting keyword --> %s in file %s\n",key1,init.name);
		errorCode=216;
	}
	
	/* get the output filename prefix */
	if (!errorCode && scan_value(init,output->outprefix,'s'))
	{
		printf("ERROR reading outfile prefix: output_init(), output_init.c\n");
		errorCode=21601;
	}
	
	/* scan flags for daily output */
	if (!errorCode && scan_value(init, &output->dodaily, 'i'))
	{
		printf("ERROR reading daily output flag: output_init()\n");
		errorCode=21601;
	}
	/* scan flag for monthly average output (operates on daily variables) */
	if (!errorCode && scan_value(init, &output->domonavg, 'i'))
	{
		printf("ERROR reading monthly average output flag: output_init()\n");
		errorCode=21601;
	}
	/* scan flag for annual average output (operates on daily variables) */
	if (!errorCode && scan_value(init, &output->doannavg, 'i'))
	{
		printf("ERROR reading annual average output flag: output_init()\n");
		errorCode=21601;
	}
	/* scan flag for annual output */
	if (!errorCode && scan_value(init, &output->doannual, 'i'))
	{
		printf("ERROR reading annual output flag: output_init()\n");
		errorCode=21601;
	}
	/* scan flag for on-screen progress indicator */
	if (!errorCode && scan_value(init, &output->onscreen, 'i'))
	{
		printf("ERROR reading on-screen indicator flag: output_init()\n");
		errorCode=21601;
	}
	

	/* open outfiles if specified */
	strcpy(output->log_file.name,output->outprefix);
	strcat(output->log_file.name,".log");
	if (file_open(&(output->log_file),'o',1))
	{
		printf("ERROR opening log_file (%s) in output_init()\n",output->log_file.name);
		errorCode=21603;
	}

	/* open outfiles if specified */
	strcpy(output->econout_file.name,output->outprefix);
	strcat(output->econout_file.name,".econout");

	if (HRV->HRV_num)
	{
		if (file_open(&(output->econout_file),'o',1))
		{
			printf("ERROR opening econout_file (%s) in output_init()\n",output->log_file.name);
			errorCode=21603;
		}
	}


	/* open outfiles if specified */
	if (!errorCode && output->dodaily)
	{
		strcpy(output->dayout.name, output->outprefix);
		strcat(output->dayout.name, ".dayout");

		/* transient output */
		strcpy(output->dayoutT.name,output->outprefix);
		strcat(output->dayoutT.name,"_T.dayout");
		
		/* flag = 1 -> binary; flag = 2 -> ascii */
		if (output->dodaily == 1)
		{
			if (file_open(&(output->dayout),'w',1))
			{
				printf("ERROR opening daily outfile (%s) in output_init()\n",output->dayout.name);
				errorCode=21604;
			}
			if (transient)
			{
				if (file_open(&(output->dayoutT),'w',1))
				{
					printf("ERROR opening transient daily outfile (%s) in output_init()\n",output->dayout.name);
					errorCode=21604;
				}
			}

		}
		else
		{
			if (file_open(&(output->dayout),'o',1))
			{
				printf("ERROR opening daily outfile (%s) in output_init()\n",output->dayout.name);
				errorCode=21604;
			}
			if (transient)
			{
				if (file_open(&(output->dayoutT),'o',1))
				{
					printf("ERROR opening transient daily outfile (%s) in output_init()\n",output->dayout.name);
					errorCode=21604;
				}
			}
		}
	}
	if (!errorCode && output->domonavg)
	{
		strcpy(output->monavgout.name, output->outprefix);
		strcat(output->monavgout.name,".monavgout");

		/* transient output */
		strcpy(output->monavgoutT.name,output->outprefix);
		strcat(output->monavgoutT.name,"_T.monavgout");

		/* flag = 1 -> binary; flag = 2 -> ascii */
		if (output->domonavg == 1)
		{
			if (file_open(&(output->monavgout),'w',1))
			{
				printf("ERROR opening monthly average outfile (%s) in output_init()\n",output->monavgout.name);
				errorCode=21604;
			}
			if (transient)
			{
				if (file_open(&(output->monavgoutT),'w',1))
				{
					printf("ERROR opening transient monthly average outfile (%s) in output_init()\n",output->dayout.name);
					errorCode=21604;
				}
			}
		}
		else
		{
			if (file_open(&(output->monavgout),'o',1))
			{
				printf("ERROR opening monthly average outfile (%s) in output_init()\n",output->monavgout.name);
				errorCode=21604;
			}
			if (transient)
			{
				if (file_open(&(output->monavgoutT),'o',1))
				{
					printf("ERROR opening transient monthly average outfile (%s) in output_init()\n",output->dayout.name);
					errorCode=21604;
				}
			}
		}
	}
	if (!errorCode && output->doannavg)
	{
		strcpy(output->annavgout.name, output->outprefix);
		strcat(output->annavgout.name,".annavgout");

		/* transient output */
		strcpy(output->annavgoutT.name,output->outprefix);
		strcat(output->annavgoutT.name,"_T.annavgout");

		/* flag = 1 -> binary; flag = 2 -> ascii */
		if (output->doannavg == 1)
		{
			if (file_open(&(output->annavgout),'w',1))
			{
				printf("ERROR opening annual average outfile (%s) in output_init()\n",output->annavgout.name);
				errorCode=21604;
			}
			if (transient)
			{
				if (file_open(&(output->annavgoutT),'w',1))
				{
					printf("ERROR opening transient annual average outfile (%s) in output_init()\n",output->dayout.name);
					errorCode=21604;
				}
			}
		}
		else
		{
			if (file_open(&(output->annavgout),'o',1))
			{
				printf("ERROR opening annual average outfile (%s) in output_init()\n",output->annavgout.name);
				errorCode=21604;
			}
			if (transient)
			{
				if (file_open(&(output->annavgoutT),'o',1))
				{
					printf("ERROR opening transient annual average outfile (%s) in output_init()\n",output->dayout.name);
					errorCode=21604;
				}
			}
		}
	}
	if (!errorCode && output->doannual)
	{
		strcpy(output->annout.name, output->outprefix);
		strcat(output->annout.name,".annout");

		/* transient output */
		strcpy(output->annoutT.name,output->outprefix);
		strcat(output->annoutT.name,"_T.annout");

		/* flag = 1 -> binary; flag = 2 -> ascii */
		if (output->doannual == 1)
		{
			if (file_open(&(output->annout),'w',1))
			{
				printf("ERROR opening annual outfile (%s) in output_init()\n",output->annout.name);
				errorCode=21604;
			}
			if (transient)
			{
				if (file_open(&(output->annoutT),'w',1))
				{
					printf("ERROR opening transient annual outfile (%s) in output_init()\n",output->dayout.name);
					errorCode=21604;
				}
			}
		}
		else
		{
			if (file_open(&(output->annout),'o',1))
			{
				printf("ERROR opening annual  outfile (%s) in output_init()\n",output->annout.name);
				errorCode=21604;
			}
			if (transient)
			{
				if (file_open(&(output->annoutT),'o',1))
				{
					printf("ERROR opening transient annual outfile (%s) in output_init()\n",output->dayout.name);
					errorCode=21604;
				}
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
	if (!errorCode && scan_value(init, keyword, 's'))
	{
		printf("ERROR reading keyword for output file data\n");
		errorCode=21605;
	}
	if (!errorCode && strcmp(keyword, key2))
	{
		printf("Expecting keyword --> %s file %s\n",key2, init.name);
		errorCode=21605;
	}
	
	/* read the number of daily output variables */
	if (!errorCode && scan_value(init, &output->ndayout, 'i'))
	{
		printf("ERROR reading number of daily outputs: output_init()\n");
		errorCode=21605;
	}
	/* allocate space for the daily output variable indices */
	if (!errorCode && output->ndayout > 0)
	{
		output->daycodes = (int*) malloc(output->ndayout * sizeof(int));
		output->daynames = (char**) malloc(output->ndayout * sizeof(char*));
		if 	(!output->daycodes || !output->daynames)
		{
			printf("ERROR allocating for daycodes/daynames array: output_init()\n");
			errorCode=21605;
		}
	}
	/* begin loop to read in the daily output variable indices (daily_wn: with variable names in the 2. column */
	if (output->dodaily == 2 || output->domonavg == 2  || output->doannavg == 2 )
	{
		for (i=0 ; !errorCode && i<output->ndayout ; i++)
		{
			if (!errorCode && scan_array(init, &(output->daycodes[i]), 'i', 0, 1))
			{
				printf("ERROR reading daily output index #%d: output_init()\n",i);
				errorCode=21605;
			}
			output->daynames[i] = (char*) malloc(STRINGSIZE * sizeof(char));
			if (!errorCode && scan_array(init, output->daynames[i], 's', 1, 1))
			{
				printf("ERROR reading daily output names #%d: output_init()\n",i);
				errorCode=21605; 
			}
		}
	}
	else
	{
		for (i=0 ; !errorCode && i<output->ndayout ; i++)
		{
			if (!errorCode && scan_array(init, &(output->daycodes[i]), 'i', 1, 1))
			{
				printf("ERROR reading daily output index #%d: output_init()\n",i);
				errorCode=21605;
			}
			output->daynames[i] = (char*) malloc(STRINGSIZE * sizeof(char));
			sprintf(output->daynames[i], "%s%i", "var", output->daycodes[i]);

		}
	}
	
	
	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** ANNUAL_OUTPUT                                                   ** 
	**                                                                 **
	********************************************************************/
	
	/* scan for the output file block keyword, exit if not next */
	if (!errorCode && scan_value(init, keyword, 's'))
	{
		printf("ERROR reading keyword for output file data\n");
		errorCode=21606;
	}

	if (!errorCode && strcmp(keyword, key3))
	{
		printf("Expecting keyword --> %s in file %s\n",key3, init.name);
		errorCode=21606;
	}
	
	/* read the number of annual output variables */
	if (!errorCode && scan_value(init, &output->nannout, 'i'))
	{
		printf("ERROR reading number of annual outputs: output_init()\n");
		errorCode=21606;
	}
	/* allocate space for the annual output variable indices */
	if (!errorCode)
	{
		output->anncodes = (int*) malloc(output->nannout * sizeof(int));
		output->annnames = (char**) malloc(output->nannout * sizeof(char*));
		if 	(!output->anncodes || !output->annnames)
		{
			printf("ERROR allocating for anncodes or annnames array: output_init()\n");
			errorCode=21606;
		}
	}
	
	/* begin loop to read in the annual output variable indices */
	if (output->doannual == 2)
	{
		for (i=0 ; !errorCode && i<output->nannout ; i++)
		{
			if (!errorCode && scan_array(init, &(output->anncodes[i]), 'i', 0, 1))
			{
				printf("ERROR reading annual output index #%d: output_init()\n",i);
				errorCode=21606;
			}
			output->annnames[i] = (char*) malloc(STRINGSIZE * sizeof(char));
			if (!errorCode && scan_array(init, output->annnames[i], 's',1, 1))
			{
				printf("ERROR reading annual output name #%d: output_init()\n",i);
				errorCode=21606;
			}
		}
	}
	else
	{
		for (i=0 ; !errorCode && i<output->nannout ; i++)
		{
			if (!errorCode && scan_array(init, &(output->anncodes[i]), 'i', 1, 1))
			{
				printf("ERROR reading annual output index #%d: output_init()\n",i);
				errorCode=21606;
			}
			output->annnames[i] = (char*) malloc(STRINGSIZE * sizeof(char));
			sprintf(output->annnames[i], "%s%i", "var", output->anncodes[i]);
		}
	}


	if (errorCode)
	{
		fprintf(output->log_file.ptr, "ERROR in reading output section of INI file\n");
		fprintf(output->log_file.ptr, "SIMULATION STATUS [0 - failure; 1 - success]\n");
		fprintf(output->log_file.ptr, "0\n");
	}
	
	return (errorCode);
}
