list(APPEND JSON_SRC
  "jsoncpp-0.10.4/src/lib_json/json_reader.cpp"
  "jsoncpp-0.10.4/src/lib_json/json_value.cpp"
  "jsoncpp-0.10.4/src/lib_json/json_writer.cpp"
)

list(APPEND JSON_HPP
  "jsoncpp-0.10.4/include/json/assertions.h"
  "jsoncpp-0.10.4/include/json/config.h"
  "jsoncpp-0.10.4/include/json/features.h"
  "jsoncpp-0.10.4/include/json/forwards.h"
  "jsoncpp-0.10.4/include/json/reader.h"
  "jsoncpp-0.10.4/include/json/value.h"
  "jsoncpp-0.10.4/include/json/version.h"
  "jsoncpp-0.10.4/include/json/writer.h"
  "jsoncpp-0.10.4/src/lib_json/json_tool.h"
  "jsoncpp-0.10.4/src/lib_json/json_valueiterator.inl"
  
)

source_group("" FILES ${JSON_SRC} ${JSON_HPP})

add_library("jsoncpp" ${JSON_SRC} ${JSON_HPP})

set_property(TARGET "jsoncpp" PROPERTY FOLDER "External Libraries")

if(MSVC)
  sm_add_compile_definition("jsoncpp" _CRT_SECURE_NO_WARNINGS)
endif()

disable_project_warnings("jsoncpp")

target_include_directories("jsoncpp" PUBLIC "jsoncpp-0.10.4/include")

