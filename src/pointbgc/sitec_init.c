/* 
sitec_init.c
Initialize the site physical constants for bgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "pointbgc.h"

int sitec_init(file init, siteconst_struct* sitec)
{
	/* reads the site physical constants from *.init */ 

	int ok=1;
	char key[] = "SITE";
	char keyword[80];
	double sand,silt,clay; /* percent sand, silt, and clay */

	/* first scan keyword to ensure proper *.init format */ 
	if (ok && scan_value(init, keyword, 's'))
	{
		bgc_printf(BV_ERROR, "Error reading keyword, sitec_init()\n");
		ok=0;
	}
	if (ok && strcmp(keyword,key))
	{
		bgc_printf(BV_ERROR, "Expecting keyword --> %s in %s\n",key,init.name);
		ok=0;
	}

	/* begin reading constants from *.init */
	if (ok && scan_value(init, &sitec->soil_depth, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading soil depth, sitec_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &sand, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading percent sand, sitec_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &silt, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading percent clay, sitec_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &clay, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading percent clay, sitec_init()\n");
		ok=0;
	}
	
	if (ok && scan_value(init, &sitec->elev, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading elevation, sitec_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &sitec->lat, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading site latitude, sitec_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &sitec->sw_alb, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading shortwave albedo, sitec_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &sitec->ndep, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading N deposition, sitec_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &sitec->nfix, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading N fixation, sitec_init()\n");
		ok=0;
	}
	
	/* calculate the soil pressure-volume coefficients from texture data */
	/* Uses the multivariate regressions from Cosby et al., 1984 */
	/* first check that the percentages add to 100.0 */
	if (ok && fabs(sand+silt+clay-100) > FLT_COND_TOL)
	{
		bgc_printf(BV_ERROR, "Error: %%sand + %%silt + %%clay  MUST EQUAL 100%%\n");
		bgc_printf(BV_ERROR, "Check values in initialization file.\n");
		ok=0;
	}
	if (ok)
	{
		sitec->soil_b = -(3.10 + 0.157*clay - 0.003*sand);
		sitec->vwc_sat = (50.5 - 0.142*sand - 0.037*clay)/100.0;
		sitec->psi_sat = -(exp((1.54 - 0.0095*sand + 0.0063*silt)*log(10.0))*9.8e-5);
		sitec->vwc_fc = sitec->vwc_sat*pow((-0.015/sitec->psi_sat),1.0/sitec->soil_b);
	
		/* define maximum soilwater content, for outflow calculation
		converts volumetric water content (m3/m3) --> (kg/m2) */
		sitec->soilw_fc = sitec->soil_depth * sitec->vwc_fc * 1000.0;
		sitec->soilw_sat = sitec->soil_depth * sitec->vwc_sat * 1000.0;
	}
	
	return (!ok);
}
