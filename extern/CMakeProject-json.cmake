list(APPEND JSON_SRC
  "jsoncpp-0.10.4/src/lib_json/json_reader.cpp"
  "jsoncpp-0.10.4/src/lib_json/json_value.cpp"
  "jsoncpp-0.10.4/src/lib_json/json_writer.cpp"
)

list(APPEND JSON_HPP
  "jsoncpp-0.10.4/src/lib_json/json_tool.h"
  "jsoncpp-0.10.4/src/lib_json/json_valueiterator.inl"
)

list(APPEND JSON_API
  "jsoncpp-0.10.4/include/json/assertions.h"
  "jsoncpp-0.10.4/include/json/config.h"
  "jsoncpp-0.10.4/include/json/features.h"
  "jsoncpp-0.10.4/include/json/forwards.h"
  "jsoncpp-0.10.4/include/json/reader.h"
  "jsoncpp-0.10.4/include/json/value.h"
  "jsoncpp-0.10.4/include/json/version.h"
  "jsoncpp-0.10.4/include/json/writer.h"
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

target_include_directories("jsoncpp" PUBLIC "jsoncpp-0.10.4/include")
