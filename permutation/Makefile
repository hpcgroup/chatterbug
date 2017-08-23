include ../Makefile.common

all: permutation$(SUFFIX)

permutation$(SUFFIX): permutation.C
	$(CC) $(OPTS) $(INCS) -o $@ $<

clean:
	rm -f permutation$(SUFFIX)
