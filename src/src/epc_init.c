/* 
epc_init.c
read epc file for pointbgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2020, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
20.03.01 Galina Churkina added variable "sum" substituting  t1+t2+t3 in IF statement,
which gave an error.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "ini.h"
#include "bgc_struct.h"
#include "pointbgc_struct.h"
#include "pointbgc_func.h"
#include "bgc_constants.h"



int epc_init(file init, epconst_struct* epc, control_struct* ctrl, int EPCfromINI)
{
	int errorCode=0;
	int dofilecloseEPC = 1;
	double t1 = 0;
	double t2 = 0;
	double t3 = 0;
	double t4,r1;
	double sum;
	int phenphase, scanflag;
	file epc_file, WPM_file, MSC_file, SGS_file, EGS_file; 	
	char key[] = "EPC_FILE";
	char keyword[STRINGSIZE];
	char header[STRINGSIZE];

	int p1;
	double p2;

	int* OPTINyear_array;							
	int dataread;
	int ndata = 0;


	ctrl->planttypeName = (char*) malloc(STRINGSIZE * sizeof(char));
			
	/* allocate space for the optional input temporal variables */
	OPTINyear_array    = (int*) malloc(ctrl->simyears*sizeof(int));  
	
	
	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** EPC_FILES                                                       ** 
	**                                                                 **
	********************************************************************/
	
	/* if EPC file reading occurs from INI  */
	
	if (EPCfromINI)
	{
		/* scan for the EPC file keyword, exit if not next */
		if (!errorCode && scan_value(init, keyword, 's'))
		{
			printf("ERROR reading keyword for control data\n");
			errorCode=209;
		}
		if (!errorCode && strcmp(keyword, key))
		{
			printf("Expecting keyword --> %s in file %s\n",key,init.name);
			errorCode=209;
		}
		/* open simple EPC file  */
		if (!errorCode && scan_open(init,&epc_file,'r',1)) 
		{
			printf("ERROR opening epconst file, epc_init()\n");
			dofilecloseEPC = 0;
			errorCode=20900;
		}
	}
	else
	{
		epc_file = init;
		/* open simple EPC file */
		if (!errorCode && file_open(&epc_file,'r',1)) 
		{
			printf("ERROR opening epconst file, epc_init()\n");
			dofilecloseEPC = 0;
			errorCode=20900;
		}
	}

	/* first scan epc keyword to ensure proper *.epc format */
	if (!errorCode && scan_value(epc_file, ctrl->planttypeName, 's'))
	{
		printf("ERROR reading keyword, epc_init()\n");
		errorCode=20900;
	}



	/****************************************************************************************************************/
	/* dividing line from file */ 
	if (!errorCode && scan_value(epc_file, header, 's'))
	{
		printf("ERROR reading 1. dividing line, epc_init()\n");
		errorCode=20901;
	}
	if (!errorCode && scan_value(epc_file, header, 's'))
	{
		printf("ERROR reading 2. dividing line, epc_init()\n");
		errorCode=20901;
	}
	/****************************************************************************************************************/
	/* FLAGS */
	/****************************************************************************************************************/
	if (!errorCode && scan_value(epc_file, &epc->woody, 'i'))
	{
		printf("ERROR reading woody/non-woody flag, epc_init()\n");
		errorCode=20902;
	}
	if (!errorCode && scan_value(epc_file, &epc->evergreen, 'i'))
	{
		printf("ERROR reading evergreen/deciduous flag, epc_init()\n");
		errorCode=20903;
	}
	if (!errorCode && scan_value(epc_file, &epc->c3_flag, 'i'))
	{
		printf("ERROR reading C3/C4 flag, epc_init()\n");
		errorCode=20904;
	}

	
	/****************************************************************************************************************/
	/* dividing line from file */ 
	
	if (!errorCode && scan_value(epc_file, header, 's'))
	{
		printf("ERROR reading 2. dividing line, epc_init()\n");
		errorCode=20905;
	}
	if (!errorCode && scan_value(epc_file, header, 's'))
	{
		printf("ERROR reading 2. dividing line, epc_init()\n");
		errorCode=20905;
	}

	/****************************************************************************************************************/
	/* PLANT FUNCTIONING PARAMETERS */
	/****************************************************************************************************************/
	
	/*-------------------------------------------------------------*/
	/* using varying onday values (in transient or in normal run)*/
	if (!errorCode && scan_value(epc_file, &epc->onday, 'i'))
	{
		printf("ERROR reading onday, epc_init()\n");
		errorCode=20906;
	}

	if (ctrl->spinup == 0) 
		strcpy(SGS_file.name,  "onday_normal.txt");
	else
	{
		if (ctrl->spinup == 1) 
			strcpy(SGS_file.name,  "onday_spinup.txt");
		else
			strcpy(SGS_file.name,  "onday_transient.txt");
	}
	
	/* SGS flag: constans or varying SGS from file */
	if (!errorCode && !file_open(&SGS_file,'j',1)) 
		ctrl->varSGS_flag = 1;
	else
		ctrl->varSGS_flag = 0;

	if (ctrl->varSGS_flag == 1 && epc->onday ==  DATA_GAP)
	{
		printf("ERROR in using annual varying onday data, if user-defined bareground simulation is defined (onday=-9999), epc_init()\n");
		errorCode=20906;
	}

	if (!errorCode && ctrl->varSGS_flag) 
	{
		/* allocate space for the annual SGS array */
		epc->SGS_array = (double*) malloc(ctrl->simyears * sizeof(double));
		if (!epc->SGS_array)
		{
			printf("ERROR allocating for annual SGS array, epc_init()\n");
			errorCode=20906;
		}
	
		/* initialization with default value */
		for (ndata = 0; ndata < ctrl->simyears; ndata++) epc->SGS_array[ndata] = epc->onday;
		
		ndata=0;
		while (!errorCode && !(dataread = scan_array (SGS_file, &p1, 'i', 0, 0)))
		{
			dataread = fscanf(SGS_file.ptr, "%lf%*[^\n]", &p2);
				
			if (p1 >= ctrl->simstartyear && p1 < ctrl->simstartyear + ctrl->simyears)
			{
				OPTINyear_array[ndata] = p1;
				epc->SGS_array[ndata]  = p2;

				if (epc->SGS_array[ndata] < 0.0 && epc->SGS_array[ndata] != DATA_GAP)
				{
					printf("ERROR in epc_init(): sgs must be positive\n");
					errorCode=20906;
				}
			
				ndata += 1;
			}
		}
		/* read year and SGS for each simday in each simyear */
		
		fclose(SGS_file.ptr);
	}	
	else /* if no changing data constant EPC parameter are used */
	{
		epc->SGS_array = 0;
	}	

	/*-------------------------------------------------------------*/
	/* using varying offday values (in transient or in normal run)*/

	if (!errorCode && scan_value(epc_file, &epc->offday, 'i'))
	{
		printf("ERROR reading offday, epc_init()\n");
		errorCode=20907;
	}

	if (ctrl->spinup == 0) 
		strcpy(EGS_file.name, "offday_normal.txt");
	else
	{
		if (ctrl->spinup == 1) 
			strcpy(EGS_file.name, "offday_spinup.txt");
		else
			strcpy(EGS_file.name, "offday_transient.txt");
	}
	
	/* EGS flag: constans or varying EGS from file */
	if (!errorCode && !file_open(&EGS_file,'j',1)) 
		ctrl->varEGS_flag = 1;
	else
		ctrl->varEGS_flag = 0;


	if (ctrl->varEGS_flag == 1 && epc->offday ==  DATA_GAP)
	{
		printf("ERROR in using annual varying offday data, if user-defined bareground simulation is defined (offday=-9999), epc_init()\n");
		errorCode=20906;
	}

	if (!errorCode && ctrl->varEGS_flag) 
	{
		/* allocate space for the annual EGS array */
		epc->EGS_array = (double*) malloc(ctrl->simyears * sizeof(double));
		if (!epc->EGS_array)
		{
			printf("ERROR allocating for annual EGS array, epc_init()\n");
			errorCode=20907;
		}
	
		/* initialization with default value */
		for (ndata = 0; ndata < ctrl->simyears; ndata++) epc->EGS_array[ndata] = epc->offday;
		
		ndata=0;
		while (!errorCode && !(dataread = scan_array (EGS_file, &p1, 'i', 0, 0)))
		{
			dataread = fscanf(EGS_file.ptr, "%lf%*[^\n]", &p2);
				
			if (p1 >= ctrl->simstartyear && p1 < ctrl->simstartyear + ctrl->simyears)
			{
				OPTINyear_array[ndata] = p1;
				epc->EGS_array[ndata]  = p2;

				if (epc->EGS_array[ndata] < 0.0 && epc->EGS_array[ndata] != DATA_GAP)
				{
					printf("ERROR in epc_init(): EGS must be positive\n");
					errorCode=20907;
				}
			
				ndata += 1;
			}
		}
		/* read year and EGS for each simday in each simyear */
		
		fclose(EGS_file.ptr);
	}	
	else /* if no changing data constant EPC parameter are used */
	{
		epc->EGS_array = 0;
	}	
	
	/*-------------------------------------------------------------*/
	/* transfer growth and litterfall period */
	if (!errorCode && scan_value(epc_file, &epc->transfer_pdays, 'd'))
	{
		printf("ERROR reading transfer_pdays, epc_init()\n");
		errorCode=20908;
	}
	if (!errorCode && scan_value(epc_file, &epc->litfall_pdays, 'd'))
	{
		printf("ERROR reading litfall_pdays, epc_init()\n");
		errorCode=20909;
	}
	
	/*-------------------------------------------------------------*/
	/* base temperature for calculation GDD / heatsum  */
	if (!errorCode && scan_value(epc_file, &epc->base_temp, 'd'))
	{
		printf("ERROR reading base_temp parameter: epc_init()\n");
		errorCode=20910;
	}

	/*-------------------------------------------------------------*/
	/* minimum/optimal/maximum temperature for growth displayed on current day (-9999: no T-dependence of allocation) */
	if (!errorCode && scan_value(epc_file, &epc->pnow_minT, 'd'))
	{
		printf("ERROR reading pnow_minT parameter: epc_init()\n");
		errorCode=20911;
	}
	if (!errorCode && scan_value(epc_file, &epc->pnow_opt1T, 'd'))
	{
		printf("ERROR reading pnow_opt1T parameter: epc_init()\n");
		errorCode=20911;
	}
	if (!errorCode && scan_value(epc_file, &epc->pnow_opt2T, 'd'))
	{
		printf("ERROR reading pnow_opt1T parameter: epc_init()\n");
		errorCode=20911;
	}
	if (!errorCode && scan_value(epc_file, &epc->pnow_maxT, 'd'))
	{
		printf("ERROR reading pnow_maxT parameter: epc_init()\n");
		errorCode=20911;
	}

	/* control of temperature for growth data */
	if (!errorCode)
	{
		if (epc->pnow_minT == DATA_GAP ||epc->pnow_opt1T == DATA_GAP || epc->pnow_opt2T == DATA_GAP || epc->pnow_maxT == DATA_GAP)
		{
			if (epc->pnow_minT != DATA_GAP ||epc->pnow_opt1T != DATA_GAP || epc->pnow_opt2T != DATA_GAP || epc->pnow_maxT != DATA_GAP) 
			{
				printf("ERROR in minimum/optimal/maximum temperature for growth data in EPC file\n");
				printf("All or none temperature data should to be set by the user\n");
				errorCode=2091101;
			}
		}
		if (epc->pnow_minT > epc->pnow_opt1T ||epc->pnow_opt1T > epc->pnow_opt2T || epc->pnow_opt2T > epc->pnow_maxT) 
		{
				printf("ERROR in minimum/optimal/maximum temperature for pnow data in EPC file\n");
				printf("Correct temperature data: minT <= opt1T <= opt2T <= maxT\n");
				errorCode=2091102;
		}
	}

	/*-------------------------------------------------------------*/
	/* minimum/optimal/maximum temperature for C-assimilation displayed on current day (-9999: no limitation) */
	
	if (!errorCode && scan_value(epc_file, &epc->assim_minT, 'd'))
	{
		printf("ERROR reading assim_minT parameter: epc_init()\n");
		errorCode=20912;
	}
	if (!errorCode && scan_value(epc_file, &epc->assim_opt1T, 'd'))
	{
		printf("ERROR reading assim_opt1T parameter: epc_init()\n");
		errorCode=20912;
	}
	if (!errorCode && scan_value(epc_file, &epc->assim_opt2T, 'd'))
	{
		printf("ERROR reading assim_opt1T parameter: epc_init()\n");
		errorCode=20912;
	}
	if (!errorCode && scan_value(epc_file, &epc->assim_maxT, 'd'))
	{
		printf("ERROR reading assim_maxT parameter: epc_init()\n");
		errorCode=20912;
	}
	
	/* control of temperature data for C-assimilation */
	if (!errorCode)
	{
		if (epc->assim_minT == DATA_GAP ||epc->assim_opt1T == DATA_GAP || epc->assim_opt2T == DATA_GAP || epc->assim_maxT == DATA_GAP)
		{
			if (epc->assim_minT != DATA_GAP ||epc->assim_opt1T != DATA_GAP || epc->assim_opt2T != DATA_GAP || epc->assim_maxT != DATA_GAP) 
			{
				printf("ERROR in minimum/optimal/maximum temperature for assim data in EPC file\n");
				printf("All or none temperature data should to be set by the user\n");
				errorCode=2091201;
			}
		}
		if (epc->assim_minT > epc->assim_opt1T ||epc->assim_opt1T > epc->assim_opt2T || epc->assim_opt2T > epc->assim_maxT) 
		{
				printf("ERROR in minimum/optimal/maximum temperature for assim data in EPC file\n");
				printf("Correct temperature data: minT <= opt1T <= opt2T <= maxT\n");
				errorCode=2091202;
		}
	}

	/*-------------------------------------------------------------*/
	/*  leaf turnover fraction */
	if (!errorCode && scan_value(epc_file, &epc->nonwoody_turnover, 'd'))
	{
		printf("ERROR reading non-woody biomass turnover, epc_init()\n");
		errorCode=20913;
	}
	/* force leaf turnover fraction to 1.0 if deciduous */
	if (!epc->evergreen)
	{
		epc->nonwoody_turnover = 1.0;
	}
	if (!errorCode && scan_value(epc_file, &epc->woody_turnover, 'd'))
	{
		printf("ERROR reading woody biomass turnover, epc_init()\n");
		errorCode=20914;
	}

	/* ------------------------------------------------------ */
	/* fire mortality */
	if (!errorCode && scan_value(epc_file, &t1, 'd'))
	{
		printf("ERROR reading fire mortality, epc_init()\n");
		errorCode=20915;
	}
	if (!errorCode) epc->daily_fire_turnover = t1/nDAYS_OF_YEAR;

	/*-------------------------------------------------------------*/
	/* whole plant mortality */

	if (!errorCode && scan_value(epc_file, &epc->wholeplant_mortality, 'd'))
	{
		printf("ERROR reading whole-plant mortality, epc_init()\n");
		errorCode=20916;
	}

	/* using varying whole plant mortality values */
	if (ctrl->spinup == 0)
		strcpy(WPM_file.name, "mortality_normal.txt");
	else
	{
		if (ctrl->spinup == 1) 
			strcpy(WPM_file.name, "mortality_spinup.txt");
		else
			strcpy(WPM_file.name, "mortality_transient.txt");
	}

	/* WPM flag: constans or varying EGS from file */
	if (!errorCode && !file_open(&WPM_file,'j',1)) 
		ctrl->varWPM_flag = 1;
	else
		ctrl->varWPM_flag = 0;
	
	if (!errorCode && ctrl->varWPM_flag) 
	{
		/* allocate space for the annual WPM array */
		epc->WPM_array = (double*) malloc(ctrl->simyears * sizeof(double));
		if (!epc->WPM_array)
		{
			printf("ERROR allocating for annual WPM array, epc_init()\n");
			errorCode=20916;
		}
	
		/* initialization with default value */
		for (ndata = 0; ndata < ctrl->simyears; ndata++) epc->WPM_array[ndata] = epc->wholeplant_mortality;
		
		ndata=0;
		while (!errorCode && !(dataread = scan_array (WPM_file, &p1, 'i', 0, 0)))
		{
			dataread = fscanf(WPM_file.ptr, "%lf%*[^\n]", &p2);
				
			if (p1 >= ctrl->simstartyear && p1 < ctrl->simstartyear + ctrl->simyears)
			{
				OPTINyear_array[ndata] = p1;
				epc->WPM_array[ndata]  = p2;

				if (epc->WPM_array[ndata] < 0.0 && epc->WPM_array[ndata] != DATA_GAP)
				{
					printf("ERROR in epc_init(): WPM must be positive\n");
					errorCode=20916;
				}
			
				ndata += 1;
			}
		}
		/* read year and WPM for each simday in each simyear */
		
		fclose(WPM_file.ptr);
	}	
	else /* if no changing data constant EPC parameter are used */
	{
		epc->WPM_array = 0;
	}	
	

	/*--------------------------------------------------------------------*/
	/* C:N ratios */
	if (!errorCode && scan_value(epc_file, &epc->leaf_cn, 'd'))
	{
		printf("ERROR reading average leaf C:N, epc_init()\n");
		errorCode=20917;
	}

	/* test for leaf C:N > 0 */
	if (!errorCode && epc->leaf_cn <= 0	)
	{
		printf("ERROR: leaf  C:N must be > 0\n");
		printf("change the values in EPC file\n");
		errorCode=20917;
	}

	if (!errorCode && scan_value(epc_file, &epc->leaflitr_cn, 'd'))
	{
		printf("ERROR reading leaf litter C:N, epc_init()\n");
		errorCode=20917;
	}
	/* test for leaflitter C:N > leaf C:N */
	if (!errorCode && epc->leaflitr_cn < epc->leaf_cn)
	{
		printf("ERROR: leaf litter C:N must be >= leaf C:N\n");
		printf("change the values in EPC file\n");
		errorCode=20917;
	}

	if (!errorCode && scan_value(epc_file, &epc->froot_cn, 'd'))
	{
		printf("ERROR reading initial fine root C:N, epc_init()\n");
		errorCode=20917;
	}


	if (!errorCode && scan_value(epc_file, &epc->fruit_cn, 'd'))
	{
		printf("ERROR reading initial fruit C:N, epc_init()\n");
		errorCode=20917;
	}

	if (!errorCode && scan_value(epc_file, &epc->softstem_cn, 'd'))
	{
		printf("ERROR reading initial softstem C:N, epc_init()\n");
		errorCode=20917;
	}

	if (!errorCode && scan_value(epc_file, &epc->livewood_cn, 'd'))
	{
		printf("ERROR reading initial livewood C:N, epc_init()\n");
		errorCode=20917;
	}

	if (!errorCode && scan_value(epc_file, &epc->deadwood_cn, 'd'))
	{
		printf("ERROR reading initial deadwood C:N, epc_init()\n");
		errorCode=20917;
	}


	/* test for froot C:N > leaf C:N */
	if (!errorCode && epc->froot_cn < epc->leaf_cn)
	{
		printf("ERROR: fine root C:N must be >= leaf C:N\n");
		printf("change the values in EPC file\n");
		errorCode=2091701;
	}

	if (!errorCode && epc->fruit_cn > 0 && epc->fruit_cn < epc->leaf_cn)
	{
		printf("ERROR: fruit C:N must be >= leaf C:N\n");
		printf("change the values in EPC file\n");
		errorCode=2091701;
	}

	if (!errorCode && epc->softstem_cn > 0 && epc->softstem_cn < epc->leaf_cn)
	{
		printf("ERROR: softstem C:N must be >= leaf C:N\n");
		printf("change the values in EPC file\n");
		errorCode=2091701;
	}

	
	/* test for livewood C:N setting for woody biomes */
	if (!errorCode && epc->woody && epc->livewood_cn == 0)
	{
		printf("ERROR: livewood C:N must be > 0 in woody simulation \n");
		printf("change the values in EPC file\n");
		errorCode=2091702;
	}

	/* test for deadwood C:N > livewood C:N */
	if (!errorCode && epc->deadwood_cn < epc->livewood_cn)
	{
		printf("ERROR: deadwood C:N must be >= livewood C:N\n");
		printf("change the values in EPC file\n");
		errorCode=2091703;
	}

	/*--------------------------------------------------------------------*/
	/* C:DM ratios */
	if (!errorCode && scan_value(epc_file, &epc->leafC_DM, 'd'))
	{
		printf("ERROR reading dry matter carbon content of leaves, epc_init()\n");
		errorCode=20918;
	}
	if (!errorCode && scan_value(epc_file, &epc->leaflitrC_DM, 'd'))
	{
		printf("ERROR reading dry matter carbon content of leaf litter, epc_init()\n");
		errorCode=20918;
	}
	if (!errorCode && scan_value(epc_file, &epc->frootC_DM, 'd'))
	{
		printf("ERROR reading dry matter carbon content of fine root, epc_init()\n");
		errorCode=20918;
	}
	if (!errorCode && scan_value(epc_file, &epc->fruitC_DM, 'd'))
	{
		printf("ERROR reading dry matter carbon content of fruit, epc_init()\n");
		errorCode=20918;
	}
	if (!errorCode && scan_value(epc_file, &epc->softstemC_DM, 'd'))
	{
		printf("ERROR reading dry matter carbon content of softstem, epc_init()\n");
		errorCode=20918;
	}
	if (!errorCode && scan_value(epc_file, &epc->livewoodC_DM, 'd'))
	{
		printf("ERROR reading dry matter carbon content of live wood, epc_init()\n");
		errorCode=20918;
	}
	if (!errorCode && scan_value(epc_file, &epc->deadwoodC_DM, 'd'))
	{
		printf("ERROR reading dry matter carbon content of dead wood, epc_init()\n");
		errorCode=20918;
	}
	/*--------------------------------------------------------------------*/

	/* LEAF LITTER PROPORTION */
	if (!errorCode && scan_value(epc_file, &t1, 'd'))
	{
		printf("ERROR reading leaf litter labile proportion, epc_init()\n");
		errorCode=20919;
	}
	if (!errorCode) epc->leaflitr_flab = t1;

	if (!errorCode && scan_value(epc_file, &t2, 'd'))
	{
		printf("ERROR reading leaf litter cellulose proportion, epc_init()\n");
		errorCode=20919;
	}

	/* livecroot lignin proportion is calculated from labile and cellulose */
	t3 = 1 - t2 - t1 ;

    /* test for fractions sum to 1.0 */
	if (!errorCode && ((t1+t2) > 1))
	{
		printf("ERROR in fractions: %.2f %.2f %.2f %.3f\n",t1, t2, t3, t1+t2+t3);
		printf("leaf litter proportions of labile and cellulose \n");
		printf("must less than 1.0. Check initialization file and try again.\n");
		errorCode=2091901;
	}

	if (!errorCode) epc->leaflitr_flig = t3;

	/* calculate shielded and unshielded cellulose fraction */
	if (!errorCode)
	{
		r1 = t3/t2;
		if (r1 <= 0.45)
		{
			epc->leaflitr_fscel = 0.0;
			epc->leaflitr_fucel = t2;
		}
		else if (r1 > 0.45 && r1 < 0.7)
		{
			t4 = (r1 - 0.45)*3.2;
			epc->leaflitr_fscel = t4*t2;
			epc->leaflitr_fucel = (1.0 - t4)*t2;
		}
		else
		{
			epc->leaflitr_fscel = 0.8*t2;
			epc->leaflitr_fucel = 0.2*t2;
		}
	}

	/* FROOT LITTER PROPORTION */
	if (!errorCode && scan_value(epc_file, &t1, 'd'))
	{
		printf("ERROR reading froot litter labile proportion, epc_init()\n");
		errorCode=20921;
	}
	epc->frootlitr_flab = t1;
	if (!errorCode && scan_value(epc_file, &t2, 'd'))
	{
		printf("ERROR reading froot litter cellulose proportion, epc_init()\n");
		errorCode=20921;
	}
		
	/* livecroot lignin proportion is calculated from labile and cellulose */
	t3 = 1 - t2 - t1;
	
     
	/* test for fractions sum to 1.0 */
	if (!errorCode && ((t1+t2) > 1))
	{
		printf("ERROR in fractions: %.2f %.2f %.2f %.3f\n",t1, t2, t3, t1+t2+t3);
		printf("froot proportions of labile and cellulose \n");
		printf("must less than 1.0. Check initialization file and try again.\n");
		errorCode=2092001;
	}

	if (!errorCode) epc->frootlitr_flig = t3;
	

	/* calculate shielded and unshielded cellulose fraction */
	if (!errorCode)
	{
		r1 = t3/t2;
		if (r1 <= 0.45)
		{
			epc->frootlitr_fscel = 0.0;
			epc->frootlitr_fucel = t2;
		}
		else if (r1 > 0.45 && r1 < 0.7)
		{
			t4 = (r1 - 0.45)*3.2;
			epc->frootlitr_fscel = t4*t2;
			epc->frootlitr_fucel = (1.0 - t4)*t2;
		}
		else
		{
			epc->frootlitr_fscel = 0.8*t2;
			epc->frootlitr_fucel = 0.2*t2;
		}
	}


	/* FRUIT LITTER PROPORTION */
	if (!errorCode && scan_value(epc_file, &t1, 'd'))
	{
		printf("ERROR reading fruit litter labile proportion, epc_init()\n");
		errorCode=20921;
	}
	epc->fruitlitr_flab = t1;
	if (!errorCode && scan_value(epc_file, &t2, 'd'))
	{
		printf("ERROR reading fruit litter cellulose proportion, epc_init()\n");
		errorCode=20921;
	}
			
	/* livecroot lignin proportion is calculated from labile and cellulose */
	t3 = 1 - t2 - t1;

     
	/* test for fractions sum to 1.0 */
	if (!errorCode && ((t1+t2) > 1))
	{
		printf("ERROR in fractions: %.2f %.2f %.2f %.3f\n",t1, t2, t3, t1+t2+t3);
		printf("froot proportions of labile and cellulose \n");
		printf("must less than 1.0. Check initialization file and try again.\n");
		errorCode=2092101;
	}


	if (!errorCode) epc->fruitlitr_flig = t3;

	/* calculate shielded and unshielded cellulose fraction */
	if (!errorCode)
	{
		r1 = t3/t2;
		if (r1 <= 0.45)
		{
			epc->fruitlitr_fscel = 0.0;
			epc->fruitlitr_fucel = t2;
		}
		else if (r1 > 0.45 && r1 < 0.7)
		{
			t4 = (r1 - 0.45)*3.2;
			epc->fruitlitr_fscel = t4*t2;
			epc->fruitlitr_fucel = (1.0 - t4)*t2;
		}
		else
		{
			epc->fruitlitr_fscel = 0.8*t2;
			epc->fruitlitr_fucel = 0.2*t2;
		}
	}

	/* SOFT STEM LITTER PROPORTION */
	if (!errorCode && scan_value(epc_file, &t1, 'd'))
	{
		printf("ERROR reading soft stem litter labile proportion, epc_init()\n");
		errorCode=20922;
	}
	epc->softstemlitr_flab = t1;
	if (!errorCode && scan_value(epc_file, &t2, 'd'))
	{
		printf("ERROR reading soft stem  litter cellulose proportion, epc_init()\n");
		errorCode=20922;
	}
	
	/* livecroot lignin proportion is calculated from labile and cellulose */
	t3 = 1 - t2 - t1;

	 /* test for fractions sum to 1.0 */
	if (!errorCode && ((t1+t2) > 1))
	{
		printf("ERROR in fractions: %.2f %.2f %.2f %.3f\n",t1, t2, t3, t1+t2+t3);
		printf("softstem proportions of labile and cellulose \n");
		printf("must less than 1.0. Check initialization file and try again.\n");
		errorCode=2092201;
	}


	if (!errorCode) epc->softstemlitr_flig = t3;

	/* calculate shielded and unshielded cellulose fraction */
	if (!errorCode)
	{
		r1 = t3/t2;
		if (r1 <= 0.45)
		{
			epc->softstemlitr_fscel = 0.0;
			epc->softstemlitr_fucel = t2;
		}
		else if (r1 > 0.45 && r1 < 0.7)
		{
			t4 = (r1 - 0.45)*3.2;
			epc->softstemlitr_fscel = t4*t2;
			epc->softstemlitr_fucel = (1.0 - t4)*t2;
		}
		else
		{
			epc->softstemlitr_fscel = 0.8*t2;
			epc->softstemlitr_fucel = 0.2*t2;
		}
	}

	/* DEAD WOOD LITTER PROPORTION */
	if (!errorCode && scan_value(epc_file, &t1, 'd'))
	{
		printf("ERROR reading dead wood %% cellulose, epc_init()\n");
		errorCode=20923;
	}

	
     /* lignin proportion is calculated from cellulose */
	t2 = 1 - t1;

      /* test for fractions sum to 1.0 */
	if (!errorCode && t1 > 1)
	{
		printf("ERROR in fractions: %.2f %.2f %.3f\n",t1, t2, t1+t2);
		printf("dead wood cellulose, and lignin\n");
		printf("must less than 1.0. Check initialization file and try again.\n");
		errorCode=2092301;
	}


	if (!errorCode) epc->deadwood_flig = t2;

	if (!errorCode && epc->woody && epc->deadwood_flig == 0)
	{
		printf("ERROR: dead wood cellulose proportion must be > 0 in woody simulation \n");
		printf("change the values in EPC file\n");
		errorCode=2092302;
	}
	


	/* calculate shielded and unshielded cellulose fraction */
	if (!errorCode)
	{
		r1 = t2/t1;
		if (r1 <= 0.45)
		{
			epc->deadwood_fscel = 0.0;
			epc->deadwood_fucel = t1;
		}
		else if (r1 > 0.45 && r1 < 0.7)
		{
			t4 = (r1 - 0.45)*3.2;
			epc->deadwood_fscel = t4*t1;
			epc->deadwood_fucel = (1.0 - t4)*t1;
		}
		else
		{
			epc->deadwood_fscel = 0.8*t1;
			epc->deadwood_fucel = 0.2*t1;
		}
	}

	/* ---------------------------------------------- */
	/* radiation and LAI-parameters */

	if (!errorCode && scan_value(epc_file, &epc->int_coef, 'd'))
	{
		printf("ERROR reading canopy water int coef, epc_init()\n");
		errorCode=20924;
	}
	if (!errorCode && scan_value(epc_file, &epc->ext_coef, 'd'))
	{
		printf("ERROR reading canopy light ext.coef, epc_init()\n");
		errorCode=20925;
	}

	if (!errorCode && scan_value(epc_file, &epc->potRUE, 'd'))
	{
		printf("ERROR reading potential radiation use efficiency, epc_init()\n");
		errorCode=20926;
	}

	if (!errorCode && scan_value(epc_file, &epc->rad_param1, 'd'))
	{
		printf("ERROR reading radiation parameter1, epc_init()\n");
		errorCode=20927;
	}

	if (!errorCode && scan_value(epc_file, &epc->rad_param2, 'd'))
	{
		printf("ERROR reading radiation parameter2, epc_init()\n");
		errorCode=20927;
	}


	if (!errorCode && scan_value(epc_file, &epc->lai_ratio, 'd'))
	{
		printf("ERROR reading all to projected LAI ratio, epc_init()\n");
		errorCode=20928;
	}

	if (!errorCode && scan_value(epc_file, &epc->sla_ratio, 'd'))
	{
		printf("ERROR reading shaded to sunlit SLA ratio, epc_init()\n");
		errorCode=20929;
	}

	if (!errorCode && scan_value(epc_file, &epc->flnr, 'd'))
	{
		printf("ERROR reading Rubisco N fraction, epc_init()\n");
		errorCode=20930;
	}
	if (!errorCode && scan_value(epc_file, &epc->flnp, 'd'))
	{
		printf("ERROR reading PeP N fraction, epc_init()\n");
		errorCode=20931;
	}
	

	/* ------------------------------------------------------ */
	/* maximum stomatal conductance (optional annual varying value) */

	if (!errorCode && scan_value(epc_file, &epc->gl_sMAX, 'd'))
	{
		printf("ERROR reading gl_sMAX, epc_init()\n");
		errorCode=20932;
	}

	if (ctrl->spinup == 0)
		strcpy(MSC_file.name, "conductance_normal.txt");
	else
	{
		if (ctrl->spinup == 1) 
			strcpy(MSC_file.name, "conductance_spinup.txt");
		else
			strcpy(MSC_file.name, "conductance_transient.txt");
	}
	
	/* MSC flag: constans or varying MSC from file */
	if (!errorCode && !file_open(&MSC_file,'j',1)) 
		ctrl->varMSC_flag = 1;
	else
		ctrl->varMSC_flag = 0;

	if (!errorCode && ctrl->varMSC_flag) 
	{
		/* allocate space for the annual MSC array */
		epc->MSC_array = (double*) malloc(ctrl->simyears * sizeof(double));
		if (!epc->MSC_array)
		{
			printf("ERROR allocating for annual MSC array, epc_init()\n");
			errorCode=20932;
		}
	
		/* initialization with default value */
		for (ndata = 0; ndata < ctrl->simyears; ndata++) epc->MSC_array[ndata] = epc->gl_sMAX;
		
		ndata=0;
		while (!errorCode && !(dataread = scan_array (MSC_file, &p1, 'i', 0, 0)))
		{
			dataread = fscanf(MSC_file.ptr, "%lf%*[^\n]", &p2);
				
			if (p1 >= ctrl->simstartyear && p1 < ctrl->simstartyear + ctrl->simyears)
			{
				OPTINyear_array[ndata] = p1;
				epc->MSC_array[ndata]  = p2;

				if (epc->MSC_array[ndata] < 0.0 && epc->MSC_array[ndata] != DATA_GAP)
				{
					printf("ERROR in epc_init(): MSC must be positive\n");
					errorCode=20932;
				}
			
				ndata += 1;
			}
		}
		/* read year and MSC for each simday in each simyear */
		
		fclose(MSC_file.ptr);
	}	
	else /* if no changing data constant EPC parameter are used */
	{
		epc->MSC_array = 0;
	}	
	

	
	/* ------------------------------------------------------ */
	if (!errorCode && scan_value(epc_file, &epc->gl_c, 'd'))
	{
		printf("ERROR reading gl_c, epc_init()\n");
		errorCode=20933;
	}
	if (!errorCode && scan_value(epc_file, &epc->gl_bl, 'd'))
	{
		printf("ERROR reading gl_bl, epc_init()\n");
		errorCode=20934;
	}

	/* max_plant_height */
	if (!errorCode && scan_value(epc_file, &epc->max_plant_height, 'd'))
	{
		printf("ERROR reading number of max_plant_height\n");
		errorCode=20935;
	}

	/* plantheight parameter 1 (weight of stem at maximum height */  
	if (!errorCode && scan_value(epc_file, &epc->plantheight_par1, 'd'))
	{
		printf("ERROR reading plantheight_par1\n");
		errorCode=20936;
	}

	/* plantheight parameter 2 (slope) */  
	if (!errorCode && scan_value(epc_file, &epc->plantheight_par2, 'd'))
	{
		printf("ERROR reading plantheight_par2\n");
		errorCode=20936;
	}

	/* max_rootzone_depth  */
	if (!errorCode && scan_value(epc_file, &epc->max_rootzone_depth, 'd'))
	{
		printf("ERROR reading number of max_rootzone_depth\n");
		errorCode=20937;
	}

	/* CONTROL to avoid negative data */
 	if (!errorCode && epc->max_rootzone_depth <= 0)
	{
		printf("ERROR in epc data in EPC file: negative or zero rootzone_depth, epc_init()\n");
		errorCode=2093701;
	}

	if (!errorCode && scan_value(epc_file, &epc->rootdistrib_param, 'd'))
	{
		printf("ERROR reading rootdistrib_param: epc_init()\n");
		errorCode=20938;
	}
	
	if (!errorCode && scan_value(epc_file, &epc->rootlength_par1, 'd'))
	{
		printf("ERROR reading rootlength_par1: epc_init()\n");
		errorCode=20939;
	}
	
	if (!errorCode && scan_value(epc_file, &epc->rootlength_par2, 'd'))
	{
		printf("ERROR reading rootlength_par2: epc_init()\n");
		errorCode=20939;
	}

	if (!errorCode && scan_value(epc_file, &epc->rootweight2length, 'd'))
	{
		printf("ERROR reading rootweight2length: epc_init()\n");
		errorCode=20940;
	}
	
	if (!errorCode && scan_value(epc_file, &epc->GR_ratio, 'd'))
	{
		printf("ERROR reading growth resp.ratio: epc_init()\n");
		errorCode=20941;
	}


	if (!errorCode && scan_value(epc_file, &epc->mrpern, 'd'))
	{
		printf("ERROR reading mrpern: epc_init()\n");
		errorCode=20942;
	}

	if (!errorCode && scan_value(epc_file, &epc->NSC_SC_prop, 'd'))
	{
		printf("ERROR reading NSC_SC_prop: epc_init()\n");
		errorCode=20943;
	}

	if (!errorCode && scan_value(epc_file, &epc->NSC_avail_prop, 'd'))
	{
		printf("ERROR reading NSC_avail_prop: epc_init()\n");
		errorCode=20944;
	}

	if (!errorCode && scan_value(epc_file, &epc->nfix, 'd'))
	{
		printf("ERROR reading N fixation, epc_init()\n");
		errorCode=20945;
	}

	if (!errorCode && scan_value(epc_file, &epc->tau, 'd'))
	{
		printf("ERROR reading tau (time delay photosynthesis acclim.), epc_init()\n");
		errorCode=20946;
	}

	/* CONTROL to avoid irrealistic temperature data */
 	if (!errorCode && epc->tau < 1 && epc->tau > 0)
	{
		if (epc->tau > 0.5) 
			epc->tau = 1;
		else
			epc->tau = 0;
	}
	
	/*********************************************************************************************************************/
	/* dividing line from file */ 
	if (!errorCode && scan_value(epc_file, header, 's'))
	{
		printf("ERROR reading 3. dividing line, epc_init()\n");
		errorCode=20947;
	}
	if (!errorCode && scan_value(epc_file, header, 's'))
	{
		printf("ERROR reading header, epc_init()\n");
		errorCode=20947;
	}
	/*********************************************************************************************************************/
	/* CROP SPECIFIC PARAMETERS */
	/*********************************************************************************************************************/

	/* number of phenophase of germination */
	if (!errorCode && scan_value(epc_file, &epc->n_germ_phenophase, 'i'))
	{
		printf("ERROR reading number of phenophase of germination \n");
		errorCode=20948;
	}

	/* number of phenophase of emergence */
	if (!errorCode && scan_value(epc_file, &epc->n_emerg_phenophase, 'i'))
	{
		printf("ERROR reading number of phenophase of emergence\n");
		errorCode=20949;
	}

	
	/* critical relative VWC (prop. to FC) in germination */
	if (!errorCode && scan_value(epc_file, &epc->grmn_paramVWC, 'd'))
	{
		printf("ERROR reading critical VWC ratio in germination, epc_init()\n");
		errorCode=20950;
	}
	
	/* number of phenophase of photoperiodic slowing effect */
	if (!errorCode && scan_value(epc_file, &epc->n_phpsl_phenophase, 'i'))
	{
		printf("ERROR reading number of phenophase of photoperiodic slowing effect \n");
		errorCode=20951;
	}

	/* critical photoslow daylength */
	if (!errorCode && scan_value(epc_file, &epc->phpsl_parDL, 'd'))
	{
		printf("ERROR reading critical photoslow daylength, epc_init()\n");
		errorCode=20952;
	}

	
	/* slope of relative photoslow development rate */
	if (!errorCode && scan_value(epc_file, &epc->phpsl_parDR, 'd'))
	{
		printf("ERROR reading slope of relative photoslow development rate, epc_init()\n");
		errorCode=20952;
	}

	/* number of phenophase of vernalization */
	if (!errorCode && scan_value(epc_file, &epc->n_vern_phenophase, 'i'))
	{
		printf("ERROR reading number of vernalization phenophase\n");
		errorCode=20953;
	}

	/* critical vernalization temperature data */
	if (!errorCode && scan_value(epc_file, &epc->vern_parT1, 'd'))
	{
		printf("ERROR reading critical vernalization temperature 1, epc_init()\n");
		errorCode=20954;
	}
	
	if (!errorCode && scan_value(epc_file, &epc->vern_parT2, 'd'))
	{
		printf("ERROR reading critical vernalization temperature 2, epc_init()\n");
		errorCode=20954;
	}
	
	if (!errorCode && scan_value(epc_file, &epc->vern_parT3, 'd'))
	{
		printf("ERROR reading critical vernalization temperature 3, epc_init()\n");
		errorCode=20954;
	}
	
	if (!errorCode && scan_value(epc_file, &epc->vern_parT4, 'd'))
	{
		printf("ERROR reading critical vernalization temperature 4, epc_init()\n");
		errorCode=20954;
	}
	
	/* control of vernalization temperature parameters */
	if (!errorCode)
	{
		if (epc->vern_parT1 > epc->vern_parT2 ||epc->vern_parT2 > epc->vern_parT3 || epc->vern_parT3 > epc->vern_parT4) 
		{
				printf("ERROR in vernalization temperature data in EPC file\n");
				printf("Correct temperature data: vern_parT1 <= vern_parT2 <= vern_parT3 <= vern_parT4\n");
				errorCode=2095401;
		}
	}

	/* slope of relative vernalization development rate  */
	if (!errorCode && scan_value(epc_file, &epc->vern_parDR1, 'd'))
	{
		printf("ERROR reading slope of relative vernalization development rate, epc_init()\n");
		errorCode=20955;
	}

	/* required vernalization days (in vernalization development rate  */
	if (!errorCode && scan_value(epc_file, &epc->vern_parDR2, 'd'))
	{
		printf("ERROR reading required vernalization days (in vernalization development rate, epc_init()\n");
		errorCode=20955;
	}
	
	/* number of phenophase of flowering heat stress */
	if (!errorCode && scan_value(epc_file, &epc->n_flowHS_phenophase, 'i'))
	{
		printf("ERROR reading number of phenophase of flowering heat stress\n");
		errorCode=20956;
	}

	/* critical flowering heat stress temperatures */
	if (!errorCode && scan_value(epc_file, &epc->flowHS_parT1, 'd'))
	{
		printf("ERROR reading critical flowering heat stress temperature 1, epc_init()\n");
		errorCode=20957;
	}

	if (!errorCode && scan_value(epc_file, &epc->flowHS_parT2, 'd'))
	{
		printf("ERROR reading critical flowering heat stress temperature 2, epc_init()\n");
		errorCode=20957;
	}

	/* control of flowering heat stress temperature parameter */
	if (epc->flowHS_parT1 > epc->flowHS_parT2) 
	{
			printf("ERROR in flowering heat stress temperature data in EPC file\n");
			printf("Correct temperature data: flowHS_parT1 <= flowHS_parT2 \n");
			errorCode=2095701;
	}

	/* mortality parameter of flowering heat stress */
	if (!errorCode && scan_value(epc_file, &epc->flowHS_parMORT, 'd'))
	{
		printf("ERROR reading mortality parameter of flowering heat stress, epc_init()\n");
		errorCode=20958;
	}

	/*********************************************************************************************************************/
	/* dividing line from file */ 
	if (!errorCode && scan_value(epc_file, header, 's'))
	{
		printf("ERROR reading 4. dividing line, epc_init()\n");
		errorCode=20959;
	}
	if (!errorCode && scan_value(epc_file, header, 's'))
	{
		printf("ERROR reading header, epc_init()\n");
		errorCode=20959;
	}
	/*********************************************************************************************************************/
	/* STRESS AND SENESCENCE PARAMETERS */
	/*********************************************************************************************************************/

	if (!errorCode && scan_value(epc_file, &epc->VWCratio_crit1, 'd'))
	{
		printf("ERROR reading VWCratio_crit1, epc_init()\n");
		errorCode=20960;
	}
	
   if (!errorCode && scan_value(epc_file, &epc->VWCratio_crit2, 'd'))
	{
		printf("ERROR reading VWCratio_crit2, epc_init()\n");
		errorCode=20961;
	}
	
	 if (!errorCode && scan_value(epc_file, &epc->m_fullstress2, 'd'))
	{
		printf("ERROR reading m_fullstress2, epc_init()\n");
		errorCode=20962;
	}

	if (!errorCode && scan_value(epc_file, &epc->vpd_open, 'd')) 
	{
		printf("ERROR reading vpd_max, epc_init()\n");
		errorCode=20963;
	}
	if (!errorCode && scan_value(epc_file, &epc->vpd_close, 'd'))
	{
		printf("ERROR reading vpd_min, epc_init()\n");
		errorCode=20964;
	}

	if (!errorCode && scan_value(epc_file, &epc->SNSCmort_abovebiom_max, 'd'))
	{
		printf("ERROR reading senescence mortality parameter of aboveground biomass: epc_init()\n");
		errorCode=20965;
	}

	if (!errorCode && scan_value(epc_file, &epc->SNSCmort_belowbiom_max, 'd'))
	{
		printf("ERROR reading senescence mortality parameter of belowground biomass: epc_init()\n");
		errorCode=20965;
	}

	if (!errorCode && scan_value(epc_file, &epc->SNSCmort_nsc_max, 'd'))
	{
		printf("ERROR reading senescence mortality parameter of non-stuctured biomass: epc_init()\n");
		errorCode=20965;
	}

	if (!errorCode && scan_value(epc_file, &epc->SNSC_extremT1, 'd'))
	{
		printf("ERROR reading lower limit extreme high temperature effect on senesncene mortality: epc_init()\n");
		errorCode=20966;
	}

	if (!errorCode && scan_value(epc_file, &epc->SNSC_extremT2, 'd'))
	{
		printf("ERROR reading lower limit extreme high temperature effect on senesncene mortality: epc_init()\n");
		errorCode=20966;
	}

	/* control of flowering heat stress temperature parameter */
	if (!errorCode && epc->SNSC_extremT1 >= epc->SNSC_extremT2) 
	{
		printf("ERROR in extreme high temperature effect data in EPC file\n");
		printf("Correct temperature data: SNSC_extremT1 < SNSC_extremT2 \n");
		errorCode=2096601;
	}

	if (!errorCode && scan_value(epc_file, &epc->mort_SNSC_to_litter, 'd'))
	{
		printf("ERROR reading turnover rate of wilted standing biomass to litter parameter: epc_init()\n");
		errorCode=20967;
	}

	if (!errorCode && scan_value(epc_file, &epc->mort_CnW_to_litter, 'd'))
	{
		printf("ERROR reading turnover rate of cut-down non-woody biomass to litter parameter: epc_init()\n");
		errorCode=20967;
	}

	if (!errorCode && scan_value(epc_file, &epc->mort_CW_to_litter, 'd'))
	{
		printf("ERROR reading turnover rate of cut-down woody biomass to litter parameter: epc_init()\n");
		errorCode=20967;
	}


	if (!errorCode && scan_value(epc_file, &epc->SWCstressLENGTH_crit, 'd'))
	{
		printf("ERROR reading SWCstressLENGTH_crit: epc_init()\n");
		errorCode=20968;
	}

	if (!errorCode && scan_value(epc_file, &epc->photoSTRESSeffect, 'd'))
	{
		printf("ERROR reading photoSTRESSeffect: epc_init()\n");
		errorCode=20968;
	}


	/*********************************************************************************************************************/
	/* dividing line from file */ 
	if (!errorCode && scan_value(epc_file, header, 's'))
	{
		printf("ERROR reading 5. dividing line, epc_init()\n");
		errorCode=20970;
	}
	if (!errorCode && scan_value(epc_file, header, 's'))
	{
		printf("ERROR reading 5. dividing line, epc_init()\n");
		errorCode=20970;
	}
	/*********************************************************************************************************************/
	/* GROWING SEASON PARAMETERS */
	/*********************************************************************************************************************/
	
	/* snowcover_limit for calculation heatsum */
	if (!errorCode && scan_value(epc_file, &epc->snowcover_limit, 'd'))
	{
		printf("ERROR reading snowcover_limit parameter: epc_init()\n");
		errorCode=20971;
	}
	
	/* heatsum_limit for calculation heatsum index */
	if (!errorCode && scan_value(epc_file, &epc->heatsum_limit1, 'd'))
	{
		printf("ERROR reading heatsum_limit1 parameter: epc_init()\n");
		errorCode=20972;
	}

	if (!errorCode && scan_value(epc_file, &epc->heatsum_limit2, 'd'))
	{
		printf("ERROR reading heatsum_limit2 parameter: epc_init()\n");
		errorCode=20972;
	}

	if (!errorCode && epc->heatsum_limit1 > epc->heatsum_limit2) 
	{
		printf("ERROR in heatsum_limit data in EPC file\n");
		printf("Correct data: limit1 < limit2 \n");
		errorCode=2097201;
	}

	/* tmin_limit for calculation tmin index */
	if (!errorCode && scan_value(epc_file, &epc->tmin_limit1, 'd'))
	{
		printf("ERROR reading tmin_limit1 parameter: epc_init()\n");
		errorCode=20973;
	}

	if (!errorCode && scan_value(epc_file, &epc->tmin_limit2, 'd'))
	{
		printf("ERROR reading tmin_limit2 parameter: epc_init()\n");
		errorCode=20973;
	}

	if (!errorCode && epc->tmin_limit1 > epc->tmin_limit2) 
	{
		printf("ERROR in tmin_limit data in EPC file\n");
		printf("Correct data: limit1 < limit2 \n");
		errorCode=2097301;
	}

	/* vpd_limit for calculation vpd index */
	if (!errorCode && scan_value(epc_file, &epc->vpd_limit1, 'd'))
	{
		printf("ERROR reading vpd_limit1 parameter: epc_init()\n");
		errorCode=20974;
	}

	
	if (!errorCode && scan_value(epc_file, &epc->vpd_limit2, 'd'))
	{
		printf("ERROR reading vpd_limit2 parameter: epc_init()\n");
		errorCode=20974;
	}
	
	if (!errorCode && epc->vpd_limit1 < epc->vpd_limit2) 
	{
		printf("ERROR in vpd_limit data in EPC file\n");
		printf("Correct data: limit1 > limit2 \n");
		errorCode=2097401;
	}

	/* dayl_limit for calculation dayl index */
	if (!errorCode && scan_value(epc_file, &epc->dayl_limit1, 'd'))
	{
		printf("ERROR reading dayl_limit1 parameter: epc_init()\n");
		errorCode=20975;
	}

	if (!errorCode && scan_value(epc_file, &epc->dayl_limit2, 'd'))
	{
		printf("ERROR reading dayl_limit2 parameter: epc_init()\n");
		errorCode=20975;
	}

	if (!errorCode && epc->dayl_limit1 > epc->dayl_limit2) 
	{
		printf("ERROR in vpd_limit data in EPC file\n");
		printf("Correct data: limit1 > limit2 \n");
		errorCode=2097501;
	}

	/* n_moving_avg for calculation moving average from indexes */
	if (!errorCode && scan_value(epc_file, &epc->n_moving_avg, 'i'))
	{
		printf("ERROR reading n_moving_avg parameter: epc_init()\n");
		errorCode=20976;
	}
	if (!errorCode && (epc->n_moving_avg < 2))
	{
		printf("ERROR in moving average parameter of epc calculation:\n");
		printf("must greater then 2. Check EPC file and try again.\n");
		errorCode=2097601;
	}

	/* epc_limit for calculation yday of start growing season  */
	if (!errorCode && scan_value(epc_file, &epc->GSI_limit_SGS, 'd'))
	{
		printf("ERROR reading epc_limit_SGS parameter: epc_init()\n");
		errorCode=20977;
	}

	/* epc_limit for calculation yday of end growing season  */
	if (!errorCode && scan_value(epc_file, &epc->GSI_limit_EGS, 'd'))
	{
		printf("ERROR reading epc_limit_EGS parameter: epc_init()\n");
		errorCode=20978;
	}
	
	/*********************************************************************************************************************/
	/* dividing line from file */ 
	if (!errorCode && scan_value(epc_file, header, 's'))
	{
		printf("ERROR reading 6. dividing line from, epc_init()\n");
		errorCode=20979;
	}
	if (!errorCode && scan_value(epc_file, header, 's'))
	{
		printf("ERROR reading header, epc_init()\n");
		errorCode=20979;
	}
	/*********************************************************************************************************************/
	/* PHENOLOGICAL (ALLOCATION) PARAMETERS (7 phenological phases) */
    /*********************************************************************************************************************/

	if (!errorCode && scan_value(epc_file, header, 's'))
	{
		printf("ERROR reading name of the phenophases\n");
		errorCode=20980;
	}

	scanflag=0; 
	for (phenphase=0; phenphase<N_PHENPHASES; phenphase++)
	{
		if (phenphase==N_PHENPHASES-1) scanflag=1;
		if (!errorCode && scan_array(epc_file, &(epc->phenophase_length[phenphase]), 'd', scanflag, 1))
		{
			printf("ERROR reading phenophase_length in phenophase %i, epc_init()\n", phenphase+1);
			errorCode=20981;
		}
	}

	scanflag=0; 
	for (phenphase=0; phenphase<N_PHENPHASES; phenphase++)
	{
		if (phenphase==N_PHENPHASES-1) scanflag=1;
		if (!errorCode && scan_array(epc_file, &(epc->alloc_leafc[phenphase]), 'd', scanflag, 1))
		{
			printf("ERROR reading alloc_leafc in phenophase %i, epc_init()\n", phenphase+1);
			errorCode=20982;
		}
	}

	scanflag=0; 
	for (phenphase=0; phenphase<N_PHENPHASES; phenphase++)
	{
		if (phenphase==N_PHENPHASES-1) scanflag=1;
		if (!errorCode && scan_array(epc_file, &(epc->alloc_frootc[phenphase]), 'd', scanflag, 1))
		{
			printf("ERROR reading alloc_frootc in phenophase %i, epc_init()\n", phenphase+1);
			errorCode=20983;
		}
	}

	scanflag=0; 
	for (phenphase=0; phenphase<N_PHENPHASES; phenphase++)
	{
		if (phenphase==N_PHENPHASES-1) scanflag=1;
		if (!errorCode && scan_array(epc_file, &(epc->alloc_fruitc[phenphase]), 'd', scanflag, 1))
		{
			printf("ERROR reading alloc_fruitc in phenophase %i, epc_init()\n", phenphase+1);
			errorCode=20984;
		}
	}

	scanflag=0; 
	for (phenphase=0; phenphase<N_PHENPHASES; phenphase++)
	{
		if (phenphase==N_PHENPHASES-1) scanflag=1;
		if (!errorCode && scan_array(epc_file, &(epc->alloc_softstemc[phenphase]), 'd', scanflag, 1))
		{
			printf("ERROR reading alloc_softstemc in phenophase %i, epc_init()\n", phenphase+1);
			errorCode=20985;
		}
	}

	scanflag=0; 
	for (phenphase=0; phenphase<N_PHENPHASES; phenphase++)
	{
		if (phenphase==N_PHENPHASES-1) scanflag=1;
		if (!errorCode && scan_array(epc_file, &(epc->alloc_livestemc[phenphase]), 'd', scanflag, 1))
		{
			printf("ERROR reading alloc_woodystemc_live in phenophase %i, epc_init()\n", phenphase+1);
			errorCode=20986;
		}
	}

	scanflag=0; 
	for (phenphase=0; phenphase<N_PHENPHASES; phenphase++)
	{
		if (phenphase==N_PHENPHASES-1) scanflag=1;
		if (!errorCode && scan_array(epc_file, &(epc->alloc_deadstemc[phenphase]), 'd', scanflag, 1))
		{
			printf("ERROR reading alloc_woodystemc_dead in phenophase %i, epc_init()\n", phenphase+1);
			errorCode=20987;
		}
	}

	scanflag=0; 
	for (phenphase=0; phenphase<N_PHENPHASES; phenphase++)
	{
		if (phenphase==N_PHENPHASES-1) scanflag=1;
		if (!errorCode && scan_array(epc_file, &(epc->alloc_livecrootc[phenphase]), 'd', scanflag, 1))
		{
			printf("ERROR reading alloc_crootc_live in phenophase %i, epc_init()\n", phenphase+1);
			errorCode=20988;
		}
	}

	scanflag=0; 
	for (phenphase=0; phenphase<N_PHENPHASES; phenphase++)
	{
		if (phenphase==N_PHENPHASES-1) scanflag=1;
		if (!errorCode && scan_array(epc_file, &(epc->alloc_deadcrootc[phenphase]), 'd', scanflag, 1))
		{
			printf("ERROR reading alloc_crootc_dead in phenophase %i, epc_init()\n", phenphase+1);
			errorCode=20989;
		}
	}

	/* control of allocation parameter: test for alloc.fractions sum to 1.0  in every phenophases */

	for (phenphase=0; phenphase<N_PHENPHASES; phenphase++)
	{
		sum = epc->alloc_leafc[phenphase] + epc->alloc_frootc[phenphase] + epc->alloc_fruitc[phenphase] + epc->alloc_softstemc[phenphase] +
			  epc->alloc_livestemc[phenphase]   + epc->alloc_deadstemc[phenphase] + 
			  epc->alloc_livecrootc[phenphase]  + epc->alloc_deadcrootc[phenphase];

		if (!errorCode && sum != 0 && fabs(sum - 1.) > CRIT_PREC)
		{
			printf("ERROR in allocation parameters in phenophase %i, epc_init()\n", phenphase+1);
			printf("Allocation parameters must sum to 1.0 in every phenophase. Check EPC file and try again.\n");
			errorCode=2098901;
		
		}

		/* control: after germination allocation parameter setting is necessary */
		if (!errorCode && (phenphase == 0)
			   && (epc->alloc_leafc[phenphase] == 0 || epc->alloc_frootc[phenphase] == 0))
		{
			printf("ERROR in allocation parameters in phenophase %i, epc_init()\n", phenphase+1);
			printf("In the first phenophase leaf and froot allocation parameter setting is necessary. Check EPC file and try again.\n");
			errorCode=2098802;
		}

		/* control: woody biomes - no softstem */
		if (!errorCode && epc->woody && epc->alloc_softstemc[phenphase] > 0)
		{
			printf("ERROR in allocation parameters in phenophase %i, epc_init()\n", phenphase+1);
			printf("No softstem allocation in case of woody biomass. Check EPC file and try again.\n");
			errorCode=2098803;
		}

		/* control: non-woody biomes - no woody allocation */
		if (!errorCode && !epc->woody && 
		   (epc->alloc_livecrootc[phenphase] > 0 || epc->alloc_livestemc[phenphase] > 0 || epc->alloc_deadcrootc[phenphase] > 0 || epc->alloc_deadstemc[phenphase] > 0))
		{
			printf("ERROR in allocation parameters in phenophase %i, epc_init()\n", phenphase+1);
			printf("No woody allocation in case of non-woody biomass. Check EPC file and try again.\n");
			errorCode=2098804;
		}

	}


	scanflag=0; 
	for (phenphase=0; phenphase<N_PHENPHASES; phenphase++)
	{
		if (phenphase==N_PHENPHASES-1) scanflag=1;
		if (!errorCode && scan_array(epc_file, &(epc->avg_proj_sla[phenphase]), 'd', scanflag, 1))
		{
			printf("ERROR reading avg_proj_sla in phenophase %i, epc_init()\n", phenphase+1);
			errorCode=20990;
		}

		if (!errorCode && epc->avg_proj_sla[phenphase] < 0)
		{
			printf("ERROR in specific leaf area in phenophase %i, epc_init()\n", phenphase+1);
			printf("Specific leaf area must be positive in every phenophase. Check EPC file and try again.\n");
			errorCode=2099001;
		
		}

		if (!errorCode && epc->avg_proj_sla[phenphase] == 0 && epc->alloc_leafc[phenphase] > 0)
		{
			printf("ERROR in specific leaf area in phenophase %i, epc_init()\n", phenphase+1);
			printf("Specific leaf area must greater than to 0.0 in case of leaf allocation. Check EPC file and try again.\n");
			errorCode=2099002;
		
		}
	}

	scanflag=0; 
	for (phenphase=0; phenphase<N_PHENPHASES; phenphase++)
	{
		if (phenphase==N_PHENPHASES-1) scanflag=1;
		if (!errorCode && scan_array(epc_file, &(epc->curgrowth_prop[phenphase]), 'd', scanflag, 1))
		{
			printf("ERROR reading curgrowth_prop in phenophase %i, epc_init()\n", phenphase+1);
			errorCode=20991;
		}
	}

	scanflag=0; 
	for (phenphase=0; phenphase<N_PHENPHASES; phenphase++)
	{
		if (phenphase==N_PHENPHASES-1) scanflag=1;
		if (!errorCode && scan_array(epc_file, &(epc->max_plantlifetime[phenphase]), 'd', scanflag, 1))
		{
			printf("ERROR reading max_plantlifetime in phenophase %i, epc_init()\n", phenphase+1);
			errorCode=20992;
		}


		if (!errorCode && ((epc->max_plantlifetime[0] == DATA_GAP && epc->max_plantlifetime[phenphase] != DATA_GAP) ||
			(epc->max_plantlifetime[0] != DATA_GAP && epc->max_plantlifetime[phenphase] == DATA_GAP)))
		{
			printf("ERROR in max_plantlifetime data: All or none maximal lifetime of plant tissue variables should be set at same time. Check EPC file and try again. \n");
			errorCode=2099201;
		}
	}

	

	free(OPTINyear_array);		

	/* -------------------------------------------*/
	if (dofilecloseEPC) fclose(epc_file.ptr);

		
	return (errorCode);

}
