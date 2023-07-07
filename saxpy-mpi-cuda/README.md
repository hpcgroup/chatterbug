## MPI+CUDA implementation of the saxpy program

#### Benchmark information
- Uses two processes and one GPU per process.
- Rank 0 initializes the input arrays and sends them to Rank 1.
- Both processes does the same saxpy calculation.
- Rank 1 sends the results to Rank 0.

The following parameters can be used to run the program:
- -i -> number of iterations.
- -N -> the problem size. 

#### Example
`./saxpy-mpi-cuda -i 5 -N 32768000`

