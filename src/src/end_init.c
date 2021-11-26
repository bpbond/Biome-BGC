/* 
end_init.c
Final housekeeping for initialization file reading 

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
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

int end_init(file init)
{
	int errorCode=0;
	char key1[] = "END_INIT";
	char keyword[STRINGSIZE];
	
	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** END_INIT                                                        ** 
	**                                                                 **
	********************************************************************/
	
	/* scan for the end keyword, exit if not next */
	if (!errorCode && scan_value(init, keyword, 's'))
	{
		printf("ERROR reading keyword for end of initialization file\n");
		printf("Expecting keyword --> %s in file %s\n",key1,init.name);
		printf("This indicates that you have the wrong number of lines\n");
		printf("of information in your initialization file.\n");
		printf("Refer to documentation to determine proper format.\n");
		errorCode=217;
	}
	if (!errorCode && strcmp(keyword, key1))
	{
		printf("ERROR reading keyword for end of initialization file\n");
		printf("Expecting keyword --> %s in file %s\n",key1,init.name);
		printf("This indicates that you have the wrong number of lines\n");
		printf("of information in your initialization file.\n");
		printf("Refer to documentation to determine proper format.\n");
		errorCode=217;
	}
	
	return (errorCode);
}
