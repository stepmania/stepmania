# only tested with autoconf 2.57
AC_PREREQ(2.53)
AC_INIT(src/StepMania.cpp)
AC_CONFIG_AUX_DIR(autoconf)
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE(StepMania, 3.001)
AM_CONFIG_HEADER(src/config.h)

# We don't want PROG_CC/CXX default settings, but don't ignore explicit settings.
test -z "$CFLAGS"    && DEFAULT_CFLAGS=yes
test -z "$CXXFLAGS"  && DEFAULT_CXXFLAGS=yes
test -z "$LDFLAGS"   && DEFAULT_LDFLAGS=yes

AC_PROG_CC
AC_PROG_CXX

test "$DEFAULT_CFLAGS" = "yes" && CFLAGS=""
test "$DEFAULT_CXXFLAGS" = "yes" && CXXFLAGS=""
test "$DEFAULT_LDFLAGS" = "yes" && LDFLAGS=""

MDL_HAVE_OPENGL

AM_PATH_SDL
# sdl-config puts -L/usr/lib in the library search path, which reorders things
# in a way that breaks some configurations.
SDL_LIBS="`echo $SDL_LIBS | sed 's_-L/usr/lib/\?[[ $]]__'`"


# These tests break my custom build environment.  I'll put them back in later.
#AC_SEARCH_LIBS(IMG_Load, [SDL_image], have_SDL_image=yes,  have_SDL_image=no)
#if test "x$have_SDL_image" = "xno"; then
#  echo
#  echo "*** SDL_image is required to build StepMania; please"
#  echo "*** make sure that SDL_image is installed to continue"
#  echo "*** the installation process."
#  exit 0;
#fi

AC_ARG_WITH(vorbis, AC_HELP_STRING([--without-vorbis], [Disable Vorbis support]), with_vorbis=$enableval, with_vorbis=yes)

# This is used to force the integer decoder, on systems that prefer it.
AC_ARG_WITH(integer-vorbis, AC_HELP_STRING([--with-integer-vorbis], [Integer vorbis decoding only]), with_int_vorbis=$withval, with_int_vorbis=no)

echo $with_int_vorbis
have_vorbis=no
if test "$with_vorbis" = "yes" -a "$with_int_vorbis" = "no"; then
	oldlibs="$LIBS"
	AC_SEARCH_LIBS(ogg_stream_init, [ogg], have_libogg=yes,  have_libogg=no)
	AC_SEARCH_LIBS(vorbis_comment_add, [vorbis], have_libvorbis=yes,  have_libvorbis=no)
	AC_SEARCH_LIBS(ov_open, [vorbisfile], have_libvorbisfile=yes,  have_libvorbisfile=no)
	if test "$have_libvorbis" = "yes" -a "$have_libogg" = "yes" -a "$have_libvorbisfile" = "yes"; then
		have_vorbis=yes
	else
		echo Not all vorbis libraries found.
		LIBS="$oldlibs"
	fi
fi

# Search for the integer decoder if specified, or if Vorbis is enabled and we failed
# above.
if test "$with_vorbis" = "yes" -a "$have_vorbis" = "no"; then
	AC_SEARCH_LIBS(ov_open, [ivorbisfile], have_libivorbisfile=yes,  have_libivorbisfile=no)
	if test "$have_libivorbisfile" = "yes"; then
		have_vorbis=yes
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
	AC_SEARCH_LIBS(mad_synth_init, [mad], have_mad=yes,  have_mad=no)
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
fi

AM_CONDITIONAL(HAVE_MP3, test "$have_mad" = "yes" )

AC_SEARCH_LIBS(avcodec_init, [avcodec], have_libavcodec=yes,  have_libavcodec=no)
AC_SEARCH_LIBS(guess_format, [avformat], have_libavformat=yes,  have_libavformat=no)

have_ffmpeg=no
if test "$have_libavformat" = "yes" -a "$have_libavcodec" = "yes"; then
	have_ffmpeg=yes
	AC_DEFINE(HAVE_FFMPEG, 1, [FMPEG support available])
fi
AM_CONDITIONAL(HAVE_FFMPEG, test "$have_ffmpeg" == "yes" )

SM_CHECK_CRASH_HANDLER

AM_PATH_ALSA(0.9.0,AC_DEFINE([HAVE_ALSA],1,[Define presence of ALSA]),alsa=false)

AC_ARG_ENABLE(gtk2, AC_HELP_STRING([--disable-gtk2], [Disable GTK support]), enable_gtk2=$enableval, enable_gtk2=yes)

if test x$enable_gtk2 = xyes; then
  AM_PATH_GTK_2_0(2.0.0,AC_DEFINE([HAVE_GTK],1,[Define presence of GTK]),enable_gtk2=no)
fi

AC_CHECK_HEADER(linux/soundcard.h, [AC_DEFINE(HAVE_OSS, 1, [OSS support available])])
AC_CHECK_HEADERS([inttypes.h])

AM_CONDITIONAL(HAVE_ALSA, test x$alsa != xfalse )
AM_CONDITIONAL(HAVE_GTK, test "$enable_gtk2" != "no" )
AM_CONDITIONAL(HAVE_OSS, test x$ac_cv_header_linux_soundcard_h = xyes )
AM_CONDITIONAL(USE_CRASH_HANDLER, test "$use_crash_handler" = "yes" )

# We could do this more cleanly with an AC_CHECK_FUNCS wrapper.
AC_CHECK_FUNCS([powf sqrtf sinf tanf cosf acosf roundf truncf])
AH_VERBATIM([VA_ZZZ_NEEDED_FUNCS],
[#if !defined(HAVE_POWF)
#define NEED_POWF
#endif
#if !defined(HAVE_SQRTF)
#define NEED_SQRTF
#endif
#if !defined(HAVE_SINF)
#define NEED_SINF
#endif
#if !defined(HAVE_COSF)
#define NEED_COSF
#endif
#if !defined(HAVE_TANF)
#define NEED_TANF
#endif
#if !defined(HAVE_ACOSF)
#define NEED_ACOSF
#endif
#if !defined(HAVE_ROUNDF)
#define NEED_ROUNDF
#endif
#if !defined(HAVE_TRUNCF)
#define NEED_TRUNCF
#endif
])

AC_CHECK_DECLS([SIGPWR],,,[#include <signal.h>])

case $host in
	*-*-darwin*)
		AC_DEFINE(DARWIN, 1, [Use different sound drivers than linux])
		darwin=true;;
esac
AM_CONDITIONAL(DARWIN, test x$darwin = xtrue)

AC_OUTPUT([Makefile src/Makefile])
