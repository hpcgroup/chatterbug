#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include "shared-alloc.h"
#include "cktiming.h"
void changeMessage(BgTimeLog *log);

#define EFFICIENT_TRACE 1

#if CMK_BIGSIM_CHARM
extern "C" void BgMark(const char *str);
#endif

#define wrap_x(a)	(((a)+num_blocks_x)%num_blocks_x)
#define wrap_y(a)	(((a)+num_blocks_y)%num_blocks_y)
#define wrap_z(a)	(((a)+num_blocks_z)%num_blocks_z)

#define calc_pe(a,b,c)	(a + b*num_blocks_x + c*num_blocks_x*num_blocks_y)

#define MAX	        2
#define LEFT		1
#define RIGHT		2
#define TOP		3
#define BOTTOM		4
#define FRONT		5
#define BACK		6

double startTime;
double endTime;

int main(int argc, char **argv) {
  int myRank, numPes;
  int *rankmap,*rrankmap;
  int color = 1;
  int MAX_ITER = MAX;

  MPI_Init(&argc, &argv);
#if EFFICIENT_TRACE
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Set_trace_status(0);
  MPI_Barrier(MPI_COMM_WORLD);
#endif
  MPI_Comm_size(MPI_COMM_WORLD, &numPes);
  MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
  MPI_Request req[6];
  MPI_Status status[6];

  int blockDimX, blockDimY, blockDimZ;
  int arrayDimX, arrayDimY, arrayDimZ;
  int noBarrier = 0;

  if (argc != 4 && argc != 8) {
    printf("%s [array_size] [block_size] [Iters]\n", argv[0]);
    printf("%s [array_size_X] [array_size_Y] [array_size_Z] [block_size_X] [block_size_Y] [block_size_Z] [Iters] \n", argv[0]);
    MPI_Abort(MPI_COMM_WORLD, -1);
  }

  if(argc == 4) {
    arrayDimZ = arrayDimY = arrayDimX = atoi(argv[1]);
    blockDimZ = blockDimY = blockDimX = atoi(argv[2]);
    MAX_ITER = atoi(argv[3]);
  }
  else {
    arrayDimX = atoi(argv[1]);
    arrayDimY = atoi(argv[2]);
    arrayDimZ = atoi(argv[3]);
    blockDimX = atoi(argv[4]);
    blockDimY = atoi(argv[5]);
    blockDimZ = atoi(argv[6]);
    MAX_ITER = atoi(argv[7]);
  }

  int num_blocks_x = (arrayDimX / blockDimX);
  int num_blocks_y = (arrayDimY / blockDimY);
  int num_blocks_z = (arrayDimZ / blockDimZ);
  
  int myXcoord = (myRank % num_blocks_x);
  int myYcoord = (myRank % (num_blocks_x * num_blocks_y)) / num_blocks_x;
  int myZcoord = (myRank % (num_blocks_x * num_blocks_y * num_blocks_z)) / (num_blocks_x * num_blocks_y);

  int iterations = 0, j, k, l;
  double error = 1.0, max_error = 0.0;

  if(myRank == 0) {
    printf("Running Jacobi on %d processors with (%d, %d, %d) elements\n", numPes, num_blocks_x, num_blocks_y, num_blocks_z);
    printf("Block Dimensions: %d %d %d\n", blockDimX, blockDimY, blockDimZ);
  }

  int msg_size = 1;
  /* Copy left, right, bottom, top, back, forward and backward  blocks into temporary arrays.*/

  double *left_block_out    = (double *)shalloc(sizeof(double) * msg_size, color++);
  double *right_block_out   = (double *)shalloc(sizeof(double) * msg_size, color++);
  double *left_block_in     = (double *)shalloc(sizeof(double) * msg_size, color++);
  double *right_block_in    = (double *)shalloc(sizeof(double) * msg_size, color++);

  double *bottom_block_out  = (double *)shalloc(sizeof(double) * msg_size, color++);  
  double *top_block_out     = (double *)shalloc(sizeof(double) * msg_size, color++);
  double *bottom_block_in   = (double *)shalloc(sizeof(double) * msg_size, color++);
  double *top_block_in      = (double *)shalloc(sizeof(double) * msg_size, color++);
  
  double *front_block_out   = (double *)shalloc(sizeof(double) * msg_size, color++);
  double *back_block_out    = (double *)shalloc(sizeof(double) * msg_size, color++);
  double *front_block_in    = (double *)shalloc(sizeof(double) * msg_size, color++);
  double *back_block_in     = (double *)shalloc(sizeof(double) * msg_size, color++);
  
  MPI_Barrier(MPI_COMM_WORLD);
#if CMK_BIGSIM_CHARM
#if EFFICIENT_TRACE
  MPI_Set_trace_status(1);
#endif
  AMPI_Set_startevent(MPI_COMM_WORLD);
#endif

#if CMK_BIGSIM_CHARM
  BgTimeLine &timeLine = tTIMELINEREC.timeline;  
  if(!myRank)
    BgPrintf("Current time is %f\n");
#endif
  while(/*error > 0.001 &&*/ iterations < MAX_ITER) {
    iterations++;
    MPI_Irecv(right_block_in, msg_size, MPI_DOUBLE, calc_pe(wrap_x(myXcoord+1), myYcoord, myZcoord), RIGHT, MPI_COMM_WORLD, &req[RIGHT-1]);
    MPI_Irecv(left_block_in, msg_size, MPI_DOUBLE, calc_pe(wrap_x(myXcoord-1), myYcoord, myZcoord), LEFT, MPI_COMM_WORLD, &req[LEFT-1]);
    MPI_Irecv(top_block_in, msg_size, MPI_DOUBLE, calc_pe(myXcoord,wrap_y(myYcoord+1), myZcoord), TOP, MPI_COMM_WORLD, &req[TOP-1]);
    MPI_Irecv(bottom_block_in, msg_size, MPI_DOUBLE, calc_pe(myXcoord,wrap_y(myYcoord-1), myZcoord), BOTTOM, MPI_COMM_WORLD, &req[BOTTOM-1]);
    MPI_Irecv(front_block_in, msg_size, MPI_DOUBLE, calc_pe(myXcoord, myYcoord, wrap_z(myZcoord+1)),FRONT, MPI_COMM_WORLD, &req[FRONT-1]);
    MPI_Irecv(back_block_in, msg_size, MPI_DOUBLE, calc_pe(myXcoord, myYcoord, wrap_z(myZcoord-1)),BACK, MPI_COMM_WORLD, &req[BACK-1]);

    MPI_Send(left_block_out, msg_size, MPI_DOUBLE, calc_pe(wrap_x(myXcoord-1), myYcoord, myZcoord), RIGHT, MPI_COMM_WORLD);
    changeMessage(timeLine[timeLine.length() - 3]);

    MPI_Send(right_block_out, msg_size, MPI_DOUBLE, calc_pe(wrap_x(myXcoord+1), myYcoord, myZcoord), LEFT, MPI_COMM_WORLD);
    changeMessage(timeLine[timeLine.length() - 3]);

    MPI_Send(bottom_block_out, msg_size, MPI_DOUBLE, calc_pe(myXcoord, wrap_y(myYcoord-1), myZcoord), TOP, MPI_COMM_WORLD);
    changeMessage(timeLine[timeLine.length() - 3]);

    MPI_Send(top_block_out, msg_size, MPI_DOUBLE, calc_pe(myXcoord, wrap_y(myYcoord+1), myZcoord), BOTTOM, MPI_COMM_WORLD);
    changeMessage(timeLine[timeLine.length() - 3]);

    MPI_Send(back_block_out, msg_size, MPI_DOUBLE, calc_pe(myXcoord, myYcoord, wrap_z(myZcoord-1)), FRONT, MPI_COMM_WORLD);
    changeMessage(timeLine[timeLine.length() - 3]);

    MPI_Send(front_block_out, msg_size, MPI_DOUBLE, calc_pe(myXcoord, myYcoord, wrap_z(myZcoord+1)), BACK, MPI_COMM_WORLD);
    changeMessage(timeLine[timeLine.length() - 3]);

    MPI_Waitall(6, req, status);

#if CMK_BIGSIM_CHARM
    BgMark("Stencil3D");
#endif
    MPI_Loop_to_start();
  }
#if CMK_BIGSIM_CHARM
  AMPI_Set_endevent();
#endif
  MPI_Barrier(MPI_COMM_WORLD);
#if CMK_BIGSIM_CHARM
  if(!myRank)
    BgPrintf("After loop Current time is %f\n");
#endif

  if(myRank == 0) {
    endTime = MPI_Wtime();
    printf("Completed %d iterations\n", iterations);
    printf("Time elapsed per iteration: %f\n", (endTime - startTime)/(MAX_ITER));
  }
#if EFFICIENT_TRACE
  MPI_Set_trace_status(0);
  MPI_Barrier(MPI_COMM_WORLD);
#endif

  MPI_Finalize();
  return 0;
} /* end function main */

void changeMessage(BgTimeLog *log)
{
  log->msgs[0]->msgsize = 5242880;
}

