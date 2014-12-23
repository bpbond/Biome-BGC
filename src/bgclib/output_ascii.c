/* ASCII output function for Biome-BGC 		*/
/* Written by: Matt Jolly 			*/
/* Dev. Date : 15 Feb 2005			*/
/* Rev. Date :					*/
/* Purpose: take a set of binary inputs and write them as ascii outputs */

/**
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
**/

#include "bgc.h"

int output_ascii(float arr[],int nvars,  FILE *ptr)
{
	int i = 0;
	
	for(i = 0;i < nvars;i++){ fprintf(ptr,"%10.8f\t",arr[i]);}
	fprintf(ptr,"\n");
	
	return(EXIT_SUCCESS);

}
	
