//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018, Lawrence Livermore National Security, LLC.
// Produced at the Lawrence Livermore National Laboratory.
//
// Written by:
//     Nikhil Jain <nikhil.jain@acm.org>
//     Abhinav Bhatele <bhatele@llnl.gov>
//
// LLNL-CODE-756471. All rights reserved.
//
// This file is part of Chatterbug. For details, see:
// https://github.com/LLNL/chatterbug
// Please also read the LICENSE file for the MIT License notice.
//////////////////////////////////////////////////////////////////////////////

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>

#if WRITE_OTF2_TRACE
#include <scorep/SCOREP_User.h>
#endif

#define calc_pe(a,b,c,d)	(a + b*nx + c*nx*ny + d*nx*ny*nz)
#define wrap_x(a)	(((a)+nx)%nx)
#define wrap_y(a)	(((a)+ny)%ny)
#define wrap_z(a)	(((a)+nz)%nz)
#define wrap_t(a)	(((a)+nt)%nt)

#define LEFT		    1
#define RIGHT		    2
#define TOP		      3
#define BOTTOM		  4
#define FRONT		    5
#define BACK		    6
#define FORWARD		  7
#define BACKWARD    8

int main(int argc, char **argv) {
  int myrank, numranks;
  MPI_Init(&argc, &argv);
#if WRITE_OTF2_TRACE
  SCOREP_RECORDING_OFF();
#endif
  MPI_Comm_size(MPI_COMM_WORLD, &numranks);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  
  if(argc != 11) {
    if(!myrank)
      printf("\nThis is the stencil4D (aka halo4d) communication proxy. The correct usage is:\n"
             "%s nx ny nz nt bx by bz bt nvar MAX_ITER\n\n"
             "    nx, ny, nz, nt: layout of process grid in 4D\n"
             "    bx, by, bz, bt: grid size on each process\n"
             "    nvar: number of variables at each grid point\n"
             "    MAX_ITER: how many iters to run\n\n",
             argv[0]);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  MPI_Request req[8];
  MPI_Request sreq[8];
  MPI_Status status[8];

  int nx = atoi(argv[1]);
  int ny = atoi(argv[2]);
  int nz = atoi(argv[3]);
  int nt = atoi(argv[4]);
  int bx = atoi(argv[5]);
  int by = atoi(argv[6]);
  int bz = atoi(argv[7]);
  int bt = atoi(argv[8]);
  int nvar = atoi(argv[9]);
  int MAX_ITER = atoi(argv[10]);

  if(nx * ny * nz * nt != numranks) {
    if(!myrank) {
      printf("\n nx * ny * nz * nt does not equal number of ranks. \n");
    }
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  // figure out my coordinates
  int myXcoord = (myrank % nx);
  int myYcoord = (myrank % (nx * ny)) / nx;
  int myZcoord = (myrank % (nx * ny * nz)) / (nx * ny);
  int myTcoord = (myrank % (nx * ny * nz * nt)) / (nx * ny * nz);

  if(myrank == 0) {
    printf("Running stencil4d on %d processors each with (%d, %d, %d, %d) grid points with %d variables\n", numranks, bx, by, bz, bt, nvar);
  }

  /* left, right, bottom, top, back, forward and backward  blocks into arrays.*/
  double *left_block_out    = (double *)malloc(sizeof(double) * by * bz * bt * nvar);
  double *right_block_out   = (double *)malloc(sizeof(double) * by * bz * bt * nvar);
  double *left_block_in     = (double *)malloc(sizeof(double) * by * bz * bt * nvar);
  double *right_block_in    = (double *)malloc(sizeof(double) * by * bz * bt * nvar);

  double *bottom_block_out  = (double *)malloc(sizeof(double) * bx * bz * bt * nvar);  
  double *top_block_out     = (double *)malloc(sizeof(double) * bx * bz * bt * nvar);
  double *bottom_block_in   = (double *)malloc(sizeof(double) * bx * bz * bt * nvar);
  double *top_block_in      = (double *)malloc(sizeof(double) * bx * bz * bt * nvar);
  
  double *front_block_out   = (double *)malloc(sizeof(double) * bx * by * bt * nvar);
  double *back_block_out    = (double *)malloc(sizeof(double) * bx * by * bt * nvar);
  double *front_block_in    = (double *)malloc(sizeof(double) * bx * by * bt * nvar);
  double *back_block_in     = (double *)malloc(sizeof(double) * bx * by * bt * nvar);
  
  double *forward_block_out   = (double *)malloc(sizeof(double) * bx * by * bz * nvar);
  double *backward_block_out  = (double *)malloc(sizeof(double) * bx * by * bz * nvar);
  double *forward_block_in    = (double *)malloc(sizeof(double) * bx * by * bz * nvar);
  double *backward_block_in   = (double *)malloc(sizeof(double) * bx * by * bz * nvar);

  double startTime, stopTime;

  MPI_Barrier(MPI_COMM_WORLD);
#if WRITE_OTF2_TRACE
  SCOREP_RECORDING_ON();
  // Marks the beginning of code region to be repeated in simulation
  SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_Loop", SCOREP_USER_REGION_TYPE_COMMON);
  // Marks when to print a timer in simulation
  if(!myrank)
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_WallTime_stencil4d", SCOREP_USER_REGION_TYPE_COMMON);
#endif
  
  startTime = MPI_Wtime();
  for (int i = 0; i < MAX_ITER; i++) {
#if WRITE_OTF2_TRACE
    // Marks compute region before messaging
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_stencil4d_pre_msg", SCOREP_USER_REGION_TYPE_COMMON);
    SCOREP_USER_REGION_BY_NAME_END("TRACER_stencil4d_pre_msg");
#endif
    // post receives: one for each direction and dimension
    MPI_Irecv(right_block_in, by * bz * bt * nvar, MPI_DOUBLE, calc_pe(wrap_x(myXcoord+1), myYcoord, myZcoord, myTcoord), RIGHT, MPI_COMM_WORLD, &req[RIGHT-1]);
    MPI_Irecv(left_block_in, by * bz * bt * nvar, MPI_DOUBLE, calc_pe(wrap_x(myXcoord-1), myYcoord, myZcoord, myTcoord), LEFT, MPI_COMM_WORLD, &req[LEFT-1]);
    MPI_Irecv(top_block_in, bx * bz * bt * nvar, MPI_DOUBLE, calc_pe(myXcoord,wrap_y(myYcoord+1), myZcoord, myTcoord), TOP, MPI_COMM_WORLD, &req[TOP-1]);
    MPI_Irecv(bottom_block_in, bx * bz * bt * nvar, MPI_DOUBLE, calc_pe(myXcoord,wrap_y(myYcoord-1), myZcoord, myTcoord), BOTTOM, MPI_COMM_WORLD, &req[BOTTOM-1]);
    MPI_Irecv(front_block_in, bx * by * bt * nvar, MPI_DOUBLE, calc_pe(myXcoord, myYcoord, wrap_z(myZcoord+1), myTcoord), FRONT, MPI_COMM_WORLD, &req[FRONT-1]);
    MPI_Irecv(back_block_in, bx * by * bt * nvar, MPI_DOUBLE, calc_pe(myXcoord, myYcoord, wrap_z(myZcoord-1), myTcoord), BACK, MPI_COMM_WORLD, &req[BACK-1]);
    MPI_Irecv(forward_block_in, bx * by * bz * nvar, MPI_DOUBLE, calc_pe(myXcoord, myYcoord, myZcoord, wrap_t(myTcoord+1)), FORWARD, MPI_COMM_WORLD, &req[FORWARD-1]);
    MPI_Irecv(backward_block_in, bx * by * bz * nvar, MPI_DOUBLE, calc_pe(myXcoord, myYcoord, myZcoord, wrap_t(myTcoord-1)), BACKWARD, MPI_COMM_WORLD, &req[BACKWARD-1]);

    // initiate sends: one for each direction and dimension
    MPI_Isend(left_block_out, by * bz * bt * nvar, MPI_DOUBLE, calc_pe(wrap_x(myXcoord-1), myYcoord, myZcoord, myTcoord), RIGHT, MPI_COMM_WORLD, &sreq[RIGHT-1]);
    MPI_Isend(right_block_out, by * bz * bt * nvar, MPI_DOUBLE, calc_pe(wrap_x(myXcoord+1), myYcoord, myZcoord, myTcoord), LEFT, MPI_COMM_WORLD, &sreq[LEFT-1]);
    MPI_Isend(bottom_block_out, bx * bz * bt * nvar, MPI_DOUBLE, calc_pe(myXcoord, wrap_y(myYcoord-1), myZcoord, myTcoord), TOP, MPI_COMM_WORLD, &sreq[TOP-1]);
    MPI_Isend(top_block_out, bx * bz * bt * nvar, MPI_DOUBLE, calc_pe(myXcoord, wrap_y(myYcoord+1), myZcoord, myTcoord), BOTTOM, MPI_COMM_WORLD, &sreq[BOTTOM-1]);
    MPI_Isend(back_block_out, bx * by * bt * nvar, MPI_DOUBLE, calc_pe(myXcoord, myYcoord, wrap_z(myZcoord-1), myTcoord), FRONT, MPI_COMM_WORLD, &sreq[FRONT-1]);
    MPI_Isend(front_block_out, bx * by * bt * nvar, MPI_DOUBLE, calc_pe(myXcoord, myYcoord, wrap_z(myZcoord+1), myTcoord), BACK, MPI_COMM_WORLD, &sreq[BACK-1]);
    MPI_Isend(backward_block_out, bx * by * bz * nvar, MPI_DOUBLE, calc_pe(myXcoord, myYcoord, myZcoord, wrap_t(myTcoord-1)), FORWARD, MPI_COMM_WORLD, &sreq[FORWARD-1]);
    MPI_Isend(forward_block_out, bx * by * bz * nvar, MPI_DOUBLE, calc_pe(myXcoord, myYcoord, myZcoord, wrap_t(myTcoord+1)), BACKWARD, MPI_COMM_WORLD, &sreq[BACKWARD-1]);

#if WRITE_OTF2_TRACE
    // Marks compute region for computation-communication overlap
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_stencil4d_overlap", SCOREP_USER_REGION_TYPE_COMMON);
    SCOREP_USER_REGION_BY_NAME_END("TRACER_stencil4d_overlap");
#endif
  
    //wait for all communication to complete
    MPI_Waitall(8, req, status);
    MPI_Waitall(8, sreq, status);

#if WRITE_OTF2_TRACE
    // Marks compute region after messaging
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_stencil4d_post_msg", SCOREP_USER_REGION_TYPE_COMMON);
    SCOREP_USER_REGION_BY_NAME_END("TRACER_stencil4d_post_msg");
#endif
  }

#if WRITE_OTF2_TRACE
  // Marks the end of code region to be repeated in simulation
  SCOREP_USER_REGION_BY_NAME_END("TRACER_Loop");
#endif
  MPI_Barrier(MPI_COMM_WORLD);
  stopTime = MPI_Wtime();

#if WRITE_OTF2_TRACE
  // Marks when to print a timer in simulation
  if(!myrank)
    SCOREP_USER_REGION_BY_NAME_END("TRACER_WallTime_stencil4d");
  SCOREP_RECORDING_OFF();
#endif

  //finalized summary output
  if(myrank == 0 && MAX_ITER != 0) {
    printf("Finished %d iterations\n",MAX_ITER);
    printf("Time elapsed per iteration for grid size (%d,%d,%d,%d) x %d x 8: %f s\n", 
    bx, by, bz, bt, nvar, (stopTime - startTime)/MAX_ITER);
  }

  MPI_Finalize();
}

