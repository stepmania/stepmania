set(JSON_DIR "jsoncpp-0.10.4")

set(JSON_SRC
  "${JSON_DIR}/src/lib_json/json_reader.cpp"
  "${JSON_DIR}/src/lib_json/json_value.cpp"
  "${JSON_DIR}/src/lib_json/json_writer.cpp"
)

set(JSON_HPP
  "${JSON_DIR}/src/lib_json/json_tool.h"
  "${JSON_DIR}/src/lib_json/json_valueiterator.inl"
)

set(JSON_API
  "${JSON_DIR}/include/json/assertions.h"
  "${JSON_DIR}/include/json/config.h"
  "${JSON_DIR}/include/json/features.h"
  "${JSON_DIR}/include/json/forwards.h"
  "${JSON_DIR}/include/json/reader.h"
  "${JSON_DIR}/include/json/value.h"
  "${JSON_DIR}/include/json/version.h"
  "${JSON_DIR}/include/json/writer.h"
)

source_group("Source Files" FILES ${JSON_SRC})
source_group("Header Files" FILES ${JSON_HPP})
source_group("Public API" FILES ${JSON_API})

add_library("jsoncpp" ${JSON_SRC} ${JSON_HPP} ${JSON_API})

set_property(TARGET "jsoncpp" PROPERTY FOLDER "External Libraries")

if(MSVC)
  sm_add_compile_definition("jsoncpp" _CRT_SECURE_NO_WARNINGS)
endif()

disable_project_warnings("jsoncpp")

target_include_directories("jsoncpp" PUBLIC "${JSON_DIR}/include")
