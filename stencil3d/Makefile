include ../Makefile.common

all:stencil3d$(SUFFIX) 

stencil3d$(SUFFIX) : stencil3d.C
	$(CXX) $(OPTS) $(INCS_DEF) -o $@ $< 

clean:
	rm -f stencil3d$(SUFFIX) 
