CC ?= mpixlcxx
OPTS = -O3

all:qbox_mini 

qbox_mini: qbox_mini.C
	$(CC) $(OPTS) $(INCS_DEF) -o $@ $<

clean:
	rm -f qbox_mini 
