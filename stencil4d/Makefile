include ../Makefile.common

all: stencil4d

stencil4d: stencil4d.C
	$(CC) $(OPTS) $(INCS_DEF) -o $@ $<

stencil4d.otf2: stencil4d.C
	scorep --user --nocompiler --noopenmp --nopomp --nocuda --noopenacc --noopencl --nomemory $(CC) $(OPTS) $(INCS) -o $@ $<

clean:
	rm -f stencil4d stencil4d.otf2
