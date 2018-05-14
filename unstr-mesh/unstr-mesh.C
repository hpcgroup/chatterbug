#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
using namespace std;
#include <vector>

#if WRITE_OTF2_TRACE
#include <scorep/SCOREP_User.h>
#endif

// Calculate MPI rank based on 3D cartesian coordinates
#define calc_pe(a,b,c)	(a + b*nx + c*nx*ny)
#define wrap_x(a)	(((a)+nx)%nx)
#define wrap_y(a)	(((a)+ny)%ny)
#define wrap_z(a)	(((a)+nz)%nz)


int main(int argc, char **argv)
{
  int i, myrank, numranks;
  MPI_Init(&argc,&argv);
#if WRITE_OTF2_TRACE
  SCOREP_RECORDING_OFF();
#endif
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  MPI_Comm_size(MPI_COMM_WORLD,&numranks);

  if(argc != 9) {
    if(!myrank)
      printf("\nThis is the unstr-mesh communication proxy. The correct usage is:\n"
             "%s nx ny nz minD maxD neighborhood msg_size MAX_ITER\n\n"
             "    nx, ny, nz: layout of process grid in 3D\n"
             "    minD, maxD: range of expected degree of each rank (for example, degree of a 7-point 3D stencil is 7\n"
             "    neighborhood: how far in each dimension can we go for searching a neighbor (usually a small number like 2-3)\n"
             "    msg_size: size of message per pair (in bytes)\n"
             "    MAX_ITER: how many iters to run\n\n",
             argv[0]);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  int nx = atoi(argv[1]);
  int ny = atoi(argv[2]);
  int nz = atoi(argv[3]);
  int minD = atoi(argv[4]);
  int maxD = atoi(argv[5]);
  int neighborhood = atoi(argv[6]);
  int msg_size = atoi(argv[7]);
  int MAX_ITER = atoi(argv[8]);

  if(nx * ny * nz != numranks) {
    if(!myrank) {
      printf("\n nx * ny * nz does not equal number of ranks. \n");
    }
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  // figure out my coordinates
  int myXcoord = (myrank % nx);
  int myYcoord = (myrank % (nx * ny)) / nx;
  int myZcoord = (myrank % (nx * ny * nz)) / (nx * ny);

  if(!myrank)
    printf("Create neighbors\n");

  // figure out number of destinations for each process
  srand(myrank);
  int range = maxD - minD;
  if(range == 0) range = 1;
  int my_degree = (minD + (rand() % range));
  
  // figure out destination processes for a process
  int *neighbors = new int[my_degree];
  for(int i = 0; i < my_degree; i++) {
    int tx = wrap_x(myXcoord + (((rand() % 2) ? -1 : 1) * (rand() % neighborhood)));
    int ty = wrap_y(myYcoord + (((rand() % 2) ? -1 : 1) * (rand() % neighborhood)));
    int tz = wrap_z(myZcoord + (((rand() % 2) ? -1 : 1) * (rand() % neighborhood)));
    neighbors[i] = calc_pe(tx, ty, tz);
  }

  // figure out how many messages will a process receive
  int numNeighbors;
  int *counts = new int[numranks];
  int *new_counts = new int[numranks];
  for(int j = 0; j < numranks; j++) counts[j] = 0;
  for(int j = 0; j < my_degree; j++) {
    counts[neighbors[j]]++;
  }
  MPI_Allreduce(counts, new_counts, numranks, MPI_INTEGER, MPI_SUM, MPI_COMM_WORLD);
  numNeighbors = new_counts[myrank];
  delete [] counts;
  delete [] new_counts;

  double startTime, stopTime;
  char *sendbuf, *recvbuf;

  sendbuf = (char*) malloc (my_degree * msg_size * sizeof(char));
  recvbuf = (char*) malloc (numNeighbors * msg_size * sizeof(char));

  MPI_Request *sreq, *rreq;
  sreq = new MPI_Request[my_degree];
  rreq = new MPI_Request[numNeighbors];

  MPI_Barrier(MPI_COMM_WORLD);
#if WRITE_OTF2_TRACE
  SCOREP_RECORDING_ON();
  // Marks the beginning of code region to be repeated in simulation
  SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_Loop", SCOREP_USER_REGION_TYPE_COMMON);
  // Marks when to print a timer in simulation
  if(!myrank)
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_WallTime_umesh", SCOREP_USER_REGION_TYPE_COMMON);
#endif

  startTime = MPI_Wtime();
  for (i = 0; i < MAX_ITER; i++) {
#if WRITE_OTF2_TRACE
    // Marks compute region before messaging
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_umesh_pre_msg", SCOREP_USER_REGION_TYPE_COMMON);
    SCOREP_USER_REGION_BY_NAME_END("TRACER_umesh_pre_msg");
#endif

    for(int j = 0; j < numNeighbors; j++) {
      MPI_Irecv(&recvbuf[j * msg_size], msg_size, MPI_CHAR, MPI_ANY_SOURCE, i,
        MPI_COMM_WORLD, &rreq[j]);
    }
    for(int j = 0; j < my_degree; j++) {
      MPI_Isend(&sendbuf[j * msg_size], msg_size, MPI_CHAR, neighbors[j], i,
        MPI_COMM_WORLD, &sreq[j]);
    }

#if WRITE_OTF2_TRACE
    // Marks compute region for computation-communication overlap
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_umesh_overlap", SCOREP_USER_REGION_TYPE_COMMON);
    SCOREP_USER_REGION_BY_NAME_END("TRACER_umesh_overlap");
#endif

    MPI_Waitall(my_degree, &sreq[0], MPI_STATUSES_IGNORE);
    MPI_Waitall(numNeighbors, &rreq[0], MPI_STATUSES_IGNORE);

#if WRITE_OTF2_TRACE
    // Marks compute region after messaging
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_umesh_post_msg", SCOREP_USER_REGION_TYPE_COMMON);
    SCOREP_USER_REGION_BY_NAME_END("TRACER_umesh_post_msg");
#endif
  }

#if WRITE_OTF2_TRACE
  // Marks the end of code region to be repeated in simulation
  SCOREP_USER_REGION_BY_NAME_END("TRACER_Loop");
#endif
  MPI_Barrier(MPI_COMM_WORLD);
  stopTime = MPI_Wtime();

#if WRITE_OTF2_TRACE
  // Marks when to print a timer in simulation
  if(!myrank)
    SCOREP_USER_REGION_BY_NAME_END("TRACER_WallTime_umesh");
  SCOREP_RECORDING_OFF();
#endif

  if(myrank == 0 && MAX_ITER != 0) {
    printf("Finished %d iterations\n",MAX_ITER);
    printf("Time elapsed per iteration for size %d: %f s\n", msg_size, 
    (stopTime - startTime)/MAX_ITER);
  }

  MPI_Finalize();
}
