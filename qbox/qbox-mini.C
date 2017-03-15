#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define MP_X 0
#define MP_Y 1

#if WRITE_OTF2_TRACE
#include <scorep/SCOREP_User.h>
#elif CMK_BIGSIM_CHARM
#include "shared-alloc.h"
#include "blue.h"
#include "cktiming.h"     

void changeMessage(BgTimeLog *log, int size)                  
{                                                   
  log->msgs[0]->msgsize = size;            
}

extern "C" void BgMark(const char *str);
#endif

#define calc_pe(a,b)  ((a)+(b)*dims[MP_X])

void All2All(void *sendbuf, void *recvbuf, int size, int myrank, int *dims, int overlap) {
  int largerGroup = dims[MP_X];
  MPI_Request *sreq, *rreq;
  sreq = new MPI_Request[largerGroup];
  rreq = new MPI_Request[largerGroup];

  int myXcoord = myrank % dims[MP_X];
  int myYcoord = myrank / dims[MP_X];

#if CMK_BIGSIM_CHARM
  BgTimeLine &timeLine = tTIMELINEREC.timeline;
#endif
  for(int xcoord = 1; xcoord < dims[MP_X]; xcoord++) {
    MPI_Isend(sendbuf, 1, MPI_CHAR,
        calc_pe((xcoord + myXcoord) % dims[MP_X], myYcoord), 0,
        MPI_COMM_WORLD, &sreq[xcoord]);
#if CMK_BIGSIM_CHARM
    changeMessage(timeLine[timeLine.length() - 3], size);
#endif
    MPI_Irecv(recvbuf, 1, MPI_CHAR,
        calc_pe((xcoord + myXcoord) % dims[MP_X], myYcoord), 0,
        MPI_COMM_WORLD, &rreq[xcoord]);
  }
#if CMK_BIGSIM_CHARM
  if(overlap) {
    BgMark("QBOX_OverlapWork");    
  }
#endif
  MPI_Waitall(dims[MP_X] - 1, &sreq[1], MPI_STATUSES_IGNORE);
  MPI_Waitall(dims[MP_X] - 1, &rreq[1], MPI_STATUSES_IGNORE);
#if CMK_BIGSIM_CHARM
  if(!overlap) {
    BgMark("QBOX_FFTTime");    
  }
#endif
}

void Reduce(void *sendbuf, void *recvbuf, int size, int myrank, int *dims, int alongDim) {
  int otherDim = 1 - alongDim;
  int commSize = dims[alongDim];
  int mycoord[2];
  mycoord[MP_X] = myrank % dims[MP_X];
  mycoord[MP_Y] = myrank / dims[MP_X];
  int myRank = mycoord[alongDim];

  int myParent = (myRank - 1) / 2;
  int myChild1 = 2*myRank + 1;
  int myChild2 = myChild1 + 1;

#if CMK_BIGSIM_CHARM
  BgTimeLine &timeLine = tTIMELINEREC.timeline;
#endif
  int partCoord[2];
  partCoord[otherDim] = mycoord[otherDim];
  if(myChild1 < commSize) {
    partCoord[alongDim] = myChild1;
    MPI_Recv(recvbuf, 1, MPI_CHAR, calc_pe(partCoord[MP_X], partCoord[MP_Y]), 0,
      MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
  if(myChild2 < commSize) {
    partCoord[alongDim] = myChild2;
    MPI_Recv(recvbuf, 1, MPI_CHAR, calc_pe(partCoord[MP_X], partCoord[MP_Y]), 0,
      MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
  if(myRank != 0) {
    partCoord[alongDim] = myParent;
    MPI_Send(sendbuf, 1, MPI_CHAR,  calc_pe(partCoord[MP_X], partCoord[MP_Y]), 0, MPI_COMM_WORLD);
#if CMK_BIGSIM_CHARM
    changeMessage(timeLine[timeLine.length() - 3], size);
#endif
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
  int myChild1 = 2*myRank + 1;
  int myChild2 = myChild1 + 1;

#if CMK_BIGSIM_CHARM
  BgTimeLine &timeLine = tTIMELINEREC.timeline;
#endif
  int partCoord[2];
  partCoord[otherDim] = mycoord[otherDim];
  if(myRank != 0) {
    partCoord[alongDim] = myParent;
    MPI_Recv(sendbuf, 1, MPI_CHAR,  calc_pe(partCoord[MP_X], partCoord[MP_Y]), 0, 
      MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
  MPI_Request requests[2];
  int count = 0;
  if(myChild1 < commSize) {
    partCoord[alongDim] = myChild1;
    MPI_Isend(sendbuf, 1, MPI_CHAR, calc_pe(partCoord[MP_X], partCoord[MP_Y]), 0,
      MPI_COMM_WORLD, &requests[count++]);
#if CMK_BIGSIM_CHARM
    changeMessage(timeLine[timeLine.length() - 3], size);
#endif
  }
  if(myChild2 < commSize) {
    partCoord[alongDim] = myChild2;
    MPI_Isend(sendbuf, 1, MPI_CHAR, calc_pe(partCoord[MP_X], partCoord[MP_Y]), 0,
      MPI_COMM_WORLD, &requests[count++]);
#if CMK_BIGSIM_CHARM
    changeMessage(timeLine[timeLine.length() - 3], size);
#endif
  }
  MPI_Waitall(count, &requests[0], MPI_STATUSES_IGNORE);
}

int main(int argc, char **argv)
{
  int i, myrank, numranks, off;
  MPI_Init(&argc,&argv);
#if WRITE_OTF2_TRACE
  SCOREP_RECORDING_OFF();
#elif CMK_BIGSIM_CHARM
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Set_trace_status(0);
  MPI_Barrier(MPI_COMM_WORLD);
#endif
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  MPI_Comm_size(MPI_COMM_WORLD,&numranks);

  if(!myrank && argc != 6) {
    printf("Correct usage: ./%s dimX fft_pair_size bcast_size all_reduce_size num_iter\n", argv[0]);
    MPI_Abort(MPI_COMM_WORLD, 0);
  }

  int dims[2] = {0, 0};

  dims[MP_X] = atoi(argv[1]);
  dims[MP_Y] = numranks / dims[MP_X];
  int fft_size = atoi(argv[2]);
  int bcast_size = atoi(argv[3]);
  int allreduce_size = atoi(argv[4]);
  int MAX_ITER = atoi(argv[5]);

  int myXcoord = myrank % dims[MP_X];	// column number
  int myYcoord = myrank / dims[MP_X];	// row number

  MPI_Comm X_comm, Y_comm;
  MPI_Comm_split(MPI_COMM_WORLD, myYcoord, myXcoord, &X_comm);
  MPI_Comm_split(MPI_COMM_WORLD, myXcoord, myYcoord, &Y_comm);

  double startTime, stopTime;
  char *sendbuf, *recvbuf;
  int *rdispls=NULL, *recvcounts=NULL, *sdispls=NULL, *sendcounts=NULL;
  int max_msg_size = fft_size * dims[MP_X];

#if CMK_BIGSIM_CHARM
  sendbuf = (char*) shalloc(1, 1);
  recvbuf = (char*) shalloc(1, 1);
#else
  sendbuf = (char*) malloc (max_msg_size * sizeof(char));
  recvbuf = (char*) malloc (max_msg_size * sizeof(char));
  sendcounts = (int *) malloc (dims[MP_X] *sizeof(int));
  recvcounts = (int *) malloc (dims[MP_X] *sizeof(int));
  sdispls = (int *) malloc (dims[MP_X] *sizeof(int));
  rdispls = (int *) malloc (dims[MP_X] *sizeof(int));

  int disp = 0;
  for ( i = 0; i < dims[MP_X]; i++) {
    recvcounts[i] = fft_size;
    sendcounts[i] = fft_size;
    rdispls[i] = disp;
    sdispls[i] = disp;
    disp += fft_size;
  }
#endif

  MPI_Barrier(MPI_COMM_WORLD);
#if WRITE_OTF2_TRACE
  SCOREP_RECORDING_ON();
  SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_Loop", SCOREP_USER_REGION_TYPE_COMMON);
  if(!myrank)
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_WallTime_Loop", SCOREP_USER_REGION_TYPE_COMMON);
#elif CMK_BIGSIM_CHARM
  MPI_Set_trace_status(1);
  AMPI_Set_startevent(MPI_COMM_WORLD);
  BgTimeLine &timeLine = tTIMELINEREC.timeline;
  if(!myrank)
    BgPrintf("Current time is %f\n");
#endif
  startTime = MPI_Wtime();

  for (i = 0; i < MAX_ITER; i++) {
#if CMK_BIGSIM_CHARM
    BgMark("QBOX_Setup");    
    All2All(sendbuf, recvbuf, fft_size, myrank, dims, 0);
    Reduce(sendbuf, recvbuf, allreduce_size, myrank, dims, MP_X);
    Bcast(sendbuf, allreduce_size, myrank, dims, MP_X);
    BgMark("QBOX_Work1");    
    All2All(sendbuf, recvbuf, fft_size, myrank, dims, 0);
    BgMark("QBOX_Work2");    
    MPI_Loop_to_start();
#else
    MPI_Alltoallv(sendbuf, sendcounts, sdispls, MPI_CHAR, recvbuf, recvcounts, rdispls, MPI_CHAR, X_comm);
    MPI_Allreduce(sendbuf, recvbuf, allreduce_size, MPI_CHAR, MPI_LAND, Y_comm);
    MPI_Bcast(sendbuf, bcast_size, MPI_CHAR, 0, Y_comm);
#endif
  }

#if WRITE_OTF2_TRACE
  SCOREP_USER_REGION_BY_NAME_END("TRACER_Loop");
#elif CMK_BIGSIM_CHARM
  AMPI_Set_endevent();
#endif

  MPI_Barrier(MPI_COMM_WORLD);
  stopTime = MPI_Wtime();

  if(myrank == 0 && MAX_ITER != 0) {
    printf("Finished %d iterations\n", MAX_ITER);
    printf("Time elapsed per iteration: %f\n", (stopTime - startTime)/MAX_ITER);
  }

#if WRITE_OTF2_TRACE
  if(!myrank)
    SCOREP_USER_REGION_BY_NAME_END("TRACER_WallTime_Loop");
  SCOREP_RECORDING_OFF();
#elif CMK_BIGSIM_CHARM
  if(!myrank)
    BgPrintf("After loop Current time is %f\n");
  MPI_Set_trace_status(0);
  MPI_Barrier(MPI_COMM_WORLD);
#endif
  MPI_Finalize();
}

