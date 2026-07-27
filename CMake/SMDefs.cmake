# Set up version numbers according to the new scheme.
set(SM_VERSION_MAJOR 5)
set(SM_VERSION_MINOR 1)
set(SM_VERSION_PATCH 0)
set(SM_VERSION_TRADITIONAL
    "${SM_VERSION_MAJOR}.${SM_VERSION_MINOR}.${SM_VERSION_PATCH}")

execute_process(COMMAND git rev-parse --short HEAD
                WORKING_DIRECTORY "${SM_ROOT_DIR}"
                OUTPUT_VARIABLE SM_VERSION_GIT_HASH
                RESULT_VARIABLE ret
                OUTPUT_STRIP_TRAILING_WHITESPACE)

if(NOT (ret STREQUAL "0"))
  message(
    WARNING
      "git was not found on your path. If you collect bug reports, please add git to your path and rerun cmake."
    )
  set(SM_VERSION_GIT_HASH "UNKNOWN")
  if(WITH_FULL_RELEASE)
    set(SM_VERSION_GIT
        "${SM_VERSION_MAJOR}.${SM_VERSION_MINOR}.${SM_VERSION_PATCH}")
  else()
    set(SM_VERSION_GIT
        "${SM_VERSION_MAJOR}.${SM_VERSION_MINOR}.${SM_VERSION_PATCH}-${SM_VERSION_GIT_HASH}")
  endif()
else()
  if(WITH_FULL_RELEASE)
    set(SM_VERSION_GIT
        "${SM_VERSION_MAJOR}.${SM_VERSION_MINOR}.${SM_VERSION_PATCH}")
  else()
    set(SM_VERSION_GIT
        "${SM_VERSION_MAJOR}.${SM_VERSION_MINOR}.${SM_VERSION_PATCH}-git-${SM_VERSION_GIT_HASH}")
  endif()
endif()

string(TIMESTAMP SM_TIMESTAMP_DATE "%Y%m%d")
string(TIMESTAMP SM_TIMESTAMP_TIME "%H:%M:%S" UTC)
