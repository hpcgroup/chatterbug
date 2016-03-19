CC = mpicc
OPTS = -O3 

all: fftw

fftw: fftw.c
	$(CC) $(OPTS) $(INCS_DEF) -I${FFTW_INCLUDES} -o $@ $< -L${FFTW_LIBS} -lfftw3 

clean:
	rm -f fftw
