#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
using namespace std;
#include <vector>

int main(int argc, char **argv)
{
  int i, myrank, numranks, off;
  MPI_Init(&argc,&argv);
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

  vector<int> degree;
  vector<int> neighbors;
  degree.resize(numranks);

  srand(1331);
  int range = maxD - minD;
  if(range == 0) range = 1;
  long long sum = 0;
  for(int i = 0; i < numranks; i++) {
    degree[i] = minD + (rand() % range);
    sum += degree[i];
  }

  long long count = 25*sum;
  while(sum > 0 && count > 0) {
    count--;
    int src = rand() % numranks;
    if(degree[src] == 0) continue;
    int multby = 1;
    if(rand() % 2) multby = -1;
    int dst = ((src + multby * (rand() % neighborhood)) + numranks) % numranks;
    if(degree[dst] == 0) continue;
    if(src == dst) continue;
    degree[src]--;
    degree[dst]--;
    sum--;
    if(src == myrank) {
      neighbors.push_back(dst);
    }
    if(dst == myrank) {
      neighbors.push_back(src);
    }
  }

  int numNeighbors = neighbors.size();
  double startTime, stopTime;
  char *sendbuf, *recvbuf;

  sendbuf = (char*) malloc (numNeighbors * msg_size * sizeof(char));
  recvbuf = (char*) malloc (numNeighbors * msg_size * sizeof(char));

  MPI_Request *sreq, *rreq;
  sreq = new MPI_Request[numNeighbors];
  rreq = new MPI_Request[numNeighbors];

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
    for(int i = 0; i < numNeighbors; i++) {
      MPI_Isend(&sendbuf[off], msg_size, MPI_CHAR, neighbors[i], 0,
        MPI_COMM_WORLD, &sreq[i]);
      MPI_Irecv(&recvbuf[off], msg_size, MPI_CHAR, neighbors[i], 0,
        MPI_COMM_WORLD, &rreq[i]);
      off += msg_size;
    }
    MPI_Waitall(numNeighbors, &sreq[0], MPI_STATUSES_IGNORE);
    MPI_Waitall(numNeighbors, &rreq[0], MPI_STATUSES_IGNORE);
  }
  stopTime = MPI_Wtime();
#if CMK_BIGSIM_CHARM
  if(!myrank)
    BgPrintf("After loop Current time is %f\n");
#endif

  if(myrank == 0) {
    printf("Finished %d iterations\n",MAX_ITER);
    printf("Time elapsed per iteration for size %d: %f\n", msg_size, (stopTime -
    startTime)/MAX_ITER);
  }

  MPI_Finalize();
}
