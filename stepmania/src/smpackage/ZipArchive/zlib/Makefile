# Makefile for zlib
# Copyright (C) 1995-1998 Jean-loup Gailly.
# For conditions of distribution and use, see copyright notice in zlib.h
# To install /usr/lib/libz.* and /usr/include/zlib.h, type:
#    make install


CC=cc

CFLAGS=-O
#CFLAGS=-O -DMAX_WBITS=14 -DMAX_MEM_LEVEL=7
#CFLAGS=-g -DDEBUG
#CFLAGS=-O3 -Wall -Wwrite-strings -Wpointer-arith -Wconversion \
#           -Wstrict-prototypes -Wmissing-prototypes

CPP=$(CC) -E

VER=1.1.3
LIBS=libz.a

AR=ar rc
RANLIB=ranlib

prefix = /usr
exec_prefix = ${prefix}
libdir = ${exec_prefix}/lib
includedir = ${prefix}/include

OBJS = adler32.o compress.o crc32.o uncompr.o deflate.o trees.o \
       zutil.o inflate.o infblock.o inftrees.o infcodes.o infutil.o inffast.o

OBJA =
# to use the asm code: make OBJA=match.o

libz.a: $(OBJS) $(OBJA)
	$(AR) $@ $(OBJS) $(OBJA)
	-@ ($(RANLIB) $@ || true) >/dev/null 2>&1


install: $(LIBS)
	-@if [ ! -d $(includedir)  ]; then mkdir $(includedir); fi
	-@if [ ! -d $(libdir) ]; then mkdir $(libdir); fi
	cp zlib.h zconf.h $(includedir)
	chmod 644 $(includedir)/zlib.h $(includedir)/zconf.h
	cp $(LIBS) $(libdir)
	cd $(libdir); chmod 755 $(LIBS)
	-@(cd $(libdir); $(RANLIB) libz.a || true) >/dev/null 2>&1

# The ranlib in install is needed on NeXTSTEP which checks file times
# ldconfig is for Linux

uninstall:
	cd $(includedir); \
	v=$(VER); \
	if test -f zlib.h; then \
	  v=`sed -n '/VERSION "/s/.*"\(.*\)".*/\1/p' < zlib.h`; \
          rm -f zlib.h zconf.h; \
	fi; \
	cd $(libdir); rm -f libz.a; \

clean:
	rm -f *.o *~ libz.a
# DO NOT DELETE THIS LINE -- make depend depends on it.

adler32.o: zlib.h zconf.h
compress.o: zlib.h zconf.h
crc32.o: zlib.h zconf.h
deflate.o: deflate.h zutil.h zlib.h zconf.h
infblock.o: infblock.h inftrees.h infcodes.h infutil.h zutil.h zlib.h zconf.h
infcodes.o: zutil.h zlib.h zconf.h
infcodes.o: inftrees.h infblock.h infcodes.h infutil.h inffast.h
inffast.o: zutil.h zlib.h zconf.h inftrees.h
inffast.o: infblock.h infcodes.h infutil.h inffast.h
inflate.o: zutil.h zlib.h zconf.h infblock.h
inftrees.o: zutil.h zlib.h zconf.h inftrees.h
infutil.o: zutil.h zlib.h zconf.h infblock.h inftrees.h infcodes.h infutil.h
trees.o: deflate.h zutil.h zlib.h zconf.h trees.h
uncompr.o: zlib.h zconf.h
zutil.o: zutil.h zlib.h zconf.h
