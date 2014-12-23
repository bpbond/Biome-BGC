/* 
ndep_init.c
Initialize the annual ndep values for bgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "pointbgc.h"

int ndep_init(file ndepfile, ndepcontrol_struct* ndepctrl)
{
	int ok = 1,i = 0,reccount = 0;
	
	/* if using variable CO2 file, open it, otherwise
	discard the next line of the ini file */
	if (file_open(&ndepfile,'r')) 
	{
		bgc_printf(BV_ERROR, "Error opening annual NDEP file: %s\n",ndepfile);
		ok=0;
	}

	/* Find the number of lines in the file*/
	/* Then use that number to malloc the appropriate arrays */
	while(fscanf(ndepfile.ptr,"%*lf%*lf") != EOF) reccount++;
	rewind(ndepfile.ptr);

	/* Store the total number of ndep records found in the ndepvals variable */
	ndepctrl->ndepvals = reccount;

	bgc_printf(BV_DIAG,"Found: %i NDEP records in co2_init()\n",reccount);
	
	/* allocate space for the annual CO2 array */
	if (ok && !(ndepctrl->ndep_array = (double*) malloc(reccount * sizeof(double))))
	{
		bgc_printf(BV_ERROR, "Error allocating for annual ndep array, ndep_init()\n");
		ok=0;
	}
	if (ok && !(ndepctrl->ndepyear_array = (int*) malloc(reccount * sizeof(int))))
	{
		bgc_printf(BV_ERROR, "Error allocating for annual ndep year array, co2_init()\n");
		ok=0;
	}
	/* read year and co2 concentration for each simyear */
	for (i=0 ; ok && i<reccount ; i++)
	{
		if (fscanf(ndepfile.ptr,"%i%lf", &(ndepctrl->ndepyear_array[i]),&(ndepctrl->ndep_array[i]))==EOF)
		{
			bgc_printf(BV_ERROR, "Error reading annual NDEP array, ndep_init()\n");
			bgc_printf(BV_ERROR, "Note: file must contain a pair of values for each\n");
			bgc_printf(BV_ERROR, "simyear: year and ndep.\n");
			ok=0;
		}
		bgc_printf(BV_DIAG, "NDEP value read: %i %lf\n",ndepctrl->ndepyear_array[i],ndepctrl->ndep_array[i]);
		
	}
	fclose(ndepfile.ptr);
	
	return (!ok);
}
