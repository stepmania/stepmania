set(SM_OGG_SRC_DIR "${SM_EXTERN_DIR}/libogg-git")

if (NOT (IS_DIRECTORY "${SM_OGG_SRC_DIR}"))
  message(ERROR "Submodule for ogg missing. Run git submodule init && git submodule update first.")
endif()

externalproject_add("ogg"
  CMAKE_ARGS -DCMAKE_BUILD_TYPE:STRING=Release
  SOURCE_DIR "${SM_OGG_SRC_DIR}"
  UPDATE_COMMAND ""
  INSTALL_COMMAND ""
  TEST_COMMAND ""
)

externalproject_get_property("ogg" BINARY_DIR)
set(SM_OGG_ROOT ${BINARY_DIR})
set(SM_OGG_INCLUDE_DIR "${SM_OGG_SRC_DIR}/include")

set(SM_VORBIS_SRC_DIR "${SM_EXTERN_DIR}/libvorbis-git")

if (NOT (IS_DIRECTORY "${SM_VORBIS_SRC_DIR}"))
  message(ERROR "Submodule for vorbis missing. Run git submodule init && git submodule update first.")
endif()

externalproject_add("vorbis"
  CMAKE_ARGS -DCMAKE_BUILD_TYPE:STRING=Release -DOGG_INCLUDE_DIRS:STRING=${SM_OGG_INCLUDE_DIR} -DOGG_LIBRARIES:STRING=${SM_OGG_ROOT}/$<$<CONFIG:Release>:Release>$<$<CONFIG:Debug>:Debug>$<$<CONFIG:MinSizeRel>:MinSizeRel>$<$<CONFIG:RelWithDebInfo>:RelWithDebInfo>/libogg.a
  SOURCE_DIR "${SM_VORBIS_SRC_DIR}"
  UPDATE_COMMAND ""
  INSTALL_COMMAND ""
  TEST_COMMAND ""
)

externalproject_get_property("vorbis" BINARY_DIR)
set(SM_VORBIS_ROOT ${BINARY_DIR})
set(SM_VORBIS_INCLUDE_DIR "${SM_VORBIS_SRC_DIR}/include")

add_dependencies("vorbis" "ogg")

