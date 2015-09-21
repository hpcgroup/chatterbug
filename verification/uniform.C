#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  MPI_Init(&argc,&argv);
  int rank, numranks;

  double starttime, endtime;
  MPI_Status status;

  int size = atoi(argv[1]);
  int numLoops = atoi(argv[2]);
  int numIter = atoi(argv[3]);

  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&numranks);

  char *question, *answer;

  question = (char*)malloc(size*sizeof(char));
  answer = (char*)malloc(size*sizeof(char));

  int *dest = (int*)malloc(numLoops*numIter*sizeof(int));
  int *allRanks = (int*)calloc(numranks, sizeof(int));
  int *recvRanks = (int*)calloc(numranks, sizeof(int));
  int numRecv;

  srand(rank);
  for(int i = 0; i < numLoops*numIter; i++) {
    dest[i] = rand() % numranks;
    allRanks[dest[i]]++;
  }
  for(int i = 0; i < numranks; i++) {
    recvRanks[i] = 1;
  }
  MPI_Reduce_scatter(allRanks, &numRecv, recvRanks, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
  MPI_Barrier(MPI_COMM_WORLD);
#if CMK_BIGSIM_CHARM
  AMPI_Set_startevent(MPI_COMM_WORLD);
#endif
  starttime = MPI_Wtime();
#if CMK_BIGSIM_CHARM
  if(!rank)
    BgPrintf("Current time is %f\n");
#endif
  int next = 0;
  for(int l = 0; l < numLoops; l++) {
    for(int i = 0; i < numIter; i++) {
      MPI_Send(question,size,MPI_CHAR,dest[next],0,MPI_COMM_WORLD);
      next++;
    }
  }
  for(int i = 0; i < numRecv; i++) {
    MPI_Recv(answer,size,MPI_CHAR,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  endtime = MPI_Wtime();
#if CMK_BIGSIM_CHARM
  if(!rank)
    BgPrintf("After loop Current time is %f\n");
#else
  if(!rank)
    printf("After loop Current time is %f\n", MPI_Wtime() - starttime);
#endif
  
  if(rank == 0)
    printf("[%d] Time for size %d is %lf : (%lf %lf)\n",rank,size,(endtime-starttime)/(numIter),endtime,starttime);

  MPI_Finalize();
} 
