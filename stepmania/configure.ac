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

AC_SEARCH_LIBS(IMG_Load, [SDL_image], have_SDL_image=yes,  have_SDL_image=no)
# Since we link statically against SDL_sound, we need to link all
# of its support libraries ourself.
AC_SEARCH_LIBS(vorbis_comment_add, [vorbis], have_vorbis=yes,  have_vorbis=no)
AC_SEARCH_LIBS(ogg_stream_init, [ogg], have_ogg=yes,  have_ogg=no)
AC_SEARCH_LIBS(ov_open, [vorbisfile], have_vorbisfile=yes,  have_vorbisfile=no)
AC_SEARCH_LIBS(mad_synth_init, [mad], have_mad=yes,  have_mad=no)

AM_PATH_SDL

AM_PATH_ALSA(0.9.0, alsa=true ,alsa=0 )
AC_DEFINE_UNQUOTED([ALSA], $alsa)
AH_VERBATIM([ALSA],
[#if ALSA
#define HAVE_ALSA
#endif
])
AM_CONDITIONAL(ALSA, test x$alsa = xtrue )

# We could do this more cleanly with an AC_CHECK_FUNCS wrapper.
AC_CHECK_FUNCS([powf sqrtf sinf tanf cosf acosf])
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
])

AC_CHECK_DECLS([SIGPWR],,,[#include <signal.h>])

case $host in
	*-*-darwin*)
		AC_DEFINE(DARWIN, 1, [Use different sound drivers than linux])
		darwin=true;;
esac
AM_CONDITIONAL(DARWIN, test x$darwin = xtrue)


# sdl-config puts -L/usr/lib in the library search path, which reorders things
# in a way that breaks some configurations.
SDL_LIBS="`echo $SDL_LIBS | sed 's_-L/usr/lib/\?[[ $]]__'`"

AC_OUTPUT([Makefile src/Makefile])

