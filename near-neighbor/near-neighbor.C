#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
using namespace std;
#include <vector>

#if CMK_BIGSIM_CHARM
#include "shared-alloc.h"
#include "blue.h"
#include "cktiming.h"     
int *degree = NULL;
int *srcP = NULL, *destP = NULL;
long long sum_g = 0;

extern "C" void BgMark(const char *str);

long long val = 0;

#define my_rand() rand()

void changeMessage(BgTimeLog *log)                  
{                                                   
  log->msgs[0]->msgsize = 5242880;            
}
#endif

#define calc_pe(a,b,c)	(a + b*nx + c*nx*ny)
#define wrap_x(a)	(((a)+nx)%nx)
#define wrap_y(a)	(((a)+ny)%ny)
#define wrap_z(a)	(((a)+nz)%nz)


int main(int argc, char **argv)
{
  int i, myrank, numranks, off;
  MPI_Init(&argc,&argv);
#if CMK_BIGSIM_CHARM
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Set_trace_status(0);
  MPI_Barrier(MPI_COMM_WORLD);
#endif
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  MPI_Comm_size(MPI_COMM_WORLD,&numranks);

  if(argc < 9) {
    if(!myrank)
      printf("Correct usage: %s minD maxD neighborhood base_grid_size (x,y,z) msg_size_bytes num_iters <count_size>\n",
          argv[0]);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  int minD = atoi(argv[1]);
  int maxD = atoi(argv[2]);
  int neighborhood = atoi(argv[3]);
  int nx = atoi(argv[4]);
  int ny = atoi(argv[5]);
  int nz = atoi(argv[6]);
  int msg_size = atoi(argv[7]);
  int MAX_ITER = atoi(argv[8]);
  int count_size = 100;
  if(argc > 9) {
    count_size = atoi(argv[9]);
  }
  
  if(!myrank)
    printf("Create neighbors\n");

  srand(myrank);
  int range = maxD - minD;
  if(range == 0) range = 1;
  int my_degree = (minD + (rand() % range))/1;
  
  int myXcoord = (myrank % nx);
  int myYcoord = (myrank % (nx * ny)) / nx;
  int myZcoord = (myrank % (nx * ny * nz)) / (nx * ny);

  int *neighbors = new int[my_degree];
  for(int i = 0; i < my_degree; i++) {
    int tx = wrap_x(myXcoord + (((rand() % 2) ? -1 : 1) * (rand() % neighborhood)));
    int ty = wrap_y(myYcoord + (((rand() % 2) ? -1 : 1) * (rand() % neighborhood)));
    int tz = wrap_z(myZcoord + (((rand() % 2) ? -1 : 1) * (rand() % neighborhood)));
    neighbors[i] = calc_pe(tx, ty, tz);
  }

  int numNeighbors;
  int *counts = new int[count_size];
  int *new_counts = new int[count_size];
  for(int i = 0; i < numranks; i += count_size) {
    if(!myrank) {
      printf("Reducing %d - %d\n", i, i + count_size);
    }
    for(int j = 0; j < count_size; j++) counts[j] = 0;
    for(int j = 0; j < my_degree; j++) {
      int abs_rank = neighbors[j] - i;
      if((abs_rank >= 0) && (abs_rank < count_size)) {
        counts[abs_rank]++;
      }
    }
    MPI_Allreduce(counts, new_counts, count_size, MPI_INTEGER, MPI_SUM, MPI_COMM_WORLD);
    if(myrank >= i && (myrank < (i + count_size))) {
      numNeighbors = new_counts[myrank - i];
    }
  }

  delete [] counts;
  delete [] new_counts;

  double startTime, stopTime;
  char *sendbuf, *recvbuf;

#if CMK_BIGSIM_CHARM
  sendbuf = (char*) shalloc (1 * msg_size * sizeof(char), 1);
  recvbuf = (char*) shalloc (1 * msg_size * sizeof(char), 1);
#else
  sendbuf = (char*) malloc (my_degree * msg_size * sizeof(char));
  recvbuf = (char*) malloc (numNeighbors * msg_size * sizeof(char));
#endif

  MPI_Request *sreq, *rreq;
  sreq = new MPI_Request[my_degree];
  rreq = new MPI_Request[numNeighbors];

  MPI_Barrier(MPI_COMM_WORLD);
#if CMK_BIGSIM_CHARM
  MPI_Set_trace_status(1);
  AMPI_Set_startevent(MPI_COMM_WORLD);
#endif
  startTime = MPI_Wtime();
#if CMK_BIGSIM_CHARM
  BgTimeLine &timeLine = tTIMELINEREC.timeline;  
  if(!myrank)
    BgPrintf("Current time is %f\n");
#endif
  for (i = 0; i < MAX_ITER; i++) {
#if CMK_BIGSIM_CHARM
    BgMark("SmallMsgs_Setup");
#endif
    for(int i = 0; i < numNeighbors; i++) {
      MPI_Irecv(&recvbuf[0], msg_size, MPI_CHAR, MPI_ANY_SOURCE, 0,
        MPI_COMM_WORLD, &rreq[i]);
    }
    for(int i = 0; i < my_degree; i++) {
#if CMK_BIGSIM_CHARM
      BgMark("SmallMsgs_SendTime");
#endif
      MPI_Isend(&sendbuf[0], msg_size, MPI_CHAR, neighbors[i], 0,
        MPI_COMM_WORLD, &sreq[i]);
#if CMK_BIGSIM_CHARM
      changeMessage(timeLine[timeLine.length() - 3]);
#endif
    }
    MPI_Waitall(my_degree, &sreq[0], MPI_STATUSES_IGNORE);
    MPI_Waitall(numNeighbors, &rreq[0], MPI_STATUSES_IGNORE);
#if CMK_BIGSIM_CHARM
    BgMark("SmallMsgs_Work");
    MPI_Loop_to_start();
#endif
  }
#if CMK_BIGSIM_CHARM
  AMPI_Set_endevent();
  if(!myrank)
    BgPrintf("Before barrier Current time is %f\n");
#endif
  MPI_Barrier(MPI_COMM_WORLD);
  stopTime = MPI_Wtime();
#if CMK_BIGSIM_CHARM
  if(!myrank)
    BgPrintf("After loop Current time is %f\n");
#endif

  if(myrank == 0 && MAX_ITER != 0) {
    printf("Finished %d iterations\n",MAX_ITER);
    printf("Time elapsed per iteration for size %d: %f\n", msg_size, (stopTime -
    startTime)/MAX_ITER);
  }
#if CMK_BIGSIM_CHARM
  MPI_Set_trace_status(0);
  MPI_Barrier(MPI_COMM_WORLD);
#endif
  MPI_Finalize();
}
