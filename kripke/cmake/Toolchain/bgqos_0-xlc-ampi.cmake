# NOTE: update AMPI_DIR to point to AMPI compilers
set(AMPI_DIR "/scratch/nikhil/charm-master/netlrts-linux-x86_64")

set(CMAKE_C_COMPILER "${AMPI_DIR}bin/ampicc")
set(CMAKE_CXX_COMPILER "${AMPI_DIR}bin/ampicxx")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -qarch=auto")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -qsimd=auto -qhot=novector -qnostrict -qreport -qsource -qlist -qlistfmt=html")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-zmuldefs")

set(PKG_PATH "/usr/gapps/bdiv/${SYS_TYPE}/opt")

