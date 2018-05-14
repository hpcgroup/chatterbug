include ../Makefile.common

all:pingping$(SUFFIX)

pingping$(SUFFIX): pingping.C
	$(CXX) $(OPTS) $(INCS) -o $@ $< 

clean:
	rm -f pingping$(SUFFIX)
