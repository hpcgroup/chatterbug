#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define MP_X 0
#define MP_Y 1
#define MP_Z 2

#if WRITE_OTF2_TRACE
#include <scorep/SCOREP_User.h>
#elif CMK_BIGSIM_CHARM
#include "shared-alloc.h"
#include "blue.h"
#include "cktiming.h"     

void changeMessage(BgTimeLog *log)                  
{                                                   
  log->msgs[0]->msgsize = 5242880;            
}
#endif

#define calc_pe(a,b,c)  ((a)+(b)*dims[MP_X]+(c)*dims[MP_X]*dims[MP_Y])

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
    printf("Correct usage: ./%s dimX dimY bytes_X bytes_Y num_iter\n", argv[0]);
    MPI_Abort(MPI_COMM_WORLD, 0);
  }

  int dims[3] = {0, 0, 0};
  double startTime, stopTime;

  MPI_Request *sreq, *rreq;

  dims[MP_X] = atoi(argv[1]);
  dims[MP_Y] = atoi(argv[2]);
  dims[MP_Z] = numranks / (dims[MP_X] * dims[MP_Y]);
  int perrank_x = atoi(argv[3]);
  int perrank_y = atoi(argv[4]);
  int MAX_ITER = atoi(argv[5]);

  int largerGroup = (dims[MP_X] > dims[MP_Y]) ? dims[MP_X] : dims[MP_Y];
  int largerMsg = (perrank_x*dims[MP_X] > perrank_y*dims[MP_Y]) ? perrank_x*dims[MP_X] : perrank_y*dims[MP_Y];

#if CMK_BIGSIM_CHARM
  char *sendbuf = (char*)shalloc(largerMsg, 1);
  char *recvbuf = (char*)shalloc(largerMsg, 1);
#else
  char *sendbuf = (char*)malloc(largerMsg);
  char *recvbuf = (char*)malloc(largerMsg);
#endif

  sreq = new MPI_Request[largerGroup];
  rreq = new MPI_Request[largerGroup];

  int myXcoord = myrank % dims[MP_X];
  int myYcoord = (myrank % (dims[MP_X] * dims[MP_Y])) / dims[MP_X];
  int myZcoord = (myrank % (dims[MP_X] * dims[MP_Y] * dims[MP_Z])) / (dims[MP_X] * dims[MP_Y]);

  MPI_Comm X_comm, Y_comm;
  MPI_Comm_split(MPI_COMM_WORLD, myZcoord*dims[MP_Y] + myYcoord, myXcoord, &X_comm);
  MPI_Comm_split(MPI_COMM_WORLD, myZcoord*dims[MP_X] + myXcoord, myYcoord, &Y_comm);

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
    off = 0;
    for(int ycoord = 1; ycoord < dims[MP_Y]; ycoord++) {
      MPI_Isend(&sendbuf[off], perrank_y, MPI_CHAR,
        calc_pe(myXcoord, (ycoord + myYcoord) % dims[MP_Y], myZcoord), 0,
        MPI_COMM_WORLD, &sreq[ycoord]);
      changeMessage(timeLine[timeLine.length() - 3]);
      MPI_Irecv(&recvbuf[off], perrank_y, MPI_CHAR,
        calc_pe(myXcoord, (ycoord + myYcoord) % dims[MP_Y], myZcoord), 0,
        MPI_COMM_WORLD, &rreq[ycoord]);
      off += perrank;
    }
    BgAdvance(100);    
    MPI_Waitall(dims[MP_Y] - 1, &sreq[1], MPI_STATUSES_IGNORE);
    MPI_Waitall(dims[MP_Y] - 1, &rreq[1], MPI_STATUSES_IGNORE);
    off = 0;
    for(int xcoord = 1; xcoord < dims[MP_X]; xcoord++) {
      MPI_Isend(&sendbuf[off], perrank_x, MPI_CHAR,
        calc_pe((xcoord + myXcoord) % dims[MP_X], myYcoord, myZcoord), 0,
        MPI_COMM_WORLD, &sreq[xcoord]);
      changeMessage(timeLine[timeLine.length() - 3]);
      MPI_Irecv(&recvbuf[off], perrank_x, MPI_CHAR,
        calc_pe((xcoord + myXcoord) % dims[MP_X], myYcoord, myZcoord), 0,
        MPI_COMM_WORLD, &rreq[xcoord]);
      off += perrank;
    }
    BgAdvance(100);    
    MPI_Waitall(dims[MP_X] - 1, &sreq[1], MPI_STATUSES_IGNORE);
    MPI_Waitall(dims[MP_X] - 1, &rreq[1], MPI_STATUSES_IGNORE);
#else
    MPI_Alltoall(sendbuf, perrank_x, MPI_CHAR, recvbuf, perrank_x, MPI_CHAR, X_comm);
    MPI_Alltoall(sendbuf, perrank_y, MPI_CHAR, recvbuf, perrank_y, MPI_CHAR, Y_comm);
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
    printf("Finished %d iterations\n",MAX_ITER);
    printf("Time elapsed per iteration for sizes %d %d: %f\n", perrank_x, perrank_y, (stopTime - startTime)/MAX_ITER);
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

