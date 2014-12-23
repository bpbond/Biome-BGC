/*
output_map.c
defines an array of pointers to doubles that map to all the intermediate
variables in bgc

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information

Revisions from version 4.1.1:
Added cpool_*_storage_gr fluxes to the cf block.
Added fpi to the epv block
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include "bgc.h"

int output_map_init(double** output_map, metvar_struct* metv, wstate_struct* ws,
wflux_struct* wf, cstate_struct* cs, cflux_struct* cf, nstate_struct* ns,
nflux_struct* nf, phenology_struct* phen, epvar_struct* epv,
psn_struct* psn_sun, psn_struct* psn_shade, summary_struct* summary)
{
	int ok=1;
	int i;
	
	for (i=0 ; ok && i<NMAP ; i++)
	{
		output_map[i] = NULL;
	}
	
	if (ok)
	{
		/* daily meteorological variables */
		output_map[0] = &metv->prcp;
		output_map[1] = &metv->tmax;
		output_map[2] = &metv->tmin;
		output_map[3] = &metv->tavg;
		output_map[4] = &metv->tday;
		output_map[5] = &metv->tnight;
		output_map[6] = &metv->tsoil;
		output_map[7] = &metv->vpd;
		output_map[8] = &metv->swavgfd;
		output_map[9] = &metv->swabs;
		output_map[10] = &metv->swtrans;
		output_map[11] = &metv->swabs_per_plaisun;
		output_map[12] = &metv->swabs_per_plaishade;
		output_map[13] = &metv->ppfd_per_plaisun;
		output_map[14] = &metv->ppfd_per_plaishade;
		output_map[15] = &metv->par;
		output_map[16] = &metv->parabs;
		output_map[17] = &metv->pa;
		output_map[18] = &metv->co2;
		output_map[19] = &metv->dayl;
		
		/* water state variables */
		output_map[20] = &ws->soilw;
		output_map[21] = &ws->snoww;
		output_map[22] = &ws->canopyw;
		output_map[23] = &ws->prcp_src;
		output_map[24] = &ws->outflow_snk;
		output_map[25] = &ws->soilevap_snk;
		output_map[26] = &ws->snowsubl_snk;
		output_map[27] = &ws->canopyevap_snk;
		output_map[28] = &ws->trans_snk;
		
		/* water flux variables */
		output_map[35] = &wf->prcp_to_canopyw;
		output_map[36] = &wf->prcp_to_soilw;
		output_map[37] = &wf->prcp_to_snoww;
		output_map[38] = &wf->canopyw_evap;
		output_map[39] = &wf->canopyw_to_soilw;
		output_map[40] = &wf->snoww_subl;
		output_map[41] = &wf->snoww_to_soilw;
		output_map[42] = &wf->soilw_evap;
		output_map[43] = &wf->soilw_trans;
		output_map[44] = &wf->soilw_outflow;
		
		/* carbon state variables */
		output_map[50] = &cs->leafc;
		output_map[51] = &cs->leafc_storage;
		output_map[52] = &cs->leafc_transfer;
		output_map[53] = &cs->frootc;
		output_map[54] = &cs->frootc_storage;
		output_map[55] = &cs->frootc_transfer;
		output_map[56] = &cs->livestemc;
		output_map[57] = &cs->livestemc_storage;
		output_map[58] = &cs->livestemc_transfer;
		output_map[59] = &cs->deadstemc;
		output_map[60] = &cs->deadstemc_storage;
		output_map[61] = &cs->deadstemc_transfer;
		output_map[62] = &cs->livecrootc;
		output_map[63] = &cs->livecrootc_storage;
		output_map[64] = &cs->livecrootc_transfer;
		output_map[65] = &cs->deadcrootc;
		output_map[66] = &cs->deadcrootc_storage;
		output_map[67] = &cs->deadcrootc_transfer;
		output_map[68] = &cs->gresp_storage;
		output_map[69] = &cs->gresp_transfer;
		output_map[70] = &cs->cwdc;
		output_map[71] = &cs->litr1c;
		output_map[72] = &cs->litr2c;
		output_map[73] = &cs->litr3c;
		output_map[74] = &cs->litr4c;
		output_map[75] = &cs->soil1c;
		output_map[76] = &cs->soil2c;
		output_map[77] = &cs->soil3c;
		output_map[78] = &cs->soil4c;
		output_map[79] = &cs->cpool;
		output_map[80] = &cs->psnsun_src;
		output_map[81] = &cs->psnshade_src;
		output_map[82] = &cs->leaf_mr_snk;
		output_map[83] = &cs->leaf_gr_snk;
		output_map[84] = &cs->froot_mr_snk;
		output_map[85] = &cs->froot_gr_snk;
		output_map[86] = &cs->livestem_mr_snk;
		output_map[87] = &cs->livestem_gr_snk;
		output_map[88] = &cs->deadstem_gr_snk;
		output_map[89] = &cs->livecroot_mr_snk;
		output_map[90] = &cs->livecroot_gr_snk;
		output_map[91] = &cs->deadcroot_gr_snk;
		output_map[92] = &cs->litr1_hr_snk;
		output_map[93] = &cs->litr2_hr_snk;
		output_map[94] = &cs->litr4_hr_snk;
		output_map[95] = &cs->soil1_hr_snk;
		output_map[96] = &cs->soil2_hr_snk;
		output_map[97] = &cs->soil3_hr_snk;
		output_map[98] = &cs->soil4_hr_snk;
		output_map[99] = &cs->fire_snk;
		
		/* carbon flux variables */
		output_map[120] = &cf->m_leafc_to_litr1c;
		output_map[121] = &cf->m_leafc_to_litr2c;
		output_map[122] = &cf->m_leafc_to_litr3c;
		output_map[123] = &cf->m_leafc_to_litr4c;
		output_map[124] = &cf->m_frootc_to_litr1c;
		output_map[125] = &cf->m_frootc_to_litr2c;
		output_map[126] = &cf->m_frootc_to_litr3c;
		output_map[127] = &cf->m_frootc_to_litr4c;
		output_map[128] = &cf->m_leafc_storage_to_litr1c;
		output_map[129] = &cf->m_frootc_storage_to_litr1c;
		output_map[130] = &cf->m_livestemc_storage_to_litr1c;
		output_map[131] = &cf->m_deadstemc_storage_to_litr1c;
		output_map[132] = &cf->m_livecrootc_storage_to_litr1c;
		output_map[133] = &cf->m_deadcrootc_storage_to_litr1c;
		output_map[134] = &cf->m_leafc_transfer_to_litr1c;
		output_map[135] = &cf->m_frootc_transfer_to_litr1c;
		output_map[136] = &cf->m_livestemc_transfer_to_litr1c;
		output_map[137] = &cf->m_deadstemc_transfer_to_litr1c;
		output_map[138] = &cf->m_livecrootc_transfer_to_litr1c;
		output_map[139] = &cf->m_deadcrootc_transfer_to_litr1c;
		output_map[140] = &cf->m_livestemc_to_cwdc;
		output_map[141] = &cf->m_deadstemc_to_cwdc;
		output_map[142] = &cf->m_livecrootc_to_cwdc;
		output_map[143] = &cf->m_deadcrootc_to_cwdc;
		output_map[144] = &cf->m_gresp_storage_to_litr1c;
		output_map[145] = &cf->m_gresp_transfer_to_litr1c;
		output_map[146] = &cf->m_leafc_to_fire;
		output_map[147] = &cf->m_frootc_to_fire;
		output_map[148] = &cf->m_leafc_storage_to_fire;
		output_map[149] = &cf->m_frootc_storage_to_fire;
		output_map[150] = &cf->m_livestemc_storage_to_fire;
		output_map[151] = &cf->m_deadstemc_storage_to_fire;
		output_map[152] = &cf->m_livecrootc_storage_to_fire;
		output_map[153] = &cf->m_deadcrootc_storage_to_fire;
		output_map[154] = &cf->m_leafc_transfer_to_fire;
		output_map[155] = &cf->m_frootc_transfer_to_fire;
		output_map[156] = &cf->m_livestemc_transfer_to_fire;
		output_map[157] = &cf->m_deadstemc_transfer_to_fire;
		output_map[158] = &cf->m_livecrootc_transfer_to_fire;
		output_map[159] = &cf->m_deadcrootc_transfer_to_fire;
		output_map[160] = &cf->m_livestemc_to_fire;
		output_map[161] = &cf->m_deadstemc_to_fire;
		output_map[162] = &cf->m_livecrootc_to_fire;
		output_map[163] = &cf->m_deadcrootc_to_fire;
		output_map[164] = &cf->m_gresp_storage_to_fire;
		output_map[165] = &cf->m_gresp_transfer_to_fire;
		output_map[166] = &cf->m_litr1c_to_fire;
		output_map[167] = &cf->m_litr2c_to_fire;
		output_map[168] = &cf->m_litr3c_to_fire;
		output_map[169] = &cf->m_litr4c_to_fire;
		output_map[170] = &cf->m_cwdc_to_fire;
		output_map[171] = &cf->leafc_transfer_to_leafc;
		output_map[172] = &cf->frootc_transfer_to_frootc;
		output_map[173] = &cf->livestemc_transfer_to_livestemc;
		output_map[174] = &cf->deadstemc_transfer_to_deadstemc;
		output_map[175] = &cf->livecrootc_transfer_to_livecrootc;
		output_map[176] = &cf->deadcrootc_transfer_to_deadcrootc;
		output_map[177] = &cf->leafc_to_litr1c;
		output_map[178] = &cf->leafc_to_litr2c;
		output_map[179] = &cf->leafc_to_litr3c;
		output_map[180] = &cf->leafc_to_litr4c;
		output_map[181] = &cf->frootc_to_litr1c;
		output_map[182] = &cf->frootc_to_litr2c;
		output_map[183] = &cf->frootc_to_litr3c;
		output_map[184] = &cf->frootc_to_litr4c;
		output_map[185] = &cf->leaf_day_mr;
		output_map[186] = &cf->leaf_night_mr;
		output_map[187] = &cf->froot_mr;
		output_map[188] = &cf->livestem_mr;
		output_map[189] = &cf->livecroot_mr;
		output_map[190] = &cf->psnsun_to_cpool;
		output_map[191] = &cf->psnshade_to_cpool;
		output_map[192] = &cf->cwdc_to_litr2c;
		output_map[193] = &cf->cwdc_to_litr3c;
		output_map[194] = &cf->cwdc_to_litr4c;
		output_map[195] = &cf->litr1_hr;
		output_map[196] = &cf->litr1c_to_soil1c;
		output_map[197] = &cf->litr2_hr;
		output_map[198] = &cf->litr2c_to_soil2c;
		output_map[199] = &cf->litr3c_to_litr2c;
		output_map[200] = &cf->litr4_hr;
		output_map[201] = &cf->litr4c_to_soil3c;
		output_map[202] = &cf->soil1_hr;
		output_map[203] = &cf->soil1c_to_soil2c;
		output_map[204] = &cf->soil2_hr;
		output_map[205] = &cf->soil2c_to_soil3c;
		output_map[206] = &cf->soil3_hr;
		output_map[207] = &cf->soil3c_to_soil4c;
		output_map[208] = &cf->soil4_hr;
		output_map[209] = &cf->cpool_to_leafc;
		output_map[210] = &cf->cpool_to_leafc_storage;
		output_map[211] = &cf->cpool_to_frootc;
		output_map[212] = &cf->cpool_to_frootc_storage;
		output_map[213] = &cf->cpool_to_livestemc;
		output_map[214] = &cf->cpool_to_livestemc_storage;
		output_map[215] = &cf->cpool_to_deadstemc;
		output_map[216] = &cf->cpool_to_deadstemc_storage;
		output_map[217] = &cf->cpool_to_livecrootc;
		output_map[218] = &cf->cpool_to_livecrootc_storage;
		output_map[219] = &cf->cpool_to_deadcrootc;
		output_map[220] = &cf->cpool_to_deadcrootc_storage;
		output_map[221] = &cf->cpool_to_gresp_storage;
		output_map[222] = &cf->cpool_leaf_gr;
		output_map[223] = &cf->transfer_leaf_gr;
		output_map[224] = &cf->cpool_froot_gr;
		output_map[225] = &cf->transfer_froot_gr;
		output_map[226] = &cf->cpool_livestem_gr;
		output_map[227] = &cf->transfer_livestem_gr;
		output_map[228] = &cf->cpool_deadstem_gr;
		output_map[229] = &cf->transfer_deadstem_gr;
		output_map[230] = &cf->cpool_livecroot_gr;
		output_map[231] = &cf->transfer_livecroot_gr;
		output_map[232] = &cf->cpool_deadcroot_gr;
		output_map[233] = &cf->transfer_deadcroot_gr;
		output_map[234] = &cf->leafc_storage_to_leafc_transfer;
		output_map[235] = &cf->frootc_storage_to_frootc_transfer;
		output_map[236] = &cf->livestemc_storage_to_livestemc_transfer;
		output_map[237] = &cf->deadstemc_storage_to_deadstemc_transfer;
		output_map[238] = &cf->livecrootc_storage_to_livecrootc_transfer;
		output_map[239] = &cf->deadcrootc_storage_to_deadcrootc_transfer;
		output_map[240] = &cf->gresp_storage_to_gresp_transfer;
		output_map[241] = &cf->livestemc_to_deadstemc;
		output_map[242] = &cf->livecrootc_to_deadcrootc;
		output_map[243] = &cf->cpool_leaf_storage_gr;
		output_map[244] = &cf->cpool_froot_storage_gr;
		output_map[245] = &cf->cpool_livestem_storage_gr;
		output_map[246] = &cf->cpool_deadstem_storage_gr;
		output_map[247] = &cf->cpool_livecroot_storage_gr;
		output_map[248] = &cf->cpool_deadcroot_storage_gr;
		
		/* nitrogen state variables */
		output_map[280] = &ns->leafn;
		output_map[281] = &ns->leafn_storage;
		output_map[282] = &ns->leafn_transfer;
		output_map[283] = &ns->frootn;
		output_map[284] = &ns->frootn_storage;
		output_map[285] = &ns->frootn_transfer;
		output_map[286] = &ns->livestemn;
		output_map[287] = &ns->livestemn_storage;
		output_map[288] = &ns->livestemn_transfer;
		output_map[289] = &ns->deadstemn;
		output_map[290] = &ns->deadstemn_storage;
		output_map[291] = &ns->deadstemn_transfer;
		output_map[292] = &ns->livecrootn;
		output_map[293] = &ns->livecrootn_storage;
		output_map[294] = &ns->livecrootn_transfer;
		output_map[295] = &ns->deadcrootn;
		output_map[296] = &ns->deadcrootn_storage;
		output_map[297] = &ns->deadcrootn_transfer;
		output_map[298] = &ns->cwdn;
		output_map[299] = &ns->litr1n;
		output_map[300] = &ns->litr2n;
		output_map[301] = &ns->litr3n;
		output_map[302] = &ns->litr4n;
		output_map[303] = &ns->soil1n;
		output_map[304] = &ns->soil2n;
		output_map[305] = &ns->soil3n;
		output_map[306] = &ns->soil4n;
		output_map[307] = &ns->sminn;
		output_map[308] = &ns->retransn;
		output_map[309] = &ns->npool;
		output_map[310] = &ns->nfix_src;
		output_map[311] = &ns->ndep_src;
		output_map[312] = &ns->nleached_snk;
		output_map[313] = &ns->nvol_snk;
		output_map[314] = &ns->fire_snk;
		
		/* nitrogen flux variables */
		output_map[340] = &nf->m_leafn_to_litr1n;
		output_map[341] = &nf->m_leafn_to_litr2n;
		output_map[342] = &nf->m_leafn_to_litr3n;
		output_map[343] = &nf->m_leafn_to_litr4n;
		output_map[344] = &nf->m_frootn_to_litr1n;
		output_map[345] = &nf->m_frootn_to_litr2n;
		output_map[346] = &nf->m_frootn_to_litr3n;
		output_map[347] = &nf->m_frootn_to_litr4n;
		output_map[348] = &nf->m_leafn_storage_to_litr1n;
		output_map[349] = &nf->m_frootn_storage_to_litr1n;
		output_map[350] = &nf->m_livestemn_storage_to_litr1n;
		output_map[351] = &nf->m_deadstemn_storage_to_litr1n;
		output_map[352] = &nf->m_livecrootn_storage_to_litr1n;
		output_map[353] = &nf->m_deadcrootn_storage_to_litr1n;
		output_map[354] = &nf->m_leafn_transfer_to_litr1n;
		output_map[355] = &nf->m_frootn_transfer_to_litr1n;
		output_map[356] = &nf->m_livestemn_transfer_to_litr1n;
		output_map[357] = &nf->m_deadstemn_transfer_to_litr1n;
		output_map[358] = &nf->m_livecrootn_transfer_to_litr1n;
		output_map[359] = &nf->m_deadcrootn_transfer_to_litr1n;
		output_map[360] = &nf->m_livestemn_to_litr1n;
		output_map[361] = &nf->m_livestemn_to_cwdn;
		output_map[362] = &nf->m_deadstemn_to_cwdn;
		output_map[363] = &nf->m_livecrootn_to_litr1n;
		output_map[364] = &nf->m_livecrootn_to_cwdn;
		output_map[365] = &nf->m_deadcrootn_to_cwdn;
		output_map[366] = &nf->m_retransn_to_litr1n;
		output_map[367] = &nf->m_leafn_to_fire;
		output_map[368] = &nf->m_frootn_to_fire;
		output_map[369] = &nf->m_leafn_storage_to_fire;
		output_map[370] = &nf->m_frootn_storage_to_fire;
		output_map[371] = &nf->m_livestemn_storage_to_fire;
		output_map[372] = &nf->m_deadstemn_storage_to_fire;
		output_map[373] = &nf->m_livecrootn_storage_to_fire;
		output_map[374] = &nf->m_deadcrootn_storage_to_fire;
		output_map[375] = &nf->m_leafn_transfer_to_fire;
		output_map[376] = &nf->m_frootn_transfer_to_fire;
		output_map[377] = &nf->m_livestemn_transfer_to_fire;
		output_map[378] = &nf->m_deadstemn_transfer_to_fire;
		output_map[379] = &nf->m_livecrootn_transfer_to_fire;
		output_map[380] = &nf->m_deadcrootn_transfer_to_fire;
		output_map[381] = &nf->m_livestemn_to_fire;
		output_map[382] = &nf->m_deadstemn_to_fire;
		output_map[383] = &nf->m_livecrootn_to_fire;
		output_map[384] = &nf->m_deadcrootn_to_fire;
		output_map[385] = &nf->m_retransn_to_fire;
		output_map[386] = &nf->m_litr1n_to_fire;
		output_map[387] = &nf->m_litr2n_to_fire;
		output_map[388] = &nf->m_litr3n_to_fire;
		output_map[389] = &nf->m_litr4n_to_fire;
		output_map[390] = &nf->m_cwdn_to_fire;
		output_map[391] = &nf->leafn_transfer_to_leafn;
		output_map[392] = &nf->frootn_transfer_to_frootn;
		output_map[393] = &nf->livestemn_transfer_to_livestemn;
		output_map[394] = &nf->deadstemn_transfer_to_deadstemn;
		output_map[395] = &nf->livecrootn_transfer_to_livecrootn;
		output_map[396] = &nf->deadcrootn_transfer_to_deadcrootn;
		output_map[397] = &nf->leafn_to_litr1n;
		output_map[398] = &nf->leafn_to_litr2n;
		output_map[399] = &nf->leafn_to_litr3n;
		output_map[400] = &nf->leafn_to_litr4n;
		output_map[401] = &nf->leafn_to_retransn;
		output_map[402] = &nf->frootn_to_litr1n;
		output_map[403] = &nf->frootn_to_litr2n;
		output_map[404] = &nf->frootn_to_litr3n;
		output_map[405] = &nf->frootn_to_litr4n;
		output_map[406] = &nf->ndep_to_sminn;
		output_map[407] = &nf->nfix_to_sminn;
		output_map[408] = &nf->cwdn_to_litr2n;
		output_map[409] = &nf->cwdn_to_litr3n;
		output_map[410] = &nf->cwdn_to_litr4n;
		output_map[411] = &nf->litr1n_to_soil1n;
		output_map[412] = &nf->sminn_to_soil1n_l1;
		output_map[413] = &nf->litr2n_to_soil2n;
		output_map[414] = &nf->sminn_to_soil2n_l2;
		output_map[415] = &nf->litr3n_to_litr2n;
		output_map[416] = &nf->litr4n_to_soil3n;
		output_map[417] = &nf->sminn_to_soil3n_l4;
		output_map[418] = &nf->soil1n_to_soil2n;
		output_map[419] = &nf->sminn_to_soil2n_s1;
		output_map[420] = &nf->soil2n_to_soil3n;
		output_map[421] = &nf->sminn_to_soil3n_s2;
		output_map[422] = &nf->soil3n_to_soil4n;
		output_map[423] = &nf->sminn_to_soil4n_s3;
		output_map[424] = &nf->soil4n_to_sminn;
		output_map[425] = &nf->sminn_to_nvol_l1s1;
		output_map[426] = &nf->sminn_to_nvol_l2s2;
		output_map[427] = &nf->sminn_to_nvol_l4s3;
		output_map[428] = &nf->sminn_to_nvol_s1s2;
		output_map[429] = &nf->sminn_to_nvol_s2s3;
		output_map[430] = &nf->sminn_to_nvol_s3s4;
		output_map[431] = &nf->sminn_to_nvol_s4;
		output_map[432] = &nf->sminn_leached;
		output_map[433] = &nf->retransn_to_npool;
		output_map[434] = &nf->sminn_to_npool;
		output_map[435] = &nf->npool_to_leafn;
		output_map[436] = &nf->npool_to_leafn_storage;
		output_map[437] = &nf->npool_to_frootn;
		output_map[438] = &nf->npool_to_frootn_storage;
		output_map[439] = &nf->npool_to_livestemn;
		output_map[440] = &nf->npool_to_livestemn_storage;
		output_map[441] = &nf->npool_to_deadstemn;
		output_map[442] = &nf->npool_to_deadstemn_storage;
		output_map[443] = &nf->npool_to_livecrootn;
		output_map[444] = &nf->npool_to_livecrootn_storage;
		output_map[445] = &nf->npool_to_deadcrootn;
		output_map[446] = &nf->npool_to_deadcrootn_storage;
		output_map[447] = &nf->leafn_storage_to_leafn_transfer;
		output_map[448] = &nf->frootn_storage_to_frootn_transfer;
		output_map[449] = &nf->livestemn_storage_to_livestemn_transfer;
		output_map[450] = &nf->deadstemn_storage_to_deadstemn_transfer;
		output_map[451] = &nf->livecrootn_storage_to_livecrootn_transfer;
		output_map[452] = &nf->deadcrootn_storage_to_deadcrootn_transfer;
		output_map[453] = &nf->livestemn_to_deadstemn;
		output_map[454] = &nf->livestemn_to_retransn;
		output_map[455] = &nf->livecrootn_to_deadcrootn;
		output_map[456] = &nf->livecrootn_to_retransn;
		
		/* phenological variables */
		output_map[480] = &phen->remdays_curgrowth;
		output_map[481] = &phen->remdays_transfer;
		output_map[482] = &phen->remdays_litfall;
		output_map[483] = &phen->predays_transfer;
		output_map[484] = &phen->predays_litfall;
		
		/* ecophysiological variables */
		output_map[500] = &epv->day_leafc_litfall_increment;
		output_map[501] = &epv->day_frootc_litfall_increment;
		output_map[502] = &epv->day_livestemc_turnover_increment;
		output_map[503] = &epv->day_livecrootc_turnover_increment;
		output_map[504] = &epv->annmax_leafc;
		output_map[505] = &epv->annmax_frootc;
		output_map[506] = &epv->annmax_livestemc;
		output_map[507] = &epv->annmax_livecrootc;
		output_map[508] = &epv->dsr;
		output_map[509] = &epv->proj_lai;
		output_map[510] = &epv->all_lai;
		output_map[511] = &epv->plaisun;
		output_map[512] = &epv->plaishade;
		output_map[513] = &epv->sun_proj_sla;
		output_map[514] = &epv->shade_proj_sla;
		output_map[515] = &epv->psi;
		output_map[516] = &epv->vwc;
		output_map[517] = &epv->dlmr_area_sun;
		output_map[518] = &epv->dlmr_area_shade;
		output_map[519] = &epv->gl_t_wv_sun;
		output_map[520] = &epv->gl_t_wv_shade;
		output_map[521] = &epv->assim_sun;
		output_map[522] = &epv->assim_shade;
		output_map[523] = &epv->t_scalar;
		output_map[524] = &epv->w_scalar;
		output_map[525] = &epv->rate_scalar;
		output_map[526] = &epv->daily_gross_nmin;
		output_map[527] = &epv->daily_gross_nimmob;
		output_map[528] = &epv->daily_net_nmin;
		output_map[529] = &epv->m_tmin;
		output_map[530] = &epv->m_psi;
		output_map[531] = &epv->m_co2;
		output_map[532] = &epv->m_ppfd_sun;
		output_map[533] = &epv->m_ppfd_shade;
		output_map[534] = &epv->m_vpd;
		output_map[535] = &epv->m_final_sun;
		output_map[536] = &epv->m_final_shade;
		output_map[537] = &epv->gl_bl;
		output_map[538] = &epv->gl_c;
		output_map[539] = &epv->gl_s_sun;
		output_map[540] = &epv->gl_s_shade;
		output_map[541] = &epv->gl_e_wv;
		output_map[542] = &epv->gl_sh;
		output_map[543] = &epv->gc_e_wv;
		output_map[544] = &epv->gc_sh;
		output_map[545] = &epv->ytd_maxplai;
		output_map[546] = &epv->fpi;
				
		/* photosynthesis variables */
		/* sunlit canopy fraction */
		output_map[560] = &psn_sun->pa;
		output_map[561] = &psn_sun->co2;
		output_map[562] = &psn_sun->t;
		output_map[563] = &psn_sun->lnc;
		output_map[564] = &psn_sun->flnr;
		output_map[565] = &psn_sun->ppfd;
		output_map[566] = &psn_sun->g;
		output_map[567] = &psn_sun->dlmr;
		output_map[568] = &psn_sun->Ci;
		output_map[569] = &psn_sun->O2;
		output_map[570] = &psn_sun->Ca;
		output_map[571] = &psn_sun->gamma;
		output_map[572] = &psn_sun->Kc;
		output_map[573] = &psn_sun->Ko;
		output_map[574] = &psn_sun->Vmax;
		output_map[575] = &psn_sun->Jmax;
		output_map[576] = &psn_sun->J;
		output_map[577] = &psn_sun->Av;
		output_map[578] = &psn_sun->Aj;
		output_map[579] = &psn_sun->A;
		
		/* photosynthesis variables */
		/* shaded canopy fraction */
		output_map[590] = &psn_shade->pa;
		output_map[591] = &psn_shade->co2;
		output_map[592] = &psn_shade->t;
		output_map[593] = &psn_shade->lnc;
		output_map[594] = &psn_shade->flnr;
		output_map[595] = &psn_shade->ppfd;
		output_map[596] = &psn_shade->g;
		output_map[597] = &psn_shade->dlmr;
		output_map[598] = &psn_shade->Ci;
		output_map[599] = &psn_shade->O2;
		output_map[600] = &psn_shade->Ca;
		output_map[601] = &psn_shade->gamma;
		output_map[602] = &psn_shade->Kc;
		output_map[603] = &psn_shade->Ko;
		output_map[604] = &psn_shade->Vmax;
		output_map[605] = &psn_shade->Jmax;
		output_map[606] = &psn_shade->J;
		output_map[607] = &psn_shade->Av;
		output_map[608] = &psn_shade->Aj;
		output_map[609] = &psn_shade->A;
		
		/* carbon budget summary output variables */
		output_map[620] = &summary->daily_npp;
		output_map[621] = &summary->daily_nep;
		output_map[622] = &summary->daily_nee;
		output_map[623] = &summary->daily_gpp;
		output_map[624] = &summary->daily_mr;
		output_map[625] = &summary->daily_gr;
		output_map[626] = &summary->daily_hr;
		output_map[627] = &summary->daily_fire;
		output_map[628] = &summary->cum_npp;
		output_map[629] = &summary->cum_nep;
		output_map[630] = &summary->cum_nee;
		output_map[631] = &summary->cum_gpp;
		output_map[632] = &summary->cum_mr;
		output_map[633] = &summary->cum_gr;
		output_map[634] = &summary->cum_hr;
		output_map[635] = &summary->cum_fire;
		output_map[636] = &summary->vegc;
		output_map[637] = &summary->litrc;
		output_map[638] = &summary->soilc;
		output_map[639] = &summary->totalc;
		output_map[640] = &summary->daily_litfallc;
		output_map[641] = &summary->daily_et;
		output_map[642] = &summary->daily_outflow;
		output_map[643] = &summary->daily_evap;
		output_map[644] = &summary->daily_trans;
		output_map[645] = &summary->daily_soilw;
		output_map[646] = &summary->daily_snoww;
	}
	
	return (!ok);
}
		
