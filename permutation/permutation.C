#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#if WRITE_OTF2_TRACE
#include <scorep/SCOREP_User.h>
#elif CMK_BIGSIM_CHARM
#include "shared-alloc.h"
#include "blue.h"
#include "cktiming.h"     
int *partners = NULL;

void changeMessage(BgTimeLog *log)                  
{                                                   
  log->msgs[0]->msgsize = 5242880;            
}

extern "C" void BgMark(const char *str);
#endif

int main(int argc, char **argv)
{
  MPI_Init(&argc,&argv);
#if WRITE_OTF2_TRACE
  SCOREP_RECORDING_OFF();
#elif CMK_BIGSIM_CHARM
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Set_trace_status(0);
  MPI_Barrier(MPI_COMM_WORLD);
#endif
  int myrank, numranks;
  int numIter = 2;

  double starttime, endtime;
  MPI_Status status;

  int size = atoi(argv[1]);
  if(argc > 2) {
    numIter = atoi(argv[2]);
  }

  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  MPI_Comm_size(MPI_COMM_WORLD,&numranks);

  char *question, *answer;

#if CMK_BIGSIM_CHARM
  question = (char*)shalloc(size*sizeof(char), 1);
  answer = (char*)shalloc(size*sizeof(char), 1);
#else
  question = (char*)malloc(size*sizeof(char));
  answer = (char*)malloc(size*sizeof(char));
#endif
  
  if(!myrank) {
    printf("Creating rank pairs\n");
  }

#if CMK_BIGSIM_CHARM
  if(partners == NULL) {
    partners = (int*)malloc(numranks/2*sizeof(int));
    for(int i = 0; i < numranks/2; i++) {
      partners[i] = numranks/2 + i;
    }
    srand(1331);
    for(int i = 0; i < numranks/2; i++) {
      int index1 = rand() % (numranks/2);
      int index2 = rand() % (numranks/2);
      int temp = partners[index1];
      partners[index1] = partners[index2];
      partners[index2] = temp;
    }
  }
#else
  int *partners = (int*)malloc(numranks/2*sizeof(int));
  if(myrank == 0) {
    for(int i = 0; i < numranks/2; i++) {
      partners[i] = numranks/2 + i;
    }

    srand(1331);
    for(int i = 0; i < 2*numranks; i++) {
      int index1 = rand() % (numranks/2);
      int index2 = rand() % (numranks/2);
      int temp = partners[index1];
      partners[index1] = partners[index2];
      partners[index2] = temp;
    }
  }
  MPI_Bcast(partners, numranks/2, MPI_INT, 0, MPI_COMM_WORLD);
#endif

  int partner;
  if(myrank < numranks/2) {
    partner = partners[myrank];
    MPI_Send(&myrank, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
  } else {
    MPI_Recv(&partner, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }

#if !CMK_BIGSIM_CHARM
  free(partners);
#endif

  int requests[2];
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
  starttime = MPI_Wtime();

  for(int i = 0; i < numIter; i++) {
#if CMK_BIGSIM_CHARM
    BgMark("Permuation_Setup");    
#endif
    MPI_Irecv(answer,size,MPI_CHAR,partner,0,MPI_COMM_WORLD,&requests[0]);
    MPI_Isend(question,size,MPI_CHAR,partner,0,MPI_COMM_WORLD,&requests[1]);
#if CMK_BIGSIM_CHARM
    changeMessage(timeLine[timeLine.length() - 3]);
#endif
    MPI_Waitall(2, requests, MPI_STATUSES_IGNORE);
#if CMK_BIGSIM_CHARM
    BgMark("Permuation_Work");    
    MPI_Loop_to_start();
#endif
  }

#if WRITE_OTF2_TRACE
  SCOREP_USER_REGION_BY_NAME_END("TRACER_Loop");
#elif CMK_BIGSIM_CHARM
  AMPI_Set_endevent();
#endif
  MPI_Barrier(MPI_COMM_WORLD);
  endtime = MPI_Wtime();

  if(myrank == 0 && numIter != 0)
    printf("[%d] Iters %d Time for size %d is %lf s: (%lf %lf)\n",myrank, numIter, size,(endtime-starttime)/(numIter),endtime,starttime);

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
