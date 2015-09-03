if(NOT GTK3_FOUND)
  return()
endif()

add_library("GtkModule"
  SHARED
  "arch/LoadingWindow/LoadingWindow_GtkModule.cpp"
  "arch/LoadingWindow/LoadingWindow_GtkModule.h"
)
sm_add_compile_definition("GtkModule" "${ENDIANNESS}")

sm_add_compile_flag("GtkModule" "-std=gnu++11")
if (CMAKE_CXX_COMPILER MATCHES "clang")
  sm_add_compile_flag("GtkModule" "-stdlib=libc++")
  set_target_properties("GtkModule" PROPERTIES LINK_FLAGS "-stdlib=libc++")
endif()

# It is normally not appropriate to set the prefix to the empty string.
# This is to maintain compatibility with the current source.
# At some point, it may be worth being more flexible.
set_target_properties("GtkModule" PROPERTIES PREFIX "")
set_target_properties("GtkModule" PROPERTIES OUTPUT_NAME "GtkModule")
set_target_properties("GtkModule" PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${SM_ROOT_DIR}")
set_target_properties("GtkModule" PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE "${SM_ROOT_DIR}")
set_target_properties("GtkModule" PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG "${SM_ROOT_DIR}")
set_target_properties("GtkModule" PROPERTIES LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${SM_ROOT_DIR}")
set_target_properties("GtkModule" PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${SM_ROOT_DIR}")
target_link_libraries("GtkModule" ${GTK3_LIBRARIES})
set_property(TARGET "GtkModule" PROPERTY FOLDER "Internal Libraries")
list(APPEND SM_GTK_INCLUDE_DIRS
  "${SM_SRC_DIR}"
  "${SM_SRC_DIR}/arch/LoadingWindow"
  "${GTK3_INCLUDE_DIRS}"
)
target_include_directories("GtkModule" PUBLIC ${SM_GTK_INCLUDE_DIRS})
