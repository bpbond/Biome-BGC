/*
bgc_func.h
header file for function prototypes

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2018, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

int soilb_estimation(double sand, double silt, double* soil_b, double* vwc_sat,double* vwc_fc, double* vwc_wp,  double* BD, double* RCN);

int output_map_init(double** output_map, phenology_struct* phen, metvar_struct* metv, wstate_struct* ws,
	wflux_struct* wf, cstate_struct* cs, cflux_struct* cf, nstate_struct* ns, nflux_struct* nf, 
	epvar_struct* epv, psn_struct* psn_sun, psn_struct* psn_shade, summary_struct* summary);

/*int output_map_init(double** output_map, const phenology_struct* phen, const metvar_struct* metv, const wstate_struct* ws,
	const wflux_struct* wf, const cstate_struct* cs, const cflux_struct* cf, const nstate_struct* ns, const nflux_struct* nf,
	const epvar_struct* epv, const psn_struct* psn_sun, const psn_struct* psn_shade, const summary_struct* summary);*/

int make_zero_flux_struct(wflux_struct* wf, cflux_struct* cf, nflux_struct* nf);

int atm_pres(double elev, double* pa);

int restart_input(const epconst_struct* epc, wstate_struct* ws, cstate_struct* cs, nstate_struct* ns, epvar_struct* epv, restart_data_struct* restart);

int firstday(const control_struct* ctrl, const siteconst_struct* sitec, const epconst_struct* epc, const cinit_struct* cinit,
	         phenology_struct* phen, epvar_struct* epv, cstate_struct* cs, nstate_struct* ns, metvar_struct* metv);

int zero_srcsnk(cstate_struct* cs, nstate_struct* ns, wstate_struct* ws, summary_struct* summary);

int dayphen(const control_struct* ctrl, const epconst_struct* epc, const phenarray_struct* phenarr, phenology_struct* phen);

int management(const control_struct* ctrl, fertilizing_struct* FRZ, grazing_struct* GRZ, harvesting_struct* HRV, 
			   mowing_struct* MOW,planting_struct* PLT, ploughing_struct* PLG, thinning_struct* THN, irrigation_struct* IRG);

int multilayer_hydrolparams(const siteconst_struct* sitec,  wstate_struct* ws, epvar_struct* epv);

int daymet(const metarr_struct* metarr, const siteconst_struct* sitec, 
		   metvar_struct* metv, double* tair_annavg_ptr, double snoww, int metday);

int phenphase(const control_struct* ctrl, const epconst_struct* epc, const siteconst_struct* sitec, 
	          phenology_struct *phen, metvar_struct *metv, epvar_struct* epv, cstate_struct* cs);
	int vernalization(const epconst_struct* epc, const metvar_struct* metv, phenology_struct *phen);
	int photoslow(const epconst_struct* epc, const metvar_struct* metv, phenology_struct *phen);

int multilayer_tsoil(const epconst_struct* epc, const siteconst_struct* sitec, const epvar_struct* epv, int yday, double snoww,
					 metvar_struct* metv);

int phenology(const control_struct* ctrl, const epconst_struct* epc, const cstate_struct* cs, const nstate_struct* ns, 
	          phenology_struct* phen, metvar_struct* metv,epvar_struct* epv, cflux_struct* cf, nflux_struct* nf);
	int leaf_litfall(const epconst_struct* epc, double litfallc, cflux_struct* cf, nflux_struct* nf);
	int froot_litfall(const epconst_struct* epc, double litfallc, cflux_struct* cf, nflux_struct* nf);
	int fruit_litfall(const epconst_struct* epc, double litfallc, cflux_struct* cf, nflux_struct* nf);
	int softstem_litfall(const epconst_struct* epc, double litfallc, cflux_struct* cf, nflux_struct* nf);
	int transfer_fromGDD(const control_struct* ctrl, const epconst_struct* epc, const cstate_struct* cs, const nstate_struct* ns, 
	                     phenology_struct *phen, metvar_struct *metv, epvar_struct* epv, cflux_struct* cf, nflux_struct* nf);

int multilayer_rootdepth(const control_struct* ctrl, const phenology_struct* phen, const epconst_struct* epc, const siteconst_struct* sitec, const cstate_struct* cs, epvar_struct* epv);

int radtrans(const control_struct* ctrl, const phenology_struct* phen, const cstate_struct* cs, const epconst_struct* epc, const siteconst_struct *sitec,
	         metvar_struct* metv, epvar_struct* epv);

int prcp_route(const metvar_struct* metv, double precip_int_coef,double all_lai, wflux_struct* wf);

int snowmelt(const metvar_struct* metv, wflux_struct* wf, double snoww);

int baresoil_evap(const epconst_struct* epc, const metvar_struct* metv, wflux_struct* wf, double* dsr_ptr);

int conduct_calc(const control_struct* ctrl, const metvar_struct* metv, const epconst_struct* epc, const siteconst_struct* sitec, 
				 epvar_struct* epv, int simyr);

int maint_resp(const cstate_struct* cs, const nstate_struct* ns, const epconst_struct* epc, const metvar_struct* metv, 
	           epvar_struct* epv, cflux_struct* cf);

int canopy_et(const epconst_struct* epc, const metvar_struct* metv, const epvar_struct* epv, wflux_struct* wf);
	int penmon(const pmet_struct* in, int out_flag,	double* et);

int photosynthesis(const epconst_struct* epc, const metvar_struct* metv, epvar_struct* epv, psn_struct* psn_sun, psn_struct* psn_shade, cflux_struct *cf); 
	int farquhar(const epconst_struct* epc, const metvar_struct* metv, psn_struct* psn);

int decomp(const metvar_struct* metv, const epconst_struct* epc, const siteconst_struct* sitec, const cstate_struct* cs, const nstate_struct* ns, 
	       epvar_struct* epv, cflux_struct* cf, nflux_struct* nf, ntemp_struct* nt);
	int CH4flux_estimation(const epconst_struct* epc, double BD, double VWC, double T, double* CH4_flux);

int daily_allocation(const epconst_struct* epc, const metvar_struct* metv, cstate_struct*cs,  nstate_struct* ns, 
	                 cflux_struct* cf, nflux_struct* nf, epvar_struct* epv, ntemp_struct* nt, double naddfrac);


int flowering_heatstress(const epconst_struct* epc, const metvar_struct *metv, epvar_struct* epv, cflux_struct* cf, nflux_struct* nf);

int annual_rates(const epconst_struct* epc, epvar_struct* epv);

int growth_resp(const epconst_struct* epc, cflux_struct* cf);

int multilayer_transpiration(const control_struct* ctrl, const siteconst_struct* sitec, const epvar_struct* epv, const wstate_struct* ws, wflux_struct* wf);

int irrigation(const control_struct* ctrl, const irrigation_struct* IRG, wflux_struct* wf);

int groundwater(const siteconst_struct* sitec, control_struct* ctrl, epvar_struct* epv, wstate_struct* ws, wflux_struct* wf);

int multilayer_hydrolprocess(const control_struct* ctrl, const siteconst_struct* sitec, const epconst_struct* epc,
							 epvar_struct* epv, wstate_struct* ws, wflux_struct* wf);
	int richards(const siteconst_struct* sitec, const epconst_struct* epc, epvar_struct* epv, wstate_struct* ws, wflux_struct* wf);
	int tipping(const siteconst_struct* sitec, const epconst_struct* epc, epvar_struct* epv, wstate_struct* ws, wflux_struct* wf);

int daily_water_state_update(const wflux_struct* wf, wstate_struct* ws);


int daily_CN_state_update(const epconst_struct* epc, control_struct* ctrl, epvar_struct* epv, cflux_struct* cf, nflux_struct* nf, cstate_struct* cs, nstate_struct* ns, 
	                      int alloc, int woody, int evergreen);
	int nsc_maintresp(const epconst_struct* epc, epvar_struct* epv, cflux_struct* cf, nflux_struct* nf, cstate_struct* cs, nstate_struct* ns);
	int CNratio_control(control_struct* ctrl, double CNratio, double cpool, double npool, double cflux, double nflux, double CNratio_flux);

int senescence(int yday, const epconst_struct* epc, const grazing_struct* GRZ, const metvar_struct* metv, const phenology_struct* phen, 
			   cstate_struct* cs, cflux_struct* cf, nstate_struct* ns, nflux_struct* nf, epvar_struct* epv);
	int genprog_senescence(const epconst_struct* epc, const metvar_struct* metv, epvar_struct* epv, cflux_struct* cf, nflux_struct* nf);

int mortality(const control_struct* ctrl, const epconst_struct* epc, const epvar_struct* epv, 
	          cstate_struct* cs, cflux_struct* cf, nstate_struct* ns, nflux_struct* nf, int simyr);	

int multilayer_sminn(const epconst_struct* epc, const epvar_struct* epv, const siteconst_struct* sitec, const cflux_struct* cf, 
	                 nstate_struct* ns, nflux_struct* nf);

int multilayer_leaching(const epconst_struct* epc, const epvar_struct* epv,
					    cstate_struct* cs, cflux_struct* cf, nstate_struct* ns, nflux_struct* nf, wstate_struct* ws, wflux_struct* wf);

int planting(control_struct* ctrl, const epconst_struct* epc, const siteconst_struct* sitec, const planting_struct* PLT, 
	         epvar_struct* epv, phenology_struct* phen, cstate_struct* cs, nstate_struct*ns, cflux_struct* cf, nflux_struct* nf);

int thinning(const control_struct* ctrl, const epconst_struct* epc, const thinning_struct* THN, 
	         cstate_struct* cs, nstate_struct* ns, wstate_struct* ws, cflux_struct* cf, nflux_struct* nf, wflux_struct* wf);

int mowing(const control_struct* ctrl, const epconst_struct* epc, const mowing_struct* MOW, const epvar_struct* epv, 
		   cstate_struct* cs, nstate_struct* ns, wstate_struct* ws, cflux_struct* cf, nflux_struct* nf, wflux_struct* wf);

int grazing(const control_struct* ctrl, const epconst_struct* epc, grazing_struct* GRZ, 
	        cstate_struct* cs, nstate_struct* ns, wstate_struct* ws, cflux_struct* cf, nflux_struct* nf, wflux_struct* wf);

int harvesting(const control_struct* ctrl, const epconst_struct* epc, const harvesting_struct* HRV, epvar_struct* epv, 
	           cstate_struct* cs, nstate_struct* ns, wstate_struct* ws, cflux_struct* cf, nflux_struct* nf, wflux_struct* wf);

int ploughing(const control_struct* ctrl, const epconst_struct* epc, siteconst_struct* sitec, metvar_struct* metv, epvar_struct* epv, ploughing_struct* PLG, 
			  cstate_struct* cs, nstate_struct* ns, wstate_struct* ws, cflux_struct* cf, nflux_struct* nf, wflux_struct* wf);

int fertilizing(const control_struct* ctrl, const siteconst_struct* sitec, fertilizing_struct* FRZ, epvar_struct* epv, 
				cstate_struct* cs, nstate_struct* ns, wstate_struct* ws, cflux_struct* cf, nflux_struct* nf, wflux_struct* wf);

int cutdown2litter(const epconst_struct* epc, const epvar_struct* epv, cstate_struct* cs, cflux_struct* cf, nstate_struct* ns, nflux_struct* nf);

int precision_control(wstate_struct* ws, cstate_struct* cs, nstate_struct* ns);

int check_water_balance(wstate_struct* ws, int first_balance);
int check_carbon_balance(cstate_struct* cs, int first_balance);
int check_nitrogen_balance(nstate_struct* ns, int first_balance);

int cnw_summary(int yday, const siteconst_struct* sitec, const metvar_struct* metv, const cstate_struct* cs, const cflux_struct* cf, 
	            const nstate_struct* ns, const nflux_struct* nf, const wflux_struct* wf, const epvar_struct* epv, summary_struct* summary);

int restart_output(const wstate_struct* ws, const cstate_struct* cs, const nstate_struct* ns, const epvar_struct* epv, restart_data_struct* restart);