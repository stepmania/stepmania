#!/bin/sh

currdir=`pwd`

rm $currdir/build/Vorbis.framework/Versions/A/Frameworks
cd $currdir/build/Vorbis.framework/Versions/A && ln -s ../.. Frameworks
