/* 
epc_init.c
read epc file for pointbgc simulation

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "pointbgc.h"


int epc_init(file init, epconst_struct* epc)
{
	int ok = 1;
	double t1,t2,t3,t4,r1;
	file temp;
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
		bgc_printf(BV_ERROR, "Error reading keyword for control data\n");
		ok=0;
	}
	if (ok && strcmp(keyword, key1))
	{
		bgc_printf(BV_ERROR, "Expecting keyword --> %s in file %s\n",key1,init.name);
		ok=0;
	}

	/* open file  */
	if (ok && scan_open(init,&temp,'r')) 
	{
		bgc_printf(BV_ERROR, "Error opening epconst file, epc_init()\n");
		ok=0;
	}
	
	/* first scan epc keyword to ensure proper *.init format */
	if (ok && scan_value(temp, keyword, 's'))
	{
		bgc_printf(BV_ERROR, "Error reading keyword, epc_init()\n");
		ok=0;
	}
	if (ok && strcmp(keyword,key2))
	{
		bgc_printf(BV_ERROR, "Expecting keyword --> %s in %s\n",key2,init.name);
		ok=0;
	}
	
	/* begin reading constants from *.init */
	if (ok && scan_value(temp, &epc->woody, 'i'))
	{
		bgc_printf(BV_ERROR, "Error reading woody/non-woody flag, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->evergreen, 'i'))
	{
		bgc_printf(BV_ERROR, "Error reading evergreen/deciduous flag, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->c3_flag, 'i'))
	{
		bgc_printf(BV_ERROR, "Error reading C3/C4 flag, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->phenology_flag, 'i'))
	{
		bgc_printf(BV_ERROR, "Error reading phenology flag, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->onday, 'i'))
	{
		bgc_printf(BV_ERROR, "Error reading onday, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->offday, 'i'))
	{
		bgc_printf(BV_ERROR, "Error reading offday, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->transfer_pdays, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading transfer_pdays, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->litfall_pdays, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading litfall_pdays, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->leaf_turnover, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading leaf turnover, epc_init()\n");
		ok=0;
	}
	/* force leaf turnover fraction to 1.0 if deciduous */
	if (!epc->evergreen)
	{
		epc->leaf_turnover = 1.0;
	}
	epc->froot_turnover = epc->leaf_turnover;
	if (ok && scan_value(temp, &epc->livewood_turnover, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading livewood turnover, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &t1, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading whole-plant mortality, epc_init()\n");
		ok=0;
	}
	epc->daily_mortality_turnover = t1/365;
	if (ok && scan_value(temp, &t1, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading fire mortality, epc_init()\n");
		ok=0;
	}
	epc->daily_fire_turnover = t1/365;
	if (ok && scan_value(temp, &epc->alloc_frootc_leafc, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading froot C:leaf C, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->alloc_newstemc_newleafc, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading new stemC:new leaf C, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->alloc_newlivewoodc_newwoodc, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading new livewood C:new wood C, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->alloc_crootc_stemc, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading croot C:stem C, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->alloc_prop_curgrowth, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading new growth:storage growth, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->leaf_cn, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading average leaf C:N, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->leaflitr_cn, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading leaf litter C:N, epc_init()\n");
		ok=0;
	}
	/* test for leaflitter C:N > leaf C:N */
	if (epc->leaflitr_cn < epc->leaf_cn)
	{
		bgc_printf(BV_ERROR, "Error: leaf litter C:N must be >= leaf C:N\n");
		bgc_printf(BV_ERROR, "change the values in ECOPHYS block of initialization file\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->froot_cn, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading initial fine root C:N, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->livewood_cn, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading initial livewood C:N, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->deadwood_cn, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading initial deadwood C:N, epc_init()\n");
		ok=0;
	}
	/* test for deadwood C:N > livewood C:N */
	if (epc->deadwood_cn < epc->livewood_cn)
	{
		bgc_printf(BV_ERROR, "Error: livewood C:N must be >= deadwood C:N\n");
		bgc_printf(BV_ERROR, "change the values in ECOPHYS block of initialization file\n");
		ok=0;
	}
	if (ok && scan_value(temp, &t1, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading leaf litter labile proportion, epc_init()\n");
		ok=0;
	}
	epc->leaflitr_flab = t1;
	if (ok && scan_value(temp, &t2, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading leaf litter cellulose proportion, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &t3, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading leaf litter lignin proportion, epc_init()\n");
		ok=0;
	}
	epc->leaflitr_flig = t3;

	/* test for litter fractions sum to 1.0 */
	if ( ok && (fabs(t1+t2+t3-1.0) > FLT_COND_TOL) )
	{
		bgc_printf(BV_ERROR, "Error:\n");
		bgc_printf(BV_ERROR, "leaf litter proportions of labile, cellulose, and lignin\n");
		bgc_printf(BV_ERROR, "must sum to 1.0. Check initialization file and try again.\n");
		ok=0;
	}
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
	if (ok && scan_value(temp, &t1, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading froot litter labile proportion, epc_init()\n");
		ok=0;
	}
	epc->frootlitr_flab = t1;
	if (ok && scan_value(temp, &t2, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading froot litter cellulose proportion, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &t3, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading froot litter lignin proportion, epc_init()\n");
		ok=0;
	}
	
	epc->frootlitr_flig = t3;

	/* test for litter fractions sum to 1.0 */
	if ( ok && (fabs(t1+t2+t3-1.0) > FLT_COND_TOL) )
	{
		bgc_printf(BV_ERROR, "Error:\n");
		bgc_printf(BV_ERROR, "froot litter proportions of labile, cellulose, and lignin\n");
		bgc_printf(BV_ERROR, "must sum to 1.0. Check initialization file and try again.\n");
		ok=0;
	}
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
	if (ok && scan_value(temp, &t1, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading dead wood %% cellulose, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &t2, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading dead wood %% lignin, epc_init()\n");
		ok=0;
	}
	epc->deadwood_flig = t2;

	/* test for litter fractions sum to 1.0 */
	if (ok && (fabs(t1+t2-1.0) > FLT_COND_TOL) )
	{
		bgc_printf(BV_ERROR, "Error:\n");
		bgc_printf(BV_ERROR, "deadwood proportions of cellulose and lignin must sum\n");
		bgc_printf(BV_ERROR, "to 1.0. Check initialization file and try again.\n");
		ok=0;
	}
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
		bgc_printf(BV_ERROR, "Error reading canopy water int coef, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->ext_coef, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading canopy light ext coef, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->lai_ratio, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading all to projected LA ratio, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->avg_proj_sla, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading canopy average projected specific leaf area, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->sla_ratio, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading shaded to sunlit SLA ratio, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->flnr, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading Rubisco N fraction, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->gl_smax, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading gl_smax, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->gl_c, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading gl_c, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->gl_bl, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading gl_bl, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->psi_open, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading psi_close, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->psi_close, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading psi_sat, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->vpd_open, 'd')) 
	{
		bgc_printf(BV_ERROR, "Error reading vpd_max, epc_init()\n");
		ok=0;
	}
	if (ok && scan_value(temp, &epc->vpd_close, 'd'))
	{
		bgc_printf(BV_ERROR, "Error reading vpd_min, epc_init()\n");
		ok=0;
	}
	
	fclose(temp.ptr);
		
	return (!ok);
}
