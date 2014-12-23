/* 
end_init.c
Final housekeeping for initialization file reading 

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "pointbgc.h"

int end_init(file init)
{
	int ok = 1;
	char key1[] = "END_INIT";
	char keyword[80];
	
	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** END_INIT                                                        ** 
	**                                                                 **
	********************************************************************/
	
	/* scan for the end keyword, exit if not next */
	if (ok && scan_value(init, keyword, 's'))
	{
		bgc_printf(BV_ERROR, "Error reading keyword for end of initialization file\n");
		bgc_printf(BV_ERROR, "Expecting keyword --> %s in file %s\n",key1,init.name);
		bgc_printf(BV_ERROR, "This indicates that you have the wrong number of lines\n");
		bgc_printf(BV_ERROR, "of information in your initialization file.\n");
		bgc_printf(BV_ERROR, "Refer to documentation to determine proper format.\n");
		ok=0;
	}
	if (ok && strcmp(keyword, key1))
	{
		bgc_printf(BV_ERROR, "Error reading keyword for end of initialization file\n");
		bgc_printf(BV_ERROR, "Expecting keyword --> %s in file %s\n",key1,init.name);
		bgc_printf(BV_ERROR, "This indicates that you have the wrong number of lines\n");
		bgc_printf(BV_ERROR, "of information in your initialization file.\n");
		bgc_printf(BV_ERROR, "Refer to documentation to determine proper format.\n");
		ok=0;
	}
	
	return (!ok);
}
