/*
pointbgc_func.h
function prototypes for pointbgc
for use with pointbgc front-end to BIOME-BGC library v4.1

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.1.1
Copyright 2000, Peter E. Thornton
Numerical Terradynamics Simulation Group (NTSG)
School of Forestry, University of Montana
Missoula, MT 59812
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

int met_init(file init, point_struct* point);
int restart_init(file init, restart_ctrl_struct* restart);
int time_init(file init, control_struct *ctrl);
int scc_init(file init, climchange_struct* scc);
int co2_init(file init, co2control_struct* co2, int simyears);
int sitec_init(file init, siteconst_struct* sitec);
int ramp_ndep_init(file init, ramp_ndep_struct* ramp_ndep);
int epc_init(file init, epconst_struct* epc);
int wstate_init(file init, const siteconst_struct* sitec, wstate_struct* ws);
int cnstate_init(file init, const epconst_struct* epc, cstate_struct* cs,
cinit_struct* cinit, nstate_struct* ns);
int output_init(file init, output_struct* output);
int end_init(file init);
int metarr_init(file metf, metarr_struct* metarr, const climchange_struct* scc,
int nyears);
int presim_state_init(wstate_struct* ws, cstate_struct* cs, nstate_struct* ns,
cinit_struct* cinit);
