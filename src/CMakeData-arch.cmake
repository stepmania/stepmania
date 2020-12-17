list(APPEND SMDATA_ARCH_SRC "arch/RageDriver.cpp")

list(APPEND SMDATA_ARCH_HPP "arch/arch_default.h" "arch/RageDriver.h")

source_group("Arch Specific" FILES ${SMDATA_ARCH_SRC} ${SMDATA_ARCH_HPP})

list(APPEND SMDATA_ARCH_THREADS_HPP "arch/Threads/Threads.h")

if(WIN32)
  list(APPEND SMDATA_ARCH_THREADS_HPP "arch/Threads/Threads_Win32.h")
  list(APPEND SMDATA_ARCH_THREADS_SRC "arch/Threads/Threads_Win32.cpp")
elseif(APPLE)
  list(APPEND SMDATA_ARCH_THREADS_HPP "arch/Threads/Threads_Pthreads.h")
  list(APPEND SMDATA_ARCH_THREADS_SRC "arch/Threads/Threads_Pthreads.cpp")
else()
  if(HAS_PTHREAD)
    list(APPEND SMDATA_ARCH_THREADS_HPP "arch/Threads/Threads_Pthreads.h")
    list(APPEND SMDATA_ARCH_THREADS_SRC "arch/Threads/Threads_Pthreads.cpp")
  endif()
endif()

source_group("Arch Specific\\\\Threads"
             FILES
             ${SMDATA_ARCH_THREADS_SRC}
             ${SMDATA_ARCH_THREADS_HPP})

list(APPEND SMDATA_ARCH_SOUND_SRC
            "arch/Sound/RageSoundDriver.cpp"
            "arch/Sound/RageSoundDriver_Generic_Software.cpp"
            "arch/Sound/RageSoundDriver_Null.cpp")

list(APPEND SMDATA_ARCH_SOUND_HPP "arch/Sound/RageSoundDriver.h"
            "arch/Sound/RageSoundDriver_Null.h")

if(WIN32)
  list(APPEND SMDATA_ARCH_SOUND_SRC
              "arch/Sound/DSoundHelpers.cpp"
              "arch/Sound/RageSoundDriver_DSound_Software.cpp"
              "arch/Sound/RageSoundDriver_WaveOut.cpp"
              "arch/Sound/RageSoundDriver_WDMKS.cpp")
  list(APPEND SMDATA_ARCH_SOUND_HPP
              "arch/Sound/DSoundHelpers.h"
              "arch/Sound/RageSoundDriver_DSound_Software.h"
              "arch/Sound/RageSoundDriver_WaveOut.h"
              "arch/Sound/RageSoundDriver_WDMKS.h")
elseif(APPLE)
  list(APPEND SMDATA_ARCH_SOUND_SRC "arch/Sound/RageSoundDriver_AU.cpp")
  list(APPEND SMDATA_ARCH_SOUND_HPP "arch/Sound/RageSoundDriver_AU.h")
else() # Unix
  if(HAS_PULSE)
    list(APPEND SMDATA_ARCH_SOUND_SRC
                "arch/Sound/RageSoundDriver_PulseAudio.cpp")
    list(APPEND SMDATA_ARCH_SOUND_HPP "arch/Sound/RageSoundDriver_PulseAudio.h")
  endif()
  if(HAS_ALSA)
    list(APPEND SMDATA_ARCH_SOUND_SRC
                "arch/Sound/ALSA9Dynamic.cpp"
                "arch/Sound/ALSA9Helpers.cpp"
                "arch/Sound/RageSoundDriver_ALSA9_Software.cpp")
    list(APPEND SMDATA_ARCH_SOUND_HPP
                "arch/Sound/ALSA9Dynamic.h"
                "arch/Sound/ALSA9Functions.h"
                "arch/Sound/ALSA9Helpers.h"
                "arch/Sound/RageSoundDriver_ALSA9_Software.h")
  endif()
  if(HAS_JACK)
    list(APPEND SMDATA_ARCH_SOUND_SRC "arch/Sound/RageSoundDriver_JACK.cpp")
    list(APPEND SMDATA_ARCH_SOUND_HPP "arch/Sound/RageSoundDriver_JACK.h")
  endif()
  if(HAS_OSS)
    list(APPEND SMDATA_ARCH_SOUND_SRC "arch/Sound/RageSoundDriver_OSS.cpp")
    list(APPEND SMDATA_ARCH_SOUND_HPP "arch/Sound/RageSoundDriver_OSS.h")
  endif()
endif()

source_group("Arch Specific\\\\Sound"
             FILES
             ${SMDATA_ARCH_SOUND_SRC}
             ${SMDATA_ARCH_SOUND_HPP})

list(APPEND SMDATA_ARCH_MOVIE_TEXTURE_SRC
            "arch/MovieTexture/MovieTexture.cpp"
            "arch/MovieTexture/MovieTexture_Generic.cpp"
            "arch/MovieTexture/MovieTexture_Null.cpp")
list(APPEND SMDATA_ARCH_MOVIE_TEXTURE_HPP
            "arch/MovieTexture/MovieTexture.h"
            "arch/MovieTexture/MovieTexture_Generic.h"
            "arch/MovieTexture/MovieTexture_Null.h")

if(APPLE)
  if(${HAS_FFMPEG})
    list(APPEND SMDATA_ARCH_MOVIE_TEXTURE_SRC
                "arch/MovieTexture/MovieTexture_FFMpeg.cpp")
    list(APPEND SMDATA_ARCH_MOVIE_TEXTURE_HPP
                "arch/MovieTexture/MovieTexture_FFMpeg.h")
  endif()
elseif(MSVC)
  list(APPEND SMDATA_ARCH_MOVIE_TEXTURE_SRC
              "arch/MovieTexture/MovieTexture_FFMpeg.cpp")
  list(APPEND SMDATA_ARCH_MOVIE_TEXTURE_HPP
              "arch/MovieTexture/MovieTexture_FFMpeg.h")
else() # Unix
  if(${HAS_FFMPEG})
    list(APPEND SMDATA_ARCH_MOVIE_TEXTURE_SRC
                "arch/MovieTexture/MovieTexture_FFMpeg.cpp")
    list(APPEND SMDATA_ARCH_MOVIE_TEXTURE_HPP
                "arch/MovieTexture/MovieTexture_FFMpeg.h")
  endif()
endif()

source_group("Arch Specific\\\\Movie Texture"
             FILES
             ${SMDATA_ARCH_MOVIE_TEXTURE_SRC}
             ${SMDATA_ARCH_MOVIE_TEXTURE_HPP})

list(APPEND SMDATA_ARCH_MEMORY_SRC "arch/MemoryCard/MemoryCardDriver.cpp"
            "arch/MemoryCard/MemoryCardDriverThreaded_Folder.cpp")
list(APPEND SMDATA_ARCH_MEMORY_HPP
            "arch/MemoryCard/MemoryCardDriver.h"
            "arch/MemoryCard/MemoryCardDriver_Null.h"
            "arch/MemoryCard/MemoryCardDriverThreaded_Folder.h")

if(WIN32)
  list(APPEND SMDATA_ARCH_MEMORY_SRC
              "arch/MemoryCard/MemoryCardDriverThreaded_Windows.cpp")
  list(APPEND SMDATA_ARCH_MEMORY_HPP
              "arch/MemoryCard/MemoryCardDriverThreaded_Windows.h")
elseif(APPLE)
  list(APPEND SMDATA_ARCH_MEMORY_SRC
              "arch/MemoryCard/MemoryCardDriverThreaded_MacOSX.cpp")
  list(APPEND SMDATA_ARCH_MEMORY_HPP
              "arch/MemoryCard/MemoryCardDriverThreaded_MacOSX.h")
elseif(LINUX)
  list(APPEND SMDATA_ARCH_MEMORY_SRC
              "arch/MemoryCard/MemoryCardDriverThreaded_Linux.cpp")
  list(APPEND SMDATA_ARCH_MEMORY_HPP
              "arch/MemoryCard/MemoryCardDriverThreaded_Linux.h")
endif()

source_group("Arch Specific\\\\Memory Card"
             FILES
             ${SMDATA_ARCH_MEMORY_SRC}
             ${SMDATA_ARCH_MEMORY_HPP})

list(APPEND SMDATA_ARCH_LOWLEVEL_SRC "arch/LowLevelWindow/LowLevelWindow.cpp")
list(APPEND SMDATA_ARCH_LOWLEVEL_HPP "arch/LowLevelWindow/LowLevelWindow.h")

if(WIN32)
  list(APPEND SMDATA_ARCH_LOWLEVEL_SRC
              "arch/LowLevelWindow/LowLevelWindow_Win32.cpp")
  list(APPEND SMDATA_ARCH_LOWLEVEL_HPP
              "arch/LowLevelWindow/LowLevelWindow_Win32.h")
elseif(APPLE)
  list(APPEND SMDATA_ARCH_LOWLEVEL_SRC
              "arch/LowLevelWindow/LowLevelWindow_MacOSX.mm")
  list(APPEND SMDATA_ARCH_LOWLEVEL_HPP
              "arch/LowLevelWindow/LowLevelWindow_MacOSX.h")
else(UNIX)
  if(X11_FOUND)
    list(APPEND SMDATA_ARCH_LOWLEVEL_SRC
                "arch/LowLevelWindow/LowLevelWindow_X11.cpp")
    list(APPEND SMDATA_ARCH_LOWLEVEL_HPP
                "arch/LowLevelWindow/LowLevelWindow_X11.h")
  endif()
endif(WIN32)

source_group("Arch Specific\\\\Low Level Window"
             FILES
             ${SMDATA_ARCH_LOWLEVEL_SRC}
             ${SMDATA_ARCH_LOWLEVEL_HPP})

list(APPEND SMDATA_ARCH_LOADING_SRC "arch/LoadingWindow/LoadingWindow.cpp")
list(APPEND SMDATA_ARCH_LOADING_HPP "arch/LoadingWindow/LoadingWindow.h")

if(WIN32)
  list(APPEND SMDATA_ARCH_LOADING_SRC
              "arch/LoadingWindow/LoadingWindow_Win32.cpp")
  list(
    APPEND SMDATA_ARCH_LOADING_HPP "arch/LoadingWindow/LoadingWindow_Win32.h")
else()
  list(APPEND SMDATA_ARCH_LOADING_HPP "arch/LoadingWindow/LoadingWindow_Null.h")
  if(APPLE)
    list(APPEND SMDATA_ARCH_LOADING_SRC
                "arch/LoadingWindow/LoadingWindow_MacOSX.mm")
    list(APPEND SMDATA_ARCH_LOADING_HPP
                "arch/LoadingWindow/LoadingWindow_MacOSX.h")
  elseif(LINUX)
    if(GTK3_FOUND)
      list(APPEND SMDATA_ARCH_LOADING_SRC
                  "arch/LoadingWindow/LoadingWindow_Gtk.cpp")
      list(APPEND SMDATA_ARCH_LOADING_HPP
                  "arch/LoadingWindow/LoadingWindow_Gtk.h")
    endif()
  endif()
endif()

source_group("Arch Specific\\\\Loading Window"
             FILES
             ${SMDATA_ARCH_LOADING_SRC}
             ${SMDATA_ARCH_LOADING_HPP})

list(APPEND SMDATA_ARCH_LIGHTS_SRC "arch/Lights/LightsDriver.cpp"
            "arch/Lights/LightsDriver_SystemMessage.cpp")
list(APPEND SMDATA_ARCH_LIGHTS_HPP "arch/Lights/LightsDriver.h"
            "arch/Lights/LightsDriver_SystemMessage.h")

list(APPEND SMDATA_ARCH_LIGHTS_SRC "arch/Lights/LightsDriver_SextetStream.cpp")
list(APPEND SMDATA_ARCH_LIGHTS_HPP "arch/Lights/LightsDriver_SextetStream.h")

# TODO: Confirm if Apple can use the export.
if(NOT APPLE)
  list(APPEND SMDATA_ARCH_LIGHTS_SRC "arch/Lights/LightsDriver_Export.cpp")
  list(APPEND SMDATA_ARCH_LIGHTS_HPP "arch/Lights/LightsDriver_Export.h")

  if(WIN32)
    list(APPEND SMDATA_ARCH_LIGHTS_SRC
                "arch/Lights/LightsDriver_Win32Parallel.cpp"
                "arch/Lights/LightsDriver_PacDrive.cpp")
    list(APPEND SMDATA_ARCH_LIGHTS_HPP
                "arch/Lights/LightsDriver_Win32Parallel.h"
                "arch/Lights/LightsDriver_PacDrive.cpp")
    if(WITH_MINIMAID)
      list(APPEND SMDATA_ARCH_LIGHTS_SRC
                  "arch/Lights/LightsDriver_Win32Minimaid.cpp")
      list(APPEND SMDATA_ARCH_LIGHTS_HPP
                  "arch/Lights/LightsDriver_Win32Minimaid.h")
    endif()
  else() # Unix/Linux TODO: Linux HAVE_PARALLEL_PORT
    if(LINUX)
      list(APPEND SMDATA_ARCH_LIGHTS_SRC
                  "arch/Lights/LightsDriver_Linux_PIUIO.cpp"
                  "arch/Lights/LightsDriver_Linux_PIUIO_Leds.cpp"
                  "arch/Lights/LightsDriver_Linux_PIUIOBTN_Leds.cpp"
                  "arch/Lights/LightsDriver_Linux_ITGIO.cpp"
                  "arch/Lights/LightsDriver_LinuxWeedTech.cpp")
      list(APPEND SMDATA_ARCH_LIGHTS_HPP
                  "arch/Lights/LightsDriver_Linux_PIUIO.h"
                  "arch/Lights/LightsDriver_Linux_PIUIO_Leds.h"
                  "arch/Lights/LightsDriver_Linux_PIUIOBTN_Leds.h"
                  "arch/Lights/LightsDriver_Linux_ITGIO.h"
                  "arch/Lights/LightsDriver_LinuxWeedTech.h")
      if(WITH_PARALLEL_PORT)
        list(APPEND SMDATA_ARCH_LIGHTS_SRC
                    "arch/Lights/LightsDriver_LinuxParallel.cpp")
        list(APPEND SMDATA_ARCH_LIGHTS_HPP
                    "arch/Lights/LightsDriver_LinuxParallel.h")
      endif()
      if(WITH_MINIMAID)
        list(APPEND SMDATA_ARCH_LIGHTS_SRC
                    "arch/Lights/LightsDriver_LinuxMinimaid.cpp")
        list(APPEND SMDATA_ARCH_LIGHTS_HPP
                    "arch/Lights/LightsDriver_LinuxMinimaid.h")
      endif()
    endif()
  endif(WIN32)
endif(NOT APPLE)

source_group("Arch Specific\\\\Lights"
             FILES
             ${SMDATA_ARCH_LIGHTS_SRC}
             ${SMDATA_ARCH_LIGHTS_HPP})

list(APPEND SMDATA_ARCH_INPUT_SRC "arch/InputHandler/InputHandler.cpp"
            "arch/InputHandler/InputHandler_MonkeyKeyboard.cpp")
list(APPEND SMDATA_ARCH_INPUT_HPP "arch/InputHandler/InputHandler.h"
            "arch/InputHandler/InputHandler_MonkeyKeyboard.h")

if(WIN32)
  list(APPEND SMDATA_ARCH_INPUT_SRC
              "arch/InputHandler/InputHandler_DirectInput.cpp"
              "arch/InputHandler/InputHandler_DirectInputHelper.cpp"
              "arch/InputHandler/InputHandler_Win32_MIDI.cpp"
              "arch/InputHandler/InputHandler_Win32_Para.cpp"
              "arch/InputHandler/InputHandler_Win32_Pump.cpp"
              "arch/InputHandler/InputHandler_Win32_RTIO.cpp")
  list(APPEND SMDATA_ARCH_INPUT_HPP
              "arch/InputHandler/InputHandler_DirectInput.h"
              "arch/InputHandler/InputHandler_DirectInputHelper.h"
              "arch/InputHandler/InputHandler_Win32_MIDI.h"
              "arch/InputHandler/InputHandler_Win32_Para.h"
              "arch/InputHandler/InputHandler_Win32_Pump.h"
              "arch/InputHandler/InputHandler_Win32_RTIO.h")
  if(NOT MSVC)
    list(APPEND SMDATA_ARCH_INPUT_SRC
                "arch/InputHandler/InputHandler_SextetStream.cpp")
    list(APPEND SMDATA_ARCH_INPUT_HPP
                "arch/InputHandler/InputHandler_SextetStream.h")
  endif()
elseif(APPLE)
  list(APPEND SMDATA_ARCH_INPUT_SRC
              "arch/InputHandler/InputHandler_MacOSX_HID.cpp")
  list(APPEND SMDATA_ARCH_INPUT_HPP
              "arch/InputHandler/InputHandler_MacOSX_HID.h")
else() # Unix/Linux
  if(LINUX)
    list(APPEND SMDATA_ARCH_INPUT_SRC
                "arch/InputHandler/LinuxInputManager.cpp"
                "arch/InputHandler/InputHandler_Linux_Joystick.cpp"
                "arch/InputHandler/InputHandler_Linux_Event.cpp"
                "arch/InputHandler/InputHandler_Linux_PIUIO.cpp"
                "arch/InputHandler/InputHandler_SextetStream.cpp")
    list(APPEND SMDATA_ARCH_INPUT_SRC
                "arch/InputHandler/LinuxInputManager.h"
                "arch/InputHandler/InputHandler_Linux_Joystick.h"
                "arch/InputHandler/InputHandler_Linux_Event.h"
                "arch/InputHandler/InputHandler_Linux_PIUIO.h"
                "arch/InputHandler/InputHandler_SextetStream.h")
    if(WITH_TTY)
      list(APPEND SMDATA_ARCH_INPUT_SRC
                  "arch/InputHandler/InputHandler_Linux_tty.cpp")
      list(APPEND SMDATA_ARCH_INPUT_HPP
                  "arch/InputHandler/InputHandler_Linux_tty.h"
                  "arch/InputHandler/InputHandler_Linux_tty_keys.h")
    endif()
  endif()
  if(X11_FOUND)
    list(APPEND SMDATA_ARCH_INPUT_SRC "arch/InputHandler/InputHandler_X11.cpp")
    list(APPEND SMDATA_ARCH_INPUT_HPP "arch/InputHandler/InputHandler_X11.h")
  endif()
endif()

source_group("Arch Specific\\\\Input Handler"
             FILES
             ${SMDATA_ARCH_INPUT_SRC}
             ${SMDATA_ARCH_INPUT_HPP})

list(APPEND SMDATA_ARCH_DIALOG_SRC "arch/Dialog/Dialog.cpp"
            "arch/Dialog/DialogDriver.cpp")
list(APPEND SMDATA_ARCH_DIALOG_HPP "arch/Dialog/Dialog.h"
            "arch/Dialog/DialogDriver.h")

if(WIN32)
  list(APPEND SMDATA_ARCH_DIALOG_SRC "arch/Dialog/DialogDriver_Win32.cpp")
  list(APPEND SMDATA_ARCH_DIALOG_HPP "arch/Dialog/DialogDriver_Win32.h")
elseif(APPLE)
  list(APPEND SMDATA_ARCH_DIALOG_SRC "arch/Dialog/DialogDriver_MacOSX.cpp")
  list(APPEND SMDATA_ARCH_DIALOG_HPP "arch/Dialog/DialogDriver_MacOSX.h")
endif(WIN32)

source_group("Arch Specific\\\\Dialog"
             FILES
             ${SMDATA_ARCH_DIALOG_SRC}
             ${SMDATA_ARCH_DIALOG_HPP})

list(APPEND SMDATA_ARCH_HOOKS_SRC "arch/ArchHooks/ArchHooks.cpp")
list(APPEND SMDATA_ARCH_HOOKS_HPP "arch/ArchHooks/ArchHooks.h")

if(NOT APPLE)
  list(APPEND SMDATA_ARCH_HOOKS_SRC "arch/ArchHooks/ArchHooksUtil.cpp")
  if(WIN32)
    list(APPEND SMDATA_ARCH_HOOKS_SRC "arch/ArchHooks/ArchHooks_Win32.cpp"
                "arch/ArchHooks/ArchHooks_Win32Static.cpp")
    list(APPEND SMDATA_ARCH_HOOKS_HPP "arch/ArchHooks/ArchHooks_Win32.h")
  else(WIN32)
    list(APPEND SMDATA_ARCH_HOOKS_SRC "arch/ArchHooks/ArchHooks_Unix.cpp")
    list(APPEND SMDATA_ARCH_HOOKS_HPP "arch/ArchHooks/ArchHooks_Unix.h")
  endif(WIN32)
else(NOT APPLE)
  list(APPEND SMDATA_ARCH_HOOKS_SRC "arch/ArchHooks/ArchHooks_MacOSX.mm")
  list(APPEND SMDATA_ARCH_HOOKS_HPP "arch/ArchHooks/ArchHooks_MacOSX.h")
endif(NOT APPLE)

source_group("Arch Specific\\\\Arch Hooks"
             FILES
             ${SMDATA_ARCH_HOOKS_SRC}
             ${SMDATA_ARCH_HOOKS_HPP})

list(APPEND SMDATA_ALL_ARCH_SRC
            ${SMDATA_ARCH_SRC}
            ${SMDATA_ARCH_DIALOG_SRC}
            ${SMDATA_ARCH_HOOKS_SRC}
            ${SMDATA_ARCH_INPUT_SRC}
            ${SMDATA_ARCH_LIGHTS_SRC}
            ${SMDATA_ARCH_LOADING_SRC}
            ${SMDATA_ARCH_LOWLEVEL_SRC}
            ${SMDATA_ARCH_MEMORY_SRC}
            ${SMDATA_ARCH_MOVIE_TEXTURE_SRC}
            ${SMDATA_ARCH_SOUND_SRC}
            ${SMDATA_ARCH_THREADS_SRC})
list(APPEND SMDATA_ALL_ARCH_HPP
            ${SMDATA_ARCH_HPP}
            ${SMDATA_ARCH_DIALOG_HPP}
            ${SMDATA_ARCH_HOOKS_HPP}
            ${SMDATA_ARCH_INPUT_HPP}
            ${SMDATA_ARCH_LIGHTS_HPP}
            ${SMDATA_ARCH_LOADING_HPP}
            ${SMDATA_ARCH_LOWLEVEL_HPP}
            ${SMDATA_ARCH_MEMORY_HPP}
            ${SMDATA_ARCH_MOVIE_TEXTURE_HPP}
            ${SMDATA_ARCH_SOUND_HPP}
            ${SMDATA_ARCH_THREADS_HPP})
