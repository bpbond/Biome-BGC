/* 
phenphase.c
daily crop-specific phenology fluxes

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0.
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
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

int phenphase(const control_struct* ctrl, const epconst_struct* epc, const siteconst_struct* sitec, 
	phenology_struct *phen, metvar_struct *metv, epvar_struct* epv, cstate_struct* cs)
{

	int pp,	counter;
	double  dev_rate;
	int ok = 1;

	
	/* 1. initalize GDD and phenological variables on the first day of the year */
	if (epv->n_actphen == 0) 
	{
		metv->GDD = 0;
		metv->GDD_wMOD = 0;
		epv->leafday = 0;
		epv->leafday_lastmort = 0;
		phen->vern_dev_rate = 0;
		phen->phpsl_dev_rate = 0;
		phen->vern_days = 0;
		phen->GDD_emergSTART = 0;
		phen->GDD_emergEND   = 0;
		phen->GDD_limit = 0;
		epv->sla_avg = 0;
		for (pp = 0; pp < N_PHENPHASES; pp++) 
		{
			phen->GDD_crit[pp] = 0;
			cs->leafcSUM_phenphase[pp] = 0;
		}

	}

	
	/* 2. first day of vegetation period */
	if (ctrl->yday == phen->onday)
	{
		epv->n_actphen = 1;
		phen->yday_phen = ctrl->yday;
	}

	/* 3. last day of vegetation period */
	if (phen->yday_phen == phen->offday+1)
	{
		epv->n_actphen = 0;
		phen->yday_phen = 0;
		phen->remdays_litfall =-1;
	}



		//if (!ctrl->HRV_flag && ctrl->yday == 0) metv->GDD = 0; // if no harvesting, only palnting, the start of the "new year" is the first day of year

	/* 3. slowing effects of degree day cumulation: vernalization and photoperiodic slowing effect */
	/* 3.1 phenophase of vernalization: slowing effect */
	if (epv->n_actphen == epc->n_vern_phenophase)
	{
		if (vernalization(epc, metv, phen))
		{
			printf("ERROR: vernalization() in phenphase.c... \n");
			ok=0;
		}
	}
	else
		phen->vern_dev_rate = 1;

	/* 3.2 phenophase of photoperiodic slowing effect */
	if (epv->n_actphen == epc->n_phpsl_phenophase)
	{
		if (photoslow(epc, metv, phen))
		{
			printf("ERROR: photoslow() in phenphase.c... \n");
			ok=0;
		}
	}
	else
		phen->phpsl_dev_rate = 1;

	dev_rate = MIN(phen->vern_dev_rate, phen->phpsl_dev_rate);

	
	/* 4. start of GDD calcuclation - first day of vegetation period (if no planting), day of planting (if planting) */
	if ((ctrl->PLT_flag == 0 && ctrl->yday > 0) || 
		(ctrl->PLT_flag > 0 && epv->n_actphen > 0)) 
	{
		/* if aboveground biomass exists -> air temperature, if not (plant is below the ground): soil temperature */
		if (epv->n_actphen >= epc->n_emerg_phenophase)
		{
			if (metv->tavg > epc->base_temp) 
			{
				metv->GDD       += (metv->tavg - epc->base_temp);
				metv->GDD_wMOD  += (metv->tavg - epc->base_temp) * dev_rate;
			}
		}
		else
		{
			if (metv->tsoil[ctrl->germ_layer] > epc->base_temp) 
			{
				metv->GDD       += (metv->tsoil[ctrl->germ_layer] - epc->base_temp);
				metv->GDD_wMOD  += (metv->tsoil[ctrl->germ_layer] - epc->base_temp) * phen->vern_dev_rate;
			}
		}
	}

	
	/* 5. determinig the borders of the new phenological phase */
	if (epv->n_actphen > 0  && epv->n_actphen < N_PHENPHASES && phen->vern_dev_rate == 1) 
	{
		pp = (int) epv->n_actphen- 1;
		phen->GDD_limit = epc->phenophase_length[pp];
		
		if (epv->n_actphen == 1)
		{
			if (metv->GDD_wMOD > phen->GDD_limit && epv->vwc[ctrl->germ_layer]/sitec->vwc_fc[ctrl->germ_layer] > epc->grmn_paramSWC && ctrl->yday > phen->onday)
			{
				phen->GDD_crit[pp] = metv->GDD_wMOD;
				epv->n_actphen += 1;
			}
		}
		else
		{
			if (metv->GDD_wMOD > phen->GDD_limit + phen->GDD_crit[pp-1])
			{
				phen->GDD_crit[pp] = metv->GDD_wMOD;
				epv->n_actphen += 1;
			}
		}
	}

	/* 6. calculation of transfer and litterfall days */
	if (epv->n_actphen)
	{
		/* calculation of remdays_transfer and predays_transfer */
		counter = (int) phen->yday_phen - (int) phen->onday;
		if (counter >= 0 && phen->yday_phen < phen->onday+phen->n_transferday) 
		{
			phen->remdays_transfer = phen->n_transferday - counter;
			phen->predays_transfer = phen->n_transferday - phen->remdays_transfer;
		}
		else
		{
			phen->remdays_transfer = 0;
			phen->predays_transfer = 0;
		}

	
		/* calculation of remdays_litfall and predays_litfall */
		counter=(int) phen->offday - (int) phen->yday_phen + 1;
		if (phen->yday_phen > phen->offday-phen->n_litfallday && phen->yday_phen <= phen->offday) 
		{
			phen->remdays_litfall = counter;
			phen->predays_litfall = phen->n_litfallday - phen->remdays_litfall;
		}
		else
		{
			phen->remdays_litfall = 0;	
			phen->predays_litfall = 0;

		}

	
		/* calculation of remdays_curgrowth */
		if (phen->yday_phen >= phen->onday && phen->yday_phen <= phen->offday) 
		{
			phen->remdays_curgrowth = phen->offday - phen->yday_phen + 1;
		}
		else
		{
			phen->remdays_curgrowth = 0;
		}

		phen->yday_phen += 1;
	}
	else
	{
		phen->remdays_transfer = 0;
		phen->predays_transfer = 0;
		if (ctrl->yday != phen->offday + 1) phen->remdays_litfall = 0;
		phen->predays_litfall = 0;
		phen->remdays_curgrowth = 0;
		phen->yday_phen = 0;
	}
	

			
	return (!ok);

}

int vernalization(const epconst_struct* epc, const metvar_struct* metv, phenology_struct *phen)
{
	int ok=1;
	double RVE; /* relative vernalization effectiveness */

	/* calculation of RVE of a given day */
	if (metv->tavg < epc->vern_parT1)
		RVE = 0;
	else
	{
		if (metv->tavg < epc->vern_parT2)
			RVE = (metv->tavg - epc->vern_parT1) / (epc->vern_parT2 - epc->vern_parT1);
		else
		{
			if(metv->tavg < epc->vern_parT3)
				RVE = 1;
			else
			{
				if(metv->tavg < epc->vern_parT4)
					RVE = (epc->vern_parT4 - metv->tavg) / (epc->vern_parT4 - epc->vern_parT3);
				else
					RVE = 0;
			}
		}
	}

	/* calculation of summarized vern_days of a given day -> vernalization development rate (can vary between 0 and 1) */
	phen->vern_days += RVE;
	if (phen->vern_days < epc->vern_parDR2)
		phen->vern_dev_rate = 1 - epc->vern_parDR1 * (epc->vern_parDR2 - phen->vern_days);
	else
	{
		phen->vern_dev_rate = 1;
	}

	/* minimum of develpoment rate is 0 */
	if (phen->vern_dev_rate < 0) 
		phen->vern_dev_rate = 0;
	else
		ok = 1;


	return (!ok);
}

int photoslow(const epconst_struct* epc, const metvar_struct* metv, phenology_struct *phen)
{
	int ok=1;
	double dayl_hour;
	
	dayl_hour = metv->dayl / (NSEC_IN_DAY / NHOUR_IN_DAY);
	
	if (dayl_hour < epc->phpsl_parDL)
		phen->phpsl_dev_rate = 1 - epc->phpsl_parDR * pow((epc->phpsl_parDL - dayl_hour),2);
	else
	{
		phen->phpsl_dev_rate = 1;
	}

	

	return (!ok);
}