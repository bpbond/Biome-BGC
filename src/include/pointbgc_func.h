/*
pointbgc_func.h
function prototypes for pointbgc
for use with pointbgc front-end to BBGC MuSo v4 library

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2018, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Modified:
13/07/2000: Added input of Ndep from file=> added simyears function to
ndep_init function. Changes are made by Galina Churkina.
*/

int met_init(file init, point_struct* point);
int restart_init(file init, restart_ctrl_struct* restart);
int time_init(file init, control_struct *ctrl);
int scc_init(file init, climchange_struct* scc);
int co2_init(file init, co2control_struct* co2, int simyears);
int sitec_init(file init, siteconst_struct* sitec);
int ndep_init(file init, ndep_control_struct* ndep, int simyears);
int epc_init(file init, epconst_struct* epc, GSI_struct* GSI, control_struct* ctrl);
int wstate_init(file init, const siteconst_struct* sitec, wstate_struct* ws);
int cnstate_init(file init, const epconst_struct* epc, cstate_struct* cs,
	cinit_struct* cinit, nstate_struct* ns);
int output_init(file init, output_struct* output);
int end_init(file init);
int metarr_init(point_struct* point, metarr_struct* metarr, const climchange_struct* scc, const siteconst_struct* sitec,int nyears);
int presim_state_init(wstate_struct* ws, cstate_struct* cs, nstate_struct* ns,
	cinit_struct* cinit);


int GSI_calculation(const metarr_struct* metarr, const control_struct* ctrl, const siteconst_struct* sitec, const epconst_struct* epc, 
	GSI_struct* GSI, phenarray_struct* phenarr);

int conduct_limit_factors(file logfile, const control_struct* ctrl, const siteconst_struct* sitec, const epconst_struct* epc, epvar_struct* epv);

int prephenology(file logfile, const planting_struct* PLT, const harvesting_struct* HRV, const epconst_struct* epc, 
	const siteconst_struct* sitec, const metarr_struct* metarr, const control_struct* ctrl, phenarray_struct* phenarr);

int output_handling(int monday, int endday, control_struct* ctrl, double** output_map, 
					double* dayarr, double* monavgarr, double* annavgarr, double* annarr, 
					file dayout,file monavgout, file annavgout, file annout);

int thinning_init(file init, const control_struct* ctrl, thinning_struct* THN);
int mowing_init(file init, const control_struct* ctrl, mowing_struct* MOW);
int grazing_init(file init, const control_struct* ctrl, grazing_struct* GRZ);
int harvesting_init(file init, const control_struct* ctrl, harvesting_struct* HRV);
int ploughing_init(file init,  const control_struct* ctrl, ploughing_struct* PLG);
int fertilizing_init(file init, const control_struct* ctrl, fertilizing_struct* FRZ);
int irrigation_init(file init, const control_struct* ctrl, irrigation_struct* IRG);
int planting_init(file init, const control_struct* ctrl, planting_struct* PLT);
int read_mgmarray(int simyr, int varMGM, file MGM_file, double*** mgmarray);
int groundwater_init(siteconst_struct* sitec, control_struct* ctrl);






