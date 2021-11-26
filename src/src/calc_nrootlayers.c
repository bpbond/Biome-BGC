/* 
calc_nrootlayers.c
calculation of changing rooting depth based on empirical function 
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Copyright 2021, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "ini.h"
#include "bgc_struct.h"
#include "bgc_func.h"
#include "bgc_constants.h"

int calc_nrootlayers(int flag, double maxRD, double frootc, siteconst_struct* sitec, epvar_struct* epv)
{

	int errorCode=0;


	if (flag == 0)
	{
		/* ***************************************************************************************************** */	
		/* 1. Calculating the number of the soil layers in which root can be found. It determines the rootzone depth (only on first day) */
	
		if (maxRD > 0)
		{
			if (maxRD > sitec->soillayer_depth[0])
			{
				if (maxRD > sitec->soillayer_depth[1])
				{	
					if (maxRD > sitec->soillayer_depth[2])
					{
						if (maxRD > sitec->soillayer_depth[3])
						{
							if (maxRD > sitec->soillayer_depth[4])
							{
								if (maxRD > sitec->soillayer_depth[5])
								{
									if (maxRD > sitec->soillayer_depth[6])
									{
										if (maxRD > sitec->soillayer_depth[7])
										{
											if (maxRD > sitec->soillayer_depth[8])
											{
													epv->n_maxrootlayers = 10;
											}
											else 
											{
												epv->n_maxrootlayers = 9;
											}
										}
										else 
										{
											epv->n_maxrootlayers = 8;
										}
									}
									else 
									{
										epv->n_maxrootlayers = 7;
									}
								}
								else 
								{
									epv->n_maxrootlayers = 6;
								}
							}
							else 
							{
								epv->n_maxrootlayers = 5;
							}
						}
						else
						{
							epv->n_maxrootlayers = 4;
						}
					}
					else 
					{
						epv->n_maxrootlayers = 3;
					}	
				}
				else 
				{
					epv->n_maxrootlayers = 2;
				}
			}
			else
			{
				epv->n_maxrootlayers = 1;
			}
		}
		else 
		{
			epv->n_maxrootlayers = 0;
			printf("\n");
			printf("ERROR in multilayer_rootdepth: maximum of rooting depth is 0\n");
			errorCode=1;
		}
	
	}
	else
	{

		/* ***************************************************************************************************** */	
		/* 3. Calculating the number of the soil layers in which root can be found. It determines the rootzone depth (epv->n_rootlayers) */
	
		if (epv->rootdepth > 0)
		{
			if (epv->rootdepth > sitec->soillayer_depth[0])
			{
				if (epv->rootdepth > sitec->soillayer_depth[1])
				{	
					if (epv->rootdepth > sitec->soillayer_depth[2])
					{
						if (epv->rootdepth > sitec->soillayer_depth[3])
						{
							if (epv->rootdepth > sitec->soillayer_depth[4])
							{
								if (epv->rootdepth > sitec->soillayer_depth[5])
								{
									if (epv->rootdepth > sitec->soillayer_depth[6])
									{
										if (epv->rootdepth > sitec->soillayer_depth[7])
										{
											if (epv->rootdepth > sitec->soillayer_depth[8])
											{	
												epv->n_rootlayers = 10;		
											}
											else 
											{
												epv->n_rootlayers = 9;
											}
										}
										else 
										{
											epv->n_rootlayers = 8;
										}
									}
									else 
									{
										epv->n_rootlayers = 7;
									}
								}
								else 
								{
									epv->n_rootlayers = 6;
								}
							}
							else 
							{
								epv->n_rootlayers = 5;
							}
						}
						else
						{
							epv->n_rootlayers = 4;
						}
					}
					else 
					{
						epv->n_rootlayers = 3;
					}	
				}
				else 
				{
					epv->n_rootlayers = 2;
				}
			}
			else
			{
				epv->n_rootlayers = 1;
			}
		}
		else 
		{
			epv->n_rootlayers = 0;
			if (frootc > CRIT_PREC)
			{
				printf("\n");
				printf("ERROR in multilayer_rootdepth: root is available but rooting depth is 0\n");
				errorCode=1;
			}
		}
	}
	
	return(errorCode);
}

