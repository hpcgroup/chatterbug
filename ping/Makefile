include ../Makefile.common

all:ping$(SUFFIX)

ping$(SUFFIX): ping.C
	$(CC) $(OPTS) $(INCS) -o $@ $< 

clean:
	rm -f ping$(SUFFIX)
