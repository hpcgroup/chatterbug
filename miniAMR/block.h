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

typedef struct {
   int number;
   int level;
   int refine;
   int new_proc;
   int parent;       // if original block -1,
                     // else if on node, number in structure
                     // else (-2 - parent->number)
   int parent_node;
   int child_number;
   int nei_refine[6];
   int nei_level[6];  /* 0 to 5 = W, E, S, N, D, U; use -2 for boundary */
   int nei[6][2][2];  /* negative if off processor (-1 - proc) */
   int cen[3];
   double ****array;
} block;
EXT_VAR THREAD_VAR block *blocks;

typedef struct {
   int number;
   int level;
   int parent;      // -1 if original block
   int parent_node;
   int child_number;
   int refine;
   int child[8];    // n if on node, number if not
                    // if negative, then onnode child is a parent (-1 - n)
   int child_node[8];
   int cen[3];
} parent;
EXT_VAR THREAD_VAR parent *parents;

typedef struct {
   int number;     // number of block
   int n;          // position in block array
} sorted_block;
EXT_VAR THREAD_VAR sorted_block *sorted_list;
EXT_VAR THREAD_VAR int *sorted_index;

EXT_VAR THREAD_VAR int my_pe;
EXT_VAR THREAD_VAR int num_pes;

EXT_VAR THREAD_VAR int max_num_blocks;
EXT_VAR THREAD_VAR int target_active;
EXT_VAR THREAD_VAR int target_max;
EXT_VAR THREAD_VAR int target_min;
EXT_VAR THREAD_VAR int num_refine;
EXT_VAR THREAD_VAR int uniform_refine;
EXT_VAR THREAD_VAR int x_block_size, y_block_size, z_block_size;
EXT_VAR THREAD_VAR int num_vars;
EXT_VAR THREAD_VAR int comm_vars;
EXT_VAR THREAD_VAR int init_block_x, init_block_y, init_block_z;
EXT_VAR THREAD_VAR int reorder;
EXT_VAR THREAD_VAR int npx, npy, npz;
EXT_VAR THREAD_VAR int inbalance;
EXT_VAR THREAD_VAR int refine_freq;
EXT_VAR THREAD_VAR int report_diffusion;
EXT_VAR THREAD_VAR int checksum_freq;
EXT_VAR THREAD_VAR int stages_per_ts;
EXT_VAR THREAD_VAR int error_tol;
EXT_VAR THREAD_VAR int num_tsteps;
EXT_VAR THREAD_VAR int stencil;
EXT_VAR THREAD_VAR int report_perf;
EXT_VAR THREAD_VAR int plot_freq;
EXT_VAR THREAD_VAR int lb_opt;
EXT_VAR THREAD_VAR int block_change;
EXT_VAR THREAD_VAR int code;
EXT_VAR THREAD_VAR int permute;
EXT_VAR THREAD_VAR int nonblocking;
EXT_VAR THREAD_VAR int refine_ghost;

EXT_VAR THREAD_VAR int max_num_parents;
EXT_VAR THREAD_VAR int num_parents;
EXT_VAR THREAD_VAR int max_active_parent;
EXT_VAR THREAD_VAR int cur_max_level;
EXT_VAR THREAD_VAR int *num_blocks;
EXT_VAR THREAD_VAR int *local_num_blocks;
EXT_VAR THREAD_VAR int *block_start;
EXT_VAR THREAD_VAR int num_active;
EXT_VAR THREAD_VAR int max_active_block;
EXT_VAR THREAD_VAR int global_active;
EXT_VAR THREAD_VAR int x_block_half, y_block_half, z_block_half;
EXT_VAR THREAD_VAR double tol;
EXT_VAR THREAD_VAR double *grid_sum;
EXT_VAR THREAD_VAR int *p8, *p2;
EXT_VAR THREAD_VAR int mesh_size[3];
EXT_VAR THREAD_VAR int max_mesh_size;
EXT_VAR THREAD_VAR int *from, *to;
EXT_VAR THREAD_VAR int msg_len[3][4];
EXT_VAR THREAD_VAR int local_max_b;
EXT_VAR THREAD_VAR int global_max_b;
EXT_VAR THREAD_VAR int num_objects;
typedef struct {
   int type;
   int bounce;
   double cen[3];
   double orig_cen[3];
   double move[3];
   double orig_move[3];
   double size[3];
   double orig_size[3];
   double inc[3];
} object;
EXT_VAR THREAD_VAR object *objects;

EXT_VAR THREAD_VAR int num_dots;
EXT_VAR THREAD_VAR int max_num_dots;
EXT_VAR THREAD_VAR int max_active_dot;
typedef struct {
   int cen[3];
   int number;
   int n;
   int proc;
   int new_proc;
} dot;
EXT_VAR THREAD_VAR dot *dots;
