set(VORBISFILE_DIR "${SM_EXTERN_DIR}/newvorbis")

list(APPEND VORBISFILE_SRC "${VORBISFILE_DIR}/lib/vorbisfile.c")

list(APPEND VORBISFILE_HPP "${VORBISFILE_DIR}/include/vorbis/vorbisfile.h")

source_group("Source Files" FILES ${VORBISFILE_SRC})
source_group("Header Files" FILES ${VORBISFILE_HPP})

add_library("vorbisfile" ${VORBISFILE_SRC} ${VORBISFILE_HPP})

set_property(TARGET "vorbisfile" PROPERTY FOLDER "External Libraries")

disable_project_warnings("vorbisfile")

list(APPEND VORBIS_INCLUDE_DIRS "${VORBISDIR_DIR}/include")

target_include_directories("vorbisfile" PUBLIC ${VORBIS_INCLUDE_DIRS})

list(APPEND VORBISFILE_LINK_LIBS "ogg")

target_link_libraries("vorbisfile" ${VORBISFILE_LINK_LIBS})
