#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>

#define index(a,b,c)		((a)+(b)*(blockDimX+2)+(c)*(blockDimX+2)*(blockDimY+2))

#define min(a,b)        (a < b ? a : b)
#define MAX_ITER	5
#define DIVIDEBY7	0.1428571429

double startTime;
double endTime;

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);
  int blockSize, blockDimX, blockDimY, blockDimZ;

  blockDimX = atoi(argv[1]);
  blockDimY = atoi(argv[2]);
  blockDimZ = atoi(argv[3]);
  blockSize = atoi(argv[4]);

  int iterations = 0, i, j, k, l;
  double error = 1.0, max_error = 0.0;

  printf("Block Dimensions: %d %d %d \n", blockDimX, blockDimY, blockDimZ);

  double *restrict temperature;
  double *restrict new_temperature;

  /* allocate one dimensional arrays */
  temperature = new double[(blockDimX+2) * (blockDimY+2) * (blockDimZ+2)];
  new_temperature = new double[(blockDimX+2) * (blockDimY+2) * (blockDimZ+2)];

  for(k=0; k<blockDimZ+2; k++)
    for(j=0; j<blockDimY+2; j++)
      for(i=0; i<blockDimX+2; i++) {
        temperature[index(i, j, k)] = 0.0;
      }

  while(/*error > 0.001 &&*/ iterations < MAX_ITER) {
    startTime = MPI_Wtime();
    iterations++;

    for(int loop = 0; loop < 40; loop++) {
      for(int k1=1; k1<blockDimZ+1; k1 += blockSize) 
        for(int j1=1; j1<blockDimY+1; j1 += blockSize)
          for(int i1=1; i1<blockDimX+1; i1 += blockSize) {
            int km = min(k1+blockSize, blockDimZ+1);
            int jm = min(j1+blockSize, blockDimY+1);
            int im = min(i1+blockSize, blockDimX+1);
            for(k=k1; k<km; k++)
              for(j=j1; j<jm; j++)
                for(i=i1; i<im; i++) {
                  new_temperature[index(i, j, k)] = (temperature[index(i-1, j, k)]
                      +  temperature[index(i+1, j, k)]
                      +  temperature[index(i, j-1, k)]
                      +  temperature[index(i, j+1, k)]
                      +  temperature[index(i, j, k-1)]
                      +  temperature[index(i, j, k+1)]
                      +  temperature[index(i, j, k)]
                      +  temperature[index(i, j, k)]
                      +  temperature[index(i, j, k)] ) * DIVIDEBY7;
                }

          }
    }
    double *tmp;
    tmp = temperature;
    temperature = new_temperature;
    new_temperature = tmp;

    endTime = MPI_Wtime();
    printf("Completed %d iterations; Time elapsed : %f\n", iterations, (endTime - startTime));
  }

  MPI_Finalize();
  return 0;
} /* end function main */

