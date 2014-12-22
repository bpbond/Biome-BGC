# makefile for: pointbgc
#
# Creates the executable for single-point, single-biome BIOME-BGC simulations.
# Uses the BIOME-BGC core science library, Version 4.1.1
#
# 17 July 1999
#
# *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
# Biome-BGC version 4.1.1
# Copyright 2000, Peter E. Thornton
# Numerical Terradynamics Simulation Group (NTSG)
# School of Forestry, University of Montana
# Missoula, MT 59812
# *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
# Modified:
# 6/30/2000 (PET): updated directory names for Biome-BGC version 4.1.1
#
# For a new compilation you will have to change the ROOTDIR
# definition to match your directory structure

ROOTDIR = /ntsg/peter/unixbgc411/src
BINDIR = ${ROOTDIR}/..
INCDIR = ${ROOTDIR}/include
BGCLIBDIR = ${ROOTDIR}/lib

BGCLIB = ${BGCLIBDIR}/bgclib411.a

ALLOBJS = ${OBJS1} ${OBJS2} ${OBJS3} ${BGCLIB}
OBJS1 = pointbgc.o met_init.o restart_init.o time_init.o scc_init.o co2_init.o\
	sitec_init.o epc_init.o state_init.o output_init.o metarr_init.o\
	presim_state_init.o ramp_ndep_init.o
OBJS2 = end_init.o ini.o
OBJS3 = smooth.o metarr_init.o

INCLUDE1 = ${INCDIR}/ini.h ${INCDIR}/bgc_struct.h ${INCDIR}/pointbgc_struct.h\
	${INCDIR}/pointbgc_func.h 
INCLUDE2 = ${INCDIR}/ini.h
INCLUDE3 = ${INCDIR}/misc_func.h

LIB = -lm

CFLAGS = -O -I${INCDIR}

CC = xlC

all : pointbgc

pointbgc : ${OBJS1} ${OBJS2} ${OBJS3}
	${CC} -o $@ ${CFLAGS} ${ALLOBJS} ${LIB}
	mv $@ ${BINDIR}

${OBJS1} : ${INCLUDE1}
${OBJS2} : ${INCLUDE2}
${OBJS3} : ${INCLUDE3}
state_init.o : ${INCDIR}/bgc_constants.h
pointbgc.o : ${INCDIR}/bgc_io.h
pointbgc.o : ${BGCLIB}
