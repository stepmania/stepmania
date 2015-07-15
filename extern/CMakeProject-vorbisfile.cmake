set(VORBISFILE_SRC "libvorbis-1.3.5/src/vorbisfile.c")

set(VORBISFILE_HPP "libvorbis-1.3.5/include/vorbis/vorbisfile.h")

source_group("" FILES ${VORBISFILE_SRC} ${VORBISFILE_HPP})

add_library("vorbisfile" ${VORBISFILE_SRC} ${VORBISFILE_HPP})

set_property(TARGET "vorbisfile" PROPERTY FOLDER "External Libraries")

target_include_directories("vorbisfile" PUBLIC "libogg-1.3.2/include")
target_include_directories("vorbisfile" PUBLIC "libvorbis-1.3.5/include")

disable_project_warnings("vorbisfile")
