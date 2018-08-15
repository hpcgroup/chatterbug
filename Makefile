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

proxies := pairs ping-ping qbox spread stencil3d stencil4d subcom-a2a unstr-mesh

.PHONY: all $(proxies)
all: $(proxies)

$(proxies):
	$(MAKE) --directory=$@

clean:
	for dir in $(proxies) $(libraries);  \
	do									 \
		$(MAKE) --directory=$$dir clean; \
	done
