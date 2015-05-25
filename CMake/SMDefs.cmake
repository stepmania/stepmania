# Set up version numbers according to the new scheme.
set(SM_VERSION_MAJOR 5)
set(SM_VERSION_MINOR 0)
set(SM_VERSION_PATCH 8)
set(SM_VERSION_TRADITIONAL "${SM_VERSION_MAJOR}.${SM_VERSION_MINOR}.${SM_VERSION_PATCH}")

execute_process(COMMAND git rev-parse --short HEAD
  WORKING_DIRECTORY "${SM_ROOT_DIR}"
  OUTPUT_VARIABLE SM_VERSION_GIT_HASH
  RESULT_VARIABLE ret
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT (ret STREQUAL "0"))
  set(SM_VERSION_GIT_HASH "UNKNOWN")
  set(SM_VERSION_FULL "${SM_VERSION_MAJOR}.${SM_VERSION_MINOR}-${SM_VERSION_GIT_HASH}")
else()
  set(SM_VERSION_FULL "${SM_VERSION_MAJOR}.${SM_VERSION_MINOR}-git-${SM_VERSION_GIT_HASH}")
endif()