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

test "$DEFAULT_CFLAGS" = "yes" && CFLAGS="-Wall -W -Wno-unused -Wno-switch"
test "$DEFAULT_CXXFLAGS" = "yes" && CXXFLAGS="-Wall -W -Wno-unused -Wno-switch"
test "$DEFAULT_LDFLAGS" = "yes" && LDFLAGS=""

compile=release
AC_ARG_WITH(debug, AC_HELP_STRING([--with-debug], [Enable debug mode]), with_debug=$withval, with_debug=no)
AC_ARG_WITH(fast-compile, AC_HELP_STRING([--with-fast-compile], [Enable fast compile]), with_fast_compile=$withval, with_fast_compile=no)
if test "$with_debug" = "yes"; then
	compile=debug
fi
if test "$with_fast_compile" = "yes"; then
	compile=fast
fi

case $compile in
	release)
	test "$DEFAULT_CFLAGS" = "yes" && CFLAGS="$CFLAGS -O3"
	test "$DEFAULT_CXXFLAGS" = "yes" && CXXFLAGS="$CXXFLAGS -O3"
	;;
	debug)
	test "$DEFAULT_CFLAGS" = "yes" && CFLAGS="$CFLAGS -g"
	test "$DEFAULT_CXXFLAGS" = "yes" && CXXFLAGS="$CXXFLAGS -g"
	;;
	fast)
	test "$DEFAULT_CFLAGS" = "yes" && CFLAGS="$CFLAGS -O2 -fno-inline"
	test "$DEFAULT_CXXFLAGS" = "yes" && CXXFLAGS="$CXXFLAGS -O2 -fno-inline"
	;;
esac

AC_ARG_WITH(prof, AC_HELP_STRING([--with-prof], [Enable profiling]), with_prof=$withval, with_prof=no)
if test "$with_prof" = "yes"; then
	test "$DEFAULT_CFLAGS" = "yes" && CFLAGS="$CFLAGS -pg"
	test "$DEFAULT_CXXFLAGS" = "yes" && CXXFLAGS="$CXXFLAGS -pg"
	test "$DEFAULT_LDFLAGS" = "yes" && LDFLAGS="$LDFLAGS -pg"
fi

SM_OPENGL

AM_PATH_SDL(1.2.6,enable_sdl=yes,enable_sdl=no)
AM_CONDITIONAL(NHAVE_SDL, test x$enable_sdl = xno )

# sdl-config puts -L/usr/lib in the library search path, which reorders things
# in a way that breaks some configurations.
# Does this still need to be in?
# not sure
# SDL_LIBS="`echo $SDL_LIBS | sed 's_-L/usr/lib/\?[[ $]]__'`"

# These tests break my custom build environment.  I'll put them back in later.
#AC_SEARCH_LIBS(IMG_Load, [SDL_image], have_SDL_image=yes,  have_SDL_image=no)
#if test "x$have_SDL_image" = "xno"; then
#  echo
#  echo "*** SDL_image is required to build StepMania; please"
#  echo "*** make sure that SDL_image is installed to continue"
#  echo "*** the installation process."
#  exit 0;
#fi

SM_AUDIO

AC_SEARCH_LIBS(avcodec_init, [avcodec], have_libavcodec=yes,  have_libavcodec=no)
AC_SEARCH_LIBS(guess_format, [avformat], have_libavformat=yes,  have_libavformat=no)

if test "$have_libavcodec" = "yes"; then
  AC_MSG_CHECKING([for libavcodec >= 0.4.8])
  AC_TRY_RUN([
      #include <ffmpeg/avcodec.h>
      int main()
      {
	      return ( FFMPEG_VERSION_INT < 0x000408 )? 1:0;
      }
      ],,have_libavcodec=no,)
  AC_MSG_RESULT($have_libavformat)
fi

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
	# This is just a general define for things we do in Linux that aren't
	# yet fully generalized.  It should probably be defined in other *nixes,
	# too.
	*)
		AC_DEFINE(LINUX, 1, [Linux])
		;;
esac

AC_OUTPUT([Makefile src/Makefile])
