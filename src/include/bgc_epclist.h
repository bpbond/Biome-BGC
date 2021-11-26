/*
bgc_epclist.h
structure for array of ecophysiological constants structures 

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v6.2.
Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group (NTSG)
School of Forestry, University of Montana
Missoula, MT 59812
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

typedef struct
{
	int nvegtypes;         /* number of vegetation types */
	epconst_struct* epc;   /* pointer to array of epc structures */
} epclist_struct;

/* function prototypes */
int epclist_init(file init, epclist_struct* epclist);
int epc_init(file init, epconst_struct* epc, control_struct* ctrl, int EPCfromINI);
