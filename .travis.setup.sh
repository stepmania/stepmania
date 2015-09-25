#!/bin/sh
set -e

# Setup a directory for dependencies. Remember the original location.
cd ~
export DEPS_DIR="${PWD}/deps"
mkdir ${DEPS_DIR} && cd ${DEPS_DIR}

# There's a binary for CMake available: don't need to make it. Just use it.
export CMAKE_URL=http://www.cmake.org/files/v3.3/cmake-3.3.1-Linux-x86_64.tar.gz
mkdir cmake
wget --quiet -O - ${CMAKE_URL} | tar --strip-components=1 -xz -C cmake
export PATH=${PWD}/cmake/bin:${PATH}

# Ensure libc++ and libc++abi are available for clang.
if [ "${COMPILER}" =~ clang\+\+-3\.6 ]; then
  export LLVM_URL="http://llvm.org/releases/3.6.1/llvm-3.6.1.src.tar.xz"
  export LIBCXX_URL="http://llvm.org/releases/3.6.1/libcxx-3.6.1.src.tar.xz"
  export LIBCXXABI_URL="http://llvm.org/releases/3.6.1/libcxxabi-3.6.1.src.tar.xz"
  export TAR_FMT="-xJ"

  mkdir -p llvm llvm/build llvm-projects/libcxx llvm/projects/libcxxabi
  wget --quiet -O - ${LLVM_URL} | tar --strip-components=1 ${TAR_FMT} -C llvm
  wget --quiet -O - ${LIBCXX_URL} | tar --strip-components=1 ${TAR_FMT} -C llvm/projects/libcxx
  wget --quiet -O - ${LIBCXXABI_URL} | tar --strip-components=1 ${TAR_FMT} -C llvm/projects/libcxxabi
  cd llvm/build
  cmake .. -DCMAKE_CXX_COMPILER=clang++
  make cxx -j2
  cd ../..
fi

cd ${TRAVIS_BUILD_DIR}

