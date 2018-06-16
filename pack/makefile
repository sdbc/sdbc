LIBDIR=$(SDBCDIR)/lib
INCDIR=$(SDBCDIR)/include
CFLAGS= -Wall -fPIC -I$(INCDIR) -O3
CC=cc

LIBOBJS= netvar.o mkvalues.o pack.o json_pack.o patt_copy.o colidx.o except.o SDBC_type.o \
	mkfield.o setbind.o SRM.o SRM_json.o SRM_utility.o mk_where.o tpl_lib.o jerr.o
$(LIBDIR)/libscbase.a:$(LIBOBJS)
	ar -rv $(LIBDIR)/libscbase.a $?
$(LIBOBJS):$(INCDIR)/pack.h $(INCDIR)/strproc.h
SRM.o SRM_utility.o:$(INCDIR)/SRM.h
SRM_json.o:$(INCDIR)/SRM_json.h $(INCDIR)/SRM.h
json_pack.o:$(INCDIR)/json_pack.h
