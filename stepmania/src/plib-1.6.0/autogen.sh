#!/bin/sh

OSTYPE=`uname -s`

AMFLAGS="--add-missing"
if test "$OSTYPE" = "IRIX" -o "$OSTYPE" = "IRIX64"; then
   AMFLAGS=$AMFLAGS" --include-deps";
fi

echo "Running aclocal"
aclocal
echo "Running automake"
automake $AMFLAGS
echo "Running autoconf"
autoconf

echo "======================================"
echo "Now you are ready to run './configure'"
echo "======================================"
