CC ?= mpixlcxx_r
OPTS = -O3

all:uniform

uniform: uniform.C
	$(CC) $(OPTS) $(INCS) -o $@ $< # -L/soft/perftools/HPM  -lmpitrace

clean:
	rm -f uniform
