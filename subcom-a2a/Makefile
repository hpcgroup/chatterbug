CC	= mpicxx
OPTS	= -O3 -DWRITE_OTF2_TRACE=1 -DSCOREP_USER_ENABLE
INCS	= -I${SCOREP_INSTALL}/include -I${SCOREP_INSTALL}/include/scorep

all: a2a-line

a2a-line: a2a-line.C
	$(CC) $(OPTS) $(INCS) -o $@ $<

subcomm-a2a.otf2: a2a-line.C
	scorep --user --nocompiler --noopenmp --nopomp --nocuda --noopenacc --noopencl --nomemory $(CC) $(OPTS) $(INCS) -o $@ $<

clean:
	rm -f a2a-line subcomm-a2a.otf2
