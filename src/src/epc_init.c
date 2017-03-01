/* 
epc_init.c
read epc file for pointbgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.4

Original code: Copyright 2000, Peter E. Thornton
20.03.01 Galina Churkina added variable "sum" substituting  t1+t2+t3 in IF statement,
which gave an error.
Numerical Terradynamic Simulation Group, The University of Montana, USA

Modified code: Copyright 2017, D. Hidy [dori.hidy@gmail.com]
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
#include "pointbgc_struct.h"
#include "pointbgc_func.h"
#include "bgc_constants.h"



int epc_init(file init, epconst_struct* epc, control_struct* ctrl)
{
	int ok = 1;
	int dofileclose = 1;
	double t1 = 0;
	double t2 = 0;
	double t3 = 0;
	double diff = 0;
	double t4,r1;
	int i;
	file temp, wpm_file, msc_file, sgs_file, egs_file; 	// Hidy 2011.
	char key1[] = "EPC_FILE";
	char key2[] = "ECOPHYS";
	char keyword[80];

	
	/********************************************************************
	**                                                                 **
	** Begin reading initialization file block starting with keyword:  **
	** EPC_FILES                                                       ** 
	**                                                                 **
	********************************************************************/
	
	/* scan for the EPC file keyword, exit if not next */
	if (ok && scan_value(init, keyword, 's'))
	{
		printf("Error reading keyword for control data\n");
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
		printf("Error opening epconst file, epc_init()\n");
		dofileclose = 0;
		ok=0;
	}
	
	/* first scan epc keyword to ensure proper *.init format */
	if (ok && scan_value(temp, keyword, 's'))
	{
		printf("Error reading keyword, epc_init()\n");
		ok=0;
	}
	if (ok && strcmp(keyword,key2))
	{
		printf("Expecting keyword --> %s in %s\n",key2,init.name);
		ok=0;
	}
	
	/* begin reading constants from *.init */
	if (ok && scan_value(temp, &epc->woody, 'i'))
	{
		printf("Error reading woody/non-woody flag, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->evergreen, 'i'))
	{
		printf("Error reading evergreen/deciduous flag, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->c3_flag, 'i'))
	{
		printf("Error reading C3/C4 flag, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->phenology_flag, 'i'))
	{
		printf("Error reading phenology flag, epc_init()\n");
		ok=0;
	}
	/* ------------------------------------------------------ */
	/* new flags (Hidy 2015) */

	/* temperature dependent q10 value */
	if (ok && scan_value(temp, &epc->q10depend_flag, 'i'))
	{
		printf("Error reading q10depend_flag, epc_init()\n");
		ok=0;
	}
	/* acclimation */
	if (ok && scan_value(temp, &epc->acclimation_flag, 'i'))
	{
		printf("Error reading acclimation_flag, epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->CO2conduct_flag, 'i'))
	{
		printf("Error reading CO2conduct_flag, epc_init()\n");
		ok=0;
	}

	/* soil water calculation flag */
	if (ok && scan_value(temp, &epc->SHCM_flag, 'i'))
	{
		printf("Error reading soil hydrological calculation method flag: epc_init()\n");
		ok=0;
	}

	/*  discretitaion level of SWC calculation simulation */
	if (ok && scan_value(temp, &epc->discretlevel_Richards, 'i'))
	{
		printf("Error reading discretitaion level of SWC calculation: epc_init.c\n");
		ok=0;
	}

	/* soil temperature calculation flag */
	if (ok && scan_value(temp, &epc->STCM_flag, 'i'))
	{
		printf("Error reading soil temperature calculation flag: epc_init()\n");
		ok=0;
	}
	
	/* ------------------------------------------------------ */
	/* Hidy 2015 - using varying onday values (in transient or in normal run)*/

	if (ok && scan_value(temp, &epc->onday, 'i'))
	{
		printf("Error reading onday, epc_init()\n");
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
			printf("Error allocating for annual SGS array, epc_init()\n");
			ok=0;
		}

		/* read year and SGS for each simyear */
		for (i=0 ; ok && i<ctrl->simyears ; i++)
		{
			if (fscanf(sgs_file.ptr,"%*i%lf", &(epc->sgs_array[i]))==EOF)
			{
				printf("Error reading annual SGS array, epc_init()\n");
				printf("Note: file must contain a pair of values for each\n");
				printf("simyear: year and SGS.\n");
				ok=0;
			}
			if (epc->sgs_array[i] < 0.0)
			{
				printf("Error in epc_init(): sgs must be positive\n");
				ok=0;
			}
		}
		fclose(sgs_file.ptr);
	}	
	else /* if no changing data constant EPC parameter are used */
	{
		epc->sgs_array = 0;
	}	

	/* ------------------------------------------------------ */


	if (ok && scan_value(temp, &epc->offday, 'i'))
	{
		printf("Error reading offday, epc_init()\n");
		ok=0;
	}

	/* ------------------------------------------------------ */
	/* Hidy 2015 - using varying offday values (in transient or in normal run)*/

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
			printf("Error allocating for annual EGS array, epc_init()\n");
			ok=0;
		}

		/* read year and EGS for each simyear */
		for (i=0 ; ok && i<ctrl->simyears ; i++)
		{
			if (fscanf(egs_file.ptr,"%*i%lf", &(epc->egs_array[i]))==EOF)
			{
				printf("Error reading annual EGS array, epc_init()\n");
				printf("Note: file must contain a pair of values for each\n");
				printf("simyear: year and EGS.\n");
				ok=0;
			}
			if (epc->egs_array[i] < 0.0)
			{
				printf("Error in epc_init(): egs must be positive\n");
				ok=0;
			}
		}
		fclose(egs_file.ptr);
	}	
	else /* if no changing data constant EPC parameter are used */
	{
 		epc->egs_array = 0;
	}	



	/* ------------------------------------------------------ */
	

	if (ok && scan_value(temp, &epc->transfer_pdays, 'd'))
	{
		printf("Error reading transfer_pdays, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->litfall_pdays, 'd'))
	{
		printf("Error reading litfall_pdays, epc_init()\n");
		ok=0;
	}

	/* basic_temperature for calculation GDD / heatsum - Hidy 2015 */
	if (ok && scan_value(temp, &epc->base_temp, 'i'))
	{
		printf("Error reading base_temp parameter: epc_init()\n");
		ok=0;
	}

	/* growing degree days for start of fruit allocation and leaf senescence  - Hidy 2015*/
	if (ok && scan_value(temp, &epc->GDD_fruitalloc, 'i'))
	{
		printf("Error reading GDD_fruitalloc, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->GDD_maturity, 'i'))
	{
		printf("Error reading GDD_maturity, epc_init()\n");
		ok=0;
	}
	

	if (ok && scan_value(temp, &epc->leaf_turnover, 'd'))
	{
		printf("Error reading leaf turnover, epc_init()\n");
		ok=0;
	}
	/* force leaf turnover fraction to 1.0 if deciduous */
	if (!epc->evergreen)
	{
		epc->leaf_turnover = 1.0;
	}
	if (ok) epc->froot_turnover    = epc->leaf_turnover;
	if (ok) epc->fruit_turnover    = epc->leaf_turnover;
	if (ok) epc->softstem_turnover = epc->leaf_turnover;

	if (ok && scan_value(temp, &epc->livewood_turnover, 'd'))
	{
		printf("Error reading livewood turnover, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &t1, 'd'))
	{
		printf("Error reading whole-plant mortality, epc_init()\n");
		ok=0;
	}
	if (ok) epc->daily_mortality_turnover = t1/NDAY_OF_YEAR;

	/* ------------------------------------------------------ */
	/* Hidy 2013 - using varying whole plant mortality values (in transient or in normal run)*/

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
			printf("Error allocating for annual WPM array, epc_init()\n");
			ok=0;
		}

		/* read year and WPM for each simyear */
		for (i=0 ; ok && i<ctrl->simyears ; i++)
		{
			if (fscanf(wpm_file.ptr,"%*i%lf", &(epc->wpm_array[i]))==EOF)
			{
				printf("Error reading annual WPM array, epc_init()\n");
				printf("Note: file must contain a pair of values for each\n");
				printf("simyear: year and WPM.\n");
				ok=0;
			}
			if (epc->wpm_array[i] < 0.0)
			{
				printf("Error in epc_init(): wpm must be positive\n");
				ok=0;
			}
		}
		fclose(wpm_file.ptr);
	}	
	else /* if no changing data constant EPC parameter are used */
	{		
		epc->wpm_array = 0;
	}	
	

	if (ok && scan_value(temp, &t1, 'd'))
	{
		printf("Error reading fire mortality, epc_init()\n");
		ok=0;
	}
	if (ok) epc->daily_fire_turnover = t1/NDAY_OF_YEAR;

	if (ok && scan_value(temp, &epc->alloc_frootc_leafc, 'd'))
	{
		printf("Error reading froot C:leaf C, epc_init()\n");
		ok=0;
	}
	/* fruit simulation - Hidy 2013. */
	if (ok && scan_value(temp, &epc->alloc_fruitc_leafc, 'd'))
	{
		printf("Error reading fruit C: leaf c, epc_init()\n");
		ok=0;
	}
	/* sofstem simulation - Hidy 2015. */
	if (ok && scan_value(temp, &epc->alloc_softstemc_leafc, 'd'))
	{
		printf("Error reading sofstem C: leaf c, epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->alloc_newstemc_newleafc, 'd'))
	{
		printf("Error reading new stemC:new leaf C, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->alloc_newlivewoodc_newwoodc, 'd'))
	{
		printf("Error reading new livewood C:new wood C, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->alloc_crootc_stemc, 'd'))
	{
		printf("Error reading croot C:stem C, epc_init()\n");
		ok=0;
	}
	
	if (ok && scan_value(temp, &epc->alloc_prop_curgrowth, 'd'))
	{
		printf("Error reading new growth:storage growth, epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->leaf_cn, 'd'))
	{
		printf("Error reading average leaf C:N, epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->leaflitr_cn, 'd'))
	{
		printf("Error reading leaf litter C:N, epc_init()\n");
		ok=0;
	}
	/* test for leaflitter C:N > leaf C:N */
	if (ok && epc->leaflitr_cn < epc->leaf_cn)
	{
		printf("Error: leaf litter C:N must be >= leaf C:N\n");
		printf("change the values in ECOPHYS block of initialization file\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->froot_cn, 'd'))
	{
		printf("Error reading initial fine root C:N, epc_init()\n");
		ok=0;
	}

	/* test for froot C:N > leaf C:N */
	if (ok && epc->froot_cn < epc->leaf_cn)
	{
		printf("Error: fine root C:N must be >= leaf C:N\n");
		printf("change the values in ECOPHYS block of initialization file\n");
		ok=0;
	}

	/* fruit simulation - Hidy 2013. */
	if (ok && scan_value(temp, &epc->fruit_cn, 'd'))
	{
		printf("Error reading initial fruit C:N, epc_init()\n");
		ok=0;
	}

	if (ok && epc->fruit_cn < epc->leaf_cn && epc->alloc_fruitc_leafc > 0)
	{
		printf("Error: fruit C:N must be >= leaf C:N\n");
		printf("change the values in ECOPHYS block of initialization file\n");
		ok=0;
	}


	/* softstem simulation - Hidy 2013. */
	if (ok && scan_value(temp, &epc->softstem_cn, 'd'))
	{
		printf("Error reading initial softstem C:N, epc_init()\n");
		ok=0;
	}

	if (ok && epc->softstem_cn < epc->leaf_cn && epc->alloc_softstemc_leafc > 0 && !epc->woody)
	{
		printf("Error: softstem C:N must be >= leaf C:N\n");
		printf("change the values in ECOPHYS block of initialization file\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->livewood_cn, 'd'))
	{
		printf("Error reading initial livewood C:N, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->deadwood_cn, 'd'))
	{
		printf("Error reading initial deadwood C:N, epc_init()\n");
		ok=0;
	}
	/* test for deadwood C:N > livewood C:N */
	if (ok && epc->deadwood_cn < epc->livewood_cn)
	{
		printf("Error: livewood C:N must be >= deadwood C:N\n");
		printf("change the values in ECOPHYS block of initialization file\n");
		ok=0;
	}

	/* CONTROL to avoid division by zero - Hidy 2015 */
	if (epc->leaf_cn == 0)     epc->leaf_cn     = fabs(DATA_GAP);
	if (epc->leaflitr_cn == 0) epc->leaflitr_cn = fabs(DATA_GAP);
	if (epc->froot_cn == 0)	   epc->froot_cn    = fabs(DATA_GAP);
	if (epc->fruit_cn == 0)	   epc->fruit_cn    = fabs(DATA_GAP);
	if (epc->softstem_cn == 0) epc->softstem_cn = fabs(DATA_GAP);
	if (epc->livewood_cn == 0) epc->livewood_cn = fabs(DATA_GAP);
	if (epc->deadwood_cn == 0) epc->deadwood_cn = fabs(DATA_GAP);
	/*--------------------------------------------------------------------*/

	/* LEAF LITTER PROPORTION */
	if (ok && scan_value(temp, &t1, 'd'))
	{
		printf("Error reading leaf litter labile proportion, epc_init()\n");
		ok=0;
	}
	if (ok) epc->leaflitr_flab = t1;

	if (ok && scan_value(temp, &t2, 'd'))
	{
		printf("Error reading leaf litter cellulose proportion, epc_init()\n");
		ok=0;
	}

	/* Hidy 2015 - lignin proportion is calculated from labile and cellulose */
	t3 = 1 - t2 - t1 ;

        /* test for fractions sum to 1.0 */
	if (ok) diff=1.0 - (t1+t2+t3);

	if (ok && (fabs(diff) > 1e-6))
	{
		printf("Error in fractions: %.2f %.2f %.2f %.3f\n",t1, t2, t3, t1+t2+t3);
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
		printf("Error reading froot litter labile proportion, epc_init()\n");
		ok=0;
	}
	epc->frootlitr_flab = t1;
	if (ok && scan_value(temp, &t2, 'd'))
	{
		printf("Error reading froot litter cellulose proportion, epc_init()\n");
		ok=0;
	}
		
	/* Hidy 2015 - lignin proportion is calculated from labile and cellulose */
	t3 = 1 - t2 - t1;
	
       /* test for fractions sum to 1.0 */
	if (ok) diff=1.0 - (t1+t2+t3);

	if (ok && (fabs(diff) > 1e-6))
	{
		printf("Error in fractions: %.2f %.2f %.2f %.3f\n",t1, t2, t3, t1+t2+t3);
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
		printf("Error reading fruit litter labile proportion, epc_init()\n");
		ok=0;
	}
	epc->fruitlitr_flab = t1;
	if (ok && scan_value(temp, &t2, 'd'))
	{
		printf("Error reading fruit litter cellulose proportion, epc_init()\n");
		ok=0;
	}
			
	/* Hidy 2015 - lignin proportion is calculated from labile and cellulose */
	t3 = 1 - t2 - t1;

        /* test for fractions sum to 1.0 */
	if (ok) diff=1.0 - (t1+t2+t3);

	if (ok && (fabs(diff) > 1e-6))
	{
		printf("Error in fractions: %.2f %.2f %.2f %.3f\n",t1, t2, t3, t1+t2+t3);
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
		printf("Error reading soft stem litter labile proportion, epc_init()\n");
		ok=0;
	}
	epc->softstemlitr_flab = t1;
	if (ok && scan_value(temp, &t2, 'd'))
	{
		printf("Error reading soft stem  litter cellulose proportion, epc_init()\n");
		ok=0;
	}
	
	/* Hidy 2015 - lignin proportion is calculated from labile and cellulose */
	t3 = 1 - t2 - t1;

       /* test for fractions sum to 1.0 */
	if (ok) diff=1.0 - (t1+t2+t3);

	if (ok && (fabs(diff) > 1e-6))
	{
		printf("Error in fractions: %.2f %.2f %.2f %.3f\n",t1, t2, t3, t1+t2+t3);
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
		printf("Error reading dead wood %% cellulose, epc_init()\n");
		ok=0;
	}
	
        /* Hidy 2015 - lignin proportion is calculated from labile and cellulose */
	t2 = 1 - t1;

        /* test for fractions sum to 1.0 */
	if (ok) diff=1.0 - (t1+t2);

	if (ok && (fabs(diff) > 1e-6))
	{
		printf("Error in fractions: %.2f %.2f %.3f\n",t1, t2, t1+t2);
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

	if (ok && scan_value(temp, &epc->int_coef, 'd'))
	{
		printf("Error reading canopy water int coef, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->ext_coef, 'd'))
	{
		printf("Error reading canopy light ext.coef, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->lai_ratio, 'd'))
	{
		printf("Error reading all to projected LAI ratio, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->avg_proj_sla, 'd'))
	{
		printf("Error reading canopy average projected specific leaf area, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->sla_ratio, 'd'))
	{
		printf("Error reading shaded to sunlit SLA ratio, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->flnr, 'd'))
	{
		printf("Error reading Rubisco N fraction, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->flnp, 'd'))
	{
		printf("Error reading PeP N fraction, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->gl_smax, 'd'))
	{
		printf("Error reading gl_smax, epc_init()\n");
		ok=0;
	}

	/* ------------------------------------------------------ */
	/* Hidy 2013 - using varying maximum stomatal conductance values */


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
			printf("Error allocating for annual MSC array, epc_init()\n");
			ok=0;
		}

		/* read year and co2 concentration for each simyear */
		for (i=0 ; ok && i<ctrl->simyears ; i++)
		{
			if (fscanf(msc_file.ptr,"%*i%lf", &(epc->msc_array[i]))==EOF)
			{
				printf("Error reading annual MSC array, epc_init()\n");
				printf("Note: file must contain a pair of values for each\n");
				printf("simyear: year and MSC.\n");
				ok=0;
			}
			if (epc->msc_array[i] < 0.0)
			{
				printf("Error in epc_init(): msc must be positive\n");
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
		printf("Error reading gl_c, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->gl_bl, 'd'))
	{
		printf("Error reading gl_bl, epc_init()\n");
		ok=0;
	}
    /******************************************************************/
	/* Hidy 2012: - multiplier for conductance limitation */
	
	if (ok && scan_value(temp, &epc->relVWC_crit1, 'd'))
	{
		printf("Error reading relVWC_crit1, epc_init()\n");
		ok=0;
	}
	
    if (ok && scan_value(temp, &epc->relVWC_crit2, 'd'))
	{
		printf("Error reading relVWC_crit2, epc_init()\n");
		ok=0;
	}

	/* -------------------------------------------*/
	/* multiplier for PSI */
	if (ok && scan_value(temp, &epc->PSI_crit1, 'd'))
	{
		printf("Error reading PSI_crit1, epc_init()\n");
		ok=0;
	}
	
    if (ok && scan_value(temp, &epc->PSI_crit2, 'd'))
	{
		printf("Error reading PSI_crit2, epc_init()\n");
		ok=0;
	}


	/******************************************************************/

	if (ok && scan_value(temp, &epc->vpd_open, 'd')) 
	{
		printf("Error reading vpd_max, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->vpd_close, 'd'))
	{
		printf("Error reading vpd_min, epc_init()\n");
		ok=0;
	}
	
	
	/* -------------------------------------------*/
	/* Hidy 2011 - plant wilting mortality parameter */
	if (ok && scan_value(temp, &epc->mort_SNSC_abovebiom, 'd'))
	{
		printf("Error reading senescence mortality parameter of aboveground biomass: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->mort_SNSC_belowbiom, 'd'))
	{
		printf("Error reading senescence mortality parameter of belowground biomass: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->mort_SNSC_leafphen, 'd'))
	{
		printf("Error reading leaf phenology mortality parameter: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->mort_SNSC_to_litter, 'd'))
	{
		printf("Error reading turnover rate of wilted standing biomass to litter parameter: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->mort_CnW_to_litter, 'd'))
	{
		printf("Error reading turnover rate of cut-down non-woody biomass to litter parameter: epc_init()\n");
		ok=0;
	}

	/* -------------------------------------------*/
	/* Hidy 2013 - denetirification proportion and mobilen proportion
		original: BBGC constant - new version: can be set in EPC file*/

	if (ok && scan_value(temp, &epc->denitrif_prop, 'd'))
	{
		printf("Error reading denitrif_prop: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->bulkN_denitrif_prop_WET, 'd'))
	{
		printf("Error reading denitrif_prop: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->bulkN_denitrif_prop_DRY, 'd'))
	{
		printf("Error reading bulkN_denitrif_prop: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->mobilen_prop, 'd'))
	{
		printf("Error reading mobilen_prop: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->nfix, 'd'))
	{
		printf("Error reading N fixation, epc_init()\n");
		ok=0;
	}

     if (ok && scan_value(temp, &epc->storage_MGMmort, 'd'))
	{
		printf("Error reading storage_MGMmort: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->m_soilstress_crit, 'd'))
	{
		printf("Error reading m_soilstress_crit: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->n_stressdays_crit, 'i'))
	{
		printf("Error reading n_stressdays_crit: epc_init()\n");
		ok=0;
	}

	/* max_rootzone_depth - Hidy 2014 */
	if (ok && scan_value(temp, &epc->max_rootzone_depth, 'd'))
	{
		printf("Error reading number of max_rootzone_depth\n");
		ok=0;
	}

	/* CONTROL to avoid negative  data */
 	if (epc->max_rootzone_depth <= 0)
	{
		printf("Error in epc data in EPC file: negative or zero rootzone_depth, epc_init()\n");
		ok=0;
	}
	

	if (ok && scan_value(temp, &epc->rootdistrib_param, 'd'))
	{
		printf("Error reading rootdistrib_param: epc_init()\n");
		ok=0;
	}
	
	if (ok && scan_value(temp, &epc->maturity_coeff, 'd'))
	{
		printf("Error reading maturity_coeff: epc_init()\n");
		ok=0;
	}

	
	if (ok && scan_value(temp, &epc->GR_ratio, 'd'))
	{
		printf("Error reading growth resp.ratio: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->c_param_tsoil, 'd'))
	{
		printf("Error reading c_param_tsoil: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->mrpern, 'd'))
	{
		printf("Error reading mrpern: epc_init()\n");
		ok=0;
	}

	/* respiration fractions for fluxes between compartments - Hidy 2015 */
	if (ok && scan_value(temp, &epc->rfl1s1, 'd'))
	{
		printf("Error reading rfl1s1: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->rfl2s2, 'd'))
	{
		printf("Error reading rfl2s2: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->rfl4s3, 'd'))
	{
		printf("Error reading rfl4s3: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->rfs1s2, 'd'))
	{
		printf("Error reading rfs1s2: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->rfs2s3, 'd'))
	{
		printf("Error reading rfs2s3: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->rfs3s4, 'd'))
	{
		printf("Error reading rfs3s4: epc_init()\n");
		ok=0;
	}

	/* 	base values of rate constants are (1/day) - Hidy 2015  */
	if (ok && scan_value(temp, &epc->kl1_base, 'd'))
	{
		printf("Error reading kl1_base: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->kl2_base, 'd'))
	{
		printf("Error reading kl2_base: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->kl4_base, 'd'))
	{
		printf("Error reading kl4_base: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->ks1_base, 'd'))
	{
		printf("Error reading ks1_base: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->ks2_base, 'd'))
	{
		printf("Error reading ks2_base: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->ks3_base, 'd'))
	{
		printf("Error reading ks3_base: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->ks4_base, 'd'))
	{
		printf("Error reading ks4_base: epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->kfrag_base, 'd'))
	{
		printf("Error reading kfrag_base: epc_init()\n");
		ok=0;
	}

	/* empirical N2O and CH4 modeling - Hidy 2015 */
	if (ok && scan_value(temp, &epc->N_pCNR1, 'd'))
	{
		printf("Error reading N_pCNR1: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->N_pCNR2, 'd'))
	{
		printf("Error reading N_pCNR2: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->N_pVWC1, 'd'))
	{
		printf("Error reading N_pVWC1: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->N_pVWC2, 'd'))
	{
		printf("Error reading N_pVWC2: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->N_pVWC3, 'd'))
	{
		printf("Error reading N_pVWC3: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->N_pVWC4, 'd'))
	{
		printf("Error reading N_pVWC4: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->N_pTS, 'd'))
	{
		printf("Error reading N_pTS: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->C_pBD1, 'd'))
	{
		printf("Error reading C_pBD1: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->C_pBD2, 'd'))
	{
		printf("Error reading C_pBD2: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->C_pVWC1, 'd'))
	{
		printf("Error reading C_pVWC1: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->C_pVWC2, 'd'))
	{
		printf("Error reading C_pVWC2: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->C_pVWC3, 'd'))
	{
		printf("Error reading C_pVWC3: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->C_pVWC4, 'd'))
	{
		printf("Error reading C_pVWC4: epc_init()\n");
		ok=0;
	}

	if (ok && scan_value(temp, &epc->C_pTS, 'd'))
	{
		printf("Error reading C_pTS: epc_init()\n");
		ok=0;
	}
	
	/* -------------------------------------------*/
	if (dofileclose) fclose(temp.ptr);
		
	return (!ok);
}
