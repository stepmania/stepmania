#!/bin/sh
set -e
if [ "$CXX" == "g++" ]; then
  export CC=/usr/bin/gcc-4.9
  export CXX=/usr/bin/g++-4.9
  export CXXFLAGS="--std=c++11"
fi

$(CXX) --version
cd Build
$HOME/cmake/bin/cmake -G 'Unix Makefiles' ..
make

