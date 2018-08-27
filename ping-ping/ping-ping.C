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

#if WRITE_OTF2_TRACE
#include <scorep/SCOREP_User.h>
#endif

int main(int argc, char **argv)
{
  int myrank, numranks;
  MPI_Init(&argc,&argv);
#if WRITE_OTF2_TRACE
  SCOREP_RECORDING_OFF();
#endif
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  MPI_Comm_size(MPI_COMM_WORLD,&numranks);

  if(argc < 5) {
    if(!myrank) {
      printf("\nThis is the multi-pair ping-ping communication proxy. The correct usage is:\n"
             "%s src_0 dst_0 <src_i dst_i>* msg_size MAX_ITER\n\n"
             "    src_0 dst_0: first communicating pair\n"
             "    (optional) src_i dst_i: more communicating pairs as desired\n"
             "    msg_size: size of message per pair (in bytes)\n"
             "    MAX_ITER: how many iters to run\n\n",
             argv[0]);
    }
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  int paired = -1;
  int numPairs = (argc - 3)/2;
  int curP = 1;
  // figure out communicating pairs 
  for(int p = 0; p < numPairs; p++) {
    if(atoi(argv[curP]) == myrank) {
      paired = atoi(argv[curP + 1]);
    }
    if(atoi(argv[curP+1]) == myrank) {
      paired = atoi(argv[curP]);
    }
    curP += 2;
  }
  int msg_size = atoi(argv[curP]);
  int MAX_ITER = atoi(argv[curP+1]);

  char *sendbuf, *recvbuf;
  MPI_Request requests[2];

  if(paired != -1) {
    sendbuf = (char*)malloc(msg_size * sizeof(char));
    recvbuf = (char*)malloc(msg_size * sizeof(char));
  }
  double startTime, stopTime;

  MPI_Barrier(MPI_COMM_WORLD);
#if WRITE_OTF2_TRACE
  SCOREP_RECORDING_ON();
  // Marks the beginning of code region to be repeated in simulation
  SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_Loop", SCOREP_USER_REGION_TYPE_COMMON);
  // Marks when to print a timer in simulation
  if(!myrank)
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_WallTime_pingping", SCOREP_USER_REGION_TYPE_COMMON);
#endif

  startTime = MPI_Wtime();
  for(int i = 0; i < MAX_ITER; i++) {
#if WRITE_OTF2_TRACE
    // Marks compute region before messaging
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_pingping_pre_msg", SCOREP_USER_REGION_TYPE_COMMON);
    SCOREP_USER_REGION_BY_NAME_END("TRACER_pingping_pre_msg");
#endif

    if(paired != -1) {
      MPI_Isend(sendbuf, msg_size, MPI_CHAR, paired, 0, MPI_COMM_WORLD, &requests[0]);
      MPI_Irecv(recvbuf, msg_size, MPI_CHAR, paired, 0, MPI_COMM_WORLD, &requests[1]);
    }

#if WRITE_OTF2_TRACE
    // Marks compute region for computation-communication overlap
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_pingping_overlap", SCOREP_USER_REGION_TYPE_COMMON);
    SCOREP_USER_REGION_BY_NAME_END("TRACER_pingping_overlap");
#endif

    if(paired != -1) {
      MPI_Waitall(2, requests, MPI_STATUSES_IGNORE);
    }

#if WRITE_OTF2_TRACE
    // Marks compute region after messaging
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_pingping_post_msg", SCOREP_USER_REGION_TYPE_COMMON);
    SCOREP_USER_REGION_BY_NAME_END("TRACER_pingping_post_msg");
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
    SCOREP_USER_REGION_BY_NAME_END("TRACER_WallTime_pingping");
  SCOREP_RECORDING_OFF();
#endif
  
  if(myrank == 0 && MAX_ITER != 0) {
    printf("Finished %d iterations\n", MAX_ITER);
    printf("Time elapsed per iteration for size %d: %f s\n", msg_size, 
    (stopTime - startTime)/MAX_ITER);
  }

  MPI_Finalize();
}
