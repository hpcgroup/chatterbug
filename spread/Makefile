include ../Makefile.common

all: spread$(SUFFIX) 

spread$(SUFFIX) : spread.C
	$(CXX) $(OPTS) $(INCS) -o $@ $<

clean:
	rm -f spread$(SUFFIX) 
