
LIBDIR=$(SDBCDIR)/lib
INCDIR=$(SDBCDIR)/include
CFLAGS= -m64 -w -fPIC -I$(INCDIR) -I$(INCDIR)/ldap -g
CC=cc

LIBOBJS= envcfg.o isdir.o showlog.o ldap_auth.o openauth.o decode.o encryptproc.o 

$(LIBDIR)/libscbase.a:$(LIBOBJS)
	ar -rv $(LIBDIR)/libscbase.a $?
$(LIBOBJS):$(INCDIR)/strproc.h

