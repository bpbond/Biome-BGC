/*
pointbgc_func.h
function prototypes for pointbgc
for use with pointbgc front-end to BBGC MuSo v4 library

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2020, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Modified:
13/07/2000: Added input of Ndep from file=> added simyears function to
ndep_init function. Changes are made by Galina Churkina.
*/
int writeErrorCode(int errorCode);
int met_init(file init, point_struct* point);
int restart_init(file init, restart_ctrl_struct* restart);
int time_init(file init, control_struct *ctrl);
int scc_init(file init, climchange_struct* scc);
int co2_init(file init, co2control_struct* co2, control_struct *ctrl);
int sitec_init(file init, siteconst_struct* sitec, control_struct *ctrl);
int ndep_init(file init, ndep_control_struct* ndep, control_struct *ctrl);
int epc_init(file init, epconst_struct* epc, control_struct* ctrl, int EPCfromINI);
int sprop_init(file init, siteconst_struct* sitec, soilprop_struct* sprop, control_struct* ctrl);
int mgm_init(file init, control_struct *ctrl, epconst_struct* epc, fertilizing_struct* FRZ, grazing_struct* GRZ, harvesting_struct* HRV, mowing_struct* MOW, 
	         planting_struct* PLT, ploughing_struct* PLG, thinning_struct* THN, irrigating_struct* IRG);
int simctrl_init(file init, epconst_struct* epc, control_struct* ctrl, planting_struct* PLT);
int wstate_init(file init, const siteconst_struct* sitec, const soilprop_struct* sprop, wstate_struct* ws);
int cnstate_init(file init, const epconst_struct* epc, const soilprop_struct* sprop, const siteconst_struct* sitec, 
	             cstate_struct* cs, cinit_struct* cinit, nstate_struct* ns);
int output_init(file init, int transient, harvesting_struct* HRV, output_struct* output);
int end_init(file init);
int metarr_init(point_struct* point, metarr_struct* metarr, const climchange_struct* scc, const siteconst_struct* sitec, const control_struct* ctrl);
int presim_state_init(wstate_struct* ws, cstate_struct* cs, nstate_struct* ns,
	cinit_struct* cinit);


int GSI_calculation(const metarr_struct* metarr, const siteconst_struct* sitec, epconst_struct* epc, 
	                phenarray_struct* phenarr, control_struct* ctrl);

int conduct_limit_factors(file logfile, const control_struct* ctrl, const soilprop_struct* sprop, const epconst_struct* epc, epvar_struct* epv);

int prephenology(file logfile, const epconst_struct* epc, const metarr_struct* metarr, 
	             planting_struct* PLT, harvesting_struct* HRV, control_struct* ctrl, phenarray_struct* phenarr);
	
	int date_to_doy(int month, int day);

int output_handling(int monday, int endday, control_struct* ctrl, 
                    double** output_map, double* dayarr, double* monavgarr, double* annavgarr, double* annarr, 
					file dayout, file monavgout, file annavgout, file annout);

	int doy_to_date(int yday, int* month, int* day, int from1);

int thinning_init(file init, const control_struct* ctrl, thinning_struct* THN);
int mowing_init(file init, const control_struct* ctrl, mowing_struct* MOW);
int grazing_init(file init, const control_struct* ctrl, grazing_struct* GRZ);
int harvesting_init(file init, const control_struct* ctrl, planting_struct* PLT, harvesting_struct* HRV);
int ploughing_init(file init,  const control_struct* ctrl, ploughing_struct* PLG);
int fertilizing_init(file init, const control_struct* ctrl, fertilizing_struct* FRZ);
int irrigating_init(file init, const control_struct* ctrl, irrigating_struct* IRG);
int planting_init(file init, control_struct* ctrl, planting_struct* PLT, epconst_struct* epc);
int conditionalMGM_init(file init, control_struct* ctrl, irrigating_struct* IRG, mowing_struct* MOW);
int read_mgmarray(int simyr, int varMGM, file MGM_file, double*** mgmarray);
int groundwater_init(groundwater_struct* gws, control_struct* ctrl);






