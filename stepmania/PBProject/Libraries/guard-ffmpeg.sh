#!/bin/sh

for i in ffmpeg/libavcodec/ppc/*.[hc] \
ffmpeg/libswscale/yuv2rgb_altivec.c; do
	sed -i '' -e '1i\
#ifdef __ppc__
$a\
#endif' $i
done
for i in ffmpeg/libavcodec/i386/*.[hc]; do
	sed -i '' -e '1i\
#ifdef __i386__
$a\
#endif' $i
done
