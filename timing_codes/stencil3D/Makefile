BGPHOME	= /bgsys/drivers/ppcfloor
CC	= mpicxx
CFLAGS	= -I$(BGPHOME)/comm/include -O3

TRACE_LIB = -L/home/ihchung/hpct_bgp/lib -lmpitrace -lgetarg -llicense
PREP	= $(HOME)/softwares/scalasca/bin/scalasca -instrument
SCALIB	= -L$(HOME)/softwares/scalasca/lib/be -lcube3

all: stencil3D 

stencil3D: stencil3D.C
	$(CC) -o stencil3D $< $(CFLAGS)

trace: stencil3D.C
	$(CC) -o stencil3D.trace $< $(CFLAGS) $(TRACE_LIB)

scal: stencil3D.C
	$(PREP) $(CC) -o stencil3D.scal $< $(CFLAGS) $(SCALIB)

clean:
	rm -f *.o stencil3D
