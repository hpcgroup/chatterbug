include ../Makefile.common

all: unstr-mesh$(SUFFIX)

unstr-mesh$(SUFFIX) : unstr-mesh.C
	$(CXX) $(OPTS) $(INCS) -o $@ $<

clean:
	rm -f unstr-mesh$(SUFFIX)
