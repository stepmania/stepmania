#!/bin/sh
set -e
if [ "$CXX" == "g++" ]; then
	export ORIG_BUILD_DIR=$(pwd)
	wget https://github.com/Viq111/travis-container-packets/releases/download/gcc-4.9.2/gcc.tar.bz2
	tar -xjf gcc.tar.bz2
	rm gcc.tar.bz2
	export PATH=$(pwd)/gcc/bin:$PATH
	export LIBRARY_PATH=$(pwd)/gcc/lib64:$LIBRARY_PATH
	export LD_LIBRARY_PATH=$(pwd)/gcc/lib64:$LD_LIBRARY_PATH
	export CPLUS_INCLUDE_PATH=$(pwd)/gcc/include/c++/4.9.2:$CPLUS_INCLUDE_PATH
	export CXXFLAGS="--std=c++11"
	export CXX="g++-4.9.2"
	export CC="gcc-4.9.2"
	cd $ORIG_BUILD_DIR
fi

if [ "$CXX" == "clang++" ]; then
	export ORIG_BUILD_DIR=$(pwd)
	wget https://github.com/Viq111/travis-container-packets/releases/download/clang%2Blibcxx-3.5.1/clang_libcxx.tar.bz2
	tar -xjf clang_libcxx.tar.bz2
	rm clang_libcxx.tar.bz2
	mv clang_libcxx clang
	export PATH=$(pwd)/clang/bin:$PATH
	export LIBRARY_PATH=$(pwd)/clang/lib:$LIBRARY_PATH
	export LD_LIBRARY_PATH=$(pwd)/clang/lib:$LD_LIBRARY_PATH
	export CPLUS_INCLUDE_PATH=$(pwd)/clang/include/c++/v1:$CPLUS_INCLUDE_PATH
	export CXXFLAGS="-std=c++11 -stdlib=libc++"
	export CXX="clang++-3.5.1"
	export CC="clang-3.5.1"
	cd $ORIG_BUILD_DIR
fi

