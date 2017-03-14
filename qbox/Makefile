CC	= mpicxx
OPTS	= -O3 -DWRITE_OTF2_TRACE=1 -DSCOREP_USER_ENABLE
INCS	= -I${SCOREP_INSTALL}/include -I${SCOREP_INSTALL}/include/scorep

all: qbox-mini

qbox-mini: qbox-mini.C
	$(CC) $(OPTS) $(INCS) -o $@ $<

qbox-mini.otf2: qbox-mini.C
	scorep --user --nocompiler --noopenmp --nopomp --nocuda --noopenacc --noopencl --nomemory $(CC) $(OPTS) $(INCS) -o $@ $<

clean:
	rm -f qbox-mini qbox-mini.otf2
