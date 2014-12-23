/* 
canopy_et.c
A single-function treatment of canopy evaporation and transpiration
fluxes.  

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "bgc.h"

int canopy_et(const metvar_struct* metv, const epconst_struct* epc, 
epvar_struct* epv, wflux_struct* wf, int verbose)
{
	int ok=1;
	double gl_bl, gl_c, gl_s_sun, gl_s_shade;
	double gl_e_wv, gl_t_wv_sun, gl_t_wv_shade, gl_sh;
	double gc_e_wv, gc_sh;
	double tday;
	double tmin;
	double dayl;
	double vpd,vpd_open,vpd_close;
	double psi,psi_open,psi_close;
	double m_ppfd_sun, m_ppfd_shade;
	double m_tmin, m_psi, m_co2, m_vpd, m_final_sun, m_final_shade;
	double proj_lai;
	double canopy_w;
	double gcorr;
	
	double e, cwe, t, trans, trans_sun, trans_shade, e_dayl,t_dayl;	
	pmet_struct pmet_in;

	/* assign variables that are used more than once */
	tday =      metv->tday;
	tmin =      metv->tmin;
	vpd =       metv->vpd;
	dayl =      metv->dayl;
	psi =       epv->psi;
	proj_lai =  epv->proj_lai;
	canopy_w =  wf->prcp_to_canopyw;
	psi_open =  epc->psi_open;
	psi_close = epc->psi_close;
	vpd_open =  epc->vpd_open;
	vpd_close = epc->vpd_close;

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

	/* soil-leaf water potential multiplier */
	if (psi > psi_open)    /* no water stress */
		m_psi = 1.0;        
	else
	if (psi <= psi_close)   /* full water stress */
	{
		m_psi = 0.0;      
	}
	else                   /* partial water stress */
		m_psi = (psi_close - psi) / (psi_close - psi_open);

	/* CO2 multiplier */
	m_co2 = 1.0;

	/* freezing night minimum temperature multiplier */
	if (tmin > 0.0)        /* no effect */
		m_tmin = 1.0;
	else
	if (tmin < -8.0)       /* full tmin effect */
		m_tmin = 0.0;
	else                   /* partial reduction (0.0 to -8.0 C) */
		m_tmin = 1.0 + (0.125 * tmin);
	
	/* vapor pressure deficit multiplier, vpd in Pa */
	if (vpd < vpd_open)    /* no vpd effect */
		m_vpd = 1.0;
	else
	if (vpd > vpd_close)   /* full vpd effect */
		m_vpd = 0.0;
	else                   /* partial vpd effect */
		m_vpd = (vpd_close - vpd) / (vpd_close - vpd_open);

	/* apply all multipliers to the maximum stomatal conductance */
	m_final_sun = m_ppfd_sun * m_psi * m_co2 * m_tmin * m_vpd;
	if (m_final_sun < 0.00000001) m_final_sun = 0.00000001;
	m_final_shade = m_ppfd_shade * m_psi * m_co2 * m_tmin * m_vpd;
	if (m_final_shade < 0.00000001) m_final_shade = 0.00000001;
	gl_s_sun = epc->gl_smax * m_final_sun * gcorr;
	gl_s_shade = epc->gl_smax * m_final_shade * gcorr;
	
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
	gc_e_wv = gl_e_wv * proj_lai;
	
	/* Canopy conductane to sensible heat */
	gc_sh = gl_sh * proj_lai;
	
	cwe = trans = 0.0;
	
	/* Assign values in pmet_in that don't change */
	pmet_in.ta = tday;
	pmet_in.pa = metv->pa;
	pmet_in.vpd = vpd;
	
	/* Canopy evaporation, if any water was intercepted */
	/* Calculate Penman-Monteith evaporation, given the canopy conductances to
	evaporated water and sensible heat.  Calculate the time required to 
	evaporate all the canopy water at the daily average conditions, and 
	subtract that time from the daylength to get the effective daylength for
	transpiration. */
	if (canopy_w)
	{
		/* assign appropriate resistance and radiation for pmet_in */
		pmet_in.rv = 1.0/gc_e_wv;
		pmet_in.rh = 1.0/gc_sh;
		pmet_in.irad = metv->swabs;
		
		/* call penman-monteith function, returns e in kg/m2/s */
		if (penmon(&pmet_in, 0, &e))
		{
			bgc_printf(BV_ERROR, "Error: penmon() for canopy evap... \n");
			ok=0;
		}
		
		/* calculate the time required to evaporate all the canopy water */
		e_dayl = canopy_w/e;
		
		if (e_dayl > dayl)  
		{
			/* day not long enough to evap. all int. water */
			trans = 0.0;    /* no time left for transpiration */
			cwe = e * dayl;   /* daylength limits canopy evaporation */
		}
		else                
		{
			/* all intercepted water evaporated */
			cwe = canopy_w;
			
			/* adjust daylength for transpiration */
			t_dayl = dayl - e_dayl;
			 
			/* calculate transpiration using adjusted daylength */
			/* first for sunlit canopy fraction */
			pmet_in.rv = 1.0/gl_t_wv_sun;
			pmet_in.rh = 1.0/gl_sh;
			pmet_in.irad = metv->swabs_per_plaisun;
			if (penmon(&pmet_in, 0, &t))
			{
				bgc_printf(BV_ERROR, "Error: penmon() for adjusted transpiration... \n");
				ok=0;
			}
			trans_sun = t * t_dayl * epv->plaisun;
			
			/* next for shaded canopy fraction */
			pmet_in.rv = 1.0/gl_t_wv_shade;
			pmet_in.rh = 1.0/gl_sh;
			pmet_in.irad = metv->swabs_per_plaishade;
			if (penmon(&pmet_in, 0, &t))
			{
				bgc_printf(BV_ERROR, "Error: penmon() for adjusted transpiration... \n");
				ok=0;
			}
			trans_shade = t * t_dayl * epv->plaishade;
			trans = trans_sun + trans_shade;
		}
	}    /* end if canopy_water */
	
	else /* no canopy water, transpiration with unadjusted daylength */
	{
		/* first for sunlit canopy fraction */
		pmet_in.rv = 1.0/gl_t_wv_sun;
		pmet_in.rh = 1.0/gl_sh;
		pmet_in.irad = metv->swabs_per_plaisun;
		if (penmon(&pmet_in, 0, &t))
		{
			bgc_printf(BV_ERROR, "Error: penmon() for adjusted transpiration... \n");
			ok=0;
		}
		trans_sun = t * dayl * epv->plaisun;
		
		/* next for shaded canopy fraction */
		pmet_in.rv = 1.0/gl_t_wv_shade;
		pmet_in.rh = 1.0/gl_sh;
		pmet_in.irad = metv->swabs_per_plaishade;
		if (penmon(&pmet_in, 0, &t))
		{
			bgc_printf(BV_ERROR, "Error: penmon() for adjusted transpiration... \n");
			ok=0;
		}
		trans_shade = t * dayl * epv->plaishade;
		trans = trans_sun + trans_shade;
	}
		
	/* assign water fluxes, all excess not evaporated goes
	to soil water compartment */
	wf->canopyw_evap = cwe;
	wf->canopyw_to_soilw = canopy_w - cwe;
	wf->soilw_trans = trans;
	
	/* assign leaf-level conductance to transpired water vapor, 
	for use in calculating co2 conductance for farq_psn() */
	epv->gl_t_wv_sun = gl_t_wv_sun; 
	epv->gl_t_wv_shade = gl_t_wv_shade;
	
	/* assign verbose output variables if requested */
	if (verbose)
	{
		epv->m_ppfd_sun  = m_ppfd_sun;
		epv->m_ppfd_shade  = m_ppfd_shade;
		epv->m_psi   = m_psi;
		epv->m_co2   = m_co2;
		epv->m_tmin  = m_tmin;
		epv->m_vpd   = m_vpd;
		epv->m_final_sun = m_final_sun;
		epv->m_final_shade = m_final_shade;
		epv->gl_bl   = gl_bl;
		epv->gl_c    = gl_c;
		epv->gl_s_sun   = gl_s_sun;
		epv->gl_s_shade = gl_s_shade;
		epv->gl_e_wv = gl_e_wv;
		epv->gl_sh   = gl_sh;
		epv->gc_e_wv = gc_e_wv;
		epv->gc_sh   = gc_sh;
	}
	
	return (!ok);
}

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

    int ok=1;
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
    
    return (!ok);
}
