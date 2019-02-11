if(LINUX)
  if(WITH_MINIMAID)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
      list(APPEND SMDATA_LINK_LIB
                  "${SM_EXTERN_DIR}/libmmmagic/linux-64bit/libmmmagic.a" "udev")
    else()
      list(APPEND SMDATA_LINK_LIB
                  "${SM_EXTERN_DIR}/libmmmagic/linux-32bit/libmmmagic.a" "udev")
    endif()
  endif()
endif()
