#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define MP_X 0
#define MP_Y 1
#define MP_Z 2

int main(int argc, char **argv)
{
  int i,myrank, numranks, groupsize;

  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  MPI_Comm_size(MPI_COMM_WORLD,&numranks);

  if(!myrank && argc != 6) {
    printf("Correct usage: ./%s dimX dimY dimZ bytes num_iter\n", argv[0]);
    MPI_Abort(MPI_COMM_WORLD, 0);
  }

  int dims[3] = {0, 0, 0};
  int temp[3] = {0, 0, 0};
  int coord[3] = {0, 0, 0};
  int periods[3] = {1, 1, 1};
  double startTime, start5Time, stopTime;

  MPI_Comm cartcomm, subcommX, subcommY, subcommZ;

  dims[MP_X] = atoi(argv[1]);
  dims[MP_Y] = atoi(argv[2]);
  dims[MP_Z] = atoi(argv[3]);
  int msg_size = atoi(argv[4]);
  int MAX_ITER = atoi(argv[5]);

  MPI_Dims_create(numranks, 3, dims);
  MPI_Cart_create(MPI_COMM_WORLD, 3, dims, periods, 1, &cartcomm);
  MPI_Cart_get(cartcomm, 3, dims, periods, coord);

  temp[MP_X] = 1; temp[MP_Y] = 0; temp[MP_Z] = 0;
  MPI_Cart_sub(cartcomm, temp, &subcommX);
  temp[MP_X] = 0; temp[MP_Y] = 1; temp[MP_Z] = 0;
  MPI_Cart_sub(cartcomm, temp, &subcommY);
  temp[MP_X] = 0; temp[MP_Y] = 0; temp[MP_Z] = 1;
  MPI_Cart_sub(cartcomm, temp, &subcommZ);

  int *bufX = (int*)malloc(msg_size * sizeof(char));
  int *bufY = (int*)malloc(msg_size * sizeof(char));
  int *bufZ = (int*)malloc(msg_size * sizeof(char));
  int *redX = (int*)malloc(msg_size * sizeof(char));
  int *redY = (int*)malloc(msg_size * sizeof(char));
  int *redZ = (int*)malloc(msg_size * sizeof(char));

  srand(7177);

  int rootX = rand() % dims[MP_X];
  int rootY = rand() % dims[MP_Y];
  int rootZ = rand() % dims[MP_Z];

  startTime = MPI_Wtime();
  for (i = 0; i < MAX_ITER; i++) {
#if CMK_BIGSIM_CHARM
    if(!myrank)
      BgPrintf("Current time is %f\n");
#else
    if(!myrank)
      printf("Current time is %f\n", MPI_Wtime());
#endif
      MPI_Reduce(bufX, redX, msg_size/4, MPI_INTEGER, MPI_SUM, rootX, subcommX);
      MPI_Reduce(bufY, redY, msg_size/4, MPI_INTEGER, MPI_SUM, rootY, subcommY);
      MPI_Reduce(bufZ, redZ, msg_size/4, MPI_INTEGER, MPI_SUM, rootZ, subcommZ);
  }
  stopTime = MPI_Wtime();
#if CMK_BIGSIM_CHARM
  if(!myrank)
    BgPrintf("After loop Current time is %f\n");
#endif

  if(myrank == 0) {
    printf("Finished %d iterations\n",MAX_ITER);
    printf("Time elapsed per iteration for size %d: %f\n", msg_size,
      (stopTime - startTime)/MAX_ITER);
  }
  MPI_Finalize();
}








