include ../Makefile.common

all: near-neighbor$(SUFFIX)

near-neighbor$(SUFFIX) : near-neighbor.C
	$(CC) $(OPTS) $(INCS) -o $@ $<

clean:
	rm -f near-neighbor$(SUFFIX) 
