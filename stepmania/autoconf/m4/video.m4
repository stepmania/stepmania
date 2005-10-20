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



AC_ARG_WITH(theora, AC_HELP_STRING([--without-theora], [Disable Theora support]), with_theora=$enableval, with_theora=yes)

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
