CC ?= mpixlcxx
OPTS = -g

all:stencil4d

stencil4d: stencil4d.C
	$(CC) $(OPTS) $(INCS_DEF) -o $@ $<

clean:
	rm -f stencil4d
