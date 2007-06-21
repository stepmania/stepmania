#!/bin/sh

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
	if [ $# -gt 0 ]; then
		error=": $1"
	else
		error='.'
	fi
	if [ $verbose -gt 0 ]; then
		echo "$msg...failed$error"
	else
		echo failed$error
	fi
	echo "Consider passing --verbose to $0. Pass --help for details."
	exit 1
}

call () {
	if [ $verbose -ge 2 ]; then
		echo "$@"
	fi
	if [ $verbose -gt 0 ]; then
		eval "$@" || failure
	else
		eval "$@" >/dev/null 2>&1 || failure
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
	echo 'build.sh (StepMania) 2.5'
	echo 'Copyright (C) 2006-2007 Steve Checkoway'
	echo 'StepMania is Copyright (C) 2001-2007 Chris Danford et al.'
	exit 0
}

if ! test -x configure; then
	msg='Checking for configure'
	verbose=1
	failure 'file not found.'
fi

if which getopt >/dev/null 2>&1; then
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
revision=8448
ffmpeg=ffmpeg-r$revision
repository=svn://svn.mplayerhq.hu/ffmpeg/trunk/
if [ ! -d $ffmpeg ]; then
	message 'Downloading ffmpeg'
	if which svn &>/dev/null; then
		call svn co -r $revision $repository $ffmpeg
	else
		failure 'Install subversion.'
	fi
fi

if [ -n "$s_download" ]; then exit 0; fi
args='--disable-shared --enable-static --disable-debug --disable-vhook
--enable-memalign-hack --disable-network --enable-small
--disable-encoders --disable-ffmpeg --disable-ffserver
--disable-ffplay --disable-muxers --enable-demuxer=avi
--enable-demuxer=h261 --enable-demuxer=h263 --enable-demuxer=h264
--enable-demuxer=m4v --enable-demuxer=mjpeg --enable-demuxer=mov
--enable-demuxer=mpegps --enable-demuxer=mpegts
--enable-demuxer=mpegvideo --enable-demuxer=ogg
--enable-demuxer=rawvideo --enable-demuxer=yuv4mpegpipe
--disable-decoders   --enable-decoder=h261 --enable-decoder=h263
--enable-decoder=h263i --enable-decoder=h264 --enable-decoder=huffyuv
--enable-decoder=mjpeg --enable-decoder=mjpegb
--enable-decoder=mpeg_xvmc --enable-decoder=mpeg1video
--enable-decoder=mpeg2video --enable-decoder=mpeg4
--enable-decoder=mpegvideo --enable-decoder=msmpeg4v1
--enable-decoder=msmpeg4v2 --enable-decoder=msmpeg4v3
--enable-decoder=rawvideo --enable-decoder=theora --disable-parsers
--enable-parser=h261 --enable-parser=h263 --enable-parser=h264
--enable-parser=mjpeg --enable-parser=mpeg4video
--enable-parser=mpegaudio --enable-parser=mpegvideo
--enable-parser=ac3'
if [ ! -f $ffmpeg/_inst/lib/libavcodec.a ]; then
	cd $ffmpeg
	message 'Configuring ffmpeg'
	call ./configure --prefix="`pwd`/_inst" $args
	message 'Building ffmpeg'
	call make install-libs install-headers
	cd ..
fi
if [ -n "$s_ffmpeg" ]; then exit 0; fi
if [ ! -f _build/src/config.h ]; then
	message 'Configuring StepMania'
	mkdir -p _build
	cd _build
	call ../configure --with-ffmpeg=../$ffmpeg/_inst "$@"
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


# (c) 2006-2007 Steve Checkoway
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


