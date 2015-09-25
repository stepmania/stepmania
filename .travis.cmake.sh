#!/bin/sh
set -e

# No matter where we are, ensure we are at the right spot.
cd ${TRAVIS_BUILD_DIR}

if [ "${COMPILER}" == "clang++" ]; then
  export CXXFLAGS="-I ${DEPS_DIR}/llvm/build/include/c++/v1"
  export LDFLAGS="-L ${DEPS_DIR}/llvm/build/lib -l c++ -l c++abi"
  export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${DEPS_DIR}/llvm/build/lib"
fi

cd Build
cmake .. -DCMAKE_CXX_COMPILER=${COMPILER} -DCMAKE_BUILD_TYPE=${BUILD_TYPE}

