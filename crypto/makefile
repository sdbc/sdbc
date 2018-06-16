
MDIR=$(SDBCDIR)
LIBDIR=$(MDIR)/lib
INCDIR=$(MDIR)/include
CFLAGS= -m64 -Wall -fPIC -I$(INCDIR) -g
CC=cc

LIBOBJS=  bignum.o crc32.o expmod.o fcrypt.o md5_dgst.o  \
	a64n.o crc.o family.o fcrypt_b.o md5_one.o mulmod.o set_key.o \
	enigma.o enigma2.o prikey128.o quicklz.o mac.o fingerprint.o

$(LIBDIR)/libscbase.a:$(LIBOBJS)
	ar -rv $(LIBDIR)/libscbase.a $?
enigma.o enigma2.o : $(INCDIR)/enigma.h
crc.o:$(INCDIR)/crc.h
crc32.o:$(INCDIR)/crc32.h

#enigmaº”√‹≤‚ ‘ 
teni: teni.o 
	$(CC) -m64 -o teni teni.o -L$(SDBCDIR)/lib -lscbase

tnum: tnum.o 
	$(CC) -m64 -o tnum tnum.o -L$(SDBCDIR)/lib -lsc -lsc
