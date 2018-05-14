include ../Makefile.common

all: subcom-a2a$(SUFFIX) 

subcom-a2a$(SUFFIX) : subcom-a2a.C
	$(CXX) $(OPTS) $(INCS) -o $@ $<

clean:
	rm -f subcom-a2a$(SUFFIX)
