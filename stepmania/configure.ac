# only tested with autoconf 2.57
AC_PREREQ(2.53)
AC_INIT(src/StepMania.cpp)
AC_CONFIG_AUX_DIR(autoconf)
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE(StepMania, 3.001)
AM_CONFIG_HEADER(src/config.h)

MDL_HAVE_OPENGL

AC_SEARCH_LIBS(IMG_Load, [SDL_image], have_SDL_image=yes,  have_SDL_image=no)
# Since we link statically against SDL_sound, we need to link all
# of its support libraries ourself.
AC_SEARCH_LIBS(vorbis_comment_add, [vorbis], have_vorbis=yes,  have_vorbis=no)
AC_SEARCH_LIBS(ogg_stream_init, [ogg], have_ogg=yes,  have_ogg=no)
AC_SEARCH_LIBS(ov_open, [vorbisfile], have_vorbisfile=yes,  have_vorbisfile=no)
AC_SEARCH_LIBS(mad_synth_init, [mad], have_mad=yes,  have_mad=no)

AC_PROG_CC
AC_PROG_CXX

AM_PATH_SDL

# We could do this more cleanly with an AC_CHECK_FUNCS wrapper.
AC_CHECK_FUNCS([powf sqrtf sinf cosf acosf])
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
#if !defined(HAVE_ACOSF)
#define NEED_ACOSF
#endif
])

# sdl-config puts -L/usr/lib in the library search path, which reorders things
# in a way that breaks some configurations.
SDL_LIBS="`echo $SDL_LIBS | sed 's_-L/usr/lib/\?[[ $]]__'`"

AC_OUTPUT([Makefile src/Makefile])

