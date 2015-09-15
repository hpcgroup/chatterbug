// ************************************************************************
//
// miniAMR: stencil computations with boundary exchange and AMR.
//
// Copyright (2014) Sandia Corporation. Under the terms of Contract
// DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government 
// retains certain rights in this software.
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
// Questions? Contact Courtenay T. Vaughan (ctvaugh@sandia.gov)
//                    Richard F. Barrett (rfbarre@sandia.gov)
//
// ************************************************************************

#include <stdlib.h>
#include <stdio.h>

#include "vars.h"

void read_input(void)
{
   int i, j, k, n, p;
   FILE *fd;

   fd = fopen(infile, "r");
   fscanf(fd, "%d %d %d %d %d", &num_blocks, &num_l, &nb[0], &nb[1], &nb[2]);
   p2 = (int *) malloc((num_l+2)*sizeof(int));
   p2[0] = 1;
   for (i = 0; i < num_l+1; i++)
      p2[i+1] = 2*p2[i];
   blocks = (block *) malloc(num_blocks*sizeof(block));
   for (p = i = 0; i < num_blocks; p++) {
      fscanf(fd, "%d", &n);
      for (k = 0; k < n; k++, i++) {
         blocks[i].number = i;
         blocks[i].proc = p;
         fscanf(fd, "%d %d %d %d", &blocks[i].level, &blocks[i].cen[0],
                &blocks[i].cen[1], &blocks[i].cen[2]);
         for (j = 0; j < 3; j++) {
            blocks[i].cor_i[j][0] = blocks[i].cen[j] -
                                    p2[num_l-blocks[i].level];
            blocks[i].cor[j][0] = ((double) blocks[i].cor_i[j][0])/
                                  ((double) nb[j]*p2[num_l+1]);
            blocks[i].cor_i[j][1] = blocks[i].cen[j] +
                                    p2[num_l-blocks[i].level];
            blocks[i].cor[j][1] = ((double) blocks[i].cor_i[j][1])/
                                  ((double) nb[j]*p2[num_l+1]);
         }
      }
   }
   num_procs = p;

   fclose(fd);
}
