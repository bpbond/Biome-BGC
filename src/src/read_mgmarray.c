	/* 
read_mgmarray.c
read management information for pointbgc simulation (MGM_flag=1: from ini file, MGM_flag=2: from independent file)

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
BBGC MuSo v4
Copyright 2014, D. Hidy (dori.hidy@gmail.com)
Hungarian Academy of Sciences
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


int read_mgmarray(int simyr, int MGM_flag, file MGM_file, double*** mgmarray)
{
	char header[100];
	int ok = 1;
	int nmd = 0;
	int ny = 0;
	int scanflag = 0;

	int arraySizeX=N_MGMDAYS;
	int arraySizeY=simyr;

	/* allocate space for the MGM array */
	(*mgmarray) = (double**) malloc(arraySizeX*sizeof(double*));  
 
	for (nmd = 0; nmd < arraySizeX; nmd++)  
	{
		(*mgmarray)[nmd] = (double*) malloc(arraySizeY*sizeof(double));  

	}

	/* header lines (simyear) in independent management file */
	if (MGM_flag == 2)
	{
		if (ok && scan_value(MGM_file, header, 's'))
		{
			printf("Error reading header string: grazing_int.c\n");
			exit(1);
		}
	}

	/* MGM data reading from ini of indep.file */
	for (ny=0; ny<simyr; ny++)
	{
		
		/* scanflag: 0 - step to next row, 1 - reading form the same row */
		scanflag = 0;
		for (nmd=0; nmd<N_MGMDAYS; nmd++)
		{
			if (nmd==N_MGMDAYS-1) scanflag=1;
			if (ok && scan_array(MGM_file, &((*mgmarray)[nmd][ny]), 'd', scanflag))
			{
				printf("Error reading management data (read_mgmarray.c)\n");
				ok=0;
			}
		}
	}
	


		
	return (!ok);

}