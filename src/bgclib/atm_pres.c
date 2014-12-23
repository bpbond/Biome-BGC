/* 
atm_pres.c
estimate atmospheric pressure as a function of elevation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "bgc.h"

int atm_pres(double elev, double* pa)
{
	/* daily atmospheric pressure (Pa) as a function of elevation (m) */
	/* From the discussion on atmospheric statics in:
	Iribane, J.V., and W.L. Godson, 1981. Atmospheric Thermodynamics, 2nd
		Edition. D. Reidel Publishing Company, Dordrecht, The Netherlands.
		(p. 168)
	*/
	
	int ok=1;
	double t1,t2;
	
	t1 = 1.0 - (LR_STD * elev)/T_STD;
	t2 = G_STD / (LR_STD * (R / MA));
	*pa = P_STD * pow(t1,t2);
	
	return(!ok);
}
