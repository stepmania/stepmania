if(LINUX AND WITH_MINIMAID)
  list(APPEND SMDATA_LINK_LIB
    "${SM_EXTERN_DIR}/libmmmagic/linux-${SM_BITS}bit/libmmmagic.a"
    "udev"
  )
endif()
