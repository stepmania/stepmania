This directory is only for Windows.  For other OS's, install it yourself
as usual.

Compile with MingW.
Configured with ./configure --enable-shared --enable-mingw32 --enable-small --disable-debug

svn co -r 8448 svn://svn.mplayerhq.hu/ffmpeg

./configure --enable-shared --disable-static --disable-debug --disable-vhook --enable-memalign-hack --disable-network  --enable-small \
  --disable-encoders --disable-ffmpeg --disable-ffserver --disable-ffplay --disable-muxers \
  --disable-demuxers --enable-demuxer=avi --enable-demuxer=h261 --enable-demuxer=h263 --enable-demuxer=h264 --enable-demuxer=m4v --enable-demuxer=mjpeg --enable-demuxer=mov --enable-demuxer=mpegps --enable-demuxer=mpegts --enable-demuxer=mpegvideo --enable-demuxer=ogg --enable-demuxer=rawvideo --enable-demuxer=yuv4mpegpipe \
  --disable-decoders   --enable-decoder=h261 --enable-decoder=h263 --enable-decoder=h263i --enable-decoder=h264 --enable-decoder=huffyuv --enable-decoder=mjpeg --enable-decoder=mjpegb --enable-decoder=mpeg_xvmc --enable-decoder=mpeg1video --enable-decoder=mpeg2video --enable-decoder=mpeg4 --enable-decoder=mpegvideo --enable-decoder=msmpeg4v1 --enable-decoder=msmpeg4v2 --enable-decoder=msmpeg4v3 --enable-decoder=rawvideo --enable-decoder=theora \
  --disable-parsers --enable-parser=h261 --enable-parser=h263 --enable-parser=h264 --enable-parser=mjpeg --enable-parser=mpeg4video --enable-parser=mpegaudio --enable-parser=mpegvideo --enable-parser=ac3 \
  --target-os=mingw32

