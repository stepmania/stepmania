set(VORBISFILE_SRC "vorbis/lib/vorbisfile.c")

set(VORBISFILE_HPP "vorbis/include/vorbis/vorbisfile.h")

source_group("Source Files" FILES ${VORBISFILE_SRC})
source_group("Header Files" FILES ${VORBISFILE_HPP})

add_library("vorbisfile" STATIC ${VORBISFILE_SRC} ${VORBISFILE_HPP})

set_property(TARGET "vorbisfile" PROPERTY FOLDER "External Libraries")

disable_project_warnings("vorbisfile")

target_include_directories("vorbisfile" PUBLIC "vorbis/include")

target_link_libraries("vorbisfile" "ogg")
