CC ?= mpixlcxx_r
OPTS = -O3 -g

all:permutation

permutation: permutation.C
	$(CC) $(OPTS) $(INCS) -o $@ $< # -L/soft/perftools/HPM  -lmpitrace

clean:
	rm -f permutation
