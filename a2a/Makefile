include ../Makefile.common

all: a2a$(SUFFIX)

a2a$(SUFFIX): a2a.C
	$(CC) $(OPTS) $(INCS_DEF) -o $@ $<

clean:
	rm -f a2a$(SUFFIX)
