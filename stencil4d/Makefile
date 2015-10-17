CC ?= mpixlcxx
OPTS = -O3

all:stencil4d

stencil4d: stencil4d.C
	$(CC) $(OPTS) $(INCS_DEF) -o $@ $<

clean:
	rm -f a2a
