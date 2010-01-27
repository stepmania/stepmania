This directory is only for Windows and Xbox.
For other OS's, install it yourself as usual.

Windows:
Compile with MingW.
Originally configured with:
./configure --enable-memalign-hack --enable-shared --enable-small --disable-debug

The last revision of ffmpeg used was r20782. (20091208)

the old instructions look like this:
svn co -r 8448 svn://svn.mplayerhq.hu/ffmpeg/trunk/ ffmpeg

and this is a nice big config file that I'm not sure will work.
./configure --enable-shared --disable-static --disable-debug --disable-vhook --enable-memalign-hack --disable-network  --enable-small \
  --disable-encoders --disable-ffmpeg --disable-ffserver --disable-ffplay --disable-muxers \
  --disable-demuxers --enable-demuxer=avi --enable-demuxer=h261 --enable-demuxer=h263 --enable-demuxer=h264 --enable-demuxer=m4v --enable-demuxer=mjpeg --enable-demuxer=mov --enable-demuxer=mpegps --enable-demuxer=mpegts --enable-demuxer=mpegvideo --enable-demuxer=ogg --enable-demuxer=rawvideo --enable-demuxer=yuv4mpegpipe \
  --disable-decoders   --enable-decoder=h261 --enable-decoder=h263 --enable-decoder=h263i --enable-decoder=h264 --enable-decoder=huffyuv --enable-decoder=mjpeg --enable-decoder=mjpegb --enable-decoder=mpeg_xvmc --enable-decoder=mpeg1video --enable-decoder=mpeg2video --enable-decoder=mpeg4 --enable-decoder=mpegvideo --enable-decoder=msmpeg4v1 --enable-decoder=msmpeg4v2 --enable-decoder=msmpeg4v3 --enable-decoder=rawvideo --enable-decoder=theora \
  --disable-parsers --enable-parser=h261 --enable-parser=h263 --enable-parser=h264 --enable-parser=mjpeg --enable-parser=mpeg4video --enable-parser=mpegaudio --enable-parser=mpegvideo --enable-parser=ac3 \
  --target-os=mingw32
================================================================================
Xbox:
You should probably compile with MingW.

arch/MovieTexture/MovieTexture_FFMpeg.cpp says:
/* NOTES: ffmpeg static libraries arent included in SVN. You have to build
 *them yourself or remove this file to produce the xbox build.
 * 
 * build ffmpeg with mingw32 ( howto http://ffmpeg.arrozcru.org/wiki/index.php?title=Main_Page )
 * ./configure --enable-memalign-hack --enable-static --disable-mmx --target-os=mingw32 --arch=x86
 * you can use various switches to enable/disable codecs/muxers/etc. 
 * 
 * libgcc.a and libmingwex.a comes from mingw installation
 * msys\mingw\lib\gcc\mingw32\3.4.5\libgcc.a */

note that the xbox version still doesn't exactly run yet.

During my latest run, I used
./configure --enable-memalign-hack --enable-static --disable-mmx --arch=x86 --enable-small --disable-debug
or something of the sort.

So for those confused, the .a files are the xbox stuff and the .lib files are the
windows stuff. yay microsoft |: -aj