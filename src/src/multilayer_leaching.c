/* 
multilayer_leaching.c
Calculating soil mineral nitrogen and DOC-DON leaching in multilayer soil 
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Copyright 2020, D. Hidy [dori.hidy@gmail.com]
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

int multilayer_leaching(const soilprop_struct* sprop, const epvar_struct* epv,
					    control_struct* ctrl, cstate_struct* cs, cflux_struct* cf, nstate_struct* ns, nflux_struct* nf, wstate_struct* ws, wflux_struct* wf)
{
	int errorCode=0;
	int layer=0;
	int datatype=0;
	int n_data=9;
	double soilwater_NH4conc_downward, soilwater_NH4conc_upward, soilwater_NO3conc_downward, soilwater_NO3conc_upward; 
	double soilwater_DOC1conc_downward, soilwater_DON1conc_downward, soilwater_DOC1conc_upward, soilwater_DON1conc_upward;
	double soilwater_DOC2conc_downward, soilwater_DON2conc_downward, soilwater_DOC2conc_upward, soilwater_DON2conc_upward;
	double soilwater_DOC3conc_downward, soilwater_DON3conc_downward, soilwater_DOC3conc_upward, soilwater_DON3conc_upward;
	double soilwater_DOC4conc_downward, soilwater_DON4conc_downward, soilwater_DOC4conc_upward, soilwater_DON4conc_upward;
	double wflux_downward, wflux_upward;
	double state0, state1,lflux, dflux, pool0, pool1;
	double cn_ratio1, cn_ratio2, cn_ratio3, cn_ratio4;



	soilwater_NH4conc_downward=soilwater_NH4conc_upward=soilwater_NO3conc_downward=soilwater_NO3conc_upward=0;
	soilwater_DOC1conc_downward=soilwater_DON1conc_downward=soilwater_DOC1conc_upward=soilwater_DON1conc_upward=0;
	soilwater_DOC2conc_downward=soilwater_DON2conc_downward=soilwater_DOC2conc_upward=soilwater_DON2conc_upward=0;
	soilwater_DOC3conc_downward=soilwater_DON3conc_downward=soilwater_DOC3conc_upward=soilwater_DON3conc_upward=0;
	soilwater_DOC4conc_downward=soilwater_DON4conc_downward=soilwater_DOC4conc_upward=soilwater_DON4conc_upward=0;

    state0=state1=lflux=dflux=cn_ratio1=cn_ratio2=cn_ratio3=cn_ratio4=0;

	

	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		soilwater_NH4conc_downward = ns->sminNH4avail[layer]  / ws->soilw[layer];
		soilwater_NO3conc_downward = ns->sminNO3avail[layer]  / ws->soilw[layer];

		if (layer < N_SOILLAYERS-1)
		{
			soilwater_NH4conc_upward   = ns->sminNH4avail[layer+1]  / ws->soilw[layer+1];
			soilwater_NO3conc_upward   = ns->sminNO3avail[layer+1]  / ws->soilw[layer+1];
		}
		else
		{
			soilwater_NH4conc_upward   = 0;
			soilwater_NO3conc_upward   = 0;
		}

		
		if (ns->soil1n[layer] > 0)
		{
				cn_ratio1=cs->soil1c[layer]/ns->soil1n[layer]; 
				soilwater_DOC1conc_downward = sprop->SOIL1_dissolv_prop * cs->soil1c[layer]   / ws->soilw[layer];
				soilwater_DON1conc_downward = soilwater_DOC1conc_downward/cn_ratio1;
				if (layer < N_SOILLAYERS-1)
				{
					soilwater_DOC1conc_upward   = sprop->SOIL1_dissolv_prop * cs->soil1c[layer+1] / ws->soilw[layer+1];
					soilwater_DON1conc_upward   = soilwater_DOC1conc_upward/cn_ratio1;
				}
				else
				{
					soilwater_DOC1conc_upward   = 0;
					soilwater_DON1conc_upward   = 0;
				}
		}
		else
		{
				soilwater_DOC1conc_downward = 0;
				soilwater_DON1conc_downward = 0;
				soilwater_DOC1conc_upward   = 0;
				soilwater_DON1conc_upward   = 0;
		
		}

		
		if (ns->soil2n[layer] > 0)
		{
				cn_ratio2=cs->soil2c[layer]/ns->soil2n[layer]; 
				soilwater_DOC2conc_downward = sprop->SOIL2_dissolv_prop * cs->soil2c[layer]   / ws->soilw[layer];
				soilwater_DON2conc_downward = soilwater_DOC2conc_downward/cn_ratio2;
				if (layer < N_SOILLAYERS-1)
				{
					soilwater_DOC2conc_upward   = sprop->SOIL2_dissolv_prop * cs->soil2c[layer+1] / ws->soilw[layer+1];
					soilwater_DON2conc_upward   = soilwater_DOC2conc_upward/cn_ratio2;
				}
				else
				{
					soilwater_DOC2conc_upward   = 0;
					soilwater_DON2conc_upward   = 0;
				}
		}
		else
		{	
				soilwater_DOC2conc_downward = 0;
				soilwater_DON2conc_downward = 0;
				soilwater_DOC2conc_upward   = 0;
				soilwater_DON2conc_upward   = 0;
		}
		
		if (ns->soil3n[layer] > 0)
		{
				cn_ratio3=cs->soil3c[layer]/ns->soil3n[layer]; 
				soilwater_DOC3conc_downward = sprop->SOIL3_dissolv_prop * cs->soil3c[layer]   / ws->soilw[layer];
				soilwater_DON3conc_downward = soilwater_DOC3conc_downward/cn_ratio3;
				if (layer < N_SOILLAYERS-1)
				{
					soilwater_DOC3conc_upward   = sprop->SOIL3_dissolv_prop * cs->soil3c[layer+1] / ws->soilw[layer+1];
					soilwater_DON3conc_upward   = soilwater_DOC3conc_upward/cn_ratio3;
				}
				else
				{
					soilwater_DOC3conc_upward   = 0;
					soilwater_DON3conc_upward   = 0;
				}
		}
		else
		{	
				soilwater_DOC3conc_downward = 0;
				soilwater_DON3conc_downward = 0;
				soilwater_DOC3conc_upward   = 0;
				soilwater_DON3conc_upward   = 0;
		}

		if (ns->soil4n[layer] > 0)
		{
				cn_ratio4=cs->soil4c[layer]/ns->soil4n[layer]; 
				soilwater_DOC4conc_downward = sprop->SOIL4_dissolv_prop * cs->soil4c[layer]   / ws->soilw[layer];
				soilwater_DON4conc_downward = soilwater_DOC4conc_downward/cn_ratio4;
				if (layer < N_SOILLAYERS-1)
				{
					soilwater_DOC4conc_upward   = sprop->SOIL4_dissolv_prop * cs->soil4c[layer+1] / ws->soilw[layer+1];
					soilwater_DON4conc_upward   = soilwater_DOC4conc_upward/cn_ratio4;
				}
				else
				{
					soilwater_DOC4conc_upward   = 0;
					soilwater_DON4conc_upward   = 0;
				}
		}
		else
		{	
				soilwater_DOC4conc_downward = 0;
				soilwater_DON4conc_downward = 0;
				soilwater_DOC4conc_upward   = 0;
				soilwater_DON4conc_upward   = 0;
		}
		


		if (wf->soilw_diffused[layer] > 0)
		{
			if (wf->soilw_percolated[layer] + wf->soilw_diffused[layer] < ws->soilw[layer]) wflux_downward = wf->soilw_percolated[layer] + wf->soilw_diffused[layer];
			else wflux_downward = ws->soilw[layer];

			wflux_upward   = 0;	
		}
		else
		{
			if (wf->soilw_percolated[layer] < ws->soilw[layer]) wflux_downward = wf->soilw_percolated[layer];
			else wflux_downward = ws->soilw[layer];

			if (-1*wf->soilw_diffused[layer] < ws->soilw[layer+1]) wflux_upward   = wf->soilw_diffused[layer];
			else wflux_upward  = ws->soilw[layer+1];
		}


		nf->sminNH4_percol[layer]  = soilwater_NH4conc_downward * wflux_downward;
		nf->sminNO3_percol[layer]  = soilwater_NO3conc_downward * wflux_downward;
		nf->sminNH4_diffus[layer] = soilwater_NH4conc_upward   * wflux_upward;
		nf->sminNO3_diffus[layer] = soilwater_NO3conc_upward   * wflux_upward;


		cf->soil1_DOC_percol[layer]  = soilwater_DOC1conc_downward * wflux_downward;
		nf->soil1_DON_percol[layer]  = soilwater_DON1conc_downward * wflux_downward;
		cf->soil1_DOC_diffus[layer] = soilwater_DOC1conc_upward   * wflux_upward;
		nf->soil1_DON_diffus[layer] = soilwater_DON1conc_upward   * wflux_upward;
		
		cf->soil2_DOC_percol[layer]  = soilwater_DOC2conc_downward * wflux_downward;
		nf->soil2_DON_percol[layer]  = soilwater_DON2conc_downward * wflux_downward;
		cf->soil2_DOC_diffus[layer] = soilwater_DOC2conc_upward   * wflux_upward;
		nf->soil2_DON_diffus[layer] = soilwater_DON2conc_upward   * wflux_upward;

		cf->soil3_DOC_percol[layer]  = soilwater_DOC3conc_downward * wflux_downward;
		nf->soil3_DON_percol[layer]  = soilwater_DON3conc_downward * wflux_downward;
		cf->soil3_DOC_diffus[layer] = soilwater_DOC3conc_upward   * wflux_upward;
		nf->soil3_DON_diffus[layer] = soilwater_DON3conc_upward   * wflux_upward;

		cf->soil4_DOC_percol[layer]  = soilwater_DOC4conc_downward * wflux_downward;
		nf->soil4_DON_percol[layer]  = soilwater_DON4conc_downward * wflux_downward;
		cf->soil4_DOC_diffus[layer] = soilwater_DOC4conc_upward   * wflux_upward;
		nf->soil4_DON_diffus[layer] = soilwater_DON4conc_upward   * wflux_upward;



	}
	cf->DOC_leached_RZ   = cf->soil1_DOC_percol[epv->n_maxrootlayers-1] + cf->soil2_DOC_percol[epv->n_maxrootlayers-1] + 
		                       cf->soil3_DOC_percol[epv->n_maxrootlayers-1] + cf->soil4_DOC_percol[epv->n_maxrootlayers-1] + 
							   cf->soil1_DOC_diffus[epv->n_maxrootlayers-1] + cf->soil2_DOC_diffus[epv->n_maxrootlayers-1] + 
		                       cf->soil3_DOC_diffus[epv->n_maxrootlayers-1] + cf->soil4_DOC_diffus[epv->n_maxrootlayers-1];

	nf->DON_leached_RZ   = nf->soil1_DON_percol[epv->n_maxrootlayers-1] + nf->soil2_DON_percol[epv->n_maxrootlayers-1] + 
		                       nf->soil3_DON_percol[epv->n_maxrootlayers-1] + nf->soil4_DON_percol[epv->n_maxrootlayers-1] +
							   nf->soil1_DON_diffus[epv->n_maxrootlayers-1] + nf->soil2_DON_diffus[epv->n_maxrootlayers-1] + 
		                       nf->soil3_DON_diffus[epv->n_maxrootlayers-1] + nf->soil4_DON_diffus[epv->n_maxrootlayers-1];
	
	wf->soilw_leached_RZ  = wf->soilw_percolated[epv->n_maxrootlayers-1]  + wf->soilw_diffused[epv->n_maxrootlayers-1];

	nf->sminN_leached_RZ  = nf->sminNH4_percol[epv->n_maxrootlayers-1]   + nf->sminNO3_percol[epv->n_maxrootlayers-1]+ 
		                     nf->sminNH4_diffus[epv->n_maxrootlayers-1]   + nf->sminNO3_diffus[epv->n_maxrootlayers-1];
	
	
	ns->sminNH4_total=0;
	ns->sminNO3_total=0;

	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{

		for (datatype = 0; datatype <= n_data; datatype++)
		{	
			/* NH4 pool  */
			if (datatype == 0)
			{	
			
				state0 = ns->sminNH4[layer];
				if (layer < N_SOILLAYERS-1) state1 = ns->sminNH4[layer+1];
				lflux  = nf->sminNH4_percol[layer];
				dflux  = nf->sminNH4_diffus[layer];

			}

			/* NO3 pool  */
			if (datatype == 1)
			{	
				state0 = ns->sminNO3[layer];
				if (layer < N_SOILLAYERS-1) state1 = ns->sminNO3[layer+1];
				lflux  = nf->sminNO3_percol[layer];
				dflux  = nf->sminNO3_diffus[layer];

			}

			/* SOIL1C pool  */
			if (datatype == 2)
			{	
				state0 = cs->soil1c[layer];
				if (layer < N_SOILLAYERS-1) state1 = cs->soil1c[layer+1];
				lflux  = cf->soil1_DOC_percol[layer];
				dflux  = cf->soil1_DOC_diffus[layer];
			}

			/* SOIL2C pool  */
			if (datatype == 3)
			{	
				state0 = cs->soil2c[layer];
				if (layer < N_SOILLAYERS-1) state1 = cs->soil2c[layer+1];
				lflux  = cf->soil2_DOC_percol[layer];
				dflux  = cf->soil2_DOC_diffus[layer];
			}

			/* SOIL3C pool  */
			if (datatype == 4)
			{	
				state0 = cs->soil3c[layer];
				if (layer < N_SOILLAYERS-1) state1 = cs->soil3c[layer+1];
				lflux  = cf->soil3_DOC_percol[layer];
				dflux  = cf->soil3_DOC_diffus[layer];
			}

			/* SOIL4C pool  */
			if (datatype == 5)
			{	
				state0 = cs->soil4c[layer];
				if (layer < N_SOILLAYERS-1) state1 = cs->soil4c[layer+1];
				lflux  = cf->soil4_DOC_percol[layer];
				dflux  = cf->soil4_DOC_diffus[layer];
			}

			/* SOIL1N pool  */
			if (datatype == 6)
			{	
				state0 = ns->soil1n[layer];
				if (layer < N_SOILLAYERS-1) state1 = ns->soil1n[layer+1];
				lflux  = nf->soil1_DON_percol[layer];
				dflux  = nf->soil1_DON_diffus[layer];
			}

			/* SOIL2N pool  */
			if (datatype == 7)
			{	
				state0 = ns->soil2n[layer];
				if (layer < N_SOILLAYERS-1) state1 = ns->soil2n[layer+1];
				lflux  = nf->soil2_DON_percol[layer];
				dflux  = nf->soil2_DON_diffus[layer];
			}

			/* SOIL3N pool  */
			if (datatype == 8)
			{	
				state0 = ns->soil3n[layer];
				if (layer < N_SOILLAYERS-1) state1 = ns->soil3n[layer+1];
				lflux  = nf->soil3_DON_percol[layer];
				dflux  = nf->soil3_DON_diffus[layer];
			}

			/* SOIL4N pool  */
			if (datatype == 9)
			{	
				state0 = ns->soil4n[layer];
				if (layer < N_SOILLAYERS-1) state1 = ns->soil4n[layer+1];
				lflux  = nf->soil4_DON_percol[layer];
				dflux  = nf->soil4_DON_diffus[layer];
			}
			
			pool0 = state0 - (lflux + dflux); 
			pool1 = state1 + (lflux + dflux);
		
			if (pool0 < 0)
			{
				lflux += pool0;
				/* limitleach_flag: flag of WARNING writing in log file (only at first time) */
				if (fabs(pool0) > CRIT_PREC && !ctrl->limitleach_flag) ctrl->limitleach_flag = 1;
			}

			if (pool1 < 0)
			{
				dflux -= pool1;
				/* limitdiffus_flag: flag of WARNING writing in log file (only at first time) */
				if (fabs(pool1) > CRIT_PREC && !ctrl->limitdiffus_flag) ctrl->limitdiffus_flag = 1; 
			}

			state0    -= (lflux + dflux); 
			state1    += (lflux + dflux); 

			/* control */
			if(state0 < 0)
			{
				if (fabs (state0) > CRIT_PREC)
				{
					printf("FATAL ERROR: negative pool (multilayer_leaching.c)\n");
					errorCode=1;
				}
				else
				{
					ns->Nprec_snk     += state0;
					state0 = 0;
				}
			}


			/* NH4 pool  */
			if (datatype == 0)
			{	
				ns->sminNH4[layer]			= state0;
				if (layer < N_SOILLAYERS-1) ns->sminNH4[layer+1]		= state1;
				nf->sminNH4_percol[layer]   = lflux;
				nf->sminNH4_diffus[layer]   = dflux;
			}

			/* NO3 pool  */
			if (datatype == 1)
			{	
				ns->sminNO3[layer]			= state0;
				if (layer < N_SOILLAYERS-1) ns->sminNO3[layer+1]		= state1;
				nf->sminNO3_percol[layer]   = lflux;
				nf->sminNO3_diffus[layer]   = dflux;
			}

		

			/* SOIL1C pool  */
			if (datatype == 2)
			{	
				cs->soil1c[layer]			  = state0;
				if (layer < N_SOILLAYERS-1) cs->soil1c[layer+1]		      = state1;
				cf->soil1_DOC_percol[layer]   = lflux;
				cf->soil1_DOC_diffus[layer]   = dflux;
				cs->soil1_DOC[layer]		  = sprop->SOIL1_dissolv_prop * cs->soil1c[layer];
				if (layer < N_SOILLAYERS-1) cs->soil1_DOC[layer+1]		  = sprop->SOIL1_dissolv_prop * cs->soil1c[layer+1];
			}

			/* SOIL2C pool  */
			if (datatype == 3)
			{	
				cs->soil2c[layer]			  = state0;
				if (layer < N_SOILLAYERS-1) cs->soil2c[layer+1]		      = state1;
				cf->soil2_DOC_percol[layer]   = lflux;
				cf->soil2_DOC_diffus[layer]   = dflux;
				cs->soil2_DOC[layer]		  = sprop->SOIL2_dissolv_prop * cs->soil2c[layer];
				if (layer < N_SOILLAYERS-1) cs->soil2_DOC[layer+1]		  = sprop->SOIL2_dissolv_prop * cs->soil2c[layer+1];
			}

			/* SOIL3C pool  */
			if (datatype == 4)
			{	
				cs->soil3c[layer]			  = state0;
				if (layer < N_SOILLAYERS-1) cs->soil3c[layer+1]		      = state1;
				cf->soil3_DOC_percol[layer]   = lflux;
				cf->soil3_DOC_diffus[layer]   = dflux;
				cs->soil3_DOC[layer]		  = sprop->SOIL3_dissolv_prop * cs->soil3c[layer];
				if (layer < N_SOILLAYERS-1) cs->soil3_DOC[layer+1]		  = sprop->SOIL3_dissolv_prop * cs->soil3c[layer+1];
			}

			/* SOIL4C pool  */
			if (datatype == 5)
			{	
				cs->soil4c[layer]			  = state0;
				if (layer < N_SOILLAYERS-1) cs->soil4c[layer+1]		      = state1;
				cf->soil4_DOC_percol[layer]   = lflux;
				cf->soil4_DOC_diffus[layer]   = dflux;
				cs->soil4_DOC[layer]		  = sprop->SOIL4_dissolv_prop * cs->soil4c[layer];
				if (layer < N_SOILLAYERS-1) cs->soil4_DOC[layer+1]		  = sprop->SOIL4_dissolv_prop * cs->soil4c[layer+1];
			}

			/* SOIL1N pool  */
			if (datatype == 6)
			{	
				ns->soil1n[layer]			  = state0;
				if (layer < N_SOILLAYERS-1) ns->soil1n[layer+1]		      = state1;
				nf->soil1_DON_percol[layer]   = lflux;
				nf->soil1_DON_diffus[layer]   = dflux;
				ns->soil1_DON[layer]		  = sprop->SOIL1_dissolv_prop * ns->soil1n[layer];
				if (layer < N_SOILLAYERS-1) ns->soil1_DON[layer+1]		  = sprop->SOIL1_dissolv_prop * ns->soil1n[layer+1];
			}

			/* SOIL2N pool  */
			if (datatype == 7)
			{	
				ns->soil2n[layer]			  = state0;
				if (layer < N_SOILLAYERS-1) ns->soil2n[layer+1]		      = state1;
				nf->soil2_DON_percol[layer]   = lflux;
				nf->soil2_DON_diffus[layer]   = dflux;
				ns->soil2_DON[layer]		  = sprop->SOIL2_dissolv_prop * ns->soil2n[layer];
				if (layer < N_SOILLAYERS-1) ns->soil2_DON[layer+1]		  = sprop->SOIL2_dissolv_prop * ns->soil2n[layer+1];
			}

			/* SOIL3N pool  */
			if (datatype == 8)
			{	
				ns->soil3n[layer]			  = state0;
				if (layer < N_SOILLAYERS-1) ns->soil3n[layer+1]		      = state1;
				nf->soil3_DON_percol[layer]   = lflux;
				nf->soil3_DON_diffus[layer]   = dflux;
				ns->soil3_DON[layer]		  = sprop->SOIL3_dissolv_prop * ns->soil3n[layer];
				if (layer < N_SOILLAYERS-1) ns->soil3_DON[layer+1]		  = sprop->SOIL3_dissolv_prop * ns->soil3n[layer+1];
			}

			/* SOIL4N pool  */
			if (datatype == 9)
			{	
				ns->soil4n[layer]			  = state0;
				if (layer < N_SOILLAYERS-1) ns->soil4n[layer+1]		      = state1;
				nf->soil4_DON_percol[layer]   = lflux;
				nf->soil4_DON_diffus[layer]   = dflux;
				ns->soil4_DON[layer]		  = sprop->SOIL4_dissolv_prop * ns->soil4n[layer];
				if (layer < N_SOILLAYERS-1) ns->soil4_DON[layer+1]		  = sprop->SOIL4_dissolv_prop * ns->soil4n[layer+1];
			}


		} /* endfor of datatpye */
		/*-----------------------------------*/

	} /* endfor of layer */
	

	/* deepleach calculation from the bottom layer */
	ns->Ndeepleach_snk += nf->sminNH4_percol[N_SOILLAYERS-1] + nf->sminNO3_percol[N_SOILLAYERS-1]  + 
		                  nf->soil1_DON_percol[N_SOILLAYERS-1] +nf->soil2_DON_percol[N_SOILLAYERS-1] +
						  nf->soil3_DON_percol[N_SOILLAYERS-1] +nf->soil4_DON_percol[N_SOILLAYERS-1];

	cs->Cdeepleach_snk += cf->soil1_DOC_percol[N_SOILLAYERS-1] +cf->soil2_DOC_percol[N_SOILLAYERS-1] +
		                  cf->soil3_DOC_percol[N_SOILLAYERS-1] +cf->soil4_DOC_percol[N_SOILLAYERS-1];



	/* state update available SMINN */
	for (layer = 0; layer < N_SOILLAYERS; layer++)
	{
		ns->sminNH4avail[layer] = ns->sminNH4[layer] * sprop->NH4_mobilen_prop;
		ns->sminNO3avail[layer] = ns->sminNO3[layer] * NO3_mobilen_prop;
	}
	
	return (errorCode);
}

