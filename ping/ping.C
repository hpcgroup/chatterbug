#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  MPI_Init(&argc,&argv);
  int rank, numranks;

  double starttime, endtime;
  MPI_Status status;

  int src = atoi(argv[1]);
  int dst = atoi(argv[2]);
  int size = atoi(argv[3]);
  int numIter = atoi(argv[4]);

  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&numranks);

  char *question;

  if(src == rank || dst == rank) {
    question = (char*)malloc(size*sizeof(char));
  }

  //warm up
  for(int i = 0; i < 10; i++) {
    if(rank == src) {
      MPI_Send(question,size,MPI_CHAR,dst,0,MPI_COMM_WORLD);
    } else if(rank == dst) {
      MPI_Recv(question,size,MPI_CHAR,src,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);
  starttime = MPI_Wtime();
#if CMK_BIGSIM_CHARM
    if(!rank)
      BgPrintf("Before iteration Current time is %f\n");
#endif
  for(int i = 0; i < numIter; i++) {
#if CMK_BIGSIM_CHARM
    if(!rank)
      BgPrintf("Current time is %f\n");
#endif
    if(rank == src) {
      MPI_Send(question,size,MPI_CHAR,dst,0,MPI_COMM_WORLD);
    } else if(rank == dst) {
      MPI_Recv(question,size,MPI_CHAR,src,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    }
  }
  endtime = MPI_Wtime();
  MPI_Barrier(MPI_COMM_WORLD);
#if CMK_BIGSIM_CHARM
  if(!rank)
    BgPrintf("After barrier Current time is %f\n");
#endif
  
  if(rank == dst)
    printf("[%d] Time for %d size %lf: (%lf %lf)\n",rank,size,(endtime-starttime)/numIter,endtime,starttime);
  
  MPI_Barrier(MPI_COMM_WORLD);
#if CMK_BIGSIM_CHARM
  if(!rank)
    BgPrintf("Current time is %f\n");
#endif
  starttime = MPI_Wtime();
  if(rank == src) {
    MPI_Send(question,size,MPI_CHAR,dst,0,MPI_COMM_WORLD);
  } else if(rank == dst) {
    MPI_Recv(question,size,MPI_CHAR,src,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
  }
  endtime = MPI_Wtime();
#if CMK_BIGSIM_CHARM
  if(!rank)
    BgPrintf("Current time is %f\n");
#endif
  
  if(rank == dst || rank == src)
    printf("[%d] Time for %d size %lf: (%lf %lf)\n",rank,size,(endtime-starttime),endtime,starttime);

  MPI_Finalize();
}

  
      
