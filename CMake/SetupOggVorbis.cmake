set(SM_OGG_SRC_DIR "${SM_EXTERN_DIR}/libogg-git")

if(NOT (IS_DIRECTORY "${SM_OGG_SRC_DIR}"))
  message(
    ERROR
    "Submodule for ogg missing. Run git submodule init && git submodule update first."
    )
endif()

externalproject_add("ogg"
                    CMAKE_ARGS
                    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                    SOURCE_DIR
                    "${SM_OGG_SRC_DIR}"
                    UPDATE_COMMAND
                    ""
                    INSTALL_COMMAND
                    ""
                    TEST_COMMAND
                    "")

externalproject_get_property("ogg" BINARY_DIR)
set(SM_OGG_ROOT ${BINARY_DIR})
set(SM_OGG_INCLUDE_DIR "${SM_OGG_SRC_DIR}/include")

if(APPLE AND CMAKE_GENERATOR MATCHES "Unix Makefiles")
  # Xcode does this and CMake is somehow aware, but with Unix Makefiles this is
  # necessary for the time being.
  externalproject_add_step("ogg"
                           fix-path
                           COMMAND
                           ${CMAKE_COMMAND}
                           -E
                           make_directory
                           ${SM_OGG_ROOT}/${CMAKE_BUILD_TYPE}
                           DEPENDEES
                           build)
  externalproject_add_step("ogg"
                           copy-libogg
                           COMMAND
                           ${CMAKE_COMMAND}
                           -E
                           copy
                           ${SM_OGG_ROOT}/libogg.a
                           ${SM_OGG_ROOT}/${CMAKE_BUILD_TYPE}/
                           DEPENDEES
                           fix-path)
endif()

set(SM_VORBIS_SRC_DIR "${SM_EXTERN_DIR}/libvorbis-git")

if(NOT (IS_DIRECTORY "${SM_VORBIS_SRC_DIR}"))
  message(
    ERROR
    "Submodule for vorbis missing. Run git submodule init && git submodule update first."
    )
endif()

externalproject_add(
  "vorbis"
  CMAKE_ARGS
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DOGG_INCLUDE_DIRS:STRING=${SM_OGG_INCLUDE_DIR}
  -DOGG_LIBRARIES:STRING=${SM_OGG_ROOT}/${CMAKE_BUILD_TYPE}/libogg.a
  SOURCE_DIR
  "${SM_VORBIS_SRC_DIR}"
  UPDATE_COMMAND
  ""
  INSTALL_COMMAND
  ""
  TEST_COMMAND
  "")

externalproject_get_property("vorbis" BINARY_DIR)
set(SM_VORBIS_ROOT ${BINARY_DIR})
set(SM_VORBIS_INCLUDE_DIR "${SM_VORBIS_SRC_DIR}/include")

if(APPLE AND CMAKE_GENERATOR MATCHES "Unix Makefiles")
  # See note above
  externalproject_add_step("vorbis"
                           fix-path
                           COMMAND
                           ${CMAKE_COMMAND}
                           -E
                           make_directory
                           ${SM_VORBIS_ROOT}/lib/${CMAKE_BUILD_TYPE}
                           DEPENDEES
                           build)
  externalproject_add_step("vorbis"
                           copy-libvorbis
                           COMMAND
                           ${CMAKE_COMMAND}
                           -E
                           copy
                           ${SM_VORBIS_ROOT}/lib/libvorbis.a
                           ${SM_VORBIS_ROOT}/lib/${CMAKE_BUILD_TYPE}/
                           DEPENDEES
                           fix-path)
  externalproject_add_step("vorbis"
                           copy-libvorbisfile
                           COMMAND
                           ${CMAKE_COMMAND}
                           -E
                           copy
                           ${SM_VORBIS_ROOT}/lib/libvorbisfile.a
                           ${SM_VORBIS_ROOT}/lib/${CMAKE_BUILD_TYPE}/
                           DEPENDEES
                           fix-path)
endif()

add_dependencies("vorbis" "ogg")
