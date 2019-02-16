set(VORBIS_DIR "${SM_EXTERN_DIR}/newvorbis")

list(APPEND VORBIS_SRC
            "${VORBIS_DIR}/lib/analysis.c"
            "${VORBIS_DIR}/lib/bitrate.c"
            "${VORBIS_DIR}/lib/block.c"
            "${VORBIS_DIR}/lib/codebook.c"
            "${VORBIS_DIR}/lib/envelope.c"
            "${VORBIS_DIR}/lib/floor0.c"
            "${VORBIS_DIR}/lib/floor1.c"
            "${VORBIS_DIR}/lib/info.c"
            "${VORBIS_DIR}/lib/lookup.c"
            "${VORBIS_DIR}/lib/lpc.c"
            "${VORBIS_DIR}/lib/lsp.c"
            "${VORBIS_DIR}/lib/mapping0.c"
            "${VORBIS_DIR}/lib/mdct.c"
            "${VORBIS_DIR}/lib/psy.c"
            "${VORBIS_DIR}/lib/registry.c"
            "${VORBIS_DIR}/lib/res0.c"
            "${VORBIS_DIR}/lib/sharedbook.c"
            "${VORBIS_DIR}/lib/smallft.c"
            "${VORBIS_DIR}/lib/synthesis.c"
            "${VORBIS_DIR}/lib/vorbisenc.c"
            "${VORBIS_DIR}/lib/window.c")

list(APPEND VORBIS_HPP
            "${VORBIS_DIR}/lib/backends.h"
            "${VORBIS_DIR}/lib/bitrate.h"
            "${VORBIS_DIR}/lib/codebook.h"
            "${VORBIS_DIR}/include/vorbis/codec.h"
            "${VORBIS_DIR}/lib/codec_internal.h"
            "${VORBIS_DIR}/lib/envelope.h"
            "${VORBIS_DIR}/lib/modes/floor_all.h"
            "${VORBIS_DIR}/lib/books/floor/floor_books.h"
            "${VORBIS_DIR}/lib/highlevel.h"
            "${VORBIS_DIR}/lib/lookup.h"
            "${VORBIS_DIR}/lib/lookup_data.h"
            "${VORBIS_DIR}/lib/lpc.h"
            "${VORBIS_DIR}/lib/lsp.h"
            "${VORBIS_DIR}/lib/masking.h"
            "${VORBIS_DIR}/lib/mdct.h"
            "${VORBIS_DIR}/lib/misc.h"
            "${VORBIS_DIR}/lib/os.h"
            "${VORBIS_DIR}/lib/psy.h"
            "${VORBIS_DIR}/lib/modes/psych_11.h"
            "${VORBIS_DIR}/lib/modes/psych_16.h"
            "${VORBIS_DIR}/lib/modes/psych_44.h"
            "${VORBIS_DIR}/lib/modes/psych_8.h"
            "${VORBIS_DIR}/lib/registry.h"
            "${VORBIS_DIR}/lib/books/coupled/res_books_stereo.h"
            "${VORBIS_DIR}/lib/books/uncoupled/res_books_uncoupled.h"
            "${VORBIS_DIR}/lib/modes/residue_16.h"
            "${VORBIS_DIR}/lib/modes/residue_44.h"
            "${VORBIS_DIR}/lib/modes/residue_44u.h"
            "${VORBIS_DIR}/lib/modes/residue_8.h"
            "${VORBIS_DIR}/lib/scales.h"
            "${VORBIS_DIR}/lib/modes/setup_11.h"
            "${VORBIS_DIR}/lib/modes/setup_16.h"
            "${VORBIS_DIR}/lib/modes/setup_22.h"
            "${VORBIS_DIR}/lib/modes/setup_32.h"
            "${VORBIS_DIR}/lib/modes/setup_44.h"
            "${VORBIS_DIR}/lib/modes/setup_44u.h"
            "${VORBIS_DIR}/lib/modes/setup_8.h"
            "${VORBIS_DIR}/lib/modes/setup_X.h"
            "${VORBIS_DIR}/lib/window.h")

source_group("Source Files" FILES ${VORBIS_SRC})
source_group("Header Files" FILES ${VORBIS_HPP})

add_library("vorbis" ${VORBIS_SRC} ${VORBIS_HPP})

set_property(TARGET "vorbis" PROPERTY FOLDER "External Libraries")

disable_project_warnings("vorbis")

list(APPEND VORBIS_INCLUDE_DIRS
            "${VORBIS_DIR}/lib"
            "${VORBIS_DIR}/include"
            "${SM_EXTERN_DIR}/newogg/include")

target_include_directories("vorbis" PUBLIC ${VORBIS_INCLUDE_DIRS})

list(APPEND VORBIS_LINK_LIBS "ogg")

target_link_libraries("vorbis" ${VORBIS_LINK_LIBS})
