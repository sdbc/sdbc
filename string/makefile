
LIBDIR=$(SDBCDIR)/lib
INCDIR=$(SDBCDIR)/include
CFLAGS= -Wall -m64 -fPIC -I$(INCDIR) -I/usr/local/include -O1 
CC=cc

LIBOBJS= basename.o  dateconvert.o  datejul.o  fround.o  skipblk.o  strproc.o  sysjul.o  trim.o \
		 strapp.o strrev.o strtcpy.o itoa.o trimall.o stctok.o GBK_flag.o

$(LIBDIR)/libscbase.a:$(LIBOBJS)
	ar -rv $(LIBDIR)/libscbase.a $?
$(LIBOBJS):$(INCDIR)/strproc.h
datejul.o dateconvert.o:$(INCDIR)/datejul.h

