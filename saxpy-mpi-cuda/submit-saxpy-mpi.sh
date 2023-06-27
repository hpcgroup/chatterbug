#!/bin/bash
#SBATCH -N 2
#SBATCH --gres=gpu:a100:1
#SBATCH -t 00:10:00
#SBATCH -n 2
####SBATCH --exclusive 
#SBATCH -p gpu 

cd /home/ocankur/software/chatterbug/saxpy-mpi-cuda

module load cuda
module load openmpi 
#dcgmi profile --pause

echo "running job..."
mpirun -n 2 nsys profile --stats=true --trace=nvtx,mpi,cuda --cudabacktrace=all -o saxpy-mpi-cuda_nsys%q{OMPI_COMM_WORLD_RANK} ./saxpy-mpi-cuda -i 5 -N 32768000
nsys stats --report nvtxpptrace,cudaapitrace,gputrace,kernexectrace --format=csv --output saxpy-mpi-cuda_nsys0 saxpy-mpi-cuda_nsys0.sqlite
#nsys stats --report nvtxpptrace,cudaapitrace,gputrace,kernexectrace --format=csv --output saxpy-mpi-cuda_nsys1 saxpy-mpi-cuda_nsys1.sqlite

# NCU IS NOT NEEDED.
#mpirun -n 2 ncu --nvtx --target-processes all --print-summary=per-nvtx --call-stack --print-kernel-base function --set detailed --page=raw --csv ./saxpy-mpi-cuda -i 5 -N 32768000
#mpirun -n 2 ncu --nvtx -o saxpy-mpi-cuda_ncu ./saxpy-mpi-cuda -i 5 -N 32768000
#ncu -i saxpy-mpi-cuda_ncu.ncu-rep --csv --log-file saxpy-mpi-cuda_ncu.csv
