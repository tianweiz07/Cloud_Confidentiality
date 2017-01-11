PROGRAMS=spy trojan shmget
SPYOBJS=spy.o
TROJANOBJS=trojan.o
SHMGETOBJS=shmget.o
OBJS=cachemap.o probe.o pageset.o cpuid.o comm.o clock.o analyse.o
ALLOBJS=${OBJS} ${SPYOBJS} ${TROJANOBJS} ${SHMGETOBJS} ${AOBJS}

#INCLUDES=-Ilibcpuid
#LDFLAGS=-Llibcpuid
#LDLIBS=-lm -lcpuid
INCLUDES=
LDFLAGS=
LDLIBS=-lm 
#CFLAGS=-O2 -g -std=gnu99
CFLAGS=${INCLUDES} -g -std=gnu99





all: ${PROGRAMS}

spy: ${SPYOBJS} ${OBJS}

trojan: ${TROJANOBJS} ${OBJS}

shmget: ${SHMGETOBJS} ${OBJS}




clean:
	rm -f ${ALLOBJS} ${PROGRAMS} *~ core.*
