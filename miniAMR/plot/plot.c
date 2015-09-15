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
#include <string.h>

#include "proto.h"
#include "vars.h"

int main(int argc, char** argv)
{
   int i, out;

   out = 0;
   dir = -1;
   for (i = 1; i < argc; i++)
      if (!strcmp(argv[i], "-i")) {
         if ((i+1 == argc) || (strlen(argv[i+1]) == 0)) {
            printf("The input file pathname is empty.  Aborting.\n");
            print_help_message ();
         }
         if (strlen(argv[i+1]) > 1023) {
            printf("The input file pathname is too long.  Aborting.\n");
            print_help_message ();
         }
         strcpy(infile, argv[++i]);
      } else if (!strcmp(argv[i], "-o")) {
         if ((i+1 == argc) || (strlen(argv[i+1]) == 0)) {
            printf("The output file pathname is empty.  Aborting.\n");
            print_help_message ();
         }
         if (strlen(argv[i+1]) > 1023) {
            printf("The output file pathname is too long.  Aborting.\n");
            print_help_message ();
         }
         strcpy(outfile, argv[++i]);
         out++;
      } else if (!strcmp(argv[i], "-3")) {
         dir = 10;
      } else if (!strcmp(argv[i], "-x")) {
         dir = 0;
         plane = atof(argv[++i]);
      } else if (!strcmp(argv[i], "-y")) {
         dir = 1;
         plane = atof(argv[++i]);
      } else if (!strcmp(argv[i], "-z")) {
         dir = 2;
         plane = atof(argv[++i]);
      } else if (!strcmp(argv[i], "--help")) {
         print_help_message();
      } else {
         printf("** Error ** Unknown input parameter %s\n", argv[i]);
         print_help_message();
      }

   if (!out)
      strcpy(outfile, "plot.gen");

   if (check_input())
      exit(-1);

   read_input();

   if (dir == 10) {
      sort_data_3d();
      write_genesis_3d();
   } else {
      sort_data();
      write_genesis();
   }
}

void print_help_message(void)
{
   printf("(Optional) command line input is of the form: \n\n");

   printf("-3 - output all blocks in 3D\n");
   printf("-x - cutting plane (>= 0.0 && <= 1.0)\n");
   printf("-y - cutting plane (>= 0.0 && <= 1.0)\n");
   printf("-z - cutting plane (>= 0.0 && <= 1.0)\n");
   printf("-i - input filename\n");
   printf("-o - output filename (genesis file)\n");
}

int check_input(void)
{
   int error = 0;

   if (dir == -1) {
      printf("No direction chosen\n");
      error = 1;
   }
   if (plane < 0.0 || plane > 1.0) {
      printf("Cutting plane must be >= 0.0 and <= 1.0\n");
      error = 1;
   }

   return(error);
}
