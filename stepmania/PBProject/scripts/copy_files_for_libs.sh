#!/bin/sh
# Copies the files from `files' into the directory `./local'.
# Use this with the PackageMaker file to create `StepMania Libs.pkg'.
# Keep in mind that you must change the Root directory under the files
# tab in `StepMania Libs.pmsp'.

CURR=`pwd`/$0
CURR=`dirname $CURR`
DEST=$CURR/..
FILES=`cat $CURR/files`

mkdir $DEST/local


for i in $FILES; do
    FIRST_CHAR=`echo $i|cut -c 1-1`
    if [ "x$FIRST_CHAR" != "x" -a "$FIRST_CHAR" != "#" ]; then
	cp -d --recursive -v --parents $i $DEST/local/
    fi
done