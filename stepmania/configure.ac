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
AC_SEARCH_LIBS(id3_tag_parse, [id3tag], have_mad=yes,  have_mad=no)
AC_SEARCH_LIBS(deflate, [z], have_libz=yes,  have_libz=no)

AC_PROG_CC
AC_PROG_CXX

AM_PATH_SDL

# sdl-config puts -L/usr/lib in the library search path, which reorders things
# in a way that breaks some configurations.
SDL_LIBS="`echo $SDL_LIBS | sed 's_-L/usr/lib/\?[[ $]]__'`"


# For now, we're only using autoconf on Linux
AH_TEMPLATE([WIN32],
	[Indicates compilation on Windows])
AH_TEMPLATE([LINUX],
	[Indicates compilation on Linux])
AC_DEFINE(LINUX)


# Some extra defines
AH_TEMPLATE([GL_COLOR_TABLE_FORMAT_EXT],
	[Some systems (ie, Linux) don't define GL_COLOR_TABLE_FORMAT_EXT.
	 In these cases, use GL_COLOR_TABLE_FORMAT instead])
AC_CHECK_DECL(GL_COLOR_TABLE_FORMAT_EXT, [],
	AC_DEFINE(GL_COLOR_TABLE_FORMAT_EXT,GL_COLOR_TABLE_FORMAT) )

AC_OUTPUT([Makefile src/Makefile])

