MPI_DIR         = $(OPENMPI_ROOT)
NVCC            = nvcc
NVCC_FLAGS      = -I$(MPI_DIR)/include
CUDA_INCLUDE    = $(CUDA_HOME)/include
LIBS            = -L$(MPI_DIR)/lib -lmpi -L$(CUDA_HOME)/lib64 -lcudart -lnvToolsExt
PROFILE_FLAGS   = -lineinfo
SRCS            = saxpy-mpi-cuda.cu
OBJS            = saxpy-mpi-cuda.o
EXECUTABLE      = saxpy-mpi-cuda
ARCH            = #-arch=sm_80

all: $(EXECUTABLE)
$(EXECUTABLE): $(OBJS)
	$(NVCC) $(OBJS) -o $(EXECUTABLE) $(LIBS)

$(OBJS): $(SRCS)
	$(NVCC) $(NVCC_FLAGS) $(ARCH) $(PROFILE_FLAGS) -I$(CUDA_INCLUDE) -c $(SRCS) -o $(OBJS)

clean:
	rm -rf $(OBJS) $(EXECUTABLE)

