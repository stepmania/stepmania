function(sm_append_simple_target_property target property str)
  get_target_property(current_property ${target} ${property})
  if(current_property)
    list(APPEND current_property ${str})
    set_target_properties(${target}
                          PROPERTIES ${property} "${current_property}")
  else()
    set_target_properties(${target} PROPERTIES ${property} ${str})
  endif()
endfunction()

function(sm_add_link_flag target flag)
  if(MSVC)
    # Use a modified form of sm_append_simple_target_property.
    get_target_property(current_property ${target} LINK_FLAGS)
    if(current_property)
      set_target_properties(${target}
                            PROPERTIES LINK_FLAGS "${current_property} ${flag}")
    else()
      set_target_properties(${target} PROPERTIES LINK_FLAGS ${flag})
    endif()
  else()
    sm_append_simple_target_property(${target} LINK_FLAGS ${flag})
  endif()
endfunction()

function(disable_project_warnings projectName)
  if(NOT WITH_EXTERNAL_WARNINGS)
    if(MSVC)
      target_compile_options(${projectName} PRIVATE "/W0")
    elseif(CMAKE_GENERATOR STREQUAL Xcode)
      set_property(TARGET ${projectName} PROPERTY XCODE_ATTRIBUTE_GCC_WARN_INHIBIT_ALL_WARNINGS "YES")
    else()
      target_compile_options(${projectName} PRIVATE "-w")
    endif()
  endif()
endfunction()

macro(check_compile_features
      BIN_DIR
      SOURCE_FILE
      GREETER
      GREET_SUCCESS
      GREET_FAILURE
      TARGET_VAR
      ON_SUCCESS)
  if(NOT DEFINED "${TARGET_VAR}")
    message(STATUS "${GREETER}")
    try_compile(${TARGET_VAR} "${BIN_DIR}" SOURCES "${SOURCE_FILE}")
    if(${TARGET_VAR})
      if(${ON_SUCCESS})
        message(STATUS "${GREETER} - ${GREET_SUCCESS}")
        set(${TARGET_VAR} 1 CACHE INTERNAL "${GREETER}")
      else()
        message(STATUS "${GREETER} - ${GREET_FAILURE}")
        set(${TARGET_VAR} "" CACHE INTERNAL "${GREETER}")
      endif()
    else()
      if(${ON_SUCCESS})
        message(STATUS "${GREETER} - ${GREET_FAILURE}")
        set(${TARGET_VAR} 1 CACHE INTERNAL "${GREETER}")
      else()
        message(STATUS "${GREETER} - ${GREET_SUCCESS}")
        set(${TARGET_VAR} "" CACHE INTERNAL "${GREETER}")
      endif()
    endif()
  endif()
endmacro()
