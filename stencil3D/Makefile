CC?=mpixlcxx
OPTS=-O3

all:stencil

stencil: stencil.C
	$(CC) $(OPTS) $(INCS_DEF) -o $@ $<

clean:
	rm -f stencil charmrun
