MPI_FLAGS = -DUSE_MPI -I$(OPENMPI_ROOT)/include
CUDA_FLAGS = -I$(CUDA_HOME)/include

NVCC		= nvcc -ccbin $(MPICXX)
FLAGS		= -arch=sm_80
#DFLAGS	= $(MPI_FLAGS) $(CUDA_FLAGS) -lineinfo
RFLAGS 	= $(MPI_FLAGS) $(CUDA_FLAGS) -g -O3 -DNDEBUG 

LINKFLAGS = -lmpi -L$(OPENMPI_ROOT)/lib -L$(CUDA_HOME)/lib64 -lnvToolsExt 

all: release 
release: 	FLAGS += $(RFLAGS)
release: saxpy-mpi-cuda

saxpy-mpi-cuda: saxpy-mpi-cuda.o
	$(NVCC) $(LINKFLAGS) saxpy-mpi-cuda.o -o saxpy-mpi-cuda

saxpy-mpi-cuda.o: saxpy-mpi-cuda.cu
	$(NVCC) $(FLAGS) saxpy-mpi-cuda.cu -I ./ -c -o saxpy-mpi-cuda.o

clean: 
	rm -rf *.o
