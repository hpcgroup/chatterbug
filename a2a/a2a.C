#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define MP_X 0
#define MP_Y 1
#define MP_Z 2

#define MAX_ITER 50

main(int argc, char **argv)
{
  MPI_Init(&argc,&argv);

  int i,myrank, numranks, groupsize;
  int dims[3] = {0, 0, 0};          
  int temp[3] = {0, 0, 0};          
  int coord[3] = {0, 0, 0};          
  int periods[3] = {1, 1, 1};
  double startTime, start5Time, stopTime;

  MPI_Comm cartcomm, subcomm;

  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  MPI_Comm_size(MPI_COMM_WORLD,&numranks);

  dims[MP_X] = atoi(argv[1]);
  dims[MP_Y] = atoi(argv[2]);
  dims[MP_Z] = atoi(argv[3]);
  MPI_Dims_create(numranks, 3, dims);
  MPI_Cart_create(MPI_COMM_WORLD, 3, dims, periods, 1, &cartcomm);
  MPI_Cart_get(cartcomm, 3, dims, periods, coord);
  temp[MP_X] = 0; temp[MP_Y] = 1; temp[MP_Z] = 0;
  MPI_Cart_sub(cartcomm, temp, &subcomm);

  MPI_Comm_size(subcomm,&groupsize);
  int perrank = atoi(argv[4]);
  char *sendbuf = (char*)malloc(perrank*groupsize);
  char *recvbuf = (char*)malloc(perrank*groupsize);

  MPI_Barrier(cartcomm);
  startTime = MPI_Wtime();
  for (i=0; i<MAX_ITER; i++) {
    if(i == 5) {
      MPI_Barrier(MPI_COMM_WORLD);
      start5Time = MPI_Wtime();
    }
    MPI_Alltoall(sendbuf, perrank, MPI_CHAR, recvbuf, perrank, MPI_CHAR, subcomm);
  }
  MPI_Barrier(cartcomm);
  stopTime = MPI_Wtime();

  if(myrank == 0) {
    printf("Finished %d iterations\n",MAX_ITER);
    printf("Time elapsed per iteration for size %d: %f\n", perrank, (stopTime - start5Time)/(MAX_ITER-5));
  }
  MPI_Finalize();
}








