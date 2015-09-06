#!/bin/sh
set -e
# See if the cmake folder is empty.
if [ ! -d "$HOME/cmake/lib" ]; then
	wget http://www.cmake.org/files/v3.3/cmake-3.3.1.tar.gz
	tar -xzf cmake-3.3.1.tar.gz
	cd cmake-3.3.1 && ./configure --prefix=$HOME/cmake && make && make install
else
	echo "Using cached directory."
fi

