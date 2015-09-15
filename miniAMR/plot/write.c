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

#include "netcdf.h"
#include "exodusII.h"
#include "vars.h"

void write_genesis(void)
{
   int exoid, num_dim, num_ns, num_ss, i, CPUws, IOws, num_ev;
   double time;
   char *cor_names[2], *var_name[1];

   CPUws = 8;
   IOws = 8;
   exoid = ex_create(outfile, EX_CLOBBER, &CPUws, &IOws);

   num_dim = 2;
   num_ns = 0;
   num_ss = 0;
   ex_put_init(exoid, "plot file from miniAMR", num_dim, num_nodes, num_elem,
               num_el_b, num_ns, num_ss);

   ck = (double *) malloc(num_nodes*sizeof(double));
   ex_put_coord(exoid, ci, cj, ck);
   if (dir_i == 0)
      cor_names[0] = "x_cor";
   else
      cor_names[0] = "y_cor";
   if (dir_j == 1)
      cor_names[1] = "y_cor";
   else
      cor_names[1] = "z_cor";
   ex_put_coord_names(exoid, cor_names);

   for (i = 0; i < num_l+1; i++)
      if (num_el_l[i] > 0)
         ex_put_elem_block(exoid, i+1, "QUAD", num_el_l[i], 4, 0);

   for (i = 0; i < num_l+1; i++)
      if (num_el_l[i] > 0)
         ex_put_elem_conn(exoid, i+1, el_conn[i]);

   num_ev = 1;
   var_name[0] = "proc";
   ex_put_var_param(exoid, "e", num_ev);
   ex_put_var_names(exoid, "e", num_ev, var_name);

   time = 0.0;
   ex_put_time(exoid, 1, &time);

   for (i = 0; i < num_l+1; i++)
      if (num_el_l[i] > 0)
         ex_put_elem_var(exoid, 1, 1, i+1, num_el_l[i], proc[i]);

   ex_update(exoid);

   ex_close(exoid);
}

void write_genesis_3d(void)
{
   int exoid, num_dim, num_ns, num_ss, i, CPUws, IOws, num_ev;
   double time;
   char *cor_names[3], *var_name[1];

   CPUws = 8;
   IOws = 8;
   exoid = ex_create(outfile, EX_CLOBBER, &CPUws, &IOws);

   num_dim = 3;
   num_ns = 0;
   num_ss = 0;
   ex_put_init(exoid, "plot file from miniAMR", num_dim, num_nodes, num_elem,
               num_el_b, num_ns, num_ss);

   ex_put_coord(exoid, ci, cj, ck);
   cor_names[0] = "x_cor";
   cor_names[1] = "y_cor";
   cor_names[2] = "z_cor";
   ex_put_coord_names(exoid, cor_names);

   for (i = 0; i < num_procs; i++)
      if (num_el_l[i] > 0)
         ex_put_elem_block(exoid, i+1, "HEX", num_el_l[i], 8, 0);

   for (i = 0; i < num_procs; i++)
      if (num_el_l[i] > 0)
         ex_put_elem_conn(exoid, i+1, el_conn[i]);

   num_ev = 1;
   var_name[0] = "proc";
   ex_put_var_param(exoid, "e", num_ev);
   ex_put_var_names(exoid, "e", num_ev, var_name);

   time = 0.0;
   ex_put_time(exoid, 1, &time);

   for (i = 0; i < num_procs; i++)
      if (num_el_l[i] > 0)
         ex_put_elem_var(exoid, 1, 1, i+1, num_el_l[i], proc[i]);

   ex_update(exoid);

   ex_close(exoid);
}
