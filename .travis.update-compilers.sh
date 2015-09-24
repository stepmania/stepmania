#!/bin/sh
set -e
if [ "$CXX" == "g++" ]; then
	export CC="gcc-4.8"
	export CXX="g++-4.8"
	export CXXFLAGS="--std=c++11"
	echo "CXX flags: $CXXFLAGS"
fi

if [ "$CXX" == "clang++" ]; then
	export ORIG_BUILD_DIR=$(pwd)
	cd ~
	wget https://github.com/Viq111/travis-container-packets/releases/download/clang%2Blibcxx-3.5.1/clang_libcxx.tar.bz2
	tar -xjf clang_libcxx.tar.bz2
	rm clang_libcxx.tar.bz2
	mv clang_libcxx clang
	export PATH=$(pwd)/clang/bin:$PATH
	export LIBRARY_PATH=$(pwd)/clang/lib:$LIBRARY_PATH
	export LD_LIBRARY_PATH=$(pwd)/clang/lib:$LD_LIBRARY_PATH
	export CPLUS_INCLUDE_PATH=$(pwd)/clang/include/c++/v1:$CPLUS_INCLUDE_PATH
	export CXXFLAGS="-std=c++11 -stdlib=libc++"
	cd $ORIG_BUILD_DIR
	echo "New path for clang: $PATH"
	echo "New library path for clang: $LIBRARY_PATH"
	echo "CXX flags: $CXXFLAGS"
fi

