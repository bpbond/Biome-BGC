  /*
bgc_struct.h 
header file for structure definitions

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v4.0.1
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2016, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Modified:
4/17/2000 (PET): Added new nf structure element (sminn_to_denitrif). This is
part of a larger modification that increases denitrification in the presence
of excess soil mineral N.
Modified:
13/07/2000: Added input of Ndep from file => added new ndep structure
element ndep_array (array). Changes are made by Galina Churkina adn Dora Hidy (2014)
*/

#define N_POOLS 3			/* Hidy 2010 - number of type of pools: water, carbon, nitrogen */
#define N_MGMDAYS 7			/* Hidy 2013 - number of type of management events in a single year */
#define N_SOILLAYERS 7		/* Hidy 2013 - number of type of soil layers in multilayer soil module */


/* simulation control variables */
typedef struct
{
	int metyears;          /* # years of met data */
    int simyears;          /* # years of simulation */
    int simstartyear;      /* first year of simulation */
	int spinup;            /* (flag) 1=spinup run, 0=normal run */
	int maxspinyears;      /* maximum number of years for spinup run */
	int dodaily;           /* flag for daily output */
	int domonavg;          /* flag for monthly average of daily outputs */
	int doannavg;          /* flag for annual average of daily outputs */
	int doannual;          /* flag for annual output */
	int ndayout;           /* number of daily outputs */
	int nannout;           /* number of annual outputs */
	int* daycodes;         /* array of indices for daily outputs */
	int* anncodes;         /* array of indices for annual outputs */
	int read_restart;      /* flag to read restart file */
	int write_restart;     /* flag to write restart file */
	int keep_metyr;        /* (flag) 1=retain restart metyr, 0=reset metyr */
	int onscreen;          /* (flag) 1=show progress on-screen 0=don't */

	/*  !!!!!!!!!!!!!!!!!! Hidy 2009 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */	


	int GSI_flag;		/*(flag) 1=if onday and offday calc. is not user-defined (epc file), use GSI_calculation to determine onday&offday*/
	int THN_flag;
	int MOW_flag;
	int GRZ_flag;
	int HRV_flag;
	int PLG_flag;
	int PLT_flag;
	int FRZ_flag;
	int IRG_flag;
	int simyr;			     /* Hidy 2010 - counter */
	int yday;			     /* Hidy 2010 - counter */
	int spinyears;		     /* Hidy 2010 - counter */
	int varWPM_flag;	     /* Hidy 2012 - changing WPM value */
	int varMSC_flag;	     /* Hidy 2012 - changing MSC value */
	int varSGS_flag;	     /* Hidy 2012 - changing WPM value */
	int varEGS_flag;	     /* Hidy 2012 - changing WPM value */
	int GWD_flag;			 /* Hidy 2012 - using gorundwater depth */
} control_struct;

/* a structure to hold information about varied N-deposition scenario */
typedef struct
{
	int varndep;         	/* (flag) 1=use file Ndep, 0=constant Ndep */
	double ndep;			/* (kgN/m2/yr) wet+dry atmospheric deposition of N */
	double* ndep_array;	   /* (kgN/m2/yr) annual Ndep array*/
} ndep_control_struct;

/* a structure to hold information on the annual co2 concentration */
typedef struct
{
	int varco2;             /* (flag) 0=const 1=use file  */
    double co2ppm;          /* (ppm)     constant CO2 concentration */
	double* co2ppm_array;   /* (ppm)     annual CO2 concentration array */
} co2control_struct;	

/* meteorological variable arrays */
/* inputs from mtclim, except tavg11, tavg30, tavg30_ra and tavg11_ra
which are used for an 11-day running average of daily average air T,
computed for the whole length of the met array prior to the 
daily model loop */
typedef struct
{
    double* tmax;          /* (deg C) daily maximum air temperature */
    double* tmin;          /* (deg C) daily minimum air temperature */
    double* prcp;          /* (cm)    precipitation */
    double* vpd;           /* (Pa)    vapor pressure deficit */
    double* swavgfd;       /* (W/m2)  daylight avg shortwave flux density */
    double* par;           /* (W/m2)  photosynthetically active radiation */
    double* dayl;          /* (s)     daylength */
	double* tday;          /* (deg C) daylight average temperature */
	double* tavg;          /* (deg C) daily average temperature */
    double* tavg11_ra;       /* (deg C) 11-day running avg of daily avg temp */
	double* tavg10_ra;       /* (deg C) 10-day running avg of daily avg temp */
    double* tavg30_ra;       /* (deg C) 30-day running avg of daily avg temp */
	double* F_temprad;		/* (dimless) soil temperature factor of radtiation and air temperature  */
	double* F_temprad_ra;  /* (dimless) 5-day running avg soil temperature factor of radtiation and air temperature  */
} metarr_struct;


/* daily values that are passed to daily model subroutines */
typedef struct
{
	double prcp;							 /* (kg/m2) precipitation */
	double tmax;							 /* (deg C) daily maximum air temperature */
	double tmin;							 /* (deg C) daily minimum air temperature */
	double tavg;							 /* (deg C) daily average air temperature */
	double tday;							 /* (deg C) daylight average air temperature */
	double tnight;							 /* (deg C) nightime average air temperature */
	double tavg11_ra;						 /* (deg C) Hidy 2010 - 11-day running average air temperature (linear weighted) */
	double tavg10_ra;						 /* (deg C) Hidy 2015 - 10-day running average air temperature */
	double tavg30_ra;						 /* (deg C) Hidy 2015 - 30-day running average air temperature */
	double F_temprad;				         /* (dimless)soil temperature factor of radtiation and air temperature  */
	double F_temprad_ra;				     /* (dimless) 5-day running avg soil temperature factor of radtiation and air temperature  */
	double tday_pre;						 /* (deg C) Hidy 2010 - daily average air temperature on the previous day */
	double tsoil_surface;				   	 /* (deg C)  surface temperature  Hidy 2015 */
	double tsoil[N_SOILLAYERS];				 /* (deg C) Hidy 2010 - daily soil layer temperature */
	double tsoil_avg;						 /* (deg C) Hidy 2010 - average soil temperature */
	double tsoil_surface_pre;			     /* (deg C) Hidy 2010 -  daily soil surface temperature of the previous day */
	double vpd;								 /* (Pa)    vapor pressure deficit */
	double swavgfd;							 /* (W/m2)  daylight average shortwave flux */
	double swabs;							 /* (W/m2)  canopy absorbed shortwave flux */ 
	double swtrans;							 /* (W/m2)  transmitted shortwave flux */
	double swabs_per_plaisun;				 /* (W/m2) swabs per unit sunlit proj LAI */
	double swabs_per_plaishade;				 /* (W/m2) swabs per unit shaded proj LAI */
	double ppfd_per_plaisun;				 /* (umol/m2/s) ppfd per unit sunlit proj LAI */
	double ppfd_per_plaishade;				 /* (umol/m2/s) ppfd per unit shaded proj LAI */
	double par;								 /* (W/m2)  photosynthetically active radiation */
	double parabs;							 /* (W/m2)  PAR absorbed by canopy */
	double pa;								 /* (Pa)    atmospheric pressure */
	double co2;								 /* (ppm)   atmospheric concentration of CO2 */
	double dayl;							 /* (s)     daylength */
	double GDD;								 /* (Celsius) growing degree day */
} metvar_struct;

/* water state variables (including sums for sources and sinks) */
typedef struct
{
    double soilw[N_SOILLAYERS];		 /* (kgH2O/m2) water stored in the soil layers; Hidy 2010 - Hidy 2010: multilayer soil */
	double soilw_SUM;				 /* (kgH2O/m2) water stored in soil; Hidy 2010 - Hidy 2010: multilayer soil */
	double pond_water;				 /* (kgH2O/m2) water stored on surface which can not infiltrated because of saturation - Hidy 2010 - Hidy 2010: multilayer soil */
    double snoww;					 /* (kgH2O/m2) water stored in snowpack */
    double canopyw;					 /* (kgH2O/m2) water stored on canopy */
    double prcp_src;				 /* (kgH2O/m2) SUM of precipitation */
    double soilevap_snk;			 /* (kgH2O/m2) SUM of soil water evaporation */
    double snowsubl_snk;			 /* (kgH2O/m2) SUM of snow water sublimation */
    double canopyevap_snk;			 /* (kgH2O/m2) SUM of canopy water evaporation */
	double pondwevap_snk;		 /* (kgH2O/m2) SUM of pond water evaporation */
    double trans_snk;				 /* (kgH2O/m2) SUM of transpiration */
	/* soil-water submodel - by Hidy 2008. */
	double runoff_snk;			 	 /* (kgH2O/m2) SUM of runoff */
	double deeppercolation_snk;		 /* (kgH2O/m2) SUM of percolated water out of the system */
	double deepdiffusion_snk;		 /* (kgH2O/m2) SUM of percolated water out of the system */
	double deeptrans_src;			 /* (kgH2O/m2) SUM of transpirated water from the bottom layer */
	double groundwater_src;			 /* (kgH2O/m2) SUM of water plus from goundwater */
    double pondwater_src;			 /* (kgH2O/m2) SUM of water plus from goundwater */
	/* thinning - Hidy 2012. */
	double canopyw_THNsnk;				/* (kgH2O/m2) water stored on canopy is disappered because of thinning*/
	/* mowing - Hidy 2008. */
	double canopyw_MOWsnk;				/* (kgH2O/m2) water stored on canopy is disappered because of mowing*/
	/* harvesting - Hidy 2012. */
	double canopyw_HRVsnk;
	/* ploughing - Hidy 2012. */
	double canopyw_PLGsnk;				/* (kgH2O/m2) water stored on canopy is disappered because of harvesting*/
	/* grazing - Hidy 2009. */
	double canopyw_GRZsnk;				/* (kgH2O/m2) water stored on canopy is disappered because of grazing*/
	/* irrigation - by Hidy 2015. */
    double IRGsrc;						/* (kgH2O/m2) planted N */
    double balance;
} wstate_struct;

/* water flux variables */
typedef struct
{
    double prcp_to_canopyw;							/* (kgH2O/m2/d) interception on canopy */
    double prcp_to_soilw;							/* (kgH2O/m2/d) precip entering soilwater pool  -  Hidy 2010: on the top soil layer */
    double prcp_to_snoww;							/* (kgH2O/m2/d) snowpack accumulation */
	double prcp_to_runoff;						    /* (kgH2O/m2/d) Hidy 2010 - runoff flux */
    double canopyw_evap;							/* (kgH2O/m2/d) evaporation from canopy */
    double canopyw_to_soilw;						/* (kgH2O/m2/d) canopy drip and stemflow -  Hidy 2010: on the top soil layer */
	double pondw_evap;                              /* (kgH2O/m2/d) pond water evaporation -  Hidy 2014 */
	double snoww_subl;								/* (kgH2O/m2/d) sublimation from snowpack */
	double snoww_to_soilw;							/* (kgH2O/m2/d) melt from snowpack -  Hidy 2010: on the top soil layer */
    double soilw_evap;								/* (kgH2O/m2/d) evaporation from soil */
    double soilw_trans[N_SOILLAYERS];				/* (kgH2O/m2/d) Hidy 2010 - transpiration from the soil layers */
    double soilw_trans_SUM;	
	double evapotransp;								/* (kgH2O/m2/d) Hidy 2013 - total water evaporation (canopyw_evap+soilw_evap+soilw_trans+snoww_subl) */
    double prcp_to_pondw;
	double pondw_to_soilw;
	double soilw_to_pondw;
	double soilw_percolated[N_SOILLAYERS];		    /* (kgH2O/m2/d) Hidy 2010 - percolation fluxes between soil layers */
	double soilw_diffused[N_SOILLAYERS];			/* (kgH2O/m2/d) Hidy 2010 - diffusion flux between the soil layers (positive: downward) */
	double soilw_from_GW[N_SOILLAYERS];				/* (kgH2O/m2/d) Hidy 2013 - soil water pélus from ground water */
	/* thinning - by Hidy 2008. */
	double canopyw_to_THN;							/* (kgH2O/m2/d) water stored on canopy is disappered because of thinning*/
	/* mowing - by Hidy 2008. */
	double canopyw_to_MOW;							/* (kgH2O/m2/d) water stored on canopy is disappered because of mowing*/
	/* harvesting - by Hidy 2008.*/
	double canopyw_to_HRV;							/* (kgH2O/m2/d) water stored on canopy is disappered because of harvesting */
	/* ploughing - Hidy 2012. */
	double canopyw_to_PLG;							/* (kgH2O/m2/d) water stored on canopy is disappered because of ploughing */
	/* grazing - by Hidy 2009. */
	double canopyw_to_GRZ;							/* (kgH2O/m2/d) water stored on canopy is disappered because of grazing*/
	/* irrigation - Hidy 2015 */
	double IRG_to_prcp;								/* (kgH2O/m2/d) irrigatied water amount*/	
	double pot_evap;                                 /* potential evaporation to calcualte pond evaporation */
} wflux_struct;

/* carbon state initialization structure */
typedef struct
{
	double max_leafc;      /* (kgC/m2) first-year displayed + stored leafc */
	double max_stemc;      /* (kgC/m2) first-year total stem carbon */
} cinit_struct;

/* carbon state variables (including sums for sources and sinks) */
typedef struct 
{
    double leafc;            /* (kgC/m2) leaf C */
    double leafc_storage;    /* (kgC/m2) leaf C storage */
    double leafc_transfer;   /* (kgC/m2) leaf C transfer */
    double frootc;           /* (kgC/m2) fine root C */
    double frootc_storage;   /* (kgC/m2) fine root C storage */
    double frootc_transfer;  /* (kgC/m2) fine root C transfer */
    double livestemc;        /* (kgC/m2) live stem C */
    double livestemc_storage;  /* (kgC/m2) live stem C storage */
    double livestemc_transfer; /* (kgC/m2) live stem C transfer */
    double deadstemc;          /* (kgC/m2) dead stem C */
    double deadstemc_storage;  /* (kgC/m2) dead stem C storage */
    double deadstemc_transfer; /* (kgC/m2) dead stem C transfer */
    double livecrootc;         /* (kgC/m2) live coarse root C */
    double livecrootc_storage; /* (kgC/m2) live coarse root C storage */
    double livecrootc_transfer;/* (kgC/m2) live coarse root C transfer */
    double deadcrootc;         /* (kgC/m2) dead coarse root C */
    double deadcrootc_storage; /* (kgC/m2) dead coarse root C storage */
    double deadcrootc_transfer;/* (kgC/m2) dead coarse root C transfer */
	double gresp_storage;    /* (kgC/m2) growth respiration storage */
	double gresp_transfer;   /* (kgC/m2) growth respiration transfer */
    double cwdc;             /* (kgC/m2) coarse woody debris C */
    double litr1c;           /* (kgC/m2) litter labile C */
    double litr2c;           /* (kgC/m2) litter unshielded cellulose C */
    double litr3c;           /* (kgC/m2) litter shielded cellulose C */
    double litr4c;           /* (kgC/m2) litter lignin C */
	double litr_aboveground; /* (kgC/m2) total belowground litter (due to ploughing) - Hidy 2015*/
	double litr_belowground; /* (kgC/m2) total belowground litter (due to ploughing) - Hidy 2015*/
	/* Hidy 2013: senescence */
	double litr1c_STDB; /* (kgC/m2)  amount of wilted plant biomass before turning into litter pool */
	double litr2c_STDB; /* (kgC/m2)  amount of wilted plant biomass before turning into litter pool */
	double litr3c_STDB; /* (kgC/m2)  amount of wilted plant biomass before turning into litter pool */
	double litr4c_STDB; /* (kgC/m2)  amount of wilted plant biomass before turning into litter pool */
	double STDBc;/* (kgC/m2)  amount of standing dead biomass */
	double CTDBc;/* (kgC/m2)  amount of cut-down dead biomass */
    double soil1c;           /* (kgC/m2) microbial recycling pool C (fast) */
    double soil2c;           /* (kgC/m2) microbial recycling pool C (medium) */
    double soil3c;           /* (kgC/m2) microbial recycling pool C (slow) */
    double soil4c;           /* (kgC/m2) recalcitrant SOM C (humus, slowest) */
	double cpool;            /* (kgC/m2) temporary photosynthate C pool */
    double psnsun_src;       /* (kgC/m2) SUM of gross PSN from sulit canopy */
    double psnshade_src;     /* (kgC/m2) SUM of gross PSN from shaded canopy */
    double leaf_mr_snk;      /* (kgC/m2) SUM of leaf maint resp */
    double leaf_gr_snk;      /* (kgC/m2) SUM of leaf growth resp */
    double froot_mr_snk;     /* (kgC/m2) SUM of fine root maint resp */
    double froot_gr_snk;     /* (kgC/m2) SUM of fine root growth resp */
    double livestem_mr_snk;  /* (kgC/m2) SUM of live stem maint resp */
    double livestem_gr_snk;  /* (kgC/m2) SUM of live stem growth resp */
    double deadstem_gr_snk;  /* (kgC/m2) SUM of dead stem growth resp */
    double livecroot_mr_snk; /* (kgC/m2) SUM of live coarse root maint resp */
    double livecroot_gr_snk; /* (kgC/m2) SUM of live coarse root growth resp */
    double deadcroot_gr_snk; /* (kgC/m2) SUM of dead coarse root growth resp */
    double litr1_hr_snk;     /* (kgC/m2) SUM of labile litr microbial resp */
    double litr2_hr_snk;     /* (kgC/m2) SUM of cellulose litr microbial resp */
    double litr4_hr_snk;     /* (kgC/m2) SUM of lignin litr microbial resp */
    double soil1_hr_snk;     /* (kgC/m2) SUM of fast microbial respiration */
    double soil2_hr_snk;     /* (kgC/m2) SUM of medium microbial respiration */
    double soil3_hr_snk;     /* (kgC/m2) SUM of slow microbial respiration */
    double soil4_hr_snk;     /* (kgC/m2) SUM of recalcitrant SOM respiration */
	double fire_snk;         /* (kgC/m2) SUM of fire losses */
	double SNSCsnk;		 /* (kgC/m2) SUM of senescence losses */
	double SNSCsrc;		 /* (kgC/m2) SUM of wilted plant material which turns into the litter pool */
	/* planting - by Hidy 2012. */
    double PLTsrc;     /* (kgN/m2) planted N */
	/* thinning - by Hidy 2012.  */
	double THNsnk;              /* (kgC/m2) thinned leaf C */
	double THNsrc;			    /* (kgC/m2) thinned plant material (C content) returns to the litter  */
	double THN_transportC;      /* (kgC/m2) thinned and transported plant material (C content)  */
	double litr1c_strg_THN;     /* (kgC/m2) amount of thinned plant biomass before turning into litter pool */
	double litr2c_strg_THN;		/* (kgC/m2) amount of thinned plant biomass before turning into litter pool */
	double litr3c_strg_THN;		/* (kgC/m2) amount of thinned plant biomass before turning into litter pool */
	double litr4c_strg_THN;		/* (kgC/m2) amount of thinned plant biomass before turning into litter pool */
	double cwdc_strg_THN;		/* (kgC/m2) amount of thinned plant biomass before turning into litter pool */
	/* mowing - by Hidy 2008.   */
	double MOWsnk;              /* (kgC/m2) mowed leaf C */
	double MOWsrc;			    /* (kgC/m2) mowed plant material (C content) returns to the litter */
	double MOW_transportC;      /* (kgC/m2) mowed and transported plant material (C content)  */
	double litr1c_strg_MOW;     /* (kgC/m2) amount of mowed plant biomass before turning into litter pool */
	double litr2c_strg_MOW;		/* (kgC/m2) amount of mowed plant biomass before turning into litter pool */
	double litr3c_strg_MOW;		/* (kgC/m2) amount of mowed plant biomass before turning into litter pool */
	double litr4c_strg_MOW;		/* (kgC/m2) amount of mowed plant biomass before turning into litter pool */
	/* harvesting - Hidy 2012.  */
	double HRVsnk;              /* (kgC/m2) harvested leaf C */
	double HRVsrc;			    /* (kgC/m2) harvested plant material (C content) returns to the litter */
	double HRV_transportC;      /* (kgC/m2) harvested and transported plant material (C content)  */
	double litr1c_strg_HRV;     /* (kgC/m2)  amount of harvested plant biomass before turning into litter pool */
	double litr2c_strg_HRV;		/* (kgC/m2)  amount of harvested plant biomass before turning into litter pool */
	double litr3c_strg_HRV;		/* (kgC/m2)  amount of harvested plant biomass before turning into litter pool */
	double litr4c_strg_HRV;		/* (kgC/m2)  amount of harvested plant biomass before turning into litter pool */
	/* ploughing - Hidy 2012.   */
	double PLGsnk;              /* (kgC/m2) ploughed leaf C */
	double PLGsrc;			    /* (kgC/m2) ploughed plant material (C content)to the soil (labile litter) */
	double PLG_cpool;			/* (kgC/m2) temporary pool of ploughed plant material (C content) before entering litter  */
	/* grazing - by Hidy 2009.  */
	double GRZsnk;              /* (kgC/m2) grazed leaf C */
	double GRZsrc;              /* (kgC/m2) added carbon from fertilizer */
	/* fertilizing - Hidy 2009. */
	double FRZsrc;			    /* (kgC/m2) carbon content of fertilizer return to the litter pool */
	/* fruit simulation - Hidy 2009. */
	 double fruitc;             /* (kgC/m2) SUM of fruitc */
     double fruitc_storage;     /* (kgC/m2) SUM of fruitc */
     double fruitc_transfer;    /* (kgC/m2) SUM of fruitc */
	 double fruit_gr_snk;       /* (kgC/m2) SUM of fruit growth resp. */
	 double fruit_mr_snk;       /* (kgC/m2) SUM of fruit maint resp.*/
	 /* soft stem simulation - Hidy 2015. */
	 double softstemc;             /* (kgC/m2) SUM of softstemc */
     double softstemc_storage;     /* (kgC/m2) SUM of softstemc */
     double softstemc_transfer;    /* (kgC/m2) SUM of softstemc */
	 double softstem_gr_snk;       /* (kgC/m2) SUM of softstem growth resp. */
	 double softstem_mr_snk;       /* (kgC/m2) SUM of softstem maint resp.*/
         double balance;
} cstate_struct;

/* daily carbon flux variables */
typedef struct
{
	/* mortality fluxes */ 
	double m_leafc_to_litr1c;              /* (kgC/m2/d) */            
	double m_leafc_to_litr2c;              /* (kgC/m2/d) */
	double m_leafc_to_litr3c;              /* (kgC/m2/d) */
	double m_leafc_to_litr4c;              /* (kgC/m2/d) */
	double m_frootc_to_litr1c;             /* (kgC/m2/d) */
	double m_frootc_to_litr2c;             /* (kgC/m2/d) */
	double m_frootc_to_litr3c;             /* (kgC/m2/d) */
	double m_frootc_to_litr4c;             /* (kgC/m2/d) */
	double m_leafc_storage_to_litr1c;      /* (kgC/m2/d) */
	double m_frootc_storage_to_litr1c;     /* (kgC/m2/d) */
	double m_livestemc_storage_to_litr1c;  /* (kgC/m2/d) */
	double m_deadstemc_storage_to_litr1c;  /* (kgC/m2/d) */
	double m_livecrootc_storage_to_litr1c; /* (kgC/m2/d) */
	double m_deadcrootc_storage_to_litr1c; /* (kgC/m2/d) */
	double m_leafc_transfer_to_litr1c;     /* (kgC/m2/d) */
	double m_frootc_transfer_to_litr1c;    /* (kgC/m2/d) */
	double m_livestemc_transfer_to_litr1c; /* (kgC/m2/d) */
	double m_deadstemc_transfer_to_litr1c; /* (kgC/m2/d) */
	double m_livecrootc_transfer_to_litr1c;/* (kgC/m2/d) */
	double m_deadcrootc_transfer_to_litr1c;/* (kgC/m2/d) */
	double m_livestemc_to_cwdc;            /* (kgC/m2/d) */
	double m_deadstemc_to_cwdc;            /* (kgC/m2/d) */
	double m_livecrootc_to_cwdc;           /* (kgC/m2/d) */
	double m_deadcrootc_to_cwdc;           /* (kgC/m2/d) */
	double m_gresp_storage_to_litr1c;      /* (kgC/m2/d) */
	double m_gresp_transfer_to_litr1c;     /* (kgC/m2/d) */
	/* mortality - SOFT STEM simulation -  by Hidy 2013. */
	double m_softstemc_to_litr1c;            /* (kgC/m2/d) */
	double m_softstemc_to_litr2c;            /* (kgC/m2/d) */
	double m_softstemc_to_litr3c;            /* (kgC/m2/d) */
	double m_softstemc_to_litr4c;            /* (kgC/m2/d) */
	double m_softstemc_storage_to_litr1c;     /* (kgC/m2/d) */
	double m_softstemc_transfer_to_litr1c;    /* (kgC/m2/d) */
	/* mortality - fruit simulation -  by Hidy 2013. */
	double m_fruitc_to_litr1c;             /* (kgC/m2/d) */
	double m_fruitc_to_litr2c;             /* (kgC/m2/d) */
	double m_fruitc_to_litr3c;             /* (kgC/m2/d) */
	double m_fruitc_to_litr4c;             /* (kgC/m2/d) */
	double m_fruitc_storage_to_litr1c;     /* (kgC/m2/d) */
	double m_fruitc_transfer_to_litr1c;    /* (kgC/m2/d) */
	/* fire fluxes */
	double m_leafc_to_fire;                /* (kgC/m2/d) */
	double m_frootc_to_fire;               /* (kgC/m2/d) */
	double m_leafc_storage_to_fire;        /* (kgC/m2/d) */
	double m_frootc_storage_to_fire;       /* (kgC/m2/d) */
	double m_livestemc_storage_to_fire;    /* (kgC/m2/d) */
	double m_deadstemc_storage_to_fire;    /* (kgC/m2/d) */
	double m_livecrootc_storage_to_fire;   /* (kgC/m2/d) */
	double m_deadcrootc_storage_to_fire;   /* (kgC/m2/d) */
	double m_leafc_transfer_to_fire;       /* (kgC/m2/d) */
	double m_frootc_transfer_to_fire;      /* (kgC/m2/d) */
	double m_livestemc_transfer_to_fire;   /* (kgC/m2/d) */
	double m_deadstemc_transfer_to_fire;   /* (kgC/m2/d) */
	double m_livecrootc_transfer_to_fire;  /* (kgC/m2/d) */
	double m_deadcrootc_transfer_to_fire;  /* (kgC/m2/d) */
	double m_livestemc_to_fire;            /* (kgC/m2/d) */
	double m_deadstemc_to_fire;            /* (kgC/m2/d) */
	double m_livecrootc_to_fire;           /* (kgC/m2/d) */
	double m_deadcrootc_to_fire;           /* (kgC/m2/d) */
	double m_gresp_storage_to_fire;        /* (kgC/m2/d) */
	double m_gresp_transfer_to_fire;       /* (kgC/m2/d) */
	double m_litr1c_to_fire;               /* (kgC/m2/d) */
	double m_litr2c_to_fire;               /* (kgC/m2/d) */
	double m_litr3c_to_fire;               /* (kgC/m2/d) */
	double m_litr4c_to_fire;               /* (kgC/m2/d) */
	double m_cwdc_to_fire;                 /* (kgC/m2/d) */
	/* fire - fruit simulation (Hidy 2013.) */
	double m_fruitc_to_fire;               /* (kgC/m2/d) */
	double m_fruitc_storage_to_fire;       /* (kgC/m2/d) */
	double m_fruitc_transfer_to_fire;      /* (kgC/m2/d) */
	/* fire - softstem simulation (Hidy 2015.) */
	double m_softstemc_to_fire;               /* (kgC/m2/d) */
	double m_softstemc_storage_to_fire;       /* (kgC/m2/d) */
	double m_softstemc_transfer_to_fire;      /* (kgC/m2/d) */
	/* Hidy 2010 - senescence fluxes */
	double m_vegc_to_SNSC;                /* (kgC/m2/d) */
	double m_leafc_to_SNSC;                /* (kgC/m2/d) */
	double m_frootc_to_SNSC;               /* (kgC/m2/d) */
	double m_leafc_storage_to_SNSC;                /* (kgC/m2/d) */
	double m_frootc_storage_to_SNSC;               /* (kgC/m2/d) */
	double m_leafc_transfer_to_SNSC;                /* (kgC/m2/d) */
	double m_frootc_transfer_to_SNSC;               /* (kgC/m2/d) */
	double m_gresp_storage_to_SNSC;
	double m_gresp_transfer_to_SNSC;
	double m_STDBc_to_SNSC;
	double SNSC_to_litr1c;
	double SNSC_to_litr2c;
	double SNSC_to_litr3c;
	double SNSC_to_litr4c;
	/* senescence fluxes  - fruit simulation (Hidy 2013.) */
	double m_fruitc_to_SNSC;                /* (kgC/m2/d) */
	double m_fruitc_storage_to_SNSC;        /* (kgC/m2/d) */
	double m_fruitc_transfer_to_SNSC;       /* (kgC/m2/d) */
	/* senescence fluxes  - softstem simulation (Hidy 2013.) */
	double m_softstemc_to_SNSC;                /* (kgC/m2/d) */
	double m_softstemc_storage_to_SNSC;        /* (kgC/m2/d) */
	double m_softstemc_transfer_to_SNSC;       /* (kgC/m2/d) */
	/* phenology fluxes from transfer pool */
	double leafc_transfer_to_leafc;          /* (kgC/m2/d) */
	double frootc_transfer_to_frootc;        /* (kgC/m2/d) */
	double livestemc_transfer_to_livestemc;  /* (kgC/m2/d) */
	double deadstemc_transfer_to_deadstemc;  /* (kgC/m2/d) */
	double livecrootc_transfer_to_livecrootc;/* (kgC/m2/d) */
	double deadcrootc_transfer_to_deadcrootc;/* (kgC/m2/d) */
	/* phenology fluxes - fruit simulation (Hidy 2013.) */
	double fruitc_transfer_to_fruitc;
	/* phenology fluxes - softstem simulation (Hidy 2013.) */
	double softstemc_transfer_to_softstemc;
	/* leaf and fine root litterfall */
	double leafc_to_litr1c;              /* (kgC/m2/d) */
	double leafc_to_litr2c;              /* (kgC/m2/d) */
	double leafc_to_litr3c;              /* (kgC/m2/d) */
	double leafc_to_litr4c;              /* (kgC/m2/d) */
	double frootc_to_litr1c;             /* (kgC/m2/d) */
	double frootc_to_litr2c;             /* (kgC/m2/d) */
	double frootc_to_litr3c;             /* (kgC/m2/d) */
	double frootc_to_litr4c;             /* (kgC/m2/d) */
	/* litterfall  - fruit simulation (Hidy 2013.) */
	double fruitc_to_litr1c;
	double fruitc_to_litr2c;
	double fruitc_to_litr3c;
	double fruitc_to_litr4c;
	/* litterfall  - softstem simulation (Hidy 2015.) */
	double softstemc_to_litr1c;
	double softstemc_to_litr2c;
	double softstemc_to_litr3c;
	double softstemc_to_litr4c;
	/* maintenance respiration fluxes */
	double leaf_day_mr;                  /* (kgC/m2/d) */
	double leaf_night_mr;                /* (kgC/m2/d) */
	double froot_mr;                     /* (kgC/m2/d) */
	double livestem_mr;                  /* (kgC/m2/d) */
	double livecroot_mr;                 /* (kgC/m2/d) */
	/* maintenance  - fruit simulation (Hidy 2013.) */
	double fruit_mr;   	
	/* maintenance  - softstem simulation (Hidy 2013.) */
	double softstem_mr;   	
	/* photosynthesis flux */
	double psnsun_to_cpool;              /* (kgC/m2/d) */
	double psnshade_to_cpool;            /* (kgC/m2/d) */
	/* litter decomposition fluxes */
	double cwdc_to_litr2c;               /* (kgC/m2/d) */
	double cwdc_to_litr3c;               /* (kgC/m2/d) */
	double cwdc_to_litr4c;               /* (kgC/m2/d) */
	double litr1_hr;                     /* (kgC/m2/d) */
	double litr1c_to_soil1c;             /* (kgC/m2/d) */
	double litr2_hr;                     /* (kgC/m2/d) */
	double litr2c_to_soil2c;             /* (kgC/m2/d) */
	double litr3c_to_litr2c;             /* (kgC/m2/d) */
	double litr4_hr;                     /* (kgC/m2/d) */
	double litr4c_to_soil3c;             /* (kgC/m2/d) */
	double soil1_hr;                     /* (kgC/m2/d) */
	double soil1c_to_soil2c;             /* (kgC/m2/d) */
	double soil2_hr;                     /* (kgC/m2/d) */
	double soil2c_to_soil3c;             /* (kgC/m2/d) */
	double soil3_hr;                     /* (kgC/m2/d) */
	double soil3c_to_soil4c;             /* (kgC/m2/d) */
	double soil4_hr;                     /* (kgC/m2/d) */
	/* daily allocation fluxes from current GPP */
	double cpool_to_leafc;               /* (kgC/m2/d) */
	double cpool_to_leafc_storage;       /* (kgC/m2/d) */
	double cpool_to_frootc;              /* (kgC/m2/d) */
	double cpool_to_frootc_storage;      /* (kgC/m2/d) */
	double cpool_to_livestemc;           /* (kgC/m2/d) */
	double cpool_to_livestemc_storage;   /* (kgC/m2/d) */
	double cpool_to_deadstemc;           /* (kgC/m2/d) */
	double cpool_to_deadstemc_storage;   /* (kgC/m2/d) */
	double cpool_to_livecrootc;          /* (kgC/m2/d) */
	double cpool_to_livecrootc_storage;  /* (kgC/m2/d) */
	double cpool_to_deadcrootc;          /* (kgC/m2/d) */
	double cpool_to_deadcrootc_storage;  /* (kgC/m2/d) */
	double cpool_to_gresp_storage;       /* (kgC/m2/d) */
	/* daily allocation fluxes  - fruit simulation (Hidy 2013.)  */
	double cpool_to_fruitc;                /* (kgC/m2/d) */
	double cpool_to_fruitc_storage;        /* (kgC/m2/d) */
	/* daily allocation fluxes  - soft stem simulation (Hidy 2013.)  */
	double cpool_to_softstemc;           /* (kgC/m2/d) */
	double cpool_to_softstemc_storage;   /* (kgC/m2/d) */
	/* daily growth respiration fluxes */
	double cpool_leaf_gr;                /* (kgC/m2/d) */
	double cpool_leaf_storage_gr;        /* (kgC/m2/d) */
	double transfer_leaf_gr;             /* (kgC/m2/d) */
	double cpool_froot_gr;               /* (kgC/m2/d) */
	double cpool_froot_storage_gr;       /* (kgC/m2/d) */
	double transfer_froot_gr;            /* (kgC/m2/d) */
	double cpool_livestem_gr;            /* (kgC/m2/d) */
	double cpool_livestem_storage_gr;    /* (kgC/m2/d) */
	double transfer_livestem_gr;         /* (kgC/m2/d) */
	double cpool_deadstem_gr;            /* (kgC/m2/d) */
	double cpool_deadstem_storage_gr;    /* (kgC/m2/d) */
	double transfer_deadstem_gr;         /* (kgC/m2/d) */
	double cpool_livecroot_gr;           /* (kgC/m2/d) */
	double cpool_livecroot_storage_gr;   /* (kgC/m2/d) */
	double transfer_livecroot_gr;        /* (kgC/m2/d) */
	double cpool_deadcroot_gr;           /* (kgC/m2/d) */
	double cpool_deadcroot_storage_gr;   /* (kgC/m2/d) */
	double transfer_deadcroot_gr;        /* (kgC/m2/d) */
	/* daily growth respiration  - fruit simulation (Hidy 2013.)  */
	double cpool_fruit_gr;                 /* (kgC/m2/d) */
	double cpool_fruit_storage_gr;         /* (kgC/m2/d) */
	double transfer_fruit_gr;             /* (kgC/m2/d) */
	/* daily growth respiration  - softstem simulation (Hidy 2015.)  */
	double cpool_softstem_gr;                 /* (kgC/m2/d) */
	double cpool_softstem_storage_gr;         /* (kgC/m2/d) */
	double transfer_softstem_gr;             /* (kgC/m2/d) */
	/* annual turnover of storage to transfer pools */
	double leafc_storage_to_leafc_transfer;           /* (kgC/m2/d) */
	double frootc_storage_to_frootc_transfer;         /* (kgC/m2/d) */
	double livestemc_storage_to_livestemc_transfer;    /* (kgC/m2/d) */
	double deadstemc_storage_to_deadstemc_transfer;    /* (kgC/m2/d) */
	double livecrootc_storage_to_livecrootc_transfer; /* (kgC/m2/d) */
	double deadcrootc_storage_to_deadcrootc_transfer; /* (kgC/m2/d) */
	double gresp_storage_to_gresp_transfer;           /* (kgC/m2/d) */
	/* annual turnover  - fruit simulation (Hidy 2013.)  */
	double fruitc_storage_to_fruitc_transfer;   /* (kgC/m2/d) */
	/* annual turnover  - softstem simulation (Hidy 2015.)  */
	double softstemc_storage_to_softstemc_transfer;   /* (kgC/m2/d) */
	/* turnover of live wood to dead wood */
	double livestemc_to_deadstemc;        /* (kgC/m2/d) */
	double livecrootc_to_deadcrootc;      /* (kgC/m2/d) */
	/* planting - Hidy 2012. */
	double leafc_transfer_from_PLT;		/* (kgC/m2/d) */
	double frootc_transfer_from_PLT;	/* (kgC/m2/d) */
	double fruitc_transfer_from_PLT;		/* (kgC/m2/d) */
	double softstemc_transfer_from_PLT;		/* (kgC/m2/d) */
	/* thinning - Hidy 2012. */
	double leafc_to_THN;				 /* (kgC/m2/d) */
	double leafc_storage_to_THN;         /* (kgC/m2/d) */
	double leafc_transfer_to_THN;        /* (kgC/m2/d) */
	double frootc_to_THN;				 /* (kgC/m2/d) */
	double frootc_storage_to_THN;         /* (kgC/m2/d) */
	double frootc_transfer_to_THN;        /* (kgC/m2/d) */
	double livecrootc_to_THN;				 /* (kgC/m2/d) */
	double livecrootc_storage_to_THN;         /* (kgC/m2/d) */
	double livecrootc_transfer_to_THN;        /* (kgC/m2/d) */
	double deadcrootc_to_THN;				 /* (kgC/m2/d) */
	double deadcrootc_storage_to_THN;         /* (kgC/m2/d) */
	double deadcrootc_transfer_to_THN;        /* (kgC/m2/d) */
	double livestemc_to_THN;				 /* (kgC/m2/d) */
	double livestemc_storage_to_THN;         /* (kgC/m2/d) */
	double livestemc_transfer_to_THN;        /* (kgC/m2/d) */
	double deadstemc_to_THN;				 /* (kgC/m2/d) */
	double deadstemc_storage_to_THN;         /* (kgC/m2/d) */
	double deadstemc_transfer_to_THN;        /* (kgC/m2/d) */
	double gresp_storage_to_THN;         /* (kgC/m2/d) */
	double gresp_transfer_to_THN;        /* (kgC/m2/d) */
	double THN_to_litr1c;				 /* (kgC/m2/d) */
	double THN_to_litr2c;				 /* (kgC/m2/d) */
	double THN_to_litr3c;				 /* (kgC/m2/d) */
	double THN_to_litr4c;				 /* (kgC/m2/d) */
	double THN_to_cwdc;			    	 /* (kgC/m2/d) */
	double STDBc_to_THN;
	/* thinning  - fruit simulation (Hidy 2013.)  */
	double fruitc_to_THN;				 /* (kgC/m2/d) */
	double fruitc_storage_to_THN;         /* (kgC/m2/d) */
	double fruitc_transfer_to_THN;        /* (kgC/m2/d) */
	/* mowing - Hidy 2008. */
	double leafc_to_MOW;				 /* (kgC/m2/d) */
	double leafc_storage_to_MOW;         /* (kgC/m2/d) */
	double leafc_transfer_to_MOW;        /* (kgC/m2/d) */
	double STDBc_to_MOW;
	double gresp_storage_to_MOW;         /* (kgC/m2/d) */
	double gresp_transfer_to_MOW;        /* (kgC/m2/d) */
	double MOW_to_litr1c;				 /* (kgC/m2/d) */
	double MOW_to_litr2c;				 /* (kgC/m2/d) */
	double MOW_to_litr3c;				 /* (kgC/m2/d) */
	double MOW_to_litr4c;				 /* (kgC/m2/d) */
	/* mowing  - fruit simulation (Hidy 2013.)  */
	double fruitc_to_MOW;				 /* (kgC/m2/d) */
	double fruitc_storage_to_MOW;         /* (kgC/m2/d) */
	double fruitc_transfer_to_MOW;        /* (kgC/m2/d) */
	/* mowing  - softstem simulation (Hidy 2013.)  */
	double softstemc_to_MOW;				 /* (kgC/m2/d) */
	double softstemc_storage_to_MOW;         /* (kgC/m2/d) */
	double softstemc_transfer_to_MOW;        /* (kgC/m2/d) */
	/* harvesting - Hidy 2012. */
	double leafc_to_HRV;				 /* (kgC/m2/d) */
	double leafc_storage_to_HRV;         /* (kgC/m2/d) */
	double leafc_transfer_to_HRV;        /* (kgC/m2/d) */
	double STDBc_to_HRV;
	double gresp_storage_to_HRV;         /* (kgC/m2/d) */
	double gresp_transfer_to_HRV;        /* (kgC/m2/d) */
	double HRV_to_litr1c;				 /* (kgC/m2/d) */
	double HRV_to_litr2c;				 /* (kgC/m2/d) */
	double HRV_to_litr3c;				 /* (kgC/m2/d) */
	double HRV_to_litr4c;				 /* (kgC/m2/d) */
	/* harvesting  - fruit simulation (Hidy 2013.)  */
	double fruitc_to_HRV;				 /* (kgC/m2/d) */
	double fruitc_storage_to_HRV;         /* (kgC/m2/d) */
	double fruitc_transfer_to_HRV;        /* (kgC/m2/d) */
	/* harvesting  - softstem simulation (Hidy 2013.)  */
	double softstemc_to_HRV;				 /* (kgC/m2/d) */
	double softstemc_storage_to_HRV;         /* (kgC/m2/d) */
	double softstemc_transfer_to_HRV;        /* (kgC/m2/d) */
	/* ploughing - Hidy 2012. */
	double leafc_to_PLG;					 /* (kgC/m2/d) */
	double leafc_storage_to_PLG;          /* (kgC/m2/d) */
	double leafc_transfer_to_PLG;         /* (kgC/m2/d) */
	double STDBc_to_PLG;				/* standing dead biome to PLG */
	double CTDBc_to_PLG;				/* cut-down biome to PLG */
	double gresp_storage_to_PLG;          /* (kgC/m2/d) */
	double gresp_transfer_to_PLG;         /* (kgC/m2/d) */
	double frootc_to_PLG;					/* (kgC/m2/d) */
	double frootc_storage_to_PLG;         /* (kgC/m2/d) */
	double frootc_transfer_to_PLG;        /* (kgC/m2/d) */
	double PLG_to_litr1c;
	double PLG_to_litr2c;
	double PLG_to_litr3c;
	double PLG_to_litr4c;
	/* ploughing  - fruit simulation (Hidy 2013.)  */
	double fruitc_to_PLG;				 /* (kgC/m2/d) */
	double fruitc_storage_to_PLG;         /* (kgC/m2/d) */
	double fruitc_transfer_to_PLG;        /* (kgC/m2/d) */
	/* ploughing  - softstem simulation (Hidy 2013.)  */
	double softstemc_to_PLG;				 /* (kgC/m2/d) */
	double softstemc_storage_to_PLG;         /* (kgC/m2/d) */
	double softstemc_transfer_to_PLG;        /* (kgC/m2/d) */
	/* grazing - Hidy 2009. */
	double leafc_to_GRZ;					/* (kgC/m2/d) */
	double leafc_storage_to_GRZ;         /* (kgC/m2/d) */
	double leafc_transfer_to_GRZ;        /* (kgC/m2/d) */
	double gresp_storage_to_GRZ;         /* (kgC/m2/d) */
	double gresp_transfer_to_GRZ;        /* (kgC/m2/d) */
	double STDBc_to_GRZ;
	double GRZ_to_litr1c;				 /* (kgC/m2/d) */
	double GRZ_to_litr2c;				 /* (kgC/m2/d) */
	double GRZ_to_litr3c;				 /* (kgC/m2/d) */
	double GRZ_to_litr4c;				 /* (kgC/m2/d) */   
    /* grazing  - fruit simulation (Hidy 2013.)  */
	double fruitc_to_GRZ;				 /* (kgC/m2/d) */
	double fruitc_storage_to_GRZ;         /* (kgC/m2/d) */
	double fruitc_transfer_to_GRZ;        /* (kgC/m2/d) */
	/* grazing  - softstem simulation (Hidy 2013.)  */
	double softstemc_to_GRZ;				 /* (kgC/m2/d) */
	double softstemc_storage_to_GRZ;         /* (kgC/m2/d) */
	double softstemc_transfer_to_GRZ;        /* (kgC/m2/d) */
	/* fertiliziation -  by Hidy 2008 */
	double FRZ_to_litr1c;				 /* (kgC/m2/d) */
	double FRZ_to_litr2c;				 /* (kgC/m2/d) */
	double FRZ_to_litr3c;				 /* (kgC/m2/d) */
	double FRZ_to_litr4c;				 /* (kgC/m2/d) */ 
	/* CH4 flux based on empirical estimation */
	double CH4_flux_soil;				/* gC/ha/d */
	double CH4_flux_MANURE;				/* gC/ha/d */
	double CH4_flux_FERMENT;			/* gC/ha/d */

} cflux_struct;

/* nitrogen state variables (including sums for sources and sinks) */ 
typedef struct
{
    double leafn;              /* (kgN/m2) leaf N */
    double leafn_storage;      /* (kgN/m2) leaf N */
    double leafn_transfer;     /* (kgN/m2) leaf N */
    double frootn;             /* (kgN/m2) fine root N */ 
    double frootn_storage;     /* (kgN/m2) fine root N */ 
    double frootn_transfer;    /* (kgN/m2) fine root N */ 
    double livestemn;          /* (kgN/m2) live stem N */
    double livestemn_storage;  /* (kgN/m2) live stem N */
    double livestemn_transfer; /* (kgN/m2) live stem N */
    double deadstemn;          /* (kgN/m2) dead stem N */
    double deadstemn_storage;  /* (kgN/m2) dead stem N */
    double deadstemn_transfer; /* (kgN/m2) dead stem N */
    double livecrootn;         /* (kgN/m2) live coarse root N */
    double livecrootn_storage; /* (kgN/m2) live coarse root N */
    double livecrootn_transfer;/* (kgN/m2) live coarse root N */
    double deadcrootn;         /* (kgN/m2) dead coarse root N */
    double deadcrootn_storage; /* (kgN/m2) dead coarse root N */
    double deadcrootn_transfer;/* (kgN/m2) dead coarse root N */
    double cwdn;               /* (kgN/m2) coarse woody debris N */
    double litr1n;             /* (kgN/m2) litter labile N */
    double litr2n;             /* (kgN/m2) litter unshielded cellulose N */
    double litr3n;             /* (kgN/m2) litter shielded cellulose N */
    double litr4n;             /* (kgN/m2) litter lignin N */
	/* fruit simulation (Hidy 2013.) */
	double fruitn;              /* (kgN/m2) fruit N */
    double fruitn_storage;      /* (kgN/m2) fruit N */
    double fruitn_transfer;     /* (kgN/m2) fruit N */
	/* softstem simulation (Hidy 2013.) */
	double softstemn;              /* (kgN/m2) softstem N */
    double softstemn_storage;      /* (kgN/m2) softstem N */
    double softstemn_transfer;     /* (kgN/m2) softstem N */
	/* Hidy 2013: senescence */
	double litr1n_STDB;	/* (kgC/m2)  amount of wilted plant biomass before turning into litter pool */
	double litr2n_STDB;	/* (kgC/m2)  amount of wilted plant biomass before turning into litter pool */
	double litr3n_STDB;	/* (kgC/m2)  amount of wilted plant biomass before turning into litter pool */
	double litr4n_STDB;	/* (kgC/m2)  amount of wilted plant biomass before turning into litter pool */
	double STDBn;  /* (kgC/m2)  amount of standing dead biomass */
    double soil1n;             /* (kgN/m2) microbial recycling pool N (fast) */
    double soil2n;             /* (kgN/m2) microbial recycling pool N (medium) */
    double soil3n;             /* (kgN/m2) microbial recycling pool N (slow) */
    double soil4n;             /* (kgN/m2) recalcitrant SOM N (humus, slowest) */
	/* multilayer soil - Hidy 2011 */
    double sminn[N_SOILLAYERS];		    /* (kgN/m2) soil mineral N in multilayer soil */
	double sminn_RZ;					/* (kgN/m2) sum of the soil mineral N in the rootzone on actual day */
	double retransn;					/* (kgN/m2) plant pool of retranslocated N */
	double npool;						/* (kgN/m2) temporary plant N pool */
    double nfix_src;					/* (kgN/m2) SUM of biological N fixation */
    double ndep_src;					/* (kgN/m2) SUM of N deposition inputs */
    double nleached_snk;				/* (kgN/m2) SUM of N leached */
    double nvol_snk;					/* (kgN/m2) SUM of N lost to volatilization */
	double fire_snk;					/* (kgN/m2) SUM of N lost to fire */
	/* upward movement of SMINN - Hidy 2015 */
	 double ndiffused_snk;				/* (kgN/m2) SUM of N leached */
	/* sensescence simulation - Hidy 2011 */
	double SNSCsnk;				/* (kgN/m2) SUM of senescence losses */
	double SNSCsrc;				/* (kgN/m2) SUM of wilted plant material which turns into the litter pool */
	/* fertilization - by Hidy 2008. */
	double FRZsrc;				 /*(kgN/m2) SUM of N fertilization inputs */	
	/* planting - by Hidy 2012. */
    double PLTsrc;				 /* (kgN/m2) planted leaf N */
	/* thinning - by Hidy 2008. */
	double THNsnk;				 /* (kgN/m2) thinned leaf N */
	double THNsrc;				 /* (kgC/m2) thinned plant material (N content) returns to the soil (labile litter) */
	double THN_transportN; 		/* (kgC/m2) thinned and transported plant material (N content)  */
	double litr1n_strg_THN;		/* (kgC/m2)  amount of thinned plant biomass before turning into litter pool */
	double litr2n_strg_THN;		/* (kgC/m2)  amount of thinned plant biomass before turning into litter pool */
	double litr3n_strg_THN;		/* (kgC/m2)  amount of thinned plant biomass before turning into litter pool */
	double litr4n_strg_THN;		/* (kgC/m2)  amount of thinned plant biomass before turning into litter pool */
	double cwdn_strg_THN;		/* (kgC/m2) amount of thinned plant biomass before turning into litter pool */
	/* mowing - by Hidy 2008. */
	double MOWsnk;              /* (kgN/m2) mowed leaf N */
	double MOWsrc;				/* (kgC/m2) mowed plant material (N content) returns to the soil (labile litter) */
	double MOW_transportN; 		/* (kgC/m2) harvested and transported plant material (N content)  */
	double litr1n_strg_MOW;		/* (kgC/m2)  amount of mowed plant biomass before turning into litter pool */
	double litr2n_strg_MOW;		/* (kgC/m2)  amount of mowed plant biomass before turning into litter pool */
	double litr3n_strg_MOW;		/* (kgC/m2)  amount of mowed plant biomass before turning into litter pool */
	double litr4n_strg_MOW;		/* (kgC/m2)  amount of mowed plant biomass before turning into litter pool */
	/* harvesting - by Hidy 2012. */
	double HRVsnk;
	double HRVsrc;				/* (kgN/m2) harvested leaf N */
	double HRV_transportN; 		/* (kgC/m2) harvested and transported plant material (N content)  */
	double litr1n_strg_HRV;		/* (kgC/m2)  amount of harvested plant biomass before turning into litter pool */
	double litr2n_strg_HRV;		/* (kgC/m2)  amount of harvested plant biomass before turning into litter pool */
	double litr3n_strg_HRV;		/* (kgC/m2)  amount of harvested plant biomass before turning into litter pool */
	double litr4n_strg_HRV;		/* (kgC/m2)  amount of harvested plant biomass before turning into litter pool */
	/* ploughing - Hidy 2012.    */
	double PLGsnk;				/* (kgN/m2) plouhged leaf N */
	double PLGsrc;				/* (kgC/m2) ploughed plant material (N content) returns to the soil (labile litter) */
	double PLG_npool;			/* (kgC/m2) temporary pool of ploughed plant material (N content) before entering litter  */
	/* grazing - by Hidy 2009.   */
	double GRZsnk;              /* (kgN/m2) grazed leaf N */
	double GRZsrc;              /* (kgN/m2) leaf N from fertilizer*/
	/* effect of boundary layer with constant N-content - Hidy 2015 */
	double BNDRYsrc;             /* (kgN/m2) leaf N from fertilizer*/
	double sum_ndemand;          /* (kgN/m2) leaf N from fertilizer*/
        double balance;
} nstate_struct;

/* daily nitrogen flux variables */
typedef struct
{
	/* mortality fluxes */
	double m_leafn_to_litr1n;              /* (kgN/m2/d) */
	double m_leafn_to_litr2n;              /* (kgN/m2/d) */
	double m_leafn_to_litr3n;              /* (kgN/m2/d) */
	double m_leafn_to_litr4n;              /* (kgN/m2/d) */
	double m_frootn_to_litr1n;             /* (kgN/m2/d) */
	double m_frootn_to_litr2n;             /* (kgN/m2/d) */
	double m_frootn_to_litr3n;             /* (kgN/m2/d) */
	double m_frootn_to_litr4n;             /* (kgN/m2/d) */
	double m_leafn_storage_to_litr1n;      /* (kgN/m2/d) */
	double m_frootn_storage_to_litr1n;     /* (kgN/m2/d) */
	double m_livestemn_storage_to_litr1n;  /* (kgN/m2/d) */
	double m_deadstemn_storage_to_litr1n;  /* (kgN/m2/d) */
	double m_livecrootn_storage_to_litr1n; /* (kgN/m2/d) */
	double m_deadcrootn_storage_to_litr1n; /* (kgN/m2/d) */
	double m_leafn_transfer_to_litr1n;     /* (kgN/m2/d) */
	double m_frootn_transfer_to_litr1n;    /* (kgN/m2/d) */
	double m_livestemn_transfer_to_litr1n; /* (kgN/m2/d) */
	double m_deadstemn_transfer_to_litr1n; /* (kgN/m2/d) */
	double m_livecrootn_transfer_to_litr1n;/* (kgN/m2/d) */
	double m_deadcrootn_transfer_to_litr1n;/* (kgN/m2/d) */
        double m_livestemn_to_litr1n;          /* (kgN/m2/d) */
	double m_livestemn_to_cwdn;            /* (kgN/m2/d) */
	double m_deadstemn_to_cwdn;            /* (kgN/m2/d) */
	double m_livecrootn_to_litr1n;         /* (kgN/m2/d) */
	double m_livecrootn_to_cwdn;           /* (kgN/m2/d) */
	double m_deadcrootn_to_cwdn;           /* (kgN/m2/d) */
	double m_retransn_to_litr1n;           /* (kgN/m2/d) */
	/* mortality - fruit simulation (Hidy 2013.) */
	double m_fruitn_to_litr1n;              /* (kgN/m2/d) */
	double m_fruitn_to_litr2n;              /* (kgN/m2/d) */
	double m_fruitn_to_litr3n;              /* (kgN/m2/d) */
	double m_fruitn_to_litr4n;              /* (kgN/m2/d) */
	double m_fruitn_storage_to_litr1n;      /* (kgN/m2/d) */
	double m_fruitn_transfer_to_litr1n;      /* (kgN/m2/d) */
	/* mortality - SOFT STEM simulation (Hidy 2013.) */
	double m_softstemn_to_litr1n;          /* (kgN/m2/d) */
	double m_softstemn_to_litr2n;          /* (kgN/m2/d) */
	double m_softstemn_to_litr3n;          /* (kgN/m2/d) */
	double m_softstemn_to_litr4n;          /* (kgN/m2/d) */
	double m_softstemn_storage_to_litr1n;      /* (kgN/m2/d) */
	double m_softstemn_transfer_to_litr1n;      /* (kgN/m2/d) */
	/* 2010 Hidy - senescence fluxes */
	double m_leafn_to_SNSC;                /* (kgN/m2/d) */
	double m_frootn_to_SNSC;               /* (kgN/m2/d) */
	double m_leafn_storage_to_SNSC;		   /* (kgN/m2/d) */
	double m_frootn_storage_to_SNSC;       /* (kgN/m2/d) */
	double m_leafn_transfer_to_SNSC;       /* (kgN/m2/d) */
	double m_frootn_transfer_to_SNSC;      /* (kgN/m2/d) */
	double m_retransn_to_SNSC;
	double m_STDBn_to_SNSC;
	double SNSC_to_litr1n;
	double SNSC_to_litr2n;
	double SNSC_to_litr3n;
	double SNSC_to_litr4n;	
	/* senescence - fruit simulation (Hidy 2013.) */
	double m_fruitn_to_SNSC;                /* (kgN/m2/d) */
	double m_fruitn_storage_to_SNSC;		   /* (kgN/m2/d) */
	double m_fruitn_transfer_to_SNSC;       /* (kgN/m2/d) */
	/* senescence - softstem simulation (Hidy 2015.) */
	double m_softstemn_to_SNSC;                /* (kgN/m2/d) */
	double m_softstemn_storage_to_SNSC;		   /* (kgN/m2/d) */
	double m_softstemn_transfer_to_SNSC;       /* (kgN/m2/d) */
	/* fire fluxes */
	double m_leafn_to_fire;                /* (kgN/m2/d) */
	double m_frootn_to_fire;               /* (kgN/m2/d) */
	double m_leafn_storage_to_fire;        /* (kgN/m2/d) */
	double m_frootn_storage_to_fire;       /* (kgN/m2/d) */
	double m_livestemn_storage_to_fire;    /* (kgN/m2/d) */
	double m_deadstemn_storage_to_fire;    /* (kgN/m2/d) */
	double m_livecrootn_storage_to_fire;   /* (kgN/m2/d) */
	double m_deadcrootn_storage_to_fire;   /* (kgN/m2/d) */
	double m_leafn_transfer_to_fire;       /* (kgN/m2/d) */
	double m_frootn_transfer_to_fire;      /* (kgN/m2/d) */
	double m_livestemn_transfer_to_fire;   /* (kgN/m2/d) */
	double m_deadstemn_transfer_to_fire;   /* (kgN/m2/d) */
	double m_livecrootn_transfer_to_fire;  /* (kgN/m2/d) */
	double m_deadcrootn_transfer_to_fire;  /* (kgN/m2/d) */
	double m_livestemn_to_fire;            /* (kgN/m2/d) */
	double m_deadstemn_to_fire;            /* (kgN/m2/d) */
	double m_livecrootn_to_fire;           /* (kgN/m2/d) */
	double m_deadcrootn_to_fire;           /* (kgN/m2/d) */
	double m_retransn_to_fire;             /* (kgN/m2/d) */
	double m_litr1n_to_fire;               /* (kgN/m2/d) */
	double m_litr2n_to_fire;               /* (kgN/m2/d) */
	double m_litr3n_to_fire;               /* (kgN/m2/d) */
	double m_litr4n_to_fire;               /* (kgN/m2/d) */
	double m_cwdn_to_fire;                 /* (kgN/m2/d) */
	/* fire - fruit simulation (Hidy 2013.) */
	double m_fruitn_to_fire;                /* (kgN/m2/d) */
	double m_fruitn_storage_to_fire;		   /* (kgN/m2/d) */
	double m_fruitn_transfer_to_fire;       /* (kgN/m2/d) */
	/* fire - softstem simulation (Hidy 2013.) */
	double m_softstemn_to_fire;                /* (kgN/m2/d) */
	double m_softstemn_storage_to_fire;		   /* (kgN/m2/d) */
	double m_softstemn_transfer_to_fire;       /* (kgN/m2/d) */
	/* phenology fluxes from transfer pool */
	double leafn_transfer_to_leafn;           /* (kgN/m2/d) */
	double frootn_transfer_to_frootn;         /* (kgN/m2/d) */
	double livestemn_transfer_to_livestemn;   /* (kgN/m2/d) */
	double deadstemn_transfer_to_deadstemn;   /* (kgN/m2/d) */
	double livecrootn_transfer_to_livecrootn; /* (kgN/m2/d) */
	double deadcrootn_transfer_to_deadcrootn; /* (kgN/m2/d) */
	/* phenology - fruit simulation (Hidy 2013.)  */
	double fruitn_transfer_to_fruitn;         /* (kgN/m2/d) */
	/* phenology - softstem simulation (Hidy 2013.)  */
	double softstemn_transfer_to_softstemn;         /* (kgN/m2/d) */
	/* litterfall fluxes */
	double leafn_to_litr1n;               /* (kgN/m2/d) */
	double leafn_to_litr2n;               /* (kgN/m2/d) */ 
	double leafn_to_litr3n;               /* (kgN/m2/d) */
	double leafn_to_litr4n;               /* (kgN/m2/d) */
	double leafn_to_retransn;             /* (kgN/m2/d) */
	double frootn_to_litr1n;              /* (kgN/m2/d) */
	double frootn_to_litr2n;              /* (kgN/m2/d) */
	double frootn_to_litr3n;              /* (kgN/m2/d) */
	double frootn_to_litr4n;              /* (kgN/m2/d) */
	/* litterfall  - fruit simulation (Hidy 2013.)  */
	double fruitn_to_litr1n;               /* (kgN/m2/d) */
	double fruitn_to_litr2n;               /* (kgN/m2/d) */ 
	double fruitn_to_litr3n;               /* (kgN/m2/d) */
	double fruitn_to_litr4n;               /* (kgN/m2/d) */
	/* litterfall  - softstem simulation (Hidy 2015.)  */
	double softstemn_to_litr1n;               /* (kgN/m2/d) */
	double softstemn_to_litr2n;               /* (kgN/m2/d) */ 
	double softstemn_to_litr3n;               /* (kgN/m2/d) */
	double softstemn_to_litr4n;               /* (kgN/m2/d) */
	/* deposition flux */
	double ndep_to_sminn;                 /* (kgN/m2/d) */
	double nfix_to_sminn;                 /* (kgN/m2/d) */
	/* litter and soil decomposition fluxes */
	double cwdn_to_litr2n;                /* (kgN/m2/d) */
	double cwdn_to_litr3n;                /* (kgN/m2/d) */
	double cwdn_to_litr4n;                /* (kgN/m2/d) */
	double litr1n_to_soil1n;              /* (kgN/m2/d) */
	double sminn_to_soil1n_l1;            /* (kgN/m2/d) */
	double litr2n_to_soil2n;              /* (kgN/m2/d) */
	double sminn_to_soil2n_l2;            /* (kgN/m2/d) */
	double litr3n_to_litr2n;              /* (kgN/m2/d) */
	double litr4n_to_soil3n;              /* (kgN/m2/d) */
	double sminn_to_soil3n_l4;            /* (kgN/m2/d) */
	double soil1n_to_soil2n;              /* (kgN/m2/d) */
	double sminn_to_soil2n_s1;            /* (kgN/m2/d) */
	double soil2n_to_soil3n;              /* (kgN/m2/d) */
	double sminn_to_soil3n_s2;            /* (kgN/m2/d) */
	double soil3n_to_soil4n;              /* (kgN/m2/d) */
	double sminn_to_soil4n_s3;            /* (kgN/m2/d) */
	double soil4n_to_sminn;               /* (kgN/m2/d) */
	/* denitrification (volatilization) fluxes */
	double sminn_to_nvol_l1s1;            /* (kgN/m2/d) */
	double sminn_to_nvol_l2s2;            /* (kgN/m2/d) */
	double sminn_to_nvol_l4s3;            /* (kgN/m2/d) */
	double sminn_to_nvol_s1s2;            /* (kgN/m2/d) */
	double sminn_to_nvol_s2s3;            /* (kgN/m2/d) */
	double sminn_to_nvol_s3s4;            /* (kgN/m2/d) */
	double sminn_to_nvol_s4;              /* (kgN/m2/d) */
	double sminn_to_denitrif;             /* (kgN/m2/d) */
	/* SMINN change caused by soil process - Hidy 2011 */
	double sminn_to_soil_SUM;
	double sminn_to_soil[N_SOILLAYERS];    /* (kgN/m2/d) */
	double sminn_leached[N_SOILLAYERS];    /* (kgN/m2/d) */
	double sminn_diffused[N_SOILLAYERS];    /* (kgN/m2/d) */
	/* daily allocation fluxes */
	double retransn_to_npool;             /* (kgN/m2/d) */
	double sminn_to_npool;                /* (kgN/m2/d) */
	double npool_to_leafn;                /* (kgN/m2/d) */
	double npool_to_leafn_storage;        /* (kgN/m2/d) */
	double nplus;                         /* (kgN/m2/d) */
	/* fruit simulation (Hidy 2013.)  */
	double npool_to_fruitn;                /* (kgN/m2/d) */
	double npool_to_fruitn_storage;        /* (kgN/m2/d) */
	/* softstem simulation (Hidy 2013.)  */
	double npool_to_softstemn;                /* (kgN/m2/d) */
	double npool_to_softstemn_storage;        /* (kgN/m2/d) */

	double npool_to_frootn;               /* (kgN/m2/d) */
	double npool_to_frootn_storage;       /* (kgN/m2/d) */
	double npool_to_livestemn;            /* (kgN/m2/d) */
	double npool_to_livestemn_storage;    /* (kgN/m2/d) */
	double npool_to_deadstemn;            /* (kgN/m2/d) */
	double npool_to_deadstemn_storage;    /* (kgN/m2/d) */
	double npool_to_livecrootn;           /* (kgN/m2/d) */
	double npool_to_livecrootn_storage;   /* (kgN/m2/d) */
	double npool_to_deadcrootn;           /* (kgN/m2/d) */
	double npool_to_deadcrootn_storage;   /* (kgN/m2/d) */
	/* annual turnover of storage to transfer */
	double leafn_storage_to_leafn_transfer;           /* (kgN/m2/d) */
	double frootn_storage_to_frootn_transfer;         /* (kgN/m2/d) */
	double livestemn_storage_to_livestemn_transfer;   /* (kgN/m2/d) */
	double deadstemn_storage_to_deadstemn_transfer;   /* (kgN/m2/d) */
	double livecrootn_storage_to_livecrootn_transfer; /* (kgN/m2/d) */
	double deadcrootn_storage_to_deadcrootn_transfer; /* (kgN/m2/d) */
	/* annual turnover  - fruit simulation (Hidy 2013.) */
	double fruitn_storage_to_fruitn_transfer;           /* (kgN/m2/d) */
	/* annual turnover  - softstem simulation (Hidy 2013.) */
	double softstemn_storage_to_softstemn_transfer;           /* (kgN/m2/d) */
	/* turnover of live wood to dead wood, with retranslocation */
	double livestemn_to_deadstemn;        /* (kgN/m2/d) */
	double livestemn_to_retransn;         /* (kgN/m2/d) */
	double livecrootn_to_deadcrootn;      /* (kgN/m2/d) */
	double livecrootn_to_retransn;        /* (kgN/m2/d) */
	/* planting - Hidy 2013. */
	double leafn_transfer_from_PLT;		/* (kgN/m2/d) */
	double frootn_transfer_from_PLT;		/* (kgN/m2/d) */
	double fruitn_transfer_from_PLT;		/* (kgN/m2/d) */
	double softstemn_transfer_from_PLT;		/* (kgN/m2/d) */
	/* thinning - by Hidy 2012. */
	double leafn_to_THN;				 /* (kgN/m2/d) */
	double leafn_storage_to_THN;         /* (kgN/m2/d) */
	double leafn_transfer_to_THN;        /* (kgN/m2/d) */
	double frootn_to_THN;				 /* (kgN/m2/d) */
	double frootn_storage_to_THN;         /* (kgN/m2/d) */
	double frootn_transfer_to_THN;        /* (kgN/m2/d) */
	double livecrootn_to_THN;				 /* (kgN/m2/d) */
	double livecrootn_storage_to_THN;         /* (kgN/m2/d) */
	double livecrootn_transfer_to_THN;        /* (kgN/m2/d) */
	double deadcrootn_to_THN;				 /* (kgN/m2/d) */
	double deadcrootn_storage_to_THN;         /* (kgN/m2/d) */
	double deadcrootn_transfer_to_THN;        /* (kgN/m2/d) */
	double livestemn_to_THN;				 /* (kgN/m2/d) */
	double livestemn_storage_to_THN;         /* (kgN/m2/d) */
	double livestemn_transfer_to_THN;        /* (kgN/m2/d) */
	double deadstemn_to_THN;				 /* (kgN/m2/d) */
	double deadstemn_storage_to_THN;         /* (kgN/m2/d) */
	double deadstemn_transfer_to_THN;        /* (kgN/m2/d) */
	double retransn_to_THN;
	double THN_to_litr1n;				 /* (kgN/m2/d) */
	double THN_to_litr2n;				 /* (kgN/m2/d) */
	double THN_to_litr3n;				 /* (kgN/m2/d) */
	double THN_to_litr4n;				 /* (kgN/m2/d) */
	double THN_to_cwdn;
	double STDBn_to_THN;
	/* thinning - fruit simulation (Hidy 2013.)  */
	double fruitn_to_THN;				 /* (kgN/m2/d) */
	double fruitn_storage_to_THN;         /* (kgN/m2/d) */
	double fruitn_transfer_to_THN;        /* (kgN/m2/d) */
	/* mowing - by Hidy 2008. */
	double leafn_to_MOW;                 /* (kgN/m2/d) */
	double leafn_storage_to_MOW;         /* (kgN/m2/d) */
	double leafn_transfer_to_MOW;        /* (kgN/m2/d) */
	double STDBn_to_MOW;
	double retransn_to_MOW;
	double MOW_to_litr1n;				 /* (kgN/m2/d) */
	double MOW_to_litr2n;				 /* (kgN/m2/d) */
	double MOW_to_litr3n;				 /* (kgN/m2/d) */
	double MOW_to_litr4n;				 /* (kgN/m2/d) */
	/* mowing - fruit simulation (Hidy 2013.)  */
	double fruitn_to_MOW;				 /* (kgN/m2/d) */
	double fruitn_storage_to_MOW;         /* (kgN/m2/d) */
	double fruitn_transfer_to_MOW;        /* (kgN/m2/d) */
	/* mowing - softstem simulation (Hidy 2013.)  */
	double softstemn_to_MOW;				 /* (kgN/m2/d) */
	double softstemn_storage_to_MOW;         /* (kgN/m2/d) */
	double softstemn_transfer_to_MOW;        /* (kgN/m2/d) */
	/* harvesting - by Hidy 2012. */
	double leafn_to_HRV;                 /* (kgN/m2/d) */
	double leafn_storage_to_HRV;         /* (kgN/m2/d) */
	double leafn_transfer_to_HRV;        /* (kgN/m2/d) */
	double STDBn_to_HRV;
	double retransn_to_HRV;
	double HRV_to_litr1n;				 /* (kgN/m2/d) */
	double HRV_to_litr2n;				 /* (kgN/m2/d) */
	double HRV_to_litr3n;				 /* (kgN/m2/d) */
	double HRV_to_litr4n;				 /* (kgN/m2/d) */
	/* harvesting - fruit simulation (Hidy 2013.)  */
	double fruitn_to_HRV;				 /* (kgN/m2/d) */
	double fruitn_storage_to_HRV;         /* (kgN/m2/d) */
	double fruitn_transfer_to_HRV;        /* (kgN/m2/d) */
	/* harvesting - softstem simulation (Hidy 2013.)  */
	double softstemn_to_HRV;				 /* (kgN/m2/d) */
	double softstemn_storage_to_HRV;         /* (kgN/m2/d) */
	double softstemn_transfer_to_HRV;        /* (kgN/m2/d) */
	/* ploughing - Hidy 2012. */
	double leafn_to_PLG;                 /* (kgN/m2/d) */
	double leafn_storage_to_PLG;         /* (kgN/m2/d) */
	double leafn_transfer_to_PLG;        /* (kgN/m2/d) */
	double STDBn_to_PLG;
	double CTDBn_to_PLG;
	double frootn_to_PLG;				/* (kgN/m2/d) */
	double frootn_storage_to_PLG;         /* (kgN/m2/d) */
	double frootn_transfer_to_PLG;        /* (kgN/m2/d) */
	double retransn_to_PLG;
	double PLG_to_litr1n;
	double PLG_to_litr2n;
	double PLG_to_litr3n;
	double PLG_to_litr4n;
	/* ploughing - fruit simulation (Hidy 2013.)  */
	double fruitn_to_PLG;				 /* (kgN/m2/d) */
	double fruitn_storage_to_PLG;         /* (kgN/m2/d) */
	double fruitn_transfer_to_PLG;        /* (kgN/m2/d) */
	/* ploughing - softstem simulation (Hidy 2013.)  */
	double softstemn_to_PLG;				 /* (kgN/m2/d) */
	double softstemn_storage_to_PLG;         /* (kgN/m2/d) */
	double softstemn_transfer_to_PLG;        /* (kgN/m2/d) */
	/* grazing - by Hidy 2008. */
	double leafn_to_GRZ;                 /* (kgN/m2/d) */
	double leafn_storage_to_GRZ;         /* (kgN/m2/d) */
	double leafn_transfer_to_GRZ;        /* (kgN/m2/d) */
	double STDBn_to_GRZ;
	double retransn_to_GRZ;
	double GRZ_to_litr1n;				 /* (kgN/m2/d) */
	double GRZ_to_litr2n;				 /* (kgN/m2/d) */
	double GRZ_to_litr3n;				 /* (kgN/m2/d) */
	double GRZ_to_litr4n;				 /* (kgN/m2/d) */
	/* grazing - fruit simulation (Hidy 2013.)  */
	double fruitn_to_GRZ;				 /* (kgN/m2/d) */
	double fruitn_storage_to_GRZ;         /* (kgN/m2/d) */
	double fruitn_transfer_to_GRZ;        /* (kgN/m2/d) */
	/* grazing - softstem simulation (Hidy 2013.)  */
	double softstemn_to_GRZ;				 /* (kgN/m2/d) */
	double softstemn_storage_to_GRZ;         /* (kgN/m2/d) */
	double softstemn_transfer_to_GRZ;        /* (kgN/m2/d) */
	/* fertiliziation -  by Hidy 2008 */
	double FRZ_to_sminn;      /* (kgN/m2/d) */
	double FRZ_to_litr1n;				 /* (kgN/m2/d) */
	double FRZ_to_litr2n;				 /* (kgN/m2/d) */
	double FRZ_to_litr3n;				 /* (kgN/m2/d) */
	double FRZ_to_litr4n;				 /* (kgN/m2/d) */
	/* N2O flux based on empirical estimation */
	double N2O_flux_soil;				 /* mgN/m2/d */
	double N2O_flux_GRZ;
	double N2O_flux_FRZ;
} nflux_struct;

/* temporary nitrogen variables for reconciliation of decomposition
immobilization fluxes and plant growth N demands */
typedef struct 
{
	double mineralized;
	double potential_immob;
	double plant_ndemand;		// Hidy 2009.
	double excess_c;
	double day_gpp;
	double plitr1c_loss;
	double pmnf_l1s1;
	double plitr2c_loss;
	double pmnf_l2s2;
	double plitr4c_loss;
	double pmnf_l4s3;
	double psoil1c_loss;
	double pmnf_s1s2;
	double psoil2c_loss;
	double pmnf_s2s3;
	double psoil3c_loss;
	double pmnf_s3s4;
	double psoil4c_loss;
	double kl4;
} ntemp_struct;
	
/* phenological control arrays */
typedef struct
{
	int* remdays_curgrowth; /* (nmetdays) days left in current growth season */
	int* remdays_transfer;  /* (nmetdays) number of transfer days remaining */
	int* remdays_litfall;   /* (nmetdays) number of litfall days remaining */
	int* predays_transfer;  /* (nmetdays) number of transfer days previous */
	int* predays_litfall;   /* (nmetdays) number of litfall days previous */
	int* onday_arr;			/* Hidy 2009 - (nmetyears) first day of transfer period */
	int* offday_arr;		/* Hidy 2009 - (nmetyears) last day of transfer period */
} phenarray_struct;

/* daily phenological data array */
typedef struct
{  
	double remdays_curgrowth; /* days left in current growth season */
	double remdays_transfer;  /* number of transfer days remaining */
	double remdays_litfall;   /* number of litfall days remaining */
	double predays_transfer;  /* number of transfer days previous */
	double predays_litfall;   /* number of litfall days previous */
	int onday;				  /* Hidy 2012 - first day of vegetation period */
	int offday;				  /* Hidy 2012 - last day of vegetation period */
} phenology_struct;


/* ecophysiological variables */
typedef struct
{
	double day_leafc_litfall_increment;      /* (kgC/m2/d) rate leaf litfall */
	double day_fruitc_litfall_increment;     /* (kgC/m2/d) rate fruit litfall - fruit simulation (Hidy 2013) */
	double day_softstemc_litfall_increment;     /* (kgC/m2/d) rate softstem litfall - softstem simulation (Hidy 2013) */
	double day_frootc_litfall_increment;     /* (kgC/m2/d) rate froot litfall */
	double day_livestemc_turnover_increment; /* (kgC/m2/d) rate livestem turnover */
	double day_livecrootc_turnover_increment;/* (kgC/m2/d) rate livecroot turnover */
	double annmax_leafc;					 /* (kgC/m2) annual maximum daily leaf C */
	double annmax_fruitc;					 /* (kgC/m2) annual maximum daily fruit C - fruit simulation (Hidy 2013) */
	double annmax_softstemc;					 /* (kgC/m2) annual maximum daily softstem C - softstem simulation (Hidy 2013) */
	double annmax_frootc;					 /* (kgC/m2) annual maximum daily froot C */
	double annmax_livestemc;				 /* (kgC/m2) annual maximum daily livestem C */
	double annmax_livecrootc;				 /* (kgC/m2) annual maximum daily livecroot C */
	double dsr;							    /* (days) number of days since rain, for soil evap */
	double dsws;							/* Hidy 2010 - (days) number of days since soil water stress */
	double dsws_FULL;							/* Hidy 2010 - (days) number of days since total soil water stress (below WP)*/
    double proj_lai;						/* (DIM) live projected leaf area index */
    double all_lai;							/* (DIM) live all-sided leaf area index */
    double plaisun;							/* (DIM) sunlit projected leaf area index */
    double plaishade;						/* (DIM) shaded projected leaf area index */
    double sun_proj_sla;					/* (m2/kgC) sunlit projected SLA */
    double shade_proj_sla;					/* (m2/kgC) shaded projected SLA */
    
	/* Hidy 2010 - new variables */
	double rootlength_prop[N_SOILLAYERS];		    /* (prop) Hidy 2014 - array contains the proportion of total root lenght in the given soil layer (Jarvis 1989) */
	double psi[N_SOILLAYERS];						/* (MPa) water potential of soil and leaves - Hidy 2010: multilayer soil  */
	double pF[N_SOILLAYERS];						/* (cm) soil water suction derived from log(soil water potential) - Hidy 2010  */
	double hydr_conduct_S[N_SOILLAYERS];			/* (m/s) hydraulic conductivity at the beginning of the day - Hidy 2015: multilayer soil  */
	double hydr_diffus_S[N_SOILLAYERS];				/* (m2/s) hydraulic diffusivity at the beginning of the day  - Hidy 2015: multilayer soil  */
	double hydr_conduct_E[N_SOILLAYERS];			/* (m/s) hydraulic conductivity at the end of the day  - Hidy 2015: multilayer soil  */
	double hydr_diffus_E[N_SOILLAYERS];				/* (m2/s) hydraulic diffusivity at the end of the day  - Hidy 2015: multilayer soil  */
    double vwc[N_SOILLAYERS];						/* (DIM) volumetric water content - Hidy 2015: multilayer soil */
	int n_rootlayers;								/* (number) number of of the soil layers in which root can be found  - actual */
	int n_maxrootlayers;							/* (number) number of of the soil layers in which root can be found  - potential */
	double rooting_depth;							/* (m) actual depth of the rooting zone */
	double psi_avg;									/* (MPa) average water potential of soil and leaves  - Hidy 2010 */
    double vwc_avg;									/* (m3/m3) average volumetric water content in active soil  - Hidy 2010 */
    double vwc_ratio_crit1[N_SOILLAYERS];			/* (DIM) volumetric water content ratio at start of conductance reduction - Hidy 2012*/
	double vwc_ratio_crit2[N_SOILLAYERS];			/* (DIM) volumetric water content ratio at stomatal closure - Hidy 2012*/
    double vwc_crit1[N_SOILLAYERS];					/* (DIM) volumetric water content at start of conductance reduction - Hidy 2012*/
	double vwc_crit2[N_SOILLAYERS];					/* (DIM) volumetric water content at stomatal closure - Hidy 2012*/
	double psi_crit1[N_SOILLAYERS];					/* (DIM) soil water potential at start of conductance reduction - Hidy 2012*/
	double psi_crit2[N_SOILLAYERS];					/* (DIM) soil water potential at stomatal closure - Hidy 2012*/
	int flowering;									/* (yday) start of flowering (fruit allocation) - Hidy 2015 */
	int maturity;									/* (yday) start of leaf senenscence (maturity) - Hidy 2015 */

	double dlmr_area_sun;					/* (umolC/m2projected leaf area/s) sunlit leaf MR */
    double dlmr_area_shade;					/* (umolC/m2projected leaf area/s) shaded leaf MR */
    double gl_t_wv_sun;						/* (m/s) leaf-scale conductance to transpired water */
    double gl_t_wv_shade;					/* (m/s) leaf-scale conductance to transpired water */
    double assim_sun;						/* (umol/m2/s) sunlit assimilation per unit pLAI */
    double assim_shade;						/* (umol/m2/s) shaded assimilation per unit pLAI */
    /* decomp variables */
    double t_scalar[N_SOILLAYERS];			/* (DIM) decomp temperature scalar */
    double w_scalar[N_SOILLAYERS];			/* (DIM) decomp water scalar */
    double rate_scalar[N_SOILLAYERS];		/* (DIM) decomp combined scalar */
	double rate_scalar_avg;					/* (DIM) decomp combined and averaged scalar */
	double daily_gross_nmin;				/* (kgN/m2/d) daily gross N mineralization */
	double daily_gross_nimmob;				/* (kgN/m2/d) daily gross N immobilization */ 
	double daily_net_nmin;					/* (kgN/m2/d) daily net N mineralization */

    /* the following are optional outputs, usually set if the appropriate
    functions are called with the flag verbose = 1 */
	double max_conduct;						   	   /* (DIM) stomatal conductance with atmospheric [CO2] multiplier */
	double m_tmin;			/* (DIM) freezing night temperature multiplier */
	double m_soilstress_layer[N_SOILLAYERS]; /* (DIM) Hidy 2010: CONTROL - soil water stress multiplier */
	double m_soilstress;					      /* (DIM) soil water properties (water content and depth) multiplier */
	double m_ppfd_sun;						 /* (DIM) PAR flux density multiplier */
	double m_ppfd_shade;					 /* (DIM) PAR flux density multiplier */
	double m_vpd;							 /* (DIM) vapor pressure deficit multiplier */
	double m_final_sun;						 /* (DIM) product of all other multipliers */
	double m_final_shade;        /* (DIM) product of all other multipliers */
	double gl_bl;          /* (m/s) leaf boundary layer conductance */
    double gl_c;           /* (m/s) leaf cuticular conductance */
    double gl_s_sun;           /* (m/s) leaf-scale stomatal conductance */
    double gl_s_shade;           /* (m/s) leaf-scale stomatal conductance */
	double gl_e_wv;        /* (m/s) leaf conductance to evaporated water */
	double gl_sh;          /* (m/s) leaf conductance to sensible heat */
    double gc_e_wv;        /* (m/s) canopy conductance to evaporated water */
    double gc_sh;          /* (m/s) canopy conductance to sensible heat */
	
	/* diagnostic variables for ouput purposes only */
	double ytd_maxplai;    /* (DIM) year-to-date maximum projected LAI */
	double n_limitation;	     /* Hidy 2010 - flag for nitrogen limitation */
} epvar_struct;

/* soil and site constants */
typedef struct
{
	double sand[N_SOILLAYERS];							/* (%)   Hidy 2015 - sand content of soil in the given soil layer */	
	double silt[N_SOILLAYERS];							/* (%)   Hidy 2015 - silt content of soil in the given soil layer */	
	double clay[N_SOILLAYERS];							/* (%)   Hidy 2015 - clay content of soil in the given soil layer */	
    double soillayer_depth[N_SOILLAYERS];			/*  (m) Hidy 2010 - array contains the soil layer depths (positive values)*/
	double soillayer_thickness[N_SOILLAYERS];		/*  (m) Hidy 2010 - array contains the soil layer thicknesses (positive values) */
	double soillayer_midpoint[N_SOILLAYERS];					/*  (m) Hidy 2010 - array contains the depths of the middle layers (positive values)*/
    double soil_b[N_SOILLAYERS];										/* (DIM) Clapp-Hornberger "b" parameter */
	double BD[N_SOILLAYERS];										/* (g/cm3) bulk density */
	double RCN;									/* (m) runoff curve number */
    double vwc_sat[N_SOILLAYERS];										/* (DIM) volumetric water content at saturation */
    double vwc_fc[N_SOILLAYERS];								/* (DIM) VWC at field capacity ( = -0.033 MPa) */
	double vwc_wp[N_SOILLAYERS];								/* (DIM) VWC at wilting point ( = pF 4.2) */
	double vwc_hw[N_SOILLAYERS];								/* (DIM) VWC at hygroscopic water point  ( = pF 6.2) */
    double psi_sat[N_SOILLAYERS];								/* (MPa) soil matric potential at saturation */
	double psi_fc[N_SOILLAYERS];								/* (MPa) soil matric potential at field capacity */
	double psi_wp[N_SOILLAYERS];								/* (MPa) soil matric potential at wilting point */
	double psi_hw;								/* (MPa) soil matric potential at hygroscopic water point */
	double hydr_conduct_sat[N_SOILLAYERS];					/* (m/s) hidraulic conductivity at saturation  - Hidy 2010 */
	double hydr_diffus_sat[N_SOILLAYERS];				    	/* (m2/s) hidraulic diffusivity at saturation  - Hidy 2010 */
	double hydr_conduct_fc[N_SOILLAYERS];				    	/* (m/s) hidraulic conductivity at field capacity  - Hidy 2010 */
	double hydr_diffus_fc[N_SOILLAYERS];				    	/* (m2/s) hidraulic diffusivity at field capacity  - Hidy 2010 */
	double tair_annavg;						/* (Celsius) mean surface air temperature - Hidy 2010 */
	double tair_annrange;					/* (Celsius) mean annual air temperature range - Hidy 2015 */
    double elev;								/* (m) site elevation */
    double lat;				 					/* (degrees) site latitude (negative for south) */
    double sw_alb;								/* (DIM) surface shortwave albedo */
 
	double RCN_mes;								/* (m) measured runoff curve number */
	double BD_mes[N_SOILLAYERS];							/* (g/cm3) Hidy 2015 - measured bulk density */
	double vwc_sat_mes[N_SOILLAYERS];							/* (m3/m3) Hidy 2010 - measured soil water content at saturation*/
	double vwc_fc_mes[N_SOILLAYERS];							/* (m3/m3) Hidy 2010 - measured soil water content at field capacity*/
	double vwc_wp_mes[N_SOILLAYERS];							/* (m3/m3) Hidy 2010 - measured soil water content at wilting point*/
	double vwc_hw_mes[N_SOILLAYERS];							/* (m3/m3) Hidy 2015 - measured hygroscopic water content */
    double gwd_act;							    /* (m)	Hidy 2014 - actual depth of the groundwater on a given day */	
	double* gwd_array;							/* (m)	Hidy 2013 - depth of the groundwater */	

} siteconst_struct;								

/* canopy ecophysiological constants */
typedef struct
{
	int woody;             /* (flag) 1=woody, 0=non-woody */
	int evergreen;         /* (flag) 1=evergreen, 0=deciduous */
	int c3_flag;           /* (flag) 1 = C3,  0 = C4 */
	int phenology_flag;    /* (flag) 1=phenology model, 0=user defined */
	int q10depend_flag;	   /* (flag) 1 = temperature dependent q10 value; 0= constans q10 value */
	int acclimation_flag;  /* (flag)  1 = acclimation 0 = no acclimation */
	int CO2conduct_flag;      /* (flag) CO2 conductance reduction flag (0: no effect, 1: multiplier) */
	int SHCM_flag;			  /* (flag) water movement calculation method (0:Richards, 1:DSSAT) */
	int discretlevel_Richards;/* Hidy 2015 - discretization level of SWC calculation */
	int STCM_flag;			  /* (flag) soil temperature calculation method (0:Zheng, 1:DSSAT) */
	int onday;             /* (yday) yearday leaves on */
	int offday;            /* (yday) yearday leaves off */
	int base_temp;			/* basic temperature fo GDD/heatsum calculation */
	int GDD_fruitalloc;		  /* (yday) growing degree day of fruit allocation */
	int GDD_maturity;         /* (yday) growing degree day of maturity */
	double transfer_pdays;    /* (prop.) fraction of growth period for transfer */
	double litfall_pdays;     /* (prop.) fraction of growth period for litfall */
    double leaf_turnover;     /* (1/yr) annual leaf turnover fraction */
    double froot_turnover;    /* (1/yr) annual fine root turnover fraction */
    double livewood_turnover; /* (1/yr) annual live wood turnover fraction */
    double daily_mortality_turnover; /* (1/day) daily mortality turnover */
    double daily_fire_turnover;      /* (1/day) daily fire turnover */
    double alloc_frootc_leafc; /* (ratio) new fine root C to new leaf C */
	double alloc_newstemc_newleafc; /* (ratio) new stem C to new leaf C */
	double alloc_newlivewoodc_newwoodc; /* (ratio) new livewood C:new wood C */
    double alloc_crootc_stemc; /* (ratio) new live croot C to new live stem C */
    double alloc_prop_curgrowth; /* (prop.) daily allocation to current growth */
    double avg_proj_sla;   /* (m2/kgC) canopy average proj. SLA */
    double sla_ratio;      /* (DIM) ratio of shaded to sunlit projected SLA */
    double lai_ratio;      /* (DIM) ratio of (all-sided LA / one-sided LA) */
    double int_coef;       /* (kg/kg/LAI/d) canopy precip interception coef */
    double ext_coef;       /* (DIM) canopy light extinction coefficient */
    double flnr;           /* (kg NRub/kg Nleaf) leaf N in Rubisco */
	double flnp;           /* (kg PeP/kg Nleaf) fraction of leaf N in PEP Carboxylase */
	double relVWC_crit1;  /* (%) Hidy 2014 - critical relative vwc to calc. soil moisture limitation 1 */
    double relVWC_crit2;  /* (%) Hidy 2014 - critical relative vwc to calc. soil moisture limitation 2 */
	double PSI_crit1;   /* (%) Hidy 2014 - critical psi to calc. soil moisture limitation 1 */
    double PSI_crit2;  /* (%) Hidy 2011 - critical psi to calc. soil moisture limitation 2 */
	double vpd_open;       /* (Pa)  vpd at start of conductance reduction */
	double vpd_close;      /* (Pa)  vpd at complete conductance reduction */
    double gl_smax;        /* (m/s) maximum leaf-scale stomatal conductance */
    double gl_c;           /* (m/s) leaf-scale cuticular conductance */
	double gl_bl;          /* (m/s) leaf-scale boundary layer conductance */
    double froot_cn;     /* (kgC/kgN) C:N for fine roots */
    double leaf_cn;      /* (kgC/kgN) C:N for leaves */
    double livewood_cn;  /* (kgC/kgN) C:N for live wood */
	double deadwood_cn;  /* (kgC/kgN) C:N for dead wood */
    double leaflitr_cn;      /* (kgC/kgN) constant C:N for leaf litter */
    double leaflitr_flab;    /* (DIM) leaf litter labile fraction */
    double leaflitr_fucel;   /* (DIM) leaf litter unshielded cellulose fract. */
    double leaflitr_fscel;   /* (DIM) leaf litter shielded cellulose fract. */
    double leaflitr_flig;    /* (DIM) leaf litter lignin fraction */
    double frootlitr_flab;   /* (DIM) froot litter labile fraction */
    double frootlitr_fucel;  /* (DIM) froot litter unshielded cellulose fract */
    double frootlitr_fscel;  /* (DIM) froot litter shielded cellulose fract */
    double frootlitr_flig;   /* (DIM) froot litter lignin fraction */
    double deadwood_fucel;   /* (DIM) dead wood unshileded cellulose fraction */
    double deadwood_fscel;   /* (DIM) dead wood shielded cellulose fraction */
    double deadwood_flig;    /* (DIM) dead wood lignin fraction */
	double mort_SNSC_abovebiom;	/* Hidy 2011 - mortality parameter of senescence of aboveground biomass */
	double mort_SNSC_belowbiom;	/* Hidy 2011 - mortality parameter of senescence of belowground biomass */
	double mort_SNSC_leafphen;	/* Hidy 2011 - leaf phenology mortality parameter */
    double mort_SNSC_to_litter; /* Hidy 2013 - turnover rate of wilted standing biomass to litter*/
	double mort_CnW_to_litter;  /* Hidy 2013 - turnover rate of cut-down non-woody biomass to litter*/
	double GR_ratio;            /* Hidy 2013 - (DIM) growth resp per unit of C grown */
	double denitrif_prop;		/* Hidy 2013 - fraction of mineralization to volatile */
	double bulkN_denitrif_prop_WET;
	double bulkN_denitrif_prop_DRY;
	double mobilen_prop;		/* Hidy 2013 -fraction mineral N avail for leaching */
	double nfix;				/* (kgN/m2/yr) symbiotic+asymbiotic fixation of N */
	double maturity_coeff;		/* Hidy 2013 - maturity coefficient (to calculate maximum rooting depth) */
	double* wpm_array;			/* Hidy 2013 - changing WPM values */
	double* msc_array;			/* Hidy 2013 - changing MSC values */
	double* sgs_array;			/* Hidy 2015 - changing SGS values */
	double* egs_array;			/* Hidy 2015 - changing EGS values */
	/* fruit simulation - Hidy 2013. */
	double fruit_turnover;          /* (1/yr) annual fruit turnover fraction */
    double alloc_fruitc_leafc;      /* (ratio) new fruit c to new leaf c */
    double fruit_cn;                /* (kgC/kgN) C:N for fruits */
    double fruitlitr_flab;          /* (DIM) fruit litter labile fraction */
    double fruitlitr_fucel;         /* (DIM) fruit litter unshielded cellulose fract. */
    double fruitlitr_fscel;         /* (DIM) fruit litter shielded cellulose fract. */
    double fruitlitr_flig;          /* (DIM) fruit litter lignin fraction */
	/* SOFT STEM simulation - Hidy 2015. */
	double softstem_turnover;          /* (1/yr) annual softstem turnover fraction */
    double alloc_softstemc_leafc;      /* (ratio) new softstem c to new leaf c */
    double softstem_cn;                /* (kgC/kgN) C:N for softstems */
	double softstemlitr_flab;          /* (DIM) softstem litter labile fraction */
    double softstemlitr_fucel;         /* (DIM) softstem litter unshielded cellulose fract. */
    double softstemlitr_fscel;         /* (DIM) softstem litter shielded cellulose fract. */
    double softstemlitr_flig;          /* (DIM) softstem litter lignin fraction */
	/* storage pool mortality  - Hidy 2014 */
	double storage_MGMmort;				/* (DIM) ratio of the storage pool mortality of the actual pool mortalitay due to management */
	/* senescence mortality - Hidy 2014*/
	double m_soilstress_crit;      /* (DIM) critical value os soil stress multiplier below wchich senescence mortaility begins */
	int n_stressdays_crit;         /* (DIM) critical number of stress days after wchich senescence mortality is totally */
	double max_rootzone_depth;		/* (m)   Hidy 2010 - maximum depth of rooting zone */
	double rootdistrib_param;       /* (DIM) root distribution parameter (Jarvis 1989) - Hidy 2015*/
	double c_param_tsoil;
	double mrpern;
	/* respiration fractions for fluxes between compartments - Hidy 2015 */
	double rfl1s1;
	double rfl2s2;
	double rfl4s3;
	double rfs1s2;
	double rfs2s3;
	double rfs3s4;		
	/* 	base values of rate constants are (1/day) - Hidy 2015  */
	double kl1_base;      /* labile litter pool */
	double kl2_base;     /* cellulose litter pool */
	double kl4_base;     /* lignin litter pool */
	double ks1_base;    /* fast microbial recycling pool */
	double ks2_base;    /* medium microbial recycling pool */
	double ks3_base;   /* slow microbial recycling pool */
	double ks4_base;   /* recalcitrant SOM (humus) pool */
	double kfrag_base; /* physical fragmentation of coarse woody debris */
	/* parameters of empirical N2O modeling */
	double N_pCNR1;
	double N_pCNR2;
	double N_pVWC1;
	double N_pVWC2;
	double N_pVWC3;
	double N_pVWC4;
	double N_pTS;
	/* parameters of empirical CH4 modeling */
	double C_pBD1;
	double C_pBD2;
	double C_pVWC1;
	double C_pVWC2;
	double C_pVWC3;
	double C_pVWC4;
	double C_pTS;
} epconst_struct;

/* strucure for thinning paramteres - by Hidy 2012. */
typedef struct
{
	int THN_flag;							/* (flag) 1=do thinning , 0=no thinning */
	int mgmd;								/* (flag) 1=do management , 0=no management on actual day */
	double** THNdays_array;					/* (array) contains the thinning days in 1 year*/
	double** thinning_rate_array;			/* (array) rate of the thinned trees */
	double** transpcoeff_woody_array;		/* (array) rate of the transported woody matter */
	double** transpcoeff_nwoody_array;		/* (array) rate of the transported non woody matter */
} thinning_struct;

/* strucure for mowing paramteres - by Hidy 2008 */
typedef struct
{
	int MOW_flag;								/* (flag) 1=do mowing , 0=no mowing */
	int mgmd;									/* (flag) 1=do management , 0=no management on actual day */
	int fixday_or_fixLAI_flag;					/* (flag) 0=mowing on fixed days, 1=mowing if LAI greater than a fixed value */
	double fixLAI_befMOW;						/* (value) LAI before mowing (fixvalue method)*/
	double fixLAI_aftMOW;						/* (value) LAI after mowing (fixvalue method) */
	double** MOWdays_array;						/* (array) contains the mowing days in 1 year (fixday method)*/
	double** LAI_limit_array;					/* (array) LAI after mowing (fixday method)*/
	double** transport_coeff_array;				/* (array) proportion of plant material transported away (fixday method)*/
} mowing_struct;

/* strucure for harvesting paramteres - by Hidy 2009 */
typedef struct
{
	int HRV_flag;								/* (flag) 1=do harvesting , 0=no harvesting */
	int mgmd;									/* (flag) 1=do management , 0=no management on actual day */
    double** HRVdays_array;						/* (array) contains the harvesting days in 1 year*/
	double** snag_array;				    	/* (array) snag after harvesting*/
	double** transport_coeff_array;				/* (array) proportion of plant material transported away */
	int afterHRV;								/* (flag) 0=before harvesting, 1=after harvesting */
} harvesting_struct;

/* strucure for ploughing paramteres - by Hidy 2012 */
typedef struct
{
	int PLG_flag;								/* (flag) 1=do plough , 0=no plough */
	int mgmd;									/* (flag) 1=do management , 0=no management on actual day */
	double** PLGdays_array;						/* (array) contains the plough days in 1 year*/
	double** PLGdepths_array;					/* (array) contains the ploughing depths in 1 year*/
	double** dissolv_coeff_array;				/* (dimless) dissolving coefficient of cut plant material (to litter) */
	double PLG_pool_litr1c;						/* (value) actual ploughing pool */
	double PLG_pool_litr2c;						/* (value) actual ploughing pool */
	double PLG_pool_litr3c;						/* (value) actual ploughing pool */
	double PLG_pool_litr4c;						/* (value) actual ploughing pool */
	double PLG_pool_litr1n;						/* (value) actual ploughing pool */
	double PLG_pool_litr2n;						/* (value) actual ploughing pool */
	double PLG_pool_litr3n;						/* (value) actual ploughing pool */
	double PLG_pool_litr4n;						/* (value) actual ploughing pool */
	double DC_act;
} ploughing_struct;

/* strucure for grazing paramteres - by Hidy 2009 */
typedef struct
{
	int GRZ_flag;								/* (flag) 1=do grazin , 0=no grazing */	
	int mgmd;									/* (flag) 0,1..6=do management (first, second...) , -1=no management on actual day */
	double** trampling_effect;					/* (prop) trampling_effect coefficient (standing dead biomass to litter) */
	double** GRZ_start_array;					/* (array) beginning of grazing */
	double** GRZ_end_array;						/* (array) end of grazing */
	double** weight_LSU;						/* (array) (kg/LSU) weight equivalent of an averaged animal */
	double** stocking_rate_array;				/* (array) (LSU/ha) animal stocking rate: Livestock Units per hectare */
	double** DMintake_array;					/* (array) pasture forgage dry matter intake */
	double** prop_DMintake2excr_array;			/* (array) prop of the dry matter intake formed excrement */
	double** prop_excr2litter_array;			/* (array) prop. of excrement return to litter */ 
	double** DM_Ccontent_array;					/* (array) carbon content of dry matter*/
	double** EXCR_Ncontent_array;				/* (array) nitrogen content of the fertilizer */
	double** EXCR_Ccontent_array;				/* (array) carbon content of the fertilizer */
	double trampleff_act;						/* (array) actual trampling_effect coefficient (standing dead biomass to litter) */
	double** Nexrate;                           /* (kgN/1000 kg animal mass/day) default N excretion rate */
	double** EFman_N2O;							/* (array) manure emission factor for direct N2O emissions from manure management	 */
	double** EFman_CH4;							/* (array) manure emission factor for direct CH4 emissions from manure management */
	double** EFfer_CH4;							/* (array) fermentation emission factor for direct CH4 emissions from manure management*/
} grazing_struct;



/* strucure for planting paramteres - by Hidy 2008 */
typedef struct
{
	int PLT_flag;								/* (flag) 1=do planting , 0=no planting */	
	int mgmd;									/* (flag) 1=do management , 0=no management on actual day */
	int afterPLT;								/* (flag) 0=before planting, 1=after planting */
    double** PLTdays_array;						/* (array) contains the planting days in 1 year*/
	double** seed_quantity_array;				/* (array) quantity of seed*/
	double** seed_carbon_array;					/* (array) carbon content of seed*/
	double** utiliz_coeff_array;				/* (array) useful part of the seed */
} planting_struct;

/* strucure for fertilizing paramteres - by Hidy 2008 */
typedef struct
{
	int FRZ_flag;								/* (flag) 1=fertilizing , 0=no fertilizing */
	int mgmd;
	double FRZ_pool_act;						/* (value) actual  nitrogen content of fertilization above the ground *//* (flag) 1=do management , 0=no management on actual day */
    double** FRZdays_array;						/* (array) contains the fertilizing days in 1 year*/
	double** fertilizer_array;					/* (array) contains the amount of fertilizer on the fertilizing days*/
	double** Ncontent_array;					/* (array) nitrate content of fertilizer*/
 	double** NH3content_array;					/* (array) ammonium content of fertilizer*/
	double** Ccontent_array;					/* (array) carbon content of fertilizer*/
	double** litr_flab_array;					/* (array) labile fraction of fertilizer*/
	double** litr_fucel_array;					/* (array) unshielded cellulose fraction of fertilizer*/
	double** litr_fscel_array;					/* (array) shielded cellulose fraction of fertilizer*/
	double** litr_flig_array;					/* (array) lignin fraction of fertilizer*/
    double** dissolv_coeff_array;				/* (array) define the quantity of the N from fertilization (ratio) get into the mineralized N pool on a given day*/
    double** utiliz_coeff_array;				/* (array) useful part of the fertilizer */
	double** EFfert_N2O;						/* (array) fertilization emission factor for direct N2O emissions from synthetic fertililers	 */
	double DC_act;
	double UC_act;
	double Ncontent_act;				
 	double NH3content_act;				
	double Ccontent_act;	
	double flab_act;
	double fucel_act;
	double fscel_act;
	double flig_act;
	double EFf_N2O_act;
} fertilizing_struct;


/* strucure for irrigation paramteres - by Hidy 2015. */
typedef struct
{
	int IRG_flag;								/* (flag) 1=do irrigation , 0=no irrigation */	
	int mgmd;									/* (flag) 1=do management , 0=no management on actual day */
    double** IRGdays_array;						/* (array) contains the irrigation days in 1 year*/
	double** IRGquantity_array;					/* (array) quantity of water*/

} irrigation_struct;

/* structure for the photosynthesis routine */
typedef struct
{
	int c3;                 /* (flag) set to 1 for C3 model, 0 for C4 model */
	double pa;              /* (Pa) atmospheric pressure */
	double co2;             /* (ppm) atmospheric [CO2] */
	double t;               /* (deg C) temperature */
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

typedef struct
{
	double ta;              /* (degC) air temperature */
	double pa;              /* (Pa)   air pressure */
	double vpd;             /* (Pa)   vapor pressure deficit */
	double irad;            /* (W/m2) incident shortwave flux density */
	double rv;              /* (s/m)  resistance to water vapor flux */
	double rh;              /* (s/m)  resistance to sensible heat flux */
} pmet_struct;

typedef struct
{
	double daily_nep;      /* kgC/m2/day = NPP - Rheretotrop */
	double daily_npp;      /* kgC/m2/day = GPP - Rmaint - Rgrowth */
	double daily_nee;      /* kgC/m2/day = GPP - Rmaint - Rgrowth - Rheretotrop - fire losses */
	double daily_nbp;      /* kgC/m2/day = GPP - Rmaint - Rgrowth - Rheretotrop - disturb_emission - fire losses*/
	double daily_gpp;      /* kgC/m2/day  gross PSN source */
	double daily_mr;       /* kgC/m2/day  maintenance respiration */
	double daily_gr;       /* kgC/m2/day  growth respiration */
	double daily_hr;       /* kgC/m2/day  heterotrophic respiration */
	double daily_sr;       /* kgC/m2/day  Hidy 2012 - soil respiration */
	double daily_tr;       /* kgC/m2/day  Hidy 2013 - total respiration */
	double daily_fire;     /* kgC/m2/day  fire losses */
	double daily_litfallc; /* kgC/m2/day  total litterfall aboveground */
	double daily_litfallc_above; /* kgC/m2/day  total litterfall aboveground */
	double daily_litfallc_below; /* kgC/m2/day  total litterfall belowground*/
	double daily_litter ;	/* kgC/m2  total litter*/
	double daily_litdecomp;	/* kgC/m2/day  total litter decomposition*/
	double daily_litfire;
	double cum_npp_ann;    /* kgC/m2  Summed over a year */
	double cum_npp;        /* kgC/m2  Summed over entire simulation */
	double cum_nep;        /* kgC/m2  Summed over entire simulation */
	double cum_nee;        /* kgC/m2  Summed over entire simulation */
	double cum_gpp;        /* kgC/m2  Summed over entire simulation */
	double cum_mr;         /* kgC/m2  Summed over entire simulation */
	double cum_gr;         /* kgC/m2  Summed over entire simulation */
	double cum_hr;         /* kgC/m2  Summed over entire simulation */
	double cum_fire;       /* kgC/m2  Summed over entire simulation */
	double cum_nplus;		/* kgN/m2 Summed over entire simulation */
	double vwc_annavg;
	double vegc;           /* kgC/m2  total vegetation C */
	double abgc;           /* kgC/m2  total abovegound biomass C */
	double litrc;          /* kgC/m2  total litter C */
	double soilc;          /* kgC/m2  total soil C */
	double soiln;          /* kgC/m2  total soil N */
	double totalc;         /* kgC/m2  total of vegc, litrc, and soilc */
	double sminn;          /* kgC/m2  total soil mineralized N */
	/*effect of planting, thinning, mowing, grazing, harvesting, ploughing and fertilizing   - Hidy 2012.*/
	double Cchange_THN;    /* kgC/m2  total of thinning carbon change   */
	double Cchange_MOW;    /* kgC/m2  total of mowing carbon change   */
	double Cchange_HRV;    /* kgC/m2  total of harvesting carbon change   */
	double Cchange_PLG;	   /* kgC/m2  total of plouging carbon change   */
	double Cchange_GRZ;	   /* kgC/m2  total of grazing carbon change   */
	double Cchange_PLT;    /* kgC/m2  total of planting carbon change   */
	double Cchange_FRZ;    /* kgC/m2  total of fertilizing carbon change   */
	double Cchange_SNSC;   /* kgC/m2  total of senescence carbon change   */
	double Nplus_GRZ;      /* kgN/m2  N from grazing   */
	double Nplus_FRZ;      /* kgN/m2  N from fertilizing   */
	/* new tpye of output data - Hidy 2015 */
	double cum_ET;			/* kgH2O/m2	ET summed over entire simulation */

} summary_struct;

/* restart data structure */
typedef struct
{
	double soilw[N_SOILLAYERS];
	double snoww;
	double canopyw;
	double leafc;
	double leafc_storage;
	double leafc_transfer;
	double frootc;
	double frootc_storage;
	double frootc_transfer;
	double livestemc;
	double livestemc_storage;
	double livestemc_transfer;
	double deadstemc;
	double deadstemc_storage;
	double deadstemc_transfer;
	double livecrootc;
	double livecrootc_storage;
	double livecrootc_transfer;
	double deadcrootc;
	double deadcrootc_storage;
	double deadcrootc_transfer;
	double gresp_storage;
	double gresp_transfer;
	double cwdc;
	double litr1c;
	double litr2c;
	double litr3c;
	double litr4c;
	/* Hidy 2015: standing dead biome, cut-dowd dead biome and litter */
	double litr1c_STDB;
	double litr2c_STDB;
	double litr3c_STDB;
	double litr4c_STDB;
	double litr1c_strg_HRV;
	double litr2c_strg_HRV;
	double litr3c_strg_HRV;
	double litr4c_strg_HRV;
	double litr1c_strg_MOW;
	double litr2c_strg_MOW;
	double litr3c_strg_MOW;
	double litr4c_strg_MOW;
	double litr1c_strg_THN;
	double litr2c_strg_THN;
	double litr3c_strg_THN;
	double litr4c_strg_THN;
	double litr_aboveground;
	double litr_belowground;
	double soil1c;
	double soil2c;
	double soil3c;
	double soil4c;
	double cpool;
	double leafn;
	double leafn_storage;
	double leafn_transfer;
	double frootn;
	double frootn_storage;
	double frootn_transfer;
	double livestemn;
	double livestemn_storage;
	double livestemn_transfer;
	double deadstemn;
	double deadstemn_storage;
	double deadstemn_transfer;
	double livecrootn;
	double livecrootn_storage;
	double livecrootn_transfer;
	double deadcrootn;
	double deadcrootn_storage;
	double deadcrootn_transfer;
	double cwdn;
	double litr1n;
	double litr2n;
	double litr3n;
	double litr4n;
	/* Hidy 2015: standing dead biome, cut-dowd dead biome and litter */
	double litr1n_STDB;
	double litr2n_STDB;
	double litr3n_STDB;
	double litr4n_STDB;
	double litr1n_strg_HRV;
	double litr2n_strg_HRV;
	double litr3n_strg_HRV;
	double litr4n_strg_HRV;
	double litr1n_strg_MOW;
	double litr2n_strg_MOW;
	double litr3n_strg_MOW;
	double litr4n_strg_MOW;
	double litr1n_strg_THN;
	double litr2n_strg_THN;
	double litr3n_strg_THN;
	double litr4n_strg_THN;
	double CTDBc;

	double soil1n;
	double soil2n;
	double soil3n;
	double soil4n;
	double retransn;
	double npool;
	double day_leafc_litfall_increment;
	double day_frootc_litfall_increment;
	double day_livestemc_turnover_increment;
	double day_livecrootc_turnover_increment;
	double annmax_leafc;
	double annmax_frootc;
	double annmax_livestemc;
	double annmax_livecrootc;
	double dsr;
	int metyr;
	/* Hidy 2011 - multilayer soil */
	double sminn[N_SOILLAYERS];
	/* fruit simulation - Hidy 2013. */
	double fruitc;
	double fruitc_storage;
	double fruitc_transfer;
	double fruitn;
	double fruitn_storage;
	double fruitn_transfer;
	double day_fruitc_litfall_increment;
	double annmax_fruitc;
	/* softstem simulation - Hidy 2013. */
	double softstemc;
	double softstemc_storage;
	double softstemc_transfer;
	double softstemn;
	double softstemn_storage;
	double softstemn_transfer;
	double day_softstemc_litfall_increment;
	double annmax_softstemc;
} restart_data_struct;
