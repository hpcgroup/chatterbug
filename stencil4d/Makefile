##############################################################################
# Copyright (c) 2018, Lawrence Livermore National Security, LLC.
# Produced at the Lawrence Livermore National Laboratory.
#
# Written by:
#     Nikhil Jain <nikhil.jain@acm.org>
#     Abhinav Bhatele <bhatele@llnl.gov>
#
# LLNL-CODE-756471. All rights reserved.
#
# This file is part of Chatterbug. For details, see:
# https://github.com/LLNL/chatterbug
# Please also read the LICENSE file for the MIT License notice.
##############################################################################

include ../Makefile.common

all: stencil4d$(SUFFIX) 

stencil4d$(SUFFIX) : stencil4d.C
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f stencil4d$(SUFFIX) 
