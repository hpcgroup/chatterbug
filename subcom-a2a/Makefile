CC ?= mpixlcxx
OPTS = -O3

all:a2a-line

a2a-line: a2a-line.C
	$(CC) $(OPTS) $(INCS_DEF) -o $@ $<

clean:
	rm -f a2a-line
