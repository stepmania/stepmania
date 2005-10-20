AC_DEFUN([SM_VIDEO], [

AC_SEARCH_LIBS(avcodec_init, [avcodec], have_libavcodec=yes,  have_libavcodec=no)
AC_SEARCH_LIBS(guess_format, [avformat], have_libavformat=yes,  have_libavformat=no)

if test "$have_libavcodec" = "yes"; then
  AC_MSG_CHECKING([for libavcodec >= 0.4.9])
  AC_TRY_RUN([
      #include <ffmpeg/avcodec.h>
      int main()
      {
	      return ( LIBAVCODEC_VERSION_INT < 0x000409 )? 1:0;
      }
      ],,have_libavcodec=no,)
  AC_MSG_RESULT($have_libavcodec)
fi

if test "$have_libavformat" = "yes"; then
  AC_MSG_CHECKING([for libavformat >= 0.4.9])
  AC_TRY_RUN([
      #include <ffmpeg/avformat.h>
      int main()
      {
	      return ( LIBAVFORMAT_VERSION_INT < 0x000409 )? 1:0;
      }
      ],,have_libavformat=no,)
  AC_MSG_RESULT($have_libavformat)
fi

have_ffmpeg=no
if test "$have_libavformat" = "yes" -a "$have_libavcodec" = "yes"; then
	have_ffmpeg=yes
	AC_DEFINE(HAVE_FFMPEG, 1, [FMPEG support available])
fi
AM_CONDITIONAL(HAVE_FFMPEG, test "$have_ffmpeg" = "yes")

])
