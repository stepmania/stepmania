#!/bin/sh

CURRENT=`pwd`/$0
CURRENT=`dirname $CURRENT`
SM=$CURRENT/../..
DEST=$SM/PBProject/StepMania
APP=$SM/PBProject/build/StepMania.app
DIRS=(Announcers/instructions.txt BGAnimations/instructions.txt \
	CDTitles/instructions.txt  Courses/instructions.txt Courses/Samples \
	NoteSkins/instructions.txt NoteSkins/dance/default NoteSkins/dance/flat \
	NoteSkins/dance/note NoteSkins/dance/solo NoteSkins/pump/Classic \
	NoteSkins/pump/default RandomMovies/instructions.txt \
	Songs/instructions.txt Themes/instructions.txt Themes/default \
	Visualizations/instructions.txt Data/Translation.dat Data/AI.ini \
	Data/VideoCardDefaults.ini COPYING.txt README-FIRST.html NEWS)
NUM=0
TOTAL=0
FIND=`which find`

#count number of files to install
for i in ${DIRS[*]}; do
    TOTAL=`expr $TOTAL + 1`
done

#goto the SM directory
cd $SM

echo "**********"
echo "Creating ${DEST}..."
echo "**********"
if [ `mkdir -p $DEST` ]; then
    echo "mkdir failed"
    exit 1
fi

echo "**********"
echo "Copying needed files and directories..."
echo "**********"
while [ $NUM -ne $TOTAL ]; do
    if [ `cp -Rf --parents ${DIRS[${NUM}]} $DEST` ]; then
	echo "cp failed to copy ${DIRS[${NUM}]}"
	exit 2
    fi
    
    TEMP=`expr \( $NUM + 1 \) '*' 100 / $TOTAL`
    echo "${TEMP}% complete."
    NUM=`expr $NUM + 1`
done

echo "**********"
echo "Removing CVS directories..."
echo "**********"
cd $DEST
if [ `$FIND . -name CVS -a -type d -a -print0|xargs -0 rm -rf` ]; then
    echo "Failed to remove all the CVS directories."
    exit 3
fi

echo "**********"
echo "Copying StepMania.app..."
echo "**********"
if [ `cp -R $APP $DEST` ]; then
    echo "Failed to copy $APP"
    exit 4
fi

echo "**********"
echo "Changing permissions..."
echo "**********"
if [ `chmod -R a=rwx $DEST` ]; then
    echo "Failed to change permissions."
    echo "Try as super user"
    if [ `sudo chmod -R a=rwx $DEST` ]; then
        echo "Failed again..."
    fi
fi
