if(WITH_SYSTEM_JSONCPP)
  find_library(JSONCPP_LIBRARY jsoncpp)
  if(JSONCPP_LIBRARY MATCHES "JSONCPP_LIBRARY-NOTFOUND")
    message(FATAL_ERROR "Need jsoncpp.")
  endif()
else()
  list(APPEND JSON_SRC
              "jsoncpp/src/lib_json/json_reader.cpp"
              "jsoncpp/src/lib_json/json_value.cpp"
              "jsoncpp/src/lib_json/json_writer.cpp")

  list(APPEND JSON_HPP
              "jsoncpp/include/json/config.h"
              "jsoncpp/include/json/features.h"
              "jsoncpp/include/json/forwards.h"
              "jsoncpp/include/json/json.h"
              "jsoncpp/include/json/reader.h"
              "jsoncpp/include/json/value.h"
              "jsoncpp/include/json/writer.h")

  source_group("" FILES ${JSON_SRC} ${JSON_HPP})

  add_library("jsoncpp" STATIC ${JSON_SRC} ${JSON_HPP})

  set_property(TARGET "jsoncpp" PROPERTY FOLDER "External Libraries")

  disable_project_warnings("jsoncpp")

  target_include_directories("jsoncpp" PUBLIC "jsoncpp/include")

  if(MSVC)
    sm_add_compile_definition("jsoncpp" _CRT_SECURE_NO_WARNINGS)
  elseif(APPLE)
    set_target_properties("jsoncpp"
                          PROPERTIES XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD
                                     "gnu++14"
                                     XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY
                                     "libc++")
    sm_add_compile_flag("jsoncpp" "-std=${SM_CPP_STANDARD}")
    sm_add_compile_flag("jsoncpp" "-stdlib=libc++")
  else() # Unix/Linux
    sm_add_compile_flag("jsoncpp" "-std=gnu++11")
    if(CMAKE_CXX_COMPILER MATCHES "clang")
      sm_add_compile_flag("jsoncpp" "-stdlib=libc++")
    endif()
  endif()
endif()
