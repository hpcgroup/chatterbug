include ../Makefile.common

all: qbox-mini$(SUFFIX)

qbox-mini$(SUFFIX): qbox-mini.C
	$(CC) $(OPTS) $(INCS) -o $@ $<

clean:
	rm -f qbox-mini$(SUFFIX)
