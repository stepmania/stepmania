set(VORBIS_DIR "libvorbis-1.3.5")

set(VORBIS_SRC
  "${VORBIS_DIR}/src/analysis.c"
  "${VORBIS_DIR}/src/bitrate.c"
  "${VORBIS_DIR}/src/block.c"
  "${VORBIS_DIR}/src/codebook.c"
  "${VORBIS_DIR}/src/envelope.c"
  "${VORBIS_DIR}/src/floor0.c"
  "${VORBIS_DIR}/src/floor1.c"
  "${VORBIS_DIR}/src/info.c"
  "${VORBIS_DIR}/src/lookup.c"
  "${VORBIS_DIR}/src/lpc.c"
  "${VORBIS_DIR}/src/lsp.c"
  "${VORBIS_DIR}/src/mapping0.c"
  "${VORBIS_DIR}/src/mdct.c"
  "${VORBIS_DIR}/src/psy.c"
  "${VORBIS_DIR}/src/registry.c"
  "${VORBIS_DIR}/src/res0.c"
  "${VORBIS_DIR}/src/sharedbook.c"
  "${VORBIS_DIR}/src/smallft.c"
  "${VORBIS_DIR}/src/synthesis.c"
  "${VORBIS_DIR}/src/vorbisenc.c"
  "${VORBIS_DIR}/src/window.c"
)

set(VORBIS_HPP
  "${VORBIS_DIR}/src/backends.h"
  "${VORBIS_DIR}/src/bitrate.h"
  "${VORBIS_DIR}/src/codebook.h"
  "${VORBIS_DIR}/include/vorbis/codec.h"
  "${VORBIS_DIR}/src/codec_internal.h"
  "${VORBIS_DIR}/src/envelope.h"
  "${VORBIS_DIR}/src/modes/floor_all.h"
  "${VORBIS_DIR}/src/books/floor/floor_books.h"
  "${VORBIS_DIR}/src/highlevel.h"
  "${VORBIS_DIR}/src/lookup.h"
  "${VORBIS_DIR}/src/lookup_data.h"
  "${VORBIS_DIR}/src/lpc.h"
  "${VORBIS_DIR}/src/lsp.h"
  "${VORBIS_DIR}/src/masking.h"
  "${VORBIS_DIR}/src/mdct.h"
  "${VORBIS_DIR}/src/misc.h"
  "${VORBIS_DIR}/src/os.h"
  "${VORBIS_DIR}/src/psy.h"
  "${VORBIS_DIR}/src/modes/psych_11.h"
  "${VORBIS_DIR}/src/modes/psych_16.h"
  "${VORBIS_DIR}/src/modes/psych_44.h"
  "${VORBIS_DIR}/src/modes/psych_8.h"
  "${VORBIS_DIR}/src/registry.h"
  "${VORBIS_DIR}/src/books/coupled/res_books_stereo.h"
  "${VORBIS_DIR}/src/books/uncoupled/res_books_uncoupled.h"
  "${VORBIS_DIR}/src/modes/residue_16.h"
  "${VORBIS_DIR}/src/modes/residue_44.h"
  "${VORBIS_DIR}/src/modes/residue_44u.h"
  "${VORBIS_DIR}/src/modes/residue_8.h"
  "${VORBIS_DIR}/src/scales.h"
  "${VORBIS_DIR}/src/modes/setup_11.h"
  "${VORBIS_DIR}/src/modes/setup_16.h"
  "${VORBIS_DIR}/src/modes/setup_22.h"
  "${VORBIS_DIR}/src/modes/setup_32.h"
  "${VORBIS_DIR}/src/modes/setup_44.h"
  "${VORBIS_DIR}/src/modes/setup_44u.h"
  "${VORBIS_DIR}/src/modes/setup_8.h"
  "${VORBIS_DIR}/src/modes/setup_X.h"
  "${VORBIS_DIR}/src/smallft.h"
  "${VORBIS_DIR}/include/vorbis/vorbisenc.h"
  "${VORBIS_DIR}/include/vorbis/vorbisfile.h"
  "${VORBIS_DIR}/src/window.h"
)

source_group("" FILES ${VORBIS_SRC} ${VORBIS_HPP})

add_library("vorbis" ${VORBIS_SRC} ${VORBIS_HPP})

set_property(TARGET "vorbis" PROPERTY FOLDER "External Libraries")

disable_project_warnings("vorbis")

target_include_directories("vorbis" PUBLIC "${OGG_DIR}/include")
target_include_directories("vorbis" PUBLIC "${VORBIS_DIR}/include")
