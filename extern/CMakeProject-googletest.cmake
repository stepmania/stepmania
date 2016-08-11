if (NOT WITH_UNIT_TESTS)
  # Explicitly skipping unit test support. No need to report a status on this.
  return()
endif()

if (NOT CAN_COMPILE_TESTS)
  message(STATUS "Unit test support disabled. If you wish to compile and run the unit tests, please update your compiler.")
  return()
endif()

if (NOT IS_DIRECTORY "${SM_EXTERN_DIR}/googletest")
  message(ERROR "Submodule for googletest missing. Run git submodule init && git submodule update first.")
  return()
endif()

list(APPEND GOOGLETEST_SRC
  "${SM_EXTERN_DIR}/googletest/googletest/src/gtest-all.cc"
)

source_group("" FILES ${GOOGLETEST_SRC})

add_library("googletest" ${GOOGLETEST_SRC})

set_property(TARGET "googletest" PROPERTY FOLDER "External Libraries")

disable_project_warnings("googletest")

target_include_directories("googletest" SYSTEM PUBLIC "${SM_EXTERN_DIR}/googletest/googletest/include")
target_include_directories("googletest" PUBLIC "${SM_EXTERN_DIR}/googletest/googletest")

if (MSVC)

elseif(APPLE)
  set_target_properties("googletest" PROPERTIES
    XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "${SM_CPP_STANDARD}"
    XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++"
  )
else()
  sm_add_compile_flag("googletest" "-std=${SM_CPP_STANDARD}")
  if (CMAKE_CXX_COMPILER MATCHES "clang")
    sm_add_compile_flag("googletest" "-stdlib=libc++")
  endif()
  if (CMAKE_CXX_COMPILER MATCHES "gcc")
    sm_add_compile_flag("googletest" "-pthread")
  endif()
endif()
