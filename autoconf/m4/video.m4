AC_DEFUN([SM_VIDEO], [

AC_REQUIRE([SM_STATIC])

AC_ARG_WITH(ffmpeg, AS_HELP_STRING([--without-ffmpeg],[Disable ffmpeg support]), with_ffmpeg=$withval, with_ffmpeg=yes)
AC_ARG_WITH(ffmpeg-rpath, AS_HELP_STRING([--with-ffmpeg-rpath],[Specify RPATH for ffmpeg]), ffmpeg_rpath=$withval, ffmpeg_rpath=no)
AC_ARG_WITH(static-ffmpeg, AS_HELP_STRING([--with-static-ffmpeg],[Statically link ffmpeg libraries]), with_static_ffmpeg=$withval, with_static_ffmpeg=no)
AM_CONDITIONAL(STATIC_FFMPEG, test "$with_static_ffmpeg" = "yes")
AC_ARG_WITH(system-ffmpeg, AS_HELP_STRING([--with-system-ffmpeg], [Use system instead of bundled ffmpeg (UNSUPPORTED)]), system_ffmpeg=yes, system_ffmpeg=no)
AM_CONDITIONAL(SYSTEM_FFMPEG, test "$system_ffmpeg" = "yes")
AM_CONDITIONAL(WITHOUT_FFMPEG, test "$with_ffmpeg" != "yes")

VIDEO_CFLAGS=
VIDEO_LIBS=

if test "$with_static_ffmpeg" = "yes"; then
	LIB_PRE=$START_STATIC
	LIB_POST=$END_STATIC
fi

have_ffmpeg=no
if test "$with_ffmpeg" = "yes"; then
	if test "$system_ffmpeg" = "yes"; then
		if pkg-config --libs libavcodec &> /dev/null; then AVCODEC_LIBS="`pkg-config --libs libavcodec`"; else AVCODEC_LIBS="-lavutil"; fi
		if pkg-config --libs libavformat &> /dev/null; then AVFORMAT_LIBS="`pkg-config --libs libavformat`"; else AVFORMAT_LIBS="-lavcodec -lavutil"; fi
		if pkg-config --libs libswscale &> /dev/null; then SWSCALE_LIBS="`pkg-config --libs libswscale`"; else SWSCALE_LIBS="-lavutil"; fi
		if test "$with_static_ffmpeg" = "yes" -o "$with_static_ffmpeg" = "no"; then
			AC_CHECK_LIB(avutil, av_free, have_libavutil=yes, have_libavutil=no)
			AC_CHECK_LIB(avcodec, avcodec_register_all, have_libavcodec=yes,  have_libavcodec=no, [$AVCODEC_LIBS])
			AC_CHECK_LIB(avformat, av_guess_format, have_libavformat=yes,  have_libavformat=no, [$AVFORMAT_LIBS])
			AC_CHECK_LIB(swscale, sws_scale, have_libswscale=yes,  have_libswscale=no, [$SWSCALE_LIBS])
			if test "$have_libavutil" = "yes" -a "$have_libavformat" = "yes" -a "$have_libavcodec" = "yes" -a "$have_libswscale" = "yes"; then
				have_ffmpeg=yes
				VIDEO_LIBS="$LIB_PRE -lswscale -lavformat -lswscale -lavcodec -lavutil $LIB_POST $VIDEO_LIBS"
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
			AC_CHECK_FUNC([avcodec_register_all], have_libavcodec=yes, have_libavcodec=no, [$AVCODEC_LIBS])
			AC_CHECK_FUNC([av_guess_format], have_libavformat=yes, have_libavformat=no, [$AVFORMAT_LIBS])
			AC_CHECK_FUNC([sws_scale], have_libswscale=yes, have_libswscale=no, [$SWSCALE_LIBS])

			if test "$have_libavutil" = "yes" -a "$have_libavformat" = "yes" -a "$have_libavcodec" = "yes" -a "$have_libswscale" = "yes"; then
				have_ffmpeg=yes
				VIDEO_CFLAGS="$FFMPEG_CFLAGS $VIDEO_CFLAGS"
				VIDEO_LIBS="$FFMPEG_LIBS $VIDEO_LIBS"
			fi

			CFLAGS="$ffmpeg_save_CFLAGS"
			CXXFLAGS="$ffmpeg_save_CXXFLAGS"
			LIBS="$ffmpeg_save_LIBS"
		fi
	else
dnl System FFMpeg
dnl HACK: $prefix is set to NONE if not user specified
		if test "$prefix" = "NONE"; then
			our_installdir="/opt"
		else
			our_installdir="$prefix"
		fi
dnl We might as well throw in GPL stuff as we're bound to GPL by libmad anyway.
		FFMPEG_CONFFLAGS="--shlibdir=$our_installdir/stepmania-$VERSION --enable-gpl --disable-programs --disable-doc --disable-avdevice --disable-swresample --disable-postproc --disable-avfilter"
		if test "$host_os" = "mingw32"; then
			FFMPEG_CONFFLAGS="$FFMPEG_CONFFLAGS --arch=x86"
		fi
		if test "$enable_lto" = "yes"; then
			FFMPEG_CONFFLAGS="$FFMPEG_CONFFLAGS --enable-lto"
		fi
		if test "$with_static_ffmpeg" = "yes"; then
			FFMPEG_CONFFLAGS="$FFMPEG_CONFFLAGS --disable-shared --enable-static"
		else
			FFMPEG_CONFFLAGS="$FFMPEG_CONFFLAGS --disable-static --enable-shared"
		fi
dnl Run ffmpeg ./configure
dnl It's NOT autoconf-based, so AC_CONFIG_SUBDIRS does not work.
		cd bundle/ffmpeg
		AC_MSG_NOTICE([Configuring ffmpeg. This is quiet and takes a bit. Be patient.])
		./configure $FFMPEG_CONFFLAGS
		if test $? != 0; then
			AC_MSG_ERROR([ffmpeg configuration failed.])
		fi
		cd ../..
		CFLAGS="$CFLAGS -I$PWD/bundle/ffmpeg"
		CXXFLAGS="$CXXFLAGS -I$PWD/bundle/ffmpeg"
dnl Make the binary find our bundled libs
		if test "$ffmpeg_rpath" = "no"; then
			LDFLAGS="$LDFLAGS -Wl,-rpath=. -Wl,-rpath=bundle/ffmpeg/libavutil,-rpath=bundle/ffmpeg/libavformat,-rpath=bundle/ffmpeg/libavcodec,-rpath=bundle/ffmpeg/libswscale"
		else
			LDFLAGS="$LDFLAGS -Wl,-rpath=$ffmpeg_rpath"
		fi
dnl Classic autoconf. This CANNOT be broken across multiple lines.
		VIDEO_LIBS="-L$PWD/bundle/ffmpeg/libavformat -lavformat -L$PWD/bundle/ffmpeg/libavcodec -lavcodec -L$PWD/bundle/ffmpeg/libswscale -lswscale -L$PWD/bundle/ffmpeg/libavutil -lavutil"
		have_ffmpeg=yes
	fi
fi

LIB_PRE=
LIB_POST=

AM_CONDITIONAL(HAVE_FFMPEG, test "$have_ffmpeg" = "yes")
if test "$have_ffmpeg" = "yes"; then
	AC_DEFINE(HAVE_FFMPEG, 1, [FFMpeg is available])
fi

AC_SUBST(VIDEO_CFLAGS)
AC_SUBST(VIDEO_LIBS)

])
