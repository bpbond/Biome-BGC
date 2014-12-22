# makefile for: bgclib411.a
#
# Creates the BIOME-BGC core science library, Version 4.1.1
#
# 17 July 1999
#
# Peter Thornton
# Numerical Terradynamics Simulation Group
# School of Forestry, University of Montana
# Missoula, MT 59812
# *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
# Biome-BGC version 4.1.1
# Copyright 1999, Peter E. Thornton
# Numerical Terradynamics Simulation Group (NTSG)
# School of Forestry, University of Montana
# Missoula, MT 59812
# *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
# Modified:
# 6/30/2000 (PET): updated directory names for Biome-BGC version 4.1.1
# 
# For a new compilation you will need to change the ROOTDIR
# definition below to represent your own directory structure.


ROOTDIR = /ntsg/peter/unixbgc411/src
LIBDIR = ${ROOTDIR}/lib
INCDIR = ${ROOTDIR}/include

OBJS = bgc.o spinup_bgc.o output_map_init.o make_zero_flux_struct.o atm_pres.o\
	prephenology.o restart_io.o firstday.o zero_srcsnk.o daymet.o\
	dayphen.o phenology.o radtrans.o prcp_route.o snowmelt.o baresoil_evap.o\
	soilpsi.o maint_resp.o canopy_et.o photosynthesis.o outflow.o decomp.o \
	daily_allocation.o annual_rates.o growth_resp.o state_update.o \
	nleaching.o mortality.o check_balance.o summary.o spinup_daily_allocation.o\
	precision_control.o

INCLUDE = ${INCDIR}/bgc_struct.h ${INCDIR}/bgc_func.h ${INCDIR}/bgc_constants.h

CFLAGS = -O -I${INCDIR}

CC = xlC

all : bgclib411.a

bgclib411.a : ${OBJS}
	ar r $@ $?
	mv $@ ${LIBDIR}

${OBJS} : ${INCLUDE}
bgc.o : ${INCDIR}/ini.h
bgc.o : ${INCDIR}/bgc_io.h
spinup_bgc.o : ${INCDIR}/ini.h
spinup_bgc.o : ${INCDIR}/bgc_io.h
