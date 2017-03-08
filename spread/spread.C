#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
using namespace std;
#include <vector>

#if WRITE_OTF2_TRACE
#include <scorep/SCOREP_User.h>
#elif CMK_BIGSIM_CHARM
#include "shared-alloc.h"
#include "blue.h"
#include "cktiming.h"     
int *degree = NULL;
int *srcP = NULL, *destP = NULL;
long long sum_g = 0;

extern "C" void BgMark(const char *str);

long long val = 0;

#define my_rand() rand()

void changeMessage(BgTimeLog *log)                  
{                                                   
  log->msgs[0]->msgsize = 5242880;            
}
#endif

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

  if(!myrank && argc != 7) {
    printf("Correct usage: %s minD maxD neighborhood msg_size_bytes num_iters count_size\n",
      argv[0]);
    return 1;
  }

  int minD = atoi(argv[1]);
  int maxD = atoi(argv[2]);
  int neighborhood = atoi(argv[3]);
  int msg_size = atoi(argv[4]);
  int MAX_ITER = atoi(argv[5]);
  int count_size = atoi(argv[6]);
  
  if(!myrank)
    printf("Create neighbors\n");

  srand(myrank);
  int range = maxD - minD;
  if(range == 0) range = 1;
  int my_degree = (minD + (rand() % range))/1;

  int *neighbors = new int[my_degree];
  for(int i = 0; i < my_degree; i++) {
    neighbors[i] = (myrank + (rand() % neighborhood)) % numranks;
  }

  int numNeighbors;
  int *counts = new int[count_size];
  int *new_counts = new int[count_size];
  for(int i = 0; i < numranks; i += count_size) {
    if(!myrank) {
      printf("Reducing %d - %d\n", i, i + count_size);
    }
    for(int j = 0; j < count_size; j++) counts[j] = 0;
    for(int j = 0; j < my_degree; j++) {
      int abs_rank = neighbors[j] - i;
      if((abs_rank >= 0) && (abs_rank < count_size)) {
        counts[abs_rank]++;
      }
    }
    MPI_Allreduce(counts, new_counts, count_size, MPI_INTEGER, MPI_SUM, MPI_COMM_WORLD);
    if(myrank >= i && (myrank < (i + count_size))) {
      numNeighbors = new_counts[myrank - i];
    }
  }

  delete [] counts;
  delete [] new_counts;

  double startTime, stopTime;
  char *sendbuf, *recvbuf;

#if CMK_BIGSIM_CHARM
  sendbuf = (char*) shalloc (1 * msg_size * sizeof(char), 1);
  recvbuf = (char*) shalloc (1 * msg_size * sizeof(char), 1);
#else
  sendbuf = (char*) malloc (my_degree * msg_size * sizeof(char));
  recvbuf = (char*) malloc (numNeighbors * msg_size * sizeof(char));
#endif

  MPI_Request *sreq, *rreq;
  sreq = new MPI_Request[my_degree];
  rreq = new MPI_Request[numNeighbors];

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
    BgMark("SmallMsgs_Setup");
#endif
    for(int i = 0; i < numNeighbors; i++) {
      MPI_Irecv(&recvbuf[0], msg_size, MPI_CHAR, MPI_ANY_SOURCE, 0,
        MPI_COMM_WORLD, &rreq[i]);
    }
    for(int i = 0; i < my_degree; i++) {
#if CMK_BIGSIM_CHARM
      BgMark("SmallMsgs_SendTime");
#endif
      MPI_Isend(&sendbuf[0], msg_size, MPI_CHAR, neighbors[i], 0,
        MPI_COMM_WORLD, &sreq[i]);
#if CMK_BIGSIM_CHARM
      changeMessage(timeLine[timeLine.length() - 3]);
#endif
    }
    MPI_Waitall(my_degree, &sreq[0], MPI_STATUSES_IGNORE);
    MPI_Waitall(numNeighbors, &rreq[0], MPI_STATUSES_IGNORE);
#if CMK_BIGSIM_CHARM
    BgMark("SmallMsgs_Work");
    MPI_Loop_to_start();
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
    printf("Time elapsed per iteration for size %d: %f\n", msg_size, (stopTime -
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
