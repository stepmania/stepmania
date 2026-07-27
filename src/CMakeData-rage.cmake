# TODO: Turn Rage into a libary.

list(APPEND SMDATA_RAGE_UTILS_SRC
            "RageUtil.cpp"
            "RageUtil_BackgroundLoader.cpp"
            "RageUtil_CharConversions.cpp"
            "RageUtil_FileDB.cpp"
            "RageUtil_WorkerThread.cpp")

list(APPEND SMDATA_RAGE_UTILS_HPP
            "RageUtil.h"
            "RageUtil_AutoPtr.h" # TODO: Remove the need for this and replace
                                 # with c++11 smart pointers
            "RageUtil_BackgroundLoader.h"
            "RageUtil_CharConversions.h"
            "RageUtil_CircularBuffer.h"
            "RageUtil_FileDB.h"
            "RageUtil_WorkerThread.h")

source_group("Rage\\\\Utils"
             FILES
             ${SMDATA_RAGE_UTILS_SRC}
             ${SMDATA_RAGE_UTILS_HPP})

list(APPEND SMDATA_RAGE_MISC_SRC
            "RageException.cpp"
            "RageInput.cpp"
            "RageInputDevice.cpp"
            "RageLog.cpp"
            "RageMath.cpp"
            "RageTypes.cpp"
            "RageThreads.cpp"
            "RageTimer.cpp")

list(APPEND SMDATA_RAGE_MISC_HPP
            "RageException.h"
            "RageInput.h"
            "RageInputDevice.h"
            "RageLog.h"
            "RageMath.h"
            "RageTypes.h"
            "RageThreads.h"
            "RageTimer.h")

source_group("Rage\\\\Misc"
             FILES
             ${SMDATA_RAGE_MISC_SRC}
             ${SMDATA_RAGE_MISC_HPP})

list(APPEND SMDATA_RAGE_GRAPHICS_SRC
            "RageBitmapTexture.cpp"
            "RageDisplay.cpp"
            "RageDisplay_Null.cpp"
            "RageDisplay_OGL.cpp"
            "RageDisplay_OGL_Helpers.cpp"
            "RageModelGeometry.cpp"
            "RageSurface.cpp"
            "RageSurface_Load.cpp"
            "RageSurface_Load_BMP.cpp"
            "RageSurface_Load_GIF.cpp"
            "RageSurface_Load_JPEG.cpp"
            "RageSurface_Load_PNG.cpp"
            "RageSurface_Load_XPM.cpp"
            "RageSurface_Save_BMP.cpp"
            "RageSurface_Save_JPEG.cpp"
            "RageSurface_Save_PNG.cpp"
            "RageSurfaceUtils.cpp"
            "RageSurfaceUtils_Dither.cpp"
            "RageSurfaceUtils_Palettize.cpp"
            "RageSurfaceUtils_Zoom.cpp"
            "RageTexture.cpp"
            "RageTextureID.cpp"
            "RageTextureManager.cpp"
            "RageTexturePreloader.cpp"
            "RageTextureRenderTarget.cpp")

list(APPEND SMDATA_RAGE_GRAPHICS_HPP
            "RageBitmapTexture.h"
            "RageDisplay.h"
            "RageDisplay_Null.h"
            "RageDisplay_OGL.h"
            "RageDisplay_OGL_Helpers.h"
            "RageModelGeometry.h"
            "RageSurface.h"
            "RageSurface_Load.h"
            "RageSurface_Load_BMP.h"
            "RageSurface_Load_GIF.h"
            "RageSurface_Load_JPEG.h"
            "RageSurface_Load_PNG.h"
            "RageSurface_Load_XPM.h"
            "RageSurface_Save_BMP.h"
            "RageSurface_Save_JPEG.h"
            "RageSurface_Save_PNG.h"
            "RageSurfaceUtils.h"
            "RageSurfaceUtils_Dither.h"
            "RageSurfaceUtils_Palettize.h"
            "RageSurfaceUtils_Zoom.h"
            "RageTexture.h"
            "RageTextureID.h"
            "RageTextureManager.h"
            "RageTexturePreloader.h"
            "RageTextureRenderTarget.h")

if(WIN32)
  list(APPEND SMDATA_RAGE_GRAPHICS_SRC "RageDisplay_D3D.cpp")
  list(APPEND SMDATA_RAGE_GRAPHICS_HPP "RageDisplay_D3D.h")
elseif(LINUX)
  if(WITH_GLES2)
    list(APPEND SMDATA_RAGE_GRAPHICS_SRC "RageDisplay_GLES2.cpp")
    list(APPEND SMDATA_RAGE_GRAPHICS_HPP "RageDisplay_GLES2.h")
  endif()
endif()

source_group("Rage\\\\Graphics"
             FILES
             ${SMDATA_RAGE_GRAPHICS_SRC}
             ${SMDATA_RAGE_GRAPHICS_HPP})

list(APPEND SMDATA_RAGE_FILE_SRC
            "RageFile.cpp"
            "RageFileBasic.cpp"
            "RageFileDriver.cpp"
            "RageFileDriverDeflate.cpp"
            "RageFileDriverDirect.cpp"
            "RageFileDriverDirectHelpers.cpp"
            "RageFileDriverMemory.cpp"
            "RageFileDriverReadAhead.cpp"
            "RageFileDriverSlice.cpp"
            "RageFileDriverTimeout.cpp"
            "RageFileDriverZip.cpp"
            "RageFileManager.cpp"
            "RageFileManager_ReadAhead.cpp")

list(APPEND SMDATA_RAGE_FILE_HPP
            "RageFile.h"
            "RageFileBasic.h"
            "RageFileDriver.h"
            "RageFileDriverDeflate.h"
            "RageFileDriverDirect.h"
            "RageFileDriverDirectHelpers.h"
            "RageFileDriverMemory.h"
            "RageFileDriverReadAhead.h"
            "RageFileDriverSlice.h"
            "RageFileDriverTimeout.h"
            "RageFileDriverZip.h"
            "RageFileManager.h"
            "RageFileManager_ReadAhead.h")

source_group("Rage\\\\File"
             FILES
             ${SMDATA_RAGE_FILE_SRC}
             ${SMDATA_RAGE_FILE_HPP})

list(APPEND SMDATA_RAGE_SOUND_SRC
            "RageSound.cpp"
            "RageSoundManager.cpp"
            "RageSoundMixBuffer.cpp"
            "RageSoundPosMap.cpp"
            "RageSoundReader.cpp"
            "RageSoundReader_Chain.cpp"
            "RageSoundReader_ChannelSplit.cpp"
            "RageSoundReader_Extend.cpp"
            "RageSoundReader_FileReader.cpp"
            "RageSoundReader_MP3.cpp"
            "RageSoundReader_Merge.cpp"
            "RageSoundReader_Pan.cpp"
            "RageSoundReader_PitchChange.cpp"
            "RageSoundReader_PostBuffering.cpp"
            "RageSoundReader_Preload.cpp"
            "RageSoundReader_Resample_Good.cpp"
            "RageSoundReader_SpeedChange.cpp"
            "RageSoundReader_ThreadedBuffer.cpp"
            "RageSoundReader_Vorbisfile.cpp"
            "RageSoundReader_WAV.cpp"
            "RageSoundUtil.cpp")

list(APPEND SMDATA_RAGE_SOUND_HPP
            "RageSound.h"
            "RageSoundManager.h"
            "RageSoundMixBuffer.h"
            "RageSoundPosMap.h"
            "RageSoundReader.h"
            "RageSoundReader_Chain.h"
            "RageSoundReader_ChannelSplit.h"
            "RageSoundReader_Extend.h"
            "RageSoundReader_FileReader.h"
            "RageSoundReader_Filter.h"
            "RageSoundReader_MP3.h"
            "RageSoundReader_Merge.h"
            "RageSoundReader_Pan.h"
            "RageSoundReader_PitchChange.h"
            "RageSoundReader_PostBuffering.h"
            "RageSoundReader_Preload.h"
            "RageSoundReader_Resample_Good.h"
            "RageSoundReader_SpeedChange.h"
            "RageSoundReader_ThreadedBuffer.h"
            "RageSoundReader_Vorbisfile.h"
            "RageSoundReader_WAV.h"
            "RageSoundUtil.h")

source_group("Rage\\\\Sound"
             FILES
             ${SMDATA_RAGE_SOUND_SRC}
             ${SMDATA_RAGE_SOUND_HPP})

list(APPEND SMDATA_ALL_RAGE_SRC
            ${SMDATA_RAGE_FILE_SRC}
            ${SMDATA_RAGE_GRAPHICS_SRC}
            ${SMDATA_RAGE_MISC_SRC}
            ${SMDATA_RAGE_SOUND_SRC}
            ${SMDATA_RAGE_UTILS_SRC})

list(APPEND SMDATA_ALL_RAGE_HPP
            ${SMDATA_RAGE_FILE_HPP}
            ${SMDATA_RAGE_GRAPHICS_HPP}
            ${SMDATA_RAGE_MISC_HPP}
            ${SMDATA_RAGE_SOUND_HPP}
            ${SMDATA_RAGE_UTILS_HPP})
