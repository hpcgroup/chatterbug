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
using namespace std;
#include <vector>

#if WRITE_OTF2_TRACE
#include <scorep/SCOREP_User.h>
#endif


int main(int argc, char **argv)
{
  int i, myrank, numranks;
  MPI_Init(&argc,&argv);
#if WRITE_OTF2_TRACE
  SCOREP_RECORDING_OFF();
#endif
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  MPI_Comm_size(MPI_COMM_WORLD,&numranks);

  if(argc != 6) {
    if(!myrank)
      printf("\nThis is the spread communication proxy. The correct usage is:\n"
             "%s minD maxD neighborhood msg_size MAX_ITER\n\n"
             "    minD, maxD: range of expected degree of each rank\n"
             "    neighborhood: how far in MPI rank space can we go for searching a neighbor\n"
             "    msg_size: size of message per pair (in bytes)\n"
             "    MAX_ITER: how many iters to run\n\n",
             argv[0]);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  int minD = atoi(argv[1]);
  int maxD = atoi(argv[2]);
  int neighborhood = atoi(argv[3]);
  int msg_size = atoi(argv[4]);
  int MAX_ITER = atoi(argv[5]);
  
  if(!myrank)
    printf("Create neighbors\n");

  // figure out number of destinations for each process
  srand(myrank);
  int range = maxD - minD;
  if(range == 0) range = 1;
  int my_degree = (minD + (rand() % range));

  // figure out destination processes for a process
  int *neighbors = new int[my_degree];
  for(int i = 0; i < my_degree; i++) {
    neighbors[i] = (myrank + (rand() % neighborhood)) % numranks;
  }

  int numNeighbors;
  int *counts = new int[numranks];
  int *new_counts = new int[numranks];
  for(int j = 0; j < numranks; j++) counts[j] = 0;
  for(int j = 0; j < my_degree; j++) {
    counts[neighbors[j]]++;
  }
  MPI_Allreduce(counts, new_counts, numranks, MPI_INTEGER, MPI_SUM, MPI_COMM_WORLD);
  numNeighbors = new_counts[myrank];
  delete [] counts;
  delete [] new_counts;

  double startTime, stopTime;
  char *sendbuf, *recvbuf;

  sendbuf = (char*) malloc (my_degree * msg_size * sizeof(char));
  recvbuf = (char*) malloc (numNeighbors * msg_size * sizeof(char));

  MPI_Request *sreq, *rreq;
  sreq = new MPI_Request[my_degree];
  rreq = new MPI_Request[numNeighbors];

  MPI_Barrier(MPI_COMM_WORLD);
#if WRITE_OTF2_TRACE
  SCOREP_RECORDING_ON();
  // Marks the beginning of code region to be repeated in simulation
  SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_Loop", SCOREP_USER_REGION_TYPE_COMMON);
  // Marks when to print a timer in simulation
  if(!myrank)
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_WallTime_spread", SCOREP_USER_REGION_TYPE_COMMON);
#endif

  startTime = MPI_Wtime();
  for (i = 0; i < MAX_ITER; i++) {
#if WRITE_OTF2_TRACE
    // Marks compute region before messaging
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_spread_pre_msg", SCOREP_USER_REGION_TYPE_COMMON);
    SCOREP_USER_REGION_BY_NAME_END("TRACER_spread_pre_msg");
#endif

    for(int j = 0; j < numNeighbors; j++) {
      MPI_Irecv(&recvbuf[j * msg_size], msg_size, MPI_CHAR, MPI_ANY_SOURCE, i,
        MPI_COMM_WORLD, &rreq[j]);
    }
    for(int j = 0; j < my_degree; j++) {
      MPI_Isend(&sendbuf[j * msg_size], msg_size, MPI_CHAR, neighbors[j], i,
        MPI_COMM_WORLD, &sreq[j]);
    }

#if WRITE_OTF2_TRACE
    // Marks compute region for computation-communication overlap
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_spread_overlap", SCOREP_USER_REGION_TYPE_COMMON);
    SCOREP_USER_REGION_BY_NAME_END("TRACER_spread_overlap");
#endif

    MPI_Waitall(my_degree, &sreq[0], MPI_STATUSES_IGNORE);
    MPI_Waitall(numNeighbors, &rreq[0], MPI_STATUSES_IGNORE);

#if WRITE_OTF2_TRACE
    // Marks compute region before messaging
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_spread_post_msg", SCOREP_USER_REGION_TYPE_COMMON);
    SCOREP_USER_REGION_BY_NAME_END("TRACER_spread_post_msg");
#endif
  }

#if WRITE_OTF2_TRACE
  // Marks the end of code region to be repeated in simulation
  SCOREP_USER_REGION_BY_NAME_END("TRACER_Loop");
#endif
  MPI_Barrier(MPI_COMM_WORLD);
  stopTime = MPI_Wtime();

#if WRITE_OTF2_TRACE
  if(!myrank)
    SCOREP_USER_REGION_BY_NAME_END("TRACER_WallTime_spread");
  SCOREP_RECORDING_OFF();
#endif

  if(myrank == 0 && MAX_ITER != 0) {
    printf("Finished %d iterations\n", MAX_ITER);
    printf("Time elapsed per iteration for size %d: %f s\n", msg_size, 
    (stopTime - startTime)/MAX_ITER);
  }

  MPI_Finalize();
}
