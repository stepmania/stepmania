list(APPEND JSON_SRC
  "jsoncpp/src/lib_json/json_reader.cpp"
  "jsoncpp/src/lib_json/json_value.cpp"
  "jsoncpp/src/lib_json/json_writer.cpp"
)

list(APPEND JSON_HPP
  "jsoncpp/include/json/config.h"
  "jsoncpp/include/json/features.h"
  "jsoncpp/include/json/forwards.h"
  "jsoncpp/include/json/json.h"
  "jsoncpp/include/json/reader.h"
  "jsoncpp/include/json/value.h"
  "jsoncpp/include/json/writer.h"
)

source_group("" FILES ${JSON_SRC} ${JSON_HPP})

add_library("jsoncpp" ${JSON_SRC} ${JSON_HPP})

set_property(TARGET "jsoncpp" PROPERTY FOLDER "External Libraries")

if(MSVC)
  sm_add_compile_definition("jsoncpp" _CRT_SECURE_NO_WARNINGS)
endif()

disable_project_warnings("jsoncpp")

target_include_directories("jsoncpp" PUBLIC "jsoncpp/include")

