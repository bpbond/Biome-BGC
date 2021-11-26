/*
output_handling.c

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2020, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "ini.h"
#include "bgc_struct.h"
#include "bgc_func.h"
#include "bgc_constants.h"
#include "pointbgc_struct.h"
#include "pointbgc_func.h"

int output_handling(int monday, int endday, control_struct* ctrl, double** output_map, 
					double* dayarr, double* monavgarr, double* annavgarr, double* annarr, 
					file dayout, file monavgout, file annavgout, file annout)
{
	int i = 0;
	int errorCode = 0;
	int outv;
	int yday, simyr;
	int yearOUT, monthOUT, dayOUT, ydayOUT;

	if(ctrl->spinup == 1)
 		simyr = ctrl->spinyears;
	else
		simyr = ctrl->simyr;

	yday = ctrl->yday;


	/* date handling: specific case for southern hemisphere */
	if (ctrl->spinup == 0)
		yearOUT=simyr+ctrl->simstartyear;
	else
		yearOUT  = simyr;
	
	/* output data: re-shifted date in southern hemisphere */
	if (ctrl->south_shift)
	{
		if (ctrl->yday < ctrl->south_shift-1)
		{
			ydayOUT = ctrl->yday + ctrl->south_shift+1;
		}
		else
		{
			ydayOUT = ctrl->yday - ctrl->south_shift+2;
			yearOUT += 1;
		}
		
		if (!errorCode && doy_to_date(ydayOUT, &monthOUT, &dayOUT, 1))
		{
			printf("\n");
			printf("ERROR in call to doy_to_date() from output_handling()\n");
			errorCode=1;
		}
		
	}
	else
	{
		monthOUT = ctrl->month;
		dayOUT   = ctrl->day;
		ydayOUT  = ctrl->yday+1;
		
	}

	


	/* DAILY OUTPUT HANDLING */
	/* fill the daily output array if daily output is requested,or if the monthly or annual average 
	   of daily output variables have been requested */
	if (!errorCode && (ctrl->dodaily || ctrl->domonavg || ctrl->doannavg))
	{
		/* fill the daily output array */
		for (outv=0 ; outv<ctrl->ndayout ; outv++)
		{
			dayarr[outv]  = (double) *(output_map[ctrl->daycodes[outv]]);
		}
	}
	/* only write daily outputs if requested */
	if (!errorCode && ctrl->dodaily)
	{
		if (ctrl->dodaily == 2)
		{
			/* write ASCII the daily output array to daily output file */
			/* header */
			if (yday == 0 && simyr == 0)
			{
				fprintf(dayout.ptr, "%19s", " year month day yday");
				for (i=0; i< ctrl->ndayout; i++)
				{
					fprintf(dayout.ptr, "%50s", ctrl->daynames[i]);
				}
				fprintf(dayout.ptr, "\n");
			}
			/* values  */
			fprintf(dayout.ptr, "%5i %4i %4i %4i ", yearOUT, monthOUT, dayOUT, ydayOUT);
		
			for (i=0; i< ctrl->ndayout; i++)
			{
				fprintf(dayout.ptr, "%14.9f ", dayarr[i]);
			}
			fprintf(dayout.ptr, "\n");
		}
		else
		{	
			/* write BINARY the daily output array to daily output file */
			if (ctrl->dodaily == 1)
			{
				if (fwrite(dayarr, sizeof(double), ctrl->ndayout, dayout.ptr)!= (size_t)ctrl->ndayout)
				{
					printf("\n");
					printf("ERROR writing to %s: simyear = %d, simday = %d\n",dayout.name, yearOUT, ydayOUT);
				}
			}
			else
			/* printing on the screen */
			{
				for (i=0; i< ctrl->ndayout; i++)
				{
					printf("%14.9f ", dayarr[i]);
				}
				printf("\n");
			}
			

		}

	}

	
	/* MONTHLY AVERAGE OF DAILY OUTPUT VARIABLES */
	if (ctrl->domonavg)
	{
		/* update the monthly average array */
		for (outv=0 ; outv<ctrl->ndayout ; outv++)
		{
			monavgarr[outv] += dayarr[outv];
		}

		/* header of monavg file (only in case of ASCII) */
		if (yday == 0 && simyr == 0 && ctrl->domonavg == 2)
		{
			fprintf(monavgout.ptr, "%10s", " year month");
			for (i=0; i< ctrl->ndayout; i++)
			{
				fprintf(monavgout.ptr, "%30s", ctrl->daynames[i]);
			}
			fprintf(monavgout.ptr, "\n");
		}
		
		/* if this is the last day of the current month, output... */
		if (yday == endday)
		{
			/* finish the averages */
			for (outv=0 ; outv<ctrl->ndayout ; outv++)
			{
				monavgarr[outv] /= (double)monday;
			}

			if (ctrl->domonavg == 2)
			{

				/* write ASCII the monthly output array to monthly output file */
				fprintf(monavgout.ptr, "%5i %4i", yearOUT, monthOUT);
			
				for (i=0; i< ctrl->ndayout; i++)
				{
					fprintf(monavgout.ptr, "%14.8f ", monavgarr[i]);
				}
				fprintf(monavgout.ptr, "\n");
			}
			else
			{
				/* write to file BINARY */
				if (ctrl->domonavg == 1)
				{
					if (fwrite(monavgarr, sizeof(double), ctrl->ndayout, monavgout.ptr)!= (size_t)ctrl->ndayout)
					{
						printf("\n");
						printf("ERROR writing to %s: simyear = %d, simday = %d\n",monavgout.name,yearOUT,ydayOUT);
						errorCode=1;
					}
				}
				else
				/* printing on the screen */
				{
					for (i=0; i< ctrl->ndayout; i++)
					{
						printf("%14.8f ", monavgarr[i]);
					}
					printf("\n");
				}
			}
			
			/* reset monthly average variables for next month */
			for (outv=0 ; outv<ctrl->ndayout ; outv++)
			{
				monavgarr[outv] = 0.0;
			}
			
		
#ifdef DEBUG
				printf("%d\t%d\tdone monavg output\n",simyr,yday);
#endif
			
			}
		}
		
	
		/* ANNUAL AVERAGE OF DAILY OUTPUT VARIABLES */
		if (ctrl->doannavg)
		{
			/* update the annual average array */
			for (outv=0 ; outv<ctrl->ndayout ; outv++)
			{
				annavgarr[outv] += dayarr[outv];
			}

			/* header of annavg file (only in case of ASCII) */
			if (yday == 0 && simyr == 0 && ctrl->doannavg == 2)
			{
				fprintf(annavgout.ptr, "%5s", " year");
				for (i=0; i< ctrl->ndayout; i++)
				{
					fprintf(annavgout.ptr, "%30s", ctrl->daynames[i]);
				}
				fprintf(annavgout.ptr, "\n");


			}
			
			/* if this is the last day of the year, output... */
			if (yday == nDAYS_OF_YEAR-1)
			{

				/* finish averages */
				for (outv=0 ; outv<ctrl->ndayout ; outv++)
				{
					annavgarr[outv] /= nDAYS_OF_YEAR;
				}
				
				/* write to file */
				if (ctrl->doannavg == 2)
				{
					/* write ASCII the annual output array to annual output file */
					fprintf(annavgout.ptr, "%5i", yearOUT);
					
					for (i=0; i< ctrl->ndayout; i++)
					{
						fprintf(annavgout.ptr, "%14.8f ", annavgarr[i]);

					}
					fprintf(annavgout.ptr, "\n");

				}
				else
				{
					/* write BINARY the annual output array to annual output file */
					if (ctrl->doannavg == 1)
					{
						if (fwrite(annavgarr, sizeof(double), ctrl->ndayout, annavgout.ptr)!= (size_t)ctrl->ndayout)
						{
							printf("\n");
							printf("ERROR writing to %s: simyear = %d, simday = %d\n", annavgout.name,simyr,yday);
							errorCode=1;
						}
					}
					else
					/* printing on the screen */
					{
						for (i=0; i< ctrl->ndayout; i++)
						{
							printf("%14.8f ", annavgarr[i]);
						}
						printf("\n");
					}
				}

				/* reset annual average variables for next month */
				for (outv=0 ; outv<ctrl->ndayout ; outv++)
				{
					annavgarr[outv] = 0.0;
				}
				
#ifdef DEBUG
				printf("%d\t%d\tdone annavg output\n",simyr,yday);
#endif
			
			}
			
		}
		/* ANNUAL OUTPUT HANDLING */
		/* only write annual outputs if requested */
		if (!errorCode && ctrl->doannual)
		{
			/* header of monavg file (only in case of ASCII) */
			if (yday == 0 && simyr == 0 && ctrl->doannual == 2)
			{
				fprintf(annout.ptr, "%5s", " year");
				for (i=0; i< ctrl->nannout; i++)
				{
					fprintf(annout.ptr, "%30s", ctrl->annnames[i]);
				}
				fprintf(annout.ptr, "\n");
			}

			if (yday == nDAYS_OF_YEAR-1)
			{
				/* fill the annual output array */
				for (outv=0 ; outv<ctrl->nannout ; outv++)
				{
					
					annarr[outv] = (double) *output_map[ctrl->anncodes[outv]];
				}
					
				/* write the annual output array to annual output file */
				if (ctrl->doannual == 2)
				{
					/* write ASCII the annual output array to annual output file */
					fprintf(annout.ptr, "%5i", yearOUT);
				
					for (i=0; i< ctrl->nannout; i++)
					{
						fprintf(annout.ptr, "%12.6f ", annarr[i]);
					}
					fprintf(annout.ptr, "\n");
				}
				else
				{
					/* write BINARY the annual output array to annual output file */
					if (ctrl->doannual == 1)
					{	
						if (fwrite(annarr, sizeof(double), ctrl->nannout, annout.ptr)!= (size_t)ctrl->nannout)
						{
							printf("\n");
							printf("ERROR writing to %s: simyear = %d, simday = %d\n",annout.name,simyr,yday);
							errorCode=1;
						}
					}
					else
					/* printing on the screen */
					{
						for (i=0; i< ctrl->ndayout; i++)
						{
							printf("%12.6f ", annarr[i]);
						}
						printf("\n");
					}
				}
			}
				
#ifdef DEBUG
			printf("%d\t%d\tdone annual output\n",simyr,yday);
#endif
		}

	

		/* calculate month and day variables at the end of a month */
		if (yday == endday)
		{
			/* if this is the last day of the year, output... */
			if (yday == nDAYS_OF_YEAR-1)
			{
				ctrl->month = 1;
				ctrl->day   = 1;
			}
			else
			{
				ctrl->month += 1;
				ctrl->day   = 1;
			}
		}
		else
		{
			ctrl->day   += 1;
		}


return (errorCode);
}

