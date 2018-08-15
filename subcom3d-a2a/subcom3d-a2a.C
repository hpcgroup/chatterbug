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

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define MP_X 0
#define MP_Y 1
#define MP_Z 2

#if WRITE_OTF2_TRACE
#include <scorep/SCOREP_User.h>
#endif

#define calc_pe(a,b,c)  ((a)+(b)*dims[MP_X]+(c)*dims[MP_X]*dims[MP_Y])

int main(int argc, char **argv)
{
  int myrank, numranks;
  MPI_Init(&argc,&argv);
#if WRITE_OTF2_TRACE
  SCOREP_RECORDING_OFF();
#endif
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  MPI_Comm_size(MPI_COMM_WORLD,&numranks);

  if(argc != 8) {
    if(!myrank)
      printf("\nThis is the subcom3d-a2a communication proxy. The correct usage is:\n"
             "%s nx ny nz msg_size_x msg_size_y msg_size_z MAX_ITER\n\n"
             "    nx, ny, nz: layout of process grid in 3D\n"
             "    msg_size_x: size of per pair all to all messages along X dimension, 0 to skip (in bytes)\n"
             "    msg_size_y: size of per pair all to all messages along Y dimension, 0 to skip  (in bytes)\n"
             "    msg_size_z: size of per pair all to all messages along Z dimension, 0 to skip  (in bytes)\n"
             "    MAX_ITER: how many iters to run\n\n",
             argv[0]);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  int dims[3] = {0, 0, 0};
  dims[MP_X] = atoi(argv[1]); // nx
  dims[MP_Y] = atoi(argv[2]); // ny
  dims[MP_Z] = atoi(argv[3]); // nz
  
  int msg_size_x = atoi(argv[4]);
  int msg_size_y = atoi(argv[5]);
  int msg_size_z = atoi(argv[6]);
  int MAX_ITER = atoi(argv[7]);
  
  if(dims[MP_X] * dims[MP_Y] * dims[MP_Z] != numranks) {
    if(!myrank) {
      printf("\n nx * ny * nz does not equal number of ranks. \n");
    }
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  //figure out my coordinates
  int myXcoord = myrank % dims[MP_X];
  int myYcoord = (myrank % (dims[MP_X] * dims[MP_Y])) / dims[MP_X];
  int myZcoord = (myrank % (dims[MP_X] * dims[MP_Y] * dims[MP_Z])) / (dims[MP_X] * dims[MP_Y]);

  bool skip[3];

  //which a2as to skip
  skip[MP_X] = msg_size_x == 0;
  skip[MP_Y] = msg_size_y == 0;
  skip[MP_Z] = msg_size_z == 0;

  //all a2a share the buffer
  int largestMsg = (msg_size_x * dims[MP_X] > msg_size_y * dims[MP_Y]) ? msg_size_x * dims[MP_X] : msg_size_y * dims[MP_Y];
  largestMsg = (largestMsg > msg_size_z * dims[MP_Z]) ? largestMsg : msg_size_z * dims[MP_Z];

  char *sendbuf = (char*)malloc(largestMsg);
  char *recvbuf = (char*)malloc(largestMsg);

  //create subcommunicators
  MPI_Comm X_comm, Y_comm, Z_comm;
  if(!skip[MP_X]) {
    MPI_Comm_split(MPI_COMM_WORLD, myZcoord * dims[MP_Y] + myYcoord, myXcoord, &X_comm);
  }
  if(!skip[MP_Y]) {
    MPI_Comm_split(MPI_COMM_WORLD, myZcoord * dims[MP_X] + myXcoord, myYcoord, &Y_comm);
  }
  if(!skip[MP_Z]) {
    MPI_Comm_split(MPI_COMM_WORLD, myYcoord * dims[MP_X] + myXcoord, myZcoord, &Z_comm);
  }

  double startTime, stopTime;
  MPI_Barrier(MPI_COMM_WORLD);
#if WRITE_OTF2_TRACE
  SCOREP_RECORDING_ON();
  // Marks the beginning of code region to be repeated in simulation
  SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_Loop", SCOREP_USER_REGION_TYPE_COMMON);
  // Marks when to print a timer in simulation
  if(!myrank)
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_WallTime_a2a", SCOREP_USER_REGION_TYPE_COMMON);
#endif

  startTime = MPI_Wtime();
  for (int i = 0; i < MAX_ITER; i++) {
    if(!skip[MP_X]) {
#if WRITE_OTF2_TRACE
      // Marks compute region before messaging
      SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_a2a_pre_x", SCOREP_USER_REGION_TYPE_COMMON);
      SCOREP_USER_REGION_BY_NAME_END("TRACER_a2a_pre_x");
#endif
      MPI_Alltoall(sendbuf, msg_size_x, MPI_CHAR, recvbuf, msg_size_x, MPI_CHAR, X_comm);
#if WRITE_OTF2_TRACE
      // Marks compute region after messaging
      SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_a2a_post_x", SCOREP_USER_REGION_TYPE_COMMON);
      SCOREP_USER_REGION_BY_NAME_END("TRACER_a2a_post_x");
#endif
    }

    if(!skip[MP_Y]) {
#if WRITE_OTF2_TRACE
      // Marks compute region before messaging
      SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_a2a_pre_y", SCOREP_USER_REGION_TYPE_COMMON);
      SCOREP_USER_REGION_BY_NAME_END("TRACER_a2a_pre_y");
#endif
      MPI_Alltoall(sendbuf, msg_size_y, MPI_CHAR, recvbuf, msg_size_y, MPI_CHAR, Y_comm);
#if WRITE_OTF2_TRACE
      // Marks compute region after messaging
      SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_a2a_post_y", SCOREP_USER_REGION_TYPE_COMMON);
      SCOREP_USER_REGION_BY_NAME_END("TRACER_a2a_post_y");
#endif
    }

    if(!skip[MP_Z]) {
#if WRITE_OTF2_TRACE
      // Marks compute region before messaging
      SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_a2a_pre_z", SCOREP_USER_REGION_TYPE_COMMON);
      SCOREP_USER_REGION_BY_NAME_END("TRACER_a2a_pre_z");
#endif
      MPI_Alltoall(sendbuf, msg_size_z, MPI_CHAR, recvbuf, msg_size_z, MPI_CHAR, Z_comm);
#if WRITE_OTF2_TRACE
      // Marks compute region after messaging
      SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_a2a_post_z", SCOREP_USER_REGION_TYPE_COMMON);
      SCOREP_USER_REGION_BY_NAME_END("TRACER_a2a_post_z");
#endif
    }
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
    SCOREP_USER_REGION_BY_NAME_END("TRACER_WallTime_a2a");
  SCOREP_RECORDING_OFF();
#endif

  //finalized summary output
  if(myrank == 0 && MAX_ITER != 0) {
    printf("Finished %d iterations\n",MAX_ITER);
    printf("Time elapsed per iteration for grid size (%d,%d,%d) with message sizes (%d,%d,%d) : %f s\n", 
    dims[MP_X], dims[MP_Y], dims[MP_Z], msg_size_x, msg_size_y, msg_size_z, (stopTime - startTime)/MAX_ITER);
  }

  MPI_Finalize();
}


