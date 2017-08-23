include ../Makefile.common

all: near-neighbor

near-neighbor: near-neighbor.C
	$(CC) $(OPTS) $(INCS) -o $@ $<

near-neighbor.otf2: near-neighbor.C
	scorep --user --nocompiler --noopenmp --nopomp --nocuda --noopenacc --noopencl --nomemory $(CC) $(OPTS) $(INCS) -o $@ $<

clean:
	rm -f near-neighbor near-neighbor.otf2
