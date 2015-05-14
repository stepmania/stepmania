#!/bin/sh
set -ex
wget http://www.cmake.org/files/v3.2/cmake-3.2.2.tar.gz
tar -xzf cmake-3.2.2.tar.gz
cd cmake-3.2.2 && ./configure && make -j && sudo make install

