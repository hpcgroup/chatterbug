#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  int rank, numranks;
  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&numranks);

  if(!rank && argc != 5) {
    printf("Correct usage: ./%s src dst bytes num_iter\n", argv[0]);
    MPI_Abort(MPI_COMM_WORLD, 0);
  }

  int src = atoi(argv[1]);
  int dst = atoi(argv[2]);
  int size = atoi(argv[3]);
  int numIter = atoi(argv[4]);

  double starttime, endtime;
  char *question;

  if(src == rank || dst == rank) {
    question = (char*)malloc(size*sizeof(char));
  }

#if CMK_BIGSIM_CHARM
  AMPI_Set_startevent(MPI_COMM_WORLD);
#endif
  starttime = MPI_Wtime();
  for(int i = 0; i < numIter; i++) {
#if CMK_BIGSIM_CHARM
    if(!rank)
      BgPrintf("Current time is %f\n");
#else
    if(!rank)
      printf("Current time is %f\n", MPI_Wtime());
#endif
    if(rank == src) {
      MPI_Send(question, size, MPI_CHAR, dst, 0, MPI_COMM_WORLD);
    } else if(rank == dst) {
      MPI_Recv(question, size, MPI_CHAR, src, 0, MPI_COMM_WORLD,
        MPI_STATUS_IGNORE);
    }
  }
  endtime = MPI_Wtime();
#if CMK_BIGSIM_CHARM
  if(!rank)
    BgPrintf("After loop Current time is %f\n");
#endif

  if(rank == dst)
    printf("[%d] Time for size %d is %lf (%lf %lf)\n", rank, size,
      (endtime-starttime)/numIter, endtime, starttime);

  MPI_Finalize();
}



