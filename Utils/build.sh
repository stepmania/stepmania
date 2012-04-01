#!/bin/bash

verbose=0
msg=
notify=0
s_configure=
s_stepmania=
s_data=
s_rebuild=
s_ffmpeg=
num_jobs=1


usage () {
	echo "Usage: $0 [OPTION] [-- [CONFIGURE_OPTIONS]]"
	echo 'Build StepMania.'
	echo "CONFIGURE_OPTIONS are passed to StepMania's configure."
	echo ''
	echo '  -c,  --configure            stop after configuring StepMania.'
	echo '  -s,  --stepmania            stop after building StepMania'
	echo '                              (do not copy binaries).'
	echo '  -d,  --data                 generate GameData.smzip.'
	echo '  -r,  --rebuild              rebuild StepMania (soft).'
	echo '  -f,  --ffmpeg               build with ffmpeg 0.10.2.'
	echo '  -j#, --jobs=#               pass -j# to make.'
	echo '  -h,  --help                 print this help and exit.'
	echo '  -v,  --verbose              increase verbosity (up to 2).'
	echo '       --version              print version and exit.'
	exit $1
}

if which notify-send  >/dev/null 2>&1; then
	#Comment the following line out to disable notify.
	notify=1
fi

if which getopt >/dev/null 2>&1; then
	set -- `getopt	-l configure,stepmania,data,rebuild,ffmpeg,help \
			-l jobs::,verbose,version -- drfcshj::v $*`
fi

while [ $# -gt 0 ]; do
	arg=$1
	shift
	case $arg in
		-c|--configure)	s_configure=yes		;;
		-s|--stepmania)	s_stepmania=yes		;;
		-d|--data)	s_data=yes		;;
		-r|--rebuild)	s_rebuild=yes		;;
		-f|--ffmpeg)	s_ffmpeg=yes		;;
		-h|--help)	usage 0			;;
		-j|--jobs)	num_jobs=$1; shift	;;
		-v|--verbose)	verbose=$[verbose+1]	;;
		--version)	version			;;
		--)		break			;;
		*)		usage 1			;;
	esac
done

message () {
	msg=$1
	if [ $verbose -gt 0 ]; then
		echo "$msg..."
	else
		echo -n "$msg..."
	fi
	if [ $notify -gt 0 ] && [ $verbose -eq 0 ]; then
		notify-send "StepMania build script" "$msg..."
	fi
}

success () {
	if [ $verbose -gt 0 ]; then
		echo "$msg...okay."
	else
		echo okay.
	fi
	if [ $notify -gt 0 ] && [ $verbose -eq 0 ]; then
		notify-send "StepMania build script" "$msg...okay."
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
		echo "Consider passing --verbose to $0. Pass --help for details."
	fi
	if [ $notify -gt 0 ] && [ $verbose -eq 0 ]; then
		notify-send "StepMania build script failed" "$msg...failed$error"
	fi;
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

version () {
	echo 'build.sh (StepMania) 2.64'
	echo 'Copyright (C) 2006-2009 Steve Checkoway'
	echo 'StepMania is Copyright (C) 2001-2012 Chris Danford et al.'
	echo 'sm-ssc is Copyright (C) 2009-2012 the spinal shark collective'
	exit 0
}

if [ -n "$s_data" ]; then
	message 'Generating GameData.smzip'
	if [ -f _build/Makefile ]; then
		cd _build
		call make SMData
		mv ./GameData.smzip ../
		cd ..
	else
		failure "You need to configure StepMania first."
	fi
	exit 0
fi

if [ -n "$s_ffmpeg" ]; then
	ffversion=0.10.2
	ffmpeg=ffmpeg-$ffversion
	if [ ! -d $ffmpeg ]; then
		message 'Downloading ffmpeg'
		if which bzip2 &>/dev/null; then
			zipcommand=jxf
			ffmarc=$ffmpeg.tar.bz2
		elif which gzip &>/dev/null; then
			zipcommand=zxf
			ffmarc=$ffmpeg.tar.gz   
        	else
			failure 'Install either bzip2 or gzip.' 
		fi
		if which wget &>/dev/null; then
			get=wget
		elif which curl &>/dev/null; then
			get='curl -O'
		else
			failure 'Install either curl or wget.'
		fi
		call $get "http://ffmpeg.org/releases/$ffmarc"
		message 'Extracting ffmpeg'
		call tar -$zipcommand $ffmarc
#		message 'Cleaning up temporary files'
#		call rm $ffmarc
	fi
	args="--enable-static --enable-gpl --enable-version3 --enable-nonfree --enable-libx264 --enable-libfaac --enable-libmp3lame --enable-libtheora --enable-libvorbis --disable-libvpx --disable-vaapi --enable-libxvid --disable-debug --enable-memalign-hack --disable-network --enable-small --disable-encoders --disable-ffserver --extra-cflags=-Dattribute_deprecated="
	cd $ffmpeg
	message 'Configuring ffmpeg'
	call ./configure --prefix="`pwd`/_inst" $args
	message 'Building ffmpeg'
	call make --jobs="$num_jobs" install-libs install-headers
	cd ..
fi

if [ ! -f _build/src/config.h ] || [ -n "$s_rebuild" ]; then
	message 'Configuring StepMania'
	mkdir -p _build
	cd _build
	if [ -n "$s_ffmpeg" ]; then
		call ../configure --with-ffmpeg --with-static-ffmpeg=../$ffmpeg/_inst "$@"
	else
		call ../configure "$@"
	fi
	cd ..
fi
if [ -n "$s_configure" ]; then exit 0; fi
if [ ! -f _build/src/stepmania ] || [ -n "$s_rebuild" ]; then
	message 'Building StepMania'
	cd _build
	call make --jobs="$num_jobs"
	cd ..
fi
if [ -n "$s_stepmania" ]; then exit 0; fi
if [ ! -f stepmania -o ! -f GtkModule.so ] || [ -n "$s_rebuild" ]; then
	message 'Copying StepMania binaries'
	cp _build/src/stepmania  .
	if [ -e _build/src/GtkModule.so ]; then
		cp _build/src/GtkModule.so . # we tend to not have this file!
	fi
	success
fi

# (c) 2006-2009 Steve Checkoway
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
