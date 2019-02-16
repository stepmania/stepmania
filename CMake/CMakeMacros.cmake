# Borrowed from http://stackoverflow.com/a/3323227/445373
function(sm_list_replace container index newvalue)
  list(INSERT ${container} ${index} ${newvalue})
  math(EXPR __INDEX "${index} + 1")
  list(REMOVE_AT ${container} ${__INDEX})
endfunction()

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

# Borrowed from http://stackoverflow.com/a/7172941/445373 TODO: Upgrade to cmake
# 3.x so that this function is not needed.
function(sm_join values glue output)
  string(REPLACE ";"
                 "${glue}"
                 _TMP_STR
                 "${values}")
  set(${output} "${_TMP_STR}" PARENT_SCOPE)
endfunction()

function(sm_add_compile_definition target def)
  sm_append_simple_target_property(${target} COMPILE_DEFINITIONS ${def})
endfunction()

function(sm_add_compile_flag target flag)
  get_target_property(current_property ${target} COMPILE_FLAGS)
  if(current_property)
    set(current_property "${current_property} ${flag}")
    set_target_properties(${target}
                          PROPERTIES COMPILE_FLAGS "${current_property}")
  else()
    set_target_properties(${target} PROPERTIES COMPILE_FLAGS ${flag})
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
      sm_add_compile_flag(${projectName} "/W0")
    elseif(APPLE)
      set_target_properties(
        ${projectName}
        PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_INHIBIT_ALL_WARNINGS "YES")
    else()
      set_target_properties(${projectName} PROPERTIES COMPILE_FLAGS "-w")
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

# Borrowed from http://stackoverflow.com/q/10113017
macro(configure_msvc_runtime)
  if(MSVC)
    # Get the compiler options generally used.
    list(APPEND COMPILER_VARIABLES
                CMAKE_C_FLAGS_DEBUG
                CMAKE_C_FLAGS_MINSIZEREL
                CMAKE_C_FLAGS_RELEASE
                CMAKE_C_FLAGS_RELWITHDEBINFO
                CMAKE_CXX_FLAGS_DEBUG
                CMAKE_CXX_FLAGS_MINSIZEREL
                CMAKE_CXX_FLAGS_RELEASE
                CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if(WITH_STATIC_LINKING)
      set(TO_REPLACE "/MD")
      set(REPLACE_WITH "/MT")
    else()
      set(TO_REPLACE "/MT")
      set(REPLACE_WITH "/MD")
    endif()
    foreach(COMPILER_VARIABLE ${COMPILER_VARIABLES})
      if(${COMPILER_VARIABLE} MATCHES "${TO_REPLACE}")
        string(REGEX
               REPLACE "${TO_REPLACE}"
                       "${REPLACE_WITH}"
                       ${COMPILER_VARIABLE}
                       "${${COMPILER_VARIABLE}}")
      endif()
    endforeach()
  endif()
endmacro()
