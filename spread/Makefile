include ../Makefile.common

all: spread$(SUFFIX) 

spread$(SUFFIX) : spread.C
	$(CC) $(OPTS) $(INCS) -o $@ $<

clean:
	rm -f spread$(SUFFIX) 
