#!/bin/sh

cd ../src/SDL_sound-1.0.0 || exit 1

if test -f ./.libs/libSDL_sound.a; then
    LIB_EXISTS=true
fi

if test -f ./ogg.o -a -f SDL_sound.o -a -f wav.o -a -f audio_convert.o -a -f madlib.o; then
    FILES_EXIST=true
fi

if [ x$LIB_EXISTS = xtrue -a x$FILES_EXIST = xtrue ]; then
    echo Statically linked files exist.
    exit 0
fi

make distclean > /dev/null 2>&1

./configure     --enable-wav \
                --enable-madlib \
                --enable-ogg \
                --disable-aiff \
                --disable-au \
                --disable-voc \
                --disable-raw \
                --disable-shn \
                --disable-midi \
                --disable-flac \
                --disable-smpeg \
                --disable-mpglib \
                --disable-mikmod \
                --disable-modplug \
                --enable-static \
                --disable-shared && make || exit 2

echo Statically linked files compiled.
exit 0
