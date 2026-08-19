The binaries in this folder where built on arch linux with mingw.

# pacman -S mingw-w64-{binutils,crt,gcc,headers,winpthreads}
# pamac install mingw-w64-{bzip2,zlib,pkg-config}

./configure \
    --arch=x86_64 \
    --target-os=mingw32 \
    --cross-prefix=x86_64-w64-mingw32- \
    --disable-autodetect \
    --disable-avdevice \
    --disable-avfilter \
    --disable-debug \
    --disable-devices \
    --disable-doc \
    --disable-filters \
    --disable-lzma \
    --disable-network \
    --disable-postproc \
    --disable-programs \
    --disable-static \
    --disable-swresample \
    --enable-bzlib \
    --enable-d3d11va \
    --enable-dxva2 \
    --enable-gpl \
    --enable-shared \
    --enable-w32threads \
    --enable-zlib \
    --extra-cflags=-w \
    --extra-ldflags=-static \
    --prefix=/

./configure \
    --arch=x86 \
    --target-os=mingw32 \
    --cross-prefix=i686-w64-mingw32- \
    --disable-autodetect \
    --disable-avdevice \
    --disable-avfilter \
    --disable-debug \
    --disable-devices \
    --disable-doc \
    --disable-filters \
    --disable-lzma \
    --disable-network \
    --disable-postproc \
    --disable-programs \
    --disable-static \
    --disable-swresample \
    --enable-bzlib \
    --enable-d3d11va \
    --enable-dxva2 \
    --enable-gpl \
    --enable-shared \
    --enable-w32threads \
    --enable-zlib \
    --extra-cflags=-w \
    --extra-ldflags=-static \
    --prefix=/
