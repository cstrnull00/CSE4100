CC = gcc
CFLAGS=-O2 -Wall
LDLIBS = -lpthread

all: multiclient stockclient stockserver

multiclient: multiclient.c csapp.c csapp.h
stockclient: stockclient.c csapp.c csapp.h
stockserver: stockserver.c trade.c csapp.c csapp.h sbuf.c sbuf.h

clean:
	rm -rf *~ multiclient stockclient stockserver *.o
