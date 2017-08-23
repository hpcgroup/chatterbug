include ../Makefile.common

all: permutation

permutation: permutation.C
	$(CC) $(OPTS) $(INCS) -o $@ $<

permutation.otf2: permutation.C
	scorep --user --nocompiler --noopenmp --nopomp --nocuda --noopenacc --noopencl --nomemory $(CC) $(OPTS) $(INCS) -o $@ $<

clean:
	rm -f permutation permutation.otf2
