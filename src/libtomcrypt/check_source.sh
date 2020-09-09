#!/bin/bash

# output version
bash printinfo.sh

make clean > /dev/null

echo "checking..."
./helper.pl --check-source --check-makefiles --check-defines|| exit 1

exit 0

# ref:         HEAD -> master, tag: v1.18.2
# git commit:  7e7eb695d581782f04b24dc444cbfde86af59853
# commit time: 2018-07-01 22:49:01 +0200
