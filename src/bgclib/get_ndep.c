/* 
get_ndep.c
retrieve the appropriate nitrogen deposition value for the current simulation year

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "bgc.h"

double get_ndep(ndepcontrol_struct * ndepctrl,int simyr)
{
	int i;
	for(i = 0;i < ndepctrl->ndepvals;i++)
	{
		if(ndepctrl->ndepyear_array[i] == simyr)
		{
			return (ndepctrl->ndep_array[i]);
		}
	}
	return(-999.9);
}
