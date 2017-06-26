/* 
multilayer_sminn.c
Calculating the change in content of soil mineral nitrogen in multilayer soil (plant N upate, soil processes, nitrogen leaching, 
depostion and fixing). State update of sminn_RZ (mineral N content of rootzone).

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.6
Copyright 2017, D. Hidy [dori.hidy@gmail.com]
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

int multilayer_sminn(const epconst_struct* epc, const siteconst_struct* sitec, const epvar_struct* epv, 
					 nstate_struct* ns, nflux_struct* nf, wstate_struct* ws, wflux_struct* wf)
{
	int ok=1;
	int layer=0;
	double soilwater_nconc_downward, soilwater_nconc_upward, wflux_downward, wflux_upward, sminn0, sminn1;
	double sminn_SOILPROC_SUM, sminn_RZ;
	double sminn_to_soil_ctrl, diff, boundary_effect, sminn_flux;

	double sminn_boundary=1e-4;

	sminn_SOILPROC_SUM = sminn_to_soil_ctrl = sminn_RZ = sminn_flux = 0;



	/* ***************************************************************************************************** */	
	/* 1. SOILPROC:
		1.A.Plant N uptake from SMINN (sminn_to_npool is determined in daily_allocation routine) - 
	    1.B.Decomposition: due microbial soil processes SMINN (soil mineral N content) is changing in the soil (determined in daily_allocation).
	    The produced/consumed N is divided between soil layers based on their N content. */

	sminn_SOILPROC_SUM= 0;

	if (ns->sminn_RZ > 0)
	{
		for (layer = 0; layer < epv->n_rootlayers; layer++)
		{
			sminn_flux               = nf->sminn_to_soil_SUM + nf->sminn_to_npool + nf->sminn_to_denitrif;
			if (layer < epv->n_rootlayers-1)
				nf->sminn_to_soil[layer] = sminn_flux * (ns->sminn[layer]/ns->sminn_RZ);
			else
				nf->sminn_to_soil[epv->n_rootlayers-1] = sminn_flux - sminn_SOILPROC_SUM;

			sminn_SOILPROC_SUM       += nf->sminn_to_soil[layer];
		}
	}
	else 
	{
		nf->sminn_to_soil[layer] = 0;
		sminn_flux               = 0;
	}


	/* calculation control */
	diff = sminn_flux - sminn_SOILPROC_SUM;
	if (fabs(diff) > CRIT_PREC)          
	{
		printf("Error in sminn change calculation in multilayer_sminn.c\n");
		ok=0;
	}
	

	/* ***************************************************************************************************** */	
	/* 2. State update SMINN: soil processes (decomposition + plant uptake), deposition and fixation */

	for (layer = 0; layer < epv->n_rootlayers; layer++)
	{
		/* 1. soil processes (decomposition + plant uptake) */
		diff = ns->sminn[layer] - nf->sminn_to_soil[layer];
	
		if (diff < 0.0 && fabs(diff) > CRIT_PREC)       
		{
			if (fabs(diff) < nf->sminn_to_soil[layer]) 
			{
				printf("WARNING: limited sminn_to_soil (multilayer_sminn.c)\n");
				nf->sminn_to_soil[layer] += diff;
				nf->sminn_to_soil_SUM += diff;
				ns->sminn[layer] -= nf->sminn_to_soil[layer];
			}
			else
			{
				printf("Fatal error: negative N content (multilayer_sminn.c)\n");
				ok = 0;
			}
		}
		else
		{
			ns->sminn[layer] -= nf->sminn_to_soil[layer];
		}
		/* 2. deposition: only in top soil layer */
		if (layer == 0) ns->sminn[0]     += nf->ndep_to_sminn;

		/* 3. fixation: based on the quantity of the root mass in the given layer */
		ns->sminn[layer] += nf->nfix_to_sminn * epv->rootlength_prop[layer];

	

	
	}

	ns->npool     += nf->sminn_to_npool;
	ns->nfix_src  += nf->nfix_to_sminn;
	ns->ndep_src  += nf->ndep_to_sminn;

//	nf->sminn_to_soil_SUM = 0;

	/* ****************************************************************************/
	/* 3. N leaching:
		  Leaching fluxes are calculated after all the other nfluxes are reconciled to avoid the possibility of removing more N than is 
		  here. Leaching calculation based on the a function of the presumed proportion of the SMINN pool which is soluble (nitrates),
		  the SWC and the percolation */


	for (layer = 0; layer < N_SOILLAYERS-1; layer++)
	{
		if (wf->soilw_diffused[layer] > 0)
		{
			wflux_downward = wf->soilw_percolated[layer] + wf->soilw_diffused[layer];
			wflux_upward   = 0;

			soilwater_nconc_downward = epc->mobilen_prop * ns->sminn[layer] / ws->soilw[layer];
			soilwater_nconc_upward   = 0;
		}
		else
		{
			wflux_downward = wf->soilw_percolated[layer];
			wflux_upward   = wf->soilw_diffused[layer];

			soilwater_nconc_downward = epc->mobilen_prop * ns->sminn[layer]   / ws->soilw[layer];
			soilwater_nconc_upward   = epc->mobilen_prop * ns->sminn[layer+1] / ws->soilw[layer+1];
		}


		nf->sminn_leached[layer]  = soilwater_nconc_downward * wflux_downward;
		nf->sminn_diffused[layer] = soilwater_nconc_upward * wflux_upward;

	}

	/* STATE UPDATE */
	for (layer = 0; layer < N_SOILLAYERS-1; layer++)
	{
		sminn0 = ns->sminn[layer]   - (nf->sminn_leached[layer] + nf->sminn_diffused[layer]); 
		sminn1 = ns->sminn[layer+1] + (nf->sminn_leached[layer] + nf->sminn_diffused[layer]); 
		if (sminn0 < 0)
		{
			nf->sminn_leached[layer] += sminn0;
//			if (fabs(sminn1) > CRIT_PREC) printf("WARNING: limited N-leaching (multilayer_sminn.c)\n");
		}

		if (sminn1 < 0)
		{
			nf->sminn_diffused[layer] -= sminn1;
//			if (fabs(sminn1) > CRIT_PREC) printf("WARNING: limited N-diffusion (multilayer_sminn.c)\n");
		}

		ns->sminn[layer]   -= (nf->sminn_leached[layer] + nf->sminn_diffused[layer]); 
		ns->sminn[layer+1] += (nf->sminn_leached[layer] + nf->sminn_diffused[layer]); 

		/* CONTROL */
		if(ns->sminn[layer] < 0)
		{
			if (fabs (ns->sminn[layer]) > CRIT_PREC)
			{
				printf("FATAL ERROR: negative sminn pool (multilayer_sminn.c)\n");
				ok=0;
			}
			else
			{
				ns->nvol_snk     += ns->sminn[layer];
				ns->sminn[layer] = 0;

			}

		}
	}
	
	/* BOUNDARY LAYER IS SPECIAL: constant N-content */
	ns->nleached_snk	+= nf->sminn_leached[N_SOILLAYERS-2];
	ns->ndiffused_snk	+= nf->sminn_diffused[N_SOILLAYERS-2];

	if (layer == N_SOILLAYERS-1)
	{
		boundary_effect  = sminn_boundary - ns->sminn[layer];
		ns->sminn[layer] = sminn_boundary;
		ns->BNDRYsrc     += boundary_effect;
	}


	
	/* ***************************************************************************************************** */	
	/* 4. Calculating the soil mineral N content of rooting zone taking into account changing rooting depth 
		  N elimitated/added to rootzone Ncontent because of changes of the soil layer's N content */
	
	sminn_RZ = 0;
	if (epv->n_rootlayers == 1)
	{
		sminn_RZ = ns->sminn[0];
	}
	else
	{
		for (layer = 0; layer < epv->n_rootlayers-1; layer++)
		{
			sminn_RZ	+= ns->sminn[layer];
		}	
		sminn_RZ	+= ns->sminn[epv->n_rootlayers-1] * (epv->rooting_depth - sitec->soillayer_depth[layer-1]) / sitec->soillayer_thickness[layer];
	}
	ns->sminn_RZ	  = sminn_RZ;



	
	return (!ok);
}

