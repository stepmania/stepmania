#!/bin/bash
set -e

test -e ProductInfo.h && cd ..
if ! test -d src; then
	echo Run this script from the top directory.
	exit 1
fi

source Utils/GetProductVer.sh src/ProductInfo.h
PRODUCTVER="$PRODUCTVER-src"

if test -e $PRODUCTVER; then
	echo "\"$PRODUCTVER\" already exists."
	exit 1
fi
echo Copying...

mkdir $PRODUCTVER
cp -a autoconf $PRODUCTVER/
cp -a Utils $PRODUCTVER/
cp -a src $PRODUCTVER/

cp Docs/Copying.txt NEWS README-FIRST.html Makefile.am aclocal.m4 \
   configure Makefile.in configure.ac   $PRODUCTVER

mkdir $PRODUCTVER/Docs/
cp Docs/ChangeLog.txt $PRODUCTVER/Docs/

echo Pruning...
cd $PRODUCTVER/src
# Obsolete and going away:
rm -rf SDL-1.2.5
rm -rf SDL-1.2.6
# Incomplete:
rm -rf Texture Font Generator
# Xbox:
rm -rf SDLx-0.02
# Unused:
rm -rf smlobby

# Windows-only stuff.  Let's leave some in the archive to make the GPL happy.
# I don't want to spend an extra half hour to upload separate Windows and
# *nix source archives.
#rm -rf mad-0.15.0b

rm -rf lua-5.0
rm -rf vorbis
rm -rf BaseClasses
rm -rf ddk
rm -rf smpackage
rm -rf sdl_xbox_includes
rm -rf SDLx-0.02.rar

cd ..

find -type d -name 'CVS' | xargs rm -rf
find -type f -name '*.lib' | xargs rm -rf
find -type f -name '*.exe' | xargs rm -rf
find -type f -name '*.a' | xargs rm -rf
find -type f -name '*.o' | xargs rm -rf

cd ..
rm -rf Utils/Font\ generation/

echo Archiving...
tar zchvf "$PRODUCTVER".tar.gz $PRODUCTVER/

