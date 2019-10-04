if(NOT MSVC)
  return()
endif()

set(IRC_DIR "${SM_SRC_DIR}/irc")

list(APPEND IRC_SRC "${IRC_DIR}/appveyor.cpp")

source_group("" FILES ${IRC_SRC})

add_executable("irc-reporter" ${IRC_SRC})

set_property(TARGET "irc-reporter" PROPERTY FOLDER "Internal Libraries")

disable_project_warnings("irc-reporter")

set_target_properties("irc-reporter"
                      PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                 "${SM_PROGRAM_DIR}"
                                 RUNTIME_OUTPUT_DIRECTORY_RELEASE
                                 "${SM_PROGRAM_DIR}"
                                 RUNTIME_OUTPUT_DIRECTORY_DEBUG
                                 "${SM_PROGRAM_DIR}"
                                 RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL
                                 "${SM_PROGRAM_DIR}"
                                 RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO
                                 "${SM_PROGRAM_DIR}")

set_target_properties("irc-reporter"
                      PROPERTIES OUTPUT_NAME
                                 "irc-reporter"
                                 RELEASE_OUTPUT_NAME
                                 "irc-reporter"
                                 DEBUG_OUTPUT_NAME
                                 "irc-reporter"
                                 MINSIZEREL_OUTPUT_NAME
                                 "irc-reporter"
                                 RELWITHDEBINFO_OUTPUT_NAME
                                 "irc-reporter")
