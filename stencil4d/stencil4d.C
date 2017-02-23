#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#if CMK_BIGSIM_CHARM
#include "shared-alloc.h"
#include "cktiming.h"
#else
#define shalloc(a,b) malloc(a)
#endif
#if OTF
#include <scorep/SCOREP_User.h>
#endif

/* We want to wrap entries around, and because mod operator % sometimes
 * misbehaves on negative values. -1 maps to the highest value.
 */

#define CA      2
#define CB      2
#define CC      2
#define CD      2
#define PA      2
#define PAB     4
#define PABC    8
#define PABCD   16
#define wrap_x(a)	(((a)+num_blocks_x)%num_blocks_x)
#define wrap_y(a)	(((a)+num_blocks_y)%num_blocks_y)
#define wrap_z(a)	(((a)+num_blocks_z)%num_blocks_z)
#define wrap_t(a)	(((a)+num_blocks_t)%num_blocks_t)

#define calc_pe(a,b,c,d)	(((a/CA) + (b/CB)*num_blocks_x_c + (c/CC)*num_blocks_x_c*num_blocks_y_c \
				+ (d/CD)*num_blocks_x_c*num_blocks_y_c*num_blocks_z_c) * PABCD + \
                                (a%CA) + (b%CB)*PA + (c%CC)*PAB + (d%CD)*PABC)

#define MAX_ITER	2
#define LEFT		1
#define RIGHT		2
#define TOP		3
#define BOTTOM		4
#define FRONT		5
#define BACK		6
#define FORWARD		7
#define BACKWARD	8
#define DIVIDEBY9	0.11111111111111111

double startTime;
double endTime;

int main(int argc, char **argv) {
  SCOREP_RECORDING_OFF()
  int myRank, numPes;
  int *rankmap,*rrankmap;
  int color = 1;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numPes);
  MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
  MPI_Request req[8];
  MPI_Status status[8];

  int blockDimX, blockDimY, blockDimZ, blockDimT;
  int arrayDimX, arrayDimY, arrayDimZ, arrayDimT;
  int noBarrier = 0;

  if (argc != 4 && argc != 10) {
    printf("%s [array_size] [block_size] +[no]barrier\n", argv[0]);
    printf("%s [array_size_X] [array_size_Y] [array_size_Z] [array_size_T] [block_size_X] [block_size_Y] [block_size_Z] [block_size_T] +[no]barrier\n", argv[0]);
    MPI_Abort(MPI_COMM_WORLD, -1);
  }

  if(argc == 4) {
    arrayDimT = arrayDimZ = arrayDimY = arrayDimX = atoi(argv[1]);
    blockDimT = blockDimZ = blockDimY = blockDimX = atoi(argv[2]);
    if(strcasecmp(argv[3], "+nobarrier") == 0)
      noBarrier = 1;
    else
      noBarrier = 0;
    if(noBarrier && myRank==0) printf("\nSTENCIL COMPUTATION WITH NO BARRIERS\n");
  }
  else {
    arrayDimX = atoi(argv[1]);
    arrayDimY = atoi(argv[2]);
    arrayDimZ = atoi(argv[3]);
    arrayDimT = atoi(argv[4]);
    blockDimX = atoi(argv[5]);
    blockDimY = atoi(argv[6]);
    blockDimZ = atoi(argv[7]);
    blockDimT = atoi(argv[8]);
    if(strcasecmp(argv[9], "+nobarrier") == 0)
      noBarrier = 1;
    else
      noBarrier = 0;
    if(noBarrier && myRank==0) printf("\nSTENCIL COMPUTATION WITH NO BARRIERS\n");
  }

  if (arrayDimX < blockDimX || arrayDimX % blockDimX != 0) {
    printf("array_size_X % block_size_X != 0!\n");
    MPI_Abort(MPI_COMM_WORLD, -1);
  }
  if (arrayDimY < blockDimY || arrayDimY % blockDimY != 0) {
    printf("array_size_Y % block_size_Y != 0!\n");
    MPI_Abort(MPI_COMM_WORLD, -1);
  }
  if (arrayDimZ < blockDimZ || arrayDimZ % blockDimZ != 0) {
    printf("array_size_Z % block_size_Z != 0!\n");
    MPI_Abort(MPI_COMM_WORLD, -1);
  }
  if (arrayDimT < blockDimT || arrayDimT % blockDimT != 0) {
    printf("array_size_T % block_size_T != 0!\n");
    MPI_Abort(MPI_COMM_WORLD, -1);
  }

  int num_blocks_x = (arrayDimX / blockDimX);
  int num_blocks_y = (arrayDimY / blockDimY);
  int num_blocks_z = (arrayDimZ / blockDimZ);
  int num_blocks_t = (arrayDimT / blockDimT);
  
  int num_blocks_x_c = (arrayDimX / blockDimX)/CA;
  int num_blocks_y_c = (arrayDimY / blockDimY)/CB;
  int num_blocks_z_c = (arrayDimZ / blockDimZ)/CC;
  int num_blocks_t_c = (arrayDimT / blockDimT)/CD;

  int myXcoord = (myRank/PABCD) % num_blocks_x_c;
  int myYcoord = (myRank/PABCD) % (num_blocks_x_c * num_blocks_y_c) / num_blocks_x_c;
  int myZcoord = (myRank/PABCD) % (num_blocks_x_c * num_blocks_y_c * num_blocks_z_c) / (num_blocks_x_c * num_blocks_y_c);
  int myTcoord = (myRank/PABCD) / (num_blocks_x_c * num_blocks_y_c * num_blocks_z_c);

  myXcoord = (myXcoord * CA) + (myRank % PABCD) % CA;
  myYcoord = (myYcoord * CB) + ((myRank % PABCD)/PA) % CB;
  myZcoord = (myZcoord * CC) + ((myRank % PABCD)/PAB) % CC;
  myTcoord = (myTcoord * CD) + ((myRank % PABCD)/PABC) % CD;

  int iterations = 0, j, k, l;
  double error = 1.0, max_error = 0.0;

  if(myRank == 0) {
    printf("Running Jacobi on %d processors with (%d, %d, %d, %d) elements\n", numPes, num_blocks_x, num_blocks_y, num_blocks_z, num_blocks_t);
    printf("Array Dimensions: %d %d %d %d\n", arrayDimX, arrayDimY, arrayDimZ, arrayDimT);
    printf("Block Dimensions: %d %d %d %d\n", blockDimX, blockDimY, blockDimZ, blockDimT);
  }

  MPI_Comm newComm;
  int newRank;
  MPI_Comm_split(MPI_COMM_WORLD, myRank % 2, myRank, &newComm);
  MPI_Comm_rank(newComm, &newRank);
  MPI_Sendrecv(&error, 1, MPI_DOUBLE, (newRank + 1) % (numPes/2), 101, &error, 1, 
    MPI_DOUBLE, (newRank - 1 + numPes/2) % (numPes/2), 101, newComm, MPI_STATUS_IGNORE);

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
  
  double *forward_block_out = (double *)shalloc(sizeof(double) * msg_size, color++);
  double *backward_block_out= (double *)shalloc(sizeof(double) * msg_size, color++);
  double *forward_block_in  = (double *)shalloc(sizeof(double) * msg_size, color++);
  double *backward_block_in = (double *)shalloc(sizeof(double) * msg_size, color++);

  MPI_Barrier(MPI_COMM_WORLD);
#if CMK_BIGSIM_CHARM
  AMPI_Set_startevent(MPI_COMM_WORLD);
#endif

  startTime = MPI_Wtime();
#if CMK_BIGSIM_CHARM
    BgTimeLine &timeLine = tTIMELINEREC.timeline;  
  if(!myRank)
    BgPrintf("Current time is %f\n");
#endif
#if OTF
  SCOREP_RECORDING_ON();
  SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_Loop", SCOREP_USER_REGION_TYPE_COMMON);
  if(myRank == 0)
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_WallTime_MainLoop", SCOREP_USER_REGION_TYPE_COMMON);
#endif
  while(/*error > 0.001 &&*/ iterations < MAX_ITER) {
    if(myRank == 0)
      SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_WallTime_InLoop", SCOREP_USER_REGION_TYPE_COMMON);
    iterations++;
    MPI_Irecv(right_block_in, msg_size, MPI_DOUBLE, calc_pe(wrap_x(myXcoord+1), myYcoord, myZcoord, myTcoord), RIGHT, MPI_COMM_WORLD, &req[RIGHT-1]);
    MPI_Irecv(left_block_in, msg_size, MPI_DOUBLE, calc_pe(wrap_x(myXcoord-1), myYcoord, myZcoord, myTcoord), LEFT, MPI_COMM_WORLD, &req[LEFT-1]);
    MPI_Irecv(top_block_in, msg_size, MPI_DOUBLE, calc_pe(myXcoord,wrap_y(myYcoord+1), myZcoord, myTcoord), TOP, MPI_COMM_WORLD, &req[TOP-1]);
    MPI_Irecv(bottom_block_in, msg_size, MPI_DOUBLE, calc_pe(myXcoord,wrap_y(myYcoord-1), myZcoord, myTcoord), BOTTOM, MPI_COMM_WORLD, &req[BOTTOM-1]);
    MPI_Irecv(front_block_in, msg_size, MPI_DOUBLE, calc_pe(myXcoord, myYcoord, wrap_z(myZcoord+1), myTcoord),FRONT, MPI_COMM_WORLD, &req[FRONT-1]);
    MPI_Irecv(back_block_in, msg_size, MPI_DOUBLE, calc_pe(myXcoord, myYcoord, wrap_z(myZcoord-1), myTcoord),BACK, MPI_COMM_WORLD, &req[BACK-1]);
    MPI_Irecv(forward_block_in, msg_size, MPI_DOUBLE, calc_pe(myXcoord, myYcoord, myZcoord, wrap_t(myTcoord+1)), FORWARD, MPI_COMM_WORLD, &req[FORWARD-1]);
    MPI_Irecv(backward_block_in, msg_size, MPI_DOUBLE, calc_pe(myXcoord, myYcoord, myZcoord, wrap_t(myTcoord-1)), BACKWARD, MPI_COMM_WORLD, &req[BACKWARD-1]);

    MPI_Send(left_block_out, msg_size, MPI_DOUBLE, calc_pe(wrap_x(myXcoord-1), myYcoord, myZcoord, myTcoord), RIGHT, MPI_COMM_WORLD);

    MPI_Send(right_block_out, msg_size, MPI_DOUBLE, calc_pe(wrap_x(myXcoord+1), myYcoord, myZcoord, myTcoord), LEFT, MPI_COMM_WORLD);

    MPI_Send(bottom_block_out, msg_size, MPI_DOUBLE, calc_pe(myXcoord, wrap_y(myYcoord-1), myZcoord, myTcoord), TOP, MPI_COMM_WORLD);

    MPI_Send(top_block_out, msg_size, MPI_DOUBLE, calc_pe(myXcoord, wrap_y(myYcoord+1), myZcoord, myTcoord), BOTTOM, MPI_COMM_WORLD);

    MPI_Send(back_block_out, msg_size, MPI_DOUBLE, calc_pe(myXcoord, myYcoord, wrap_z(myZcoord-1), myTcoord), FRONT, MPI_COMM_WORLD);

    MPI_Send(front_block_out, msg_size, MPI_DOUBLE, calc_pe(myXcoord, myYcoord, wrap_z(myZcoord+1), myTcoord), BACK, MPI_COMM_WORLD);

    MPI_Send(backward_block_out, msg_size, MPI_DOUBLE, calc_pe(myXcoord, myYcoord, myZcoord, wrap_t(myTcoord-1)), FORWARD, MPI_COMM_WORLD);

    MPI_Send(forward_block_out, msg_size, MPI_DOUBLE, calc_pe(myXcoord, myYcoord, myZcoord, wrap_t(myTcoord+1)), BACKWARD, MPI_COMM_WORLD);

    MPI_Waitall(8, req, status);
    int send = 0, recv;
    //MPI_Reduce(&send, &recv, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    //MPI_Bcast(&send, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

#if CMK_BIGSIM_CHARM
    BgAdvance(100);
#endif
  }
#if OTF
  if(myRank == 0)
    SCOREP_USER_REGION_BY_NAME_END("TRACER_WallTime_MainLoop");
  SCOREP_USER_REGION_BY_NAME_END("TRACER_Loop");
  SCOREP_RECORDING_OFF()
#endif
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

  MPI_Finalize();
  return 0;
} /* end function main */
