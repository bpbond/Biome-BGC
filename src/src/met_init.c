/* 
met_init.c
open met file for input, scan through header lines

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.4
Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group (NTSG)
School of Forestry, University of Montana
Missoula, MT 59812
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
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

int met_init(file init, point_struct* point)
{
	int ok = 1;
	int nhead = 0;
	int i;
	char key1[] = "MET_INPUT";
	char keyword[80];
	char junk_head[1024];

	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** MET_INPUT                                                       ** 
	**                                                                 **
	********************************************************************/
	
	/* scan for the input metfile keyword, exit if not next */
	if (ok && scan_value(init, keyword, 's'))
	{
		printf("Error reading keyword for met file: met_init()\n");
		ok=0;
	}
	if (ok && strcmp(keyword, key1))
	{
		printf("Expecting keyword --> %s in file %s\n",key1,init.name);
		ok=0;
	}
	
	/* get the met data filename and open file for ascii read */
    if (ok && scan_open(init,&point->metf,'i')) 
	{
		printf("Error opening met data file: met_init()\n");
		ok=0;
	}
	
	/* get number of metfile header lines */
	if (ok && scan_value(init, &nhead, 'i'))
	{
		printf("Error reading number of met file header lines: met_init()\n");
		ok=0;
	}
	
	/* read header lines from input met data file and discard */
	for (i=0 ; ok && i<nhead ; i++)
	{
		if (scan_value(point->metf, junk_head, 's'))
		{
			printf("Error reading met file header line #%d\n",i+1);
			ok=0;
		}
	}

	return (!ok);
}

