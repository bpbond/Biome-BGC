/* 
output_init.c
Reads output control information from initialization file

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
BBGC MuSo v4
Copyright 2000, Peter E. Thornton
Numerical Terradynamics Simulation Group
Copyright 2014, D. Hidy (dori.hidy@gmail.com)
Hungarian Academy of Sciences
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

	/* get the estimated control filename*/
	if (ok && scan_value(init, &output->control_file,'s')) 
	{
		printf("Error reading control_file filename: output_init()\n");
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
	if (ok)
	{
		strcpy(output->log_file.name,output->outprefix);
		strcat(output->log_file.name,".log");
		if (file_open(&(output->log_file),'w'))
		{
			printf("Error opening log_file (%s) in output_init()\n",output->log_file.name);
			ok=0;
		}
	}

	/* open outfiles if specified */
	if (ok && output->dodaily)
	{
		strcpy(output->dayout.name,output->outprefix);
		strcat(output->dayout.name,".dayout");
		if (file_open(&(output->dayout),'w'))
		{
			printf("Error opening daily outfile (%s) in output_init()\n",output->dayout.name);
			ok=0;
		}
	}
	if (ok && output->domonavg)
	{
		strcpy(output->monavgout.name,output->outprefix);
		strcat(output->monavgout.name,".monavgout");
		if (file_open(&(output->monavgout),'w'))
		{
			printf("Error opening monthly average outfile (%s) in output_init()\n",output->monavgout.name);
			ok=0;
		}
	}
	if (ok && output->doannavg)
	{
		strcpy(output->annavgout.name,output->outprefix);
		strcat(output->annavgout.name,".annavgout");
		if (file_open(&(output->annavgout),'w'))
		{
			printf("Error opening annual average outfile (%s) in output_init()\n",output->annavgout.name);
			ok=0;
		}
	}
	if (ok && output->doannual)
	{
		strcpy(output->annout.name,output->outprefix);
		strcat(output->annout.name,".annout");
		if (file_open(&(output->annout),'w'))
		{
			printf("Error opening annual outfile (%s) in output_init()\n",output->annout.name);
			ok=0;
		}
	}
	if (ok)
	{
		/* simple text output */
		strcpy(output->anntext.name,output->outprefix);
		strcat(output->anntext.name,"_ann.txt");
		if (file_open(&(output->anntext),'o'))
		{
			printf("Error opening annual text file (%s) in output_init()\n",output->anntext.name);
			ok=0;
		}
		/* write the header info for simple text file */
		fprintf(output->anntext.ptr,"Annual summary output from BBGC MuSo v4\n");
		fprintf(output->anntext.ptr,"COLUMN1: simulation year\n");
		fprintf(output->anntext.ptr,"COLUMN2: ann_PRCP = annual total precipitation (mm/yr)\n");
		fprintf(output->anntext.ptr,"COLUMN3: ann_Tavg = annual average air temperature (deg C)\n");
		fprintf(output->anntext.ptr,"COLUMN4: max_LAI = annual maximum value of projected leaf area index (m2/m2)\n");
		fprintf(output->anntext.ptr,"COLUMN5: ann_ET = annual total evapotranspiration (mm/yr)\n");
		fprintf(output->anntext.ptr,"COLUMN6: ann_DP = annual total deep percolation (mm/yr)\n");
		fprintf(output->anntext.ptr,"COLUMN7: ann_NEE = annual net ecosystem exchange (positive: net source to the atmosphere; gC/m2/yr)\n");
		fprintf(output->anntext.ptr,"COLUMN8: ann_NBP = annual net biome production (positive: net gain; gC/m2/yr)\n");
		fprintf(output->anntext.ptr,"COLUMN9: ann_Cchg_SNSC = annual net change of ecosystem's carbon content caused by senescence (positive: net gain; gC/m2/yr)\n");             /* by Hidy 2008. */
		fprintf(output->anntext.ptr,"COLUMN10: ann_Cchg_PLT = annual net change of ecosystem's carbon content caused by planting (positive: net gain; gC/m2/yr)\n");      /* by Hidy 2008. */
		fprintf(output->anntext.ptr,"COLUMN11: ann_Cchg_THN = annual net change of ecosystem's carbon content caused by thinning (positive: net gain; gC/m2/yr)\n");         /* by Hidy 2008. */
		fprintf(output->anntext.ptr,"COLUMN12: ann_Cchg_MOW = annual net change of ecosystem's carbon content caused by mowing (positive: net gain; gC/m2/yr)\n");         /* by Hidy 2008. */
		fprintf(output->anntext.ptr,"COLUMN13: ann_Cchg_GRZ = annual net change of ecosystem's carbon content caused by grazing(positive: net gain; gC/m2/yr)\n");          /* by Hidy 2008. */
		fprintf(output->anntext.ptr,"COLUMN14: ann_Cchg_HRV = annual net change of ecosystem's carbon content caused by harvesting (positive: net gain; gC/m2/yr)\n");         /* by Hidy 2008. */
		fprintf(output->anntext.ptr,"COLUMN15: ann_Cchg_FRZ = annual net change of ecosystem's carbon content caused by fertilization (positive: net gain; gC/m2/yr)\n");      /* by Hidy 2008. */
		fprintf(output->anntext.ptr,"COLUMN16: ann_N_plus_GRZ = annual total N input from grazing (gN/m2/yr)\n");             /* by Hidy 2008. */
		fprintf(output->anntext.ptr,"COLUMN17: ann_N_plus_FRZ = annual total N input from fertilizing (gN/m2/yr)\n");             /* by Hidy 2008. */
		fprintf(output->anntext.ptr,"---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
		fprintf(output->anntext.ptr,"COLUMN1  COLUMN2    COLUMN3    COLUMN4    COLUMN5    COLUMN6    COLUMN7    COLUMN8    COLUMN9    COLUMN10    COLUMN11    COLUMN12    COLUMN13    COLUMN14    COLUMN15   COLUMN16    COLUMN17\n"); 
		fprintf(output->anntext.ptr,"---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");

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
		if 	(!output->daycodes)
		{
			printf("Error allocating for daycodes array: output_init()\n");
			ok=0;
		}
	}
	/* begin loop to read in the daily output variable indices */
	for (i=0 ; ok && i<output->ndayout ; i++)
	{
		if (ok && scan_value(init, &(output->daycodes[i]), 'i'))
		{
			printf("Error reading daily output #%d: output_init()\n",i);
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
		if 	(!output->anncodes)
		{
			printf("Error allocating for anncodes array: output_init()\n");
			ok=0;
		}
	}
	
	/* begin loop to read in the annual output variable indices */
	for (i=0 ; ok && i<output->nannout ; i++)
	{
		if (ok && scan_value(init, &(output->anncodes[i]), 'i'))
		{
			printf("Error reading annual output #%d: output_init()\n",i);
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
