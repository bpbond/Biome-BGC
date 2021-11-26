/* 
conduct_calc.c
Calculation of conductance values based on limitation factors (in original BBGC this subroutine is partly included in canopy_et.c)
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2020, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
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

int conduct_calc(const control_struct* ctrl, const metvar_struct* metv, const epconst_struct* epc, epvar_struct* epv, int simyr)
{
	int errorCode=0;	
	
	
	double p_co2;
	double gl_s_sunPOT,gl_s_shadePOT;

	p_co2=0;
	
	
	/*********************************************************************************************/
	/* 1. calculate boundary layer and cuticular conductance */
	
	/* temperature and pressure correction factor for conductances */
	epv->gcorr = pow((metv->tday+273.15)/293.15, 1.75) * 101300/metv->pa;
	
	/* leaf boundary-layer conductance */
	epv->gl_bl = epc->gl_bl * epv->gcorr;
	
	/* leaf cuticular conductance */
	epv->gl_c = epc->gl_c * epv->gcorr;
	

	/*********************************************************************************************/
	/* 2. leaf stomatal conductance: first generate multipliers, then apply them to maximum stomatal conductance */
	
	/*-----------------------*/
	/* 2.1  CO2 multiplier */
	if (epc->CO2conduct_flag)
	{
		p_co2 = 39.43 * pow(360, -0.64);
		epv->m_co2 = 39.43 * pow(metv->co2, -0.64) / p_co2;
	}
	else
		epv->m_co2 = 1;

	/*-----------------------*/
	/* 2.2 changing MSC value taking into account the effect of CO2 concentration */
	if (ctrl->varMSC_flag)
		epv->stomaCONDUCT_max=epc->MSC_array[simyr] *  epv->gcorr  * epv->m_co2;
	else
		epv->stomaCONDUCT_max=epc->gl_sMAX * epv->gcorr * epv->m_co2;

	/*-----------------------*/
	/* 2.3 photosynthetic photon flux density conductance control (radiation multiplier) */
	epv->m_ppfd_sun = metv->ppfd_per_plaisun/(PPFD50 + metv->ppfd_per_plaisun);
	epv->m_ppfd_shade = metv->ppfd_per_plaishade/(PPFD50 + metv->ppfd_per_plaishade);

	/*-----------------------*/
	/* 2.4 freezing night minimum temperature multiplier */
	if (metv->tmin > 0.0)        /* no effect */
		epv->m_tmin = 1.0;
	else
	if (metv->tmin < -8.0)       /* full tmin effect */
		epv->m_tmin = 0.0;
	else                   /* partial reduction (0.0 to -8.0 C) */
		epv->m_tmin = 1.0 + (0.125 * metv->tmin);

	/*-----------------------*/
	/* 2.5 vapor pressure deficit multiplier, vpd in Pa */
	if (metv->vpd < epc->vpd_open)    /* no vpd effect */
		epv->m_vpd = 1.0;
	else
	if (metv->vpd > epc->vpd_close)   /* full vpd effect */
		epv->m_vpd = 0.0;
	else                   /* partial vpd effect */
		epv->m_vpd = (epc->vpd_close - metv->vpd) / (epc->vpd_close - epc->vpd_open);

	
	/*-----------------------*/
	/* 2.7. apply all multipliers to the maximum stomatal conductance */
	
	epv->m_final_sun   = epv->m_SWCstress * epv->m_tmin * epv->m_vpd * epv->m_ppfd_sun;
	epv->m_final_shade = epv->m_SWCstress * epv->m_tmin * epv->m_vpd * epv->m_ppfd_shade;

	epv->gl_s_sun      = epv->stomaCONDUCT_max * epv->m_final_sun;
	epv->gl_s_shade    = epv->stomaCONDUCT_max * epv->m_final_shade;

	gl_s_sunPOT    = epv->stomaCONDUCT_max * epv->m_tmin * epv->m_vpd * epv->m_ppfd_sun; 
	gl_s_shadePOT  = epv->stomaCONDUCT_max * epv->m_tmin * epv->m_vpd * epv->m_ppfd_shade; 

	/* 2.8 Leaf conductance to transpired water vapor, per unit projected LAI.  This formula is derived from stomatal and cuticular conductances
	in parallel with each other, and both in series with leaf boundary layer conductance. */
	epv->gl_t_wv_sun   = (epv->gl_bl * (epv->gl_s_sun + epv->gl_c)) / (epv->gl_bl + epv->gl_s_sun + epv->gl_c);
	epv->gl_t_wv_shade = (epv->gl_bl * (epv->gl_s_shade + epv->gl_c)) / (epv->gl_bl + epv->gl_s_shade + epv->gl_c);

	epv->gl_t_wv_sunPOT   = (epv->gl_bl * (gl_s_sunPOT + epv->gl_c)) / (epv->gl_bl + epv->gl_s_sun + epv->gl_c);
	epv->gl_t_wv_shadePOT = (epv->gl_bl * (gl_s_shadePOT + epv->gl_c)) / (epv->gl_bl + epv->gl_s_shade + epv->gl_c);
	/* *********************************************************************************************************/

	/* 3. calculate leaf-and canopy-level conductances to water vapor and sensible heat fluxes, to be used in 
	      Penman-Monteith calculations of canopy evaporation and canopy transpiration. */
	
	/* 3.1 Leaf conductance to evaporated water vapor, per unit projected LAI */
	epv->gl_e_wv = epv->gl_bl;
		
	/* 3.2 Leaf conductance to sensible heat, per unit all-sided LAI */
	epv->gl_sh = epv->gl_bl;
	
	/* 3.4 Canopy conductance to evaporated water vapor */
	epv->gc_e_wv = epv->gl_e_wv * epv->proj_lai;
	
	/* 3.5 Canopy conductane to sensible heat */
	epv->gc_sh = epv->gl_sh * epv->proj_lai;

	
	
    return (errorCode);
}
