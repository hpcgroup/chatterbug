# NOTE: update AMPI_DIR to point to AMPI compilers
set(AMPI_DIR "/p/lscratchf/jain6/network-sim/charm/mpi-linux-x86_64-bigemulator-mpicxx")

set(CMAKE_C_COMPILER "${AMPI_DIR}/bin/ampicc")
set(CMAKE_CXX_COMPILER "${AMPI_DIR}/bin/ampicxx")
set(CMAKE_LINKER "${AMPI_DIR}/bin/ampicxx")

set(CMAKE_C_FLAGS "-mtune=native ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(PKG_PATH "/usr/local")


