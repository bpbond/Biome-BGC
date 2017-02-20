/* 
sitec_init.c
Initialize the site physical constants for bgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.3
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2016, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "ini.h"
#include "bgc_struct.h"
#include "bgc_constants.h"
#include "pointbgc_struct.h"
#include "pointbgc_func.h"
#include "bgc_func.h"

int sitec_init(file init, siteconst_struct* sitec)
{
	/* reads the site physical constants from *.init */ 

	int ok=1;

	int layer, scanflag;
	char key[] = "SITE";
	char keyword[80];
	
	/* multilayer data */
	double sand, silt, clay, psi_fc, psi_wp, psi_sat;
	double soil_b, BD, RCN, vwc_sat, vwc_fc, vwc_wp, vwc_hw, hydr_conduct_sat, hydr_diffus_sat, hydr_conduct_fc,hydr_diffus_fc; 
	
	soil_b = BD = RCN = psi_sat = vwc_sat =vwc_wp = vwc_hw = hydr_conduct_sat = hydr_diffus_sat = hydr_conduct_fc =hydr_diffus_fc = 0;

	/*--------------------------------------------------------------------------------------*/
	/* 0. INITALIZING */

	/* predefined values: depth of the layers (soillayer_depth;[m]) */
	sitec->soillayer_depth[0] = 0.1;
	sitec->soillayer_depth[1] = 0.3;
	sitec->soillayer_depth[2] = 0.6;
	sitec->soillayer_depth[3] = 1.;
	sitec->soillayer_depth[4] = 2.;
	sitec->soillayer_depth[5] = 3.;
	sitec->soillayer_depth[6] = 10.;


	/* calculated values: depth of the layers (soillayer_midpoint;[m]) */
	sitec->soillayer_midpoint[0]  = 0.05;
	sitec->soillayer_midpoint[1] = 0.2;
	sitec->soillayer_midpoint[2] = 0.45;
	sitec->soillayer_midpoint[3] = 0.8;
	sitec->soillayer_midpoint[4] = 1.5;
	sitec->soillayer_midpoint[5] = 2.5;
	sitec->soillayer_midpoint[6] = 6.5;

	/* calculated values: thickness of the layers (soillayer_thickness;[m]) */
	sitec->soillayer_thickness[0]  = 0.1;
	sitec->soillayer_thickness[1] = 0.2;
	sitec->soillayer_thickness[2] = 0.3;
	sitec->soillayer_thickness[3] = 0.4;
	sitec->soillayer_thickness[4] = 1.0;
	sitec->soillayer_thickness[5] = 1.0;
	sitec->soillayer_thickness[6] = 7;

	/* initalization of groundwater depth parameter - Hidy 2015 */
	sitec->gwd_act=DATA_GAP;


	/*--------------------------------------------------------------------------------------
	1. READING INPUT DATA */

	/* 1.1 first scan keyword to ensure proper *.init format */ 
	if (ok && scan_value(init, keyword, 's'))
	{
		printf("Error reading keyword, sitec_init()\n");
		ok=0;
	}
	if (ok && strcmp(keyword,key))
	{
		printf("Expecting keyword --> %s in %s\n",key,init.name);
		ok=0;
	}


	/* 1.3.1 SAND array - mulilayer soil (Hidy 2015) */
	scanflag=0; 
	for (layer=0; layer<N_SOILLAYERS; layer++)
	{
		if (layer==N_SOILLAYERS-1) scanflag=1;
		if (ok && scan_array(init, &(sitec->sand[layer]), 'd', scanflag))
		{
			printf("Error reading percent sand in layer %i, sitec_init()\n", layer);
			ok=0;
		}
	}

	/* 1.3.2 SILT array - mulilayer soil (Hidy 2015)  */
	scanflag=0; 
	for (layer=0; layer<N_SOILLAYERS; layer++)
	{
		if (layer==N_SOILLAYERS-1) scanflag=1;
		if (ok && scan_array(init, &(sitec->silt[layer]), 'd', scanflag))
		{
			printf("Error reading percent silt in layer %i, sitec_init()\n", layer);
			ok=0;
		}
	}


	/* 1.4 other site data - elev, lat, alb */
	if (ok && scan_value(init, &sitec->elev, 'd'))
	{
		printf("Error reading elevation, sitec_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &sitec->lat, 'd'))
	{
		printf("Error reading site latitude, sitec_init()\n");
		ok=0;
	}
	if (ok && scan_value(init, &sitec->sw_alb, 'd'))
	{
		printf("Error reading shortwave albedo, sitec_init()\n");
		ok=0;
	}
	

	/* 1.5.  FIRST APPROXIMATION to initalize multilayer soil temperature -> mean_surf_air_temp [Celsius] - Hidy 2010 */
	if (ok && scan_value(init, &sitec->tair_annavg, 'd'))
	{
		printf("Error reading tair_annavg, sitec_init()\n");
		ok=0;
	}

	if (ok && scan_value(init, &sitec->tair_annrange, 'd'))
	{
		printf("Error reading tair_annrange, sitec_init()\n");
		ok=0;
	}

	/* 1.6 runoff parameter (Campbell and Diaz) - Hidy 2010 */
	if (ok && scan_value(init, &sitec->RCN_mes, 'd'))
	{
		printf("Error reading measured runoff curve number: sitec_init()\n");
		ok=0;
	}

	/* 1.7.1 measured bulk density  (Hidy 2015)  */
	scanflag=0; 
	for (layer=0; layer<N_SOILLAYERS; layer++)
	{
		if (layer==N_SOILLAYERS-1) scanflag=1;
		if (ok && scan_array(init, &(sitec->BD_mes[layer]), 'd', scanflag))
		{
			printf("Error reading BD_mes in layer %i, sitec_init()\n", layer);
			ok=0;
		}
	}

	/* 1.7.2 measured critical VWC values - saturation  (Hidy 2015)  */
	scanflag=0; 
	for (layer=0; layer<N_SOILLAYERS; layer++)
	{
		if (layer==N_SOILLAYERS-1) scanflag=1;
		if (ok && scan_array(init, &(sitec->vwc_sat_mes[layer]), 'd', scanflag))
		{
			printf("Error reading percent vwc_sat_mes in layer %i, sitec_init()\n", layer);
			ok=0;
		}
	}
	
	/* 1.7.3 measured critical VWC values - field capacity   (Hidy 2015)  */
	scanflag=0; 
	for (layer=0; layer<N_SOILLAYERS; layer++)
	{
		if (layer==N_SOILLAYERS-1) scanflag=1;
		if (ok && scan_array(init, &(sitec->vwc_fc_mes[layer]), 'd', scanflag))
		{
			printf("Error reading percent vwc_fc_mes in layer %i, sitec_init()\n", layer);
			ok=0;
		}
	}
	
	/* 1.7.4 measured critical VWC values - wilting point  (Hidy 2015)  */
	scanflag=0; 
	for (layer=0; layer<N_SOILLAYERS; layer++)
	{
		if (layer==N_SOILLAYERS-1) scanflag=1;
		if (ok && scan_array(init, &(sitec->vwc_wp_mes[layer]), 'd', scanflag))
		{
			printf("Error reading percent vwc_wp_mes in layer %i, sitec_init()\n", layer);
			ok=0;
		}
	}

	/* 1.7.5 measured critical VWC values - hygr. water  (Hidy 2015)  */
	scanflag=0; 
	for (layer=0; layer<N_SOILLAYERS; layer++)
	{
		if (layer==N_SOILLAYERS-1) scanflag=1;
		if (ok && scan_array(init, &(sitec->vwc_hw_mes[layer]), 'd', scanflag))
		{
			printf("Error reading percent vwc_hw_mes in layer %i, sitec_init()\n", layer);
			ok=0;
		}
	}

	


	/* 1.8 CONTROL - measured VWC values (all or none should to be set) */
	if (sitec->BD_mes[0] == DATA_GAP ||sitec->vwc_sat_mes[0] == DATA_GAP || 
		sitec->vwc_fc_mes[0] == DATA_GAP || sitec->vwc_wp_mes[0] == DATA_GAP  || sitec->vwc_hw_mes[0] == DATA_GAP)
	{

	
		if (sitec->BD_mes[0] == DATA_GAP && sitec->vwc_sat_mes[0] == DATA_GAP && 
			sitec->vwc_fc_mes[0] == DATA_GAP && sitec->vwc_wp_mes[0] == DATA_GAP  && sitec->vwc_hw_mes[0] == DATA_GAP)
		{	
			for (layer=0; layer < N_SOILLAYERS; layer++)
			{
				sitec->BD_mes[layer]      = DATA_GAP;
				sitec->vwc_sat_mes[layer] = DATA_GAP;
				sitec->vwc_fc_mes[layer]  = DATA_GAP;
				sitec->vwc_wp_mes[layer]  = DATA_GAP;
				sitec->vwc_hw_mes[layer]  = DATA_GAP;
			}
		}
		else
		{
			printf("Error in measured soil data in INI file\n");
			printf("All or none measured bulk density and critical VWC data should to be set by the user\n");
			ok=0;
		}
	}


	/* CONTROL to avoid negative meteorological data */
 	if (sitec->sw_alb < 0 )
	{
		printf("Error in site data in INI file: swalb, sitec_init()\n");
		ok=0;
	}



	/* -------------------------------------------------------------------------------------------------------------------------------*/
	/* 2. MULTILAYER SOIL CALCULATION */


	/* 2.1 estimated soil water potential at hygroscopic water in MPa (1MPa = 10000cm)  (fc: pF = 2.5; wp: pF = 4.2) */
	sitec->psi_hw  = pow(10,pF_hygroscopw) / (-10000);

	
	/* 2.2 	Calculate the soil pressure-volume coefficients from texture data (soil water content, soil water potential and Clapp-Hornberger parameter) 
		    - different estimation methods 4: modell estimtaion, talajharomszog, measured data )*/

	for (layer=0; layer < N_SOILLAYERS; layer++)
	{

		sand		= sitec->sand[layer];
		silt		= sitec->silt[layer];
		clay		= 100-sand-silt;

		sitec->clay[layer] = clay;

	
		/*  2.2.1 CONTROL to avoid negative data - Hidy 2013 */
 		if (sand < 0 || silt < 0 || clay < 0)
		{
			printf("Error in site data in INI file: sand/silt/clay/vwc data, sitec_init()\n");
			printf("Check values in initialization file.\n");
			ok=0;
		}
	

	
		/* 2.2.2 saturation value of soil water potential */
		psi_sat = -(exp((1.54 - 0.0095*sand + 0.0063*silt)*log(10.0))*9.8e-5);

		if (soilb_estimation(sand, silt, &soil_b, &vwc_sat, &vwc_fc, &vwc_wp,&BD, &RCN))
		{
			printf("Error: soilb_estimation() for sitec_init \n");
			ok=0;
		}	

		/* using the 4M model soil properties */
		if (sitec->vwc_sat_mes[layer] == (double) DATA_GAP)
		{
			/* VWC_fc, VWC_wp VWC_HW are the functions of VWC_sat and soil_b */
			vwc_hw = vwc_sat * (log(soil_b) / log(sitec->psi_hw/psi_sat));
		    psi_fc = exp(vwc_sat/vwc_fc*log(soil_b))*psi_sat;
			psi_wp = exp(vwc_sat/vwc_wp*log(soil_b))*psi_sat;

			/* control for soil type with high clay content - Hidy 2013. */
			if (vwc_sat - vwc_fc < 0.01)  vwc_fc = vwc_sat - 0.01;
			if (vwc_fc  - vwc_wp < 0.01)  vwc_wp = vwc_fc  - 0.01;
			if (vwc_wp  - vwc_hw < 0.01)  vwc_hw = vwc_wp  - 0.01;
	
		}
		/* measured soil water content at wilting point, field capacity and saturation in m3/m3 */
		else 	
		{

			vwc_sat = sitec->vwc_sat_mes[layer];
			vwc_fc  = sitec->vwc_fc_mes[layer];
			vwc_wp  = sitec->vwc_wp_mes[layer];
			vwc_hw  = sitec->vwc_hw_mes[layer];
			BD      = sitec->BD_mes[layer];


			psi_fc = exp(vwc_sat/vwc_fc*log(soil_b))*psi_sat;
			psi_wp = exp(vwc_sat/vwc_wp*log(soil_b))*psi_sat;

			
		}
		
	
		/* 2.2.4 CONTROL - measured VWC values: SAT>FC>WP>HW */
		if ((vwc_sat - vwc_fc) < 0.01 || (vwc_fc - vwc_wp) < 0.01  || (vwc_wp - vwc_hw) < 0.01 || vwc_hw < 0.01 || vwc_sat > 1.0) 
		{
			printf("Error in measured VWC data in INI file\n");
			printf("rules: VWC_sat > VWC_fc; VWC_fc > VWC_wp; VWC_wp > VWC_hw; VWC_sat <1.0, VWC_hw>0.01\n");
			ok=0;
		}


		/* 2.2.5 hydr. conduct and diffusivity at saturation and field capacity(Cosby et al. 1984)*/
		hydr_conduct_sat = 7.05556 * 1e-6 * pow(10, (-0.6+0.0126*sand-0.0064*clay));
		hydr_diffus_sat = (soil_b * hydr_conduct_sat * (-100*psi_sat))/vwc_sat;
		hydr_conduct_fc = hydr_conduct_sat * pow(vwc_fc/vwc_sat, 2*soil_b+3);
		hydr_diffus_fc = (((soil_b * hydr_conduct_sat * (-100*psi_sat))) / vwc_sat) * 
							pow(vwc_fc/vwc_sat, soil_b+2);
			

		
 		sitec->soil_b[layer]			= soil_b;
		sitec->BD[layer]                = BD;
		sitec->psi_sat[layer]			= psi_sat;
		sitec->psi_fc[layer]			= psi_fc;
		sitec->psi_wp[layer]			= psi_wp;
		sitec->vwc_sat[layer]			= vwc_sat;
		sitec->vwc_fc[layer]			= vwc_fc;
		sitec->vwc_wp[layer]			= vwc_wp;
		sitec->vwc_hw[layer]			= vwc_hw;
		sitec->hydr_conduct_sat[layer]	= hydr_conduct_sat;
		sitec->hydr_diffus_sat[layer]	= hydr_diffus_sat;
		sitec->hydr_conduct_fc[layer]	= hydr_conduct_fc;
		sitec->hydr_diffus_fc[layer]	= hydr_diffus_fc;

		if (layer == 0) 
		{
			if (sitec->RCN_mes == DATA_GAP)
				sitec->RCN  = RCN;
			else
				sitec->RCN  = sitec->RCN_mes;
		}

	}

		
	
 	return (!ok);
}

int soilb_estimation(double sand, double silt, double* soil_b, double* vwc_sat, double* vwc_fc, double* vwc_wp, double* BD, double* RCN)
{

	int soiltype=-1;
	int ok=1;
	
	double clay = 100-sand-silt;

	double soilb_array[12]      = {3.45,4.11,5.26,6.12,5.39,4.02,7.63,7.71,8.56,9.22,10.45,12.46};
//	double soilb_array[12]      = {2.69,3.03,3.42,3.69,4.18,3.77,3.94,4.37,4.32,4.60,5.19,5.32};
	double VWCsat_array[12]     = {0.4,0.42,0.44,0.46,0.48,0.49,0.5,0.505,0.51,0.515,0.52,0.525};
	double VWCfc_array[12]      = {0.155,0.190,0.250,0.310,0.360,0.380,0.390,0.405,0.420,0.435,0.445,0.460};
	double VWCwp_array[12]      = {0.030,0.050,0.090,0.130,0.170,0.190,0.205,0.220,0.240,0.260,0.275,0.290};
	double BD_array[12]         = {1.6,1.58,1.56,1.54,1.52,1.5,1.48,1.46,1.44,1.42,1.4,1.38};
	double RCN_array[12]        = {50,52,54,58,60,56,62,66,64,68,70,72};


	






	if (silt+1.5*clay < 15)													//sand 
		soiltype=0;	

	if ((silt+1.5*clay > 15) && (silt+2*clay < 30)) 						//loamy_sand
		soiltype=1;	

	if ((clay >= 7 && clay < 20) && (sand > 52) && (silt+2*clay >= 30))		//sandy_loam
		soiltype=2;  

	if (clay < 7 && silt < 50 && silt+2*clay >= 30)   						//sandy_loam
		soiltype=2;

	if (clay >= 7 && clay < 27 && silt >= 28 && silt < 50 && sand <= 52)	//loam
		soiltype=3;

	if (silt >= 50 && clay >= 12 && clay < 27)								//silt_loam
		soiltype=4;

	if (silt >= 50 && silt < 80 && clay < 12)								//silt_loam
		soiltype=4;

	if (silt >= 80 && clay < 12)											//silt
		soiltype=5;

	if (clay >= 20 && clay < 35 && silt < 28 && sand > 45)					//sandy_clay_loam
		soiltype=6;

	if (clay >= 27 && clay < 40 && sand > 20 && sand <= 45) 				//clay_loam	 
		soiltype=7;

	if (clay >= 27 && clay < 40 && sand <= 20)  							//silty_clay_loam				 
		soiltype=8;

	if (clay >= 35 && sand > 45)  											//sandy_clay					 
		soiltype=9;

	if (clay >= 40 && silt >= 40) 											//silty_clay						 
		soiltype=10;

	if (clay >= 40 && sand <= 45 && silt < 40) 								//clay					 
		soiltype=11;

	if (soiltype >= 0)
	{
		*soil_b  = soilb_array[soiltype];
		*vwc_sat = VWCsat_array[soiltype];
		*vwc_fc  = VWCfc_array[soiltype];
		*vwc_wp  = VWCwp_array[soiltype];
		*BD      = BD_array[soiltype];
		*RCN     = RCN_array[soiltype];
	}
	else
	{	
		printf("Error in soiltype founding\n");
		ok=0;
	}

  return (!ok);
  
}
