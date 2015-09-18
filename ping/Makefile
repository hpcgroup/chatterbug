CC ?= mpixlcxx_r
OPTS = -O3

all:ping

ping: ping.C
	$(CC) $(OPTS) $(INCS) -o $@ $< #-L/soft/perftools/HPM  -lmpitrace

clean:
	rm -f ping charmrun
