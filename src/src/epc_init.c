/* 
epc_init.c
read epc file for pointbgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2018, D. Hidy [dori.hidy@gmail.com]
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



int epc_init(file init, epconst_struct* epc, GSI_struct* GSI, control_struct* ctrl)
{
	int ok = 1;
	int dofileclose = 1;
	double t1 = 0;
	double t2 = 0;
	double t3 = 0;
	double diff = 0;
	double t4,r1;
	double sum;
	int i, phenphase, scanflag;
	file temp, wpm_file, msc_file, sgs_file, egs_file; 	
	char key1[] = "EPC_FILE";
	char key2[] = "ECOPHYS";
	char keyword[160];

	
	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** EPC_FILES                                                       ** 
	**                                                                 **
	********************************************************************/
	
	/* scan for the EPC file keyword, exit if not next */
	if (ok && scan_value(init, keyword, 's'))
	{
		printf("ERROR reading keyword for control data\n");
		ok=0;
	}
	if (ok && strcmp(keyword, key1))
	{
		printf("Expecting keyword --> %s in file %s\n",key1,init.name);
		ok=0;
	}

	/* open file  */
	if (ok && scan_open(init,&temp,'r')) 
	{
		printf("ERROR opening epconst file, epc_init()\n");
		dofileclose = 0;
		ok=0;
	}
	
	/* first scan epc keyword to ensure proper *.init format */
	if (ok && scan_value(temp, keyword, 's'))
	{
		printf("ERROR reading keyword, epc_init()\n");
		ok=0;
	}
	if (ok && strcmp(keyword,key2))
	{
		printf("Expecting keyword --> %s in %s\n",key2,init.name);
		ok=0;
	}
	/****************************************************************************************************************/
	/* dividing line from file */ 
	if (ok && scan_value(temp, keyword, 's'))
	{
		printf("ERROR reading keyword, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, keyword, 's'))
	{
		printf("ERROR reading keyword, epc_init()\n");
		ok=0;
	}
	/****************************************************************************************************************/
	/* FLAGS */
	/****************************************************************************************************************/
	if (ok && scan_value(temp, &epc->woody, 'i'))
	{
		printf("ERROR reading woody/non-woody flag, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->evergreen, 'i'))
	{
		printf("ERROR reading evergreen/deciduous flag, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->c3_flag, 'i'))
	{
		printf("ERROR reading C3/C4 flag, epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->phenology_flag, 'i'))
	{
		printf("ERROR reading phenology flag, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->transferGDD_flag, 'i'))
	{
		printf("ERROR reading transferGDD_flag, epc_init()\n");
		ok=0;
	}
	/* temperature dependent q10 value */
	if (ok && scan_value(temp, &epc->q10depend_flag, 'i'))
	{
		printf("ERROR reading q10depend_flag, epc_init()\n");
		ok=0;
	}
	/* acclimation */
	if (ok && scan_value(temp, &epc->acclimation_flag, 'i'))
	{
		printf("ERROR reading acclimation_flag, epc_init()\n");
		ok=0;
	}
	/* get flag of CO2 conductance reduction */
	if (ok && scan_value(temp, &epc->CO2conduct_flag, 'i'))
	{
		printf("ERROR reading CO2conduct_flag, epc_init()\n");
		ok=0;
	}

	/* get flag of GSI flag */
	if (ok && scan_value(temp, &GSI->GSI_flag, 'i'))
	{
		printf("ERROR reading flag indicating usage of GSI file: epc_init()\n");
		ok=0;
	}
	
	/* soil temperature calculation flag */
	if (ok && scan_value(temp, &epc->STCM_flag, 'i'))
	{
		printf("ERROR reading soil temperature calculation flag: epc_init()\n");
		ok=0;
	}
	
	/* soil water calculation flag */
	if (ok && scan_value(temp, &epc->SHCM_flag, 'i'))
	{
		printf("ERROR reading soil hydrological calculation method flag: epc_init()\n");
		ok=0;
	}

	/*  discretitaion level of SWC calculation simulation */
	if (ok && scan_value(temp, &epc->discretlevel_Richards, 'i'))
	{
		printf("ERROR reading discretitaion level of SWC calculation: epc_init.c\n");
		ok=0;
	}

	/*  photosynthesis calculation method flag */
	if (ok && scan_value(temp, &epc->photosynt_flag, 'i'))
	{
		printf("ERROR reading photosynthesis calculation method flag: epc_init.c\n");
		ok=0;
	}

	/* control: inactive flag */
	if (epc->photosynt_flag) 
	{
		printf("WARNING: Priestly-Taylor photoynthesis method is not active yet in EPC file\n");
		printf("Please check EPC file and try it again\n");
		ok=0;
	}

	/*  evapotranspiration calculation method flag */
	if (ok && scan_value(temp, &epc->evapotransp_flag, 'i'))
	{
		printf("ERROR reading evapotranspiration calculation method flag: epc_init.c\n");
		ok=0;
	}

	/*  radiation calculation method flag */
	if (ok && scan_value(temp, &epc->radiation_flag, 'i'))
	{
		printf("ERROR reading evapotranspiration calculation method flag: epc_init.c\n");
		ok=0;
	}
	/****************************************************************************************************************/
	/* dividing line from file */ 
	
	if (ok && scan_value(temp, keyword, 's'))
	{
		printf("ERROR reading keyword, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, keyword, 's'))
	{
		printf("ERROR reading keyword, epc_init()\n");
		ok=0;
	}

	/****************************************************************************************************************/
	/* PLANT FUNCTIONING PARAMETERS */
	/****************************************************************************************************************/
	
	/* using varying onday values (in transient or in normal run)*/
	if (ok && scan_value(temp, &epc->onday, 'i'))
	{
		printf("ERROR reading onday, epc_init()\n");
		ok=0;
	}


	if (ctrl->spinup == 0) 
		strcpy(sgs_file.name, "onday_normal.txt");
	else
		strcpy(sgs_file.name, "onday_transient.txt");
	
	/* SGS flag: constans or varying SGS from file */
	if (ok && !file_open(&sgs_file,'j') && epc->phenology_flag == 0) 
		ctrl->varSGS_flag = 1;
	else
		ctrl->varSGS_flag = 0;


	if (ok && ctrl->varSGS_flag) 
	{
		/* allocate space for the annual SGS array */
		epc->sgs_array = (double*) malloc(ctrl->simyears * sizeof(double));
		if (!epc->sgs_array)
		{
			printf("ERROR allocating for annual SGS array, epc_init()\n");
			ok=0;
		}

		/* read year and SGS for each simyear */
		for (i=0 ; ok && i<ctrl->simyears ; i++)
		{
			if (fscanf(sgs_file.ptr,"%*i%lf", &(epc->sgs_array[i]))==EOF)
			{
				printf("ERROR reading annual SGS array, epc_init()\n");
				printf("Note: file must contain a pair of values for each\n");
				printf("simyear: year and SGS.\n");
				ok=0;
			}
			if (epc->sgs_array[i] < 0.0)
			{
				printf("ERROR in epc_init(): sgs must be positive\n");
				ok=0;
			}
		}
		fclose(sgs_file.ptr);
	}	
	else /* if no changing data constant EPC parameter are used */
	{
		epc->sgs_array = 0;
	}	


	/* using varying offday values (in transient or in normal run)*/
	if (ok && scan_value(temp, &epc->offday, 'i'))
	{
		printf("ERROR reading offday, epc_init()\n");
		ok=0;
	}

	if (ctrl->spinup == 0) 
		strcpy(egs_file.name, "offday_normal.txt");
	else
		strcpy(egs_file.name, "offday_transient.txt");
	
	/* SGS flag: constans or varying SGS from file */
	if (ok && !file_open(&egs_file,'j') && epc->phenology_flag == 0) 
		ctrl->varEGS_flag = 1;
	else
		ctrl->varEGS_flag = 0;


	if (ok && ctrl->varEGS_flag) 
	{	
		/* allocate space for the annual EGS array */
		epc->egs_array = (double*) malloc(ctrl->simyears * sizeof(double));
		if (!epc->egs_array)
		{
			printf("ERROR allocating for annual EGS array, epc_init()\n");
			ok=0;
		}

		/* read year and EGS for each simyear */
		for (i=0 ; ok && i<ctrl->simyears ; i++)
		{
			if (fscanf(egs_file.ptr,"%*i%lf", &(epc->egs_array[i]))==EOF)
			{
				printf("ERROR reading annual EGS array, epc_init()\n");
				printf("Note: file must contain a pair of values for each\n");
				printf("simyear: year and EGS.\n");
				ok=0;
			}
			if (epc->egs_array[i] < 0.0)
			{
				printf("ERROR in epc_init(): egs must be positive\n");
				ok=0;
			}
		}
		fclose(egs_file.ptr);
	}	
	else /* if no changing data constant EPC parameter are used */
	{
 		epc->egs_array = 0;
	}	

	
	/* transfer growth and litterfall period */
	if (ok && scan_value(temp, &epc->transfer_pdays, 'd'))
	{
		printf("ERROR reading transfer_pdays, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->litfall_pdays, 'd'))
	{
		printf("ERROR reading litfall_pdays, epc_init()\n");
		ok=0;
	}

	/* base temperature for calculation GDD / heatsum  */
	if (ok && scan_value(temp, &epc->base_temp, 'd'))
	{
		printf("ERROR reading base_temp parameter: epc_init()\n");
		ok=0;
	}

	/* minimum/optimal/maximum temperature for growth displayed on current day (-9999: no T-dependence of allocation) */
	if (ok && scan_value(temp, &epc->pnow_minT, 'd'))
	{
		printf("ERROR reading pnow_minT parameter: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->pnow_opt1T, 'd'))
	{
		printf("ERROR reading pnow_opt1T parameter: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->pnow_opt2T, 'd'))
	{
		printf("ERROR reading pnow_opt1T parameter: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->pnow_maxT, 'd'))
	{
		printf("ERROR reading pnow_maxT parameter: epc_init()\n");
		ok=0;
	}

	/* control of temperature for growth data */
	if (epc->pnow_minT == DATA_GAP ||epc->pnow_opt1T == DATA_GAP || epc->pnow_opt2T == DATA_GAP || epc->pnow_maxT == DATA_GAP)
	{
		if (epc->pnow_minT != DATA_GAP ||epc->pnow_opt1T != DATA_GAP || epc->pnow_opt2T != DATA_GAP || epc->pnow_maxT != DATA_GAP) 
		{
			printf("ERROR in minimum/optimal/maximum temperature for growth data in EPC file\n");
			printf("All or none temperature data should to be set by the user\n");
			ok=0;
		}
	}
	if (epc->pnow_minT > epc->pnow_opt1T ||epc->pnow_opt1T > epc->pnow_opt2T || epc->pnow_opt2T > epc->pnow_maxT) 
	{
			printf("ERROR in minimum/optimal/maximum temperature for pnow data in EPC file\n");
			printf("Correct temperature data: minT <= opt1T <= opt2T <= maxT\n");
			ok=0;
	}

	/* minimum/optimal/maximum temperature for C-assimilation displayed on current day (-9999: no limitation) */
	
	if (ok && scan_value(temp, &epc->assim_minT, 'd'))
	{
		printf("ERROR reading assim_minT parameter: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->assim_opt1T, 'd'))
	{
		printf("ERROR reading assim_opt1T parameter: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->assim_opt2T, 'd'))
	{
		printf("ERROR reading assim_opt1T parameter: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->assim_maxT, 'd'))
	{
		printf("ERROR reading assim_maxT parameter: epc_init()\n");
		ok=0;
	}
	
	/* control of temperature data for C-assimilation */
	if (epc->assim_minT == DATA_GAP ||epc->assim_opt1T == DATA_GAP || epc->assim_opt2T == DATA_GAP || epc->assim_maxT == DATA_GAP)
	{
		if (epc->assim_minT != DATA_GAP ||epc->assim_opt1T != DATA_GAP || epc->assim_opt2T != DATA_GAP || epc->assim_maxT != DATA_GAP) 
		{
			printf("ERROR in minimum/optimal/maximum temperature for assim data in EPC file\n");
			printf("All or none temperature data should to be set by the user\n");
			ok=0;
		}
	}
	if (epc->assim_minT > epc->assim_opt1T ||epc->assim_opt1T > epc->assim_opt2T || epc->assim_opt2T > epc->assim_maxT) 
	{
			printf("ERROR in minimum/optimal/maximum temperature for assim data in EPC file\n");
			printf("Correct temperature data: minT <= opt1T <= opt2T <= maxT\n");
			ok=0;
	}
	
	/*  leaf turnover fraction */
	if (ok && scan_value(temp, &epc->nonwoody_turnover, 'd'))
	{
		printf("ERROR reading non-woody biomass turnover, epc_init()\n");
		ok=0;
	}
	/* force leaf turnover fraction to 1.0 if deciduous */
	if (!epc->evergreen)
	{
		epc->nonwoody_turnover = 1.0;
	}
	if (ok && scan_value(temp, &epc->woody_turnover, 'd'))
	{
		printf("ERROR reading woody biomass turnover, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &t1, 'd'))
	{
		printf("ERROR reading whole-plant mortality, epc_init()\n");
		ok=0;
	}
	if (ok) epc->daily_mortality_turnover = t1/NDAYS_OF_YEAR;

	
	
	/* using varying whole plant mortality values (in transient or in normal run)*/
	if (ctrl->spinup == 0)
		strcpy(wpm_file.name, "mortality_normal.txt");
	else
		strcpy(wpm_file.name, "mortality_transient.txt");
	
	/* WPM flag: constans or varying WPM from file */
	if (ok && !file_open(&wpm_file,'j')) 
		ctrl->varWPM_flag = 1;
	else
		ctrl->varWPM_flag = 0;


	if (ok && ctrl->varWPM_flag) 
	{
		/* allocate space for the annual WPM array */
		epc->wpm_array = (double*) malloc(ctrl->simyears * sizeof(double));
		if (!epc->wpm_array)
		{
			printf("ERROR allocating for annual WPM array, epc_init()\n");
			ok=0;
		}

		/* read year and WPM for each simyear */
		for (i=0 ; ok && i<ctrl->simyears ; i++)
		{
			if (fscanf(wpm_file.ptr,"%*i%lf", &(epc->wpm_array[i]))==EOF)
			{
				printf("ERROR reading annual WPM array, epc_init()\n");
				printf("Note: file must contain a pair of values for each\n");
				printf("simyear: year and WPM.\n");
				ok=0;
			}
			if (epc->wpm_array[i] < 0.0)
			{
				printf("ERROR in epc_init(): wpm must be positive\n");
				ok=0;
			}
		}
		fclose(wpm_file.ptr);
	}	
	else /* if no changing data constant EPC parameter are used */
	{		
		epc->wpm_array = 0;
	}	
	/* ------------------------------------------------------ */
	/* fire mortality */
	if (ok && scan_value(temp, &t1, 'd'))
	{
		printf("ERROR reading fire mortality, epc_init()\n");
		ok=0;
	}
	if (ok) epc->daily_fire_turnover = t1/NDAYS_OF_YEAR;

	/*--------------------------------------------------------------------*/
	/* C:N ratios */
	if (ok && scan_value(temp, &epc->leaf_cn, 'd'))
	{
		printf("ERROR reading average leaf C:N, epc_init()\n");
		ok=0;
	}

	/* test for leaf C:N > 0 */
	if (ok && epc->leaf_cn <= 0	)
	{
		printf("ERROR: leaf  C:N must be > 0\n");
		printf("change the values in ECOPHYS block of initialization file\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->leaflitr_cn, 'd'))
	{
		printf("ERROR reading leaf litter C:N, epc_init()\n");
		ok=0;
	}
	/* test for leaflitter C:N > leaf C:N */
	if (ok && epc->leaflitr_cn < epc->leaf_cn)
	{
		printf("ERROR: leaf litter C:N must be >= leaf C:N\n");
		printf("change the values in ECOPHYS block of initialization file\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->froot_cn, 'd'))
	{
		printf("ERROR reading initial fine root C:N, epc_init()\n");
		ok=0;
	}

	/* test for froot C:N > leaf C:N */
	if (ok && epc->froot_cn < epc->leaf_cn)
	{
		printf("ERROR: fine root C:N must be >= leaf C:N\n");
		printf("change the values in ECOPHYS block of initialization file\n");
		ok=0;
	}

	/* fruit simulation */
	if (ok && scan_value(temp, &epc->fruit_cn, 'd'))
	{
		printf("ERROR reading initial fruit C:N, epc_init()\n");
		ok=0;
	}

	if (ok && epc->fruit_cn > 0 && epc->fruit_cn < epc->leaf_cn)
	{
		printf("ERROR: fruit C:N must be >= leaf C:N\n");
		printf("change the values in ECOPHYS block of initialization file\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->softstem_cn, 'd'))
	{
		printf("ERROR reading initial softstem C:N, epc_init()\n");
		ok=0;
	}

	if (ok && epc->softstem_cn > 0 && epc->softstem_cn < epc->leaf_cn)
	{
		printf("ERROR: softstem C:N must be >= leaf C:N\n");
		printf("change the values in ECOPHYS block of initialization file\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->livewood_cn, 'd'))
	{
		printf("ERROR reading initial livewood C:N, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->deadwood_cn, 'd'))
	{
		printf("ERROR reading initial deadwood C:N, epc_init()\n");
		ok=0;
	}

	/* test for deadwood C:N > livewood C:N */
	if (ok && epc->woody && epc->livewood_cn == 0)
	{
		printf("ERROR: livewood C:N must be > 0 in woody simulation \n");
		printf("change the values in ECOPHYS block of initialization file\n");
		ok=0;
	}

	/* test for deadwood C:N > livewood C:N */
	if (ok && epc->deadwood_cn < epc->livewood_cn)
	{
		printf("ERROR: livewood C:N must be >= deadwood C:N\n");
		printf("change the values in ECOPHYS block of initialization file\n");
		ok=0;
	}

	/*--------------------------------------------------------------------*/

	/* LEAF LITTER PROPORTION */
	if (ok && scan_value(temp, &t1, 'd'))
	{
		printf("ERROR reading leaf litter labile proportion, epc_init()\n");
		ok=0;
	}
	if (ok) epc->leaflitr_flab = t1;

	if (ok && scan_value(temp, &t2, 'd'))
	{
		printf("ERROR reading leaf litter cellulose proportion, epc_init()\n");
		ok=0;
	}

	/* livecroot lignin proportion is calculated from labile and cellulose */
	t3 = 1 - t2 - t1 ;

        /* test for fractions sum to 1.0 */
	if (ok) diff=1.0 - (t1+t2+t3);

	if (ok && (fabs(diff) > 1e-6))
	{
		printf("ERROR in fractions: %.2f %.2f %.2f %.3f\n",t1, t2, t3, t1+t2+t3);
		printf("leaf litter proportions of labile, cellulose, and lignin\n");
		printf("must sum to 1.0. Check initialization file and try again.\n");
		ok=0;
	}
	else t1 += diff;

	if (ok) epc->leaflitr_flig = t3;

	/* calculate shielded and unshielded cellulose fraction */
	if (ok)
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
	if (ok && scan_value(temp, &t1, 'd'))
	{
		printf("ERROR reading froot litter labile proportion, epc_init()\n");
		ok=0;
	}
	epc->frootlitr_flab = t1;
	if (ok && scan_value(temp, &t2, 'd'))
	{
		printf("ERROR reading froot litter cellulose proportion, epc_init()\n");
		ok=0;
	}
		
	/* livecroot lignin proportion is calculated from labile and cellulose */
	t3 = 1 - t2 - t1;
	
       /* test for fractions sum to 1.0 */
	if (ok) diff=1.0 - (t1+t2+t3);

	if (ok && (fabs(diff) > 1e-6))
	{
		printf("ERROR in fractions: %.2f %.2f %.2f %.3f\n",t1, t2, t3, t1+t2+t3);
		printf("froot proportions of labile, cellulose, and lignin\n");
		printf("must sum to 1.0. Check initialization file and try again.\n");
		ok=0;
	}
	else t1 += diff;
	if (ok) epc->frootlitr_flig = t3;
	

	/* calculate shielded and unshielded cellulose fraction */
	if (ok)
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
	if (ok && scan_value(temp, &t1, 'd'))
	{
		printf("ERROR reading fruit litter labile proportion, epc_init()\n");
		ok=0;
	}
	epc->fruitlitr_flab = t1;
	if (ok && scan_value(temp, &t2, 'd'))
	{
		printf("ERROR reading fruit litter cellulose proportion, epc_init()\n");
		ok=0;
	}
			
	/* livecroot lignin proportion is calculated from labile and cellulose */
	t3 = 1 - t2 - t1;

        /* test for fractions sum to 1.0 */
	if (ok) diff=1.0 - (t1+t2+t3);

	if (ok && (fabs(diff) > 1e-6))
	{
		printf("ERROR in fractions: %.2f %.2f %.2f %.3f\n",t1, t2, t3, t1+t2+t3);
		printf("fruit proportions of labile, cellulose, and lignin\n");
		printf("must sum to 1.0. Check initialization file and try again.\n");
		ok=0;
	}

	else t1 += diff;

	if (ok) epc->fruitlitr_flig = t3;

	/* calculate shielded and unshielded cellulose fraction */
	if (ok)
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
	if (ok && scan_value(temp, &t1, 'd'))
	{
		printf("ERROR reading soft stem litter labile proportion, epc_init()\n");
		ok=0;
	}
	epc->softstemlitr_flab = t1;
	if (ok && scan_value(temp, &t2, 'd'))
	{
		printf("ERROR reading soft stem  litter cellulose proportion, epc_init()\n");
		ok=0;
	}
	
	/* livecroot lignin proportion is calculated from labile and cellulose */
	t3 = 1 - t2 - t1;

       /* test for fractions sum to 1.0 */
	if (ok) diff=1.0 - (t1+t2+t3);

	if (ok && (fabs(diff) > 1e-6))
	{
		printf("ERROR in fractions: %.2f %.2f %.2f %.3f\n",t1, t2, t3, t1+t2+t3);
		printf("soft stem proportions of labile, cellulose, and lignin\n");
		printf("must sum to 1.0. Check initialization file and try again.\n");
		ok=0;
	}
	else t1 += diff;

	if (ok) epc->softstemlitr_flig = t3;

	/* calculate shielded and unshielded cellulose fraction */
	if (ok)
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
	if (ok && scan_value(temp, &t1, 'd'))
	{
		printf("ERROR reading dead wood %% cellulose, epc_init()\n");
		ok=0;
	}
	
     /* lignin proportion is calculated from labile and cellulose */
	t2 = 1 - t1;

        /* test for fractions sum to 1.0 */
	if (ok) diff=1.0 - (t1+t2);

	if (ok && (fabs(diff) > 1e-6))
	{
		printf("ERROR in fractions: %.2f %.2f %.3f\n",t1, t2, t1+t2);
		printf("leaf litter proportions of cellulose, and lignin\n");
		printf("must sum to 1.0. Check initialization file and try again.\n");
		ok=0;
	}
	else t1 += diff;

	if (ok) epc->deadwood_flig = t2;


	/* calculate shielded and unshielded cellulose fraction */
	if (ok)
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
	if (ok && scan_value(temp, &epc->int_coef, 'd'))
	{
		printf("ERROR reading canopy water int coef, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->ext_coef, 'd'))
	{
		printf("ERROR reading canopy light ext.coef, epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->potRUE, 'd'))
	{
		printf("ERROR reading potential radiation use efficiency, epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->rad_param1, 'd'))
	{
		printf("ERROR reading radiation parameter1, epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->rad_param2, 'd'))
	{
		printf("ERROR reading radiation parameter2, epc_init()\n");
		ok=0;
	}


	if (ok && scan_value(temp, &epc->lai_ratio, 'd'))
	{
		printf("ERROR reading all to projected LAI ratio, epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->sla_ratio, 'd'))
	{
		printf("ERROR reading shaded to sunlit SLA ratio, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->flnr, 'd'))
	{
		printf("ERROR reading Rubisco N fraction, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->flnp, 'd'))
	{
		printf("ERROR reading PeP N fraction, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->gl_smax, 'd'))
	{
		printf("ERROR reading gl_smax, epc_init()\n");
		ok=0;
	}

	/* ------------------------------------------------------ */
	/* using varying maximum stomatal conductance values */

	if (ctrl->spinup == 0)
		strcpy(msc_file.name, "conductance_normal.txt");
	else
		strcpy(msc_file.name, "conductance_transient.txt");
	
	/* MSC flag: constans or varying MSC from file */
	if (ok && !file_open(&msc_file,'j')) 
		ctrl->varMSC_flag = 1;
	else
		ctrl->varMSC_flag = 0;



	if (ok && ctrl->varMSC_flag) 
	{
		/* allocate space for the annual MSC array */
		epc->msc_array = (double*) malloc(ctrl->simyears * sizeof(double));
		if (!epc->msc_array)
		{
			printf("ERROR allocating for annual MSC array, epc_init()\n");
			ok=0;
		}

		/* read year and co2 concentration for each simyear */
		for (i=0 ; ok && i<ctrl->simyears ; i++)
		{
			if (fscanf(msc_file.ptr,"%*i%lf", &(epc->msc_array[i]))==EOF)
			{
				printf("ERROR reading annual MSC array, epc_init()\n");
				printf("Note: file must contain a pair of values for each\n");
				printf("simyear: year and MSC.\n");
				ok=0;
			}
			if (epc->msc_array[i] < 0.0)
			{
				printf("ERROR in epc_init(): msc must be positive\n");
				ok=0;
			}
		}
		fclose(msc_file.ptr);
	}	
	else /* if no changing data constant EPC parameter are used */
	{
		epc->msc_array = 0;
	}	
	
	/* ------------------------------------------------------ */
	if (ok && scan_value(temp, &epc->gl_c, 'd'))
	{
		printf("ERROR reading gl_c, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->gl_bl, 'd'))
	{
		printf("ERROR reading gl_bl, epc_init()\n");
		ok=0;
	}
	
	if (ok && scan_value(temp, &epc->relVWC_crit1, 'd'))
	{
		printf("ERROR reading relVWC_crit1, epc_init()\n");
		ok=0;
	}
	
    if (ok && scan_value(temp, &epc->relVWC_crit2, 'd'))
	{
		printf("ERROR reading relVWC_crit2, epc_init()\n");
		ok=0;
	}

	/* multiplier for PSI */
	if (ok && scan_value(temp, &epc->PSI_crit1, 'd'))
	{
		printf("ERROR reading PSI_crit1, epc_init()\n");
		ok=0;
	}
	
    if (ok && scan_value(temp, &epc->PSI_crit2, 'd'))
	{
		printf("ERROR reading PSI_crit2, epc_init()\n");
		ok=0;
	}


	if (ok && scan_value(temp, &epc->vpd_open, 'd')) 
	{
		printf("ERROR reading vpd_max, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->vpd_close, 'd'))
	{
		printf("ERROR reading vpd_min, epc_init()\n");
		ok=0;
	}

	/* max_plant_height */
	if (ok && scan_value(temp, &epc->max_plant_height, 'd'))
	{
		printf("ERROR reading number of max_plant_height\n");
		ok=0;
	}

	/* max_stem_weight  */
	if (ok && scan_value(temp, &epc->max_stem_weight, 'd'))
	{
		printf("ERROR reading number of max_stem_weight\n");
		ok=0;
	}

	/* max_rootzone_depth  */
	if (ok && scan_value(temp, &epc->max_rootzone_depth, 'd'))
	{
		printf("ERROR reading number of max_rootzone_depth\n");
		ok=0;
	}

	/* CONTROL to avoid negative  data */
 	if (ok && epc->max_rootzone_depth <= 0)
	{
		printf("ERROR in epc data in EPC file: negative or zero rootzone_depth, epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->rootdistrib_param, 'd'))
	{
		printf("ERROR reading rootdistrib_param: epc_init()\n");
		ok=0;
	}
	
	if (ok && scan_value(temp, &epc->rootlenght_par1, 'd'))
	{
		printf("ERROR reading rootlenght_par1: epc_init()\n");
		ok=0;
	}
	
	if (ok && scan_value(temp, &epc->rootlenght_par2, 'd'))
	{
		printf("ERROR reading rootlenght_par2: epc_init()\n");
		ok=0;
	}
	
	if (ok && scan_value(temp, &epc->GR_ratio, 'd'))
	{
		printf("ERROR reading growth resp.ratio: epc_init()\n");
		ok=0;
	}


	if (ok && scan_value(temp, &epc->mrpern, 'd'))
	{
		printf("ERROR reading mrpern: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->NSC_SC_prop, 'd'))
	{
		printf("ERROR reading NSC_SC_prop: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->NSC_avail_prop, 'd'))
	{
		printf("ERROR reading NSC_avail_prop: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->nfix, 'd'))
	{
		printf("ERROR reading N fixation, epc_init()\n");
		ok=0;
	}

	
	/*********************************************************************************************************************/
	/* dividing line from file */ 
	if (ok && scan_value(temp, keyword, 's'))
	{
		printf("ERROR reading keyword, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, keyword, 's'))
	{
		printf("ERROR reading keyword, epc_init()\n");
		ok=0;
	}
	/*********************************************************************************************************************/
	/* CROP SPECIFIC PARAMETERS */
	/*********************************************************************************************************************/

	/* number of phenophase of germination */
	if (ok && scan_value(temp, &epc->n_germ_phenophase, 'i'))
	{
		printf("ERROR reading number of phenophase of germination \n");
		ok=0;
	}

	/* number of phenophase of emergence */
	if (ok && scan_value(temp, &epc->n_emerg_phenophase, 'i'))
	{
		printf("ERROR reading number of phenophase of emergence\n");
		ok=0;
	}

	/* control of phenophase number */
	if (epc->transferGDD_flag && (epc->n_germ_phenophase == 0 || epc->n_emerg_phenophase == 0))
	{
		printf("ERROR in phenophase parametrization: if transferGDD_flag = 1 -> n_germ_phenophase and n_emerg_phenophase must be specified, epc_init()\n");
		ok=0;
	}

	if (epc->transferGDD_flag == 0 && (epc->n_germ_phenophase != 0 || epc->n_emerg_phenophase != 0))
	{
		printf("ERROR in phenophase parametrization: if transferGDD_flag = 0 -> n_germ_phenophase and n_emerg_phenophase is not used, epc_init()\n");
		ok=0;
	}

	/* critical relative SWC (prop. to FC) in germination */
	if (ok && scan_value(temp, &epc->grmn_paramSWC, 'd'))
	{
		printf("ERROR reading critical relative SWC in germination, epc_init()\n");
		ok=0;
	}
	
	/* number of phenophase of photoperiodic slowing effect */
	if (ok && scan_value(temp, &epc->n_phpsl_phenophase, 'i'))
	{
		printf("ERROR reading number of phenophase of photoperiodic slowing effect \n");
		ok=0;
	}

	/* critical photoslow daylength */
	if (ok && scan_value(temp, &epc->phpsl_parDL, 'd'))
	{
		printf("ERROR reading critical photoslow daylength, epc_init()\n");
		ok=0;
	}

	
	/* slope of relative photoslow development rate */
	if (ok && scan_value(temp, &epc->phpsl_parDR, 'd'))
	{
		printf("ERROR reading slope of relative photoslow development rate, epc_init()\n");
		ok=0;
	}

	/* number of phenophase of vernalization */
	if (ok && scan_value(temp, &epc->n_vern_phenophase, 'i'))
	{
		printf("ERROR reading number of vernalization phenophase\n");
		ok=0;
	}

	/* critical vernalization temperature data */
	if (ok && scan_value(temp, &epc->vern_parT1, 'd'))
	{
		printf("ERROR reading critical vernalization temperature 1, epc_init()\n");
		ok=0;
	}
	
	if (ok && scan_value(temp, &epc->vern_parT2, 'd'))
	{
		printf("ERROR reading critical vernalization temperature 2, epc_init()\n");
		ok=0;
	}
	
	if (ok && scan_value(temp, &epc->vern_parT3, 'd'))
	{
		printf("ERROR reading critical vernalization temperature 3, epc_init()\n");
		ok=0;
	}
	
	if (ok && scan_value(temp, &epc->vern_parT4, 'd'))
	{
		printf("ERROR reading critical vernalization temperature 4, epc_init()\n");
		ok=0;
	}
	
	/* control of vernalization temperature parameters */
	if (epc->vern_parT1 > epc->vern_parT2 ||epc->vern_parT2 > epc->vern_parT3 || epc->vern_parT3 > epc->vern_parT4) 
	{
			printf("ERROR in vernalization temperature data in EPC file\n");
			printf("Correct temperature data: vern_parT1 <= vern_parT2 <= vern_parT3 <= vern_parT4\n");
			ok=0;
	}

	/* slope of relative vernalization development rate  */
	if (ok && scan_value(temp, &epc->vern_parDR1, 'd'))
	{
		printf("ERROR reading slope of relative vernalization development rate, epc_init()\n");
		ok=0;
	}

	/* required vernalization days (in vernalization development rate  */
	if (ok && scan_value(temp, &epc->vern_parDR2, 'd'))
	{
		printf("ERROR reading required vernalization days (in vernalization development rate, epc_init()\n");
		ok=0;
	}
	
	/* number of phenophase of flowering heat stress */
	if (ok && scan_value(temp, &epc->n_flowHS_phenophase, 'i'))
	{
		printf("ERROR reading number of phenophase of flowering heat stress\n");
		ok=0;
	}

	/* critical flowering heat stress temperatures */
	if (ok && scan_value(temp, &epc->flowHS_parT1, 'd'))
	{
		printf("ERROR reading critical flowering heat stress temperature 1, epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->flowHS_parT2, 'd'))
	{
		printf("ERROR reading critical flowering heat stress temperature 2, epc_init()\n");
		ok=0;
	}

	/* control of flowering heat stress temperature parameter */
	if (epc->flowHS_parT1 > epc->flowHS_parT2) 
	{
			printf("ERROR in flowering heat stress temperature data in EPC file\n");
			printf("Correct temperature data: flowHS_parT1 <= flowHS_parT2 \n");
			ok=0;
	}

	/* mortality parameter of flowering heat stress */
	if (ok && scan_value(temp, &epc->flowHS_parMORT, 'd'))
	{
		printf("ERROR reading mortality parameter of flowering heat stress, epc_init()\n");
		ok=0;
	}

	/*********************************************************************************************************************/
	/* dividing line from file */ 
	if (ok && scan_value(temp, keyword, 's'))
	{
		printf("ERROR reading keyword, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, keyword, 's'))
	{
		printf("ERROR reading keyword, epc_init()\n");
		ok=0;
	}
	/*********************************************************************************************************************/
	/* SENESCENCE AND SOIL PARAMETERS */
	/*********************************************************************************************************************/


	if (ok && scan_value(temp, &epc->SNSCmort_abovebiom_max, 'd'))
	{
		printf("ERROR reading senescence mortality parameter of aboveground biomass: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->SNSCmort_belowbiom_max, 'd'))
	{
		printf("ERROR reading senescence mortality parameter of belowground biomass: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->SNSCmort_nsc_max, 'd'))
	{
		printf("ERROR reading senescence mortality parameter of non-stuctured biomass: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->extremT_effect_max, 'd'))
	{
		printf("ERROR reading effect of extreme high temperature on senesncene mortality: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->SNSC_extremT1, 'd'))
	{
		printf("ERROR reading lower limit extreme high temperature effect on senesncene mortality: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->SNSC_extremT2, 'd'))
	{
		printf("ERROR reading lower limit extreme high temperature effect on senesncene mortality: epc_init()\n");
		ok=0;
	}


	if (ok && scan_value(temp, &epc->mort_MAX_lifetime, 'd'))
	{
		printf("ERROR reading maximal lifetime of plant tissue parameter: epc_init()\n");
		ok=0;
	}

	/* CONTROL for genetically programmed senescence */
 	if (epc->mort_MAX_lifetime == DATA_GAP)
	{
		printf("NO genetically programmed senescence, epc_init()\n");
	}

	if (ok && scan_value(temp, &epc->mort_SNSC_to_litter, 'd'))
	{
		printf("ERROR reading turnover rate of wilted standing biomass to litter parameter: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->mort_CnW_to_litter, 'd'))
	{
		printf("ERROR reading turnover rate of cut-down non-woody biomass to litter parameter: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->mort_CW_to_litter, 'd'))
	{
		printf("ERROR reading turnover rate of cut-down woody biomass to litter parameter: epc_init()\n");
		ok=0;
	}


	if (ok && scan_value(temp, &epc->dsws_crit, 'd'))
	{
		printf("ERROR reading dsws_crit: epc_init()\n");
		ok=0;
	}

	/*  denetirification proportion and mobilen proportion
		original: BBGC constant - new version: can be set in EPC file*/
	if (ok && scan_value(temp, &epc->denitr_coeff, 'd'))
	{
		printf("ERROR reading denitr_coeff: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->nitrif_coeff1, 'd'))
	{
		printf("ERROR reading nitrif_coeff1: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->nitrif_coeff2, 'd'))
	{
		printf("ERROR reading nitrif_coeff1: epc_init()\n");
		ok=0;
	}
	
	if (ok && scan_value(temp, &epc->N2Ocoeff_nitrif, 'd'))
	{
		printf("ERROR reading N2Ocoeff_nitrif: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->NdepNH4_coeff, 'd'))
	{
		printf("ERROR reading NdepNH4_coeff: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->NH4_mobilen_prop, 'd'))
	{
		printf("ERROR reading NH4_mobilen_prop: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->NO3_mobilen_prop, 'd'))
	{
		printf("ERROR reading NO3_mobilen_prop: epc_init()\n");
		ok=0;
	}

	

	if (ok && scan_value(temp, &epc->efolding_depth, 'd'))
	{
		printf("ERROR reading e-folding depth: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->SOIL1_dissolv_prop, 'd'))
	{
		printf("ERROR reading SOIL1_dissolv_prop: epc_init()\n");
		ok=0;
	}
	
	if (ok && scan_value(temp, &epc->SOIL2_dissolv_prop, 'd'))
	{
		printf("ERROR reading SOIL2_dissolv_prop: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->SOIL3_dissolv_prop, 'd'))
	{
		printf("ERROR reading SOIL3_dissolv_prop: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->SOIL4_dissolv_prop, 'd'))
	{
		printf("ERROR reading SOIL4_dissolv_prop: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->BSE_PE_prop, 'd'))
	{
		printf("ERROR reading BSE_PE_prop: epc_init()\n");
		ok=0;
	}


	/*********************************************************************************************************************/
	/* dividing line from file */ 
	if (ok && scan_value(temp, keyword, 's'))
	{
		printf("ERROR reading keyword, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, keyword, 's'))
	{
		printf("ERROR reading keyword, epc_init()\n");
		ok=0;
	}
	/*********************************************************************************************************************/
	/* RATE SCALARS */
	/*********************************************************************************************************************/
	
	/* respiration fractions for fluxes between compartments  */
	if (ok && scan_value(temp, &epc->rfl1s1, 'd'))
	{
		printf("ERROR reading rfl1s1: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->rfl2s2, 'd'))
	{
		printf("ERROR reading rfl2s2: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->rfl4s3, 'd'))
	{
		printf("ERROR reading rfl4s3: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->rfs1s2, 'd'))
	{
		printf("ERROR reading rfs1s2: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->rfs2s3, 'd'))
	{
		printf("ERROR reading rfs2s3: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->rfs3s4, 'd'))
	{
		printf("ERROR reading rfs3s4: epc_init()\n");
		ok=0;
	}

	/* 	base values of rate constants are (1/day)   */
	if (ok && scan_value(temp, &epc->kl1_base, 'd'))
	{
		printf("ERROR reading kl1_base: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->kl2_base, 'd'))
	{
		printf("ERROR reading kl2_base: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->kl4_base, 'd'))
	{
		printf("ERROR reading kl4_base: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->ks1_base, 'd'))
	{
		printf("ERROR reading ks1_base: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->ks2_base, 'd'))
	{
		printf("ERROR reading ks2_base: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->ks3_base, 'd'))
	{
		printf("ERROR reading ks3_base: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->ks4_base, 'd'))
	{
		printf("ERROR reading ks4_base: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->kfrag_base, 'd'))
	{
		printf("ERROR reading kfrag_base: epc_init()\n");
		ok=0;
	}
	
	/*********************************************************************************************************************/
	/* dividing line from file */ 
	if (ok && scan_value(temp, keyword, 's'))
	{
		printf("ERROR reading keyword, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, keyword, 's'))
	{
		printf("ERROR reading keyword, epc_init()\n");
		ok=0;
	}
	/*********************************************************************************************************************/
	/* GROWING SEASON PARAMETERS */
	/*********************************************************************************************************************/
	
	/* snowcover_limit for calculation heatsum */
	if (ok && scan_value(temp, &GSI->snowcover_limit, 'd'))
	{
		printf("ERROR reading snowcover_limit parameter: epc_init()\n");
		ok=0;
	}
	
	/* heatsum_limit1 for calculation heatsum index */
	if (ok && scan_value(temp, &GSI->heatsum_limit1, 'd'))
	{
		printf("ERROR reading heatsum_limit1 parameter: epc_init()\n");
		ok=0;
	}

	/* heatsum_limit2 for calculation heatsum index */
	if (ok && scan_value(temp, &GSI->heatsum_limit2, 'd'))
	{
		printf("ERROR reading heatsum_limit2 parameter: epc_init()\n");
		ok=0;
	}

	/* tmin_limit1 for calculation tmin index */
	if (ok && scan_value(temp, &GSI->tmin_limit1, 'd'))
	{
		printf("ERROR reading tmin_limit1 parameter: epc_init()\n");
		ok=0;
	}

	/* tmin_limit2 for calculation tmin index */
	if (ok && scan_value(temp, &GSI->tmin_limit2, 'd'))
	{
		printf("ERROR reading tmin_limit2 parameter: epc_init()\n");
		ok=0;
	}

	/* vpd_limit1 for calculation vpd index */
	if (ok && scan_value(temp, &GSI->vpd_limit1, 'd'))
	{
		printf("ERROR reading vpd_limit1 parameter: epc_init()\n");
		ok=0;
	}

	/* vpd for calculation vpd index */
	if (ok && scan_value(temp, &GSI->vpd_limit2, 'd'))
	{
		printf("ERROR reading vpd_limit2 parameter: epc_init()\n");
		ok=0;
	}

	/* dayl_limit1 for calculation dayl index */
	if (ok && scan_value(temp, &GSI->dayl_limit1, 'd'))
	{
		printf("ERROR reading dayl_limit1 parameter: epc_init()\n");
		ok=0;
	}

	/* dayl_limit2 for calculation dayl index */
	if (ok && scan_value(temp, &GSI->dayl_limit2, 'd'))
	{
		printf("ERROR reading dayl_limit2 parameter: epc_init()\n");
		ok=0;
	}

	/* n_moving_avg for calculation moving average from indexes */
	if (ok && scan_value(temp, &GSI->n_moving_avg, 'i'))
	{
		printf("ERROR reading n_moving_avg parameter: epc_init()\n");
		ok=0;
	}
	if (ok && (GSI->n_moving_avg < 2))
	{
		printf("ERROR in moving average parameter of GSI calculation:\n");
		printf("must greater then 1. Check EPC file and try again.\n");
		ok=0;
	}

	/* GSI_limit for calculation yday of start growing season  */
	if (ok && scan_value(temp, &GSI->GSI_limit_SGS, 'd'))
	{
		printf("ERROR reading GSI_limit_SGS parameter: epc_init()\n");
		ok=0;
	}

	/* GSI_limit for calculation yday of end growing season  */
	if (ok && scan_value(temp, &GSI->GSI_limit_EGS, 'd'))
	{
		printf("ERROR reading GSI_limit_EGS parameter: epc_init()\n");
		ok=0;
	}
	/*********************************************************************************************************************/
	/* dividing line from file */ 
	if (ok && scan_value(temp, keyword, 's'))
	{
		printf("ERROR reading keyword, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, keyword, 's'))
	{
		printf("ERROR reading keyword, epc_init()\n");
		ok=0;
	}
	/*********************************************************************************************************************/
	/* CH4 PARAMETERS */
	/*********************************************************************************************************************/

	if (ok && scan_value(temp, &epc->C_pBD1, 'd'))
	{
		printf("ERROR reading C_pBD1: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->C_pBD2, 'd'))
	{
		printf("ERROR reading C_pBD2: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->C_pVWC1, 'd'))
	{
		printf("ERROR reading C_pVWC1: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->C_pVWC2, 'd'))
	{
		printf("ERROR reading C_pVWC2: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->C_pVWC3, 'd'))
	{
		printf("ERROR reading C_pVWC3: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->C_pVWC4, 'd'))
	{
		printf("ERROR reading C_pVWC4: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->C_pTS, 'd'))
	{
		printf("ERROR reading C_pTS: epc_init()\n");
		ok=0;
	}
	
	/*********************************************************************************************************************/
	/* dividing line from file */ 
	if (ok && scan_value(temp, keyword, 's'))
	{
		printf("ERROR reading keyword, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, keyword, 's'))
	{
		printf("ERROR reading keyword, epc_init()\n");
		ok=0;
	}
	/*********************************************************************************************************************/
	/* PHENOLOGICAL (ALLOCATION) PARAMETERS (7 phenological phases) */
    /*********************************************************************************************************************/

	if (ok && scan_value(temp, keyword, 's'))
	{
		printf("ERROR reading name of the phenophases\n");
		ok=0;
	}

	scanflag=0; 
	for (phenphase=0; phenphase<N_PHENPHASES; phenphase++)
	{
		if (phenphase==N_PHENPHASES-1) scanflag=1;
		if (ok && scan_array(temp, &(epc->phenophase_length[phenphase]), 'd', scanflag))
		{
			printf("ERROR reading phenophase_length in phenophase %i, epc_init()\n", phenphase+1);
			ok=0;
		}
	}

	scanflag=0; 
	for (phenphase=0; phenphase<N_PHENPHASES; phenphase++)
	{
		if (phenphase==N_PHENPHASES-1) scanflag=1;
		if (ok && scan_array(temp, &(epc->alloc_leafc[phenphase]), 'd', scanflag))
		{
			printf("ERROR reading alloc_frootc in phenophase %i, epc_init()\n", phenphase+1);
			ok=0;
		}
	}

	scanflag=0; 
	for (phenphase=0; phenphase<N_PHENPHASES; phenphase++)
	{
		if (phenphase==N_PHENPHASES-1) scanflag=1;
		if (ok && scan_array(temp, &(epc->alloc_frootc[phenphase]), 'd', scanflag))
		{
			printf("ERROR reading alloc_frootc in phenophase %i, epc_init()\n", phenphase+1);
			ok=0;
		}
	}

	scanflag=0; 
	for (phenphase=0; phenphase<N_PHENPHASES; phenphase++)
	{
		if (phenphase==N_PHENPHASES-1) scanflag=1;
		if (ok && scan_array(temp, &(epc->alloc_fruitc[phenphase]), 'd', scanflag))
		{
			printf("ERROR reading alloc_fruitc in phenophase %i, epc_init()\n", phenphase+1);
			ok=0;
		}
	}

	scanflag=0; 
	for (phenphase=0; phenphase<N_PHENPHASES; phenphase++)
	{
		if (phenphase==N_PHENPHASES-1) scanflag=1;
		if (ok && scan_array(temp, &(epc->alloc_softstemc[phenphase]), 'd', scanflag))
		{
			printf("ERROR reading alloc_softstemc in phenophase %i, epc_init()\n", phenphase+1);
			ok=0;
		}
	}

	scanflag=0; 
	for (phenphase=0; phenphase<N_PHENPHASES; phenphase++)
	{
		if (phenphase==N_PHENPHASES-1) scanflag=1;
		if (ok && scan_array(temp, &(epc->alloc_livestemc[phenphase]), 'd', scanflag))
		{
			printf("ERROR reading alloc_woodystemc_live in phenophase %i, epc_init()\n", phenphase+1);
			ok=0;
		}
	}

	scanflag=0; 
	for (phenphase=0; phenphase<N_PHENPHASES; phenphase++)
	{
		if (phenphase==N_PHENPHASES-1) scanflag=1;
		if (ok && scan_array(temp, &(epc->alloc_deadstemc[phenphase]), 'd', scanflag))
		{
			printf("ERROR reading alloc_woodystemc_dead in phenophase %i, epc_init()\n", phenphase+1);
			ok=0;
		}
	}

	scanflag=0; 
	for (phenphase=0; phenphase<N_PHENPHASES; phenphase++)
	{
		if (phenphase==N_PHENPHASES-1) scanflag=1;
		if (ok && scan_array(temp, &(epc->alloc_livecrootc[phenphase]), 'd', scanflag))
		{
			printf("ERROR reading alloc_crootc_live in phenophase %i, epc_init()\n", phenphase+1);
			ok=0;
		}
	}

	scanflag=0; 
	for (phenphase=0; phenphase<N_PHENPHASES; phenphase++)
	{
		if (phenphase==N_PHENPHASES-1) scanflag=1;
		if (ok && scan_array(temp, &(epc->alloc_deadcrootc[phenphase]), 'd', scanflag))
		{
			printf("ERROR reading alloc_crootc_dead in phenophase %i, epc_init()\n", phenphase+1);
			ok=0;
		}
	}

	/* control of allocation parameter: test for alloc.fractions sum to 1.0  in every phenophases */
	for (phenphase=0; phenphase<N_PHENPHASES; phenphase++)
	{
		sum = epc->alloc_leafc[phenphase] + epc->alloc_frootc[phenphase] + epc->alloc_fruitc[phenphase] + epc->alloc_softstemc[phenphase] +
			  epc->alloc_livestemc[phenphase]   + epc->alloc_deadstemc[phenphase] + 
			  epc->alloc_livecrootc[phenphase]  + epc->alloc_deadcrootc[phenphase];

		if (ok && sum != 0 && fabs(sum - 1.) > CRIT_PREC)
		{
			printf("ERROR in allocation parameters in phenophase %i, epc_init()\n", phenphase+1);
			printf("Allocation parameters must sum to 1.0 in every phenophase. Check EPC file and try again.\n");
			ok=0;
		
		}

		/* control: after germination allocation parameter setting is necessary */
		if (ok && (phenphase == epc->n_germ_phenophase - 1 || phenphase == epc->n_emerg_phenophase - 1)
			   && (epc->alloc_leafc[phenphase] == 0 || epc->alloc_frootc[phenphase] == 0))
		{
			printf("ERROR in allocation parameters in phenophase %i, epc_init()\n", phenphase+1);
			printf("After germination leaf and froot allocation parameter setting is necessary. Check EPC file and try again.\n");
			ok=0;
		}

		/* control: woody biomes - no softstem */
		if (ok && epc->woody && epc->alloc_softstemc[phenphase] > 0)
		{
			printf("ERROR in allocation parameters in phenophase %i, epc_init()\n", phenphase+1);
			printf("No softstem allocation in case of woody biomass. Check EPC file and try again.\n");
			ok=0;
		}

		/* control: non-woody biomes - no woody allocation */
		if (ok && !epc->woody && 
		   (epc->alloc_livecrootc[phenphase] > 0 || epc->alloc_livestemc[phenphase] > 0 || epc->alloc_deadcrootc[phenphase] > 0 || epc->alloc_deadstemc[phenphase] > 0))
		{
			printf("ERROR in allocation parameters in phenophase %i, epc_init()\n", phenphase+1);
			printf("No woody allocation in case of non-woody biomass. Check EPC file and try again.\n");
			ok=0;
		}

	}


	scanflag=0; 
	for (phenphase=0; phenphase<N_PHENPHASES; phenphase++)
	{
		if (phenphase==N_PHENPHASES-1) scanflag=1;
		if (ok && scan_array(temp, &(epc->avg_proj_sla[phenphase]), 'd', scanflag))
		{
			printf("ERROR reading avg_proj_sla in phenophase %i, epc_init()\n", phenphase+1);
			ok=0;
		}

		if (ok && epc->avg_proj_sla[phenphase] < 0)
		{
			printf("ERROR in specific leaf area in phenophase %i, epc_init()\n", phenphase+1);
			printf("Specific leaf area must greater than to 0.0 in every phenophase. Check EPC file and try again.\n");
			ok=0;
		
		}

		if (ok && epc->avg_proj_sla[phenphase] == 0 && epc->alloc_leafc[phenphase] > 0)
		{
			printf("ERROR in specific leaf area in phenophase %i, epc_init()\n", phenphase+1);
			printf("Specific leaf area must greater than to 0.0 in case of leaf allocation. Check EPC file and try again.\n");
			ok=0;
		
		}
	}

	scanflag=0; 
	for (phenphase=0; phenphase<N_PHENPHASES; phenphase++)
	{
		if (phenphase==N_PHENPHASES-1) scanflag=1;
		if (ok && scan_array(temp, &(epc->curgrowth_prop[phenphase]), 'd', scanflag))
		{
			printf("ERROR reading curgrowth_prop in phenophase %i, epc_init()\n", phenphase+1);
			ok=0;
		}
	}

	/* -------------------------------------------*/
	if (dofileclose) fclose(temp.ptr);
		
	return (!ok);

}
