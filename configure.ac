AC_PREREQ(2.59)
AC_INIT(StepMania, 5.0, [https://github.com/stepmania/stepmania], StepMania, [http://stepmania.com])
AC_CONFIG_SRCDIR([src/StepMania.cpp])
AC_CONFIG_AUX_DIR(autoconf)
AC_CANONICAL_TARGET
AC_PREFIX_DEFAULT(/opt)

AM_INIT_AUTOMAKE([no-define])
AC_CONFIG_HEADERS([src/config.h])
AM_MAINTAINER_MODE

# We don't want PROG_CC/CXX default settings, but don't ignore explicit settings.
test -z "$CFLAGS"    && DEFAULT_CFLAGS=yes
test -z "$CXXFLAGS"  && DEFAULT_CXXFLAGS=yes
test -z "$LDFLAGS"   && DEFAULT_LDFLAGS=yes

AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AM_PROG_CC_C_O

test "$DEFAULT_CFLAGS" = "yes" && CFLAGS="-Wall -W -Wno-unused-parameter -Wno-switch"
test "$DEFAULT_CXXFLAGS" = "yes" && CXXFLAGS="-Wall -W -Wno-unused-parameter -Wno-switch"
test "$DEFAULT_LDFLAGS" = "yes" && LDFLAGS=""

# If on Windows, grab the prebuilts
if test "$host_os" = "mingw32"; then
	CPPFLAGS="$CPPFLAGS -I$PWD/extern/for_mingw/include"
	LDFLAGS="$LDFLAGS -L$PWD/extern/for_mingw/lib"
	# HACK: The prebuilts are 32bit only. Force the issue.
	CFLAGS="$CFLAGS -m32"
	CXXFLAGS="$CXXFLAGS -m32"
	RESFLAGS="$RESFLAGS --target=pe-i386 -I$PWD/src/archutils/Win32/mingw"
fi

AM_CONDITIONAL(WITH_TTY, test "$with_tty" = "yes")
AC_ARG_WITH(linux-tty, AS_HELP_STRING([--with-linux-tty], [Enable Linux TTY input]), with_tty=yes, with_tty=no)

compile=release
AC_ARG_WITH(debug, AS_HELP_STRING([--with-debug],[Enable debug mode]), with_debug=$withval, with_debug=no)
AC_ARG_WITH(fast-compile, AS_HELP_STRING([--with-fast-compile],[Enable fast compile]), with_fast_compile=$withval, with_fast_compile=no)
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

AC_ARG_ENABLE(lto, AS_HELP_STRING([--enable-lto], [Enable LTO]), enable_lto=$enableval, enable_lto=no)
if test x$enable_lto = xyes; then
	if test "$CXX" != "g++"; then
		AC_MSG_ERROR([g++ compiler is needed to use lto (if you have it installed, pass CXX=g++ to ./configure)])
	fi
	gxx_version="`g++ -dumpversion`"
	gxx_version_major=$(echo $gxx_version | cut -d'.' -f1)
	gxx_version_minor=$(echo $gxx_version | cut -d'.' -f2)
	if test [ "$gxx_version_major" -lt "4" ] || test [ "$gxx_version_minor" -lt "5" ]; then
		AC_MSG_ERROR([Your g++ version is too old and doesn't support lto])
	fi
	CFLAGS="$CFLAGS -O0 -flto"
	CXXFLAGS="$CXXFLAGS -O0 -flto"
	LDFLAGS="$LDFLAGS -flto"
	case $compile in
		release)
		LDFLAGS="$LDFLAGS -O3"
		;;
		debug)
		LDFLAGS="$LDFLAGS -g"
		;;
		fast)
		LDFLAGS="$LDFLAGS -O2 -fno-inline"
		;;
	esac
	if test [ "$gxx_version_minor" -ge "8" ]; then
		CFLAGS="$CFLAGS -fno-fat-lto-objects"
		CXXFLAGS="$CXXFLAGS -fno-fat-lto-objects"
	fi
fi

dnl Do this early as it has its own whole configure script to run with its own errors.
SM_VIDEO

# Some (broken?) versions of gcc treat char as unsigned, check for that.
AC_LANG_PUSH([C])
AC_MSG_CHECKING([for gcc option to use signed chars])
AC_RUN_IFELSE([AC_LANG_SOURCE([[int main(){return (char)-1==-1?0:1;}]])],
	   [sc_option=],[sc_option=-fsigned-char])
AC_MSG_RESULT([${sc_option-none}])
CFLAGS="$CFLAGS $sc_option"
CXXFLAGS="$CXXFLAGS $sc_option"
AC_LANG_POP([C])

AC_CHECK_HEADERS([sys/param.h])

if test x$with_debug = xyes; then
	AC_DEFINE([DEBUG], 1, [Special debugging logic])
	AC_LANG_PUSH([C++])
	AC_CHECK_HEADER([debug/debug.h],[
		AC_DEFINE([_GLIBCXX_DEBUG],1,[STL debug])
		AC_DEFINE([_GLIBCXX_DEBUG_PEDANTIC],1,[STL pedantic debug])])
	AC_LANG_POP([C++])
fi

# Define UNIX for all Unix-like systems.  Don't define it for cross-compiling to
# non-Unix-like systems.  (-DUNIX selects the archutils and ArchHooks to use; if
# your platform doesn't use the Unix ones, you probably don't want to define UNIX.)
case $host_os in
*linux*)
	AC_DEFINE(LINUX, 1, [Linux])
	AC_DEFINE(UNIX, 1, [Unix])
	linux=yes
	unix=yes
	;;

*bsd*)
	AC_CHECK_DECL(BSD,,AC_DEFINE(BSD, 1, [BSD]),[
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif])
	AC_DEFINE(UNIX, 1, [Unix])
	bsd=yes
	unix=yes
	;;
*mingw*)
	AC_DEFINE(WINDOWS, 1, [Windows])
	windows=yes
	;;
*)
	AC_DEFINE(UNIX, 1, [Unix])
	unix=yes
	;;
esac
if test "$windows" = "yes"; then
	SM_WINDOWS
fi
if test "$linux" = "yes"; then
	AC_DEFINE(LUA_USE_LINUX, 1, [Linux for Lua])
fi
AM_CONDITIONAL(UNIX, test "$unix" = "yes" )
AM_CONDITIONAL(LINUX, test "$linux" = "yes" )
AM_CONDITIONAL(BSD, test "$bsd" = "yes" )
AM_CONDITIONAL(WINDOWS, test "$windows" = "yes" )

# Define macros for individual CPU types, for a few bits of inline assembly.
# This is for major, compatible CPU classes--"CPU_X86" includes P2, P3, P4,
# AMD, etc.  If you need CPU-specific assembly, check at runtime--don't create
# separate binaries for each CPU if the binaries themselves are otherwise
# compatible.
case $host_cpu in
i?86*)
	AC_DEFINE(CPU_X86, 1, [x86])
	;;
x86_64*)
	AC_DEFINE(CPU_X86_64, 1, [x86-64])
	;;
powerpc*)
	AC_DEFINE(CPU_PPC, 1, [PPC])
	have_parallel_port=no
	;;
esac
AM_CONDITIONAL(HAVE_PARALLEL_PORT, test "${have_parallel_port-yes}" = "yes")

# Minimaid support flag
AC_ARG_WITH(minimaid, AS_HELP_STRING([--with-minimaid],[Enable Minimaid support]), with_minimaid=$withval, with_minimaid=no)
AM_CONDITIONAL(HAVE_MINIMAID, test "$with_minimaid" = "yes")

AC_C_BIGENDIAN(
	AC_DEFINE(ENDIAN_BIG, 1, [Big endian]),
	AC_DEFINE(ENDIAN_LITTLE, 1, [Little endian]),
	AC_MSG_ERROR([Can't determine endianness]) )

# If we're compiling with the SSE2 build flag, add that flag to CFLAGS and CXXFLAGS.
# (Handwritten SSE/SSE2 code isn't in the binary, but GCC's heuristics optimize enough
# vector arithmetic in Rage that this produces a noticeable speedup on SSE2 processors.)

AC_ARG_WITH(sse2, AC_HELP_STRING([--with-sse2], [Enable SSE2 optimization]), with_sse2=$withval, with_sse2=no)

if test "$with_sse2" = "yes"; then
	CFLAGS="$CFLAGS -msse2"
	CXXFLAGS="$CXXFLAGS -msse2"
fi

AC_ARG_WITH(prof, AS_HELP_STRING([--with-prof],[Enable profiling]), with_prof=$withval, with_prof=no)
if test "$with_prof" = "yes"; then
	test "$DEFAULT_CFLAGS" = "yes" && CFLAGS="$CFLAGS -pg"
	test "$DEFAULT_CXXFLAGS" = "yes" && CXXFLAGS="$CXXFLAGS -pg"
	test "$DEFAULT_LDFLAGS" = "yes" && LDFLAGS="$LDFLAGS -pg"
fi

dnl Usually in libc, but not always(?)
if test "$unix" = "yes"; then
	AC_SEARCH_LIBS(clock_gettime, rt,, AC_ERROR([clock_gettime() not available?]))
fi

SM_OPENGL
SM_X11
GL_CFLAGS="$GL_CFLAGS $XCFLAGS"
GL_LIBS="$GL_LIBS $XLIBS"

AC_SUBST(GL_CFLAGS)
AC_SUBST(GL_LIBS)

have_libpng=yes
SM_STATIC
AC_CHECK_LIB(png, png_create_read_struct, [x=y], have_libpng=no, [-lz -lm]) # x=y to stop autoconf from messing with LIBS
AC_CHECK_HEADER(png.h, , have_libpng=no)
AC_ARG_WITH(static-png, AS_HELP_STRING([--with-static-png],[Statically link libpng]), with_static_png=$withval, with_static_png=no)
if test "$with_static_png" = "yes"; then
	LIB_PRE=$START_STATIC
	LIB_POST=$END_STATIC
fi
if test "$have_libpng" = "no"; then
  echo
  echo "*** libpng is required to build StepMania; please make sure that"
  echo "*** it is installed to continue the build process."
  exit 1
fi

LIBS="$LIBS $LIB_PRE -lpng -lz -lm $LIB_POST"
LIB_PRE=
LIB_POST=

AC_ARG_WITH(jpeg, AS_HELP_STRING([--without-jpeg],[Disable JPEG support]), with_jpeg=$withval, with_jpeg=yes)
AC_ARG_WITH(static-jpeg, AS_HELP_STRING([--with-static-jpeg],[Statically link libjpeg]), with_static_jpeg=$withval, with_static_jpeg=no)

if test "$with_static_png" = "yes"; then
	LIB_PRE=$START_STATIC
	LIB_POST=$END_STATIC
fi
if test "$with_jpeg" = "yes"; then
	have_libjpeg=yes
	AC_CHECK_LIB(jpeg, jpeg_read_scanlines, [x=y], have_libjpeg=no) # x=y to stop autoconf from messing with LIBS
	AC_CHECK_HEADER(jpeglib.h, , have_libjpeg=no)

	if test "$have_libjpeg" = "no"; then
		# Require JPEG by default, so people don't compile without it
		# by accident and then come asking us why files won't load.
		AC_MSG_ERROR(
[libjpeg is required to build StepMania; please make sure that it is installed
to continue the build process.  If you really want to compile without JPEG
support, pass the "--without-jpeg" flag to configure.])
	fi
	have_libjpeg=

	LIBS="$LIBS $LIB_PRE -ljpeg $LIB_POST"
else
	AC_DEFINE(NO_JPEG_SUPPORT, 1, [JPEG support not available])
fi
LIB_PRE=
LIB_POST=

AC_ARG_WITH(network, AS_HELP_STRING([--without-network],[Disable networking]), with_network=$withval, with_network=yes)
if test "$with_network" = "no"; then
	AC_DEFINE(WITHOUT_NETWORKING, 1, [Networking support not available])
fi
AM_CONDITIONAL(WITHOUT_NETWORKING, test "$with_network" = "no")

AC_ARG_WITH(gles2, AS_HELP_STRING([--without-gles2],[Disable OpenGL ES 2.0]), with_gles2=$withval, with_gles2=yes)
if test "$with_gles2" = "no"; then
	AC_DEFINE(WITHOUT_GLES2, 1, [OpenGL ES 2.0 support not available])
fi
AM_CONDITIONAL(WITHOUT_GLES2, test "$with_gles2" = "no")

AM_ICONV
# HACK: The iconv check can pass erroneously on mingw64 under some conditions I'm not really sure of.
if test "$windows" = "yes"; then
	LIBS="$LIBS -liconv"
else
	LIBS="$LIBS $LIBICONV"
fi

SM_ZLIB
SM_BZIP
SM_AUDIO
dnl SM_VIDEO
SM_TLS

SM_CHECK_CRASH_HANDLER

AM_PATH_ALSA(0.9.0,AC_DEFINE([HAVE_ALSA],1,[Define presence of ALSA]),alsa=false)

AC_ARG_ENABLE(gtk2, AS_HELP_STRING([--disable-gtk2],[Disable GTK support]), enable_gtk2=$enableval, enable_gtk2=yes)

if test x$enable_gtk2 = xyes; then
  AM_PATH_GTK_2_0(2.0.0,AC_DEFINE([HAVE_GTK],1,[Define presence of GTK]),enable_gtk2=no)
fi

AC_CHECK_HEADER(sys/soundcard.h, [AC_DEFINE(HAVE_OSS, 1, [OSS support available])])
AC_CHECK_DECL(OSS_GETVERSION, AC_DEFINE([HAVE_OSS_GETVERSION],1,[OSS_GETVERSION is defined]), , [#include <sys/soundcard.h>])
AC_ARG_ENABLE(force-oss, AS_HELP_STRING([--enable-force-oss],[Force OSS]), force_oss=$enableval, force_oss=no)
AC_CHECK_HEADER(stdint.h, , [AC_DEFINE(MISSING_STDINT_H, 1, [stdint.h is missing])])
AC_CHECK_HEADERS([inttypes.h endian.h machine/endian.h alloca.h])

have_pulse=no
AC_CHECK_LIB(pulse, pa_stream_new, have_pulse=yes)
if test x$have_pulse = xyes; then
	AC_DEFINE(HAVE_PULSE, 1, [pulseaudio support available])
	LIBS="$LIBS -lpulse"
fi

dnl Search for JACK unless --without-jack or --with-jack=no is given; require it for --with-jack
AC_ARG_WITH([jack], [AS_HELP_STRING([--without-jack], [Disable JACK sound driver])])
AS_IF([test "x$with_jack" != "xno"], [
       AC_SEARCH_LIBS([jack_client_open], [jack], [have_jack=yes], [
		       AS_IF([test "x$with_jack" != x], [
			      AC_MSG_ERROR([--with-jack was specified but JACK was not found])
			     ])
		       ])
])

AC_MSG_CHECKING(if cstdlib breaks llabs)
AC_LANG_PUSH(C++)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <stdlib.h>
		 #include <cstdlib>
		 using namespace std;]], [[llabs(1)]])],[AC_MSG_RESULT(no)],[AC_MSG_RESULT(yes)
		 AC_DEFINE([NEED_CSTDLIB_WORKAROUND], 1, [cstdlib breaks llabs])
	      ])
AC_LANG_POP(C++)

# TODO: This should have a --with option to force it on
if test "$windows" != "yes"; then
AC_CHECK_LIB( pthread, pthread_create, [have_pthread=yes; AC_DEFINE([HAVE_LIBPTHREAD],1,[libpthread is available])], have_pthread=no)
AC_CHECK_LIB( pthread, pthread_mutex_timedlock, AC_DEFINE([HAVE_PTHREAD_MUTEX_TIMEDLOCK],1,[pthreads has pthread_mutex_timedlock()]) )
AC_CHECK_LIB( pthread, pthread_cond_timedwait, AC_DEFINE([HAVE_PTHREAD_COND_TIMEDWAIT],1,[pthreads has pthread_cond_timedwait()]) )
fi

# XXX: This isn't always being included when it is needed. Just force it in.
if test "$have_pthread" = "yes"; then
	LIBS="$LIBS -lpthread"
fi

AM_CONDITIONAL(HAVE_PTHREAD, test "$have_pthread" = "yes")

# Always:
AC_DEFINE(_GNU_SOURCE, 1, [Use GNU extensions])
AC_DEFINE(__STDC_FORMAT_MACROS, 1, [Use PRId64 and similar])
AC_DEFINE(USE_GITVERSION, 1, [We are generating gitversion.h])

AM_CONDITIONAL(HAVE_ALSA, test x$alsa != xfalse )
AM_CONDITIONAL(HAVE_GTK, test "$enable_gtk2" != "no" )
AM_CONDITIONAL(HAVE_JACK, test x$have_jack = xyes)
AM_CONDITIONAL(HAVE_OSS, test x$ac_cv_header_sys_soundcard_h = xyes )
AM_CONDITIONAL(HAVE_PULSE, test x$have_pulse = xyes)
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
AC_CHECK_DECL(strtof, , AC_DEFINE([NEED_STRTOF],1,[Need strtof]), [#include <stdlib.h>])

# This doesn't work on glibc math functions:
# AC_CHECK_FUNCS([sqrtf sinf tanf cosf acosf roundf truncf])

AC_CHECK_DECLS([SIGPWR, SIGUSR1],,,[#include <signal.h>])

AC_ARG_ENABLE(tests, AS_HELP_STRING([--enable-tests],[Build test programs]), enable_tests=$enableval, enable_tests=no)
AM_CONDITIONAL(BUILD_TESTS, test "$enable_tests" = "yes" )

AC_ARG_ENABLE(lua-binaries, AS_HELP_STRING([--enable-lua-binaries],[Build lua and luac]), enable_lua_binaries=$enableval, enable_lua_binaries=no)
AM_CONDITIONAL(BUILD_LUA_BINARIES, test "$enable_lua_binaries" = "yes" )

AC_SUBST(RESFLAGS)

dnl Add these after all tests are done as they don't exist just yet.
LIBS="$LIBS $FFMPEG_LIBS"

# Selection between bundled pcre and system pcre
AC_ARG_WITH(system-pcre, AS_HELP_STRING([--with-system-pcre],[Disable building of bundled libpcre]), with_system_pcre=$withval, with_system_pcre=no)
AM_CONDITIONAL(USE_SYSTEM_PCRE, test "$with_system_pcre" == "yes")

if test "$with_system_pcre" == "yes"; then
	have_libpcre=yes
	AC_CHECK_HEADER(pcre.h, , have_libpcre=no)
	if test "$have_libpcre" = "no"; then
		AC_MSG_ERROR([If you want to use system libpcre, please make sure that it is installed. Otherwise please configure with --without-system-pcre.])
	else
		AC_DEFINE([USE_SYSTEM_PCRE], 1, [Build with system libpcre])
		LIBS="$LIBS -lpcre"
	fi
fi

AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(bundle/Makefile)
AC_CONFIG_FILES(src/Makefile)
AC_OUTPUT
