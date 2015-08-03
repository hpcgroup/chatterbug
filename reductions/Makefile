CC ?= mpixlcxx
OPTS =-O3

all:reduction

reduction: reduction.C
	$(CC) $(OPTS) $(INCS_DEF) -o $@ $<

clean:
	rm -f reduction charmrun
