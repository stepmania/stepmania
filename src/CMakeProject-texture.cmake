if(NOT MSVC)
  return()
endif()

set(TEXTURE_DIR "${SM_SRC_DIR}/Texture Font Generator")

list(APPEND TEXTURE_SRC
            "${TEXTURE_DIR}/stdafx.cpp"
            "${TEXTURE_DIR}/Texture Font Generator.cpp"
            "${TEXTURE_DIR}/Texture Font GeneratorDlg.cpp"
            "${TEXTURE_DIR}/TextureFont.cpp"
            "${TEXTURE_DIR}/Utils.cpp")

list(APPEND TEXTURE_HPP
            "${TEXTURE_DIR}/Resource.h"
            "${TEXTURE_DIR}/stdafx.h"
            "${TEXTURE_DIR}/Texture Font Generator.h"
            "${TEXTURE_DIR}/Texture Font GeneratorDlg.h"
            "${TEXTURE_DIR}/TextureFont.h"
            "${TEXTURE_DIR}/Utils.h")

list(APPEND TEXTURE_DAT
            "${TEXTURE_DIR}/res/Texture Font Generator.ico"
            "${TEXTURE_DIR}/Texture Font Generator.rc"
            "${TEXTURE_DIR}/res/Texture Font Generator.rc2")

source_group("Source Files" FILES ${TEXTURE_SRC} ${TEXTURE_HPP})
source_group("Resource Files" FILES ${TEXTURE_DAT})
set(CMAKE_MFC_FLAG 2)
add_executable("TextureFontGenerator"
               WIN32
               ${TEXTURE_SRC}
               ${TEXTURE_HPP}
               ${TEXTURE_DAT})
unset(CMAKE_MFC_FLAG)
set_property(TARGET "TextureFontGenerator" PROPERTY FOLDER "Internal Libraries")

disable_project_warnings("TextureFontGenerator")

list(APPEND TEXTURE_LINK_LIB "zlib" "png")

target_link_libraries("TextureFontGenerator" ${TEXTURE_LINK_LIB})

list(APPEND TEXTURE_INCLUDE_DIRS
            "${TEXTURE_DIR}"
            "${TEXTURE_DIR}/res"
            "${SM_GENERATED_SRC_DIR}")

target_include_directories("TextureFontGenerator"
                           PUBLIC ${TEXTURE_INCLUDE_DIRS})

set_target_properties("TextureFontGenerator"
                      PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                 "${SM_PROGRAM_DIR}"
                                 RUNTIME_OUTPUT_DIRECTORY_RELEASE
                                 "${SM_PROGRAM_DIR}"
                                 RUNTIME_OUTPUT_DIRECTORY_DEBUG
                                 "${SM_PROGRAM_DIR}"
                                 RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL
                                 "${SM_PROGRAM_DIR}"
                                 RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO
                                 "${SM_PROGRAM_DIR}")

set_target_properties("TextureFontGenerator"
                      PROPERTIES OUTPUT_NAME
                                 "Texture Font Generator"
                                 RELEASE_OUTPUT_NAME
                                 "Texture Font Generator"
                                 DEBUG_OUTPUT_NAME
                                 "Texture Font Generator"
                                 MINSIZEREL_OUTPUT_NAME
                                 "Texture Font Generator"
                                 RELWITHDEBINFO_OUTPUT_NAME
                                 "Texture Font Generator")
