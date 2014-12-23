# makefile for: bgclib412.a
#
# Creates the BIOME-BGC core science library
#
# 9 April 2002
#
# Peter Thornton
# 
# For a new compilation you will need to change the ROOTDIR
# definition below to represent your own directory structure.

OBJS = bgc.o output_map_init.o make_zero_flux_struct.o atm_pres.o\
	prephenology.o restart_io.o firstday.o zero_srcsnk.o daymet.o\
	dayphen.o phenology.o radtrans.o prcp_route.o snowmelt.o baresoil_evap.o\
	soilpsi.o maint_resp.o canopy_et.o photosynthesis.o outflow.o decomp.o \
	daily_allocation.o annual_rates.o growth_resp.o state_update.o \
	nleaching.o mortality.o check_balance.o summary.o smooth.o \
	precision_control.o bgc_io.o output_ascii.o get_co2.o get_ndep.o

INCLUDE = ${INCDIR}/bgc_struct.h ${INCDIR}/bgc_func.h ${INCDIR}/bgc_constants.h

all : bgclib

bgclib : ${OBJS}
	ar r $@-${VERSION}.a $?
	mv $@-${VERSION}.a ${LIBDIR}

${OBJS} : ${INCLUDE}
bgc.o : ${INCDIR}/ini.h
bgc.o : ${INCDIR}/bgc_io.h

clean : 
	- rm -f ${OBJS} ${LIBDIR}/bgclib-${VERSION}.a
