# Makefile for Biome-BGC MuSo
# Modified by Zoltan BARCZA and Richard KOVACS for Biome-BGC MuSo 4.0; Sept 2015
# REMARK: on MS Windows standard compiler settings are used [no optimization for speed]
# so we decided to use un-optimized comlining here as well, in order to match settings under Linux and Windows.
# This is why Generic UNIX flags are used

BINDIR = ${ROOTDIR}

ALLOBJS = ${OBJS} ${OBJS1} ${OBJS2}
OBJS = bgc.o output_map_init.o make_zero_flux_struct.o atm_pres.o\
        prephenology.o restart_io.o firstday.o zero_srcsnk.o daymet.o\
        dayphen.o phenology.o radtrans.o prcp_route.o snowmelt.o baresoil_evap.o\
        maint_resp.o canopy_et.o photosynthesis.o decomp.o \
        daily_allocation.o annual_rates.o growth_resp.o state_update.o \
        mortality.o check_balance.o summary.o smooth.o \
        precision_control.o conduct_calc.o conduct_limit_factors.o fertilizing.o grazing.o\
        GSI_calculation.o harvesting.o mowing.o multilayer_hydrolparams.o\
        multilayer_hydrolprocess.o multilayer_rootdepth.o multilayer_sminn.o\
        multilayer_transpiration.o multilayer_tsoil.o planting.o ploughing.o\
        senescence.o thinning.o groundwater.o richards.o\
        tipping.o irrigation.o transient_bgc.o flowering_heatstress.o\
	phenphase.o

OBJS1 = pointbgc.o met_init.o restart_init.o time_init.o scc_init.o co2_init.o\
	sitec_init.o epc_init.o state_init.o output_init.o metarr_init.o\
	presim_state_init.o spinup_bgc.o\
	fertilizing_init.o grazing_init.o harvesting_init.o mowing_init.o\
	planting_init.o ploughing_init.o thinning_init.o management.o read_mgmarray.o\
	groundwater_init.o ndep_init.o irrigation_init.o CH4flux_estimation.o cutdown2litter.o\
	multilayer_leaching.o output_handling.o 
	
OBJS2 = end_init.o ini.o

INCLUDE = ${INCDIR}/bgc_struct.h ${INCDIR}/bgc_func.h ${INCDIR}/bgc_constants.h
INCLUDE1 = ${INCDIR}/ini.h ${INCDIR}/bgc_struct.h ${INCDIR}/pointbgc_struct.h\
	${INCDIR}/pointbgc_func.h 
INCLUDE2 = ${INCDIR}/ini.h
INCLUDE3 = ${INCDIR}/misc_func.h

all : ${OBJS} ${OBJS1} ${OBJS2} ${OBJS}
	${CC} -o muso ${CFLAGS} ${ALLOBJS} ${LDFLAGS}
	mv muso ${BINDIR}

${OBJS} : ${INCLUDE}
${OBJS1} : ${INCLUDE1}
${OBJS2} : ${INCLUDE2}
metarr_init.o : ${INCLUDE3}
state_init.o : ${INCDIR}/bgc_constants.h
pointbgc.o : ${INCDIR}/bgc_io.h ${INCDIR}/bgc_epclist.h
bgc.o : ${INCDIR}/ini.h
bgc.o : ${INCDIR}/bgc_io.h

clean : 
	 - rm -f ${OBJS} ${OBJS1} ${OBJS2} ${BINDIR}/muso






