#include "mpi.h"
#include "mpix.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  MPI_Init(&argc,&argv);
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

  question = (char*)malloc(size*sizeof(char));
  answer = (char*)malloc(size*sizeof(char));

  int *partners = (int*)malloc(numranks/2*sizeof(int));
  if(rank == 0) {
    for(int i = 0; i < numranks/2; i++) {
      partners[i] = numranks/2 + i;
    }

    srand(1331);
    for(int i = 0; i < 2*numranks; i++) {
      int index1 = rand() % numranks/2;
      int index2 = rand() % numranks/2;
      int temp = partners[index1];
      partners[index1] = partners[index2];
      partners[index2] = temp;
    }
  }
 
  MPI_Bcast(partners, numranks/2, MPI_INT, 0, MPI_COMM_WORLD);

  int partner;

  if(rank < numranks/2) {
    partner = partners[rank];
    MPI_Send(&rank, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
  } else {
    MPI_Recv(&partner, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
  free(partners);

  //warm up
  int requests[2];
  for(int i = 0; i < 10; i++) {
    MPI_Irecv(answer,size,MPI_CHAR,partner,0,MPI_COMM_WORLD,&requests[0]);
    MPI_Isend(question,size,MPI_CHAR,partner,0,MPI_COMM_WORLD,&requests[1]);
    MPI_Waitall(2, requests, MPI_STATUSES_IGNORE);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  for(int i = 0; i < numIter; i++) {
    if(i == 10) {
      MPI_Barrier(MPI_COMM_WORLD);
      starttime = MPI_Wtime();
    }
    MPI_Irecv(answer,size,MPI_CHAR,partner,0,MPI_COMM_WORLD,&requests[0]);
    MPI_Isend(question,size,MPI_CHAR,partner,0,MPI_COMM_WORLD,&requests[1]);
    MPI_Waitall(2, requests, MPI_STATUSES_IGNORE);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  endtime = MPI_Wtime();
  
  if(rank == 0)
    printf("[%d] Time for size %d is %lf : (%lf %lf)\n",rank,size,(endtime-starttime)/(numIter-10),endtime,starttime);

  MPI_Finalize();
} 
