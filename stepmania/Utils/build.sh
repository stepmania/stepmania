#!/bin/sh

# Usage: ./build options
# where options are passed to stepmania's configure script

url=http://stepmania.sf.net
ffmpeg=ffmpeg-0.4.9-pre1
patchfile=ffmpeg-0.4.9-pre1-sm.patch
if test ! -d $ffmpeg; then
	if test ! -f $ffmpeg.tar.gz; then
		echo -n 'Downloading ffmpeg...'
		if which wget &>/dev/null; then
			dl=wget
		elif which curl &>/dev/null; then
			dl="curl -O -O"
		else
			echo 'failed.'
			exit 1
		fi
		$dl $url/$ffmpeg.tar.gz $url/$patchfile &>/dev/null  \
			|| { echo 'failed.'; exit; }
		echo 'okay.'
	fi
	echo -n 'Decompressing ffmpeg...'
	tar zxf $ffmpeg.tar.gz &>/dev/null || { echo 'failed.'; exit 1; }
	echo 'okay.'
	echo -n 'Patching ffmpeg...'
	patch -p0 < $patchfile &>/dev/null || { echo 'failed.'; exit 1; }
	echo 'okay.'
fi
if test ! -f $ffmpeg/_inst/lib/libavcodec.a; then
	cd $ffmpeg
	echo -n 'Configuring ffmpeg...'
	./configure --prefix="`pwd`/_inst" &>/dev/null \
		|| { echo 'failed.'; exit 1; }
	echo 'okay.'
	echo -n 'Building ffmpeg...'
	# Hack around broken configure script
	mkdir -p _inst/lib
	make installlib &>/dev/null || { echo 'failed.'; exit 1; }
	echo 'okay.'
	cd ..
fi
if test ! -f _build/src/stepmania; then
	echo -n 'Configuring StepMania...'
	mkdir -p _build
	cd _build
	../configure --with-static-ffmpeg-prefix=../$ffmpeg/_inst \
		$@ &>/dev/null || { echo 'failed.'; exit 1; }
	echo 'okay.'
	echo -n 'Building StepMania...'
	make &>/dev/null || { echo 'failed.'; exit 1; }
	echo 'okay.'
	cd ..
fi
if test ! -f stepmania -o ! -f GtkModule.so; then
	echo -n 'Copying StepMania binaries...'
	cp _build/src/stepmania _build/src/GtkModule.so . &>/dev/null \
		|| { echo 'failed.'; exit 1; }
	echo 'okay.'
fi
