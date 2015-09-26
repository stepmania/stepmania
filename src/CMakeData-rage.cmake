# TODO: Turn Rage into a libary.

list(APPEND SMDATA_RAGE_UTILS_SRC
  "${SM_SRC_RAGE_DIR}/RageUtil.cpp"
  "${SM_SRC_RAGE_DIR}/RageUtil_BackgroundLoader.cpp"
  "${SM_SRC_RAGE_DIR}/RageUtil_CachedObject.cpp"
  "${SM_SRC_RAGE_DIR}/RageUtil_CharConversions.cpp"
  "${SM_SRC_RAGE_DIR}/RageUtil_FileDB.cpp"
  "${SM_SRC_RAGE_DIR}/RageUtil_WorkerThread.cpp"
)

list(APPEND SMDATA_RAGE_UTILS_HPP
  "${SM_SRC_RAGE_DIR}/RageUtil.h"
  "${SM_SRC_RAGE_DIR}/RageUtil_AutoPtr.h" # TODO: Remove the need for this and replace with c++11 smart pointers
  "${SM_SRC_RAGE_DIR}/RageUtil_BackgroundLoader.h"
  "${SM_SRC_RAGE_DIR}/RageUtil_CachedObject.h"
  "${SM_SRC_RAGE_DIR}/RageUtil_CharConversions.h"
  "${SM_SRC_RAGE_DIR}/RageUtil_CircularBuffer.h"
  "${SM_SRC_RAGE_DIR}/RageUtil_FileDB.h"
  "${SM_SRC_RAGE_DIR}/RageUtil_WorkerThread.h"
)

source_group("Rage\\\\Utils" FILES ${SMDATA_RAGE_UTILS_SRC} ${SMDATA_RAGE_UTILS_HPP})

list(APPEND SMDATA_RAGE_MISC_SRC
  "${SM_SRC_RAGE_DIR}/RageException.cpp"
  "${SM_SRC_RAGE_DIR}/RageInput.cpp"
  "${SM_SRC_RAGE_DIR}/RageInputDevice.cpp"
  "${SM_SRC_RAGE_DIR}/RageLog.cpp"
  "${SM_SRC_RAGE_DIR}/RageMath.cpp"
  "${SM_SRC_RAGE_DIR}/RageTypes.cpp"
  "${SM_SRC_RAGE_DIR}/RageThreads.cpp"
  "${SM_SRC_RAGE_DIR}/RageTimer.cpp"
)

list(APPEND SMDATA_RAGE_MISC_HPP
  "${SM_SRC_RAGE_DIR}/RageException.h"
  "${SM_SRC_RAGE_DIR}/RageInput.h"
  "${SM_SRC_RAGE_DIR}/RageInputDevice.h"
  "${SM_SRC_RAGE_DIR}/RageLog.h"
  "${SM_SRC_RAGE_DIR}/RageMath.h"
  "${SM_SRC_RAGE_DIR}/RageTypes.h"
  "${SM_SRC_RAGE_DIR}/RageThreads.h"
  "${SM_SRC_RAGE_DIR}/RageTimer.h"
)

source_group("Rage\\\\Misc" FILES ${SMDATA_RAGE_MISC_SRC} ${SMDATA_RAGE_MISC_HPP})

list(APPEND SMDATA_RAGE_GRAPHICS_SRC
  "${SM_SRC_RAGE_DIR}/RageBitmapTexture.cpp"
  "${SM_SRC_RAGE_DIR}/RageDisplay.cpp"
  "${SM_SRC_RAGE_DIR}/RageDisplay_Null.cpp"
  "${SM_SRC_RAGE_DIR}/RageDisplay_OGL.cpp"
  "${SM_SRC_RAGE_DIR}/RageDisplay_OGL_Helpers.cpp"
  "${SM_SRC_RAGE_DIR}/RageModelGeometry.cpp"
  "${SM_SRC_RAGE_DIR}/RageSurface.cpp"
  "${SM_SRC_RAGE_DIR}/RageSurface_Load.cpp"
  "${SM_SRC_RAGE_DIR}/RageSurface_Load_BMP.cpp"
  "${SM_SRC_RAGE_DIR}/RageSurface_Load_GIF.cpp"
  "${SM_SRC_RAGE_DIR}/RageSurface_Load_JPEG.cpp"
  "${SM_SRC_RAGE_DIR}/RageSurface_Load_PNG.cpp"
  "${SM_SRC_RAGE_DIR}/RageSurface_Load_XPM.cpp"
  "${SM_SRC_RAGE_DIR}/RageSurface_Save_BMP.cpp"
  "${SM_SRC_RAGE_DIR}/RageSurface_Save_JPEG.cpp"
  "${SM_SRC_RAGE_DIR}/RageSurface_Save_PNG.cpp"
  "${SM_SRC_RAGE_DIR}/RageSurfaceUtils.cpp"
  "${SM_SRC_RAGE_DIR}/RageSurfaceUtils_Dither.cpp"
  "${SM_SRC_RAGE_DIR}/RageSurfaceUtils_Palettize.cpp"
  "${SM_SRC_RAGE_DIR}/RageSurfaceUtils_Zoom.cpp"
  "${SM_SRC_RAGE_DIR}/RageTexture.cpp"
  "${SM_SRC_RAGE_DIR}/RageTextureID.cpp"
  "${SM_SRC_RAGE_DIR}/RageTextureManager.cpp"
  "${SM_SRC_RAGE_DIR}/RageTexturePreloader.cpp"
  "${SM_SRC_RAGE_DIR}/RageTextureRenderTarget.cpp"
)
list(APPEND SMDATA_RAGE_GRAPHICS_HPP
  "${SM_SRC_RAGE_DIR}/RageBitmapTexture.h"
  "${SM_SRC_RAGE_DIR}/RageDisplay.h"
  "${SM_SRC_RAGE_DIR}/RageDisplay_Null.h"
  "${SM_SRC_RAGE_DIR}/RageDisplay_OGL.h"
  "${SM_SRC_RAGE_DIR}/RageDisplay_OGL_Helpers.h"
  "${SM_SRC_RAGE_DIR}/RageModelGeometry.h"
  "${SM_SRC_RAGE_DIR}/RageSurface.h"
  "${SM_SRC_RAGE_DIR}/RageSurface_Load.h"
  "${SM_SRC_RAGE_DIR}/RageSurface_Load_BMP.h"
  "${SM_SRC_RAGE_DIR}/RageSurface_Load_GIF.h"
  "${SM_SRC_RAGE_DIR}/RageSurface_Load_JPEG.h"
  "${SM_SRC_RAGE_DIR}/RageSurface_Load_PNG.h"
  "${SM_SRC_RAGE_DIR}/RageSurface_Load_XPM.h"
  "${SM_SRC_RAGE_DIR}/RageSurface_Save_BMP.h"
  "${SM_SRC_RAGE_DIR}/RageSurface_Save_JPEG.h"
  "${SM_SRC_RAGE_DIR}/RageSurface_Save_PNG.h"
  "${SM_SRC_RAGE_DIR}/RageSurfaceUtils.h"
  "${SM_SRC_RAGE_DIR}/RageSurfaceUtils_Dither.h"
  "${SM_SRC_RAGE_DIR}/RageSurfaceUtils_Palettize.h"
  "${SM_SRC_RAGE_DIR}/RageSurfaceUtils_Zoom.h"
  "${SM_SRC_RAGE_DIR}/RageTexture.h"
  "${SM_SRC_RAGE_DIR}/RageTextureID.h"
  "${SM_SRC_RAGE_DIR}/RageTextureManager.h"
  "${SM_SRC_RAGE_DIR}/RageTexturePreloader.h"
  "${SM_SRC_RAGE_DIR}/RageTextureRenderTarget.h"
)

if(WIN32)
  list(APPEND SMDATA_RAGE_GRAPHICS_SRC "${SM_SRC_RAGE_DIR}/RageDisplay_D3D.cpp")
  list(APPEND SMDATA_RAGE_GRAPHICS_HPP "${SM_SRC_RAGE_DIR}/RageDisplay_D3D.h")
elseif(LINUX)
  if (WITH_GLES2)
    list(APPEND SMDATA_RAGE_GRAPHICS_SRC "${SM_SRC_RAGE_DIR}/RageDisplay_GLES2.cpp")
    list(APPEND SMDATA_RAGE_GRAPHICS_HPP "${SM_SRC_RAGE_DIR}/RageDisplay_GLES2.h")
  endif()
endif()

source_group("Rage\\\\Graphics" FILES ${SMDATA_RAGE_GRAPHICS_SRC} ${SMDATA_RAGE_GRAPHICS_HPP})

list(APPEND SMDATA_RAGE_FILE_SRC
  "${SM_SRC_RAGE_DIR}/RageFile.cpp"
  "${SM_SRC_RAGE_DIR}/RageFileBasic.cpp"
  "${SM_SRC_RAGE_DIR}/RageFileDriver.cpp"
  "${SM_SRC_RAGE_DIR}/RageFileDriverDeflate.cpp"
  "${SM_SRC_RAGE_DIR}/RageFileDriverDirect.cpp"
  "${SM_SRC_RAGE_DIR}/RageFileDriverDirectHelpers.cpp"
  "${SM_SRC_RAGE_DIR}/RageFileDriverMemory.cpp"
  "${SM_SRC_RAGE_DIR}/RageFileDriverReadAhead.cpp"
  "${SM_SRC_RAGE_DIR}/RageFileDriverSlice.cpp"
  "${SM_SRC_RAGE_DIR}/RageFileDriverTimeout.cpp"
  "${SM_SRC_RAGE_DIR}/RageFileDriverZip.cpp"
  "${SM_SRC_RAGE_DIR}/RageFileManager.cpp"
  "${SM_SRC_RAGE_DIR}/RageFileManager_ReadAhead.cpp"
)

list(APPEND SMDATA_RAGE_FILE_HPP
  "${SM_SRC_RAGE_DIR}/RageFile.h"
  "${SM_SRC_RAGE_DIR}/RageFileBasic.h"
  "${SM_SRC_RAGE_DIR}/RageFileDriver.h"
  "${SM_SRC_RAGE_DIR}/RageFileDriverDeflate.h"
  "${SM_SRC_RAGE_DIR}/RageFileDriverDirect.h"
  "${SM_SRC_RAGE_DIR}/RageFileDriverDirectHelpers.h"
  "${SM_SRC_RAGE_DIR}/RageFileDriverMemory.h"
  "${SM_SRC_RAGE_DIR}/RageFileDriverReadAhead.h"
  "${SM_SRC_RAGE_DIR}/RageFileDriverSlice.h"
  "${SM_SRC_RAGE_DIR}/RageFileDriverTimeout.h"
  "${SM_SRC_RAGE_DIR}/RageFileDriverZip.h"
  "${SM_SRC_RAGE_DIR}/RageFileManager.h"
  "${SM_SRC_RAGE_DIR}/RageFileManager_ReadAhead.h"
)

source_group("Rage\\\\File" FILES ${SMDATA_RAGE_FILE_SRC} ${SMDATA_RAGE_FILE_HPP})

list(APPEND SMDATA_RAGE_SOUND_SRC
  "${SM_SRC_RAGE_DIR}/RageSound.cpp"
  "${SM_SRC_RAGE_DIR}/RageSoundManager.cpp"
  "${SM_SRC_RAGE_DIR}/RageSoundMixBuffer.cpp"
  "${SM_SRC_RAGE_DIR}/RageSoundPosMap.cpp"
  "${SM_SRC_RAGE_DIR}/RageSoundReader.cpp"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_Chain.cpp"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_ChannelSplit.cpp"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_Extend.cpp"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_FileReader.cpp"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_Merge.cpp"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_Pan.cpp"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_PitchChange.cpp"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_PostBuffering.cpp"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_Preload.cpp"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_Resample_Good.cpp"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_SpeedChange.cpp"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_ThreadedBuffer.cpp"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_WAV.cpp"
  "${SM_SRC_RAGE_DIR}/RageSoundUtil.cpp"
)
list(APPEND SMDATA_RAGE_SOUND_HPP
  "${SM_SRC_RAGE_DIR}/RageSound.h"
  "${SM_SRC_RAGE_DIR}/RageSoundManager.h"
  "${SM_SRC_RAGE_DIR}/RageSoundMixBuffer.h"
  "${SM_SRC_RAGE_DIR}/RageSoundPosMap.h"
  "${SM_SRC_RAGE_DIR}/RageSoundReader.h"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_Chain.h"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_ChannelSplit.h"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_Extend.h"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_FileReader.h"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_Filter.h"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_Merge.h"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_Pan.h"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_PitchChange.h"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_PostBuffering.h"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_Preload.h"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_Resample_Good.h"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_SpeedChange.h"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_ThreadedBuffer.h"
  "${SM_SRC_RAGE_DIR}/RageSoundReader_WAV.h"
  "${SM_SRC_RAGE_DIR}/RageSoundUtil.h"
)

if (HAS_OGG)
  list(APPEND SMDATA_RAGE_SOUND_SRC "${SM_SRC_RAGE_DIR}/RageSoundReader_Vorbisfile.cpp")
  list(APPEND SMDATA_RAGE_SOUND_HPP "${SM_SRC_RAGE_DIR}/RageSoundReader_Vorbisfile.h")
endif()

if (HAS_MP3)
  list(APPEND SMDATA_RAGE_SOUND_SRC "${SM_SRC_RAGE_DIR}/RageSoundReader_MP3.cpp")
  list(APPEND SMDATA_RAGE_SOUND_HPP "${SM_SRC_RAGE_DIR}/RageSoundReader_MP3.h")
endif()

source_group("Rage\\\\Sound" FILES ${SMDATA_RAGE_SOUND_SRC} ${SMDATA_RAGE_SOUND_HPP})

list(APPEND SMDATA_ALL_RAGE_SRC
  ${SMDATA_RAGE_FILE_SRC}
  ${SMDATA_RAGE_GRAPHICS_SRC}
  ${SMDATA_RAGE_MISC_SRC}
  ${SMDATA_RAGE_SOUND_SRC}
  ${SMDATA_RAGE_UTILS_SRC}
)

list(APPEND SMDATA_ALL_RAGE_HPP
  ${SMDATA_RAGE_FILE_HPP}
  ${SMDATA_RAGE_GRAPHICS_HPP}
  ${SMDATA_RAGE_MISC_HPP}
  ${SMDATA_RAGE_SOUND_HPP}
  ${SMDATA_RAGE_UTILS_HPP}
)
