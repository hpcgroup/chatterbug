include ../Makefile.common

all: pairs$(SUFFIX)

pairs$(SUFFIX): pairs.C
	$(CXX) $(OPTS) $(INCS) -o $@ $<

clean:
	rm -f pairs$(SUFFIX)
