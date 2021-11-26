/*
bgc_constants.h
Holds macro definitions for constants used in bgc()

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2020, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
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
#define RAD2PAR     0.45     /* (ratio) ratio PAR / SWtotal  */
#define EPAR        4.55     /* (umol/J) PAR photon energy ratio */  
#define GRPNOW      1.0      /* (prop) proportion of storage growth resp at fixation */
#define PPFD50      75.0     /* (umol/m2/s) PPFD for 1/2 stomatal closure */
#define Q10_VALUE	2.0      /* q10 value for respiration calculation */

/* precision control */
/* This constant determines the lower limit of state variables before they
are set to 0.0 to control rounding and overflow errors */
#define CRIT_PREC 1e-12
#define CRIT_PREC_RIG 1e-14

/* spinup control */
/* maximum allowable trend in slow soil carbon at steady-state (kgC/m2/yr) 0.005 */
#define SPINUP_TOLERANCE_nw 0.005 
#define SPINUP_TOLERANCE_w 0.0005 

/* output control constants */
#define NMAP 4200

/* contants for multilayer soil model calculation */
#define HC_water			4200000.	/* (J/m3/K) heat capacity of water */
#define HC_soil				1260000.	/* (J/m3/K) heat capacity of soil */
#define HC_air				1004.		/* (J/m3/K) heat capacity of air */
#define water_density		1000.		/* (kg/m3)  mean value of water density */
#define pF_hygroscopw		6.2			/* (dimless)  pF value at hygroscopic water point (dry air) */
#define pF_wiltingpoint		4.2			/* (dimless)  pF value at wilting point */
#define pF_fieldcapacity	2.5			/* (dimless)  pF value at field capacity */
#define PD					2.65		/* (g/cm3) particle density */

/* contants for snow estimation in GSI calculation */
#define sn_abs				0.6			/* absorptivity of snow */
#define lh_fus				335.0		/* (kJ/kg) latent heat of fusion */
#define lh_sub				2845.0		/* (kJ/kg) latent heal of sublimation */
#define tcoef				0.65		/* (kg/m2/deg C/d) temp. snowmelt coef */

#define nDAYS_OF_YEAR	    365	
#define nMONTHS_OF_YEAR	    12	
#define nSEC_IN_DAY		    86400					/* (s/day)  calculation of daily value from second values */
#define nHOURS_IN_DAY		24.						/* (hour/day)  calculation of daily value from hourly values */
#define DATA_GAP			-9999
#define Celsius2Kelvin      273.15
#define m2_to_ha            10000
#define kg_to_t            0.001

/* constant for net radiation calculation */
#define PI                  3.14159265358979323846
#define STEFAN_BOLTZMANN	0.000000004901
#define C_to_K				273.16

/* FILENAMESIZE */
#define FILENAMESIZE        128
#define STRINGSIZE          200

/* NO3 mobilen proportion */
#define NO3_mobilen_prop    1


