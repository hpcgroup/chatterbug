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

extern "C" void BgMark(const char *str);
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

  if(!myrank && argc != 5) {
    printf("Correct usage: ./%s dimX dimY bytes_per_pair num_iter\n", argv[0]);
    MPI_Abort(MPI_COMM_WORLD, 0);
  }

  int dims[3] = {0, 0, 0};
  double startTime, stopTime;

  MPI_Request sreq, rreq;

  dims[MP_X] = atoi(argv[1]);
  dims[MP_Y] = atoi(argv[2]);
  dims[MP_Z] = numranks / (dims[MP_X] * dims[MP_Y]);
  if(dims[MP_X] * dims[MP_Y] * dims[MP_Z] != numranks) {
    if(!myrank) {
      printf("Product of dims does not match MPI rank count\n");
      MPI_Abort(MPI_COMM_WORLD, 0);
    }
  }
  int perrank = atoi(argv[3]);
  int MAX_ITER = atoi(argv[4]);

#if CMK_BIGSIM_CHARM
  char *sendbuf = (char*)shalloc(perrank * dims[MP_Z], 1);
  char *recvbuf = (char*)shalloc(perrank * dims[MP_Z], 1);
#else
  char *sendbuf = (char*)malloc(perrank * dims[MP_Z]);
  char *recvbuf = (char*)malloc(perrank * dims[MP_Z]);
#endif

  int myXcoord = myrank % dims[MP_X];
  int myYcoord = (myrank % (dims[MP_X] * dims[MP_Y])) / dims[MP_X];
  int myZcoord = (myrank % (dims[MP_X] * dims[MP_Y] * dims[MP_Z])) / (dims[MP_X] * dims[MP_Y]);

  MPI_Comm Z_comm;
  MPI_Comm_split(MPI_COMM_WORLD, myYcoord*dims[MP_X] + myXcoord, myZcoord, &Z_comm);

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
    BgMark("FFT_Setup");
    off = 0;
    for(int zcoord = 1; zcoord < dims[MP_Z]; zcoord++) {
      MPI_Isend(&sendbuf[off], perrank, MPI_CHAR,
        calc_pe(myXcoord, myYcoord, (zcoord + myZcoord) % dims[MP_Z]), 0,
        MPI_COMM_WORLD, &sreq);
      changeMessage(timeLine[timeLine.length() - 3]);
      MPI_Irecv(&recvbuf[off], perrank, MPI_CHAR,
        calc_pe(myXcoord, myYcoord, (myZcoord - zcoord + dims[MP_Z]) % dims[MP_Z]), 0,
        MPI_COMM_WORLD, &rreq);
      off += perrank;
      MPI_Waitall(1, &sreq, MPI_STATUSES_IGNORE);
      MPI_Waitall(1, &rreq, MPI_STATUSES_IGNORE);
    }
    BgMark("FFT_WORK");    
    MPI_Loop_to_start();
#else
    MPI_Alltoall(sendbuf, perrank, MPI_CHAR, recvbuf, perrank, MPI_CHAR, Z_comm);
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
    printf("Time elapsed per iteration for size %d: %f\n", perrank, (stopTime -
    startTime)/MAX_ITER);
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

