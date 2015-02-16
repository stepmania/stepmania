# Borrowed from orxonox.

# Several changes and additions by Fabian 'x3n' Landau
#                 > www.orxonox.net <

IF (OGG_INCLUDE_DIR AND OGG_LIBRARY)
  SET(OGG_FIND_QUIETLY TRUE)
ENDIF (OGG_INCLUDE_DIR AND OGG_LIBRARY)

IF (VORBIS_INCLUDE_DIR AND VORBIS_LIBRARY AND VORBISENC_LIBRARY AND VORBISFILE_LIBRARY)
  SET(VORBIS_FIND_QUIETLY TRUE)
ENDIF (VORBIS_INCLUDE_DIR AND VORBIS_LIBRARY AND VORBISENC_LIBRARY AND VORBISFILE_LIBRARY) 


#SET(OGGVORBIS_LIBRARY_DIR "/usr/pack/oggvorbis-1.0-ds/i686-debian-linux3.0/lib")
#SET(OGGVORBIS_LIBRARY "-L ${OGGVORBIS_LIBRARY_DIR} -lvorbisenc -lvorbisfile -logg -lvorbis ")
#SET(OGGVORBIS_INCLUDE_DIR "/usr/pack/oggvorbis-1.0-ds/i686-debian-linux3.0/include")

#
# Includes
#

FIND_PATH(OGG_INCLUDE_DIR ogg/ogg.h
  /usr/pack/oggvorbis-1.0-ds/i686-debian-linux3.0/include 		# Tardis specific hack
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/csw/include
  /opt/include
  ../libs/libogg-1.1.3/include
  ${DEPENDENCY_DIR}/libogg-1.1.3/include
  )

FIND_PATH(VORBIS_INCLUDE_DIR vorbis/codec.h
  /usr/pack/oggvorbis-1.0-ds/i686-debian-linux3.0/include 		# Tardis specific hack
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/csw/include
  /opt/include
  ../libs/libvorbis-1.2.0/include
  ${DEPENDENCY_DIR}/libvorbis-1.2.0/include
  )

#
# Libs
#

FIND_LIBRARY(OGG_LIBRARY
  NAMES ogg
  PATHS
  /usr/pack/oggvorbis-1.0-ds/i686-debian-linux3.1/lib
  /usr/local/lib
  /usr/lib
  /sw/lib
  /opt/local/lib
  /opt/csw/lib
  /opt/lib
  ../libs/libogg-1.1.3/src/.libs
  ${DEPENDENCY_DIR}/libogg-1.1.3/lib
  )

FIND_LIBRARY(VORBIS_LIBRARY
  NAMES vorbis
  PATHS
  /usr/pack/oggvorbis-1.0-ds/i686-debian-linux3.1/lib
  /usr/local/lib
  /usr/lib
  /sw/lib
  /opt/local/lib
  /opt/csw/lib
  /opt/lib
  ../libs/libvorbis-1.2.0/lib/.libs
  ${DEPENDENCY_DIR}/libvorbis-1.2.0/lib
  )

#FIND_LIBRARY(VORBISENC_LIBRARY
#  NAMES vorbisenc
#  PATHS
#  /usr/pack/oggvorbis-1.0-ds/i686-debian-linux3.1/lib
#  /usr/local/lib
#  /usr/lib
#  /sw/lib
#  /opt/local/lib
#  /opt/csw/lib
#  /opt/lib
#  ../libs/libvorbis-1.2.0/lib/.libs
#  )

FIND_LIBRARY(VORBISFILE_LIBRARY
  NAMES vorbisfile
  PATHS
  /usr/pack/oggvorbis-1.0-ds/i686-debian-linux3.1/lib
  /usr/local/lib
  /usr/lib
  /sw/lib
  /opt/local/lib
  /opt/csw/lib
  /opt/lib
  ../libs/libvorbis-1.2.0/lib/.libs
  ${DEPENDENCY_DIR}/libvorbis-1.2.0/lib
  )

SET (OGG_FOUND "NO")

IF (OGG_INCLUDE_DIR AND OGG_LIBRARY)
  SET (OGG_FOUND "YES")
  IF (NOT OGG_FIND_QUIETLY)
    MESSAGE (STATUS "Ogg was found.")
    IF (VERBOSE_FIND)
      MESSAGE (STATUS "  include path: ${OGG_INCLUDE_DIR}")
      MESSAGE (STATUS "  library path: ${OGG_LIBRARY}")
      MESSAGE (STATUS "  libraries:    ogg")
    ENDIF (VERBOSE_FIND)
  ENDIF (NOT OGG_FIND_QUIETLY)
ELSE (OGG_INCLUDE_DIR AND OGG_LIBRARY)
  IF (NOT OGG_INCLUDE_DIR)
    MESSAGE (SEND_ERROR "Ogg include path was not found.")
  ENDIF (NOT OGG_INCLUDE_DIR)
  IF (NOT OGG_LIBRARY)
    MESSAGE (SEND_ERROR "Ogg library was not found.")
  ENDIF (NOT OGG_LIBRARY)
ENDIF (OGG_INCLUDE_DIR AND OGG_LIBRARY)


SET (VORBIS_FOUND "NO")

IF (VORBIS_INCLUDE_DIR AND VORBIS_LIBRARY AND VORBISFILE_LIBRARY)# AND VORBISENC_LIBRARY)
  SET (VORBIS_FOUND "YES")
  IF (NOT VORBIS_FIND_QUIETLY)
    MESSAGE (STATUS "Vorbis was found.")
    IF (VERBOSE_FIND)
      MESSAGE (STATUS "  include path: ${VORBIS_INCLUDE_DIR}")
      MESSAGE (STATUS "  library path: ${VORBIS_LIBRARY}")
      #MESSAGE (STATUS "  library path: ${VORBISENC_LIBRARY}")
      MESSAGE (STATUS "  library path: ${VORBISFILE_LIBRARY}")
      MESSAGE (STATUS "  libraries:    vorbis vorbisenc vorbisfile")
    ENDIF (VERBOSE_FIND)
  ENDIF (NOT VORBIS_FIND_QUIETLY)
ELSE (VORBIS_INCLUDE_DIR AND VORBIS_LIBRARY AND VORBISFILE_LIBRARY)# AND VORBISENC_LIBRARY)
  IF (NOT VORBIS_INCLUDE_DIR)
    MESSAGE (SEND_ERROR "Vorbis include path was not found.")
  ENDIF (NOT VORBIS_INCLUDE_DIR)
  IF (NOT VORBIS_LIBRARY)
    MESSAGE (SEND_ERROR "Vorbis library \"vorbis\" was not found.")
  ENDIF (NOT VORBIS_LIBRARY)
  #IF (NOT VORBISENC_LIBRARY)
  #  MESSAGE (SEND_ERROR "Vorbis library \"vorbisenc\" was not found.")
  #ENDIF (NOT VORBISENC_LIBRARY)
  IF (NOT VORBISFILE_LIBRARY)
    MESSAGE (SEND_ERROR "Vorbis library \"vorbisfile\" was not found.")
  ENDIF (NOT VORBISFILE_LIBRARY)
ENDIF (VORBIS_INCLUDE_DIR AND VORBIS_LIBRARY AND VORBISFILE_LIBRARY)# AND VORBISENC_LIBRARY)

