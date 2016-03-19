#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#if CMK_BIGSIM_CHARM
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
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Set_trace_status(0);
  MPI_Barrier(MPI_COMM_WORLD);
  int rank, numranks;
  int numIter = 50;

  double starttime, endtime;
  MPI_Status status;

  int size = atoi(argv[1]);
  if(argc > 2) {
    numIter = atoi(argv[2]);
  }

  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&numranks);

  char *question, *answer;

#if CMK_BIGSIM_CHARM
  question = (char*)shalloc(size*sizeof(char), 1);
  answer = (char*)shalloc(size*sizeof(char), 1);
#else
  question = (char*)malloc(size*sizeof(char));
  answer = (char*)malloc(size*sizeof(char));
#endif
  
  if(!rank) {
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
  if(rank == 0) {
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
  if(rank < numranks/2) {
    partner = partners[rank];
    MPI_Send(&rank, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
  } else {
    MPI_Recv(&partner, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }

#if !CMK_BIGSIM_CHARM
  free(partners);
#endif

  int requests[2];
  MPI_Barrier(MPI_COMM_WORLD);

#if CMK_BIGSIM_CHARM
  MPI_Set_trace_status(1);
  AMPI_Set_startevent(MPI_COMM_WORLD);
#endif
  starttime = MPI_Wtime();
#if CMK_BIGSIM_CHARM
  BgTimeLine &timeLine = tTIMELINEREC.timeline;  
  if(!rank)
    BgPrintf("Current time is %f\n");
#endif
  for(int i = 0; i < numIter; i++) {
    MPI_Irecv(answer,size,MPI_CHAR,partner,0,MPI_COMM_WORLD,&requests[0]);
    MPI_Isend(question,size,MPI_CHAR,partner,0,MPI_COMM_WORLD,&requests[1]);
#if CMK_BIGSIM_CHARM
    changeMessage(timeLine[timeLine.length() - 3]);
#endif
#if CMK_BIGSIM_CHARM
    BgMark("Permuation");    
#endif
    MPI_Waitall(2, requests, MPI_STATUSES_IGNORE);
  }
#if CMK_BIGSIM_CHARM
  AMPI_Set_endevent();
  if(!rank)
    BgPrintf("Before barrier Current time is %f\n");
#else
  if(!rank)
    printf("%d Before barrier Current time is %f\n", rank, MPI_Wtime() - starttime);
#endif
  MPI_Barrier(MPI_COMM_WORLD);
  endtime = MPI_Wtime();
#if CMK_BIGSIM_CHARM
  if(!rank)
    BgPrintf("After loop Current time is %f\n");
#else
  if(!rank)
    printf("After loop Current time is %f\n", MPI_Wtime() - starttime);
#endif
  
  MPI_Set_trace_status(0);
  MPI_Barrier(MPI_COMM_WORLD);
  if(rank == 0 && numIter != 0)
    printf("[%d] Time for size %d is %lf : (%lf %lf)\n",rank,size,(endtime-starttime)/(numIter),endtime,starttime);

  MPI_Finalize();
} 
