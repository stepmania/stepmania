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



AC_ARG_WITH(theora, AS_HELP_STRING([--without-theora],[Disable Theora support]), with_theora=$withval, with_theora=yes)

have_vorbis=no
if test "$with_theora" = "yes"; then
	AC_CHECK_LIB(ogg, ogg_stream_init, have_libogg=yes, have_libogg=no)
	AC_CHECK_LIB(theora, theora_decode_init, have_libtheora=yes, have_libtheora=no, [-logg])
	if test "$have_libtheora" = "yes" -a "$have_libogg" = "yes" -a "$have_ffmpeg" = "yes"; then
		have_theora=yes
		LIBS="$LIBS -ltheora -logg"
		AC_DEFINE(HAVE_THEORA, 1, [Theora support available])
	else
		echo Not all theora libraries found.

		# Special note for unintuitive requirement (for color space conversion).
		if test "$have_libtheora" = "yes" -a "$have_libogg" = "yes"; then
			echo Ogg Theora support requires ffmpeg.
		fi
	fi

fi

AM_CONDITIONAL(HAVE_THEORA, test "$have_theora" = "yes" )

])
