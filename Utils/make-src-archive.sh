#!/bin/sh

LINUX= MAC= WIN=

# Parse options
while :; do
	case "$1" in
		-m) MAC=1; shift;;
		-w) WIN=1; shift;;
		*) break;;
	esac
done

# Usage message
if [ -z "$1" ]; then
	echo "\
Usage: $0 [-m] [-w] REV

Generates a source tarball at \`dist/stepmania-TAG.tar.bz2'.
  -m  Include Mac-specific build components
  -w  Include Windows-specific build components\
REV can be a Mercurial revision or tag
" 1>&2
	exit 1
fi

# Get confirmation
echo "WARNING!  This will do a clean update of the current repository." 1>&2
echo "If you have any uncommitted changes or extra files in this repository," 1>&2
echo "they will be LOST!" 1>&2
echo
read -sn1 -p "Do you want to continue [y/N]? " ISOK
if [ "$ISOK" != y -a "$ISOK" != Y ]; then
	echo n
	exit 1
fi
echo y

# Clean the source tree
hg update -C "$1"
hg clean --all

# This replaces autogen.sh.  Run now so our users don't need to have autoconf
# and friends installed.
#
# Note: the -f flag is here so any symlinks in autoconf/ get replaced with real
# files.
autoreconf -if

# Tarball will be placed here
mkdir dist

# List excludes for tar's -X flag
{
	echo './dist'
	echo '*/.hg*'
	echo './autoconf/m4'
	echo './autom4te.cache'
	echo './autogen.sh'
	echo './aclocal.m4'
	echo '*/*.a[mc]'
	echo './_assets'
	echo './Utils/Font generation'
	# OSX
	if [ -z "$MAC" ]; then
		echo './Xcode'
	fi
	# Windows/misc
	if [ -z "$WIN" ]; then
		echo './Program'
		echo './Installer'
		echo './sm-ssc.nsi'
		echo './Utils/doxygen'
		echo './Utils/Graphviz'
		echo './Utils/NSIS'
		echo './src/BaseClasses'
		echo './src/Texture Font Generator'
		echo './src/smpackage*'
		echo '*.[ao]'
		echo '*.lib'
		echo '*.dll'
		echo '*.exe'
		echo '*.bat'
	fi
} | tar cvjf "dist/stepmania-$1-src.tar.bz2" -X- . --transform "s,^\.,stepmania-$1,"
