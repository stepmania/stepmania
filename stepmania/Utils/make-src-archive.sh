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

cp COPYING.txt NEWS README-FIRST.html Makefile.am aclocal.m4 \
   configure Makefile.in configure.ac   $PRODUCTVER

echo Pruning...
cd $PRODUCTVER/src
# Obsolete and going away:
rm -rf SDL-1.2.5
# Incomplete:
rm -rf Texture Font Generator
# Xbox:
rm -rf SDLx-0.02
# Unused:
rm -rf smlobby
rm -rf SDL_net-1.2.5

# Windows-only stuff.  Let's leave some in the archive to make the GPL happy.
# I don't want to spend an extra half hour to upload separate Windows and
# *nix source archives.
#rm -rf mad-0.15.0b
#rm -rf SDL_image-1.2

rm -rf vorbis
rm -rf BaseClasses
rm -rf ddk
rm -rf smpackage

find -type d -name 'CVS' | xargs rm -rf
find -type f -name '*.lib' | xargs rm -rf

cd ../..

echo Archiving...
tar zchvf "$PRODUCTVER".tar.gz $PRODUCTVER/

