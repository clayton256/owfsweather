DEPEND = $(CC)
DEPENDFLAGS = -M $(CFLAGS)
#DEBUG_FLAGS = -D APP_DEBUG_FLAG
CFLAGS=-ggdb3 -pthread -Wall $(DEBUG_FLAGS)
PROGRAM=owfsweather
#OBJS=main.o config.o misc.o ws603.o logfile.o ds1820.o updatewu.o barometer.o humidity.o rain.o aag.o updatejmc.o daemonize.o
SRCS=main.c config.c misc.c ws603.c logfile.c ds1820.c updatewu.c barometer.c humidity.c rain.c aag.c updatejmc.c daemonize.c watchdog.c
OBJS = $(SRCS:.c=.o)

LIBS=-lconfuse -lpthread -lcurl -lbsd




all:	$(PROGRAM)

#.c.o: owfsweather.h
%.o: %.c owfsweather.h
	$(CC) $(CFLAGS) -c $<


$(PROGRAM):	$(OBJS)
	$(CC) $(CFLAGS) -o $(PROGRAM) $(OBJS) $(LIBS)


depend:
	$(DEPEND) $(DEPENDFLAGS) $(SRCS) > .depend


clean:
	$(RM) -f $(OBJS) $(PROGRAM) *~ *.core core .depend

distclean:	clean
	$(RM) -f config.cache autom4te.cache autoscan.log config.h config.status config.log




