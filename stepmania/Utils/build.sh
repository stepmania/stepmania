#!/bin/bash

verbose=0
msg=
message () {
	msg=$1
	if [ $verbose -gt 0 ]; then
		echo "$msg..."
	else
		echo -n "$msg..."
	fi
}

success () {
	if [ $verbose -gt 0 ]; then
		echo "$msg...okay."
	else
		echo okay.
	fi
}

failure () {
	if [ $verbose -gt 0 ]; then
		echo "$msg...failed."
	else
		echo failed.
	fi
	exit 1
}

call () {
	if [ $verbose -ge 2 ]; then
		echo $@
	fi
	if [ $verbose -gt 0 ]; then
		eval $@ || failure
	else
		eval $@ &>/dev/null || failure
	fi
	success
}

usage () {
	echo "Usage: $0 [OPTION] [-- [CONFIGURE_OPTIONS]]"
	echo 'Build StepMania with a statically linked ffmpeg.'
	echo "CONFIGURE_OPTIONS are passed to StepMania's configure."
	echo ''
	echo '  -d, --download             stop after ffmpeg download.'
	echo '  -f, --ffmpeg               stop after building ffmpeg.'
	echo '  -c, --configure            stop after configuring StepMania.'
	echo '  -s, --stepmania            stop after building StepMania'
	echo '                               (do not copy binaries).'
	echo '  -h, --help                 print this help and exit.'
	echo '  -v, --verbose              increase verbosity (up to 2).'
	echo '      --version              print version and exit.'
	exit $1
}

version () {
	echo 'build.sh (StepMania) 2.0'
	echo 'Copyright (C) 2006 Steve Checkoway'
	echo 'StepMania is Copyright (C) 2001-2006 Chris Danford et al.'
	exit 0
}

if which getopt &>/dev/null; then
	set -- `getopt	-l download,ffmpeg,configure,stepmania,help \
			-l verbose,version -- dfcshv $*`
fi

s_download=
s_ffmpeg=
s_configure=
s_stepmania=
while [ $# -gt 0 ]; do
	arg=$1
	shift
	case $arg in
		-d|--download)	s_download=yes		;;
		-f|--ffmpeg)	s_ffmpeg=yes		;;
		-c|--configure)	s_configure=yes		;;
		-s|--stepmania)	s_stepmania=yes		;;
		-h|--help)	usage 0			;;
		-v|--verbose)	verbose=$[verbose+1]	;;
		--version)	version			;;
		--)		break			;;
		*)		usage 1			;;
	esac
done
url=http://stepmania.sf.net
ffmpeg=ffmpeg-0.4.9-pre1
patchfile=ffmpeg-0.4.9-pre1-sm.patch
if [ ! -d $ffmpeg ]; then
	if [ ! -f $ffmpeg.tar.gz ]; then
		message 'Downloading ffmpeg'
		if which wget &>/dev/null; then
			dl=wget
		elif which curl &>/dev/null; then
			dl="curl -O -O"
		else
			failure
		fi
		call $dl $url/$ffmpeg.tar.gz $url/$patchfile
	fi
	message 'Decompressing ffmpeg'
	call tar zxf $ffmpeg.tar.gz
	message 'Patching ffmpeg'
	call patch -p0 "<$patchfile"
fi

if [ -n "$s_download" ]; then exit 0; fi
if [ ! -f $ffmpeg/_inst/lib/libavcodec.a ]; then
	cd $ffmpeg
	message 'Configuring ffmpeg'
	call ./configure --prefix="`pwd`/_inst"
	message 'Building ffmpeg'
	# Hack around broken configure script
	mkdir -p _inst/lib
	call make installlib
	cd ..
fi
if [ -n "$s_ffmpeg" ]; then exit 0; fi
if [ ! -f _build/src/config.h ]; then
	message 'Configuring StepMania'
	mkdir -p _build
	cd _build
	call ../configure --with-static-ffmpeg-prefix=../$ffmpeg/_inst $@
	cd ..
fi
if [ -n "$s_configure" ]; then exit 0; fi
if [ ! -f _build/src/stepmania ]; then
	message 'Building StepMania'
	cd _build || failure
	call make
	cd ..
fi
if [ -n "$s_stepmania" ]; then exit 0; fi
if [ ! -f stepmania -o ! -f GtkModule.so ]; then
	message 'Copying StepMania binaries'
	call cp _build/src/stepmania _build/src/GtkModule.so .
fi


# (c) 2006 Steve Checkoway
# All rights reserved.
# 
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, and/or sell copies of the Software, and to permit persons to
# whom the Software is furnished to do so, provided that the above
# copyright notice(s) and this permission notice appear in all copies of
# the Software and that both the above copyright notice(s) and this
# permission notice appear in supporting documentation.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
# THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
# INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
# OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
# OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.


