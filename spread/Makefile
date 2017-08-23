include ../Makefile.common

all: spread

spread: spread.C
	$(CC) $(OPTS) $(INCS) -o $@ $<

spread.otf2: spread.C
	scorep --user --nocompiler --noopenmp --nopomp --nocuda --noopenacc --noopencl --nomemory $(CC) $(OPTS) $(INCS) -o $@ $<

clean:
	rm -f spread spread.otf2
