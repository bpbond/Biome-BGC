/* 
multilayer_leaching.c
Calculating soil mineral nitrogen and DOC-DON leaching in multilayer soil 
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0
Copyright 2018, D. Hidy [dori.hidy@gmail.com]
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

int multilayer_leaching(const epconst_struct* epc, const epvar_struct* epv,
					    cstate_struct* cs, cflux_struct* cf, nstate_struct* ns, nflux_struct* nf, wstate_struct* ws, wflux_struct* wf)
{
	int ok=1;
	int layer=0;
	int datatype=0;
	int n_data=8;
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

	/* ****************************************************************************/
	/* I. DOC, DON and sminN leaching:
	  Leaching fluxes are calculated after all the other nfluxes are reconciled to avoid the possibility of removing more N than is 
	  here. Leaching calculation based on the a function of the presumed proportion of the SMINN pool which is soluble (nitrates),
	  the SWC and the percolation */

	for (layer = 0; layer < N_SOILLAYERS-1; layer++)
	{

		soilwater_NH4conc_downward = epc->NH4_mobilen_prop * ns->sminNH4[layer]   / ws->soilw[layer];
		soilwater_NO3conc_downward = epc->NO3_mobilen_prop * ns->sminNO3[layer]   / ws->soilw[layer];
		soilwater_NH4conc_upward   = epc->NH4_mobilen_prop * ns->sminNH4[layer+1] / ws->soilw[layer+1];
		soilwater_NO3conc_upward   = epc->NO3_mobilen_prop * ns->sminNO3[layer+1] / ws->soilw[layer+1];
		
		if (ns->soil1n[layer] > 0)
		{
				cn_ratio1=cs->soil1c[layer]/ns->soil1n[layer]; 
				soilwater_DOC1conc_downward = epc->SOIL1_dissolv_prop * cs->soil1c[layer]   / ws->soilw[layer];
				soilwater_DON1conc_downward = soilwater_DOC1conc_downward/cn_ratio1;
				soilwater_DOC1conc_upward   = epc->SOIL1_dissolv_prop * cs->soil1c[layer+1] / ws->soilw[layer+1];
				soilwater_DON1conc_upward   = soilwater_DOC1conc_upward/cn_ratio1;
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
				soilwater_DOC2conc_downward = epc->SOIL2_dissolv_prop * cs->soil2c[layer]   / ws->soilw[layer];
				soilwater_DON2conc_downward = soilwater_DOC2conc_downward/cn_ratio2;
				soilwater_DOC2conc_upward   = epc->SOIL2_dissolv_prop * cs->soil2c[layer+1] / ws->soilw[layer+1];
				soilwater_DON2conc_upward   = soilwater_DOC2conc_upward/cn_ratio2;
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
				soilwater_DOC3conc_downward = epc->SOIL3_dissolv_prop * cs->soil3c[layer]   / ws->soilw[layer];
				soilwater_DON3conc_downward = soilwater_DOC3conc_downward/cn_ratio3;
				soilwater_DOC3conc_upward   = epc->SOIL3_dissolv_prop * cs->soil3c[layer+1] / ws->soilw[layer+1];
				soilwater_DON3conc_upward   = soilwater_DOC3conc_upward/cn_ratio3;
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
				soilwater_DOC4conc_downward = epc->SOIL4_dissolv_prop * cs->soil4c[layer]   / ws->soilw[layer];
				soilwater_DON4conc_downward = soilwater_DOC4conc_downward/cn_ratio4;
				soilwater_DOC4conc_upward   = epc->SOIL4_dissolv_prop * cs->soil4c[layer+1] / ws->soilw[layer+1];
				soilwater_DON4conc_upward   = soilwater_DOC4conc_upward/cn_ratio4;
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


		nf->sminNH4_leached[layer]  = soilwater_NH4conc_downward * wflux_downward;
		nf->sminNO3_leached[layer]  = soilwater_NO3conc_downward * wflux_downward;
		nf->sminNH4_diffused[layer] = soilwater_NH4conc_upward   * wflux_upward;
		nf->sminNO3_diffused[layer] = soilwater_NO3conc_upward   * wflux_upward;

		
		cf->soil1_DOC_leached[layer]  = soilwater_DOC1conc_downward * wflux_downward;
		nf->soil1_DON_leached[layer]  = soilwater_DON1conc_downward * wflux_downward;
		cf->soil1_DOC_diffused[layer] = soilwater_DOC1conc_upward   * wflux_upward;
		nf->soil1_DON_diffused[layer] = soilwater_DON1conc_upward   * wflux_upward;
		
		cf->soil2_DOC_leached[layer]  = soilwater_DOC2conc_downward * wflux_downward;
		nf->soil2_DON_leached[layer]  = soilwater_DON2conc_downward * wflux_downward;
		cf->soil2_DOC_diffused[layer] = soilwater_DOC2conc_upward   * wflux_upward;
		nf->soil2_DON_diffused[layer] = soilwater_DON2conc_upward   * wflux_upward;

		cf->soil3_DOC_leached[layer]  = soilwater_DOC3conc_downward * wflux_downward;
		nf->soil3_DON_leached[layer]  = soilwater_DON3conc_downward * wflux_downward;
		cf->soil3_DOC_diffused[layer] = soilwater_DOC3conc_upward   * wflux_upward;
		nf->soil3_DON_diffused[layer] = soilwater_DON3conc_upward   * wflux_upward;

		cf->soil4_DOC_leached[layer]  = soilwater_DOC4conc_downward * wflux_downward;
		nf->soil4_DON_leached[layer]  = soilwater_DON4conc_downward * wflux_downward;
		cf->soil4_DOC_diffused[layer] = soilwater_DOC4conc_upward   * wflux_upward;
		nf->soil4_DON_diffused[layer] = soilwater_DON4conc_upward   * wflux_upward;



	}
	cf->DOC_rootzone_leach   = cf->soil1_DOC_leached[epv->n_maxrootlayers-1] + cf->soil2_DOC_leached[epv->n_maxrootlayers-1] + 
		                       cf->soil3_DOC_leached[epv->n_maxrootlayers-1] + cf->soil4_DOC_leached[epv->n_maxrootlayers-1] + 
							   cf->soil1_DOC_diffused[epv->n_maxrootlayers-1] + cf->soil2_DOC_diffused[epv->n_maxrootlayers-1] + 
		                       cf->soil3_DOC_diffused[epv->n_maxrootlayers-1] + cf->soil4_DOC_diffused[epv->n_maxrootlayers-1];
	nf->DON_rootzone_leach   = nf->soil1_DON_leached[epv->n_maxrootlayers-1] + nf->soil2_DON_leached[epv->n_maxrootlayers-1] + 
		                       nf->soil3_DON_leached[epv->n_maxrootlayers-1] + nf->soil4_DON_leached[epv->n_maxrootlayers-1] +
							   nf->soil1_DON_diffused[epv->n_maxrootlayers-1] + nf->soil2_DON_diffused[epv->n_maxrootlayers-1] + 
		                       nf->soil3_DON_diffused[epv->n_maxrootlayers-1] + nf->soil4_DON_diffused[epv->n_maxrootlayers-1];
	nf->sminN_rootzone_leach = nf->sminNH4_leached[epv->n_maxrootlayers-1]   + nf->sminNO3_leached[epv->n_maxrootlayers-1] + nf->sminNH4_diffused[epv->n_maxrootlayers-1]   + nf->sminNO3_diffused[epv->n_maxrootlayers-1];
	wf->soilw_rootzone_leach = wf->soilw_percolated[epv->n_maxrootlayers-1]  + wf->soilw_diffused[epv->n_maxrootlayers-1];
	
	 /* ****************************************************************************/
	/* II. STATE UPDATE */

	
	ns->sminNH4_total=0;
	ns->sminNO3_total=0;

	for (layer = 0; layer < N_SOILLAYERS-1; layer++)
	{
		for (datatype = 0; datatype <= n_data; datatype++)
		{	
			/* NH4 pool  */
			if (datatype == 0)
			{	
				state0 = ns->sminNH4[layer];
				state1 = ns->sminNH4[layer+1];
				lflux  = nf->sminNH4_leached[layer];
				dflux  = nf->sminNH4_diffused[layer];
			}

			/* NO3 pool  */
			if (datatype == 1)
			{	
				state0 = ns->sminNO3[layer];
				state1 = ns->sminNO3[layer+1];
				lflux  = nf->sminNO3_leached[layer];
				dflux  = nf->sminNO3_diffused[layer];
			}

			/* SOIL1C pool  */
			if (datatype == 2)
			{	
				state0 = cs->soil1c[layer];
				state1 = cs->soil1c[layer+1];
				lflux  = cf->soil1_DOC_leached[layer];
				dflux  = cf->soil1_DOC_diffused[layer];
			}

			/* SOIL2C pool  */
			if (datatype == 3)
			{	
				state0 = cs->soil2c[layer];
				state1 = cs->soil2c[layer+1];
				lflux  = cf->soil2_DOC_leached[layer];
				dflux  = cf->soil2_DOC_diffused[layer];
			}

			/* SOIL3C pool  */
			if (datatype == 4)
			{	
				state0 = cs->soil3c[layer];
				state1 = cs->soil3c[layer+1];
				lflux  = cf->soil3_DOC_leached[layer];
				dflux  = cf->soil3_DOC_diffused[layer];
			}

			/* SOIL4C pool  */
			if (datatype == 5)
			{	
				state0 = cs->soil4c[layer];
				state1 = cs->soil4c[layer+1];
				lflux  = cf->soil4_DOC_leached[layer];
				dflux  = cf->soil4_DOC_diffused[layer];
			}

			/* SOIL1N pool  */
			if (datatype == 6)
			{	
				state0 = ns->soil1n[layer];
				state1 = ns->soil1n[layer+1];
				lflux  = nf->soil1_DON_leached[layer];
				dflux  = nf->soil1_DON_diffused[layer];
			}

			/* SOIL2N pool  */
			if (datatype == 7)
			{	
				state0 = ns->soil2n[layer];
				state1 = ns->soil2n[layer+1];
				lflux  = nf->soil2_DON_leached[layer];
				dflux  = nf->soil2_DON_diffused[layer];
			}

			/* SOIL3N pool  */
			if (datatype == 8)
			{	
				state0 = ns->soil3n[layer];
				state1 = ns->soil3n[layer+1];
				lflux  = nf->soil3_DON_leached[layer];
				dflux  = nf->soil3_DON_diffused[layer];
			}

			/* SOIL4N pool  */
			if (datatype == 9)
			{	
				state0 = ns->soil4n[layer];
				state1 = ns->soil4n[layer+1];
				lflux  = nf->soil4_DON_leached[layer];
				dflux  = nf->soil4_DON_diffused[layer];
			}
			
			pool0 = state0 - (lflux + dflux); 
			pool1 = state1 + (lflux + dflux);
		
			if (pool0 < 0)
			{
				lflux += pool0;
				if (fabs(pool0) > CRIT_PREC) 
				{
					printf("WARNING: limited leaching (multilayer_leaching.c)\n");
				}
			}

			if (pool1 < 0)
			{
				dflux -= pool1;
				if (fabs(pool1) > CRIT_PREC) 
				{
					printf("WARNING: limited diffusion (multilayer_leaching.c)\n");
				}
			}

			state0    -= (lflux + dflux); 
			state1    += (lflux + dflux); 

			/* control */
			if(state0 < 0)
			{
				if (fabs (state0) > CRIT_PREC)
				{
					printf("FATAL ERROR: negative pool (multilayer_leaching.c)\n");
					ok=0;
				}
				else
				{
					ns->nvol_snk     += state0;
					state0 = 0;

				}
			}



			/* NH4 pool  */
			if (datatype == 0)
			{	
				ns->sminNH4[layer]			= state0;
				ns->sminNH4[layer+1]		= state1;
				nf->sminNH4_leached[layer]  = lflux;
				nf->sminNH4_diffused[layer] = dflux;
				ns->sminNH4_total           += state0;
			}

			/* NO3 pool  */
			if (datatype == 1)
			{	
				ns->sminNO3[layer]			= state0;
				ns->sminNO3[layer+1]		= state1;
				nf->sminNO3_leached[layer]  = lflux;
				nf->sminNO3_diffused[layer] = dflux;
				ns->sminNO3_total           += state0;
			}

			/* SOIL1C pool  */
			if (datatype == 2)
			{	
				cs->soil1c[layer]			  = state0;
				cs->soil1c[layer+1]		      = state1;
				cf->soil1_DOC_leached[layer]  = lflux;
				cf->soil1_DOC_diffused[layer] = dflux;
				cs->soil1_DOC[layer]		  = epc->SOIL1_dissolv_prop * cs->soil1c[layer];
				cs->soil1_DOC[layer+1]		  = epc->SOIL1_dissolv_prop * cs->soil1c[layer+1];
			}

			/* SOIL2C pool  */
			if (datatype == 3)
			{	
				cs->soil2c[layer]			  = state0;
				cs->soil2c[layer+1]		      = state1;
				cf->soil2_DOC_leached[layer]  = lflux;
				cf->soil2_DOC_diffused[layer] = dflux;
				cs->soil2_DOC[layer]		  = epc->SOIL2_dissolv_prop * cs->soil2c[layer];
				cs->soil2_DOC[layer+1]		  = epc->SOIL2_dissolv_prop * cs->soil2c[layer+1];
			}

			/* SOIL3C pool  */
			if (datatype == 4)
			{	
				cs->soil3c[layer]			  = state0;
				cs->soil3c[layer+1]		      = state1;
				cf->soil3_DOC_leached[layer]  = lflux;
				cf->soil3_DOC_diffused[layer] = dflux;
				cs->soil3_DOC[layer]		  = epc->SOIL3_dissolv_prop * cs->soil3c[layer];
				cs->soil3_DOC[layer+1]		  = epc->SOIL3_dissolv_prop * cs->soil3c[layer+1];
			}

			/* SOIL4C pool  */
			if (datatype == 5)
			{	
				cs->soil4c[layer]			  = state0;
				cs->soil4c[layer+1]		      = state1;
				cf->soil4_DOC_leached[layer]  = lflux;
				cf->soil4_DOC_diffused[layer] = dflux;
				cs->soil4_DOC[layer]		  = epc->SOIL4_dissolv_prop * cs->soil4c[layer];
				cs->soil4_DOC[layer+1]		  = epc->SOIL4_dissolv_prop * cs->soil4c[layer+1];
			}

			/* SOIL1N pool  */
			if (datatype == 6)
			{	
				ns->soil1n[layer]			  = state0;
				ns->soil1n[layer+1]		      = state1;
				nf->soil1_DON_leached[layer]  = lflux;
				nf->soil1_DON_diffused[layer] = dflux;
				ns->soil1_DON[layer]		  = epc->SOIL1_dissolv_prop * ns->soil1n[layer];
				ns->soil1_DON[layer+1]		  = epc->SOIL1_dissolv_prop * ns->soil1n[layer+1];
			}

			/* SOIL2N pool  */
			if (datatype == 7)
			{	
				ns->soil2n[layer]			  = state0;
				ns->soil2n[layer+1]		      = state1;
				nf->soil2_DON_leached[layer]  = lflux;
				nf->soil2_DON_diffused[layer] = dflux;
				ns->soil2_DON[layer]		  = epc->SOIL2_dissolv_prop * ns->soil2n[layer];
				ns->soil2_DON[layer+1]		  = epc->SOIL2_dissolv_prop * ns->soil2n[layer+1];
			}

			/* SOIL3N pool  */
			if (datatype == 8)
			{	
				ns->soil3n[layer]			  = state0;
				ns->soil3n[layer+1]		      = state1;
				nf->soil3_DON_leached[layer]  = lflux;
				nf->soil3_DON_diffused[layer] = dflux;
				ns->soil3_DON[layer]		  = epc->SOIL3_dissolv_prop * ns->soil3n[layer];
				ns->soil3_DON[layer+1]		  = epc->SOIL3_dissolv_prop * ns->soil3n[layer+1];
			}

			/* SOIL4N pool  */
			if (datatype == 9)
			{	
				ns->soil4n[layer]			  = state0;
				ns->soil4n[layer+1]		      = state1;
				nf->soil4_DON_leached[layer]  = lflux;
				nf->soil4_DON_diffused[layer] = dflux;
				ns->soil4_DON[layer]		  = epc->SOIL4_dissolv_prop * ns->soil4n[layer];
				ns->soil4_DON[layer+1]		  = epc->SOIL4_dissolv_prop * ns->soil4n[layer+1];
			}

		} /* endfor of datatpye */
		/*-----------------------------------*/

	} /* endfor of layer */
	
	
	return (!ok);
}

