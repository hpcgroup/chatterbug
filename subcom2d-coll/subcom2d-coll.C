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
#include <assert.h>

#define MP_X 0
#define MP_Y 1

#if WRITE_OTF2_TRACE
#include <scorep/SCOREP_User.h>
#endif

#define calc_pe(a,b)  ((a)+(b)*dims[MP_X])

//prototype for explict calls
void All2All(void *sendbuf, void *recvbuf, int size, int myrank, int *dims,
    int overlap);
void Reduce(void *sendbuf, void *recvbuf, int size, int myrank, int *dims,
    int alongDim);
void Bcast(void *sendbuf, int size, int myrank, int *dims, int alongDim);

int main(int argc, char **argv)
{
  int i, myrank, numranks, off;
  MPI_Init(&argc,&argv);
#if WRITE_OTF2_TRACE
  SCOREP_RECORDING_OFF();
#endif
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  MPI_Comm_size(MPI_COMM_WORLD,&numranks);

  if(argc != 7 || argc != 8) {
    if(!myrank)
      printf("\nThis is the subcom2d-coll communication proxy. The correct usage is:\n"
             "%s nx ny a2a_msg_size_x allreduce_msg_size_y bcast_msg_size_y MAX_ITER <use_explicit_calls>\n\n"
             "    nx, ny: layout of process grid in 2D\n"
             "    a2a_msg_size_x: size of per pair all to all messages along X dimension  (in bytes)\n"
             "    allreduce_msg_size_y: size of allreduce along Y dimension (in bytes)\n"
             "    bcast_msg_size_y: size of broadcast along Y dimension (in bytes)\n"
             "    MAX_ITER: how many iters to run\n"
             "    (optional) use_explicit_calls : choose 1 to make pt-to-pt calls for collectives\n\n",
             argv[0]);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  int dims[2] = {0, 0};

  dims[MP_X] = atoi(argv[1]);
  dims[MP_Y] = atoi(argv[2]);
  assert(numranks == (dims[MP_X] * dims[MP_Y]));
  int a2a_msg_size_x = atoi(argv[3]);
  int allreduce_msg_size_y = atoi(argv[4]);
  int bcast_msg_size_y = atoi(argv[5]);
  int MAX_ITER = atoi(argv[6]);
  int use_explicit_calls = 0;
  if(argc == 8) {
    use_explicit_calls = 1;
  }

  int myXcoord = myrank % dims[MP_X];	// column number
  int myYcoord = myrank / dims[MP_X];	// row number

  MPI_Comm X_comm, Y_comm;
  MPI_Comm_split(MPI_COMM_WORLD, myYcoord, myXcoord, &X_comm);
  MPI_Comm_split(MPI_COMM_WORLD, myXcoord, myYcoord, &Y_comm);

  double startTime, stopTime;
  char *sendbuf, *recvbuf;
  int *rdispls=NULL, *recvcounts=NULL, *sdispls=NULL, *sendcounts=NULL;
  int max_msg_size = (allreduce_msg_size_y > bcast_msg_size_y) ? allreduce_msg_size_y : bcast_msg_size_y;
  max_msg_size = (max_msg_size > a2a_msg_size_x * dims[MP_X]) ? max_msg_size : a2a_msg_size_x * dims[MP_X];

  sendbuf = (char*) malloc (max_msg_size * sizeof(char));
  recvbuf = (char*) malloc (max_msg_size * sizeof(char));
  sendcounts = (int *) malloc (dims[MP_X] * sizeof(int));
  recvcounts = (int *) malloc (dims[MP_X] * sizeof(int));
  sdispls = (int *) malloc (dims[MP_X] * sizeof(int));
  rdispls = (int *) malloc (dims[MP_X] * sizeof(int));

  int disp = 0;
  for ( i = 0; i < dims[MP_X]; i++) {
    recvcounts[i] = a2a_msg_size_x;
    sendcounts[i] = a2a_msg_size_x;
    rdispls[i] = disp;
    sdispls[i] = disp;
    disp += a2a_msg_size_x;
  }

  MPI_Barrier(MPI_COMM_WORLD);
#if WRITE_OTF2_TRACE
  SCOREP_RECORDING_ON();
  // Marks the beginning of code region to be repeated in simulation
  SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_Loop", SCOREP_USER_REGION_TYPE_COMMON);
  // Marks when to print a timer in simulation
  if(!myrank)
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_WallTime_coll", SCOREP_USER_REGION_TYPE_COMMON);
#endif

  startTime = MPI_Wtime();
  for (i = 0; i < MAX_ITER; i++) {
#if WRITE_OTF2_TRACE
    // Marks compute region before messaging
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_coll_pre_a2a", SCOREP_USER_REGION_TYPE_COMMON);
    SCOREP_USER_REGION_BY_NAME_END("TRACER_coll_pre_a2a");
#endif

    if(use_explicit_calls) {
      All2All(sendbuf, recvbuf, a2a_msg_size_x, myrank, dims, 0);
    } else {
      MPI_Alltoallv(sendbuf, sendcounts, sdispls, MPI_CHAR, recvbuf, recvcounts,
        rdispls, MPI_CHAR, X_comm);
    }

#if WRITE_OTF2_TRACE
    // Marks compute region before messaging
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_coll_pre_ar", SCOREP_USER_REGION_TYPE_COMMON);
    SCOREP_USER_REGION_BY_NAME_END("TRACER_coll_pre_ar");
#endif

    if(use_explicit_calls) {
      Reduce(sendbuf, recvbuf, allreduce_msg_size_y, myrank, dims, MP_X);
      Bcast(sendbuf, allreduce_msg_size_y, myrank, dims, MP_X);
    } else {
      MPI_Allreduce(sendbuf, recvbuf, allreduce_msg_size_y, MPI_CHAR, MPI_LAND,
        Y_comm);
    }

#if WRITE_OTF2_TRACE
    // Marks compute region before messaging
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_coll_pre_bcast", SCOREP_USER_REGION_TYPE_COMMON);
    SCOREP_USER_REGION_BY_NAME_END("TRACER_coll_pre_bcast");
#endif

    if(use_explicit_calls) {
      Bcast(sendbuf, bcast_msg_size_y, myrank, dims, MP_X);
    } else {
      MPI_Bcast(sendbuf, bcast_msg_size_y, MPI_CHAR, 0, Y_comm);
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
    SCOREP_USER_REGION_BY_NAME_END("TRACER_WallTime_coll");
  SCOREP_RECORDING_OFF();
#endif

  if(myrank == 0 && MAX_ITER != 0) {
    printf("Finished %d iterations\n", MAX_ITER);
    printf("Time elapsed per iteration for sizes (%d,%d,%d): %f s\n",
    a2a_msg_size_x, allreduce_msg_size_y, bcast_msg_size_y,
    (stopTime - startTime)/MAX_ITER);
  }

  MPI_Finalize();
}

void All2All(void *sendbuf, void *recvbuf, int size, int myrank, int *dims, int overlap) {
  int largerGroup = dims[MP_X];
  MPI_Request *sreq, *rreq;
  sreq = new MPI_Request[largerGroup];
  rreq = new MPI_Request[largerGroup];

  int myXcoord = myrank % dims[MP_X];
  int myYcoord = myrank / dims[MP_X];

  for(int xcoord = 1; xcoord < dims[MP_X]; xcoord++) {
    MPI_Isend(sendbuf, size, MPI_CHAR,
        calc_pe((xcoord + myXcoord) % dims[MP_X], myYcoord), 0,
        MPI_COMM_WORLD, &sreq[xcoord]);
    MPI_Irecv(recvbuf, size, MPI_CHAR,
        calc_pe((xcoord + myXcoord) % dims[MP_X], myYcoord), 0,
        MPI_COMM_WORLD, &rreq[xcoord]);
  }
  MPI_Waitall(dims[MP_X] - 1, &sreq[1], MPI_STATUSES_IGNORE);
  MPI_Waitall(dims[MP_X] - 1, &rreq[1], MPI_STATUSES_IGNORE);
}

void Reduce(void *sendbuf, void *recvbuf, int size, int myrank, int *dims, int alongDim) {
  int otherDim = 1 - alongDim;
  int commSize = dims[alongDim];
  int mycoord[2];
  mycoord[MP_X] = myrank % dims[MP_X];
  mycoord[MP_Y] = myrank / dims[MP_X];
  int myRank = mycoord[alongDim];

  int myParent = (myRank - 1) / 2;
  int myChild1 = 2 * myRank + 1;
  int myChild2 = myChild1 + 1;

  MPI_Request requests[2];
  int partCoord[2], count = 0;
  partCoord[otherDim] = mycoord[otherDim];
  if(myChild1 < commSize) {
    partCoord[alongDim] = myChild1;
    MPI_Irecv(recvbuf, size, MPI_CHAR, calc_pe(partCoord[MP_X], partCoord[MP_Y]), 0,
      MPI_COMM_WORLD, &requests[count++]);
  }
  if(myChild2 < commSize) {
    partCoord[alongDim] = myChild2;
    MPI_Irecv(recvbuf, size, MPI_CHAR, calc_pe(partCoord[MP_X], partCoord[MP_Y]), 0,
      MPI_COMM_WORLD, &requests[count++]);
  }
  MPI_Waitall(count, &requests[0], MPI_STATUSES_IGNORE);
  if(myRank != 0) {
    partCoord[alongDim] = myParent;
    MPI_Send(sendbuf, size, MPI_CHAR,  calc_pe(partCoord[MP_X], partCoord[MP_Y]), 0, MPI_COMM_WORLD);
  }
}

void Bcast(void *sendbuf, int size, int myrank, int *dims, int alongDim) {
  int otherDim = 1 - alongDim;
  int commSize = dims[alongDim];
  int mycoord[2];
  mycoord[MP_X] = myrank % dims[MP_X];
  mycoord[MP_Y] = myrank / dims[MP_X];
  int myRank = mycoord[alongDim];

  int myParent = (myRank - 1) / 2;
  int myChild1 = 2 * myRank + 1;
  int myChild2 = myChild1 + 1;

  int partCoord[2];
  partCoord[otherDim] = mycoord[otherDim];
  if(myRank != 0) {
    partCoord[alongDim] = myParent;
    MPI_Recv(sendbuf, size, MPI_CHAR,  calc_pe(partCoord[MP_X], partCoord[MP_Y]), 0,
      MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }

  MPI_Request requests[2];
  int count = 0;
  if(myChild1 < commSize) {
    partCoord[alongDim] = myChild1;
    MPI_Isend(sendbuf, size, MPI_CHAR, calc_pe(partCoord[MP_X], partCoord[MP_Y]), 0,
      MPI_COMM_WORLD, &requests[count++]);
  }
  if(myChild2 < commSize) {
    partCoord[alongDim] = myChild2;
    MPI_Isend(sendbuf, size, MPI_CHAR, calc_pe(partCoord[MP_X], partCoord[MP_Y]), 0,
      MPI_COMM_WORLD, &requests[count++]);
  }
  MPI_Waitall(count, &requests[0], MPI_STATUSES_IGNORE);
}
