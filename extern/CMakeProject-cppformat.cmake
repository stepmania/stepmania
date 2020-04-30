if(NOT IS_DIRECTORY "${SM_EXTERN_DIR}/cppformat")
  message(
    ERROR
    "Submodule for cppformat missing. Run git submodule init && git submodule update first."
    )
  return()
endif()

list(APPEND CPPFORMAT_SRC "cppformat/fmt/format.cc")

list(APPEND CPPFORMAT_HPP "cppformat/fmt/format.h")

source_group("" FILES ${CPPFORMAT_SRC} ${CPPFORMAT_HPP})

add_library("cppformat" ${CPPFORMAT_SRC} ${CPPFORMAT_HPP})

set_property(TARGET "cppformat" PROPERTY FOLDER "External Libraries")

disable_project_warnings("cppformat")

target_include_directories("cppformat" PUBLIC "cppformat")

if(MSVC)
  sm_add_compile_definition("cppformat" _CRT_SECURE_NO_WARNINGS)
elseif(APPLE)
  set_target_properties("cppformat"
                        PROPERTIES XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD
                                   "${SM_CPP_STANDARD}"
                                   XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY
                                   "libc++")
  sm_add_compile_flag("cppformat" "-std=${SM_CPP_STANDARD}")
  sm_add_compile_flag("cppformat" "-stdlib=libc++")
else() # Unix
  sm_add_compile_flag("cppformat" "-std=${SM_CPP_STANDARD}")
endif()
