AC_DEFUN([SM_VIDEO], [

AC_REQUIRE([SM_STATIC])

AC_ARG_WITH(ffmpeg, AS_HELP_STRING([--without-ffmpeg],[Disable ffmpeg support]), with_ffmpeg=$withval, with_ffmpeg=yes)
AC_ARG_WITH(static-ffmpeg, AS_HELP_STRING([--with-static-ffmpeg],[Statically link ffmpeg libraries]), with_static_ffmpeg=$withval, with_static_ffmpeg=no)

if test "$with_static_ffmpeg" = "yes"; then
	LIB_PRE=$START_STATIC
	LIB_POST=$END_STATIC
fi

have_ffmpeg=no
if test "$with_ffmpeg" = "yes"; then
	AC_CHECK_LIB(avutil, av_free, have_libavutil=yes, have_libavutil=no)
	AC_CHECK_LIB(avformat, av_guess_format, have_libavformat=yes,  have_libavformat=no)
	AC_CHECK_LIB(avcodec, avcodec_init, have_libavcodec=yes,  have_libavcodec=no)
	AC_CHECK_LIB(swscale, sws_scale, have_libswscale=yes,  have_libswscale=no)
	if test "$have_libavutil" = "yes" -a "$have_libavformat" = "yes" -a "$have_libavcodec" = "yes" -a "$have_libswscale" = "yes"; then
		have_ffmpeg=yes
		LIBS="$LIBS $LIB_PRE -lavutil -lavcodec -lavformat -lswscale $LIB_POST"
		AC_DEFINE(HAVE_FFMPEG, 1, [FFMPEG support available])
	fi
fi

LIB_PRE=
LIB_POST=

AM_CONDITIONAL(HAVE_FFMPEG, test "$have_ffmpeg" = "yes")

])
