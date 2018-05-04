/* 
radtrans.c
calculate leaf area index, sun and shade fractions, and specific
leaf area for sun and shade canopy fractions, then calculate
canopy radiation interception and transmission 

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2018, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "bgc_struct.h"
#include "bgc_func.h"
#include "bgc_constants.h"

int radtrans(const control_struct* ctrl, const phenology_struct* phen, const cstate_struct* cs, const epconst_struct* epc, const siteconst_struct *sitec,
	         metvar_struct* metv, epvar_struct* epv)
{

	int ok=1;
	int pp;
	double proj_lai, leafcSUM, sla_avg;
	double albedo_sw, albedo_par;
	double sw,par;
	double swabs, swtrans;
	double parabs;
	double k;
	double k_sw, k_par;
	double swabs_plaisun, swabs_plaishade;
	double swabs_per_plaisun, swabs_per_plaishade;
	double parabs_plaisun, parabs_plaishade;
	double parabs_per_plaisun, parabs_per_plaishade, W_to_MJperDAY;
	double crit_albedo = 0.23;

	double lwRADnet, swRADnet, RADnet, RADnet_per_plaisun, RADnet_per_plaishade, RADcs, rad, plai_crit;
	double J, delta, omega_s, d_r, solar_constant, RADextter, f_cd, Tmax_K, Tmin_K, e_sat, e_act, fi;


	/*--------------------------------------------------------------------------------------------------------------------------------------------*/
	/* 1. calculate the projected leaf area and SLA for sun and shade fractions and the canopy transmission and absorption of shortwave radiation
	based on the Beer's Law assumption of radiation attenuation as a function of projected LAI. */

	/* 1.1 The following equations estimate the albedo and extinction coefficients for the shortwave and PAR spectra from the values given for the
	entire shortwave range (from Jones, H.G., 1992. Plants and Microclimate, 2nd	Edition. Cambridge University Press. pp. 30-38.) 
	These conversions are approximated from the information given in Jones. */
	
	
	if (epv->n_actphen >= epc->n_emerg_phenophase && epv->n_actphen > 0)
	{	

		/* Calculate the sum of leafC content */
		leafcSUM = 0;
		for (pp = 0; pp < epv->n_actphen; pp++)
		{
			if (cs->leafcSUM_phenphase[pp] > 0) leafcSUM += cs->leafcSUM_phenphase[pp];
		}

		/* Calculate whole-canopy projected and all-sided LAI */
		sla_avg = 0; 
		for (pp = 0; pp < epv->n_actphen; pp++)
		{
			if (cs->leafcSUM_phenphase[pp] > 0) sla_avg += epc->avg_proj_sla[pp] * cs->leafcSUM_phenphase[pp] / leafcSUM;
		}

		/* SLA calculation problem if leafc > 0, but no SLA - except of th first simulation day in case of evergreen biomes */
		if (cs->leafc > 0 && sla_avg == 0)
		{
			if (ctrl->simyr == 0 && ctrl->yday == 0)
				sla_avg = epc->avg_proj_sla[0];
			else
			{
				printf("\n");
				printf("ERROR: Zero SLA value in radtrans.c\n");
				ok=0;
			}
		}
				
		epv->proj_lai = cs->leafc * sla_avg;
		epv->all_lai = epv->proj_lai * epc->lai_ratio;
		epv->sla_avg = sla_avg;

		/* Calculate projected LAI for sunlit and shaded canopy portions */
		epv->plaisun = 1.0 - exp(-epv->proj_lai);
		epv->plaishade = epv->proj_lai - epv->plaisun;
		if (epv->plaishade < 0.0)
		{
			printf("\n");
			printf("ERROR: Negative plaishade\n");
			printf("LAI of shaded canopy = %lf\n",epv->plaishade);
			ok=0;
		}
		
		/* calculate the projected specific leaf area for sunlit and  shaded canopy fractions */
		epv->sun_proj_sla = (epv->plaisun + (epv->plaishade/epc->sla_ratio)) /
			cs->leafc;
		epv->shade_proj_sla = epv->sun_proj_sla * epc->sla_ratio;
	}
	else
	{
		epv->all_lai = 0.0;
		epv->proj_lai = 0.0;
		epv->plaisun = 0.0;
		epv->plaishade = 0.0;
		epv->sun_proj_sla = 0.0;
		epv->shade_proj_sla = 0.0;
	}
	

	k = epc->ext_coef;
	proj_lai = epv->proj_lai;

	/* calculate NDVI based on empirical estimation */
	epv->NDVI = 0.01 * pow(proj_lai,3) - 0.12 *  pow(proj_lai,2) + 0.48 * proj_lai + 0.02;
	
	
	
	/* calculate LAI dependent albedo */
	if (sitec->sw_alb < crit_albedo)
		albedo_sw = crit_albedo - (crit_albedo - sitec->sw_alb)* exp(-0.75*proj_lai);
	else
		albedo_sw = sitec->sw_alb;

	/* 1.2 calculate total shortwave absorbed */
	k_sw = k;
	sw = 0;
	sw = metv->swavgfd * (1.0 - albedo_sw);
	swabs = sw * (1.0 - exp(-k_sw*proj_lai));
	swtrans = sw - swabs;
	
	/* 1.3 calculate PAR absorbed */
	k_par = k * 1.0;
	albedo_par = sitec->sw_alb/3.0;
	par = metv->par * (1.0 - albedo_par);
	parabs = par * (1.0 - exp(-k_par*proj_lai));
	
	/* 1.4 calculate the total shortwave absorbed by the sunlit and shaded canopy fractions */

	swabs_plaisun = k_sw * sw * epv->plaisun;
	swabs_plaishade = swabs - swabs_plaisun;
	if (swabs_plaishade < 0.0)
	{
		printf("\n");
		printf("ERROR: negative swabs_plaishade (%lf)\n",swabs_plaishade);
		ok=0;
	}

	/* 1.5 convert this to the shortwave absorbed per unit LAI in the sunlit and  shaded canopy fractions */
	
	if (proj_lai > 0.0)
	{
		swabs_per_plaisun = swabs_plaisun/epv->plaisun;
		swabs_per_plaishade = swabs_plaishade/epv->plaishade;
	}
	else
	{
		swabs_per_plaisun = swabs_per_plaishade = 0.0;
	}

	/* 1.6 calculate the total PAR absorbed by the sunlit and shaded canopy fractions */
	parabs_plaisun = k_par * par * epv->plaisun;
	parabs_plaishade = parabs - parabs_plaisun;
	if (parabs_plaishade < 0.0)
	{	
		printf("\n");
		printf("FATAL ERROR: negative parabs_plaishade (%lf)\n",parabs_plaishade);
		ok=0;
	}

	/* 1.7 convert this to the PAR absorbed per unit LAI in the sunlit and shaded canopy fractions */
	if (proj_lai > 0.0)
	{
		parabs_per_plaisun = parabs_plaisun/epv->plaisun;
		parabs_per_plaishade = parabs_plaishade/epv->plaishade;
	}
	else
	{
		parabs_per_plaisun = parabs_per_plaishade = 0.0;
	}

	/*--------------------------------------------------------------------------------------------------------------------------------------------*/
	/* 2. calculate the net radiation based on THE ASCE STANDARDIZED REFERENCE EVAPOTRANSPIRATION EQUATION” prepared by 
	Task Committee on Standardization of Reference Evapotranspiration of the Environmental and Water Resources Institute */

	/* 2.1. constant values */
	W_to_MJperDAY = 1e-6 * metv->dayl; //(NSEC_IN_DAY); 

	/* (rad) latitude */
	fi= sitec->lat * (PI/180);   
	
	/* 2.2. net short-wave radiation: swRADnet. Dimensions: [rad]=MJ/m2/d-1; [swavgfd]=W/m2*; [swRADnet]=W/m2*/
	rad = metv->swavgfd * W_to_MJperDAY; 
	swRADnet = (1 - albedo_sw) * rad  / W_to_MJperDAY;
	
	/* 2.3. net outgoing long-wave-radation: lwRADnet */
	/* number of the year between 1 and 365: J */
	J = (ctrl->yday+1) * (2*PI/365);
	
	/* solar declination: delta */
	delta = 0.409 * sin(J - 1.39);

	/* sunset hour angle: omega_s */
	omega_s = acos(-tan(fi) * tan(delta));

	/* inverse relative distance factor fo earth-sun: d_r */
	d_r = 1 + 0.033 * cos(J);

	/* extraterrestial radiation: RADextter */
	solar_constant = 4.92; 
	RADextter = (24/PI) * solar_constant * d_r * (omega_s * sin(fi) * sin(delta) + cos(fi) * cos(delta) * sin(omega_s));

	/* calculated clear-sky radiation: RADcs */
	RADcs = (0.75 + 2e-5 * sitec->elev) *RADextter;
   
	/* cloudiness function: f_cd (limited between 0.05 and 1.0) */
	f_cd = 1.35 * rad/RADcs - 0.35;

	if (f_cd < 0.05) f_cd = 0.05;
	if (f_cd > 1.0)  f_cd = 1.0;
	
	
	/* max and min temperature */
	Tmax_K = metv->tmax + C_to_K;
	Tmin_K = metv->tmin + C_to_K;

	/* vapor pressure: e_act - dimensions: e_sat:hPa to kPa, VPD: Pa to kPa */
	e_sat = 6.11 * pow(10, (7.5 * metv->tmax)/(metv->tmax + C_to_K)) / 10;
	e_act = e_sat - metv->vpd / 1000;

	if (e_act < 0)
	{
		printf("\n");
		printf("ERROR: actual vapor pressure is negative in radtrans.c \n");
		ok=0;
	}
	
	/* lwRADnet [W/m2] */
	lwRADnet = (STEFAN_BOLTZMANN * f_cd * (0.34 - 0.14 * sqrt(e_act)) * ((pow(Tmax_K,4) + pow(Tmin_K,4))/2)) /  W_to_MJperDAY;
	 
	/* 2.4. net radiation: difference between net short-wave and net long wave */
	RADnet = swRADnet - lwRADnet;

	/* 2.5. NEW METHOD - based on Jiang et al (2015) */
	RADnet = epc->rad_param1 * swRADnet + epc->rad_param2;

	/* 2.6 convert this to the shortwave absorbed per unit LAI in the sunlit and  shaded canopy fractions  */
	
	if (proj_lai > 0.0 )
	{
		/* plai_crit: in order to avoid irrealistic per LAI values */
		plai_crit = 0.1; 
		if (epv->plaisun   > plai_crit)
			RADnet_per_plaisun   = (RADnet * (swabs_plaisun  /swabs)) / epv->plaisun;
		else
			RADnet_per_plaisun   = (RADnet * (swabs_plaisun  /swabs)) / plai_crit;
		
		if (epv->plaishade   > plai_crit)
			RADnet_per_plaishade  = (RADnet * (swabs_plaishade  /swabs)) / epv->plaishade;
		else
			RADnet_per_plaishade  = (RADnet * (swabs_plaishade  /swabs)) / plai_crit;
	}
	else
	{
		RADnet_per_plaisun = RADnet_per_plaishade = 0.0;
	}

	

	/* assign structure values */
	metv->swRADnet = swRADnet;
	metv->lwRADnet = lwRADnet;
	metv->RADnet = RADnet;
	metv->RADnet_per_plaisun = RADnet_per_plaisun;
	metv->RADnet_per_plaishade = RADnet_per_plaishade;

	metv->swabs = swabs;
	metv->swtrans = swtrans;
	metv->swabs_per_plaisun = swabs_per_plaisun;
	metv->swabs_per_plaishade = swabs_per_plaishade;
	
	/* calculate PPFD: assumes an average energy for PAR photon (EPAR, umol/J) unit conversion: W/m2 --> umol/m2/s. */
	metv->ppfd_per_plaisun = parabs_per_plaisun * EPAR;
	metv->ppfd_per_plaishade = parabs_per_plaishade * EPAR;
	metv->parabs = parabs;
	metv->parabs_plaisun = parabs_plaisun;
	metv->parabs_plaishade = parabs_plaishade;

	
	return (!ok);
}
