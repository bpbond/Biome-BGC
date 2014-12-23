# recursive makefile for Biome-BGC
# 1) BIOME-BGC core science library
# 2) pointbgc executable for single-point, single-biome BIOME-BGC simulations
#
# invoke by issuing command "make" from this directory
#

ROOTDIR=${PWD}
LIBDIR=${ROOTDIR}/lib
INCDIR=${ROOTDIR}/include
VERSION=4.2
USER=`whoami`
HOST=`hostname`

# Not including these CFLAGS/LDFLAGS will surely break something
# So DON'T MODIFY THESE LINES. Modify the platform specific lines down below
CFLAGS_GENERIC = -I${INCDIR} -DVERS="\\\"${VERSION}\\\"" -DUSER="\\\"${USER}\\\"" -DHOST="\\\"${HOST}\\\""
LDFLAGS_GENERIC = -lm

# For Linux
CFLAGS = -O3 -std=c99 ${CFLAGS_GENERIC} # Fully optimized and using ISO C99 features
# CFLAGS = -O3 -std=c99 -ffloat-store ${CFLAGS_GENERIC} # Use precise IEEE Floating Point
# CFLAGS = -g -Wall -ansi -pedantic -std=c89 ${CFLAGS_GENERIC} # 'standards' testing flags 
# CFLAGS = -g -Wall -ansi -pedantic -std=c99 ${CFLAGS_GENERIC} # testing with line/file reporting
LDFLAGS = ${LDFLAGS_GENERIC}
CC = gcc

# For Generic UNIX with debugging
# CFLAGS = -g ${CFLAGS_GENERIC}
# LDFLAGS = ${LDFLAGS_GENERIC}
# CC = cc

# For AIX
# CFLAGS = -O ${CFLAGS_GENERIC}
# LDFLAGS = ${LDFLAGS_GENERIC}
# CC = xlC

# For Insure++ analysis (on Hermes or Lugburz only)
#CFLAGS = -g ${CFLAGS_GENERIC}
#LDFLAGS = ${LDFLAGS_GENERIC}
#CC = insure

# SLH 9/21/04
# Using Insure++  
# You need an x-session DISPLAY set and and x-server running on your workstation
# 'export DISPLAY=saxon.ntsg.umt.edu:0', exceed running on workstation
# build via 'make CC=insure' or 'make test CC=insure'
# 'insra' to compile and view runtime ouput

MACROS=ROOTDIR=${ROOTDIR} LIBDIR=${LIBDIR} INCDIR=${INCDIR} \
	VERSION=${VERSION} CC=${CC} CFLAGS="${CFLAGS}" LDFLAGS="${LDFLAGS}" 

all : bgclib_obj pointbgc_obj

tools:
	cd pointbgc; ${MAKE} tools ${MACROS}

bgclib_obj :
	 cd bgclib ; ${MAKE} all ${MACROS}

pointbgc_obj :
	cd pointbgc ; ${MAKE} all ${MACROS}

clean : 
	cd bgclib ; ${MAKE} clean ${MACROS}
	cd pointbgc ; ${MAKE} clean ${MACROS}
	#-rm -f ../outputs/enf_test1* ../restart/enf_test1*

test : all
	cd ../; ./bgc ini/enf_test1_spinup.ini; ./bgc ini/enf_test1.ini -a
	
diff:	all tools
	cd ../; ./bgc ini/enf_test1_spinup.ini
	cd ../; ./restart_diff restart/enf_test1.std.endpoint restart/enf_test1.endpoint
