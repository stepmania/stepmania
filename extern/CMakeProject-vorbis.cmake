list(APPEND VORBIS_SRC
  "libvorbis-1.3.5/src/analysis.c"
  "libvorbis-1.3.5/src/bitrate.c"
  "libvorbis-1.3.5/src/block.c"
  "libvorbis-1.3.5/src/codebook.c"
  "libvorbis-1.3.5/src/envelope.c"
  "libvorbis-1.3.5/src/floor0.c"
  "libvorbis-1.3.5/src/floor1.c"
  "libvorbis-1.3.5/src/info.c"
  "libvorbis-1.3.5/src/lookup.c"
  "libvorbis-1.3.5/src/lpc.c"
  "libvorbis-1.3.5/src/lsp.c"
  "libvorbis-1.3.5/src/mapping0.c"
  "libvorbis-1.3.5/src/mdct.c"
  "libvorbis-1.3.5/src/psy.c"
  "libvorbis-1.3.5/src/registry.c"
  "libvorbis-1.3.5/src/res0.c"
  "libvorbis-1.3.5/src/sharedbook.c"
  "libvorbis-1.3.5/src/smallft.c"
  "libvorbis-1.3.5/src/synthesis.c"
  "libvorbis-1.3.5/src/vorbisenc.c"
  "libvorbis-1.3.5/src/window.c"
)

list(APPEND VORBIS_HPP
  "libvorbis-1.3.5/src/backends.h"
  "libvorbis-1.3.5/src/bitrate.h"
  "libvorbis-1.3.5/src/codebook.h"
  "libvorbis-1.3.5/include/vorbis/codec.h"
  "libvorbis-1.3.5/src/codec_internal.h"
  "libvorbis-1.3.5/src/envelope.h"
  "libvorbis-1.3.5/src/modes/floor_all.h"
  "libvorbis-1.3.5/src/books/floor/floor_books.h"
  "libvorbis-1.3.5/src/highlevel.h"
  "libvorbis-1.3.5/src/lookup.h"
  "libvorbis-1.3.5/src/lookup_data.h"
  "libvorbis-1.3.5/src/lpc.h"
  "libvorbis-1.3.5/src/lsp.h"
  "libvorbis-1.3.5/src/masking.h"
  "libvorbis-1.3.5/src/mdct.h"
  "libvorbis-1.3.5/src/misc.h"
  "libvorbis-1.3.5/src/os.h"
  "libvorbis-1.3.5/src/psy.h"
  "libvorbis-1.3.5/src/modes/psych_11.h"
  "libvorbis-1.3.5/src/modes/psych_16.h"
  "libvorbis-1.3.5/src/modes/psych_44.h"
  "libvorbis-1.3.5/src/modes/psych_8.h"
  "libvorbis-1.3.5/src/registry.h"
  "libvorbis-1.3.5/src/books/coupled/res_books_stereo.h"
  "libvorbis-1.3.5/src/books/uncoupled/res_books_uncoupled.h"
  "libvorbis-1.3.5/src/modes/residue_16.h"
  "libvorbis-1.3.5/src/modes/residue_44.h"
  "libvorbis-1.3.5/src/modes/residue_44u.h"
  "libvorbis-1.3.5/src/modes/residue_8.h"
  "libvorbis-1.3.5/src/scales.h"
  "libvorbis-1.3.5/src/modes/setup_11.h"
  "libvorbis-1.3.5/src/modes/setup_16.h"
  "libvorbis-1.3.5/src/modes/setup_22.h"
  "libvorbis-1.3.5/src/modes/setup_32.h"
  "libvorbis-1.3.5/src/modes/setup_44.h"
  "libvorbis-1.3.5/src/modes/setup_44u.h"
  "libvorbis-1.3.5/src/modes/setup_8.h"
  "libvorbis-1.3.5/src/modes/setup_X.h"
  "libvorbis-1.3.5/src/smallft.h"
  "libvorbis-1.3.5/include/vorbis/vorbisenc.h"
  "libvorbis-1.3.5/include/vorbis/vorbisfile.h"
  "libvorbis-1.3.5/src/window.h"
)

source_group("" FILES ${VORBIS_SRC} ${VORBIS_HPP})

add_library("vorbis" ${VORBIS_SRC} ${VORBIS_HPP})

set_property(TARGET "vorbis" PROPERTY FOLDER "External Libraries")

target_include_directories("vorbis" PUBLIC "libogg-1.3.2/include")
target_include_directories("vorbis" PUBLIC "libvorbis-1.3.5/include")

disable_project_warnings("vorbis")
