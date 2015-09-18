#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  int rank, numranks;
  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&numranks);

  if(!rank && argc < 5) {
    printf("Correct usage: ./%s <src dst>+ bytes num_iter\n", argv[0]);
    MPI_Abort(MPI_COMM_WORLD, 0);
  }

  int src = -1, dst = -1;
  int numPairs = (argc - 3)/2;
  int curP = 1;
  for(int p = 0; p < numPairs; p++) {
    if(atoi(argv[curP]) == rank) {
      src = rank;
      dst = atoi(argv[curP + 1]);
    }
    if(atoi(argv[curP+1]) == rank) {
      src = atoi(argv[curP]);
      dst = rank;
    }
    curP += 2;
  }
  int size = atoi(argv[curP]);
  int numIter = atoi(argv[curP+1]);

  double starttime, endtime;
  char *question;
  int requests[2];

  if(src == rank || dst == rank) {
    question = (char*)malloc(size*sizeof(char));
  }

  MPI_Barrier(MPI_COMM_WORLD);
#if CMK_BIGSIM_CHARM
  AMPI_Set_startevent(MPI_COMM_WORLD);
#endif
  starttime = MPI_Wtime();
#if CMK_BIGSIM_CHARM
  if(rank == src)
    BgPrintf("Current time is %f\n");
#else
  if(rank == src)
    printf("[%d] Current time is %f\n", rank, MPI_Wtime() - starttime);
#endif
  for(int i = 0; i < numIter; i++) {
    if(rank == src) {
      MPI_Isend(question, size, MPI_CHAR, dst, 0, MPI_COMM_WORLD, &requests[0]);
      MPI_Irecv(question, size, MPI_CHAR, dst, 0, MPI_COMM_WORLD,
        &requests[1]);
      MPI_Waitall(2, requests, MPI_STATUSES_IGNORE);
    } else if(rank == dst) {
      MPI_Irecv(question, size, MPI_CHAR, src, 0, MPI_COMM_WORLD,
        &requests[0]);
      MPI_Isend(question, size, MPI_CHAR, src, 0, MPI_COMM_WORLD, &requests[1]);
      MPI_Waitall(2, requests, MPI_STATUSES_IGNORE);
    }
  }
#if CMK_BIGSIM_CHARM
  if(rank == src)
    BgPrintf("Before barrier Current time is %f\n");
#else
  if(rank == src)
    printf("[%d] Before barrier Current time is %f\n", rank, MPI_Wtime() - starttime);
#endif
  MPI_Barrier(MPI_COMM_WORLD);
  endtime = MPI_Wtime();
#if CMK_BIGSIM_CHARM
  if(rank == src)
    BgPrintf("After loop Current time is %f\n");
#else
  if(rank == src)
    printf("After loop Current time is %f\n", MPI_Wtime() - starttime);
#endif

  if(rank == src)
    printf("[%d] Time for size %d is %lf (%lf %lf)\n", rank, size,
      (endtime-starttime)/numIter, endtime, starttime);

  MPI_Finalize();
}



