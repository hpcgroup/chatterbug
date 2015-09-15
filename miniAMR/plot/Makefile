CC = pgcc
LD = pgcc
CFLAGS = -I/home/ctvaugh/Access/inc
LD_FLAGS = -L/home/ctvaugh/Access/lib -l exoIIv2c -l netcdf

EXEC = plot.x

OBJS = plot.o read.o sort.o write.o

$(EXEC): $(OBJS)
	$(LD) -o $@ $(OBJS) $(LD_FLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $<
