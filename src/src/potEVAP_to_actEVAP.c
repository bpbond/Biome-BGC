/* 
potEVAP_to_actEVAP.c
calculation of actual soil evaporation from potential soil evaporation regarding to dry day limit (Joe Rictchie, 1970)

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
#include "bgc_constants.h"
#include "bgc_func.h"    
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))   

int potEVAP_to_actEVAP(control_struct* ctrl, const siteconst_struct* sitec, soilprop_struct* sprop, 
	                   epvar_struct* epv, wstate_struct* ws, wflux_struct* wf)
{

	/* internal variables */
	int errorCode=0;
	double infiltPOT, soilw_hw0,  evap_lack;
	
	sprop->coeff_evapLIM = 0.4;
	sprop->coeff_evapCUM = 3.5;
	sprop->coeff_DSRmax  = 5;

	
	
	infiltPOT = wf->infiltPOT + wf->soilw_from_GW0;
	

	/* 1. first evaporation phase - no limit */
	if (ws->soilEvapCUM1 < sprop->soilEvapLIM)
	{
		epv->DSR = 0;
	
		if (infiltPOT >= ws->soilEvapCUM1)
			ws->soilEvapCUM1 = 0;
		else
			ws->soilEvapCUM1 -= infiltPOT;
		
		if (!errorCode && evapPHASE1toPHASE2(sprop, epv, ws, wf))
		{
			printf("\n");
			printf("ERROR in evapPHASE1toPHASE2() in POTevap_to_actEVAP.c\n");
			errorCode=1; 
		} 

	}
	/* 2. second evaporation phase - limit of DSR: ws->soilEvapCUM1 > soilEvapLIM */
	else
	{
		if (infiltPOT >= ws->soilEvapCUM2) 
		{
			epv->DSR = 0;

			/* berszivárgó csapadék feltölt CUM2-t, a maradék meg a CUM1-bõl amennyit csak tud */
			infiltPOT             -= ws->soilEvapCUM2;
			ws->soilEvapCUM2   =0;

			if (infiltPOT > sprop->soilEvapLIM)
				ws->soilEvapCUM1  = 0;
			else
				ws->soilEvapCUM1  -= infiltPOT;

			if (!errorCode && evapPHASE1toPHASE2(sprop, epv, ws, wf))
			{
				printf("\n");
				printf("ERROR in evapPHASE1toPHASE2() in POTevap_to_actEVAP.c\n");
				errorCode=1; 
			} 
		}
		else
		{

			ws->soilEvapCUM2 -= infiltPOT;
			epv->DSR = pow((ws->soilEvapCUM2/sprop->coeff_evapCUM),2); 
			epv->DSR += 1;
			wf->soilw_evap = sprop->coeff_evapCUM * pow(epv->DSR, 0.5) - ws->soilEvapCUM2;
			
			if (wf->soilw_evap > wf->soilw_evapPOT) wf->soilw_evap = wf->soilw_evapPOT;
		
			ws->soilEvapCUM2 += wf->soilw_evap;
			epv->DSR = pow((ws->soilEvapCUM2/sprop->coeff_evapCUM),2); 
		}
	}


	/* control */
	if (wf->soilw_evap < 0 ||  epv->DSR < 0) 
	{
		printf("ERROR in soil evaporation calculation (potEVAP_to_actEVAP)\n");
		errorCode=1;
	}

	/* 3: pond water evaporation: water stored on surface which can not infiltrated because of saturation */
	if (ws->pondw > 0)
	{
		/* pond_flag: flag of WARNING writing (only at first time) */
		if (!ctrl->pond_flag ) ctrl->pond_flag = 1;
		if (wf->soilw_evapPOT < ws->pondw)
		{
			wf->pondw_evap = wf->soilw_evapPOT;
			wf->soilw_evap = 0;
		}
		else 
		{
			wf->pondw_evap =  ws->pondw;
			if (wf->pondw_evap < wf->soilw_evap)
				wf->soilw_evap -= wf->pondw_evap;
			else
				wf->soilw_evap = 0;
		}

	}


	/* 4. Soil evaporation can not be larger than the extractable soil water in the top layer */

	/* actual soil water content at theoretical lower limit of water content: hygroscopic water content */
	soilw_hw0 = sprop->VWChw[0] * sitec->soillayer_thickness[0] * water_density;

	/* evap_lack: control parameter to avoid negative soil water content (due to overestimated evaporation + dry soil) */
	evap_lack = wf->soilw_evap - (ws->soilw[0] - soilw_hw0);

	/* theoretical lower limit of water content: hygroscopic water content. */
	if (evap_lack > 0)
	{
		if (ws->soilEvapCUM2 >= sprop->soilEvapLIM)
		{
			if (ws->soilEvapCUM2 > wf->soilw_evap)
			{
				ws->soilEvapCUM2 = ws->soilEvapCUM2 - wf->soilw_evap + (ws->soilw[0] - soilw_hw0);
				epv->DSR = pow((ws->soilEvapCUM2/sprop->coeff_evapCUM),2); 
			}
			else
			{
				ws->soilEvapCUM1 =  ws->soilEvapCUM1 - (wf->soilw_evap - ws->soilEvapCUM2);
				
				ws->soilEvapCUM2 = (ws->soilw[0] - soilw_hw0) - sprop->soilEvapLIM;
				if (ws->soilEvapCUM2 < 0) ws->soilEvapCUM2 = 0;

				ws->soilEvapCUM1 =  ws->soilEvapCUM1 + (ws->soilw[0] - soilw_hw0);
				if (ws->soilEvapCUM1 < sprop->soilEvapLIM) ws->soilEvapCUM2 = sprop->soilEvapLIM;

				epv->DSR = pow((ws->soilEvapCUM2/sprop->coeff_evapCUM),2); 
			}
		}
		else
		{
			ws->soilEvapCUM1 =  ws->soilEvapCUM1 - wf->soilw_evap + (ws->soilw[0] - soilw_hw0);
		}

		wf->soilw_evap = (ws->soilw[0] - soilw_hw0);
		/* limitevap_flag: flag of WARNING writing in log file (only at first time) */
		if (fabs(evap_lack) > CRIT_PREC && !ctrl->limitevap_flag) ctrl->limitevap_flag = 1;
	}

		

	ws->soilw[0] -= (wf->soilw_evap);
	epv->VWC[0]  = ws->soilw[0] / water_density / sitec->soillayer_thickness[0];



	return (errorCode);
}

int evapPHASE1toPHASE2(const soilprop_struct* sprop, epvar_struct* epv, wstate_struct* ws, wflux_struct* wf)
{
	/* internal variables */
	int errorCode=0;

	ws->soilEvapCUM1 += wf->soilw_evapPOT;
	if (ws->soilEvapCUM1 > sprop->soilEvapLIM)
	{
		wf->soilw_evap = wf->soilw_evapPOT - sprop->coeff_evapLIM*(ws->soilEvapCUM1 - sprop->soilEvapLIM);
		ws->soilEvapCUM2 = (1-sprop->coeff_evapLIM)*(ws->soilEvapCUM1 - sprop->soilEvapLIM);
		epv->DSR = pow(ws->soilEvapCUM2/sprop->coeff_evapCUM,2);
		ws->soilEvapCUM1 = sprop->soilEvapLIM;
	}
	else
	{
		wf->soilw_evap = wf->soilw_evapPOT; 
	}



	wf->soilw_evap *= epv->evapREDmulch;

return (errorCode);
}