# makefile for: pointbgc
#
# Creates the executable for single-point, single-biome BIOME-BGC simulations.
# Uses the BIOME-BGC core science library
#
# 9 April 2002
#
# For a new compilation you will have to change the ROOTDIR
# definition to match your directory structure

BINDIR = ${ROOTDIR}/..
BGCLIB = ${LIBDIR}/bgclib-${VERSION}.a

ALLOBJS = ${OBJS1} ${OBJS2}  ${BGCLIB}
OBJS1 = pointbgc.o met_init.o restart_init.o time_init.o scc_init.o co2_init.o\
	sitec_init.o epc_init.o state_init.o output_init.o metarr_init.o\
	presim_state_init.o ramp_ndep_init.o output_ctrl.o ndep_init.o
OBJS2 = end_init.o ini.o
OBJS4 = restart_diff.o

INCLUDE1 = ${INCDIR}/ini.h ${INCDIR}/bgc_struct.h ${INCDIR}/pointbgc_struct.h\
	${INCDIR}/pointbgc_func.h 
INCLUDE2 = ${INCDIR}/ini.h
INCLUDE3 = ${INCDIR}/misc_func.h

all : bgc restart_diff

tools: restart_diff 

bgc : ${OBJS1} ${OBJS2}
	${CC} -o $@ ${CFLAGS} ${ALLOBJS} ${LDFLAGS}
	mv $@ ${BINDIR}

restart_diff: ${OBJS1} ${OBJS2} $(OBJS4)
	${CC} -o restart_diff ${CFLAGS} ${OBJS4} ${LDFLAGS}
	mv restart_diff ${BINDIR}

${OBJS1} : ${INCLUDE1}
${OBJS2} : ${INCLUDE2}
metarr_init.o : ${INCLUDE3}
state_init.o : ${INCDIR}/bgc_constants.h
pointbgc.o : ${INCDIR}/bgc_io.h
pointbgc.o : ${BGCLIB}

clean : 
	 - rm -f ${OBJS1} ${OBJS2} ${OBJS4} ${BINDIR}/restart_diff ${BINDIR}/bgc 
