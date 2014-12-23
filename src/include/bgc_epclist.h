#ifndef BGC_EPCLIST_H
#define BGC_EPCLIST_H
/*
epclist.h
structure for array of ecophysiological constants structures 

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGC version 4.2 (final release)
See copyright.txt for Copyright information
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
	int nvegtypes;         /* number of vegetation types */
	epconst_struct* epc;   /* pointer to array of epc structures */
} epclist_struct;

/* function prototypes */
int epclist_init(file init, epclist_struct* epclist);
int epc_init(file init, epconst_struct* epc);

#ifdef __cplusplus
}
#endif

#endif
