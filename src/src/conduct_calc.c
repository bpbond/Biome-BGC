/* 
conduct_calc.c
Calculation of conductance values based on limitation factors (in original BBGC this subroutine is partly included in canopy_et.c)
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
BBGC MuSo v4
Copyright 2000, Peter E. Thornton
Numerical Terradynamics Simulation Group
Copyright 2014, D. Hidy (dori.hidy@gmail.com)
Hungarian Academy of Sciences
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

int conduct_calc(const control_struct* ctrl, const metvar_struct* metv, const epconst_struct* epc, const siteconst_struct* sitec, 
                 epvar_struct* epv, int simyr)
{
	int ok=1;
	double gl_bl, gl_c, gl_s_sun, gl_s_shade;
	double gl_e_wv, gl_t_wv_sun, gl_t_wv_shade, gl_sh;
	double gc_e_wv, gc_sh;
	double m_ppfd_sun, m_ppfd_shade;
	double m_tmin, m_co2, m_vpd, m_final_sun, m_final_shade;
	double gcorr;

	/* Hidy 2013 - changing MSC value */
	double max_conduct;


	/* Hidy 2010. - multiplier for soil properties in multilayer soil (instead of psi: vwc) */
	int layer;
	double m_vwcR_layer; 
	double vwc_ratio;	
	double m_soilstress_avg = 0;

	/* Hidy 2015 - m_co2 */
	double p_co2;


	
	/* temperature and pressure correction factor for conductances */
	gcorr = pow((metv->tday+273.15)/293.15, 1.75) * 101300/metv->pa;
	
	/* calculate leaf- and canopy-level conductances to water vapor and
	sensible heat fluxes */
	
	/* leaf boundary-layer conductance */
	gl_bl = epc->gl_bl * gcorr;
	
	/* leaf cuticular conductance */
	gl_c = epc->gl_c * gcorr;
	
	/* leaf stomatal conductance: first generate multipliers, then apply them
	to maximum stomatal conductance */
	/* calculate stomatal conductance radiation multiplier: 
	*** NOTE CHANGE FROM BIOME-BGC CODE ***
	The original Biome-BGC formulation follows the arguments in 
	Rastetter, E.B., A.W. King, B.J. Cosby, G.M. Hornberger, 
	   R.V. O'Neill, and J.E. Hobbie, 1992. Aggregating fine-scale 
	   ecological knowledge to model coarser-scale attributes of 
	   ecosystems. Ecological Applications, 2:55-70.

	gmult->max = (gsmax/(k*lai))*log((gsmax+rad)/(gsmax+(rad*exp(-k*lai))))

	I'm using a much simplified form, which doesn't change relative shape
	as gsmax changes. See Korner, 1995.
	*/

	/* photosynthetic photon flux density conductance control */
	m_ppfd_sun = metv->ppfd_per_plaisun/(PPFD50 + metv->ppfd_per_plaisun);
	m_ppfd_shade = metv->ppfd_per_plaishade/(PPFD50 + metv->ppfd_per_plaishade);

	
	/* ***************************************************************************/
	/* ***************************************************************************/
	/* MULTIPLIERS */

	/* ******************/
	/* 1. actual maximum stomatal conductance: CO2 multiplier */
	
	
	if (epc->CO2conduct_flag)
	{
		p_co2 = 39.43 * pow(360, -0.64);
		m_co2 = 39.43 * pow(metv->co2, -0.64) / p_co2;
	}
	else
		m_co2 = 1;


	/* Hidy 2015 - changing MSC value taking into account the effect of CO2 concentration */
	if (ctrl->varMSC_flag && ctrl->spinup != 1)
		max_conduct=epc->msc_array[simyr] * m_co2;
	else
		max_conduct=epc->gl_smax * m_co2;

	/* ******************/
	/* 2. Soil water content
		  Hidy 2014 - calculate the multipiers for soil properties (soil water content ratio) in multilayer soil  - Jarvis (1989)*/	

	for (layer = 0; layer < epv->n_rootlayers; layer++)
	{

		if (epv->vwc[layer] > sitec->vwc_wp[layer])
			vwc_ratio  = (epv->vwc[layer] - sitec->vwc_wp[layer])/(sitec->vwc_sat[layer] - sitec->vwc_wp[layer]);
		else
			vwc_ratio  = 0;

		if (vwc_ratio < epv->vwc_ratio_crit1[layer])    /* water stress due to drought */
		{
			m_vwcR_layer = vwc_ratio / epv->vwc_ratio_crit1[layer];
		}
		else
		{
			if (epv->vwc_ratio_crit1[layer] <= vwc_ratio && vwc_ratio <= epv->vwc_ratio_crit2[layer])    /* no water stress */
			{
				m_vwcR_layer = 1;
			}	
			else
			{
			   m_vwcR_layer = (1.01 - vwc_ratio) / (1.01 - epv->vwc_ratio_crit2[layer]);
			}
		}
	
		epv->m_soilstress_layer[layer] =  m_vwcR_layer;

		m_soilstress_avg	 += epv->m_soilstress_layer[layer] * epv->rootlength_prop[layer];

	}

	/* ******************/
	/* 3. freezing night minimum temperature multiplier */

	if (metv->tmin > 0.0)        /* no effect */
		m_tmin = 1.0;
	else
	if (metv->tmin < -8.0)       /* full tmin effect */
		m_tmin = 0.0;
	else                   /* partial reduction (0.0 to -8.0 C) */
		m_tmin = 1.0 + (0.125 * metv->tmin);

	/* ******************/	
	/* 3. vapor pressure deficit multiplier, vpd in Pa */

	if (metv->vpd < epc->vpd_open)    /* no vpd effect */
		m_vpd = 1.0;
	else
	if (metv->vpd > epc->vpd_close)   /* full vpd effect */
		m_vpd = 0.0;
	else                   /* partial vpd effect */
		m_vpd = (epc->vpd_close - metv->vpd) / (epc->vpd_close - epc->vpd_open);

	/* ******************/	
	/* 4. apply all multipliers to the maximum stomatal conductance */

	m_final_sun = m_ppfd_sun * m_soilstress_avg *  m_tmin * m_vpd;
	m_final_shade = m_ppfd_shade * m_soilstress_avg * m_tmin * m_vpd;
	gl_s_sun = max_conduct * m_final_sun * gcorr;
	gl_s_shade = max_conduct * m_final_shade * gcorr;

	/* ***************************************************************************/
	/* ***************************************************************************/

	/* calculate leaf-and canopy-level conductances to water vapor and
	sensible heat fluxes, to be used in Penman-Monteith calculations of
	canopy evaporation and canopy transpiration. */
	
	/* Leaf conductance to evaporated water vapor, per unit projected LAI */
	gl_e_wv = gl_bl;
		
	/* Leaf conductance to transpired water vapor, per unit projected
	LAI.  This formula is derived from stomatal and cuticular conductances
	in parallel with each other, and both in series with leaf boundary 
	layer conductance. */
	gl_t_wv_sun = (gl_bl * (gl_s_sun + gl_c)) / (gl_bl + gl_s_sun + gl_c);
	gl_t_wv_shade = (gl_bl * (gl_s_shade + gl_c)) / (gl_bl + gl_s_shade + gl_c);

	/* Leaf conductance to sensible heat, per unit all-sided LAI */
	gl_sh = gl_bl;
	
	/* Canopy conductance to evaporated water vapor */
	gc_e_wv = gl_e_wv * epv->proj_lai;
	
	/* Canopy conductane to sensible heat */
	gc_sh = gl_sh * epv->proj_lai;
	
	/* assign leaf-level conductance to transpired water vapor, 
	for use in calculating co2 conductance for farq_psn() */
	epv->gl_t_wv_sun = gl_t_wv_sun; 
	epv->gl_t_wv_shade = gl_t_wv_shade;
	
	/* assign output variables */
	epv->max_conduct			= max_conduct;
	epv->m_ppfd_sun     = m_ppfd_sun;
	epv->m_ppfd_shade   = m_ppfd_shade;
    epv->m_soilstress   = m_soilstress_avg;
	epv->m_tmin			= m_tmin;
	epv->m_vpd			= m_vpd;
	epv->m_final_sun	= m_final_sun;
	epv->m_final_shade	= m_final_shade;
	epv->gl_bl			= gl_bl;
	epv->gl_c			= gl_c;
	epv->gl_s_sun		= gl_s_sun;
	epv->gl_s_shade		= gl_s_shade;
	epv->gl_e_wv		= gl_e_wv;
	epv->gl_sh			= gl_sh;
	epv->gc_e_wv		= gc_e_wv;
	epv->gc_sh			= gc_sh;
	
	
    return (!ok);
}
