CC ?= mpixlcxx
OPTS = -O3

all:a2a a2a-t2

a2a: a2a.C
	$(CC) $(OPTS) $(INCS_DEF) -o $@ $<

a2a-t2: a2a-t2.C
	$(CC) $(OPTS) $(INCS_DEF) -o $@ $<

clean:
	rm -f a2a a2a-t2
