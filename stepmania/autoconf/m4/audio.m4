AC_DEFUN([SM_AUDIO], [

AC_ARG_WITH(vorbis, AC_HELP_STRING([--without-vorbis], [Disable Vorbis support]), with_vorbis=$enableval, with_vorbis=yes)

# This is used to force the integer decoder, on systems that prefer it.
AC_ARG_WITH(integer-vorbis, AC_HELP_STRING([--with-integer-vorbis], [Integer vorbis decoding only]), with_int_vorbis=$withval, with_int_vorbis=no)

have_vorbis=no
if test "$with_vorbis" = "yes" -a "$with_int_vorbis" = "no"; then
	AC_CHECK_LIB(ogg, ogg_stream_init, have_libogg=yes, have_libogg=no)
	AC_CHECK_LIB(vorbis, vorbis_comment_add, have_libvorbis=yes, have_libvorbis=no, [-logg])
	AC_CHECK_LIB(vorbisfile, ov_open, have_libvorbisfile=yes, have_libvorbisfile=no, [-lvorbis -logg])
	if test "$have_libvorbis" = "yes" -a "$have_libogg" = "yes" -a "$have_libvorbisfile" = "yes"; then
		have_vorbis=yes
		AUDIO_LIBS="$AUDIO_LIBS -lvorbisfile -lvorbis -logg"
	else
		echo Not all vorbis libraries found.
	fi
fi

# Search for the integer decoder if specified, or if Vorbis is enabled and we failed
# above.
if test "$with_vorbis" = "yes" -a "$have_vorbis" = "no"; then
	AC_CHECK_LIB(ivorbisfile, ov_open, have_libivorbisfile=yes,  have_libivorbisfile=no)
	if test "$have_libivorbisfile" = "yes"; then
		have_vorbis=yes
		AUDIO_LIBS="$AUDIO_LIBS -livorbisfile"
		AC_DEFINE(INTEGER_VORBIS, 1, [Integer Vorbis decoding])
	fi
fi

if test "$have_vorbis" = "no"; then
	if test "$with_vorbis" = "yes"; then
		AC_MSG_ERROR(
			 [A working installation of Ogg Vorbis could not be found.  Vorbis
support is strongly recommended.  If you really want to compile without it, pass
the "--without-vorbis" flag to configure.])
	fi

	AC_DEFINE(NO_VORBIS_SUPPORT, 1, [Vorbis support not available])
fi


AM_CONDITIONAL(HAVE_VORBIS, test "$have_vorbis" = "yes" )


AC_ARG_WITH(mp3, AC_HELP_STRING([--without-mp3], [Disable MP3 support]), with_mp3=$withval, with_mp3=yes)
if test "$with_mp3" = "yes"; then
	AC_CHECK_LIB(mad, mad_synth_init, have_mad=yes,  have_mad=no)
else
	have_mad=no
fi

if test "$have_mad" = "no"; then
	# Require MP3 by default, so people don't compile without MP3 support
	# by accident and then come asking us why files won't load.
	if test "$with_mp3" = "yes"; then
		AC_MSG_ERROR(
			 [A working installation of MAD could not be found, which is
required for MP3 support.  If you really want to compile without MP3 support,
pass the "--without-mp3" flag to configure.])
	fi
	AC_DEFINE(NO_MP3_SUPPORT, 1, [MP3 support not available])
else
	AUDIO_LIBS="$AUDIO_LIBS -lmad"
fi

AC_SUBST(AUDIO_LIBS)

AM_CONDITIONAL(HAVE_MP3, test "$have_mad" = "yes" )

])
