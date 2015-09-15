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

void sort_data(void)
{
   int i, j, *i1;
   block *bp;

   num_el_l = (int *) malloc((num_l+1)*sizeof(int));
   for (i = 0; i < num_l+1; i++)
      num_el_l[i] = 0;
   if (dir == 0) {
      dir_i = 1;
      dir_j = 2;
   } else if (dir == 1) {
      dir_i = 0;
      dir_j = 2;
   } else {
      dir_i = 0;
      dir_j = 1;
   }
   mesh_i = nb[dir_i]*p2[num_l+1] + 1;
   mesh_j = nb[dir_j]*p2[num_l+1] + 1;
   mesh = (int **) malloc((mesh_j)*sizeof(int *));
   for (j = 0; j < mesh_j; j++) {
      mesh[j] = (int *) malloc((mesh_i)*sizeof(int));
      for (i = 0; i < mesh_i; i++)
         mesh[j][i] = 0;
   }
   for (num_elem = i = 0; i < num_blocks; i++)
      if (blocks[i].cor[dir][0] <= plane && blocks[i].cor[dir][1] >= plane) {
         mesh[blocks[i].cor_i[dir_j][0]][blocks[i].cor_i[dir_i][0]]++;
         mesh[blocks[i].cor_i[dir_j][0]][blocks[i].cor_i[dir_i][1]]++;
         mesh[blocks[i].cor_i[dir_j][1]][blocks[i].cor_i[dir_i][0]]++;
         mesh[blocks[i].cor_i[dir_j][1]][blocks[i].cor_i[dir_i][1]]++;
         num_elem++;
         num_el_l[blocks[i].level]++;
      } else
         blocks[i].number = -1;
   for (num_nodes = j = 0; j < mesh_j; j++)
      for (i = 0; i < mesh_i; i++)
         if (mesh[j][i])
            mesh[j][i] = ++num_nodes;
         else
            mesh[j][i] = -1;
   ci = (double *) malloc(num_nodes*sizeof(double));
   cj = (double *) malloc(num_nodes*sizeof(double));
   for (j = 0; j < mesh_j; j++)
      for (i = 0; i < mesh_i; i++)
         if (mesh[j][i] >= 0) {
            ci[mesh[j][i]-1] = ((double) i)/((double) mesh_i);
            cj[mesh[j][i]-1] = ((double) j)/((double) mesh_j);
         }
   el_conn = (int **) malloc((num_l+1)*sizeof(int *));
   proc = (double **) malloc((num_l+1)*sizeof(double *));
   i1 = (int *) malloc((num_l+1)*sizeof(int));
   for (num_el_b = i = 0; i < num_l+1; i++)
      if (num_el_l[i] > 0) {
         el_conn[i] = (int *) malloc(4*num_el_l[i]*sizeof(int));
         proc[i] = (double *) malloc(num_el_l[i]*sizeof(double));
         i1[i] = 0;
         num_el_b++;
      }
   for (i = 0; i < num_blocks; i++)
      if ((bp = &blocks[i])->number >= 0) {
         j = bp->level;
         proc[j][i1[j]/4] = (double) bp->proc;
         el_conn[j][i1[j]++] = mesh[bp->cor_i[dir_j][0]][bp->cor_i[dir_i][0]];
         el_conn[j][i1[j]++] = mesh[bp->cor_i[dir_j][0]][bp->cor_i[dir_i][1]];
         el_conn[j][i1[j]++] = mesh[bp->cor_i[dir_j][1]][bp->cor_i[dir_i][1]];
         el_conn[j][i1[j]++] = mesh[bp->cor_i[dir_j][1]][bp->cor_i[dir_i][0]];
      }
   free(i1);
}

void sort_data_3d(void)
{
   int i, j, k, *i1;
   block *bp;

   num_el_l = (int *) malloc((num_procs)*sizeof(int));
   for (i = 0; i < num_procs; i++)
      num_el_l[i] = 0;
   mesh_i = nb[0]*p2[num_l+1] + 1;
   mesh_j = nb[1]*p2[num_l+1] + 1;
   mesh_k = nb[2]*p2[num_l+1] + 1;
   mesh3 = (int ***) malloc((mesh_k)*sizeof(int **));
   for (k = 0; k < mesh_k; k++) {
      mesh3[k] = (int **) malloc((mesh_j)*sizeof(int *));
      for (j = 0; j < mesh_j; j++) {
         mesh3[k][j] = (int *) malloc((mesh_i)*sizeof(int));
         for (i = 0; i < mesh_i; i++)
            mesh3[k][j][i] = 0;
      }
   }
   for (i = 0; i < num_blocks; i++) {
      bp = &blocks[i];
      mesh3[bp->cor_i[2][0]][bp->cor_i[1][0]][bp->cor_i[0][0]]++;
      mesh3[bp->cor_i[2][0]][bp->cor_i[1][0]][bp->cor_i[0][1]]++;
      mesh3[bp->cor_i[2][0]][bp->cor_i[1][1]][bp->cor_i[0][0]]++;
      mesh3[bp->cor_i[2][0]][bp->cor_i[1][1]][bp->cor_i[0][1]]++;
      mesh3[bp->cor_i[2][1]][bp->cor_i[1][0]][bp->cor_i[0][0]]++;
      mesh3[bp->cor_i[2][1]][bp->cor_i[1][0]][bp->cor_i[0][1]]++;
      mesh3[bp->cor_i[2][1]][bp->cor_i[1][1]][bp->cor_i[0][0]]++;
      mesh3[bp->cor_i[2][1]][bp->cor_i[1][1]][bp->cor_i[0][1]]++;
      num_el_l[blocks[i].proc]++;
   }
   num_elem = num_blocks;
   for (num_nodes = k = 0; k < mesh_k; k++)
      for (j = 0; j < mesh_j; j++)
         for (i = 0; i < mesh_i; i++)
            if (mesh3[k][j][i])
               mesh3[k][j][i] = ++num_nodes;
            else
               mesh3[k][j][i] = -1;
   ci = (double *) malloc(num_nodes*sizeof(double));
   cj = (double *) malloc(num_nodes*sizeof(double));
   ck = (double *) malloc(num_nodes*sizeof(double));
   for (k = 0; k < mesh_k; k++)
      for (j = 0; j < mesh_j; j++)
         for (i = 0; i < mesh_i; i++)
            if (mesh3[k][j][i] >= 0) {
               ci[mesh3[k][j][i]-1] = ((double) i)/((double) mesh_i);
               cj[mesh3[k][j][i]-1] = ((double) j)/((double) mesh_j);
               ck[mesh3[k][j][i]-1] = ((double) k)/((double) mesh_k);
            }
   el_conn = (int **) malloc((num_procs)*sizeof(int *));
   proc = (double **) malloc((num_procs)*sizeof(double *));
   i1 = (int *) malloc((num_procs)*sizeof(int));
   for (num_el_b = i = 0; i < num_procs; i++)
      if (num_el_l[i] > 0) {
         el_conn[i] = (int *) malloc(8*num_el_l[i]*sizeof(int));
         proc[i] = (double *) malloc(num_el_l[i]*sizeof(double));
         i1[i] = 0;
         num_el_b++;
      }
   for (i = 0; i < num_blocks; i++) {
      bp = &blocks[i];
      j = bp->proc;
      proc[j][i1[j]/8] = (double) bp->proc;
      el_conn[j][i1[j]++] =
                      mesh3[bp->cor_i[2][0]][bp->cor_i[1][0]][bp->cor_i[0][0]];
      el_conn[j][i1[j]++] =
                      mesh3[bp->cor_i[2][0]][bp->cor_i[1][0]][bp->cor_i[0][1]];
      el_conn[j][i1[j]++] =
                      mesh3[bp->cor_i[2][0]][bp->cor_i[1][1]][bp->cor_i[0][1]];
      el_conn[j][i1[j]++] =
                      mesh3[bp->cor_i[2][0]][bp->cor_i[1][1]][bp->cor_i[0][0]];
      el_conn[j][i1[j]++] =
                      mesh3[bp->cor_i[2][1]][bp->cor_i[1][0]][bp->cor_i[0][0]];
      el_conn[j][i1[j]++] =
                      mesh3[bp->cor_i[2][1]][bp->cor_i[1][0]][bp->cor_i[0][1]];
      el_conn[j][i1[j]++] =
                      mesh3[bp->cor_i[2][1]][bp->cor_i[1][1]][bp->cor_i[0][1]];
      el_conn[j][i1[j]++] =
                      mesh3[bp->cor_i[2][1]][bp->cor_i[1][1]][bp->cor_i[0][0]];
   }
   free(i1);
}
