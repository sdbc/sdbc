
LIBDIR=$(SDBCDIR)/lib
INCDIR=$(SDBCDIR)/include
CFLAGS= -m64 -Wall -fPIC -I$(INCDIR) -g
CC=gcc
all:$(LIBDIR)/libsc.a

	
LIBOBJS= chkexec.o echo.o filelist.o rexec.o  PutEvent.o return_error.o\
	get_srvname.o PPC_srv.o TPC_srv.o clikey.o set_showid.o Function.o \
		 tpool.o mod_sc.o  SIO_fd.o dmapp.o dm_app.o
# AIO_fd.o
aio:$(LIBDIR)/AIO_fd.o
$(LIBDIR)/AIO_fd.o:AIO_fd.c
	$(CC) $(CFLAGS) -c -o $@ $<


$(LIBDIR)/libsc.a:$(LIBOBJS)
	ar -rv $(LIBDIR)/libsc.a $?
$(LIBOBJS):$(INCDIR)/scsrv.h $(INCDIR)/sc.h
#linux 2.6.22 ртио

