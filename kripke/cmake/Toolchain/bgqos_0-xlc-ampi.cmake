# NOTE: update AMPI_DIR to point to AMPI compilers
set(AMPI_DIR "/home/njain/peac/network-sim/charm/pamilrts-bluegeneq-bigemulator")

set(CMAKE_C_COMPILER "${AMPI_DIR}/bin/ampicc")
set(CMAKE_CXX_COMPILER "${AMPI_DIR}/bin/ampicxx")
set(CMAKE_LINKER "${AMPI_DIR}/bin/ampicxx")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-zmuldefs")

#set(PKG_PATH "/usr/gapps/bdiv/${SYS_TYPE}/opt")

