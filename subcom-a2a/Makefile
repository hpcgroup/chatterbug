include ../Makefile.common

all: a2a-onedir a2a-twodirs

a2a-onedir: a2a-onedir.C
	$(CC) $(OPTS) $(INCS) -o $@ $<

a2a-twodirs: a2a-twodirs.C
	$(CC) $(OPTS) $(INCS) -o $@ $<

a2a-onedir.otf2: a2a-onedir.C
	scorep --user --nocompiler --noopenmp --nopomp --nocuda --noopenacc --noopencl --nomemory $(CC) $(OPTS) $(INCS) -o $@ $<

a2a-twodirs.otf2: a2a-twodirs.C
	scorep --user --nocompiler --noopenmp --nopomp --nocuda --noopenacc --noopencl --nomemory $(CC) $(OPTS) $(INCS) -o $@ $<

clean:
	rm -f a2a-onedir a2a-twodirs a2a-onedir.otf2 a2a-twodirs.otf2
