   /*
bgc_struct.h putty
header file for structure definitions

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2018, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#define N_POOLS 3			/*  number of type of pools: water, carbon, nitrogen */
#define N_MGMDAYS 7			/*  number of type of management events in a single year */
#define N_SOILLAYERS 10		/*  number of type of soil layers in multilayer soil module */
#define N_PHENPHASES 7		/*  number of phenological phases */
#define NDAYS_OF_YEAR 365    /* number of days in a year */

/* VAR ctrl: simulation control variables */
typedef struct
{
	int metyears;				/* (n) years of met data */
    int simyears;				/* (n) years of simulation */
    int simstartyear;			/* (year) first year of simulation */
	int spinup;					/* (flag) 1=spinup run, 0=normal run */
	int maxspinyears;			/* (n) maximum number of years for spinup run */
	int dodaily;				/* (flag)  for daily output */
	int domonavg;				/* (flag)  for monthly average of daily outputs */
	int doannavg;				/* (flag)  for annual average of daily outputs */
	int doannual;				/* (flag)  for annual output */
	int ndayout;				/* (n) number of daily outputs */
	int nannout;				/* (n) number of annual outputs */
	int* daycodes;				/* (flag)  ARRAY of indices for daily outputs */
	char** daynames;			/* (flag)  ARRAY of names for daily outputs */
	int* anncodes;				/* (flag)  ARRAY of indices for annual outputs */
	char** annnames;			/* (flag)  ARRAY of names for annual outputs */
	int read_restart;			/* (flag)  to read restart file */
	int write_restart;			/* (flag) to write restart file */
	int keep_metyr;				/* (flag) 1=retain restart metyr, 0=reset metyr */
	int onscreen;				/* (flag) 1=show progress on-screen 0=don't */
	int GSI_flag;				/* (flag) for using GSI index to calculate growing season */
	int THN_flag;				/* (flag) for thinning management option */
	int MOW_flag;				/* (flag) for mowing management option */
	int GRZ_flag;				/* (flag) for grazing management option */
	int HRV_flag;				/* (flag) for harvesting management option */
	int PLG_flag;				/* (flag) for ploughing management option */
	int PLT_flag;				/* (flag) for planting management option */
	int FRZ_flag;				/* (flag) for fertilizing management option */
	int IRG_flag;				/* (flag) for irrigation management option */
	int simyr;					/* (flag) for counter for simulation years */
	int metyr;					/* (flag) for counter for meteorological years */
	int yday;					/* (flag) for counter for simulation days of year  */
	int spinyears;				/* (flag) for counter for spinup years  */
	int day;					/* (flag) number of the day in a month */
	int month;					/* (flag) number of the month in a year  */
	int varWPM_flag;			/* (flag) for changing WPM value */
	int varMSC_flag;			/* (flag) for changing MSC value */
	int varSGS_flag;			/* (flag) for changing WPM value */
	int varEGS_flag;			/* (flag) for changing WPM value */
	int GWD_flag;				/* (flag) for using gorundwater depth */
	double CNerror;				/* (kgC/m2) numbering error */
	double germ_depth;          /* (m) germination depth*/
	int germ_layer;             /* (n) layer of germination*/
	double gwd_act;				/* (m) actual depth of the groundwater on a given day */	


} control_struct;
/* endVAR */

/* VAR ndep: a structure to hold information about varied N-deposition scenario */
typedef struct
{
	int varndep;            /* (flag) 1=use file Ndep, 0=constant Ndep */
	double ndep;			/* (kgN/m2/yr) wet+dry atmospheric deposition of N */
	double* ndep_array;	    /* (kgN/m2/yr) ARRAY of annual Ndep */
} ndep_control_struct;
/* endVAR */

/*VAR co2: a structure to hold information on the annual co2 concentration */
typedef struct
{
	int varco2;             /* (flag) 0=const 1=use file  */
    double co2ppm;          /* (ppm)  constant CO2 concentration */
	double* co2ppm_array;   /* (ppm)  ARRAY of annual CO2 concentration */
} co2control_struct;	
/* endVAR */

/* VAR phenarr: phenological control arrays */
typedef struct
{
	int* onday_arr;			/* (doy) ARRAY of first day of transfer period */
	int* offday_arr;		/* (doy) ARRAY of last day of transfer period */
} phenarray_struct;
/* endVAR */

/* OUT phen: daily phenological data  */
typedef struct
{  
	double onday;					/* (doy) first day of vegetation period */
	double offday;					/* (doy) last day of vegetation period */
	double remdays_curgrowth;		/* (n) days left in current growth season */
	double remdays_transfer;		/* (n) number of transfer days remaining */
	double remdays_litfall;			/* (n) number of litfall days remaining */
	double predays_transfer;		/* (n)  number of transfer days previous */
	double predays_litfall;			/* (n) number of litfall days previous */
	double n_growthday;				/* (n) number of growing days in actual simulation year */
	double n_transferday;			/* (n) number of transfer days in actual simulation year */
	double n_litfallday;			/* (n) number of litterfall days in actual simulation year */
	double yday_phen;				/* (flag) for counter for simulation days of year for crops */
	double phpsl_dev_rate;			/* (dimless) relative development rate of photoslowing effect */
	double vern_dev_rate;			/* (dimless) relative development rate of vernalization */
	double vern_days;				/* (n) number of vernalization days */
	double GDD_limit;				/* (Celsius) lower limit of GDD in the given phen.phase */
	double GDD_crit[N_PHENPHASES];	/* (Celsius) critical GDD at the beginning of phen.phases */
	double GDD_emergSTART;	        /* (Celsius) start of emergence period */
	double GDD_emergEND;	        /* (Celsius) end of emergence period */

} phenology_struct;
/* endOUT  */
/* VAR metarr: meteorological variable arrays */
/* inputs from mtclim, except tavg11, tavg30, tavg30_ra and tavg11_ra
which are used for an 11-day running average of daily average air T,
computed for the whole length of the met ARRAY prior to the 
daily model loop */
typedef struct
{
    double* tmax;          /* (Celsius) daily maximum air temperature */
    double* tmin;          /* (Celsius) daily minimum air temperature */
    double* prcp;          /* (cm)    precipitation */
    double* vpd;           /* (Pa)    vapor pressure deficit */
    double* swavgfd;       /* (W/m2)  daylight avg shortwave flux density */
    double* par;           /* (W/m2)  photosynthetically active radiation */
    double* dayl;          /* (s)     daylength */
	double* tday;          /* (Celsius) daylight average temperature */
	double* tavg;          /* (Celsius) daily average temperature */
    double* tavg11_ra;     /* (Celsius) 11-day running avg of daily avg temp */
	double* tavg10_ra;     /* (Celsius) 10-day running avg of daily avg temp */
    double* tavg30_ra;     /* (Celsius) 30-day running avg of daily avg temp */
	double* F_temprad;	   /* (dimless) soil temperature factor of radtiation and air temperature  */
	double* F_temprad_ra;  /* (dimless) 5-day running avg soil temperature factor of radtiation and air temperature  */
} metarr_struct;
/* endVAR */

/* OUT metv: daily values that are passed to daily model subroutines */
typedef struct
{
	double prcp;							 /* (kg/m2) precipitation - INPUT */
	double tmax;							 /* (Celsius) daily maximum air temperature - INPUT */
	double tmin;							 /* (Celsius) daily minimum air temperature - INPUT */
	double tavg;							 /* (Celsius) daily average air temperature - INPUT */
	double tday;							 /* (Celsius) daylight average air temperature - INPUT */
	double tday_pre;						 /* (Celsius)  daily average air temperature on the previous day - INPUT */
	double par;								 /* (W/m2)  photosynthetically active radiation - INPUT */
	double vpd;								 /* (Pa)    vapor pressure deficit - INPUT */
	double co2;								 /* (ppm)   atmospheric concentration of CO2 - INPUT */
	double dayl;							 /* (s)     daylength - INPUT */
	double tnight;							 /* (Celsius) nightime average air temperature */
	double tavg11_ra;						 /* (Celsius)  11-day running average air temperature (linear weighted) */
	double tavg10_ra;						 /* (Celsius)  10-day running average air temperature */
	double tavg30_ra;						 /* (Celsius)  30-day running average air temperature */
	double F_temprad;				         /* (dimless) soil temperature factor of radiation and air temperature  */
	double F_temprad_ra;				     /* (dimless) 5-day running avg soil temperature factor  */
	double tsoil_surface;				   	 /* (Celsius)  surface temperature  */
	double tsoil_surface_pre;			     /* (Celsius)  daily soil surface temperature of the previous day */
	double tsoil_avg;						 /* (Celsius)  average soil temperature */
	double tsoil[N_SOILLAYERS];				 /* (Celsius)  daily soil layer temperature */
	double swRADnet;						 /* (W/m2) net short-wave radiation */
	double lwRADnet;						 /* (W/m2) net outgoing long-wave-radation */
	double RADnet;                           /* (W/m2) daylight average net radiation flux */
	double RADnet_per_plaisun;				 /* (W/m2) daylight average net radiation flux per unit sunlit proj LAI */
	double RADnet_per_plaishade;			 /* (W/m2) daylight average net radiation flux per unit sunshade proj LAI */
	double swavgfd;							 /* (W/m2) daylight average shortwave flux */
	double swabs;							 /* (W/m2) canopy absorbed shortwave flux */ 
	double swtrans;							 /* (W/m2) transmitted shortwave flux */
	double swabs_per_plaisun;				 /* (W/m2) swabs per unit sunlit proj LAI */
	double swabs_per_plaishade;				 /* (W/m2) swabs per unit shaded proj LAI */
	double ppfd_per_plaisun;				 /* (umol/m2/s) ppfd per unit sunlit proj LAI */
	double ppfd_per_plaishade;				 /* (umol/m2/s) ppfd per unit shaded proj LAI */
	double parabs;							 /* (W/m2)  PAR absorbed by canopy */
	double parabs_plaisun;					 /* (W/m2)  PAR absorbed by sunlit canopy fraction */
	double parabs_plaishade;				 /* (W/m2)  PAR absorbed by shaded canopy fraction */
	double GDD;								 /* (Celsius) growing degree day */
	double GDD_wMOD;						 /* (Celsius) GDD with modification of vern. and photop.eff. */
	double GDDpre;							 /* (Celsius) growing degree day on previous day */
	double pa;								 /* (Pa)    atmospheric pressure  */
} metvar_struct;
/* endOUT */

/* OUT ws: water state variables */
/* dimension: (kgH2O/m2) */
typedef struct
{
    double soilw[N_SOILLAYERS];		 /* water stored in the soil layers */
	double soilw_SUM;				 /* water stored in soil */
	double pond_water;				 /* water stored on surface because of saturation */
    double snoww;					 /* water stored in snowpack */
    double canopyw;					 /* water stored on canopy */
    double prcp_src;				 /* SUM of precipitation */
    double soilevap_snk;			 /* SUM of soil water evaporation */
    double snowsubl_snk;			 /* SUM of snow water sublimation */
    double canopyevap_snk;			 /* SUM of canopy water evaporation */
	double pondwevap_snk;			 /* SUM of pond water evaporation */
    double trans_snk;				 /* SUM of transpiration */
	double runoff_snk;			 	 /* SUM of runoff */
	double deeppercolation_snk;		 /* SUM of percolated water out of the system */
	double groundwater_src;			 /* SUM of water plus from goundwater */
	double canopyw_THNsnk;			 /* SUM of water stored on canopy is disappered because of thinning*/
	double canopyw_MOWsnk;		     /* SUM of water stored on canopy is disappered because of mowing*/
	double canopyw_HRVsnk;           /* SUM of water stored on canopy is disappered because of harvesting*/
	double canopyw_PLGsnk;		     /* SUM of water stored on canopy is disappered because of ploughing*/
	double canopyw_GRZsnk;			 /* SUM of water stored on canopy is disappered because of grazing*/
    double IRGsrc;					 /* SUM of water from irrigation */
	double FRZsrc;					 /* SUM of water from fertilization */
    double balanceERR;               /* SUM of water balance error  */
	double in;						 /* SUM of nitrogen input */
	double out;						 /* SUM of nitrogen output */
	double store;					 /* SUM of nitrogen store */
} wstate_struct;                        
/* endOUT */

/* OUT wf: water flux variables */
/* dimension: (kgH2O/m2/d) */
typedef struct
{
    double prcp_to_canopyw;							/* interception on canopy */
    double prcp_to_soilw;							/* precip entering soilwater pool  */
    double prcp_to_snoww;							/* snowpack accumulation */
	double prcp_to_runoff;						    /* runoff flux */
    double canopyw_evap;							/* evaporation from canopy */
    double canopyw_to_soilw;						/* canopy drip and stemflow  */
	double pondw_evap;                              /* pond water evaporation  */
	double snoww_subl;								/* sublimation from snowpack */
    double snoww_to_soilw;							/* melt from snowpack  */
    double soilw_evap;								/* evaporation from soil */
    double soilw_trans[N_SOILLAYERS];				/* transpiration from the soil layers */
    double soilw_trans_SUM;	                        /* SUM of transpiration from the soil layers */
	double evapotransp;								/* evapotranspiration (evap+trans+subl) */
	double pondw_to_soilw;                          /* water flux from pond to soil */
	double soilw_to_pondw;                          /* water flux from soil to pond */
	double soilw_percolated[N_SOILLAYERS];		    /* percolation fluxes between soil layers */
	double soilw_diffused[N_SOILLAYERS];			/* diffusion flux between the soil layers */
	double soilw_from_GW[N_SOILLAYERS];				/* soil water plus from ground water */
	double soilw_rootzone_leach;					/* soil water leached from rootzone (percol+diffus) */
	double canopyw_to_THN;							/* water stored on canopy is disappered because of thinning */
	double canopyw_to_MOW;							/* water stored on canopy is disappered because of mowing */
	double canopyw_to_HRV;							/* water stored on canopy is disappered because of harvesting */
	double canopyw_to_PLG;							/* water stored on canopy is disappered because of ploughing */
	double canopyw_to_GRZ;							/* water stored on canopy is disappered because of grazing */
	double IRG_to_prcp;								/* irrigatied water amount */	
	double FRZ_to_soilw;                            /* water flux from fertilization */
	double pot_evap;                                /* potential evaporation to calcualte pond evaporation */
} wflux_struct;
/* endOUT */

/* VAR cinit: carbon state initialization structure  */
typedef struct
{
	double max_leafc;			/* (kgC/m2) first-year displayed + stored leafc */
	double max_frootc;			/* (kgC/m2) first-year displayed + stored froot */
	double max_fruitc;			/* (kgC/m2) first-year displayed + stored fruit */
	double max_softstemc;		/* (kgC/m2) first-year displayed + stored softstem */
	double max_livestemc;		/* (kgC/m2) first-year displayed + stored live woody stem */
	double max_livecrootc;		/* (kgC/m2) first-year displayed + stored live coarse root */
} cinit_struct;
/* endVAR */

/* OUT cs: carbon state variables */
/* dimension: (kgC/m2)  */
typedef struct 
{
	double leafcSUM_phenphase[N_PHENPHASES];  	/* sum of leafC in a given phenophase */
    double leafc;								/* leaf C actual pool */
    double leafc_storage;						/* leaf C storage */
    double leafc_transfer;						/* leaf C transfer */
    double frootc;								/* fine root C actual pool*/
    double frootc_storage;						/* fine root C storage */
    double frootc_transfer;						/* fine root C transfer */
	double fruitc;             					/* fruitc actual pool */
    double fruitc_storage;     					/* fruitc storage pool */
    double fruitc_transfer;    					/* fruitc transfer pool */
	double softstemc;          					/* softstemc actual poo */
    double softstemc_storage;  					/* softstemc storage pool */
    double softstemc_transfer; 					/* softstemc transfer pool */
    double livestemc;		   					/* live stem C */
    double livestemc_storage;  					/* live stem C storage */
    double livestemc_transfer; 					/* live stem C transfer */
    double deadstemc;          					/* dead stem C */
    double deadstemc_storage;  					/* dead stem C storage */
    double deadstemc_transfer; 					/* dead stem C transfer */
    double livecrootc;         					/* live coarse root C */
    double livecrootc_storage; 					/* live coarse root C storage */
    double livecrootc_transfer; 				/* live coarse root C transfer */
    double deadcrootc;          				/* dead coarse root C */
    double deadcrootc_storage;  				/* dead coarse root C storage */
    double deadcrootc_transfer; 				/* dead coarse root C transfer */
	double gresp_storage;       				/* growth respiration storage */
	double gresp_transfer;      				/* growth respiration transfer */
	double nsc_w;                               /* non-stuctured woody carbohydrate pool */
	double nsc_nw;                              /* non-stuctured non-woody carbohydrate pool */
	double sc_w;                                /* stuctured woody carbohydrate pool */
	double sc_nw;                               /* stuctured non-woody carbohydrate pool */
    double cwdc[N_SOILLAYERS];					/* coarse woody debris N */
    double litr1c[N_SOILLAYERS];				/* litter labile C */
    double litr2c[N_SOILLAYERS];				/* litter unshielded cellulose C */
    double litr3c[N_SOILLAYERS];				/* litter shielded cellulose C */
    double litr4c[N_SOILLAYERS];				/* litter lignin C */
	double litrC[N_SOILLAYERS];					/* litter total C */
	double litr1c_total;						/* litter labile C */
    double litr2c_total;            			/* litter unshielded cellulose C */
    double litr3c_total;            			/* litter shielded cellulose C */
    double litr4c_total;            			/* litter lignin C */
	double cwdc_total;              			/* coarse woody debris N */
	double STDBc_leaf;							/*  wilted leaf biomass  */
	double STDBc_froot;							/*  wilted froot biomass  */
	double STDBc_fruit;							/*  wilted fruit biomass  */
	double STDBc_softstem;						/*  wilted sofstem biomass  */
	double STDBc_transfer;						/*  wilted transfer biomass  */
	double STDBc_above;             			/*  wilted aboveground plant biomass  */
	double STDBc_below;             			/*  wilted belowground plant biomass  */
	double CTDBc_leaf;							/*  cut-down leaf biomass  */
	double CTDBc_froot;							/*  cut-down froot biomass  */
	double CTDBc_fruit;							/*  cut-down fruit biomass  */
	double CTDBc_softstem;						/*  cut-down sofstem biomass  */
	double CTDBc_transfer;						/*  cut-down transfer biomass  */
	double CTDBc_cstem;			    			/*  cut-down coarse stem biomass  */
	double CTDBc_croot;			    			/*  cut-down coarse root biomass  */
	double CTDBc_above;             			/*  cut-down aboveground plant biomass  */
	double CTDBc_below;             			/*  cut-down belowground plant biomass  */
    double soil1c[N_SOILLAYERS];		/* microbial recycling pool C (fast) */
    double soil2c[N_SOILLAYERS];		/* microbial recycling pool C (medium) */
    double soil3c[N_SOILLAYERS];		/* microbial recycling pool C (slow) */
    double soil4c[N_SOILLAYERS];		/* recalcitrant SOM C (humus, slowest) */
	double soilC[N_SOILLAYERS];	        /* soil total C */
	double soil1_DOC[N_SOILLAYERS];		/* dissolved part of microbial recycling pool C (fast) */
    double soil2_DOC[N_SOILLAYERS];		/* dissolved part of microbial recycling pool C (medium) */
    double soil3_DOC[N_SOILLAYERS];		/* dissolved part of microbial recycling pool C (slow) */
    double soil4_DOC[N_SOILLAYERS];		/* dissolved part of recalcitrant SOM C (humus, slowest) */
	double soil_DOC[N_SOILLAYERS];	    /* dissolved part of soil total C */
	double soil1c_total;				/* SUM of microbial recycling pool C (fast) */
    double soil2c_total;				/* SUM of microbial recycling pool C (medium) */
    double soil3c_total;				/* SUM of microbial recycling pool C (slow) */
    double soil4c_total;				/* SUM of recalcitrant SOM C (humus, slowest) */
	double cpool;						/* temporary photosynthate C pool */
    double psnsun_src;					/* SUM of gross PSN from sulit canopy */
    double psnshade_src;				/* SUM of gross PSN from shaded canopy */
	double NSC_mr_snk;                  /* SUM of non-structured carobhydrate MR loss */
	double actC_mr_snk;                 /* SUM of actual pool's MR loss */
    double leaf_mr_snk;					/* SUM of leaf maint resp */
    double froot_mr_snk;				/* SUM of fine root maint resp */
	double fruit_mr_snk;				/* SUM of fruit maint resp.*/
	double softstem_mr_snk;				/* SUM of softstem maint resp.*/
	double livestem_mr_snk;				/* SUM of live stem maint resp */
	double livecroot_mr_snk;			/* SUM of live coarse root maint resp */
	double leaf_gr_snk;					/* SUM of leaf growth resp */
    double froot_gr_snk;				/* SUM of fine root growth resp */
	double fruit_gr_snk;				/* SUM of fruit growth resp. */
	double softstem_gr_snk;				/* SUM of softstem growth resp. */
    double livestem_gr_snk;				/* SUM of live stem growth resp */
	double livecroot_gr_snk;			/* SUM of live coarse root growth resp */
    double deadstem_gr_snk;				/* SUM of dead stem growth resp */ 
    double deadcroot_gr_snk;			/* SUM of dead coarse root growth resp */
    double litr1_hr_snk;				/* SUM of labile litr microbial resp */
    double litr2_hr_snk;				/* SUM of cellulose litr microbial resp */
    double litr4_hr_snk;				/* SUM of lignin litr microbial resp */
    double soil1_hr_snk;				/* SUM of fast microbial respiration */
    double soil2_hr_snk;				/* SUM of medium microbial respiration */
    double soil3_hr_snk;				/* SUM of slow microbial respiration */
    double soil4_hr_snk;				/* SUM of recalcitrant SOM respiration */
	double fire_snk;					/* SUM of fire losses */
	double SNSCsnk;						/* SUM of senescence losses */
    double PLTsrc;						/* SUM of C content of planted plant material */
	double THN_transportC;				/* SUM of C content of thinned and transported plant material*/
	double HRV_transportC;				/* SUM of C content of harvested and transported plant material*/
	double MOW_transportC;              /* SUM of mowed and transported plant material (C content)  */
	double GRZsnk;						/* SUM of C content of grazed leaf */
	double GRZsrc;						/* SUM of added carbon from fertilizer */
	double FRZsrc;						/* SUM of carbon content of fertilizer return to the litter pool */
	double balanceERR;					/* SUM of carbon balance error */
	double in;							/* SUM of carbon input */
	double out;							/* SUM of carbon output */
	double store;						/* SUM of carbon store */
} cstate_struct;
/* endOUT */

/* OUT cf: daily carbon flux variables:  */
/* dimension: (kgC/m2/d)   */
typedef struct
{
	/* group: mortality fluxes */ 
	double m_leafc_to_litr1c;                        
	double m_leafc_to_litr2c;              
	double m_leafc_to_litr3c;              
	double m_leafc_to_litr4c;              
	double m_frootc_to_litr1c;             
	double m_frootc_to_litr2c;             
	double m_frootc_to_litr3c;             
	double m_frootc_to_litr4c;  
	double m_fruitc_to_litr1c;             
	double m_fruitc_to_litr2c;             
	double m_fruitc_to_litr3c;             
	double m_fruitc_to_litr4c;       
	double m_softstemc_to_litr1c;            
	double m_softstemc_to_litr2c;            
	double m_softstemc_to_litr3c;            
	double m_softstemc_to_litr4c;  
	double m_leafc_storage_to_litr1c;      
	double m_frootc_storage_to_litr1c;
	double m_softstemc_storage_to_litr1c;   
	double m_fruitc_storage_to_litr1c;     
	double m_livestemc_storage_to_litr1c;  
	double m_deadstemc_storage_to_litr1c;  
	double m_livecrootc_storage_to_litr1c; 
	double m_deadcrootc_storage_to_litr1c; 
	double m_leafc_transfer_to_litr1c;     
	double m_frootc_transfer_to_litr1c;
	double m_fruitc_transfer_to_litr1c;  
	double m_softstemc_transfer_to_litr1c;   
	double m_livestemc_transfer_to_litr1c; 
	double m_deadstemc_transfer_to_litr1c; 
	double m_livecrootc_transfer_to_litr1c;
	double m_deadcrootc_transfer_to_litr1c;
	double m_livestemc_to_cwdc;            
	double m_deadstemc_to_cwdc;            
	double m_livecrootc_to_cwdc;           
	double m_deadcrootc_to_cwdc;           
	double m_gresp_storage_to_litr1c;      
	double m_gresp_transfer_to_litr1c;        
	/* group: fire fluxes */
	double m_leafc_to_fire;                
	double m_frootc_to_fire;               
	double m_fruitc_to_fire;               
	double m_softstemc_to_fire; 
	double m_STDBc_to_fire;
	double m_CTDBc_to_fire;
	double m_leafc_storage_to_fire;        
	double m_frootc_storage_to_fire;       
	double m_fruitc_storage_to_fire;       
	double m_softstemc_storage_to_fire;    
	double m_livestemc_storage_to_fire;    
	double m_deadstemc_storage_to_fire;    
	double m_livecrootc_storage_to_fire;   
	double m_deadcrootc_storage_to_fire;   
	double m_leafc_transfer_to_fire;       
	double m_frootc_transfer_to_fire;      
	double m_fruitc_transfer_to_fire;      
	double m_softstemc_transfer_to_fire;   
	double m_livestemc_transfer_to_fire;   
	double m_deadstemc_transfer_to_fire;   
	double m_livecrootc_transfer_to_fire;  
	double m_deadcrootc_transfer_to_fire;  
	double m_livestemc_to_fire;            
	double m_deadstemc_to_fire;            
	double m_livecrootc_to_fire;           
	double m_deadcrootc_to_fire;           
	double m_gresp_storage_to_fire;        
	double m_gresp_transfer_to_fire;       
	double m_litr1c_to_fire;               
	double m_litr2c_to_fire;               
	double m_litr3c_to_fire;               
	double m_litr4c_to_fire;               
	double m_cwdc_to_fire;                 
	/* group: senescence fluxes */
	double m_vegc_to_SNSC;                
	double m_leafc_to_SNSC; 
	double m_leafc_to_SNSCgenprog;
	double m_frootc_to_SNSC;               
	double m_fruitc_to_SNSC;                
	double m_softstemc_to_SNSC;                
	double m_leafc_storage_to_SNSC;                
	double m_frootc_storage_to_SNSC;               
	double m_leafc_transfer_to_SNSC;                
	double m_frootc_transfer_to_SNSC;               
	double m_fruitc_storage_to_SNSC;        
	double m_fruitc_transfer_to_SNSC;       
	double m_softstemc_storage_to_SNSC;        
	double m_softstemc_transfer_to_SNSC;       
	double m_gresp_storage_to_SNSC;
	double m_gresp_transfer_to_SNSC;
	/* group: harvesting senescence fluxes */
	double HRV_leafc_storage_to_SNSC;               
	double HRV_leafc_transfer_to_SNSC;    
	double HRV_fruitc_storage_to_SNSC;               
	double HRV_fruitc_transfer_to_SNSC;    
	double HRV_frootc_to_SNSC;                           
	double HRV_softstemc_to_SNSC;                 
	double HRV_frootc_storage_to_SNSC;               
	double HRV_frootc_transfer_to_SNSC;               
	double HRV_softstemc_storage_to_SNSC;        
	double HRV_softstemc_transfer_to_SNSC;       
	double HRV_gresp_storage_to_SNSC;
	double HRV_gresp_transfer_to_SNSC;
	/* group: flowering heat stress  */
	double fruitc_to_flowHS;
	/* group: standing dead biomass to litter fluxes */
	double STDBc_leaf_to_litr;
	double STDBc_froot_to_litr;
	double STDBc_fruit_to_litr;
	double STDBc_softstem_to_litr;	
	double STDBc_transfer_to_litr;
	double STDBc_to_litr;
	/* group: cut-dowc dead biomass to litter pool */
	double CTDBc_leaf_to_litr;
	double CTDBc_froot_to_litr;
	double CTDBc_fruit_to_litr;
	double CTDBc_softstem_to_litr;	
	double CTDBc_transfer_to_litr;
	double CTDBc_cstem_to_cwd;			   
	double CTDBc_croot_to_cwd;			    
	double CTDBc_to_litr;  
	/* group: phenology fluxes from transfer pool */
	double leafc_transfer_to_leafc;          
	double frootc_transfer_to_frootc;        
	double fruitc_transfer_to_fruitc;
	double softstemc_transfer_to_softstemc;
	double livestemc_transfer_to_livestemc;  
	double deadstemc_transfer_to_deadstemc;  
	double livecrootc_transfer_to_livecrootc;
	double deadcrootc_transfer_to_deadcrootc;
	/* group: leaf and fine root litterfall */
	double leafc_to_litr1c;              
	double leafc_to_litr2c;              
	double leafc_to_litr3c;              
	double leafc_to_litr4c;              
	double frootc_to_litr1c;             
	double frootc_to_litr2c;             
	double frootc_to_litr3c;             
	double frootc_to_litr4c;             
	double fruitc_to_litr1c;
	double fruitc_to_litr2c;
	double fruitc_to_litr3c;
	double fruitc_to_litr4c;
	double softstemc_to_litr1c;
	double softstemc_to_litr2c;
	double softstemc_to_litr3c;
	double softstemc_to_litr4c;
	/* group: maintenance respiration fluxes */
	double leaf_day_mr;                  
	double leaf_night_mr;                
	double froot_mr;
	double fruit_mr;   	
	double softstem_mr;
	double livestem_mr;                  
	double livecroot_mr;                 
	/* group: photosynthesis fluxes */
	double psnsun_to_cpool;              
	double psnshade_to_cpool;            
	/* group: litter decomposition fluxes  */
	double cwdc_to_litr2c[N_SOILLAYERS];	
	double cwdc_to_litr3c[N_SOILLAYERS];	
	double cwdc_to_litr4c[N_SOILLAYERS];	
	double litr1_hr[N_SOILLAYERS];			
	double litr1c_to_soil1c[N_SOILLAYERS];  
	double litr2_hr[N_SOILLAYERS];			
	double litr2c_to_soil2c[N_SOILLAYERS];  
	double litr3c_to_litr2c[N_SOILLAYERS];  
	double litr4_hr[N_SOILLAYERS];			
	double litr4c_to_soil3c[N_SOILLAYERS]; 
	double soil1_hr[N_SOILLAYERS];			
	double soil1c_to_soil2c[N_SOILLAYERS];	
	double soil2_hr[N_SOILLAYERS];			
	double soil2c_to_soil3c[N_SOILLAYERS];	
	double soil3_hr[N_SOILLAYERS];			
	double soil3c_to_soil4c[N_SOILLAYERS];	
	double soil4_hr[N_SOILLAYERS];		
	/* group: SOIL components leaching*/
	double soil1_DOC_leached[N_SOILLAYERS]; 
	double soil2_DOC_leached[N_SOILLAYERS]; 
	double soil3_DOC_leached[N_SOILLAYERS]; 
	double soil4_DOC_leached[N_SOILLAYERS]; 
	double soil1_DOC_diffused[N_SOILLAYERS]; 
	double soil2_DOC_diffused[N_SOILLAYERS]; 
	double soil3_DOC_diffused[N_SOILLAYERS];
	double soil4_DOC_diffused[N_SOILLAYERS];
	double DOC_rootzone_leach;
	/* group: daily allocation fluxes from current GPP */
	double cpool_to_leafc;               
	double cpool_to_leafc_storage;       
	double cpool_to_frootc;              
	double cpool_to_frootc_storage;    
	double cpool_to_fruitc;                
	double cpool_to_fruitc_storage;        
	double cpool_to_softstemc;           
	double cpool_to_softstemc_storage; 
	double cpool_to_livestemc;           
	double cpool_to_livestemc_storage;   
	double cpool_to_deadstemc;           
	double cpool_to_deadstemc_storage;   
	double cpool_to_livecrootc;          
	double cpool_to_livecrootc_storage;  
	double cpool_to_deadcrootc;          
	double cpool_to_deadcrootc_storage;  
	double cpool_to_gresp_storage;       
	/* group: daily growth respiration fluxes */
	double cpool_leaf_gr;                
	double cpool_leaf_storage_gr;        
	double transfer_leaf_gr;             
	double cpool_froot_gr;               
	double cpool_froot_storage_gr;       
	double transfer_froot_gr;   
	double cpool_fruit_gr;                 
	double cpool_fruit_storage_gr;         
	double transfer_fruit_gr;             
	double cpool_softstem_gr;                 
	double cpool_softstem_storage_gr;         
	double transfer_softstem_gr;             
	double cpool_livestem_gr;            
	double cpool_livestem_storage_gr;    
	double transfer_livestem_gr;         
	double cpool_deadstem_gr;            
	double cpool_deadstem_storage_gr;    
	double transfer_deadstem_gr;         
	double cpool_livecroot_gr;           
	double cpool_livecroot_storage_gr;   
	double transfer_livecroot_gr;        
	double cpool_deadcroot_gr;           
	double cpool_deadcroot_storage_gr;   
	double transfer_deadcroot_gr;   
	/* group: daily maintanance respiration ensuring fluxes */
	double leafc_storage_to_maintresp;
	double frootc_storage_to_maintresp;
	double fruitc_storage_to_maintresp;
	double softstemc_storage_to_maintresp;
	double livestemc_storage_to_maintresp;
	double livecrootc_storage_to_maintresp;
	double deadstemc_storage_to_maintresp;
	double deadcrootc_storage_to_maintresp;
	double leafc_transfer_to_maintresp;
	double frootc_transfer_to_maintresp;
	double fruitc_transfer_to_maintresp;
	double softstemc_transfer_to_maintresp;
	double livestemc_transfer_to_maintresp;
	double livecrootc_transfer_to_maintresp;
	double deadstemc_transfer_to_maintresp;
	double deadcrootc_transfer_to_maintresp;
	double leafc_to_maintresp;
	double frootc_to_maintresp;
	double fruitc_to_maintresp;
	double softstemc_to_maintresp;
	double livestemc_to_maintresp;
	double livecrootc_to_maintresp;
	double NSC_nw_to_maintresp;
	double actC_nw_to_maintresp;
	double NSC_w_to_maintresp;
	double actC_w_to_maintresp;
	/* group: annual turnover of storage to transfer pools */
	double leafc_storage_to_leafc_transfer;           
	double frootc_storage_to_frootc_transfer;     
    double fruitc_storage_to_fruitc_transfer;   
	double softstemc_storage_to_softstemc_transfer;   
	double livestemc_storage_to_livestemc_transfer;    
	double deadstemc_storage_to_deadstemc_transfer;    
	double livecrootc_storage_to_livecrootc_transfer; 
	double deadcrootc_storage_to_deadcrootc_transfer; 
	double gresp_storage_to_gresp_transfer;           
	/* group: turnover of live wood to dead wood */
	double livestemc_to_deadstemc;        
	double livecrootc_to_deadcrootc;      
	/* group: planting fluxes */
	double leafc_transfer_from_PLT;		
	double frootc_transfer_from_PLT;	
	double fruitc_transfer_from_PLT;		
	double softstemc_transfer_from_PLT;		
	/* group: thinning fluxes */
	double leafc_to_THN;				    
	double leafc_storage_to_THN;            
	double leafc_transfer_to_THN;           
	double fruitc_to_THN;				 
	double fruitc_storage_to_THN;         
	double fruitc_transfer_to_THN;        
	double livestemc_to_THN;				 
	double livestemc_storage_to_THN;         
	double livestemc_transfer_to_THN;        
	double deadstemc_to_THN;				 
	double deadstemc_storage_to_THN;         
	double deadstemc_transfer_to_THN;        
	double gresp_storage_to_THN;         
	double gresp_transfer_to_THN;        
	double THN_to_CTDBc_leaf;				  
	double THN_to_CTDBc_fruit;				 
	double THN_to_CTDBc_transfer;				 
	double THN_to_CTDBc_cstem;			     		 			    	 
	double STDBc_leaf_to_THN;				 			 
	double STDBc_fruit_to_THN;				 
	double STDBc_transfer_to_THN;	
	/* group: mowing fluxes */
	double leafc_to_MOW;				 
	double leafc_storage_to_MOW;         
	double leafc_transfer_to_MOW;        
	double fruitc_to_MOW;				 
	double fruitc_storage_to_MOW;         
	double fruitc_transfer_to_MOW;        
	double softstemc_to_MOW;				 
	double softstemc_storage_to_MOW;         
	double softstemc_transfer_to_MOW;        
	double gresp_storage_to_MOW;         
	double gresp_transfer_to_MOW;        
	double MOW_to_CTDBc_leaf;				 		 
	double MOW_to_CTDBc_fruit;
	double MOW_to_CTDBc_softstem;	
	double MOW_to_CTDBc_transfer;				 		 			    	 
	double STDBc_leaf_to_MOW;				 			 
	double STDBc_fruit_to_MOW;	
	double STDBc_softstem_to_MOW;	
	double STDBc_transfer_to_MOW;	
	/* group: harvesting fluxes */
	double leafc_to_HRV;				 
	double leafc_storage_to_HRV;         
	double leafc_transfer_to_HRV;        
	double fruitc_to_HRV;				 
	double fruitc_storage_to_HRV;         
	double fruitc_transfer_to_HRV;        
	double softstemc_to_HRV;				 
	double softstemc_storage_to_HRV;         
	double softstemc_transfer_to_HRV;        
	double gresp_storage_to_HRV;         
	double gresp_transfer_to_HRV;        
	double HRV_to_CTDBc_leaf;				 			 
	double HRV_to_CTDBc_fruit;
	double HRV_to_CTDBc_softstem;	
	double HRV_to_CTDBc_transfer;				 		 			    	 
	double STDBc_leaf_to_HRV;				 			 
	double STDBc_fruit_to_HRV;	
	double STDBc_softstem_to_HRV;	
	double STDBc_transfer_to_HRV;	
	/* group: ploughing fluxes */		 
	double leafc_to_PLG;					 
	double leafc_storage_to_PLG;			
	double leafc_transfer_to_PLG;	
	double frootc_to_PLG;					
	double frootc_storage_to_PLG;         
	double frootc_transfer_to_PLG;  
	double fruitc_to_PLG;					
	double fruitc_storage_to_PLG;			
	double fruitc_transfer_to_PLG;			
	double softstemc_to_PLG;				 
	double softstemc_storage_to_PLG;         
	double softstemc_transfer_to_PLG;        
	double gresp_storage_to_PLG;          
	double gresp_transfer_to_PLG;         
	double STDBc_leaf_to_PLG;
	double STDBc_froot_to_PLG;
	double STDBc_fruit_to_PLG;
	double STDBc_softstem_to_PLG;
	double STDBc_transfer_to_PLG;
	double CTDBc_leaf_to_PLG;
	double CTDBc_fruit_to_PLG;
	double CTDBc_softstem_to_PLG;
	/* group: grazing fluxes */	
	double leafc_to_GRZ;					
	double leafc_storage_to_GRZ;         
	double leafc_transfer_to_GRZ;      
	double fruitc_to_GRZ;				 
	double fruitc_storage_to_GRZ;         
	double fruitc_transfer_to_GRZ;        
	double softstemc_to_GRZ;				 
	double softstemc_storage_to_GRZ;         
	double softstemc_transfer_to_GRZ;        
	double gresp_storage_to_GRZ;         
	double gresp_transfer_to_GRZ;        
	double STDBc_leaf_to_GRZ;
	double STDBc_fruit_to_GRZ;
	double STDBc_softstem_to_GRZ;
	double STDBc_transfer_to_GRZ;
	double GRZ_to_litr1c;				 
	double GRZ_to_litr2c;				 
	double GRZ_to_litr3c;				 
	double GRZ_to_litr4c;
	/* group: fertilizing fluxes */
	double FRZ_to_litr1c;				 
	double FRZ_to_litr2c;				 
	double FRZ_to_litr3c;				 
	double FRZ_to_litr4c;	
	/* group: CH4 flux based on empirical estimation */
	double CH4_flux_soil;			
	double CH4_flux_MANURE;			
	double CH4_flux_FERMENT;


} cflux_struct;
/* endOUT */

/* OUT ns: nitrogen state variables */ 
/* dimension: (kgN/m2)   */
typedef struct
{
    double leafn;					/* leaf N actual pool */
    double leafn_storage;			/* leaf N storage pool */
    double leafn_transfer;			/* leaf N transfer pool */
    double frootn;					/* fine root N actual pool */ 
    double frootn_storage;			/* fine root N storage pool */ 
    double frootn_transfer;			/* fine root N transfer pool */ 
	double fruitn;					/* fruit N actual pool */
    double fruitn_storage;			/* fruit N storage pool */
    double fruitn_transfer;			/* fruit N transfer pool */
	double softstemn;				/* softstem N actual pool */
    double softstemn_storage;		/* softstem N storage pool */
    double softstemn_transfer;		/* softstem N transfer pool */
    double livestemn;				/* live stem N actual pool */
    double livestemn_storage;		/* live stem N storage pool */
    double livestemn_transfer;		/* live stem N transfer pool */
    double deadstemn;				/* dead stem N actual pool */
    double deadstemn_storage;		/* dead stem N storage pool */
    double deadstemn_transfer;		/* dead stem N transfer pool */
    double livecrootn;				/* live coarse root N actual pool */
    double livecrootn_storage;		/* live coarse root N storage pool */
    double livecrootn_transfer;		/* live coarse root N transfer pool */
    double deadcrootn;				/* dead coarse root N actual pool */
    double deadcrootn_storage;		/* dead coarse root N storage pool */
    double deadcrootn_transfer;		/* dead coarse root N transfer pool */
	double npool;					/* temporary plant N pool */
    double cwdn[N_SOILLAYERS];      /* coarse woody debris N */
    double litr1n[N_SOILLAYERS];    /* litter labile N */
    double litr2n[N_SOILLAYERS];    /* litter unshielded cellulose N */
    double litr3n[N_SOILLAYERS];    /* litter shielded cellulose N */
    double litr4n[N_SOILLAYERS];    /* litter lignin N */
	double litrN[N_SOILLAYERS];	    /* litter total N */
	double litr1n_total;            /* litter labile N */
    double litr2n_total;            /* litter unshielded cellulose N */
    double litr3n_total;            /* litter shielded cellulose N */
    double litr4n_total;            /* litter lignin N */
	double cwdn_total;				/* coarse woody debris N */
	double STDBn_leaf;				/*  wilted leaf biomass  */
	double STDBn_froot;				/*  wilted froot biomass  */
	double STDBn_fruit;				/*  wilted fruit biomass  */
	double STDBn_softstem;			/*  wilted sofstem biomass  */
	double STDBn_transfer;			/*  wilted transfer biomass  */
	double STDBn_above;             /*  wilted aboveground plant biomass  */
	double STDBn_below;             /*  wilted belowground plant biomass  */
	double CTDBn_leaf;				/*  cut-down leaf biomass  */
	double CTDBn_froot;				/*  cut-down froot biomass  */
	double CTDBn_fruit;				/*  cut-down fruit biomass  */
	double CTDBn_softstem;			/*  cut-down sofstem biomass  */
	double CTDBn_transfer;			/*  cut-down transfer biomass  */
	double CTDBn_cstem;			    /*  cut-down coarse stem biomass  */
	double CTDBn_croot;			    /*  cut-down coarse root biomass  */
	double CTDBn_above;             /*  cut-down aboveground plant biomass  */
	double CTDBn_below;             /*  cut-down belowground plant biomass  */
    double soil1n[N_SOILLAYERS];		/* microbial recycling pool N (fast) */
    double soil2n[N_SOILLAYERS];		/* microbial recycling pool N (medium) */
    double soil3n[N_SOILLAYERS];		/* microbial recycling pool N (slow) */
    double soil4n[N_SOILLAYERS];		/* recalcitrant SOM N (humus, slowest) */
	double soilN[N_SOILLAYERS];	        /* soil total N */
	double soil1_DON[N_SOILLAYERS];		/* microbial recycling pool N (fast) */
    double soil2_DON[N_SOILLAYERS];		/* microbial recycling pool N (medium) */
    double soil3_DON[N_SOILLAYERS];		/* microbial recycling pool N (slow) */
    double soil4_DON[N_SOILLAYERS];		/* recalcitrant SOM N (humus, slowest) */
	double soil_DON[N_SOILLAYERS];	    /* soil total N */
	double soil1n_total;				/* SUM of microbial recycling pool N (fast) in the total soil */
    double soil2n_total;				/* SUM of microbial recycling pool N (medium) in the total soil */
    double soil3n_total;				/* SUM of microbial recycling pool N (slow) in the total soil */
    double soil4n_total;				/* SUM of recalcitrant SOM C (humus, slowest) in the total soil */
	double retransn;					/* plant pool of retranslocated N */
    double sminNH4[N_SOILLAYERS];		/* soil mineral N in multilayer soil */
	double sminNO3[N_SOILLAYERS];		/* soil mineral N in multilayer soil */
	double sminNH4_total;               /* SUM of the soil mineral N in the total soil */
	double sminNO3_total;               /* SUM of the soil mineral N in the total soil */
    double nfix_src;					/* SUM of biological N fixation */
    double ndep_src;					/* SUM of N deposition inputs */
    double nleached_snk;				/* SUM of N leached */
    double nvol_snk;					/* SUM of N lost to volatilization */
	double fire_snk;					/* SUM of N lost to fire */
	double ndiffused_snk;				/* SUM of N leached */
	double SNSCsnk;						/* SUM of senescence losses */
	double FRZsrc;						/* SUM of N fertilization inputs */	
    double PLTsrc;						/* SUM of planted leaf N */
	double THN_transportN; 				/* SUM of thinned and transported plant material (N content)  */
	double HRV_transportN; 				/* SUM of harvested and transported plant material (N content)  */
	double MOW_transportN;              /* SUM of mowed and transported plant material (N content)  */
	double GRZsnk;						/* SUM of grazed leaf N */
	double GRZsrc;						/* SUM of leaf N from fertilizer*/
	double BNDRYsrc;					/* SUM of leaf N from boundary layer*/
	double SPINUPsrc;					/* SUM of leaf N from spinup correction*/
	double sum_ndemand;					/* SUM of leaf N from fertilizer*/
    double balanceERR;                  /* SUM of nitrogen balance error */
	double in;							/* SUM of nitrogen input */
	double out;							/* SUM of nitrogen output */
	double store;						/* SUM of nitrogen store */
} nstate_struct;
/* endOUT */

/* OUT nf: daily nitrogen flux variables */
/* dimension: (kgN/m2/d)   */
typedef struct
{
	/* group: mortality fluxes */
	double m_leafn_to_litr1n;              
	double m_leafn_to_litr2n;              
	double m_leafn_to_litr3n;              
	double m_leafn_to_litr4n;              
	double m_frootn_to_litr1n;             
	double m_frootn_to_litr2n;             
	double m_frootn_to_litr3n;             
	double m_frootn_to_litr4n; 		
	double m_fruitn_to_litr1n;              
	double m_fruitn_to_litr2n;              
	double m_fruitn_to_litr3n;              
	double m_fruitn_to_litr4n;              
	double m_softstemn_to_litr1n;          
	double m_softstemn_to_litr2n;          
	double m_softstemn_to_litr3n;          
	double m_softstemn_to_litr4n;      
	double m_leafn_storage_to_litr1n;      
	double m_frootn_storage_to_litr1n; 
    double m_fruitn_storage_to_litr1n;      
	double m_fruitn_transfer_to_litr1n;      
	double m_softstemn_storage_to_litr1n;      
	double m_softstemn_transfer_to_litr1n;
	double m_livestemn_storage_to_litr1n;  
	double m_deadstemn_storage_to_litr1n;  
	double m_livecrootn_storage_to_litr1n; 
	double m_deadcrootn_storage_to_litr1n; 
	double m_leafn_transfer_to_litr1n;     
	double m_frootn_transfer_to_litr1n;    
	double m_livestemn_transfer_to_litr1n; 
	double m_deadstemn_transfer_to_litr1n; 
	double m_livecrootn_transfer_to_litr1n;
	double m_deadcrootn_transfer_to_litr1n;
    double m_livestemn_to_litr1n;          
	double m_livestemn_to_cwdn;            
	double m_deadstemn_to_cwdn;            
	double m_livecrootn_to_litr1n;         
	double m_livecrootn_to_cwdn;           
	double m_deadcrootn_to_cwdn;           
	double m_retransn_to_litr1n;              
	/* group: senescence fluxes */
	double m_vegn_to_SNSC;                
	double m_leafn_to_SNSC;  
	double m_leafn_to_SNSCgenprog; 
	double m_frootn_to_SNSC;               
	double m_leafn_storage_to_SNSC;		   
	double m_frootn_storage_to_SNSC;       
	double m_leafn_transfer_to_SNSC;       
	double m_frootn_transfer_to_SNSC;     
	double m_fruitn_to_SNSC;                
	double m_fruitn_storage_to_SNSC;		   
	double m_fruitn_transfer_to_SNSC;       
	double m_softstemn_to_SNSC;                
	double m_softstemn_storage_to_SNSC;		   
	double m_softstemn_transfer_to_SNSC; 
	double m_retransn_to_SNSC;
	/* group: harvesting senescence fluxes */
	double HRV_leafn_storage_to_SNSC;               
	double HRV_leafn_transfer_to_SNSC;    
	double HRV_fruitn_storage_to_SNSC;               
	double HRV_fruitn_transfer_to_SNSC;    
	double HRV_frootn_to_SNSC;                           
	double HRV_softstemn_to_SNSC;                 
	double HRV_frootn_storage_to_SNSC;               
	double HRV_frootn_transfer_to_SNSC;               
	double HRV_softstemn_storage_to_SNSC;        
	double HRV_softstemn_transfer_to_SNSC;       
	double HRV_retransn_to_SNSC;
	/* group: flowering heat stress  */
	double fruitn_to_flowHS;
	/* group: standing dead biomass to litter pool */
	double STDBn_leaf_to_litr;
	double STDBn_froot_to_litr;
	double STDBn_fruit_to_litr;
	double STDBn_softstem_to_litr;	
	double STDBn_transfer_to_litr;
	double STDBn_to_litr;
	/* group: cut-down dead biomass to litter pool */
	double CTDBn_leaf_to_litr;
	double CTDBn_froot_to_litr;
	double CTDBn_fruit_to_litr;
	double CTDBn_softstem_to_litr;	
	double CTDBn_transfer_to_litr;
	double CTDBn_cstem_to_cwd;			   
	double CTDBn_croot_to_cwd;			    
	double CTDBn_to_litr;  
	/* group: fire fluxes */
	double m_leafn_to_fire;                
	double m_frootn_to_fire;    
	double m_fruitn_to_fire;    
	double m_softstemn_to_fire; 
	double m_STDBn_to_fire;
	double m_CTDBn_to_fire;
	double m_leafn_storage_to_fire;        
	double m_frootn_storage_to_fire; 
	double m_fruitn_storage_to_fire;		   
	double m_fruitn_transfer_to_fire;              
	double m_softstemn_storage_to_fire;		   
	double m_softstemn_transfer_to_fire; 
	double m_livestemn_storage_to_fire;    
	double m_deadstemn_storage_to_fire;    
	double m_livecrootn_storage_to_fire;   
	double m_deadcrootn_storage_to_fire;   
	double m_leafn_transfer_to_fire;       
	double m_frootn_transfer_to_fire;      
	double m_livestemn_transfer_to_fire;   
	double m_deadstemn_transfer_to_fire;   
	double m_livecrootn_transfer_to_fire;  
	double m_deadcrootn_transfer_to_fire;  
	double m_livestemn_to_fire;            
	double m_deadstemn_to_fire;            
	double m_livecrootn_to_fire;           
	double m_deadcrootn_to_fire;           
	double m_retransn_to_fire;             
	double m_litr1n_to_fire;               
	double m_litr2n_to_fire;               
	double m_litr3n_to_fire;               
	double m_litr4n_to_fire;               
	double m_cwdn_to_fire;                 
	/* group: phenology fluxes from transfer pool */
	double leafn_transfer_to_leafn;           
	double frootn_transfer_to_frootn;    
	double fruitn_transfer_to_fruitn;         
	double softstemn_transfer_to_softstemn; 
	double livestemn_transfer_to_livestemn;   
	double deadstemn_transfer_to_deadstemn;   
	double livecrootn_transfer_to_livecrootn; 
	double deadcrootn_transfer_to_deadcrootn;        
	/* group: litterfall fluxes */
	double leafn_to_litr1n;               
	double leafn_to_litr2n;                
	double leafn_to_litr3n;               
	double leafn_to_litr4n;               
	double leafn_to_retransn;             
	double frootn_to_litr1n;              
	double frootn_to_litr2n;              
	double frootn_to_litr3n;              
	double frootn_to_litr4n;              
	double fruitn_to_litr1n;               
	double fruitn_to_litr2n;                
	double fruitn_to_litr3n;               
	double fruitn_to_litr4n;               
	double softstemn_to_litr1n;               
	double softstemn_to_litr2n;                
	double softstemn_to_litr3n;               
	double softstemn_to_litr4n;               
	/* group: N input flux */
	double ndep_to_sminn;                 
	double nfix_to_sminn;                 
	/* group: litter and soil decomposition fluxes  */
	double cwdn_to_litr2n[N_SOILLAYERS];                
	double cwdn_to_litr3n[N_SOILLAYERS];                
	double cwdn_to_litr4n[N_SOILLAYERS];                
	double litr1n_to_soil1n[N_SOILLAYERS];              
	double litr2n_to_soil2n[N_SOILLAYERS];              
	double litr3n_to_litr2n[N_SOILLAYERS];              
	double litr4n_to_soil3n[N_SOILLAYERS]; 
	double soil1n_to_soil2n[N_SOILLAYERS];              
	double soil2n_to_soil3n[N_SOILLAYERS];              
	double soil3n_to_soil4n[N_SOILLAYERS];   
	/* group: mineralization fluxes  */
	double soil4n_to_sminNH4[N_SOILLAYERS];    
	double soil4n_to_sminNH4_total;    
	/* group: immobilization fluxes  */
	double sminn_to_soil_SUM[N_SOILLAYERS];             
	double sminNH4_to_soil_SUM[N_SOILLAYERS];           
	double sminNO3_to_soil_SUM[N_SOILLAYERS];                       
	double sminn_to_soil1n_l1[N_SOILLAYERS];            
	double sminn_to_soil2n_l2[N_SOILLAYERS];            
	double sminn_to_soil3n_l4[N_SOILLAYERS];            
	double sminn_to_soil2n_s1[N_SOILLAYERS];            
	double sminn_to_soil3n_s2[N_SOILLAYERS];            
	double sminn_to_soil4n_s3[N_SOILLAYERS]; 
	double sminn_to_soil_SUM_total;             
	double sminNH4_to_soil_SUM_total;           
	double sminNO3_to_soil_SUM_total; 
	/* group: nitrification and denitrification fluxes */
	double sminNO3_to_denitr[N_SOILLAYERS];
	double sminNH4_to_nitrif[N_SOILLAYERS];
	double N2_flux_DENITR[N_SOILLAYERS];				
	double N2O_flux_NITRIF[N_SOILLAYERS];				
	double N2O_flux_DENITR[N_SOILLAYERS];
	double sminNO3_to_denitr_total;
	double sminNH4_to_nitrif_total;			
	double N2_flux_DENITR_total;				
	double N2O_flux_NITRIF_total;				
	double N2O_flux_DENITR_total;	
	/* group: SMINN change caused by soil process */
	double sminNH4_to_npool[N_SOILLAYERS];                 
	double sminNO3_to_npool[N_SOILLAYERS];  
	double sminNH4_to_npool_total;          
	double sminNO3_to_npool_total;			
	double sminn_to_npool;
	/* group: SOIL components leaching*/
	double sminNH4_leached[N_SOILLAYERS];    
	double sminNH4_diffused[N_SOILLAYERS];   
	double sminNO3_leached[N_SOILLAYERS];    
	double sminNO3_diffused[N_SOILLAYERS];
	double sminN_rootzone_leach;
	double soil1_DON_leached[N_SOILLAYERS]; 
	double soil2_DON_leached[N_SOILLAYERS]; 
	double soil3_DON_leached[N_SOILLAYERS]; 
	double soil4_DON_leached[N_SOILLAYERS]; 
	double soil1_DON_diffused[N_SOILLAYERS]; 
	double soil2_DON_diffused[N_SOILLAYERS]; 
	double soil3_DON_diffused[N_SOILLAYERS]; 
	double soil4_DON_diffused[N_SOILLAYERS]; 
	double DON_rootzone_leach;
	/* group: daily allocation fluxes */
	double retransn_to_npool;             
	double npool_to_leafn;                
	double npool_to_leafn_storage;  
	double npool_to_frootn;               
	double npool_to_frootn_storage;   
	double npool_to_fruitn;                
	double npool_to_fruitn_storage;        
	double npool_to_softstemn;                
	double npool_to_softstemn_storage;        
	double npool_to_livestemn;            
	double npool_to_livestemn_storage;    
	double npool_to_deadstemn;            
	double npool_to_deadstemn_storage;    
	double npool_to_livecrootn;           
	double npool_to_livecrootn_storage;   
	double npool_to_deadcrootn;           
	double npool_to_deadcrootn_storage;  
	/* group: daily maintanance respiration ensuring fluxes - retranslocation */
	double leafn_storage_to_maintresp;
	double frootn_storage_to_maintresp;
	double fruitn_storage_to_maintresp;
	double softstemn_storage_to_maintresp;
	double livestemn_storage_to_maintresp;
	double livecrootn_storage_to_maintresp;
	double deadstemn_storage_to_maintresp;
	double deadcrootn_storage_to_maintresp;
	double leafn_transfer_to_maintresp;
	double frootn_transfer_to_maintresp;
	double fruitn_transfer_to_maintresp;
	double softstemn_transfer_to_maintresp;
	double livestemn_transfer_to_maintresp;
	double livecrootn_transfer_to_maintresp;
	double deadstemn_transfer_to_maintresp;
	double deadcrootn_transfer_to_maintresp;
	double leafn_to_maintresp;
	double frootn_to_maintresp;
	double fruitn_to_maintresp;
	double softstemn_to_maintresp;
	double livestemn_to_maintresp;
	double livecrootn_to_maintresp;
	double NSN_nw_to_maintresp;
	double actN_nw_to_maintresp;
	double NSN_w_to_maintresp;
	double actN_w_to_maintresp;
	/* group: annual turnover of storage to transfer */
	double leafn_storage_to_leafn_transfer;           
	double frootn_storage_to_frootn_transfer;         
	double livestemn_storage_to_livestemn_transfer;   
	double deadstemn_storage_to_deadstemn_transfer;   
	double livecrootn_storage_to_livecrootn_transfer; 
	double deadcrootn_storage_to_deadcrootn_transfer; 
	double fruitn_storage_to_fruitn_transfer;           
	double softstemn_storage_to_softstemn_transfer;           
	/* group: turnover of live wood to dead wood, with retranslocation */
	double livestemn_to_deadstemn;        
	double livestemn_to_retransn;         
	double livecrootn_to_deadcrootn;      
	double livecrootn_to_retransn;        
	/* group: planting fluxes */
	double leafn_transfer_from_PLT;		
	double frootn_transfer_from_PLT;		
	double fruitn_transfer_from_PLT;		
	double softstemn_transfer_from_PLT;		
	/* group: thinning fluxes */
	double leafn_to_THN;				 
	double leafn_storage_to_THN;         
	double leafn_transfer_to_THN;        
	double fruitn_to_THN;				 
	double fruitn_storage_to_THN;         
	double fruitn_transfer_to_THN;  
	double livestemn_to_THN;				 
	double livestemn_storage_to_THN;         
	double livestemn_transfer_to_THN;        
	double deadstemn_to_THN;				 
	double deadstemn_storage_to_THN;         
	double deadstemn_transfer_to_THN;        
	double retransn_to_THN;
	double THN_to_CTDBn_leaf;				 			 
	double THN_to_CTDBn_fruit;				 
	double THN_to_CTDBn_transfer;				 
	double THN_to_CTDBn_cstem;			     		 			    	 
	double STDBn_leaf_to_THN;				  
	double STDBn_fruit_to_THN;				 
	double STDBn_transfer_to_THN;	
	/* group: mowing fluxes */
	double leafn_to_MOW;                 
	double leafn_storage_to_MOW;         
	double leafn_transfer_to_MOW;
	double fruitn_to_MOW;				 
	double fruitn_storage_to_MOW;         
	double fruitn_transfer_to_MOW;        
	double softstemn_to_MOW;				 
	double softstemn_storage_to_MOW;         
	double softstemn_transfer_to_MOW; 
	double retransn_to_MOW;
	double MOW_to_CTDBn_leaf;				 			 
	double MOW_to_CTDBn_fruit;
	double MOW_to_CTDBn_softstem;	
	double MOW_to_CTDBn_transfer;				 		 			    	 
	double STDBn_leaf_to_MOW;				 			 
	double STDBn_fruit_to_MOW;	
	double STDBn_softstem_to_MOW;	
	double STDBn_transfer_to_MOW;	
	/* group: harvesting fluxes */
	double leafn_to_HRV;                 
	double leafn_storage_to_HRV;         
	double leafn_transfer_to_HRV;  
	double fruitn_to_HRV;				 
	double fruitn_storage_to_HRV;         
	double fruitn_transfer_to_HRV;        
	double softstemn_to_HRV;				 
	double softstemn_storage_to_HRV;         
	double softstemn_transfer_to_HRV;   
	double retransn_to_HRV;
	double HRV_to_CTDBn_leaf;				 
	double HRV_to_CTDBn_fruit;
	double HRV_to_CTDBn_softstem;	
	double HRV_to_CTDBn_transfer;				 		 			    	 
	double STDBn_leaf_to_HRV;				 	 
	double STDBn_fruit_to_HRV;	
	double STDBn_softstem_to_HRV;	
	double STDBn_transfer_to_HRV;		  
	/* group: ploughing fluxes  */
	double leafn_to_PLG;					 
	double leafn_storage_to_PLG;			
	double leafn_transfer_to_PLG;	
	double frootn_to_PLG;					
	double frootn_storage_to_PLG;         
	double frootn_transfer_to_PLG;  
	double fruitn_to_PLG;					
	double fruitn_storage_to_PLG;			
	double fruitn_transfer_to_PLG;			
	double softstemn_to_PLG;				 
	double softstemn_storage_to_PLG;         
	double softstemn_transfer_to_PLG;        
	double retransn_to_PLG;                 
	double STDBn_leaf_to_PLG;
	double STDBn_froot_to_PLG;
	double STDBn_fruit_to_PLG;
	double STDBn_softstem_to_PLG;
	double STDBn_transfer_to_PLG;
	double CTDBn_leaf_to_PLG;
	double CTDBn_fruit_to_PLG;
	double CTDBn_softstem_to_PLG;
	/* group: grazing fluxes */
	double leafn_to_GRZ;                 
	double leafn_storage_to_GRZ;         
	double leafn_transfer_to_GRZ; 
	double fruitn_to_GRZ;				 
	double fruitn_storage_to_GRZ;         
	double fruitn_transfer_to_GRZ;        
	double softstemn_to_GRZ;				 
	double softstemn_storage_to_GRZ;         
	double softstemn_transfer_to_GRZ;  
	double STDBn_leaf_to_GRZ;
	double STDBn_fruit_to_GRZ;
	double STDBn_softstem_to_GRZ;
	double STDBn_transfer_to_GRZ;
	double retransn_to_GRZ;
	double GRZ_to_litr1n;				 
	double GRZ_to_litr2n;				 
	double GRZ_to_litr3n;				 
	double GRZ_to_litr4n;				 
	/* group: fertilizing fluxes */
	double FRZ_to_sminNH4;      
	double FRZ_to_sminNO3; 
	double FRZ_to_litr1n;				 
	double FRZ_to_litr2n;				 
	double FRZ_to_litr3n;				 
	double FRZ_to_litr4n;
	/* group: management N2O flux based on empirical estimation */
	double N2O_flux_GRZ;
	double N2O_flux_FRZ;

} nflux_struct;
/* endOUT */


/* VAR nt: temporary nitrogen variables */
typedef struct 
{
	double mineralized[N_SOILLAYERS];     /* (kgN/m2/d) gross mineralization */
	double potential_immob[N_SOILLAYERS]; /* (kgN/m2/d) gross immobilization */
	double plant_ndemand[N_SOILLAYERS];	  /* (kgN/m2/d) nitrogen demand of plant */	
	double plitr1c_loss[N_SOILLAYERS];    /* (kgN/m2/d) potential loss of labile litter */
	double pmnf_l1s1[N_SOILLAYERS];       /* (kgN/m2/d) mineralized nitrogen flux of labile litter */
	double plitr2c_loss[N_SOILLAYERS];    /* (kgN/m2/d) potential loss of unshielded litter */
	double pmnf_l2s2[N_SOILLAYERS];       /* (kgN/m2/d) mineralized nitrogen flux of cellulose litter */
	double plitr4c_loss[N_SOILLAYERS];	  /* (kgN/m2/d) potential loss of lignin litter */
	double pmnf_l4s3[N_SOILLAYERS];       /* (kgN/m2/d) mineralized nitrogen flux of lignin litter */
	double psoil1c_loss[N_SOILLAYERS];    /* (kgN/m2/d) potential loss of microbial recycling pool */
	double pmnf_s1s2[N_SOILLAYERS];       /* (kgN/m2/d) mineralized nitrogen flux of microbial recycling pool */
	double psoil2c_loss[N_SOILLAYERS];    /* (kgN/m2/d) potential loss of microbial recycling pool */
	double pmnf_s2s3[N_SOILLAYERS];       /* (kgN/m2/d) mineralized nitrogen flux of microbial recycling pool */
	double psoil3c_loss[N_SOILLAYERS];    /* (kgN/m2/d) potential loss of microbial recycling pool */
	double pmnf_s3s4[N_SOILLAYERS];       /* (kgN/m2/d) mineralized nitrogen flux of hummus pool */
	double psoil4c_loss[N_SOILLAYERS];    /* (kgN/m2/d) potential loss of hummus pool */
	double kl4[N_SOILLAYERS];             /* (1/day) corrected rate constants from the rate scalar */
} ntemp_struct;
/* endVAR */
	

/* OUT epv: ecophysiological variables */
typedef struct
{
	int n_rootlayers;							/* (n) actual number of soil layers in which root can be found  */
	int n_maxrootlayers;						/* (n) maximum number of soil layers in which root can be found  */
	double thermal_time;			            /* (Celsius) difference between tavg and base temperature */
	double thermal_timeSUM[NDAYS_OF_YEAR];		/* (Celsius) sum of thermal time */
	double cpool_to_leafcARRAY[NDAYS_OF_YEAR];   /* (kgC/m2/day) array of carbon from cpool to leafC */                 
	double npool_to_leafnARRAY[NDAYS_OF_YEAR];   /* (kgN/m2/day) array of nitrogen from npool to leafN */  
	double leafday;                             /* (n) counter for  days of year when leaves are on */
	double leafday_lastmort;                    /* (n) counter for  last genetical mortality day */
	double n_actphen;							/* (n) number of the actual phenophase (from 1 to 7)*/
	double thermal_timeSUM_max;		            /* (Celsius) maximal thermal_timeSUM */
	double flowHS_mort;                         /* (prop.) mortality coefficient of flwoering heat stress */
	double transfer_ratio;                      /* (prop) transfer proportion on actual day  */ 
	double day_leafc_litfall_increment;			/* (kgC/m2/d) rate leaf litfall */
	double day_fruitc_litfall_increment;		/* (kgC/m2/d) rate fruit litfall  */
	double day_softstemc_litfall_increment;		/* (kgC/m2/d) rate softstem litfall  */
	double day_frootc_litfall_increment;		/* (kgC/m2/d) rate froot litfall */
	double day_livestemc_turnover_increment;	/* (kgC/m2/d) rate livestem turnover */
	double day_livecrootc_turnover_increment;	/* (kgC/m2/d) rate livecroot turnover */
	double annmax_leafc;						/* (kgC/m2) annual maximum daily leaf C */
	double annmax_fruitc;						/* (kgC/m2) annual maximum daily fruit C  */
	double annmax_softstemc;					/* (kgC/m2) annual maximum daily softstem C */
	double annmax_frootc;						/* (kgC/m2) annual maximum daily froot C */
	double annmax_livestemc;					/* (kgC/m2) annual maximum daily livestem C */
	double annmax_livecrootc;					/* (kgC/m2) annual maximum daily livecroot C */
	double dsr;									/* (n) number of days since rain, for soil evap */
	double dsws;								/* (n) number of days since soil water stress */
    double proj_lai;							/* (m2/m2) live projected leaf area index */
    double all_lai;								/* (m2/m2) live all-sided leaf area index */
	double sla_avg;                             /* (m2/m2) canopy average proj. SLA */
    double plaisun;								/* (m2/m2) sunlit projected leaf area index */
    double plaishade;							/* (m2/m2) shaded projected leaf area index */
    double sun_proj_sla;						/* (m2/kgC) sunlit projected SLA */
    double shade_proj_sla;						/* (m2/kgC) shaded projected SLA */
	double plant_height;                        /* (m) height of plant (based on stemw and LAI)*/
	double NDVI;                                /* (ratio) normalized difference vegetation index */
	double rootlength_prop[N_SOILLAYERS];		    /* (prop) proportion of total root lenght in the given soil layer  */
	double psi[N_SOILLAYERS];						/* (MPa) water potential of soil and leaves   */
	double pF[N_SOILLAYERS];						/* (cm) soil water suction derived from log(soil water potential)  */
	double hydr_conduct_S[N_SOILLAYERS];			/* (m/s) hydraulic conductivity at the beginning of the day  */
	double hydr_diffus_S[N_SOILLAYERS];				/* (m2/s) hydraulic diffusivity at the beginning of the day   */
	double hydr_conduct_E[N_SOILLAYERS];			/* (m/s) hydraulic conductivity at the end of the day   */
	double hydr_diffus_E[N_SOILLAYERS];				/* (m2/s) hydraulic diffusivity at the end of the day  */
    double vwc[N_SOILLAYERS];						/* (m3/m3) volumetric water content  */
	double rooting_depth;							/* (m) actual depth of the rooting zone */
	double psi_avg;									/* (MPa) average water potential of soil and leaves */
    double vwc_avg;									/* (m3/m3) average volumetric water content in active layers */
    double vwc_ratio_crit1[N_SOILLAYERS];			/* (ratio) volumetric water content ratio at start of conductance reduction */
	double vwc_ratio_crit2[N_SOILLAYERS];			/* (ratio) volumetric water content ratio at stomatal closure */
    double vwc_crit1[N_SOILLAYERS];					/* (ratio) volumetric water content at start of conductance reduction */
	double vwc_crit2[N_SOILLAYERS];					/* (ratio) volumetric water content at stomatal closure */
	double psi_crit1[N_SOILLAYERS];					/* (ratio) soil water potential at start of conductance reduction */
	double psi_crit2[N_SOILLAYERS];					/* (ratio) soil water potential at stomatal closure */
	double dlmr_area_sun;					 /* (umolC/m2projected leaf area/s) sunlit leaf MR */
    double dlmr_area_shade;				  	 /* (umolC/m2projected leaf area/s) shaded leaf MR */
    double gl_t_wv_sun;						 /* (m/s) leaf-scale conductance to transpired water */
    double gl_t_wv_shade;					 /* (m/s) leaf-scale conductance to transpired water */
    double assim_sun;						 /* (umol/m2/s) sunlit assimilation per unit pLAI */
    double assim_shade;						 /* (umol/m2/s) shaded assimilation per unit pLAI */
    double t_scalar[N_SOILLAYERS];			 /* (dimless) decomp temperature scalar */
    double w_scalar[N_SOILLAYERS];			 /* (dimless) decomp water scalar */
    double rate_scalar[N_SOILLAYERS];		 /* (dimless) decomp combined scalar */
	double rate_scalar_avg;					 /* (dimless) decomp combined and averaged scalar */
	double daily_gross_nmin[N_SOILLAYERS];	 /* (kgN/m2/d) daily gross N mineralization */
	double daily_gross_nimmob[N_SOILLAYERS]; /* (kgN/m2/d) daily gross N immobilization */ 
	double daily_net_nmin[N_SOILLAYERS];	 /* (kgN/m2/d) daily net N mineralization */
	double max_conduct;						 /* (m/s) stomatal conductance with atmospheric [CO2] multiplier */
	double m_tmin;			                 /* (dimless) freezing night temperature multiplier */
	double m_soilstress_layer[N_SOILLAYERS]; /* (dimless) soil water stress multiplier */
	double m_soilstress;					 /* (dimless) soil water properties  multiplier */
	double m_ppfd_sun;						 /* (dimless) PAR flux density multiplier */
	double m_ppfd_shade;					 /* (dimless) PAR flux density multiplier */
	double m_vpd;							 /* (dimless) vapor pressure deficit multiplier */
	double m_final_sun;						 /* (dimless) product of all other multipliers */
	double m_final_shade;                    /* (dimless) product of all other multipliers */
	double gl_bl;                            /* (m/s) leaf boundary layer conductance */
    double gl_c;                             /* (m/s) leaf cuticular conductance */
    double gl_s_sun;                         /* (m/s) leaf-scale stomatal conductance */
    double gl_s_shade;                       /* (m/s) leaf-scale stomatal conductance */
	double gl_e_wv;                          /* (m/s) leaf conductance to evaporated water */
	double gl_sh;                            /* (m/s) leaf conductance to sensible heat */
    double gc_e_wv;                          /* (m/s) canopy conductance to evaporated water */
    double gc_sh;                            /* (m/s) canopy conductance to sensible heat */
	double ytd_maxplai;                      /* (m2/m2) year-to-date maximum projected LAI */
	double n_limitation[N_SOILLAYERS];	     /* (flag) for nitrogen limitation in multilayer soil */
	double plant_calloc;					 /* (kgC/m2) amount of C allocated */
	double plant_nalloc;					 /* (kgN/m2) amount of N allocated */
	double excess_c;						 /* (kgC/m2) difference between available and allocated C */
	double pnow;                             /* (prop) proportion of growth displayed on current day */ 
	double NSC_limit_nw;					 /* (flag) for NSC-limitation in maint.resp.calculation for nw-biomass */
	double NSC_limit_w;						 /* (flag) for NSC-limitation in maint.resp.calculation for nw-biomass */
	double SMSI;                             /* (prop.) soil moisture stress index */
} epvar_struct;
/* endOUT */

/* VAR sitec: soil and site constants */
typedef struct
{
	double sand[N_SOILLAYERS];							/* (%) sand content of soil in the given soil layer */	
	double silt[N_SOILLAYERS];							/* (%) silt content of soil in the given soil layer */	
	double clay[N_SOILLAYERS];							/* (%) clay content of soil in the given soil layer */
	double pH[N_SOILLAYERS];							/* (%) soil pH in the given soil layer */
    double soillayer_depth[N_SOILLAYERS];			    /* (m) contains the soil layer depths (positive values)*/
	double soillayer_thickness[N_SOILLAYERS];		    /* (m) contains the soil layer thicknesses (positive values) */
	double soillayer_midpoint[N_SOILLAYERS];			/* (m) contains the depths of the middle layers (positive values)*/
    double soil_b[N_SOILLAYERS];						/* (dimless) Clapp-Hornberger "b" parameter */
	double BD[N_SOILLAYERS];							/* (g/cm3) bulk density */
	double RCN;								            /* (m) runoff curve number */
	double pondmax;								        /* (m) maximum of pond water */
    double vwc_sat[N_SOILLAYERS];						/* (m3/m3) volumetric water content at saturation */
    double vwc_fc[N_SOILLAYERS];						/* (m3/m3) VWC at field capacity ( = -0.033 MPa) */
	double vwc_wp[N_SOILLAYERS];						/* (m3/m3) VWC at wilting point ( = pF 4.2) */
	double vwc_hw[N_SOILLAYERS];						/* (m3/m3) VWC at hygroscopic water point  ( = pF 6.2) */
    double psi_sat[N_SOILLAYERS];						/* (MPa) soil matric potential at saturation */
	double psi_fc[N_SOILLAYERS];						/* (MPa) soil matric potential at field capacity */
	double psi_wp[N_SOILLAYERS];						/* (MPa) soil matric potential at wilting point */
	double psi_hw;										/* (MPa) soil matric potential at hygroscopic water point */
	double hydr_conduct_sat[N_SOILLAYERS];				/* (m/s) hidraulic conductivity at saturation  */
	double hydr_diffus_sat[N_SOILLAYERS];				/* (m2/s) hidraulic diffusivity at saturation  */
	double hydr_conduct_fc[N_SOILLAYERS];				/* (m/s) hidraulic conductivity at field capacity  */
	double hydr_diffus_fc[N_SOILLAYERS];				/* (m2/s) hidraulic diffusivity at field capacity  */
	double tair_annavg;						            /* (Celsius) mean surface air temperature */
	double tair_annrange;					            /* (Celsius) mean annual air temperature range  */
    double elev;								        /* (m) site elevation */
    double lat;				 					        /* (degrees) site latitude (negative for south) */
    double sw_alb;								        /* (dimless) surface shortwave albedo */
	double RCN_mes;								        /* (m) measured runoff curve number */
	double BD_mes[N_SOILLAYERS];					    /* (g/cm3)  measured bulk density */
	double vwc_sat_mes[N_SOILLAYERS];					/* (m3/m3)  measured soil water content at saturation*/
	double vwc_fc_mes[N_SOILLAYERS];					/* (m3/m3)  measured soil water content at field capacity*/
	double vwc_wp_mes[N_SOILLAYERS];					/* (m3/m3)  measured soil water content at wilting point*/
	double vwc_hw_mes[N_SOILLAYERS];					/* (m3/m3)  measured hygroscopic water content */
	double* gwd_array;									/* (m) ARRAY of depth of the groundwater */	

} siteconst_struct;								
/* endVAR */

/* VAR epc: canopy ecophysiological constants */
typedef struct
{
	int woody;					/* (flag) 1=woody, 0=non-woody */
	int evergreen;				/* (flag) 1=evergreen, 0=deciduous */
	int c3_flag;				/* (flag) 1 = C3,  0 = C4 */
	int phenology_flag;			/* (flag) 1=phenology model, 0=user defined */
	int transferGDD_flag;       /* (flag) 1= transfer calculation from GDD, 0 = transfer calculation from EPC */
	int q10depend_flag;			/* (flag) 1 = temperature dependent q10 value; 0= constans q10 value */
	int acclimation_flag;		/* (flag)  1 = acclimation 0 = no acclimation */
	int CO2conduct_flag;	    /* (flag) CO2 conductance reduction flag (0: no effect, 1: multiplier) */
	int SHCM_flag;			    /* (flag) water movement calculation method (0:Richards, 1:DSSAT) */
	int discretlevel_Richards;  /*  discretization level of SWC calculation */
	int STCM_flag;			    /* (flag) soil temperature calculation method (0:Zheng, 1:DSSAT) */
	int photosynt_flag;         /* (flag) photosynthesis calculation method (0: Farquhar, 1: DSSAT) */
    int evapotransp_flag;	    /* (flag) evapotranspiration calculation method (0: Penman-Montieth, 1: Priestly-Taylor) */
    int radiation_flag;	        /* (flag) radiation calculation method (0: SWabs, 1: Rn) */
	int onday;                  /* (doy) yearday leaves on */
	int offday;					/* (doy) yearday leaves off */
	double base_temp;			/* (Celsius) basic temperature fo GDD/heatsum calculation */
	double pnow_minT;			/* (Celsius) min. temp. for growth displayed on current day (-9999: no T-dep) */
	double pnow_opt1T;			/* (Celsius) opt1.temp. for growth displayed on current day (-9999: no T-dep) */ 
	double pnow_opt2T;			/* (Celsius) opt2.temp. for growth displayed on current day (-9999: no T-dep) */ 
	double pnow_maxT;			/* (Celsius) max. temp. for growth displayed on current day (-9999: no T-dep) */ 
	double assim_minT;			/* (Celsius) min. temp. for C-assim displayed on current day (-9999: no limit) */
	double assim_opt1T;			/* (Celsius) opt1.temp. for C-assim displayed on current day (-9999: no limit) */ 
	double assim_opt2T;			/* (Celsius) opt2.temp. for C-assim displayed on current day (-9999: no limit) */ 
	double assim_maxT;			/* (Celsius) max. temp. for C-assim displayed on current day (-9999: no limit) */
	double transfer_pdays;		/* (prop) fraction of growth period for transfer */
	double litfall_pdays;		/* (prop) fraction of growth period for litfall */
    double nonwoody_turnover;	/* (1/yr) non-woody biomass turnover fraction */
    double woody_turnover;		/* (1/yr) woody biomass turnover fraction */
    double daily_mortality_turnover;	/* (1/day) daily mortality turnover */
    double daily_fire_turnover;			/* (1/day) daily fire turnover */
	double phenophase_length[N_PHENPHASES];				/* (Celsius) length of phenphase (critical heatsums) */
    double alloc_leafc[N_PHENPHASES];					/* (ratio) new leaf C to new C */
	double alloc_frootc[N_PHENPHASES];					/* (ratio) new fine root C to new C */
	double alloc_fruitc[N_PHENPHASES];					/* (ratio) new fruit C to new C */
	double alloc_softstemc[N_PHENPHASES];				/* (ratio) new softstem c to new  C */
	double alloc_livestemc[N_PHENPHASES];				/* (ratio) new live stem C to new C */
	double alloc_deadstemc[N_PHENPHASES];				/* (ratio) new dead stem C to new C */
    double alloc_livecrootc[N_PHENPHASES];				/* (ratio) new live croot C to new  C */
	double alloc_deadcrootc[N_PHENPHASES];				/* (ratio) new dead croot C to new  C */
	double avg_proj_sla[N_PHENPHASES];					/* (m2/kgC) canopy average proj. SLA in a given phenphase */
    double curgrowth_prop[N_PHENPHASES];			    /* (prop) daily allocation to current growth */
    double sla_ratio;      /* (ratio) ratio of shaded to sunlit projected SLA */
    double lai_ratio;      /* (ratio) ratio of (all-sided LA / one-sided LA) */
    double int_coef;       /* (kg/kg/LAI/d) canopy precip interception coef */
    double ext_coef;       /* (dimless) canopy light extinction coefficient */
	double potRUE;         /* (g/MJ) potential radiation use efficiency */
	double rad_param1;     /* ((DIM) radiation parameter1 (Jiang et al.2015) */
	double rad_param2;     /* ((DIM) radiation parameter2 (Jiang et al.2015) */
    double flnr;           /* (kg NRub/kg Nleaf) leaf N in Rubisco */
	double flnp;           /* (kg PeP/kg Nleaf) fraction of leaf N in PEP Carboxylase */
	double relVWC_crit1;   /* (prop)  critical relative vwc to calc. soil moisture limitation 1 */
    double relVWC_crit2;   /* (prop)  critical relative vwc to calc. soil moisture limitation 2 */
	double PSI_crit1;	   /* (prop)  critical psi to calc. soil moisture limitation 1 */
    double PSI_crit2;	   /* (prop)  critical psi to calc. soil moisture limitation 2 */
	double vpd_open;       /* (Pa)  vpd at start of conductance reduction */
	double vpd_close;      /* (Pa)  vpd at complete conductance reduction */
    double gl_smax;        /* (m/s) maximum leaf-scale stomatal conductance */
    double gl_c;           /* (m/s) leaf-scale cuticular conductance */
	double gl_bl;          /* (m/s) leaf-scale boundary layer conductance */
    double froot_cn;       /* (kgC/kgN) C:N for fine roots */
    double leaf_cn;        /* (kgC/kgN) C:N for leaves */
	double fruit_cn;                /* (kgC/kgN) C:N for fruits */
    double softstem_cn;             /* (kgC/kgN) C:N for softstems */
    double livewood_cn;    /* (kgC/kgN) C:N for live wood */
	double deadwood_cn;    /* (kgC/kgN) C:N for dead wood */
    double leaflitr_cn;      /* (kgC/kgN) constant C:N for leaf litter */
    double leaflitr_flab;    /* (prop) leaf litter labile fraction */
    double leaflitr_fucel;   /* (prop) leaf litter unshielded cellulose fract. */
    double leaflitr_fscel;   /* (prop) leaf litter shielded cellulose fract. */
    double leaflitr_flig;    /* (prop) leaf litter lignin fraction */
    double frootlitr_flab;   /* (prop) froot litter labile fraction */
    double frootlitr_fucel;  /* (prop) froot litter unshielded cellulose fract */
    double frootlitr_fscel;  /* (prop) froot litter shielded cellulose fract */
    double frootlitr_flig;   /* (prop) froot litter lignin fraction */
	double fruitlitr_flab;          /* (prop) fruit litter labile fraction */
    double fruitlitr_fucel;         /* (prop) fruit litter unshielded cellulose fract. */
    double fruitlitr_fscel;         /* (prop) fruit litter shielded cellulose fract. */
    double fruitlitr_flig;          /* (prop) fruit litter lignin fraction */
	double softstemlitr_flab;       /* (prop) softstem litter labile fraction */
    double softstemlitr_fucel;      /* (prop) softstem litter unshielded cellulose fract. */
    double softstemlitr_fscel;      /* (prop) softstem litter shielded cellulose fract. */
    double softstemlitr_flig;       /* (prop) softstem litter lignin fraction */
    double deadwood_fucel;			/* (prop) dead wood unshileded cellulose fraction */
    double deadwood_fscel;			/* (prop) dead wood shielded cellulose fraction */
    double deadwood_flig;			/* (prop) dead wood lignin fraction */
	double SNSCmort_abovebiom_max;	/* (prop) maximum mortality of senescence of aboveground biomass */
	double SNSCmort_belowbiom_max;	/* (prop) maximum mortality of senescence of belowground biomass */
	double SNSCmort_nsc_max;        /* (prop) ratio of the non-stuctured plant materail senescence mortality  */
	double extremT_effect_max;      /* (prop) effect of extreme high temperature on senesncene mortality */
	double SNSC_extremT1;           /* (Ceslius) lower limit extreme high temperature effect on senesncene mortality */
	double SNSC_extremT2;           /* (Ceslius) upper limit extreme high temperature effect on senesncene mortality */
	double mort_MAX_lifetime;		/*  (prop) leaf phenology mortality parameter */
    double mort_SNSC_to_litter;		/*  (prop) turnover rate of wilted standing biomass to litter*/
	double mort_CnW_to_litter;		/*  (prop) turnover rate of cut-down non-woody biomass to litter*/
	double mort_CW_to_litter;		/*  (prop) turnover rate of cut-down woody biomass to litter*/
	double GR_ratio;				/*  (ratio) growth resp per unit of C grown */
	double denitr_coeff;			/*  (prop) denitrification coefficient - multilayer_sminn.c */
	double nitrif_coeff1;			/*  (prop) denitrification coefficient - multilayer_sminn.c */
	double nitrif_coeff2;			/*  (prop) denitrification coefficient - multilayer_sminn.c */
	double N2Ocoeff_nitrif;			/*  (prop) coefficient of N2O emission of nitrification */
	double NdepNH4_coeff;			/*  (prop) proprortion of NH4 flux of N-deposition */
	double NH4_mobilen_prop;		/*  (prop) fraction mineral N avail for plant.soil processes and leaching */
	double NO3_mobilen_prop;		/*  (prop) fraction mineral N avail for lant.soil processes and leaching */
	double nfix;					/*  (kgN/m2/yr) symbiotic+asymbiotic fixation of N */
	double rootlenght_par1;			/*  (dimless) rootlenght parameter 1 (maximal weight) */
	double rootlenght_par2;			/*  (dimless) rootlenght parameter 1 (slope) */
	double dsws_crit;				/*  (nday) critical value of soilstress days */
	double max_plant_height;        /* (m) maximum plant height */
	double max_stem_weight;         /* kgC/m2) stem weight at which maximum height attended (only woody biomass) */
	double max_rootzone_depth;		/* (m)   maximum depth of rooting zone */
	double rootdistrib_param;       /* (dimless) root distribution parameter (Jarvis 1989) */
	double c_param_tsoil;           /* (dimless) empirical parameter for TSOIL esimation */
	double mrpern;                  /* (kgC/kgN/d) maintenance respiration in kgC/day per kg of tissue N */
	double NSC_SC_prop;             /* (prop) theoretical maximum prop. of non-structural and structural carbohydrates */
	double NSC_avail_prop;		    /* (prop) ratio of non-structural carbohydrates available for maintanance respiration */
	double efolding_depth;          /* (m) e-folding depth of decomposition rate's depth scalar (Koven et al. 2013) */
	int n_germ_phenophase;	        /* (DIM) number of phenophase when emergence occurs */
	int n_emerg_phenophase;	        /* (DIM) number of phenophase when emergence occurs */
	int n_phpsl_phenophase;	        /* (DIM) number of phenophase when photoperiodic slowing effect occurs  */
	double phpsl_parDL;				/* (hour) critical photoslow daylength 1 */
	double phpsl_parDR;				/* (DIM) slope of relative photoslow development rate */
	int n_vern_phenophase;	        /* (DIM) number of phenophase when vernalization occurs  */
	double grmn_paramSWC;           /* (prop.) critical relative SWC (prop. to FC) in germination */
	double vern_parT1;              /* (Celsius) critical vernalization temperature 1 */
	double vern_parT2;              /* (Celsius) critical vernalization temperature 2 */
	double vern_parT3;              /* (Celsius) critical vernalization temperature 3 */
	double vern_parT4;              /* (Celsius) critical vernalization temperature 4 */
	double vern_parDR1;             /* (DIM) slope of relative vernalization development rate */
	double vern_parDR2;             /* (DIM) max of vernalization days */
	int n_flowHS_phenophase;        /* (DIM) number of phenophase of flowering heat stress */ 
	double flowHS_parT1;            /* (Celsius) critical flowering heat stress temperature 1 */
	double flowHS_parT2;            /* (Celsius) critical flowering heat stress temperature 1 */
	double flowHS_parMORT;          /* (prop.) mortality parameter of flowering heat stress */
	double SOIL1_dissolv_prop;      /* (prop) fraction of dissolved part of SOIL1 organic matter */
	double SOIL2_dissolv_prop;      /* (prop) fraction of dissolved part of SOIL2 organic matter  */
	double SOIL3_dissolv_prop;      /* (prop) fraction of dissolved part of SOIL3 organic matter */
	double SOIL4_dissolv_prop;      /* (prop) fraction of dissolved part of SOIL4 organic matter */
	double BSE_PE_prop;             /* (prop) ratio of bare soil evaporation and pot.evaporation */
	double rfl1s1;                  /* (prop) respiration fractions for fluxes between compartments  */
	double rfl2s2;                  /* (prop) respiration fractions for fluxes between compartments  */
	double rfl4s3;                  /* (prop) respiration fractions for fluxes between compartments  */
	double rfs1s2;                  /* (prop) respiration fractions for fluxes between compartments  */
	double rfs2s3;                  /* (prop) respiration fractions for fluxes between compartments  */
	double rfs3s4;                  /* (prop) respiration fractions for fluxes between compartments  */		
	double kl1_base;                /* (1/day) base values of rate constants   */    
	double kl2_base;                /* (1/day) base values of rate constants   */    
	double kl4_base;                /* (1/day) base values of rate constants   */     
	double ks1_base;                /* (1/day) base values of rate constants   */     
	double ks2_base;                /* (1/day) base values of rate constants   */     
	double ks3_base;                /* (1/day) base values of rate constants   */    
	double ks4_base;                /* (1/day) base values of rate constants   */    
	double kfrag_base;              /* (1/day) base values of rate constants   */  
	double C_pBD1;                  /* (dimless) bulk density parameter of empirical CH4 modeling */
	double C_pBD2;                  /* (dimless) bulk density parameter of empirical CH4 modeling */
	double C_pVWC1;                 /* (dimless) soil water content parameter of empirical CH4 modeling */
	double C_pVWC2;                 /* (dimless) soil water content parameter of empirical CH4 modeling */
	double C_pVWC3;                 /* (dimless) soil water content parameter of empirical CH4 modeling */
	double C_pVWC4;                 /* (dimless) soil water content parameter of empirical CH4 modeling */
	double C_pTS;                   /* (dimless) soil temperature parameter of empirical CH4 modeling */
	double* wpm_array;				/* (flag) ARRAY of changing WPM flag */
	double* msc_array;				/* (flag) ARRAY of changing MSC flag */
	double* sgs_array;				/* (flag) ARRAY of changing SGS flag */
	double* egs_array;				/* (flag) ARRAY of changing EGS flag */
} epconst_struct;
/* endVAR */

/* varTHN: strucure for thinning paramteres */
typedef struct
{
	int THN_flag;							/* (flag) 1=do thinning , 0=no thinning */
	int mgmd;								/* (flag) 1=do management , 0=no management on actual day */
	double** THNdays_array;					/* (doy)  ARRAY of thinning days in 1 year*/
	double** thinningrate_w_array;			/* (prop) ARRAY of proportion of the thinned woody plant biomass */
	double** thinningrate_nw_array;			/* (prop) ARRAY of proportion of the thinned non-woody plant biomass */
	double** transpcoeff_w_array;		    /* (%) ARRAY of rate of the transported woody matter */
	double** transpcoeff_nw_array;		    /* (%) ARRAY of rate of the transported non woody matter */
} thinning_struct;
/* endVAR */

/* VAR MOW: strucure for mowing paramteres  */
typedef struct
{
	int MOW_flag;								/* (flag) 1=do mowing , 0=no mowing */
	int mgmd;									/* (flag) 1=do management , 0=no management on actual day */
	int fixday_or_fixLAI_flag;					/* (flag) 0=fixed day method, 1=fix LAI method */
	double fixLAI_befMOW;						/* (value) LAI before mowing (fixvalue method)*/
	double fixLAI_aftMOW;						/* (value) LAI after mowing (fixvalue method) */
	double** MOWdays_array;						/* (doy) ARRAY of mowing days in 1 year (fixday method)*/
	double** LAI_limit_array;					/* (m2/m2) ARRAY of LAI after mowing (fixday method)*/
	double** transport_coeff_array;				/* (%) ARRAY of proportion of plant material transported away (fixday method)*/
} mowing_struct;
/* endVAR */

/* VAR HRV: strucure for harvesting paramteres */
typedef struct
{
	int HRV_flag;								/* (flag) 1=do harvesting , 0=no harvesting */
	int mgmd;									/* (flag) 1=do management , 0=no management on actual day */
    double** HRVdays_array;						/* (doy) ARRAY of  harvesting days in 1 year*/
	double** snag_array;				    	/* (kgC/m2/d) ARRAY of snag after harvesting*/
	double** transport_coeff_array;				/* (%) ARRAY of rate of plant material transported away */
} harvesting_struct;
/* endVAR */

/* VAR PLG: strucure for ploughing paramteres  */
typedef struct
{
	int PLG_flag;								/* (flag) 1=do plough , 0=no plough */
	int mgmd;									/* (flag) 1=do management , 0=no management on actual day */
	double** PLGdays_array;						/* (doy) ARRAY of contains the plough days in 1 year*/
	double** PLGdepths_array;					/* (dimless) ARRAY of ploughing depths in 1 year*/
	double** dissolv_coeff_array;				/* (prop) ARRAY of dissolving coefficient of cut plant material (to litter) */
	double PLG_pool_litr1c;						/* (kgC/m2) actual ploughing pool */
	double PLG_pool_litr2c;						/* (kgC/m2) actual ploughing pool */
	double PLG_pool_litr3c;						/* (kgC/m2) actual ploughing pool */
	double PLG_pool_litr4c;						/* (kgC/m2) actual ploughing pool */
	double PLG_pool_litr1n;						/* (kgC/m2) actual ploughing pool */
	double PLG_pool_litr2n;						/* (kgC/m2) actual ploughing pool */
	double PLG_pool_litr3n;						/* (kgC/m2) actual ploughing pool */
	double PLG_pool_litr4n;						/* (kgC/m2) actual ploughing pool */
	double DC_act;				                /* (prop) dissolving coefficient of cut plant material (to litter) */
} ploughing_struct;
/* endVAR */

/* VAR GRZ: strucure for grazing paramteres */
typedef struct
{
	int GRZ_flag;								/* (flag) 1=do grazing , 0=no grazing */	
	int mgmd;									/* (flag) 0,1..6=do management (first, second...) , -1=no management on actual day */
	double** trampling_effect;					/* (%) ARRAY of trampling effect coefficient (standing dead biomass to litter) */
	double** GRZ_start_array;					/* (doy) ARRAY of beginning of grazing */
	double** GRZ_end_array;						/* (doy) ARRAY of end of grazing */
	double** weight_LSU;						/* (kg/LSU)) ARRAY of weight equivalent of an averaged animal */
	double** stocking_rate_array;				/* (LSU/ha) ARRAY of animal stocking rate: Livestock Units per hectare */
	double** DMintake_array;					/* (kgDM/m2/d) ARRAY of pasture forgage dry matter intake */
	double** prop_DMintake2excr_array;			/* (%) ARRAY of ratio of the dry matter intake formed excrement */
	double** prop_excr2litter_array;			/* (%) ARRAY of ratio of excrement return to litter */ 
	double** DM_Ccontent_array;					/* (%) ARRAY of carbon content of dry matter*/
	double** EXCR_Ncontent_array;				/* (%) ARRAY of nitrogen content of the fertilizer */
	double** EXCR_Ccontent_array;				/* (%) ARRAY of carbon content of the fertilizer */
	double trampleff_act;						/* (%) actual b (standing dead biomass to litter) */
	double** Nexrate;                           /* (kgN/1000 kg animal mass/day) ARRAY of default N excretion rate */
	double** EFman_N2O;							/* (kgN2O-N:kgN) ARRAY of manure emission factor for N2O emission	 */
	double** EFman_CH4;							/* (kgCH4/LSU/yr) ARRAY of manure emission factor for CH4 emission */
	double** EFfer_CH4;							/* (kgCH4/LSU/yr) ARRAY of fermentation emission factor for CH4 emission */
} grazing_struct;
/* endVAR */


/* VAR PLT: strucure for planting paramteres */
typedef struct
{
	int PLT_flag;								/* (flag) 1=do planting , 0=no planting */	
	int mgmd;									/* (flag) 1=do management , 0=no management on actual day */
    double** PLTdays_array;						/* (doy) ARRAY of contains the planting days in 1 year*/
	double** germ_depth_array;			        /* (m) ARRAY of germination_depth */
	double** n_seedlings_array;				    /* (n/m2) ARRAY of number of seedlings */
	double** weight_1000seed_array;				/* (g/1000n) ARRAY of specific weight of seed */
	double** seed_carbon_array;					/* (%) ARRAY of carbon content of seed*/
	double** emerg_rate_array;				   /* (%) ARRAY of emergence rate of the seed */
} planting_struct;
/* endVAR */

/* VAR FRZ: strucure for fertilizing paramteres  */
typedef struct
{
	int FRZ_flag;								/* (flag) 1=fertilizing , 0=no fertilizing */
	int mgmd;                                   /* (flag) 1=do management , 0=no management on actual day */
	double** FRZdays_array;						/* (doy) ARRAY of contains the fertilizing days in 1 year */
	double** FRZdepth_array;					/* (doy) ARRAY of contains the fertilizing depth */
	double** fertilizer_array;					/* (kg fertilizer/m2/d) ARRAY of amount of fertilizer on the fertilizing days */
	double** DM_array;				            /* (%) ARRAY of dry matter of fertilizer */
	double** NO3content_array;					/* (kgN/kg fertilizer) ARRAY of nitrate content of fertilizer */
 	double** NH4content_array;					/* (kgN/kg fertilizer) ARRAY of ammonium content of fertilizer */
	double** orgCcontent_array;					/* (kgC/kg fertilizer) ARRAY of organic carbon content of fertilizer */
	double** orgNcontent_array;					/* (kgC/kg fertilizer) ARRAY of organic nitrogen content of fertilizer */
	double** litr_flab_array;					/* (%) ARRAY of labile fraction of fertilizer */
	double** litr_fucel_array;					/* (%) ARRAY of unshielded cellulose fraction of fertilizer */
	double** litr_fscel_array;					/* (%) ARRAY of shielded cellulose fraction of fertilizer */
	double** litr_flig_array;					/* (%) ARRAY of lignin fraction of fertilizer */
	double** EFfert_N2O;						/* (kgN2O-N:kgN) ARRAY of fertilization emission factor for direct N2O emissions */
} fertilizing_struct;
/* endVAR */

/* VAR IRG: strucure for irrigation paramteres  */
typedef struct
{
	int IRG_flag;								/* (flag) 1=do irrigation , 0=no irrigation */	
	int mgmd;									/* (flag) 1=do management , 0=no management on actual day */
    double ** IRGdays_array;					/* (doy) ARRAY of rrigation days in 1 year*/
	double** IRGquantity_array;					/* (kgH2O/m2/d) ARRAY of quantity of water*/
	double** utiliz_coeff_array;				/* (%) ARRAY of useful part of the water */
} irrigation_struct;
/* endVAR */

/* OUT psn: structure for the photosynthesis routine */
typedef struct
{
	int c3;                 /* (flag) set to 1 for C3 model, 0 for C4 model */
	double pa;              /* (Pa) atmospheric pressure */
	double co2;             /* (ppm) atmospheric [CO2] */
	double t;               /* (Celsius) temperature */
	double lnc;             /* (kg Nleaf/m2) leaf N per unit sunlit leaf area */
	double flnr;            /* (kg NRub/kg Nleaf) fract. of leaf N in Rubisco */
	double flnp;            /* (kg NPep/kg Nleaf)fraction of leaf N in PEP Carboxylase */
	double ppfd;            /* (umol/m2/s) PAR flux per unit sunlit leaf area */
	double g;               /* (umol/m2/s/Pa) conductance to CO2 */
	double dlmr;            /* (umol/m2/s) day leaf m. resp, proj. area basis */
	double Ci;              /* (Pa) intercellular [CO2] */
	double O2;              /* (Pa) atmospheric [O2] */
	double Ca;              /* (Pa) atmospheric [CO2] */
	double gamma;           /* (Pa) CO2 compensation point, no Rd */
	double Kc;              /* (Pa) MM constant carboxylation */
	double Ko;              /* (Pa) MM constant oxygenation */
	double Vmax;            /* (umol/m2/s) max rate carboxylation */
	double Jmax;            /* (umol/m2/s) max rate electron transport */
	double J;               /* (umol/m2/s) rate of RuBP regeneration */
	double Av;              /* (umol/m2/s) carboxylation limited assimilation */
	double Aj;              /* (umol/m2/s) RuBP regen limited assimilation */
	double A;               /* (umol/m2/s) final assimilation rate */
} psn_struct;
/* endOUT */

/* VAR pmet_in: input structure for penmon function */ 
typedef struct
{
	double ta;              /* (Celsius) air temperature */
	double pa;              /* (Pa)   air pressure */
	double vpd;             /* (Pa)   vapor pressure deficit */
	double irad;            /* (W/m2) incident shortwave flux density */
	double rv;              /* (s/m)  resistance to water vapor flux */
	double rh;              /* (s/m)  resistance to sensible heat flux */
} pmet_struct;
/* endVAR */

/* OUT summary: structure for summarzing variables */
typedef struct
{
	double annprcp;                     /* (mm/yr) annual total precipitation */
	double anntavg;                     /* (Celsius) annual average air temperature */
	double annrunoff;                   /* (kgH2O/m2/yr) annual SUM of runoff */
	double annoutflow;                  /* (kgH2O/m2/yr) annual SUM of outflow */
	double daily_n2o;                   /* (kgN/m2/day)  daily N2O flux */
	double daily_nep;					/* (kgC/m2/day)  NPP - Rheretotrop */
	double daily_npp;					/* (kgC/m2/day)  GPP - Rmaint - Rgrowth */
	double daily_nee;					/* (kgC/m2/day)  GPP - Rmaint - Rgrowth - Rheretotrop - fire losses */
	double daily_nbp;					/* (kgC/m2/day)  GPP - Rmaint - Rgrowth - Rheretotrop - disturb_emission - fire losses*/
	double daily_gpp;					/* (kgC/m2/day)  gross PSN source */
	double daily_mr;					/* (kgC/m2/day)  maintenance respiration */
	double daily_gr;					/* (kgC/m2/day)  growth respiration */
	double daily_hr;					/* (kgC/m2/day)  heterotrophic respiration */
	double daily_sr;					/* (kgC/m2/day)  soil respiration */
	double daily_tr;					/* (kgC/m2/day)  total respiration */
	double daily_fire;					/* (kgC/m2/day)  fire losses */
	double daily_litfallc;				/* (kgC/m2/day)  total litterfall */
	double daily_litfallc_above;		/* (kgC/m2/day)  total litterfall aboveground */
	double daily_litfallc_below;		/* (kgC/m2/day)  total litterfall belowground */
	double daily_litdecomp;				/* (kgC/m2/day)  total litter decomposition */
	double daily_litfire;               /* (kgC/m2/day)  total litter fire mortality */
	double daily_litter;				/* (kgC/m2)  total amount of litter */
	double cum_npp;						/* (kgC/m2)  cumulative SUM of NPP */
	double cum_nep;						/* (kgC/m2)  cumulative SUM of NEP */
	double cum_nee;						/* (kgC/m2)  cumulative SUM of NEE */
	double cum_gpp;						/* (kgC/m2)  cumulative SUM of GPP */
	double cum_mr;						/* (kgC/m2)  cumulative SUM of MR */
	double cum_gr;						/* (kgC/m2)  cumulative SUM of GR */
	double cum_hr;						/* (kgC/m2)  cumulative SUM of HR */
	double cum_fire;					/* (kgC/m2)  cumulative SUM of fire mortality */
	double cum_n2o;						/* (kgN/m2)  cumulative SUM N2O flux */
	double cum_Closs_MGM;				/* (kgC/m2)  cumulative SUM of management carbon loss  */
	double cum_Cplus_MGM;				/* (kgC/m2)  cumulative SUM of management carbon plus  */
	double cum_Closs_THN_w;				/* (kgC/m2)  cumulative SUM of thinning woody carbon loss  */
	double cum_Closs_THN_nw;			/* (kgC/m2)  cumulative SUM of thinning non-woody carbon loss  */
	double cum_Closs_MOW;				/* (kgC/m2)  cumulative SUM of mowing carbon loss */
	double cum_Closs_HRV;				/* (kgC/m2)  cumulative SUM of harvesting carbon loss */
	double cum_yieldC_HRV;				/* (kgC/m2)  cumulative SUM of harvested yield */
	double cum_Closs_PLG;				/* (kgC/m2)  cumulative SUM of plouging carbon loss   */
	double cum_Closs_GRZ;				/* (kgC/m2)  cumulative SUM of grazing carbon loss   */
	double cum_Cplus_GRZ;				/* (kgC/m2)  cumulative SUM of grazing carbon plus   */
	double cum_Cplus_FRZ;				/* (kgC/m2)  cumulative SUM of fertilizing carbon plus */
	double cum_Cplus_PLT;				/* (kgC/m2)  cumulative SUM of planting carbon change  */
	double cum_Nplus_GRZ;				/* (kgN/m2)  cumulative SUM of grazing nitrogen plus   */
	double cum_Nplus_FRZ;				/* (kgN/m2)  cumulative SUM of fertilizing nitrogen plus   */
	double cum_Closs_SNSC;				/* (kgC/m2)  cumulative SUM of senescence carbon loss  */
	double cum_Cplus_STDB;				/* (kgC/m2)  cumulative SUM of standing dead biome carbon plus  */
	double cum_Cplus_CTDB;				/* (kgC/m2)  cumulative SUM of cut-down dead biome carbon plus  */
	double cum_ET;						/* (kgH2O/m2) cumulative SUM of evapotranspiration over a year */
	double vegc;						/* (kgC/m2)  total vegetation C */
	double abgc;						/* (kgC/m2)  total abovegound biomass C */
	double litrc;						/* (kgC/m2)  total litter C */
	double soilc;						/* (kgC/m2)  total soil C */
	double soiln;						/* (kgC/m2)  total soil N */
	double sminn;						/* (kgC/m2)  total soil mineralized N */
	double totalc;						/* (kgC/m2)  total of vegc, litrc, and soilc */
	double humusC_top10;				/* (gC/kgSOIL)  humus C content in 0-10 cm */
	double SOM_C_top10;					/* (gC/kgSOIL)  soil organic matter C content in 0-10 cm */
	double SOM_N_top10;					/* (gN/kgSOIL)  soil organic matter N content in 0-10 cm */
	double NH4_top10;					/* (mgN/kgSOIL)  soil NH4-content in 0-10 cm */
	double NO3_top10;					/* (mgN/kgSOIL)  soil NO3-content in 0-10 cm */
	double daily_gross_nmin_total;		/* (kgN/m2)  gross N mineralization   */
	double daily_gross_nimmob_total;	/* (kgN/m2)  gross N immobilization   */
	double daily_net_nmin_total;		/* (kgN/m2)  net N mineralization   */

} summary_struct;
/* endOUT */

/* VAR restart: restart data structure */
typedef struct
{
	double soilw[N_SOILLAYERS];		/* (kgH2O/m2) soil water content */
	double snoww;					/* (kgH2O/m2) snow water content */
	double canopyw;					/* (kgH2O/m2) canpoy water content */
	double leafc;					/* (kgC/m2) leaf carbon actual pool  */
	double leafc_storage;			/* (kgC/m2) leaf carbon storage pool  */
	double leafc_transfer;			/* (kgC/m2) leaf carbon transfer pool  */
	double frootc;					/* (kgC/m2) froot carbon actual pool  */
	double frootc_storage;			/* (kgC/m2) froot carbon storage pool  */
	double frootc_transfer;			/* (kgC/m2) froot carbon transfer pool  */
	double fruitc;					/* (kgC/m2) fruit carbon actual pool  */
	double fruitc_storage;			/* (kgC/m2) fruit carbon storage pool  */
	double fruitc_transfer;			/* (kgC/m2) fruit carbon transfer pool  */
	double softstemc;				/* (kgC/m2) softstem carbon actual pool  */
	double softstemc_storage;		/* (kgC/m2) softstem carbon storage pool  */
	double softstemc_transfer;		/* (kgC/m2) softstem carbon transfer pool  */
	double livestemc;				/* (kgC/m2) livestem carbon actual pool  */
	double livestemc_storage;		/* (kgC/m2) livestem carbon storage pool  */
	double livestemc_transfer;		/* (kgC/m2) livestem carbon transfer pool  */
	double deadstemc;				/* (kgC/m2) deadstem carbon actual pool  */
	double deadstemc_storage;		/* (kgC/m2) deadstem carbon storage pool  */
	double deadstemc_transfer;		/* (kgC/m2) deadstem carbon transfer pool  */
	double livecrootc;				/* (kgC/m2) livecroot carbon actual pool  */
	double livecrootc_storage;		/* (kgC/m2) livecroot carbon storage pool  */
	double livecrootc_transfer;		/* (kgC/m2) livecroot carbon transfer pool  */
	double deadcrootc;				/* (kgC/m2) deadcroot carbon actual pool  */
	double deadcrootc_storage;		/* (kgC/m2) deadcroot carbon storage pool  */
	double deadcrootc_transfer;		/* (kgC/m2) deadcroot carbon transfer pool  */ 
	double gresp_storage;			/* (kgC/m2) gresp carbon storage pool  */
	double gresp_transfer;			/* (kgC/m2) gresp carbon transfer pool  */
	double cwdc[N_SOILLAYERS];		/* (kgC/m2) coarse woody debris carbon pool  */
	double litr1c[N_SOILLAYERS];	/* (kgC/m2) labile litter carbon pool  */
	double litr2c[N_SOILLAYERS];	/* (kgC/m2) litter unshielded cellulose carbon pool  */
	double litr3c[N_SOILLAYERS];	/* (kgC/m2) litter shielded cellulose carbon pool  */
	double litr4c[N_SOILLAYERS];	/* (kgC/m2) litter lignin carbon pool  */
	double STDBc_leaf;				/*  wilted leaf biomass  */
	double STDBc_froot;				/*  wilted froot biomass  */
	double STDBc_fruit;				/*  wilted fruit biomass  */
	double STDBc_softstem;			/*  wilted sofstem biomass  */
	double STDBc_transfer;			/*  wilted transfer biomass  */
	double CTDBc_leaf;				/*  cut-down leaf biomass  */
	double CTDBc_froot;				/*  cut-down froot biomass  */
	double CTDBc_fruit;				/*  cut-down fruit biomass  */
	double CTDBc_softstem;			/*  cut-down sofstem biomass  */
	double CTDBc_transfer;			/*  cut-down transfer biomass  */
	double CTDBc_cstem;			    /*  cut-down coarse stem biomass  */
	double CTDBc_croot;			    /*  cut-down coarse root biomass  */
	double soil1c[N_SOILLAYERS];	/* (kgC/m2) microbial recycling pool C (fast) */
	double soil2c[N_SOILLAYERS];	/* (kgC/m2) microbial recycling pool C (medium) */
	double soil3c[N_SOILLAYERS];	/* (kgC/m2) microbial recycling pool C (medium) */ 
	double soil4c[N_SOILLAYERS];	/* (kgC/m2) recalcitrant SOM C (humus, slowest) */
	double cpool;					/* (kgC/m2) temporary photosynthate C pool */
	double leafn;					/* (kgN/m2) leaf carbon actual pool  */
	double leafn_storage;			/* (kgN/m2) leaf carbon storage pool  */
	double leafn_transfer;			/* (kgN/m2) leaf carbon transfer pool  */
	double frootn;					/* (kgN/m2) froot carbon actual pool  */
	double frootn_storage;			/* (kgN/m2) froot carbon storage pool  */
	double frootn_transfer;			/* (kgN/m2) froot carbon transfer pool  */
	double fruitn;					/* (kgN/m2) fruit carbon actual pool  */
	double fruitn_storage;			/* (kgN/m2) fruit carbon storage pool  */
	double fruitn_transfer;			/* (kgN/m2) fruit carbon transfer pool  */
	double softstemn;				/* (kgN/m2) softstem carbon actual pool  */
	double softstemn_storage;		/* (kgN/m2) softstem carbon actual pool  */
	double softstemn_transfer;		/* (kgN/m2) softstem carbon actual pool  */
	double livestemn;				/* (kgN/m2) livestem carbon actual pool  */
	double livestemn_storage;		/* (kgN/m2) livestem carbon storage pool  */
	double livestemn_transfer;		/* (kgN/m2) livestem carbon transfer pool  */
	double deadstemn;				/* (kgN/m2) deadstem carbon actual pool  */
	double deadstemn_storage;		/* (kgN/m2) deadstem carbon storage pool  */
	double deadstemn_transfer;		/* (kgN/m2) deadstem carbon transfer pool  */
	double livecrootn;				/* (kgN/m2) livecroot carbon actual pool  */
	double livecrootn_storage;		/* (kgN/m2) livecroot carbon storage pool  */
	double livecrootn_transfer;		/* (kgN/m2) livecroot carbon transfer pool  */
	double deadcrootn;				/* (kgN/m2) deadcroot carbon actual pool  */
	double deadcrootn_storage;		/* (kgN/m2) deadcroot carbon storage pool  */
	double deadcrootn_transfer;		/* (kgN/m2) deadcroot carbon transfer pool  */
	double cwdn[N_SOILLAYERS];		/* (kgN/m2) coarse woody debris carbon pool  */
	double litr1n[N_SOILLAYERS];	/* (kgN/m2) labile litter carbon pool  */
	double litr2n[N_SOILLAYERS];	/* (kgN/m2) litter unshielded cellulose carbon pool  */
	double litr3n[N_SOILLAYERS];	/* (kgN/m2) litter shielded cellulose carbon pool  */
	double litr4n[N_SOILLAYERS];	/* (kgN/m2) litter lignin carbon pool  */
	double STDBn_leaf;				/*  wilted leaf biomass  */
	double STDBn_froot;				/*  wilted froot biomass  */
	double STDBn_fruit;				/*  wilted fruit biomass  */
	double STDBn_softstem;			/*  wilted sofstem biomass  */
	double STDBn_transfer;			/*  wilted transfer biomass  */
	double CTDBn_leaf;				/*  cut-down leaf biomass  */
	double CTDBn_froot;				/*  cut-down froot biomass  */
	double CTDBn_fruit;				/*  cut-down fruit biomass  */
	double CTDBn_softstem;			/*  cut-down sofstem biomass  */
	double CTDBn_transfer;			/*  cut-down transfer biomass  */
	double CTDBn_cstem;			    /*  cut-down coarse stem biomass  */
	double CTDBn_croot;			    /*  cut-down coarse root biomass  */
	double soil1n[N_SOILLAYERS];	/* (kgN/m2) microbial recycling pool carbon (fast) */
	double soil2n[N_SOILLAYERS];	/* (kgN/m2) microbial recycling pool carbon (medium) */
	double soil3n[N_SOILLAYERS];	/* (kgN/m2) microbial recycling pool carbon (slow) */ 
	double soil4n[N_SOILLAYERS];	/* (kgN/m2) recalcitrant SOM carbon (humus, slowest) */
	double retransn;				/* (kgN/m2) retranslocated N */
	double npool;					/* (kgN/m2) temporary plant N pool */
	double sminNH4[N_SOILLAYERS];	/* (kgN/m2) mineralized NH4 pool*/
	double sminNO3[N_SOILLAYERS];	/* (kgN/m2) mineralized NO3 pool */
	double annmax_leafc;			/* (kgC/m2) annual maximum daily leaf carbon */
	double annmax_frootc;			/* (kgC/m2) annual maximum daily froot carbon */
	double annmax_fruitc;			/* (kgC/m2) annual maximum daily fruit carbon  */
	double annmax_softstemc;		/* (kgC/m2) annual maximum daily softstem carbon  */
	double annmax_livestemc;		/* (kgC/m2) annual maximum daily livestem carbon */
	double annmax_livecrootc;		/* (kgC/m2) annual maximum daily livecroot carbon */

/* endVAR*/

} restart_data_struct;
