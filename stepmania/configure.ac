# only tested with autoconf 2.57
AC_PREREQ(2.53)
AC_INIT(src/StepMania.cpp)
AC_CONFIG_AUX_DIR(autoconf)
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE(StepMania, 3.001)
AM_CONFIG_HEADER(src/config.h)
AM_MAINTAINER_MODE

# We don't want PROG_CC/CXX default settings, but don't ignore explicit settings.
test -z "$CFLAGS"    && DEFAULT_CFLAGS=yes
test -z "$CXXFLAGS"  && DEFAULT_CXXFLAGS=yes
test -z "$LDFLAGS"   && DEFAULT_LDFLAGS=yes

AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB

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

SM_X_WITH_OPENGL

AM_PATH_SDL(1.2.6,enable_sdl=yes,enable_sdl=no)
# XXX: 1.2.6 has been out for a while (as well as 1.2.7).  I'm disabling the
# fallback copy of SDL for now, to see if we can get rid of it.
if test "$enable_sdl" = "no"; then
  echo
  echo "*** SDL 1.2.6 or greater is required to build StepMania; please"
  echo "*** make sure that it is installed to continue the build process."
  exit 1
fi

# sdl-config puts -L/usr/lib in the library search path, which reorders things
# in a way that breaks some configurations.
# Does this still need to be in?
# not sure
# SDL_LIBS="`echo $SDL_LIBS | sed 's_-L/usr/lib/\?[[ $]]__'`"

AC_SEARCH_LIBS(IMG_Load, [SDL_image], have_SDL_image=yes,  have_SDL_image=no)
if test "$have_SDL_image" = "no"; then
  echo
  echo "*** SDL_image is required to build StepMania; please make sure that"
  echo "*** it is installed to continue the build process."
  exit 1
fi

have_libjpeg=yes
AC_SEARCH_LIBS(jpeg_read_scanlines, [jpeg], ,  have_libjpeg=no)
AC_CHECK_HEADER(jpeglib.h, , have_libjpeg=no)
if test "$have_libjpeg" = "no"; then
  echo
  echo "*** libjpeg is required to build StepMania; please make sure that"
  echo "*** it is installed to continue the build process."
  exit 1
fi

SM_LUA
SM_ZLIB
SM_AUDIO

AC_SEARCH_LIBS(avcodec_init, [avcodec], have_libavcodec=yes,  have_libavcodec=no)
AC_SEARCH_LIBS(guess_format, [avformat], have_libavformat=yes,  have_libavformat=no)

if test "$have_libavcodec" = "yes"; then
  AC_MSG_CHECKING([for libavcodec >= 0.4.8])
  AC_TRY_RUN([
      #include <ffmpeg/avcodec.h>
      int main()
      {
	      return ( LIBAVCODEC_VERSION_INT < 0x000408 )? 1:0;
      }
      ],,have_libavcodec=no,)
  AC_MSG_RESULT($have_libavcodec)
fi

if test "$have_libavformat" = "yes"; then
  AC_MSG_CHECKING([for libavformat >= 0.4.8])
  AC_TRY_RUN([
      #include <ffmpeg/avformat.h>
      int main()
      {
	      return ( LIBAVFORMAT_VERSION_INT < 0x000408 )? 1:0;
      }
      ],,have_libavformat=no,)
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

AC_CHECK_HEADER(sys/soundcard.h, [AC_DEFINE(HAVE_OSS, 1, [OSS support available])])
AC_ARG_ENABLE(force-oss, AC_HELP_STRING([--enable-force-oss], [Force OSS]), force_oss=$enableval, force_oss=no)
AC_CHECK_HEADER(stdint.h, , [AC_DEFINE(MISSING_STDINT_H, 1, [stdint.h is missing])])
AC_CHECK_HEADERS([inttypes.h])

AC_MSG_CHECKING(if cstdlib breaks llabs)
AC_TRY_COMPILE(	[#include <stdlib.h>
		 #include <cstdlib> ],
		[int foo() { return llabs(1); }],
		[AC_MSG_RESULT(no)],
		[AC_MSG_RESULT(yes)
		 AC_DEFINE([NEED_CSTDLIB_WORKAROUND], 1, [cstdlib breaks llabs])]
	      )

AC_CHECK_LIB( pthread, pthread_create, AC_DEFINE([HAVE_LIBPTHREAD],1,[libpthread is available]) )
AC_CHECK_LIB( pthread, pthread_mutex_timedlock, AC_DEFINE([HAVE_PTHREAD_MUTEX_TIMEDLOCK],1,[pthreads has pthread_mutex_timedlock()]) )

# Always:
AC_DEFINE(_GNU_SOURCE, 1, [Use GNU extensions])

AM_CONDITIONAL(HAVE_ALSA, test x$alsa != xfalse )
AM_CONDITIONAL(HAVE_GTK, test "$enable_gtk2" != "no" )
AM_CONDITIONAL(HAVE_OSS, test x$ac_cv_header_sys_soundcard_h = xyes )
AM_CONDITIONAL(USE_CRASH_HANDLER, test "$use_crash_handler" = "yes" )

if test x$force_oss = xyes && test x$ac_cv_header_sys_soundcard_h = xyes; then
	AC_DEFINE([FORCE_OSS], 1, [Force OSS Usage])
fi

AC_CHECK_DECL(powf, , AC_DEFINE([NEED_POWF],1,[Need powf]), [#include <math.h>])
AC_CHECK_DECL(sqrtf, , AC_DEFINE([NEED_SQRTF],1,[Need sqrtf]), [#include <math.h>])
AC_CHECK_DECL(sinf, , AC_DEFINE([NEED_SINF],1,[Need sinf]), [#include <math.h>])
AC_CHECK_DECL(tanf, , AC_DEFINE([NEED_COSF],1,[Need tanf]), [#include <math.h>])
AC_CHECK_DECL(cosf, , AC_DEFINE([NEED_TANF],1,[Need cosf]), [#include <math.h>])
AC_CHECK_DECL(acosf, , AC_DEFINE([NEED_ACOSF],1,[Need acosf]), [#include <math.h>])
AC_CHECK_DECL(roundf, , AC_DEFINE([NEED_ROUNDF],1,[Need roundf]), [#include <math.h>])
AC_CHECK_DECL(truncf, , AC_DEFINE([NEED_TRUNCF],1,[Need truncf]), [#include <math.h>])

# This doesn't work on glibc math functions:
# AC_CHECK_FUNCS([sqrtf sinf tanf cosf acosf roundf truncf])

AC_CHECK_DECLS([SIGPWR],,,[#include <signal.h>])

case $host in
	# This is just a general define for things we do in Linux that aren't
	# yet fully generalized.  It should probably be defined in other *nixes,
	# too.
	*)
		AC_DEFINE(LINUX, 1, [Linux])
		;;
esac

AC_ARG_ENABLE(tests, AC_HELP_STRING([--enable-tests], [Build test programs]), enable_tests=$enableval, enable_tests=no)
AM_CONDITIONAL(BUILD_TESTS, test "$enable_tests" = "yes" )

AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(src/Makefile)
AC_OUTPUT
