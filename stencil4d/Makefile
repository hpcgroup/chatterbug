include ../Makefile.common

all: stencil4d$(SUFFIX) 

stencil4d$(SUFFIX) : stencil4d.C
	$(CC) $(OPTS) $(INCS_DEF) -o $@ $<

clean:
	rm -f stencil4d$(SUFFIX) 
