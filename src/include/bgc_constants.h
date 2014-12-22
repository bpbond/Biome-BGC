/*
bgc_constants.h
Holds macro definitions for constants used in bgc()

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.1.1
Copyright 2000, Peter E. Thornton
Numerical Terradynamics Simulation Group (NTSG)
School of Forestry, University of Montana
Missoula, MT 59812
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

/* atmospheric constants */
/* from the definition of the standard atmosphere, as established
by the International Civil Aviation Organization, and referenced in:

Iribane, J.V., and W.L. Godson, 1981. Atmospheric Thermodynamics. 2nd 
	Edition. D. Reidel Publishing Company, Dordrecht, The Netherlands.
	(pp 10,167-168,245)
*/
#define G_STD    9.80665         /* (m/s2) standard gravitational accel. */ 
#define P_STD    101325.0        /* (Pa) standard pressure at 0.0 m elevation */
#define T_STD    288.15          /* (K) standard temp at 0.0 m elevation  */ 
#define MA       28.9644e-3      /* (kg/mol) molecular weight of air */
#define MW       18.0148e-3      /* (kg/mol) molecular weight of water */
#define CP       1010.0          /* (J/kg K) specific heat of air */
#define LR_STD   0.0065          /* (-K/m) standard temperature lapse rate */
#define R        8.3143          /* (m3 Pa/ mol K) gas law constant */
#define SBC      5.67e-8         /* (W/(m2 K4)) Stefan-Boltzmann constant */
#define EPS      0.6219          /* (MW/MA) unitless ratio of molec weights */

/* ecosystem constants */
#define RAD2PAR     0.45     /* (DIM) ratio PAR / SWtotal  */
#define EPAR        4.55     /* (umol/J) PAR photon energy ratio */  
#define SOIL1_CN    12.0     /* C:N for fast microbial recycling pool */
#define SOIL2_CN    12.0     /* C:N for slow microbial recycling pool */
#define SOIL3_CN    10.0     /* C:N for recalcitrant SOM pool (humus) */
#define SOIL4_CN    10.0     /* C:N for recalcitrant SOM pool (humus) */
#define GRPERC      0.3      /* (DIM) growth resp per unit of C grown */
#define GRPNOW      1.0      /* (DIM) proportion of storage growth resp at fixation */
#define PPFD50      75.0     /* (umol/m2/s) PPFD for 1/2 stomatal closure */
#define DENITRIF_PROPORTION  0.01  /* fraction of mineralization to volatile */
#define MOBILEN_PROPORTION   0.1   /* fraction mineral N avail for leaching */

/* precision control */
/* This constant determines the lower limit of state variables before they
are set to 0.0 to control rounding and overflow errors */
#define CRIT_PREC 1e-20

/* spinup control */
/* maximum allowable trend in slow soil carbon at steady-state (kgC/m2/yr) */
#define SPINUP_TOLERANCE 0.0005

/* output control constants */
#define NMAP 700
