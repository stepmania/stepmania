#!/bin/sh
# Run this to generate configure, the initial makefiles, etc.
#
# Source (non-CVS) distributions should come with a "configure" script
# already generated--people simply compiling the program shouldn't
# need autoconf and friends installed.
#
# CVS only contains the source files: Makefile.am, configure.ac, and
# so on.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

PKG_NAME="stepmania"

# Actually, there's a chance we'll work with later versions, too, but it's
# not worth bending over backward to try to detect it right now.
ACLOCAL=aclocal-1.7
AUTOHEADER=autoheader
AUTOMAKE=automake-1.7
ACLOCAL_OPTIONS="-I autoconf/m4/"
AUTOMAKE_OPTIONS=-a

AUTOCONF=autoconf

(test -f $srcdir/configure.ac \
  && test -d $srcdir/src) || {
    echo -n "**Error**: Directory \"$srcdir\" does not look like the"
    echo " top-level $PKG_NAME directory"

    exit 1
}

DIE=0

($AUTOCONF --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: You must have \`autoconf' installed to compile $PKG_NAME."
  echo "Download the appropriate package for your distribution,"
  echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/autoconf/"
  DIE=1
}

($AUTOMAKE --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: You must have \`automake' installed to compile $PKG_NAME."
  echo "Get ftp://ftp.gnu.org/pub/gnu/automake/automake-1.7.2.tar.gz"
  echo "(or a newer version if it is available)"
  DIE=1
  NO_AUTOMAKE=yes
}


# if no automake, don't bother testing for aclocal
test -n "$NO_AUTOMAKE" || ($ACLOCAL --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: Missing \`aclocal'.  The version of \`automake'"
  echo "installed doesn't appear recent enough."
  echo "Get ftp://ftp.gnu.org/pub/gnu/automake/automake-1.7.2.tar.gz"
  echo "(or a newer version if it is available)"
  DIE=1
}

if test "$DIE" -eq 1; then
  exit 1
fi

$ACLOCAL $ACLOCAL_OPTIONS
$AUTOCONF
$AUTOHEADER
$AUTOMAKE $AUTOMAKE_OPTIONS

