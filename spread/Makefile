CC ?= mpixlcxx_r
OPTS = -O3

all:spread 

spread: spread.C
	$(CC) $(OPTS) $(INCS) -o $@ $<

clean:
	rm -f spread
