CC ?= mpixlcxx
OPTS = -O3

all:stencil3d

stencil3d: stencil3d.C
	$(CC) $(OPTS) $(INCS_DEF) -o $@ $< 

clean:
	rm -f stencil3d
