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

ifeq ($(WITH_OTF2),YES)
# enable OTF2 tracing
	SUFFIX	= .otf2
else
# no tracing
	SUFFIX	= .x
endif

PREFIX	= .

proxies := pairs ping-ping spread stencil3d stencil4d subcom2d-coll \
		   subcom3d-a2a unstr-mesh

.PHONY: all $(proxies)
all: $(proxies)

$(proxies):
	$(MAKE) --directory=$@

clean:
	for dir in $(proxies); \
	do					   \
		$(MAKE) --directory=$$dir clean; \
	done

.PHONY: install
install: $(proxies)
	mkdir -p $(PREFIX)/bin
	for dir in $(proxies); \
	do					   \
		cp $$dir/$$dir$(SUFFIX) $(PREFIX)/bin/; \
	done
