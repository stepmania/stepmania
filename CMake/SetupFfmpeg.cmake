set(SM_FFMPEG_VERSION "2.1.3")
set(SM_FFMPEG_SRC_LIST "${SM_EXTERN_DIR}" "/ffmpeg-git")
sm_join("${SM_FFMPEG_SRC_LIST}" "" SM_FFMPEG_SRC_DIR)
set(SM_FFMPEG_ROOT "${CMAKE_BINARY_DIR}/ffmpeg-prefix/src/ffmpeg-build")
set(SM_FFMPEG_CONFIGURE_EXE "${SM_FFMPEG_SRC_DIR}/configure")
if (MINGW)
  # Borrow from http://stackoverflow.com/q/11845823
  # string(SUBSTRING ${SM_FFMPEG_CONFIGURE_EXE} 0 1 FIRST_LETTER)
  # string(TOLOWER ${FIRST_LETTER} FIRST_LETTER_LOW)
  # string(REPLACE "${FIRST_LETTER}:" "/${FIRST_LETTER_LOW}" # SM_FFMPEG_CONFIGURE_EXE ${SM_FFMPEG_CONFIGURE_EXE})
  # string(REGEX REPLACE "\\\\" "/" SM_FFMPEG_CONFIGURE_EXE "${SM_FFMPEG_CONFIGURE_EXE}")
  set(SM_FFMPEG_CONFIGURE_EXE "extern/ffmpeg-linux-${SM_FFMPEG_VERSION}/configure")
endif()
list(APPEND FFMPEG_CONFIGURE
  "${SM_FFMPEG_CONFIGURE_EXE}"
  "--disable-programs"
  "--disable-doc"
  "--disable-avdevice"
  "--disable-swresample"
  "--disable-postproc"
  "--disable-avfilter"
  "--disable-shared"
  "--enable-static"
)
if(WITH_GPL_LIBS)
  list(APPEND FFMPEG_CONFIGURE
    "--enable-gpl"
  )
endif()

if (WITH_CRYSTALHD_DISABLED)
  list(APPEND FFMPEG_CONFIGURE "--disable-crystalhd")
endif()

if (NOT WITH_EXTERNAL_WARNINGS)
  list(APPEND FFMPEG_CONFIGURE
    "--extra-cflags=-w"
  )
endif()

if (IS_DIRECTORY "${SM_FFMPEG_SRC_DIR}")
  externalproject_add("ffmpeg"
    SOURCE_DIR "${SM_FFMPEG_SRC_DIR}"
    CONFIGURE_COMMAND ${FFMPEG_CONFIGURE}
    BUILD_COMMAND "make"
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
    TEST_COMMAND ""
  )
else()
  message(ERROR "Submodule missing. Run git submodule init && git submodule update first.")
endif()

