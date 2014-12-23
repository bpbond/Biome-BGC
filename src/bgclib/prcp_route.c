/* 
prcp_route.c
routing of daily precipitation to canopy, soil, snowpack

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "bgc.h"

int prcp_route(const metvar_struct* metv, double precip_int_coef,
double all_lai, wflux_struct* wf) 
{
	/* Precipitation routing, as either rain or snow.  Rain can be
	intercepted on the canopy, and amount in excess of interception is
	routed to the soil.  The intercepted volume is saved in a temporary
	variable that is later passed to the canopy evaporation routine,
	but there is no day-to-day storage of water on the canopy: any that is
	not evaporated in one day is routed to the soil in canopy_et().
	
	There is no canopy interception of snow.
	*/
	
	int ok=1;
	double max_int;
	double prcp, through;

	prcp = metv->prcp;

	/* maximum daily canopy interception 
	(kg intercepted/kg rain/unit all-sided LAI/day) */
	max_int = precip_int_coef * prcp * all_lai;

	/* rain vs. snow, and canopy interception */
	if (metv->tavg > 0.0)             /* rain */
	{
		if (prcp <= max_int)          /* all intercepted */
		{
			wf->prcp_to_canopyw = prcp; 
			through = 0.0;
		}
		else                          /* canopy limits interception */
		{
			wf->prcp_to_canopyw = max_int;
			through = prcp - max_int;
		}
		
		wf->prcp_to_soilw = through;  /* throughfall to soil water */
	}
	else                              /* snow */
	{
		wf->prcp_to_snoww = prcp;     /* no interception */
	}
	
	return(!ok);
}
