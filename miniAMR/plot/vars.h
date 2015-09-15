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

int dir;
double plane;
char infile[1024];
char outfile[1024];
int num_l;
int nb[3];
int *p2;
int dir_i;
int dir_j;
int num_procs;

int num_blocks;
typedef struct {
   int number;
   int level;
   int proc;
   int cen[3];
   int cor_i[3][2];
   double cor[3][2];
} block;
block *blocks;

int num_elem;
int *num_el_l;
int num_el_b;
int **el_conn;
double **proc;
int num_nodes;
int mesh_i;
int mesh_j;
int mesh_k;
int **mesh;
int ***mesh3;
double *ci, *cj, *ck;
