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
#define RAND_SEED 1331

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

  if(numranks % 2) {
    if(!myrank) {
      printf("\n This code can only be run with an even number of processes. \n");
    }
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  if(argc != 3 && argc != 4) {
    if(!myrank)
      printf("\nThis is the permutation communication proxy. The correct usage is:\n"
             "%s msg_size MAX_ITER <randomize_pairs> \n\n"
             "    msg_size: size of message per pair (in bytes)\n"
             "    MAX_ITER: how many iters to run\n"
             "    (optional) randomize_pairs: 0/1 \n\n",
             argv[0]);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }
  
  int msg_size = atoi(argv[1]);
  int MAX_ITER = atoi(argv[2]);
  int randomize_pairs = 0;
  if(argc > 3) {
    randomize_pairs = atoi(argv[3]);
  }

  double startTime, stopTime;
  MPI_Status status;
  MPI_Request requests[2];
  char *sendbuf, *recvbuf;
  int partner;

  sendbuf = (char*)malloc(msg_size * sizeof(char));
  recvbuf = (char*)malloc(msg_size * sizeof(char));
  
  if(!myrank) {
    printf("Creating rank pairs\n");
  }

  if(randomize_pairs) {
    int *partners = (int*)malloc(numranks/2 * sizeof(int));
    if(myrank == 0) {
      //match every rank with diagonal opposite
      for(int i = 0; i < numranks/2; i++) {
        partners[i] = numranks/2 + i;
      }

      //randomize within the first half
      srand(RAND_SEED);
      for(int i = 0; i < 2*numranks; i++) {
        int index1 = rand() % (numranks/2);
        int index2 = rand() % (numranks/2);
        int temp = partners[index1];
        partners[index1] = partners[index2];
        partners[index2] = temp;
      }
    }
    MPI_Bcast(partners, numranks/2, MPI_INT, 0, MPI_COMM_WORLD);

    //senders tell the receivers who they are
    if(myrank < numranks/2) {
      partner = partners[myrank];
      MPI_Send(&myrank, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
    } else {
      MPI_Recv(&partner, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    free(partners);
  } else {
    if(myrank >= numranks/2) {
      partner = myrank - numranks/2;
    } else {
      partner = myrank + numranks/2;
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);
#if WRITE_OTF2_TRACE
  SCOREP_RECORDING_ON();
  // Marks the beginning of code region to be repeated in simulation
  SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_Loop", SCOREP_USER_REGION_TYPE_COMMON);
  // Marks when to print a timer in simulation
  if(!myrank)
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_WallTime_pairs", SCOREP_USER_REGION_TYPE_COMMON);
#endif

  startTime = MPI_Wtime();
  for(int i = 0; i < MAX_ITER; i++) {
#if WRITE_OTF2_TRACE
    // Marks compute region before messaging
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_pairs_pre_msg", SCOREP_USER_REGION_TYPE_COMMON);
    SCOREP_USER_REGION_BY_NAME_END("TRACER_pairs_pre_msg");
#endif

    MPI_Irecv(recvbuf, msg_size, MPI_CHAR, partner, 0, MPI_COMM_WORLD, &requests[0]);
    MPI_Isend(sendbuf, msg_size, MPI_CHAR, partner, 0, MPI_COMM_WORLD, &requests[1]);

#if WRITE_OTF2_TRACE
    // Marks compute region for computation-communication overlap
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_pairs_overlap", SCOREP_USER_REGION_TYPE_COMMON);
    SCOREP_USER_REGION_BY_NAME_END("TRACER_pairs_overlap");
#endif

    MPI_Waitall(2, requests, MPI_STATUSES_IGNORE);

#if WRITE_OTF2_TRACE
    // Marks compute region after messaging
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_pairs_post_msg", SCOREP_USER_REGION_TYPE_COMMON);
    SCOREP_USER_REGION_BY_NAME_END("TRACER_pairs_post_msg");
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
    SCOREP_USER_REGION_BY_NAME_END("TRACER_WallTime_pairs");
  SCOREP_RECORDING_OFF();
#endif
  
  if(myrank == 0 && MAX_ITER != 0) {
    printf("Finished %d iterations\n", MAX_ITER);
    printf("Time elapsed per iteration for size %d: %f s\n", msg_size, 
    (stopTime - startTime)/MAX_ITER);
  }

  MPI_Finalize();
}
