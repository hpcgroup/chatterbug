#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define MP_X 0
#define MP_Y 1

#include "shared-alloc.h"
#include "blue.h"
#include "cktiming.h"     

void changeMessage(BgTimeLog *log, int size)                  
{                                                   
  log->msgs[0]->msgsize = size;            
}

extern "C" void BgMark(const char *str);

#define calc_pe(a,b)  ((a)+(b)*dims[MP_X])

void All2All(void *sendbuf, void *recvbuf, int size, int myrank, int *dims, int overlap) {
  int largerGroup = dims[MP_X];
  MPI_Request *sreq, *rreq;
  sreq = new MPI_Request[largerGroup];
  rreq = new MPI_Request[largerGroup];

  int myXcoord = myrank % dims[MP_X];
  int myYcoord = myrank / dims[MP_X];

  BgTimeLine &timeLine = tTIMELINEREC.timeline;  
  for(int xcoord = 1; xcoord < dims[MP_X]; xcoord++) {
    MPI_Isend(sendbuf, 1, MPI_CHAR,
        calc_pe((xcoord + myXcoord) % dims[MP_X], myYcoord), 0,
        MPI_COMM_WORLD, &sreq[xcoord]);
    changeMessage(timeLine[timeLine.length() - 3], size);
    MPI_Irecv(recvbuf, 1, MPI_CHAR,
        calc_pe((xcoord + myXcoord) % dims[MP_X], myYcoord), 0,
        MPI_COMM_WORLD, &rreq[xcoord]);
  }
  if(overlap) {
    BgMark("QBOX_OverlapWork");    
  }
  MPI_Waitall(dims[MP_X] - 1, &sreq[1], MPI_STATUSES_IGNORE);
  MPI_Waitall(dims[MP_X] - 1, &rreq[1], MPI_STATUSES_IGNORE);
  if(!overlap) {
    BgMark("QBOX_FFTTime");    
  }
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

  BgTimeLine &timeLine = tTIMELINEREC.timeline;  
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
    changeMessage(timeLine[timeLine.length() - 3], size);
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

  BgTimeLine &timeLine = tTIMELINEREC.timeline;  
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
    changeMessage(timeLine[timeLine.length() - 3], size);
  }
  if(myChild2 < commSize) {
    partCoord[alongDim] = myChild2;
    MPI_Isend(sendbuf, 1, MPI_CHAR, calc_pe(partCoord[MP_X], partCoord[MP_Y]), 0,
      MPI_COMM_WORLD, &requests[count++]);
    changeMessage(timeLine[timeLine.length() - 3], size);
  }
  MPI_Waitall(count, &requests[0], MPI_STATUSES_IGNORE);
}

int main(int argc, char **argv)
{
  int i, myrank, numranks, off;
  MPI_Init(&argc,&argv);
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Set_trace_status(0);
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  MPI_Comm_size(MPI_COMM_WORLD,&numranks);

  if(!myrank && argc != 7) {
    printf("Correct usage: ./%s dimX dimY fft_pair_size bcast_size all_reduce_size num_iter\n", argv[0]);
    MPI_Abort(MPI_COMM_WORLD, 0);
  }

  int dims[2] = {0, 0};

  dims[MP_X] = atoi(argv[1]);
  dims[MP_Y] = atoi(argv[2]);
  int fft_size = atoi(argv[3]);
  int bcast_size = atoi(argv[4]);
  int allreduce_size = atoi(argv[5]);
  int MAX_ITER = atoi(argv[6]);

  char *sendbuf = (char*)shalloc(1,  1);
  char *recvbuf = (char*)shalloc(1, 1);
  
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Set_trace_status(1);
  AMPI_Set_startevent(MPI_COMM_WORLD);

  if(!myrank)
    BgPrintf("Current time is %f\n");

  for (i = 0; i < MAX_ITER; i++) {
    BgMark("QBOX_Setup");    
    All2All(sendbuf, recvbuf, fft_size, myrank, dims, 0);
    Reduce(sendbuf, recvbuf, allreduce_size, myrank, dims, MP_X);
    Bcast(sendbuf, allreduce_size, myrank, dims, MP_X);
    BgMark("QBOX_Work1");    
    All2All(sendbuf, recvbuf, fft_size, myrank, dims, 0);
    BgMark("QBOX_Work2");    
    MPI_Loop_to_start();
  }

  AMPI_Set_endevent();
  MPI_Barrier(MPI_COMM_WORLD);
#if CMK_BIGSIM_CHARM
  if(!myrank)
    BgPrintf("After loop Current time is %f\n");
#endif
  MPI_Set_trace_status(0);

  MPI_Finalize();
}








