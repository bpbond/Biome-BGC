/* 
phenphase.c
calculation of n_actphen, GDD and GDDmod (based on vernalization and photoslow effect)

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


#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

int phenphase(file logfile, const control_struct* ctrl, const epconst_struct* epc, const soilprop_struct* sprop, const planting_struct* PLT, 
	          phenology_struct *phen, metvar_struct *metv, epvar_struct* epv, cstate_struct* cs)
{

	int pp,	counter, layer;
	double  dev_rate;
	int errorCode=0;
	int lastday = 0;
	double critVWC = 0;

	
	/* 0. number of the simulation days*/
	phen->yday_total = ctrl->simyr * nDAYS_OF_YEAR + ctrl->yday;
	
	/* 1. initalize GDD and phenological variables on the first day of the year */
	if (epv->n_actphen == 0) 
	{
		metv->GDD = 0;
		metv->GDD_wMOD = 0;
		epv->leafday = -1;
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
		cs->leafcSUM_phenphase[0]=cs->leafc;

		for (layer = 0; layer < N_SOILLAYERS; layer++) 
		{
			epv->m_SWCstress_layer[layer] = 1;
		}
		epv->m_SWCstress = 1;
		epv->m_SWCstressLENGTH = 1;
		epv->m_extremT = 1;
		

	}

	/* 2.1 first day of vegetation period */
	if (phen->yday_total == phen->onday)
	{
		epv->n_actphen = 1;

		/* writing out phenphases limits into logfile */
		if (ctrl->spinup == 0 && PLT->PLT_num) 
		{
			if (ctrl->simyr != 0) fprintf(logfile.ptr, " \n");
			
			if (ctrl->month > 9)
			{
				if (ctrl->day > 9) 
					fprintf(logfile.ptr, "%2i%s%2i%s",ctrl->month,"/",ctrl->day," ");
				else 
					fprintf(logfile.ptr, "%2i%2s%i%s",ctrl->month,"/0",ctrl->day," ");

			}
			else
			{
				if (ctrl->day > 9) 
					fprintf(logfile.ptr, "%s%i%s%2i%s","0", ctrl->month,"/",ctrl->day," ");
				else 
					fprintf(logfile.ptr, "%s%i%2s%i%s","0", ctrl->month,"/0",ctrl->day," ");
			}
		}
	}


	/* 2.2 last day of vegetation period */
	if ((phen->yday_total == phen->offday+1) || (phen->onday == DATA_GAP && phen->offday == DATA_GAP))
	{
		epv->n_actphen = 0;
		phen->onday = -1;
		phen->offday = -1;
		phen->remdays_litfall =-1;
		lastday = 1;

		epv->cumSWCstress = 0;
		epv->cumNstress = 0;
		epv->SWCstressLENGTH = 0;
	}

	/* 2.3 first day of phenological phases and flowering phenophase (0: in 1th of January) */

	if (epv->n_actphen == 0)
	{
		for (pp=0; pp<N_PHENPHASES; pp++) epv->phenphase_date[pp] = -1;
		epv->flower_date = 0;
		epv->winterEnd_date = 0;
	}
	
	pp = (int) epv->n_actphen; 
	
	
	if (pp == epc->n_flowHS_phenophase && ctrl->yday-epv->phenphase_date[pp-1] == 1) 
	{
		epv->flower_date = ctrl->yday;
	}

	if (pp == 4 && ctrl->yday-epv->phenphase_date[pp-1] == 1) 
	{
		epv->winterEnd_date = ctrl->yday-1;
	}
 

	/* 3. slowing effects of degree day cumulation: vernalization and photoperiodic slowing effect */
	/* 3.1 phenophase of vernalization: slowing effect */
	if (epv->n_actphen > 0 && epv->n_actphen == epc->n_vern_phenophase)
	{
		if (vernalization(epc, metv, phen))
		{
			printf("ERROR: vernalization() in phenphase.c... \n");
			errorCode=1;
		}
	}
	else
		phen->vern_dev_rate = 1;

	/* 3.2 phenophase of photoperiodic slowing effect */
	if (epv->n_actphen > 0 && epv->n_actphen == epc->n_phpsl_phenophase)
	{
		if (photoslow(epc, metv, phen))
		{
			printf("ERROR: photoslow() in phenphase.c... \n");
			errorCode=1;
		}
	}
	else
		phen->phpsl_dev_rate = 1;

	dev_rate = MIN(phen->vern_dev_rate, phen->phpsl_dev_rate);

	
	/* 4. start of GDD calcuclation - first day of vegetation period (if no planting), day of planting (if planting) */
	if ((PLT->PLT_num == 0 && ctrl->yday > 0) || 
		(PLT->PLT_num > 0 && epv->n_actphen > 0)) 
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
			if (metv->tsoil[epv->germ_layer] > epc->base_temp) 
			{
				metv->GDD       += (metv->tsoil[epv->germ_layer] - epc->base_temp);
				metv->GDD_wMOD  += (metv->tsoil[epv->germ_layer] - epc->base_temp) * phen->vern_dev_rate;
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
			/* first day of phenological phase nd depth of rooting zone from previous phenphase */
			if (epv->phenphase_date[pp] < 0) 
			{
				epv->phenphase_date[pp]=ctrl->yday;
				epv->rootdepth_phen[pp]=epv->rootdepth;
			}

			critVWC = sprop->VWCwp[epv->germ_layer] + epc->grmn_paramVWC *(sprop->VWCfc[epv->germ_layer] - sprop->VWCwp[epv->germ_layer]);
			if (metv->GDD_wMOD > phen->GDD_limit && epv->VWC[epv->germ_layer] > critVWC && phen->yday_total > phen->onday)
			{
				phen->GDD_crit[pp] = metv->GDD_wMOD;
				if (phen->yday_total != phen->onday) 
				{
					epv->n_actphen += 1;

					/* firstday and ndays of phenological phase */
					pp = (int) epv->n_actphen- 1;
					epv->phenphase_date[pp]=ctrl->yday;
					epv->rootdepth_phen[pp]=epv->rootdepth;
					
					/* writing out phenphases limits into logfile */
					if (ctrl->spinup == 0 && PLT->PLT_num) 
					{
						if (ctrl->month > 9)
						{
							if (ctrl->day > 9) 
								fprintf(logfile.ptr, "%2i%s%2i%s",ctrl->month,"/",ctrl->day," ");
							else 
								fprintf(logfile.ptr, "%2i%2s%i%s",ctrl->month,"/0",ctrl->day," ");

						}
						else
						{
							if (ctrl->day > 9) 
								fprintf(logfile.ptr, "%s%i%s%2i%s","0", ctrl->month,"/",ctrl->day," ");
							else 
								fprintf(logfile.ptr, "%s%i%2s%i%s","0", ctrl->month,"/0",ctrl->day," ");
						}
					}
				}
			}
		}
		else
		{
			if (metv->GDD_wMOD > phen->GDD_limit + phen->GDD_crit[pp-1])
			{
				phen->GDD_crit[pp] = metv->GDD_wMOD;
				epv->n_actphen += 1;

				/* first day of phenological phase */
				pp = (int) epv->n_actphen- 1;
				epv->phenphase_date[pp]=ctrl->yday;
				epv->rootdepth_phen[pp]=epv->rootdepth;
				
				/* writing out phenphases limits into logfile */
				if (ctrl->spinup == 0 && PLT->PLT_num) 
				{
					if (ctrl->month > 9)
					{
						if (ctrl->day > 9) 
							fprintf(logfile.ptr, "%2i%s%2i%s",ctrl->month,"/",ctrl->day," ");
						else 
							fprintf(logfile.ptr, "%2i%2s%i%s",ctrl->month,"/0",ctrl->day," ");

					}
					else
					{
						if (ctrl->day > 9) 
							fprintf(logfile.ptr, "%s%i%s%2i%s","0", ctrl->month,"/",ctrl->day," ");
						else 
							fprintf(logfile.ptr, "%s%i%2s%i%s","0", ctrl->month,"/0",ctrl->day," ");
					}
				}	
			}
		}
	}

	/* 6. calculation of transfer and litterfall days */
	if (epv->n_actphen)
	{
		/* calculation of remdays_transfer and predays_transfer */
		counter = (int) phen->yday_total - (int) phen->onday +1;
		if (counter >= 0 && phen->yday_total < phen->onday+phen->n_transferday) 
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
		counter=(int) phen->offday - (int) phen->yday_total;
		if (phen->yday_total > phen->offday-phen->n_litfallday && phen->yday_total <= phen->offday) 
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
		if (phen->yday_total >= phen->onday && phen->yday_total <= phen->offday) 
		{
			phen->remdays_curgrowth = phen->offday - phen->yday_total + 1;
		}
		else
		{
			phen->remdays_curgrowth = 0;
		}

	}
	else
	{
		phen->remdays_transfer = 0;
		phen->predays_transfer = 0;
		if (!lastday) phen->remdays_litfall = 0;
		phen->predays_litfall = 0;
		phen->remdays_curgrowth = 0;
		for (pp=0; pp<N_PHENPHASES; pp++) epv->rootdepth_phen[pp] = -1;

	}
	

			
	return (errorCode);

}

int vernalization(const epconst_struct* epc, const metvar_struct* metv, phenology_struct *phen)
{
	int errorCode=0;
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
		errorCode=0;


	return (errorCode);
}

int photoslow(const epconst_struct* epc, const metvar_struct* metv, phenology_struct *phen)
{
	int errorCode=0;
	double dayl_hour;
	
	dayl_hour = metv->dayl / (nSEC_IN_DAY / nHOURS_IN_DAY);
	
	if (dayl_hour < epc->phpsl_parDL)
		phen->phpsl_dev_rate = 1 - epc->phpsl_parDR * pow((epc->phpsl_parDL - dayl_hour),2);
	else
	{
		phen->phpsl_dev_rate = 1;
	}

	

	return (errorCode);
}