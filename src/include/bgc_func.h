#ifndef BGC_FUNC_H
#define BGC_FUNC_H
/*
bgc_func.h
header file for function prototypes

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/
#include "bgc.h"

#ifdef __cplusplus
extern "C"
{
#endif

int output_map_init(double** output_map, metvar_struct* metv, wstate_struct* ws,
wflux_struct* wf, cstate_struct* cs, cflux_struct* cf, nstate_struct* ns,
nflux_struct* nf, phenology_struct* phen, epvar_struct* epv,
psn_struct* psn_sun, psn_struct* psn_shade, summary_struct* summary);
int make_zero_flux_struct(wflux_struct* wf, cflux_struct* cf,
nflux_struct* nf);
int atm_pres(double elev, double* pa);
int prephenology(const control_struct* ctrl, const epconst_struct* epc, 
const siteconst_struct* sitec, const metarr_struct* metarr,
phenarray_struct* phen);
int restart_input(control_struct* ctrl, wstate_struct* ws, cstate_struct* cs,
	nstate_struct* ns, epvar_struct* epv, int* metyr, 
	restart_data_struct* restart);
int restart_output(control_struct* ctrl, wstate_struct* ws,cstate_struct* cs,
	nstate_struct* ns, epvar_struct* epv, int metyr,
	restart_data_struct* restart);
int free_phenmem(phenarray_struct* phen);
int firstday(const epconst_struct* epc, const cinit_struct* cinit,
epvar_struct* epv, phenarray_struct* phen, cstate_struct* cs, nstate_struct* ns);
int precision_control(wstate_struct* ws, cstate_struct* cs, nstate_struct* ns);
int zero_srcsnk(cstate_struct* cs, nstate_struct* ns, wstate_struct* ws,
	summary_struct* summary);
int daymet(const metarr_struct* metarr, metvar_struct* metv, int metday);
int dayphen(const phenarray_struct* phenarr, phenology_struct* phen, int metday);
int phenology(const epconst_struct* epc, const phenology_struct* phen,
epvar_struct* epv, cstate_struct* cs, cflux_struct* cf, nstate_struct* ns,
nflux_struct* nf);
int leaf_litfall(const epconst_struct* epc, double litfallc,
cflux_struct* cf, nflux_struct* nf);
int froot_litfall(const epconst_struct* epc, double litfallc, 
cflux_struct* cf, nflux_struct* nf);
int radtrans(const cstate_struct* cs, const epconst_struct* epc, 
metvar_struct* metv, epvar_struct* epv, double albedo);
int prcp_route(const metvar_struct* metv, double precip_int_coef,
double all_lai, wflux_struct* wf); 
int snowmelt(const metvar_struct* metv, wflux_struct* wf, double snoww);
int baresoil_evap(const metvar_struct* metv, wflux_struct* wf, double* dsr_ptr);
int soilpsi(const siteconst_struct* sitec, double soilw, double* psi,
double* vwc_out);
int maint_resp(const cstate_struct* cs, const nstate_struct* ns,
const epconst_struct* epc, const metvar_struct* metv, cflux_struct* cf,
epvar_struct* epv);
int canopy_et(const metvar_struct* metv, const epconst_struct* epc, 
epvar_struct* epv, wflux_struct* wf, int verbose);
int penmon(const pmet_struct* in, int out_flag,	double* et);
int photosynthesis(psn_struct* psn);
int total_photosynthesis(const metvar_struct* metv, const epconst_struct* epc, epvar_struct* epv, cflux_struct* cf, psn_struct *psn_sun, psn_struct *psn_shade);
int outflow(const siteconst_struct* sitec, const wstate_struct* ws, wflux_struct* wf);
int decomp(double tsoil, const epconst_struct* epc, epvar_struct* epv, 
const siteconst_struct* sitec, cstate_struct* cs, cflux_struct* cf,
nstate_struct* ns, nflux_struct* nf, ntemp_struct* nt);
int daily_allocation(cflux_struct* cf, cstate_struct* cs,
nflux_struct* nf, nstate_struct* ns, epconst_struct* epc, epvar_struct* epv,
ntemp_struct* nt, double naddfrac, int mode);
int spinup_daily_allocation(cflux_struct* cf, cstate_struct* cs,
nflux_struct* nf, nstate_struct* ns, epconst_struct* epc, epvar_struct* epv,
ntemp_struct* nt, double naddfrac);
int annual_rates(const epconst_struct* epc, epvar_struct* epv);
int growth_resp(epconst_struct* epc, cflux_struct* cf);
int daily_water_state_update(wflux_struct* wf, wstate_struct* ws);
int daily_carbon_state_update(cflux_struct* cf, cstate_struct* cs,
int alloc, int woody, int evergreen);
int daily_nitrogen_state_update(nflux_struct* nf, nstate_struct* ns,
int alloc, int woody, int evergreen);
int nleaching(nstate_struct* ns, nflux_struct* nf, wstate_struct* ws, 
wflux_struct* wf);
int mortality(const epconst_struct* epc, cstate_struct* cs, cflux_struct* cf,
nstate_struct* ns, nflux_struct* nf);
int check_water_balance(wstate_struct* ws, int first_balance);
int check_carbon_balance(cstate_struct* cs, int first_balance);
int check_nitrogen_balance(nstate_struct* ns, int first_balance);
int csummary(cflux_struct* cf, cstate_struct* cs, summary_struct* summary);
int wsummary(wstate_struct* ws,wflux_struct* wf, summary_struct* summary);
int output_ascii(float arr[],int nvars, FILE *ptr); 
double get_co2(co2control_struct * co2,int simyr);		/* Added WMJ 03/16/2005 */
double get_ndep(ndepcontrol_struct * ndep,int simyr);	/* Added WMJ 03/16/2005 */
#ifdef __cplusplus
}
#endif

#endif
