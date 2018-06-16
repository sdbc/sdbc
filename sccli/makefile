
LIBDIR=$(SDBCDIR)/lib
INCDIR=$(SDBCDIR)/include
CFLAGS= -m64 -Wall -fPIC -I$(INCDIR) -O3
CC=gcc

LIBOBJS= client.o getls.o sqlc.o sqlinit.o getsvc.o netconnect.o get_clikey.o scpool.o \
	N_get_tpl.o

$(LIBDIR)/libsc.a:$(LIBOBJS)
	ar -rv $(LIBDIR)/libsc.a $?
$(LIBOBJS):$(INCDIR)/sc.h $(INCDIR)/strproc.h $(INCDIR)/strproc.h

scpool.o:scpool.c
	$(CC) -std=c99 $(CFLAGS) -c -o scpool.o scpool.c

