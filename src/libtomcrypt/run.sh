#!/bin/bash

# output version
bash printinfo.sh

bash build.sh " $1" "$2 -O2" "$3 IGNORE_SPEED=1" "$4" "$5"
if [ -a testok.txt ] && [ -f testok.txt ]; then
   echo
else
   echo
   echo "Test failed"
   exit 1
fi

rm -f testok.txt
bash build.sh " $1" "$2 -Os" "$3 IGNORE_SPEED=1 LTC_SMALL=1" "$4" "$5"
if [ -a testok.txt ] && [ -f testok.txt ]; then
   echo
else
   echo
   echo "Test failed"
   exit 1
fi

rm -f testok.txt
bash build.sh " $1" "$2" "$3 LTC_DEBUG=1" "$4" "$5"
if [ -a testok.txt ] && [ -f testok.txt ]; then
   echo
else
   echo
   echo "Test failed"
   exit 1
fi

rm -f testok.txt
bash build.sh " $1" "$2" "$3" "$4" "$5"
if [ -a testok.txt ] && [ -f testok.txt ]; then
   echo
else
   echo
   echo "Test failed"
   exit 1
fi

exit 0

# ref:         HEAD -> master, tag: v1.18.2
# git commit:  7e7eb695d581782f04b24dc444cbfde86af59853
# commit time: 2018-07-01 22:49:01 +0200
