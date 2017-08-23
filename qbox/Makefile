include ../Makefile.common

all: qbox-mini

qbox-mini: qbox-mini.C
	$(CC) $(OPTS) $(INCS) -o $@ $<

qbox-mini.otf2: qbox-mini.C
	scorep --user --nocompiler --noopenmp --nopomp --nocuda --noopenacc --noopencl --nomemory $(CC) $(OPTS) $(INCS) -o $@ $<

clean:
	rm -f qbox-mini qbox-mini.otf2
