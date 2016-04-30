list(APPEND RAGE_TEST_SRC
  "${SM_SRC_DIR}/tests/RageColorFixture.cpp"
  "${SM_SRC_DIR}/tests/RageColorTest.cpp"
  "${SM_SRC_DIR}/tests/RageMathTest.cpp"
  "${SM_SRC_DIR}/tests/RageModelVertexTest.cpp"
  "${SM_SRC_DIR}/tests/RageRectTest.cpp"
  "${SM_SRC_DIR}/tests/RageSpriteVertexTest.cpp"
  "${SM_SRC_DIR}/tests/RageStringTest.cpp"
  "${SM_SRC_DIR}/tests/RageVColorTest.cpp"
  "${SM_SRC_DIR}/tests/RageVector2Fixture.cpp"
  "${SM_SRC_DIR}/tests/RageVector2Test.cpp"
  "${SM_SRC_DIR}/tests/RageVector3Fixture.cpp"
  "${SM_SRC_DIR}/tests/RageVector3Test.cpp"
  "${SM_SRC_DIR}/tests/RageVector4Fixture.cpp"
  "${SM_SRC_DIR}/tests/RageVector4Test.cpp"
  "${SM_SRC_DIR}/tests/rage-test-main.cpp"
)

list(APPEND RAGE_TEST_HPP
  "${SM_SRC_DIR}/tests/RageColorFixture.hpp"
  "${SM_SRC_DIR}/tests/RageVector2Fixture.hpp"
  "${SM_SRC_DIR}/tests/RageVector3Fixture.hpp"
  "${SM_SRC_DIR}/tests/RageVector4Fixture.hpp"
)

source_group("" FILES ${RAGE_TEST_SRC} ${RAGE_TEST_HPP})

add_executable("rage-test" ${RAGE_TEST_SRC} ${RAGE_TEST_HPP})

set_property(TARGET "rage-test" PROPERTY FOLDER "Internal Libraries")

list(APPEND RAGE_TEST_LINK_LIB
  "${CMAKE_THREAD_LIBS_INIT}"
  "googletest"
  "rage"
)

target_link_libraries("rage-test" ${RAGE_TEST_LINK_LIB})

list(APPEND RAGE_TEST_INCLUDE_DIRS
  "${SM_EXTERN_DIR}/googletest/googletest/include"
  "${SM_SRC_DIR}/rage"
)

target_include_directories("rage-test" PUBLIC ${RAGE_TEST_INCLUDE_DIRS})

if (MSVC)

elseif(APPLE)
  set_target_properties("rage-test" PROPERTIES
    XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "${SM_CPP_STANDARD}"
    XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++"
  )
else()
  sm_add_compile_flag("rage-test" "-std=${SM_CPP_STANDARD}")
  if (CMAKE_CXX_COMPILER MATCHES "clang")
    sm_add_compile_flag("rage-test" "-stdlib=libc++")
    set_target_properties("rage-test" PROPERTIES LINK_FLAGS "-stdlib=libc++")
  endif()
  if (CMAKE_CXX_COMPILER MATCHES "gcc")
    sm_add_compile_flag("rage-test" "-pthread")
  endif()
endif()
