/* 
co2_init.c
Initialize the annual co2 concentration parameters for bgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "pointbgc.h"

int co2_init(file init, co2control_struct* co2, int simyears)
{
	int ok = 1;
	int i;
	char key1[] = "CO2_CONTROL";
	char keyword[80];
	char junk[80];
	file temp;
	int reccount = 0;
	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** CO2_CONTROL                                                     ** 
	**                                                                 **
	********************************************************************/

	/* scan for the climate change block keyword, exit if not next */
	if (ok && scan_value(init, keyword, 's'))
	{
		bgc_printf(BV_ERROR, "Error reading keyword, co2_init()\n");
		ok=0;
	}
	if (ok && strcmp(keyword,key1))
	{
		bgc_printf(BV_ERROR, "Expecting keyword --> %s in %s\n",key1,init.name);
		ok=0;
	}

	/* begin reading co2 control information */
	if (ok && scan_value(init, &co2->varco2, 'i'))
	{
		bgc_printf(BV_ERROR, "Error reading variable CO2 flag: co2_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &co2->co2ppm, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading constant CO2 value: co2_init()\n");
		ok=0;
	}
	/* if using variable CO2 file, open it, otherwise
	discard the next line of the ini file */
	if (ok && co2->varco2)
	{
    	if (scan_open(init,&temp,'r')) 
		{
			bgc_printf(BV_ERROR, "Error opening annual CO2 file\n");
			ok=0;
		}

		/* Find the number of lines in the file*/
		/* Then use that number to malloc the appropriate arrays */
		while(fscanf(temp.ptr,"%*lf%*lf") != EOF)
		{
			reccount++;
		}
		rewind(temp.ptr);

		/* Store the total number of CO2 records found in the co2vals variable */
		co2->co2vals = reccount;
		bgc_printf(BV_DIAG,"Found: %i CO2 records in co2_init()\n",reccount);
		
		/* allocate space for the annual CO2 array */
		if (ok && !(co2->co2ppm_array = (double*) malloc(reccount *
			sizeof(double))))
		{
			bgc_printf(BV_ERROR, "Error allocating for annual CO2 array, co2_init()\n");
			ok=0;
		}
		if (ok && !(co2->co2year_array = (int*) malloc(reccount *
			sizeof(int))))
		{
			bgc_printf(BV_ERROR, "Error allocating for annual CO2 year array, co2_init()\n");
			ok=0;
		}
		/* read year and co2 concentration for each simyear */
		for (i=0 ; ok && i<reccount ; i++)
		{
			if (fscanf(temp.ptr,"%i%lf",
				&(co2->co2year_array[i]),&(co2->co2ppm_array[i]))==EOF)
			{
				bgc_printf(BV_ERROR, "Error reading annual CO2 array, ctrl_init()\n");
				bgc_printf(BV_ERROR, "Note: file must contain a pair of values for each\n");
				bgc_printf(BV_ERROR, "simyear: year and CO2.\n");
				ok=0;
			}
			/* printf("CO2 value read: %i %lf\n",co2->co2year_array[i],co2->co2ppm_array[i]); */
			if (co2->co2ppm_array[i] < 0.0)
			{
				bgc_printf(BV_ERROR, "Error in co2_init(): co2 (ppm) must be positive\n");
				ok=0;
			}
		}
		fclose(temp.ptr);
	}
	else
	{
		if (scan_value(init, junk, 's'))
		{
			bgc_printf(BV_ERROR, "Error scanning annual co2 filename\n");
			ok=0;
		}
	}
	
	if (co2->co2ppm < 0.0)
	{
		bgc_printf(BV_ERROR, "Error in co2_init(): co2 (ppm) must be positive\n");
		ok=0;
	}

	return (!ok);
}
