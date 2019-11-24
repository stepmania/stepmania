if(WITH_SYSTEM_GLEW)
  find_package(GLEW REQUIRED)
  set(GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIRS} PARENT_SCOPE)
  set(GLEW_LIBRARIES ${GLEW_LIBRARIES} PARENT_SCOPE)
  set(GLEW_LIBRARY_RELEASE ${GLEW_LIBRARY_RELEASE} PARENT_SCOPE)
else()

  set(GLEW_SRC "glew-1.5.8/src/glew.c")

  source_group("" FILES ${GLEW_SRC})

  add_library("glew" STATIC ${GLEW_SRC})

  set_property(TARGET "glew" PROPERTY FOLDER "External Libraries")

  target_include_directories("glew" PUBLIC "glew-1.5.8/include")

  sm_add_compile_definition("glew" GLEW_STATIC)

  if(MSVC)
    sm_add_compile_definition("glew" _MBCS)
  endif(MSVC)

endif()
