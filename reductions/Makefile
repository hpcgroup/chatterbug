include ../Makefile.common

all:reduction$(SUFFIX)

reduction$(SUFFIX) : reduction.C
	$(CC) $(OPTS) $(INCS_DEF) -o $@ $<

clean:
	rm -f reduction$(SUFFIX) 
