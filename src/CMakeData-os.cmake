if(APPLE)
  list(APPEND SMDATA_OS_DARWIN_SRC
              "archutils/Darwin/Crash.cpp"
              "archutils/Darwin/DarwinThreadHelpers.cpp"
              "archutils/Darwin/HIDDevice.cpp"
              "archutils/Darwin/JoystickDevice.cpp"
              "archutils/Darwin/KeyboardDevice.cpp"
              "archutils/Darwin/MouseDevice.cpp"
              "archutils/Darwin/PumpDevice.cpp"
              "archutils/Darwin/SMMain.mm"
              "archutils/Darwin/SpecialDirs.cpp"
              "archutils/Darwin/VectorHelper.cpp")
  list(APPEND SMDATA_OS_DARWIN_HPP
              "archutils/Darwin/arch_setup.h"
              "archutils/Darwin/Crash.h"
              "archutils/Darwin/DarwinThreadHelpers.h"
              "archutils/Darwin/HIDDevice.h"
              "archutils/Darwin/JoystickDevice.h"
              "archutils/Darwin/KeyboardDevice.h"
              "archutils/Darwin/MouseDevice.h"
              "archutils/Darwin/PumpDevice.h"
              "archutils/Darwin/SpecialDirs.h"
              "archutils/Darwin/StepMania.pch" # precompiled header.
              "archutils/Darwin/VectorHelper.h")

  source_group("OS Specific\\\\Darwin"
               FILES
               ${SMDATA_OS_DARWIN_SRC}
               ${SMDATA_OS_DARWIN_HPP})

  list(APPEND SMDATA_OS_SRC ${SMDATA_OS_DARWIN_SRC})
  list(APPEND SMDATA_OS_HPP ${SMDATA_OS_DARWIN_HPP})
else()
  if(WIN32)
    list(APPEND SMDATA_OS_SRC
                "archutils/Win32/AppInstance.cpp"
                "archutils/Win32/arch_setup.cpp"
                "archutils/Win32/arch_time.cpp"
                "archutils/Win32/CommandLine.cpp"
                "archutils/Win32/Crash.cpp"
                "archutils/Win32/CrashHandlerChild.cpp"
                "archutils/Win32/CrashHandlerNetworking.cpp"
                "archutils/Win32/DebugInfoHunt.cpp"
                "archutils/Win32/DialogUtil.cpp"
                "archutils/Win32/DirectXGuids.cpp"
                "archutils/Win32/DirectXHelpers.cpp"
                "archutils/Win32/ErrorStrings.cpp"
                "archutils/Win32/GetFileInformation.cpp"
                "archutils/Win32/GotoURL.cpp"
                "archutils/Win32/GraphicsWindow.cpp"
                "archutils/Win32/MessageWindow.cpp"
                "archutils/Win32/RegistryAccess.cpp"
                "archutils/Win32/RestartProgram.cpp"
                "archutils/Win32/SpecialDirs.cpp"
                "archutils/Win32/USB.cpp"
                "archutils/Win32/VideoDriverInfo.cpp"
                "archutils/Win32/WindowIcon.cpp"
                "archutils/Win32/WindowsDialogBox.cpp"
                "archutils/Win32/WindowsResources.rc")

    list(APPEND SMDATA_OS_HPP
                "archutils/Win32/AppInstance.h"
                "archutils/Win32/arch_setup.h"
                "archutils/Win32/CommandLine.h"
                "archutils/Win32/Crash.h"
                "archutils/Win32/CrashHandlerInternal.h"
                "archutils/Win32/CrashHandlerNetworking.h"
                "archutils/Win32/DebugInfoHunt.h"
                "archutils/Win32/DialogUtil.h"
                "archutils/Win32/DirectXHelpers.h"
                "archutils/Win32/ErrorStrings.h"
                "archutils/Win32/GetFileInformation.h"
                "archutils/Win32/GotoURL.h"
                "archutils/Win32/GraphicsWindow.h"
                "archutils/Win32/MessageWindow.h"
                "archutils/Win32/RegistryAccess.h"
                "archutils/Win32/RestartProgram.h"
                "archutils/Win32/SpecialDirs.h"
                "archutils/Win32/USB.h"
                "archutils/Win32/VideoDriverInfo.h"
                "archutils/Win32/WindowIcon.h"
                "archutils/Win32/WindowsDialogBox.h"
                "archutils/Win32/WindowsResources.h")
  else() # Unix
    list(APPEND SMDATA_OS_SRC # TODO: X11 check, crash handler check
                "archutils/Unix/AssertionHandler.cpp"
                "archutils/Unix/EmergencyShutdown.cpp"
                "archutils/Unix/GetSysInfo.cpp"
                "archutils/Unix/RunningUnderValgrind.cpp"
                "archutils/Unix/SignalHandler.cpp"
                "archutils/Unix/SpecialDirs.cpp"
                "archutils/Unix/StackCheck.cpp")
    list(APPEND SMDATA_OS_HPP
                "archutils/Unix/arch_setup.h"
                "archutils/Unix/AssertionHandler.h"
                "archutils/Unix/EmergencyShutdown.h"
                "archutils/Unix/GetSysInfo.h"
                "archutils/Unix/RunningUnderValgrind.h"
                "archutils/Unix/SignalHandler.h"
                "archutils/Unix/SpecialDirs.h"
                "archutils/Common/gcc_byte_swaps.h")
    if(X11_FOUND)
      list(APPEND SMDATA_OS_SRC "archutils/Unix/X11Helper.cpp")
      list(APPEND SMDATA_OS_HPP "archutils/Unix/X11Helper.h")
    endif()
    if(HAS_PTHREAD)
      list(APPEND SMDATA_OS_SRC "archutils/Common/PthreadHelpers.cpp")
      list(APPEND SMDATA_OS_HPP "archutils/Common/PthreadHelpers.h")
    endif()
  endif()
  source_group("OS Specific" FILES ${SMDATA_OS_SRC} ${SMDATA_OS_HPP})
endif()

if(APPLE OR LINUX)
  if(WITH_CRASH_HANDLER)
    list(APPEND SMDATA_OS_UNIX_CRASH_SRC
                "archutils/Unix/Backtrace.cpp"
                "archutils/Unix/BacktraceNames.cpp"
                "archutils/Unix/CrashHandler.cpp"
                "archutils/Unix/CrashHandlerChild.cpp"
                "archutils/Unix/CrashHandlerInternal.cpp"
                "archutils/Unix/SignalHandler.cpp")
    list(APPEND SMDATA_OS_UNIX_CRASH_HPP
                "archutils/Unix/Backtrace.h"
                "archutils/Unix/BacktraceNames.h"
                "archutils/Unix/CrashHandler.h"
                "archutils/Unix/CrashHandlerInternal.h"
                "archutils/Unix/SignalHandler.h")
    source_group("OS Specific\\\\Unix"
                 FILES
                 ${SMDATA_OS_UNIX_CRASH_SRC}
                 ${SMDATA_OS_UNIX_CRASH_HPP})

    list(APPEND SMDATA_OS_SRC ${SMDATA_OS_UNIX_CRASH_SRC})
    list(APPEND SMDATA_OS_HPP ${SMDATA_OS_UNIX_CRASH_HPP})
  endif()
endif()
