include ../Makefile.common

all: a2a-onedir$(SUFFIX)  a2a-twodirs$(SUFFIX) 

a2a-onedir$(SUFFIX) : a2a-onedir.C
	$(CC) $(OPTS) $(INCS) -o $@ $<

a2a-twodirs$(SUFFIX) : a2a-twodirs.C
	$(CC) $(OPTS) $(INCS) -o $@ $<

clean:
	rm -f a2a-onedir$(SUFFIX)  a2a-twodirs$(SUFFIX)
