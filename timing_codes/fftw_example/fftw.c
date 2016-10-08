 #include<fftw3.h>
#include <mpi.h>

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);
  fftw_complex *in, *out;
  fftw_plan my_plan;

  int N = atoi(argv[1]);
  int L = atoi(argv[2]);
  int dims[] = {N};
  in = (fftw_complex*) fftw_malloc(L*sizeof(fftw_complex)*N);
  out = (fftw_complex*) fftw_malloc(L*sizeof(fftw_complex)*N);
  my_plan = fftw_plan_many_dft(1, dims, L, in, NULL, 1, N, out, NULL, 1, N, FFTW_FORWARD, FFTW_MEASURE);

  double testTime = MPI_Wtime();
  fftw_execute(my_plan);
  fftw_execute(my_plan);
  printf("Test time %lf\n", (MPI_Wtime() - testTime)/1);
  int loop;
  double startTime = MPI_Wtime();
  for(loop = 0; loop < 100; loop++) {
    fftw_execute(my_plan);
  }
  double endTime = MPI_Wtime();
  printf("Average time %lf\n", (endTime - startTime)/100);
}
