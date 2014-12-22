/*
epclist.h
structure for array of ecophysiological constants structures 

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.1.1
Copyright 2000, Peter E. Thornton
Numerical Terradynamics Simulation Group (NTSG)
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
int epc_init(file init, epconst_struct* epc);
