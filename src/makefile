# recursive makefile for:
# 1) BIOME-BGC core science library
# 2) pointbgc executable for single-point, single-biome BIOME-BGC simulations
#
# invoke by issuing command "make" from this directory
#
# 19 July 1999
#
# Peter Thornton
# Numerical Terradynamics Simulation Group
# School of Forestry, University of Montana
# Missoula, MT 59812

all : bgclib_obj pointbgc_obj

bgclib_obj :
	 cd bgclib_v4.1.1 ; make all

pointbgc_obj :
	cd pointbgc_v4.1.1 ; make all
