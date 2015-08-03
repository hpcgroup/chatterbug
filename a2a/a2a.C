#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define MP_X 0
#define MP_Y 1
#define MP_Z 2

#define calc_pe(a,b,c)  ((a)+(b)*dims[MP_X]+(c)*dims[MP_X]*dims[MP_Y])

int main(int argc, char **argv)
{
  int i, myrank, numranks, off;
  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  MPI_Comm_size(MPI_COMM_WORLD,&numranks);

  if(!myrank && argc != 6) {
    printf("Correct usage: ./%s dimX dimY dimZ bytes_per_pair num_iter\n", argv[0]);
    MPI_Abort(MPI_COMM_WORLD, 0);
  }

  int dims[3] = {0, 0, 0};
  double startTime, stopTime;

  MPI_Request *sreq, *rreq;

  dims[MP_X] = atoi(argv[1]);
  dims[MP_Y] = atoi(argv[2]);
  dims[MP_Z] = atoi(argv[3]);
  int perrank = atoi(argv[4]);
  int MAX_ITER = atoi(argv[5]);

  int largerGroup = (dims[MP_X] > dims[MP_Y]) ? dims[MP_X] : dims[MP_Y];

  char *sendbuf = (char*)malloc(perrank * largerGroup);
  char *recvbuf = (char*)malloc(perrank * largerGroup);
  sreq = new MPI_Request[largerGroup];
  rreq = new MPI_Request[largerGroup];

  int myXcoord = myrank % dims[MP_X];
  int myYcoord = (myrank % (dims[MP_X] * dims[MP_Y])) / dims[MP_X];
  int myZcoord = (myrank % (dims[MP_X] * dims[MP_Y] * dims[MP_Z])) / (dims[MP_X] * dims[MP_Y]);

#if CMK_BIGSIM_CHARM
  AMPI_Set_startevent(MPI_COMM_WORLD);
#endif
  startTime = MPI_Wtime();
  for (i = 0; i < MAX_ITER; i++) {
#if CMK_BIGSIM_CHARM
    if(!myrank)
      BgPrintf("Current time is %f\n");
#else
    if(!myrank)
      printf("Current time is %f\n", MPI_Wtime());
#endif
    off = 0;
    for(int ycoord = 1; ycoord < dims[MP_Y]; ycoord++) {
      MPI_Isend(&sendbuf[off], perrank, MPI_CHAR,
        calc_pe(myXcoord, (ycoord + myYcoord) % dims[MP_Y], myZcoord), 0,
        MPI_COMM_WORLD, &sreq[ycoord]);
      MPI_Irecv(&recvbuf[off], perrank, MPI_CHAR,
        calc_pe(myXcoord, (ycoord + myYcoord) % dims[MP_Y], myZcoord), 0,
        MPI_COMM_WORLD, &rreq[ycoord]);
      off += perrank;
    }
    MPI_Waitall(dims[MP_Y] - 1, &sreq[1], MPI_STATUSES_IGNORE);
    MPI_Waitall(dims[MP_Y] - 1, &rreq[1], MPI_STATUSES_IGNORE);
    off = 0;
    for(int xcoord = 1; xcoord < dims[MP_X]; xcoord++) {
      MPI_Isend(&sendbuf[off], perrank, MPI_CHAR,
        calc_pe((xcoord + myXcoord) % dims[MP_X], myYcoord, myZcoord), 0,
        MPI_COMM_WORLD, &sreq[xcoord]);
      MPI_Isend(&recvbuf[off], perrank, MPI_CHAR,
        calc_pe((xcoord + myXcoord) % dims[MP_X], myYcoord, myZcoord), 0,
        MPI_COMM_WORLD, &rreq[xcoord]);
      off += perrank;
    }
    MPI_Waitall(dims[MP_X] - 1, &sreq[1], MPI_STATUSES_IGNORE);
    MPI_Waitall(dims[MP_X] - 1, &rreq[1], MPI_STATUSES_IGNORE);
  }
  stopTime = MPI_Wtime();
#if CMK_BIGSIM_CHARM
  if(!myrank)
    BgPrintf("After loop Current time is %f\n");
#endif

  if(myrank == 0) {
    printf("Finished %d iterations\n",MAX_ITER);
    printf("Time elapsed per iteration for size %d: %f\n", perrank, (stopTime -
    startTime)/MAX_ITER);
  }
  MPI_Finalize();
}








