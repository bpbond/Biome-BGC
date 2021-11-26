/* 
canopy_et.c
A single-function treatment of canopy evaporation and transpiration
fluxes.  

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group (NTSG)
School of Forestry, University of Montana
Missoula, MT 59812
*-*-*-*-*-*-*-*-*-*-*-*-*-*
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


int penmon(const pmet_struct* in, int out_flag,	double* et)
{
    /*
	Combination equation for determining evaporation and transpiration. 
	
    For output in units of (kg/m2/s)  out_flag = 0
    For output in units of (W/m2)     out_flag = 1
   
    INPUTS:
    in->ta     (deg C)   air temperature
    in->pa     (Pa)      air pressure
    in->vpd    (Pa)      vapor pressure deficit
    in->irad   (W/m2)    incident radient flux density
    in->rv     (s/m)     resistance to water vapor flux
    in->rh     (s/m)     resistance to sensible heat flux

    INTERNAL VARIABLES:
    rho    (kg/m3)       density of air
    CP     (J/kg/degC)   specific heat of air
    lhvap  (J/kg)        latent heat of vaporization of water
    s      (Pa/degC)     slope of saturation vapor pressure vs T curve

    OUTPUT:
    et     (kg/m2/s)     water vapor mass flux density  (flag=0)
    et     (W/m2)        latent heat flux density       (flag=1)
    */

    int errorCode=0;
    double ta;
    double rho,lhvap,s;
    double t1,t2,pvs1,pvs2,e,tk;
    double rr,rh,rhr,rv;
    double dt = 0.2;     /* set the temperature offset for slope calculation */
   
    /* assign ta (Celsius) and tk (Kelvins) */
    ta = in->ta;
    tk = ta + 273.15;
        
    /* calculate density of air (rho) as a function of air temperature */
    rho = 1.292 - (0.00428 * ta);
    
    /* calculate resistance to radiative heat transfer through air, rr */
    rr = rho * CP / (4.0 * SBC * (tk*tk*tk));
    
    /* resistance to convective heat transfer */
    rh = in->rh;
    
    /* resistance to latent heat transfer */
    rv = in->rv;
    
    /* calculate combined resistance to convective and radiative heat transfer,
    parallel resistances : rhr = (rh * rr) / (rh + rr) */
    rhr = (rh * rr) / (rh + rr);

    /* calculate latent heat of vaporization as a function of ta */
    lhvap = 2.5023e6 - 2430.54 * ta;

    /* calculate temperature offsets for slope estimate */
    t1 = ta+dt;
    t2 = ta-dt;
    
    /* calculate saturation vapor pressures at t1 and t2 */
    pvs1 = 610.7 * exp(17.38 * t1 / (239.0 + t1));
    pvs2 = 610.7 * exp(17.38 * t2 / (239.0 + t2));

    /* calculate slope of pvs vs. T curve, at ta */
    s = (pvs1-pvs2) / (t1-t2);
    
    /* calculate evaporation, in W/m^2  */
    e = ( ( s * in->irad ) + ( rho * CP * in->vpd / rhr ) ) /
( ( ( in->pa * CP * rv ) / ( lhvap * EPS * rhr ) ) + s );
    
    /* return either W/m^2 or kg/m^2/s, depending on out_flag */	
    if (out_flag)
    	*et = e;
    
    if (!out_flag)
    	*et = e / lhvap;
    

    return (errorCode);
}
