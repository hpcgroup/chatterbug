CC ?= mpixlcxx_r
OPTS = -O3

all:near-neighbor near-neighbor-t2

near-neighbor-t2: near-neighbor-t2.C
	$(CC) $(OPTS) $(INCS) -o $@ $<

near-neighbor: near-neighbor.C
	$(CC) $(OPTS) $(INCS) -o $@ $<

clean:
	rm -f near-neighbor
