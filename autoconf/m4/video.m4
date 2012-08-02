AC_DEFUN([SM_VIDEO], [

AC_REQUIRE([SM_STATIC])

AC_ARG_WITH(ffmpeg, AS_HELP_STRING([--without-ffmpeg],[Disable ffmpeg support]), with_ffmpeg=$withval, with_ffmpeg=yes)
AC_ARG_WITH(static-ffmpeg, AS_HELP_STRING([--with-static-ffmpeg],[Statically link ffmpeg libraries]), with_static_ffmpeg=$withval, with_static_ffmpeg=no)

VIDEO_CFLAGS=
VIDEO_LIBS=

if test "$with_static_ffmpeg" = "yes"; then
	LIB_PRE=$START_STATIC
	LIB_POST=$END_STATIC
fi

have_ffmpeg=no
if test "$with_ffmpeg" = "yes"; then
	if test "$with_static_ffmpeg" = "yes" -o "$with_static_ffmpeg" = "no"; then
		AC_CHECK_LIB(avutil, av_free, have_libavutil=yes, have_libavutil=no)
		AC_CHECK_LIB(avcodec, avcodec_register_all, have_libavcodec=yes,  have_libavcodec=no)
		AC_CHECK_LIB(avformat, av_guess_format, have_libavformat=yes,  have_libavformat=no)
		AC_CHECK_LIB(swscale, sws_scale, have_libswscale=yes,  have_libswscale=no)
		if test "$have_libavutil" = "yes" -a "$have_libavformat" = "yes" -a "$have_libavcodec" = "yes" -a "$have_libswscale" = "yes"; then
			have_ffmpeg=yes
			VIDEO_LIBS="$LIB_PRE -lswscale -lavformat -lswscale -lavcodec -lavutil $LIB_POST $VIDEO_LIBS"
			AC_DEFINE(HAVE_FFMPEG, 1, [FFMPEG support available])
		fi
	else
		if test "$(echo $with_static_ffmpeg | cut -c 1)" != "/"; then
			with_static_ffmpeg="$(/bin/pwd)/$with_static_ffmpeg"
		fi

		ffmpeg_save_CFLAGS="$CFLAGS"
		ffmpeg_save_CXXFLAGS="$CXXFLAGS"
		ffmpeg_save_LIBS="$LIBS"

		FFMPEG_CFLAGS="-I$with_static_ffmpeg/include"

		FFMPEG_LIBS="$with_static_ffmpeg/lib/libavutil.a $FFMPEG_LIBS"
		FFMPEG_LIBS="$with_static_ffmpeg/lib/libavcodec.a $FFMPEG_LIBS"
		FFMPEG_LIBS="$with_static_ffmpeg/lib/libavformat.a $FFMPEG_LIBS"
		FFMPEG_LIBS="$with_static_ffmpeg/lib/libswscale.a $FFMPEG_LIBS"

		CFLAGS="$FFMPEG_CFLAGS $CFLAGS"
		CXXFLAGS="$FFMPEG_CFLAGS $CXXFLAGS"
		LIBS="$FFMPEG_LIBS -lpthread $LIBS"

		AC_CHECK_FUNC([av_free], have_libavutil=yes, have_libavutil=no)
		AC_CHECK_FUNC([avcodec_register_all], have_libavcodec=yes, have_libavcodec=no)
		AC_CHECK_FUNC([av_guess_format], have_libavformat=yes, have_libavformat=no)
		AC_CHECK_FUNC([sws_scale], have_libswscale=yes, have_libswscale=no)

		if test "$have_libavutil" = "yes" -a "$have_libavformat" = "yes" -a "$have_libavcodec" = "yes" -a "$have_libswscale" = "yes"; then
			have_ffmpeg=yes
			AC_DEFINE(HAVE_FFMPEG, 1, [FFMPEG support available])
			VIDEO_CFLAGS="$FFMPEG_CFLAGS $VIDEO_CFLAGS"
			VIDEO_LIBS="$FFMPEG_LIBS $VIDEO_LIBS"
		fi

		CFLAGS="$ffmpeg_save_CFLAGS"
		CXXFLAGS="$ffmpeg_save_CXXFLAGS"
		LIBS="$ffmpeg_save_LIBS"
	fi
fi

LIB_PRE=
LIB_POST=

AM_CONDITIONAL(HAVE_FFMPEG, test "$have_ffmpeg" = "yes")

AC_SUBST(VIDEO_CFLAGS)
AC_SUBST(VIDEO_LIBS)

])
