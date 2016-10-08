#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#if CMK_BIGSIM_CHARM
#include "shared-alloc.h"
#include "blue.h"
#include "cktiming.h"     

void changeMessage(BgTimeLog *log)                  
{                                                   
  log->msgs[0]->msgsize = 5242880;            
}

extern "C" void BgMark(const char *str);
#endif

int main(int argc, char **argv)
{
  int i, myrank, numranks, off;
  MPI_Init(&argc,&argv);
#if CMK_BIGSIM_CHARM
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Set_trace_status(0);
  MPI_Barrier(MPI_COMM_WORLD);
#endif
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  MPI_Comm_size(MPI_COMM_WORLD,&numranks);

  if(!myrank && argc != 3) {
    printf("Correct usage: ./%s bytes_per_pair num_iter\n", argv[0]);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  double startTime, stopTime;

  MPI_Request sreq, rreq;

  int perrank = atoi(argv[1]);
  int MAX_ITER = atoi(argv[2]);

  int largerGroup = numranks;

#if CMK_BIGSIM_CHARM
  char *sendbuf = (char*)shalloc(perrank * largerGroup, 1);
  char *recvbuf = (char*)shalloc(perrank * largerGroup, 1);
#else
  char *sendbuf = (char*)malloc(perrank * largerGroup);
  char *recvbuf = (char*)malloc(perrank * largerGroup);
#endif

  MPI_Barrier(MPI_COMM_WORLD);
#if CMK_BIGSIM_CHARM
  MPI_Set_trace_status(1);
  AMPI_Set_startevent(MPI_COMM_WORLD);
#endif
  startTime = MPI_Wtime();
#if CMK_BIGSIM_CHARM
  BgTimeLine &timeLine = tTIMELINEREC.timeline;  
  if(!myrank)
    BgPrintf("Current time is %f\n");
#endif
  for (i = 0; i < MAX_ITER; i++) {
#if CMK_BIGSIM_CHARM
    BgMark("FFT_Setup");
#endif
    off = 0;
    for(int target = myrank + 1; target < numranks + myrank; target++) {
      MPI_Isend(&sendbuf[off], perrank, MPI_CHAR,
        target % numranks, 0, MPI_COMM_WORLD, &sreq);
#if CMK_BIGSIM_CHARM
      changeMessage(timeLine[timeLine.length() - 3]);
#endif
      MPI_Irecv(&recvbuf[off], perrank, MPI_CHAR,
        (myrank - (target - myrank) + numranks) % numranks, 0, MPI_COMM_WORLD, &rreq);
      off += perrank;
      MPI_Waitall(1, &sreq, MPI_STATUSES_IGNORE);
      MPI_Waitall(1, &rreq, MPI_STATUSES_IGNORE);
    }
#if CMK_BIGSIM_CHARM
    BgMark("FFT_WORK");    
    MPI_Loop_to_start();
#endif
  }
#if CMK_BIGSIM_CHARM
  AMPI_Set_endevent();
#endif
  MPI_Barrier(MPI_COMM_WORLD);
  stopTime = MPI_Wtime();
#if CMK_BIGSIM_CHARM
  if(!myrank)
    BgPrintf("After loop Current time is %f\n");
  
  MPI_Set_trace_status(0);
#endif
  MPI_Barrier(MPI_COMM_WORLD);

  if(myrank == 0 && MAX_ITER != 0) {
    printf("Finished %d iterations\n",MAX_ITER);
    printf("Time elapsed per iteration for size %d: %f\n", perrank, (stopTime -
    startTime)/MAX_ITER);
  }
  MPI_Finalize();
}
