set(VORBISFILE_SRC "${VORBIS_DIR}/src/vorbisfile.c")

set(VORBISFILE_HPP "${VORBIS_DIR}/include/vorbis/vorbisfile.h")

source_group("" FILES ${VORBISFILE_SRC} ${VORBISFILE_HPP})

add_library("vorbisfile" ${VORBISFILE_SRC} ${VORBISFILE_HPP})

set_property(TARGET "vorbisfile" PROPERTY FOLDER "External Libraries")

disable_project_warnings("vorbisfile")

target_include_directories("vorbisfile" PUBLIC "${OGG_DIR}/include")
target_include_directories("vorbisfile" PUBLIC "${VORBIS_DIR}/include")
