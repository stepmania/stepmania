list(APPEND RAGE_SRC
  "${SM_SRC_DIR}/rage/RageColor.cpp"
  "${SM_SRC_DIR}/rage/RageVector2.cpp"
  "${SM_SRC_DIR}/rage/RageVector3.cpp"
  "${SM_SRC_DIR}/rage/RageVector4.cpp"
)

list(APPEND RAGE_HPP
  "${SM_SRC_DIR}/rage/RageColor.hpp"
  "${SM_SRC_DIR}/rage/RageMath.hpp"
  "${SM_SRC_DIR}/rage/RageVector2.hpp"
  "${SM_SRC_DIR}/rage/RageVector3.hpp"
  "${SM_SRC_DIR}/rage/RageVector4.hpp"
)

source_group("" FILES ${RAGE_SRC} ${RAGE_HPP})

add_library("rage" ${RAGE_SRC} ${RAGE_HPP})

set_property(TARGET "rage" PROPERTY FOLDER "Internal Libraries")

list(APPEND RAGE_INCLUDE_DIRS
  "${SM_EXTERNDIR}/cppformat"
)

target_include_directories("rage" PUBLIC ${RAGE_INCLUDE_DIRS})

list(APPEND RAGE_LINK_LIB
  "cppformat"
)

target_link_libraries("rage" ${RAGE_LINK_LIB})

if (MSVC)

elseif(APPLE)
  set_target_properties("rage" PROPERTIES
    XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "gnu++14"
    XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++"
  )
else()
  sm_add_compile_flag("rage" "-std=gnu++11")
  if (CMAKE_CXX_COMPILER MATCHES "clang")
    sm_add_compile_flag("rage" "-stdlib=libc++")
  endif()
endif()
