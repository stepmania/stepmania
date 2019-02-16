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
  set(SM_VERSION_FULL
      "${SM_VERSION_MAJOR}.${SM_VERSION_MINOR}-${SM_VERSION_GIT_HASH}")
  set(SM_VERSION_GIT
      "${SM_VERSION_MAJOR}.${SM_VERSION_MINOR}-${SM_VERSION_GIT_HASH}")
else()
  if(WITH_FULL_RELEASE)
    set(SM_VERSION_FULL
        "${SM_VERSION_MAJOR}.${SM_VERSION_MINOR}.${SM_VERSION_PATCH}")
    set(SM_VERSION_GIT
        "${SM_VERSION_MAJOR}.${SM_VERSION_MINOR}.${SM_VERSION_PATCH}")
  else()
    set(SM_VERSION_FULL
        "${SM_VERSION_MAJOR}.${SM_VERSION_MINOR}-git-${SM_VERSION_GIT_HASH}")
    set(SM_VERSION_GIT
        "${SM_VERSION_MAJOR}.${SM_VERSION_MINOR}-git-${SM_VERSION_GIT_HASH}")
  endif()
endif()

if(CMAKE_MAJOR_VERSION STREQUAL "3")
  # Use the CMake 3 approach whenever possible.
  string(TIMESTAMP SM_TIMESTAMP_DATE "%Y%m%d")
  string(TIMESTAMP SM_TIMESTAMP_TIME "%H:%M:%S" UTC)
else()
  if(MSVC)
    message(
      STATUS
        "Getting date and time information via PowerShell. This may take a few seconds."
      )
    execute_process(COMMAND powershell get-date -format "{yyyyMMdd}"
                    OUTPUT_VARIABLE SM_TIMESTAMP_DATE
                    RESULT_VARIABLE ret
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(COMMAND powershell get-date -format "{HH:mm:ss zzz}"
                    OUTPUT_VARIABLE SM_TIMESTAMP_TIME
                    RESULT_VARIABLE ret
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
  else()
    execute_process(COMMAND date "+%Y%m%d"
                    OUTPUT_VARIABLE SM_TIMESTAMP_DATE
                    RESULT_VARIABLE ret
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(COMMAND date "+%H:%M:%S %z"
                    OUTPUT_VARIABLE SM_TIMESTAMP_TIME
                    RESULT_VARIABLE ret
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
  endif()

  if(NOT (ret STREQUAL "0"))
    set(SM_TIMESTAMP_DATE "xxxxyyzz")
  endif()

  if(NOT (ret STREQUAL "0"))
    set(SM_TIMESTAMP_TIME "xx:yy:zz ???")
  endif()
endif()
