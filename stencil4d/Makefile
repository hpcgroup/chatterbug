include ../Makefile.common

all: stencil4d$(SUFFIX) 

stencil4d$(SUFFIX) : stencil4d.C
	$(CXX) $(OPTS) $(INCS_DEF) -o $@ $<

clean:
	rm -f stencil4d$(SUFFIX) 
