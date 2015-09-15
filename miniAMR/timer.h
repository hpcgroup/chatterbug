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

EXT_VAR THREAD_VAR double average[128];
EXT_VAR THREAD_VAR double stddev[128];
EXT_VAR THREAD_VAR double minimum[128];
EXT_VAR THREAD_VAR double maximum[128];

EXT_VAR THREAD_VAR double timer_all;

EXT_VAR THREAD_VAR double timer_comm_all;
EXT_VAR THREAD_VAR double timer_comm_dir[3];
EXT_VAR THREAD_VAR double timer_comm_recv[3];
EXT_VAR THREAD_VAR double timer_comm_pack[3];
EXT_VAR THREAD_VAR double timer_comm_send[3];
EXT_VAR THREAD_VAR double timer_comm_same[3];
EXT_VAR THREAD_VAR double timer_comm_diff[3];
EXT_VAR THREAD_VAR double timer_comm_bc[3];
EXT_VAR THREAD_VAR double timer_comm_wait[3];
EXT_VAR THREAD_VAR double timer_comm_unpack[3];

EXT_VAR THREAD_VAR double timer_calc_all;

EXT_VAR THREAD_VAR double timer_cs_all;
EXT_VAR THREAD_VAR double timer_cs_red;
EXT_VAR THREAD_VAR double timer_cs_calc;

EXT_VAR THREAD_VAR double timer_refine_all;
EXT_VAR THREAD_VAR double timer_refine_co;
EXT_VAR THREAD_VAR double timer_refine_mr;
EXT_VAR THREAD_VAR double timer_refine_cc;
EXT_VAR THREAD_VAR double timer_refine_sb;
EXT_VAR THREAD_VAR double timer_refine_c1;
EXT_VAR THREAD_VAR double timer_refine_c2;
EXT_VAR THREAD_VAR double timer_refine_sy;
EXT_VAR THREAD_VAR double timer_cb_all;
EXT_VAR THREAD_VAR double timer_cb_cb;
EXT_VAR THREAD_VAR double timer_cb_pa;
EXT_VAR THREAD_VAR double timer_cb_mv;
EXT_VAR THREAD_VAR double timer_cb_un;
EXT_VAR THREAD_VAR double timer_target_all;
EXT_VAR THREAD_VAR double timer_target_rb;
EXT_VAR THREAD_VAR double timer_target_dc;
EXT_VAR THREAD_VAR double timer_target_pa;
EXT_VAR THREAD_VAR double timer_target_mv;
EXT_VAR THREAD_VAR double timer_target_un;
EXT_VAR THREAD_VAR double timer_target_cb;
EXT_VAR THREAD_VAR double timer_target_ab;
EXT_VAR THREAD_VAR double timer_target_da;
EXT_VAR THREAD_VAR double timer_target_sb;
EXT_VAR THREAD_VAR double timer_lb_all;
EXT_VAR THREAD_VAR double timer_lb_sort;
EXT_VAR THREAD_VAR double timer_lb_pa;
EXT_VAR THREAD_VAR double timer_lb_mv;
EXT_VAR THREAD_VAR double timer_lb_un;
EXT_VAR THREAD_VAR double timer_lb_misc;
EXT_VAR THREAD_VAR double timer_lb_mb;
EXT_VAR THREAD_VAR double timer_lb_ma;
EXT_VAR THREAD_VAR double timer_rs_all;
EXT_VAR THREAD_VAR double timer_rs_ca;
EXT_VAR THREAD_VAR double timer_rs_pa;
EXT_VAR THREAD_VAR double timer_rs_mv;
EXT_VAR THREAD_VAR double timer_rs_un;

EXT_VAR THREAD_VAR double timer_plot;

EXT_VAR THREAD_VAR long total_blocks;
EXT_VAR THREAD_VAR int nb_min;
EXT_VAR THREAD_VAR int nb_max;
EXT_VAR THREAD_VAR int nrrs;
EXT_VAR THREAD_VAR int nrs;
EXT_VAR THREAD_VAR int nps;
EXT_VAR THREAD_VAR int nlbs;
EXT_VAR THREAD_VAR int num_refined;
EXT_VAR THREAD_VAR int num_reformed;
EXT_VAR THREAD_VAR int num_moved_all;
EXT_VAR THREAD_VAR int num_moved_lb;
EXT_VAR THREAD_VAR int num_moved_rs;
EXT_VAR THREAD_VAR int num_moved_reduce;
EXT_VAR THREAD_VAR int num_moved_coarsen;
EXT_VAR THREAD_VAR int counter_halo_recv[3];
EXT_VAR THREAD_VAR int counter_halo_send[3];
EXT_VAR THREAD_VAR double size_mesg_recv[3];
EXT_VAR THREAD_VAR double size_mesg_send[3];
EXT_VAR THREAD_VAR int counter_face_recv[3];
EXT_VAR THREAD_VAR int counter_face_send[3];
EXT_VAR THREAD_VAR int counter_bc[3];
EXT_VAR THREAD_VAR int counter_same[3];
EXT_VAR THREAD_VAR int counter_diff[3];
EXT_VAR THREAD_VAR int counter_malloc;
EXT_VAR THREAD_VAR double size_malloc;
EXT_VAR THREAD_VAR int counter_malloc_init;
EXT_VAR THREAD_VAR double size_malloc_init;
EXT_VAR THREAD_VAR int total_red;
