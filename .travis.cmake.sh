#!/bin/sh
set -e

# Re-setup the environment variables here.
# A new make is not required.
cd ~
export PATH=${PWD}/cmake/bin:${PATH}

# No matter where we are, ensure we are at the right spot.
cd ${TRAVIS_BUILD_DIR}

if [ "${COMPILER}" == "clang++" ]; then
  export DEPS_DIR="/home/travis/deps"
  echo ${DEPS_DIR}
  export CXXFLAGS="-I ${DEPS_DIR}/llvm/build/include/c++/v1"
  export LDFLAGS="-L ${DEPS_DIR}/llvm/build/lib -l c++ -l c++abi"
  export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${DEPS_DIR}/llvm/build/lib"
fi

pwd
echo $PATH

cd Build
~/deps/cmake/bin/cmake .. -DCMAKE_CXX_COMPILER=${COMPILER} -DCMAKE_BUILD_TYPE=${BUILD_TYPE}

