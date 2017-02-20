/*
photosynthesis.c
C3/C4 photosynthesis model

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.3
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2016, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

Updated1:
26 February 1999, PET: After going through tests on the quantum yield andthe temperature responses for C3 and C4 photosynthesis, 
as suggested by Jim Collatz, I found that a modification to the J=f(I) equation was necessary to represent the quantum yields properly. 
Also changed the  kinetic constants Kc and Ko to match de Pury and Farquhar 1997 (see below).
Added a flag for the C3/C4 model, so that all variation is handled inside photosynthesis.c (change quantum yield, increase Ca for C4).
Updated2:
Hidy 2013: correction of c4 photosynthesis routine based on the work of Vittorio et al in Biome-BGC 4.3 beta
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "bgc_struct.h"
#include "bgc_func.h"
#include "bgc_constants.h"

int photosynthesis(const epconst_struct* epc, const metvar_struct* metv, psn_struct* psn)
{
	/*
	The following variables are assumed to be defined in the psn struct
	at the time of the function call:
	c3         (flag) set to 1 for C3 model, 0 for C4 model
	pa         (Pa) atmospheric pressure 
	co2        (ppm) atmospheric [CO2] 
	t          (deg C) air temperature
	lnc        (kg Nleaf/m2) leaf N concentration, per unit projected LAI 
	flnr       (kg NRub/kg Nleaf) fraction of leaf N in Rubisco
	ppfd       (umol photons/m2/s) PAR flux density, per unit projected LAI
	g          (umol CO2/m2/s/Pa) leaf-scale conductance to CO2, proj area basis
	dlmr       (umol CO2/m2/s) day leaf maint resp, on projected leaf area basis
	
	The following variables in psn struct are defined upon function return:
	Ci         (Pa) intercellular [CO2]
	Ca         (Pa) atmospheric [CO2]
	O2         (Pa) atmospheric [O2]
	gamma      (Pa) CO2 compensation point, in the absence of maint resp.
	Kc         (Pa) MM constant for carboxylation
	Ko         (Pa) MM constant for oxygenation
	Vmax       (umol CO2/m2/s) max rate of carboxylation
	Jmax       (umol electrons/m2/s) max rate electron transport
	J          (umol RuBP/m2/s) rate of RuBP regeneration
	Av         (umol CO2/m2/s) carboxylation limited assimilation
	Aj         (umol CO2/m2/s) RuBP regen limited assimilation
	A          (umol CO2/m2/s) final assimilation rate
	*/
	
	/* the weight proportion of Rubisco to its nitrogen content, fnr, is 
	calculated from the relative proportions of the basic amino acids 
	that make up the enzyme, as listed in the Handbook of Biochemistry, 
	Proteins, Vol III, p. 510, which references:
	Kuehn and McFadden, Biochemistry, 8:2403, 1969 */
	static double fnr = 7.16;   /* kg Rub/kg NRub */
	
	/* the following enzyme kinetic constants are from: 
	Woodrow, I.E., and J.A. Berry, 1980. Enzymatic regulation of photosynthetic
	CO2 fixation in C3 plants. Ann. Rev. Plant Physiol. Plant Mol. Biol.,
	39:533-594.
	Note that these values are given in the units used in the paper, and that
	they are converted to units appropriate to the rest of this function before
	they are used. */
	/* I've changed the values for Kc and Ko from the Woodrow and Berry
	reference, and am now using the values from De Pury and Farquhar,
	1997. Simple scaling of photosynthesis from leaves to canopies
	without the errors of big-leaf models. Plant, Cell and Env. 20: 537-557. 
	All other parameters, including the q10's for Kc and Ko are the same
	as in Woodrow and Berry. */
	static double Kc25 = 404.0;   /* (ubar) MM const carboxylase, 25 deg C */ 
	static double q10Kc = 2.1;    /* (DIM) Q_10 for Kc */
	static double Ko25 = 248.0;   /* (mbar) MM const oxygenase, 25 deg C */
	static double q10Ko = 1.2;    /* (DIM) Q_10 for Ko */
	static double act25 = 3.6;    /* (umol/mgRubisco/min) Rubisco activity */
	static double q10act = 2.4;   /* (DIM) Q_10 for Rubisco activity */

	static double pabs = 0.85;    /* (DIM) fPAR effectively absorbed by PSII */

	/* NEW - this is now held constant across both routines*/
	static double ppe = 2.6;  /*efficiency of photon absorbtion by photosystem II - mol photons absorbed/mol e- transported*/

	/* NEW - the following constants are specific to the new C4 photosynthesis routine (Di Vittorio et al 2010)*/
	static double alphap = 0.1; /*photon absorption efficiency of PEP Carboxylase, chen 1994 to the .1 precision, with unit correction; (umol pepcaseCO2 / umol photons) */
	static double fnp = 7.12; /* mass proportion of PEP Carboxylase to its N content - after Joseph White's algorithm description*/
	static double actp = 438333.333; /*activity of pep carboxylase - umol CO2/(kgPEP*sec); sage et al 1987 for amaranthus retroflexus; uedan et al. 1976 gives = 413333.333;  MIGHT NEED TO ADD A TEMPERATURE CORRECTION HERE */
	static double Kp25 = 82; /*ubar pep carboxylase Michaelis-Menton constant; remember ubar=ppm*/
	static double Q10Kp = 2.1; /*(DIM) Q10 for pep carboxylase activity*/
	static double Rbs = 0.000130; /* (m2s/umol) - bundle sheath resistance to co2 flux perleaf area, bundle sheath area bases -- average of 3 types of C4 plant, von Caemmerer 2003*/

	/* acclimation - Hidy 2015 */
	static double acclim_a  = 2.59;
	static double acclim_b  = -0.035;
	double acclim_rVJ;
	
	double flnp = psn->flnp;

	
	/* local variables */
	int ok=1;	
	double t;      /* (deg C) temperature */
	double Kc;     /* (Pa) MM constant for carboxylase reaction */
	double Ko;     /* (Pa) MM constant for oxygenase reaction */
	double act;    /* (umol CO2/kgRubisco/s) Rubisco activity */
	double Jmax;   /* (umol/m2/s) max rate electron transport */
	
	double Vmax, J, gamma, Ca, Rd, O2, g;
	double a,b,c,det;
	double Av,Aj, A;

	/* NEW -  C4 local variable additions*/
	double Vpm, V4m, V4;	 /* umol pepcaseCO2/(m2s) */
	double Cm,Om,Kp;		 /* (Pa) */
	double rbs;				 /* (m2sPa/umol)*/
	
	/* begin by assigning local variables */
	g = psn->g;
	t = psn->t;
	Rd = psn->dlmr;
	rbs = Rbs * psn->pa;	/* use atmospheric pressure to convert from m2s/umol to m2sPa/umol */
	
	/* convert atmospheric CO2 from ppm --> Pa */
	Ca = psn->co2 * psn->pa / 1e6;
	
	/* calculate atmospheric O2 in Pa, assumes 21% O2 by volume */
	psn->O2 = O2 = 0.21 * psn->pa;
	
    /***********************************************************************/
	/* 1. Make calculations that are common to both the C3 and C4 routines */	

	/* correct kinetic constants for temperature, and do unit conversions */
	Ko = Ko25 * pow(q10Ko, (t-25.0)/10.0);
	psn->Ko = Ko = Ko * 100.0;   /* mbar --> Pa */
	if (t > 15.0)
	{
		Kc = Kc25 * pow(q10Kc, (t-25.0)/10.0);
		act = act25 * pow(q10act, (t-25.0)/10.0);
	}
	else
	{
		Kc = Kc25 * pow(1.8*q10Kc, (t-15.0)/10.0) / q10Kc;
		act = act25 * pow(1.8*q10act, (t-15.0)/10.0) / q10act;
	}
	psn->Kc = Kc = Kc * 0.10;   /* ubar --> Pa */
	act = act * 1e6 / 60.0;     /* umol/mg/min --> umol/kg/s */
	psn->Ca = Ca;
	/* calculate gamma (Pa), assumes Vomax/Vcmax = 0.21 */
	psn->gamma = gamma = 0.5 * 0.21 * Kc * psn->O2 / Ko;
	 
	/* calculate Vmax from leaf nitrogen data and Rubisco activity */
	
	/* kg Nleaf   kg NRub    kg Rub      umol            umol 
	   -------- X -------  X ------- X ---------   =   --------
	      m2      kg Nleaf   kg NRub   kg RUB * s       m2 * s       
	   
	     (lnc)  X  (flnr)  X  (fnr)  X   (act)     =    (Vmax)
	*/
	psn->Vmax = Vmax = psn->lnc * psn->flnr * fnr * act;
	
	/* calculate Jmax = f(Vmax), reference:
	Wullschleger, S.D., 1993.  Biochemical limitations to carbon assimilation
		in C3 plants - A retrospective analysis of the A/Ci curves from
		109 species. Journal of Experimental Botany, 44:907-920.
	*/

	psn->Jmax = Jmax = 1.97*Vmax;
	if (epc->acclimation_flag == 2 || epc->acclimation_flag == 3)
	{
		acclim_rVJ = acclim_a + acclim_b * metv->tavg30_ra;
		psn->Jmax  = acclim_rVJ * Vmax;

	}

		
	/* calculate J = f(Jmax, ppfd), reference:
	de Pury and Farquhar 1997
	Plant Cell and Env.
	*/
	a = 0.7;
	b = -Jmax - (psn->ppfd*pabs/ppe);
	c = Jmax * psn->ppfd*pabs/ppe;
	psn->J = J = (-b - sqrt(b*b - 4.0*a*c))/(2.0*a);

	/***********************************************************************/
	/* 2. c3 photosynt. */

	if (psn->c3)
	  {
		/* solve for Av and Aj using the quadratic equation, substitution for Ci
		from A = g(Ca-Ci) into the equations from Farquhar and von Caemmerer:
			 
			   Vmax (Ci - gamma)
		Av =  -------------------   -   Rd
			  Ci + Kc (1 + O2/Ko)
		
		
				 J (Ci - gamma)
		Aj  =  -------------------  -   Rd
			   4.5 Ci + 10.5 gamma  
		*/

		/* quadratic solution for Av */    
		a = -1.0/g;
		b = Ca + (Vmax - Rd)/g + Kc*(1.0 + O2/Ko);
		c = Vmax*(gamma - Ca) + Rd*(Ca + Kc*(1.0 + O2/Ko));
    
		if ((det = b*b - 4.0*a*c) < 0.0)
		{
    		printf("negative root error in psn routine\n");
    		ok=0;
		}
    
		psn->Av = Av = (-b + sqrt(det)) / (2.0*a);
    
		/* quadratic solution for Aj */
		a = -4.5/g;    
		b = 4.5*Ca + 10.5*gamma + J/g - 4.5*Rd/g;
		c = J*(gamma - Ca) + Rd*(4.5*Ca + 10.5*gamma);
			
		if ((det = b*b - 4.0*a*c) < 0.0)
		{
			printf("negative root error in psn routine\n");
			ok=0;
		}
		
		psn->Aj = Aj = (-b + sqrt(det)) / (2.0*a);
		
	  } 
	/***********************************************************************/
	/* 3. c4 photosynt. */
	else 
	{
		/* This is based on the equations above but with Ci = Cbs = Cm + rbs*(V4 - A)
		   From Chen et al. 1994
		   Cm = mesophyll CO2 concentration
		   Cbs = bundle sheat CO2 concentration
		   rbs = total bundle sheath resistance
		   A = either Av or Aj
		   V4 = PEPCase C4 cycle rate dependent on CO2 concentration
		   Vb = diffusion flux of CO2 between mesophyll and bundle sheath
		   V4m = maximum V4 velocity at given radiation, modelled after a differnt J than above
		   Ip = incident PAR photon flux density
		   Vpm = maximum PEPCase carboxilation rate
		   alpha = photon absorption efficiency of PEPCase (see parameter above)
		   kp = michaelis-menten constant
		   V4m = (alpha* Ip) / (1 + ((alpha * Ip) / Vpm)^2 )^0.5 
		   V4 = (V4m*Cm) / (Cm + kp)
		   V4 = Vb + A
		   Vb = (Cbs - Cm)/rbs
		*/
	
		/*perform C4 specific tempreature corrections*/
		Kp = Kp25 * pow(Q10Kp, (t-25.0)/10.0);	// ubar=ppm
	    
		/* Next two lines are after Long 1991 */
		/* assumes that leaf temperature equals air temperature */
		Cm = 0.7 * psn->co2 * ((1.674 - 0.061294 * t + 0.001688 * t * t - 0.0000088741 * t * t * t) / 0.73547); // this is currenlty in ppm
		Om = 210000*((0.047 - 0.0013087 * t + 0.000025603 * t * t - 0.00000021441 * t * t * t)/0.026934); // this is currently in ppm
		// convert these to Pa
		Cm = Cm * psn->pa / 1e6;
		Om = Om * psn->pa / 1e6;
		Kp = Kp * 0.10;
		
		/* calculate gamma (Pa) with Obs=Om, assumes Vomax/Vcmax = 0.21 */
		psn->gamma = gamma = 0.5 * 0.21 * Kc * Om / Ko;
		
		Vpm = psn->lnc * flnp * fnp * actp; /*Maximum PEP carboxylase rate.  If fnp is not made an epc parameter, this can be simplified by premultiplying fnp and flnp above*/
		V4m = (alphap * psn->ppfd) / (sqrt(1.0 + alphap * alphap * psn->ppfd * psn->ppfd/(Vpm*Vpm))); /*Maximum radiation limited pep carboxylase rate*/
		V4  = (V4m * Cm)/(Cm + Kp); /*Actual Carboxylation rate - PEP Carboxylase*/
		/* Solve First Quadratic - Av*/
		// for now use Om = Obs because the values for the O2 diffusion are not available -- Chen et al. 1994 do this
		a = -rbs;
		b = Cm + rbs * (V4 + Vmax - Rd) + Kc * (1 + Om / Ko);
		c = Rd * (Cm + rbs * V4 + Kc * (1 + Om / Ko)) + Vmax * (gamma - Cm - rbs * V4);
		
		if ((det = b*b - 4.0*a*c) < 0.0)
		{
			printf("negative root error in psn routine\n");
			ok=0;
		}
		psn->Av = Av = (-b + sqrt(det)) / (2.0*a);
		/* Solve Second Quaratic - Aj*/
		a = -4.5 * rbs;
		b = 4.5 * Cm + 10.5 * gamma + rbs * (4.5 * V4 + J - 4.5 * Rd);
		c = Rd * (4.5 * Cm + 4.5 * rbs * V4 + 10.5 * gamma) + J * (gamma - Cm - rbs * V4);
		if ((det = b*b - 4.0*a*c) < 0.0)
		{
			printf("negative root error in psn routine\n");
			ok=0;
		}
		
		psn->Aj = Aj = (-b + sqrt(det)) / (2.0*a);	
	}
	
	/***********************************************************************/
	/* 4. estimate A as the minimum of (Av,Aj)  */

	if (Av < Aj) A = Av; 
	else         A = Aj;
	psn->A = A;
	psn->Ci = Ca - (A/g);
	
	return (!ok);
}	

