# Makefile for ZipArchive library
# Copyright (C) 2000-2003 Tadeusz Dracz
# For conditions of distribution and use, see copyright notice in License.txt
# To install to /usr/lib and /usr/include, type:
#    make install
# to install to a different directory change prefix



CC=g++
CCC=cc

#FLAGSD = -g
FLAGSD =

ZIPARCHLIB = libziparch.a

ZIPPIELOCATION = stl/zippie/

prefix = /usr
libdir = ${prefix}/lib
includedir = ${prefix}/include
zipardir = $(includedir)/ziparchive

AR=ar rc
RANLIB=ranlib

.SUFFIXES: .c .cpp
.c.o:
	$(CCC) -c -I. -o $*.o $<
.cpp.o:
	$(CC) $(FLAGSD) -c -I. $<

OBJS = ZipArchive.o ZipAutoBuffer.o ZipCentralDir.o \
ZipCompatibility.o ZipException.o ZipFile.o ZipFileHeader.o \
ZipMemFile.o ZipPathComponent.o ZipPlatform.o ZipPlatformComm.o \
ZipStorage.o ZipString.o

#OBJSZLIB = zlib/adler32.o zlib/compress.o zlib/crc32.o zlib/uncompr.o zlib/deflate.o zlib/trees.o \
zlib/zutil.o zlib/inflate.o zlib/infblock.o zlib/inftrees.o zlib/infcodes.o zlib/infutil.o zlib/inffast.o

OBJSZLIB =

$(ZIPARCHLIB): $(OBJS) $(OBJSZLIB)
	$(AR) $@ $(OBJS) $(OBJSZLIB)
	-@ ($(RANLIB) $@ || true) >/dev/null 2>&1

ZipArchive.o: ZipArchive.h
ZipAutoBuffer.o: ZipAutoBuffer.h
ZipCentralDir.o: ZipCentralDir.h
ZipCompatibility.o: ZipCompatibility.h
ZipException.o: ZipException.h
ZipFile.o: ZipFile.h
ZipFileHeader.o: ZipFileHeader.h
ZipMemFile.o: ZipMemFile.h
ZipPathComponent.o: ZipPathComponent.h
ZipPlatform.o: ZipPlatform.h
ZipStorage.o: ZipStorage.h
ZipString.o: ZipString.h

clean:
	-rm -f *.o *~ $(ZIPARCHLIB)

zippie:

	$(CC) -I$(zipardir) $(FLAGSD) -o zippie $(ZIPPIELOCATION)zippie.cpp $(ZIPPIELOCATION)CmdLine.cpp -lz -lstdc++ -lziparch
#	$(CC) -I$(zipardir) $(FLAGSD) -o zippie ZIPPIELOCATION)zippie.cpp $(ZIPPIELOCATION)CmdLine.cpp -lstdc++ -lziparch
cleanzippie:
	-rm -f zippie

install:
	-@if [ ! -d $(includedir)  ]; then mkdir $(includedir); fi
	-@if [ ! -d $(libdir) ]; then mkdir $(libdir); fi
	-@if [ ! -d $(zipardir)  ]; then mkdir $(zipardir); fi
	cp libziparch.a $(libdir)
	chmod 755 $(libdir)/$(ZIPARCHLIB)
	cp *.h $(zipardir)
	chmod 644 $(zipardir)/*h

uninstall:
	rm -f $(zipardir)/*h
	rm -f $(libdir)/$(ZIPARCHLIB)
	rmdir $(zipardir)



