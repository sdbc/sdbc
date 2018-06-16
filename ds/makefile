
LIBDIR=$(SDBCDIR)/lib
INCDIR=$(SDBCDIR)/include
CFLAGS= -Wall -fPIC -I$(INCDIR) -g
CC=gcc

LIBOBJS= getdw.o  tree.o tree_del.o  multi_hash.o Binary_search.o bound.o

$(LIBDIR)/libscbase.a:$(LIBOBJS)
	ar -rv $(LIBDIR)/libscbase.a $?
getdw.o tree.o tree_del.o:$(INCDIR)/BB_tree.h
multi_hash.o:$(INCDIR)/multi_hash.h
Binary_search.o bound.o:$(INCDIR)/Binary_search.h

ttree:ttree.o 
	$(CC) -ottree ttree.o -L$(LIBDIR) -lscbase
tbound:tbound.o 
	$(CC) -otbound tbound.o -L$(LIBDIR) -lscbase
