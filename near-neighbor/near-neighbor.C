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

int main(int argc, char **argv)
{
  int i, myrank, numranks, off;
  MPI_Init(&argc,&argv);
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Set_trace_status(0);
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  MPI_Comm_size(MPI_COMM_WORLD,&numranks);

  if(!myrank && argc != 6) {
    printf("Correct usage: %s minD maxD neighborhood msg_size_bytes num_iters\n",
      argv[0]);
    return 1;
  }

  int minD = atoi(argv[1]);
  int maxD = atoi(argv[2]);
  int neighborhood = atoi(argv[3]);
  int msg_size = atoi(argv[4]);
  int MAX_ITER = atoi(argv[5]);
  
  if(!myrank)
    printf("Create neighbors\n");

  vector<int> neighbors;
#if CMK_BIGSIM_CHARM
  if(degree == NULL) {
#endif
    degree = (int*)malloc(numranks*sizeof(int));
    srand(1337);
    int range = maxD - minD;
    if(range == 0) range = 1;
    long long sum = 0;
    for(int i = 0; i < numranks; i++) {
      degree[i] = minD + (rand() % range);
      sum += degree[i];
    }

    srcP = (int*)malloc(sum*sizeof(int));
    destP = (int*)malloc(sum*sizeof(int));

    long long count = 2*sum;
    sum_g = 0;
    sum /= 2;
    while(sum > 0 && count > 0) {
      count--;
#if SPREAD
      int src = rand() % (numranks/2);
#else
      int src = my_rand() % numranks;
#endif
      if(degree[src] == 0) continue;
      int multby = 1;
#if SPREAD
      int dst = (numranks/2 + (rand() % (numranks/2))) % numranks;
#else
      //if(my_rand() % 2) multby = -1;
      int dst = ((src + multby * (my_rand() % neighborhood)) + numranks) % numranks;
#endif
      if(degree[dst] == 0) continue;
      if(src == dst) continue;
      degree[src]--;
      degree[dst]--;
      sum--;
      srcP[sum_g] = src;
      destP[sum_g++] = dst;
    }
    printf("Created: %lld, Left %lld\n", sum_g, sum);
#if CMK_BIGSIM_CHARM
  }
#endif

  for(int i = 0; i < sum_g; i++) {
    if(srcP[i] == myrank) {
      neighbors.push_back(destP[i]);
    }
    if(destP[i] == myrank) {
      neighbors.push_back(srcP[i]);
    }
  }

#if !CMK_BIGSIM_CHARM
  free(srcP);
  free(destP);
#endif

  int numNeighbors = neighbors.size();
  double startTime, stopTime;
  char *sendbuf, *recvbuf;

#if CMK_BIGSIM_CHARM
  sendbuf = (char*) shalloc (maxD * msg_size * sizeof(char), 1);
  recvbuf = (char*) shalloc (maxD * msg_size * sizeof(char), 1);
#else
  sendbuf = (char*) malloc (numNeighbors * msg_size * sizeof(char));
  recvbuf = (char*) malloc (numNeighbors * msg_size * sizeof(char));
#endif

  MPI_Request *sreq, *rreq;
  sreq = new MPI_Request[numNeighbors];
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
    off = 0;
    for(int i = 0; i < numNeighbors; i++) {
      BgMark("SendTime");
      MPI_Isend(&sendbuf[off], msg_size, MPI_CHAR, neighbors[i], 0,
        MPI_COMM_WORLD, &sreq[i]);
#if CMK_BIGSIM_CHARM
      changeMessage(timeLine[timeLine.length() - 3]);
#endif
      MPI_Irecv(&recvbuf[off], msg_size, MPI_CHAR, neighbors[i], 0,
        MPI_COMM_WORLD, &rreq[i]);
      off += msg_size;
    }
#if CMK_BIGSIM_CHARM
    BgAdvance(100);    
#endif
    MPI_Waitall(numNeighbors, &sreq[0], MPI_STATUSES_IGNORE);
    MPI_Waitall(numNeighbors, &rreq[0], MPI_STATUSES_IGNORE);
  }
  AMPI_Set_endevent();
#if CMK_BIGSIM_CHARM
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
