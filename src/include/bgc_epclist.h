/*
epclist.h
structure for array of ecophysiological constants structures 

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
BBGC MuSo v4
Copyright 2000, Peter E. Thornton
Numerical Terradynamics Simulation Group
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

typedef struct
{
	int nvegtypes;         /* number of vegetation types */
	epconst_struct* epc;   /* pointer to array of epc structures */
} epclist_struct;

/* function prototypes */
int epclist_init(file init, epclist_struct* epclist);
int epc_init(file init, epconst_struct* epc, control_struct* ctrl); // Hidy 2012 - changing WPM);
