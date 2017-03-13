CC	= mpicxx
OPTS	= -O3 -DWRITE_OTF2_TRACE=1 -DSCOREP_USER_ENABLE
INCS	= -I${SCOREP_INSTALL}/include -I${SCOREP_INSTALL}/include/scorep

all: permutation

permutation: permutation.C
	$(CC) $(OPTS) $(INCS) -o $@ $<

permutation.otf2: permutation.C
	scorep --user --nocompiler --noopenmp --nopomp --nocuda --noopenacc --noopencl --nomemory $(CC) $(OPTS) $(INCS) -o $@ $<

clean:
	rm -f permutation permutation.otf2
