CC ?= mpixlcxx_r
OPTS = -O3

all:near-neighbor

near-neighbor: near-neighbor.C
	$(CC) $(OPTS) $(INCS) -o $@ $<

clean:
	rm -f near-neighbor
