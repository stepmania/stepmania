# TODO: Move libtom* to extern
add_subdirectory(libtomcrypt)
add_subdirectory(libtommath)


# First part is for the version incrementer tool.
add_subdirectory(libversionupdater)

# Run the executable above.
add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/verdata.cpp
	COMMAND VersionUpdater ${CMAKE_CURRENT_BINARY_DIR}/verdata.cpp
	DEPENDS VersionUpdater
)

# Include this folder itself.
include_directories(
	"."
	"../extern/ffmpeg/include"
	"../extern/glew-1.5.8/include"
)

# Add the main library.

list(APPEND SMDATA_GLOBAL_FILES_SRC
	"GameLoop.cpp"
	"global.cpp"
	"SpecialFiles.cpp"
	# StepMania will be refactored to not have its main.
)

list(APPEND SMDATA_GLOBAL_FILES_HPP
	"GameLoop.h"
	"global.h"
	"ProductInfo.h" # At some point, have some of this file be generated perhaps.
	"SpecialFiles.h"
	"StdString.h" # At some point, remove the need for this file. We don't support Visual Studio 6.
	# StepMania will be refactored to not have its main.
)

source_group("Global Files" FILES ${SMDATA_GLOBAL_FILES_SRC} ${SMDATA_GLOBAL_FILES_HPP})

list(APPEND SMDATA_ARCH_SRC
	"arch/RageDriver.cpp"
)

list(APPEND SMDATA_ARCH_HPP
	"arch/arch_default.h"
	"arch/RageDriver.h"
)

source_group("Arch Specific" FILES ${SMDATA_ARCH_SRC} ${SMDATA_ARCH_HPP})

list(APPEND SMDATA_ARCH_THREADS_HPP
	"arch/Threads/Threads.h"
)

if(WIN32)
	list(APPEND SMDATA_ARCH_THREADS_HPP
		"arch/Threads/Threads_Win32.h"
	)

	list(APPEND SMDATA_ARCH_THREADS_SRC
		"arch/Threads/Threads_Win32.cpp"
	)
else(NOT WIN32)
	list(APPEND SMDATA_ARCH_THREADS_HPP
		"arch/Threads/Threads_Pthreads.h"
	)
	list(APPEND SMDATA_ARCH_THREADS_SRC
		"arch/Threads/Threads_Pthreads.cpp"
	)
endif(WIN32)

source_group("Arch Specific\\\\Threads" FILES ${SMDATA_ARCH_THREADS_SRC} ${SMDATA_ARCH_THREADS_HPP})


list(APPEND SMDATA_ARCH_SOUND_SRC
	"arch/Sound/RageSoundDriver.cpp"
	"arch/Sound/RageSoundDriver_Generic_Software.cpp"
	"arch/Sound/RageSoundDriver_Null.cpp"
)

list(APPEND SMDATA_ARCH_SOUND_HPP
	"arch/Sound/RageSoundDriver.h"
	"arch/Sound/RageSoundDriver_Null.h"
)

if(WIN32)
	list(APPEND SMDATA_ARCH_SOUND_SRC
		"arch/Sound/DSoundHelpers.cpp"
		"arch/Sound/RageSoundDriver_DSound_Software.cpp"
		"arch/Sound/RageSoundDriver_WaveOut.cpp"
		"arch/Sound/RageSoundDriver_WDMKS.cpp"
	)
	list(APPEND SMDATA_ARCH_SOUND_HPP
		"arch/Sound/DSoundHelpers.h"
		"arch/Sound/RageSoundDriver_DSound_Software.h"
		"arch/Sound/RageSoundDriver_WaveOut.h"
		"arch/Sound/RageSoundDriver_WDMKS.h"
	)
elseif(APPLE)
	list(APPEND SMDATA_ARCH_SOUND_SRC
		"arch/Sound/RageSoundDriver_AU.cpp"
	)
	list(APPEND SMDATA_ARCH_SOUND_HPP
		"arch/Sound/RageSoundDriver_AU.h"
	)
else(UNIX)
	# TODO: Address Jack/Alsa situation.

endif(WIN32)

source_group("Arch Specific\\\\Sound" FILES ${SMDATA_ARCH_SOUND_SRC} ${SMDATA_ARCH_SOUND_HPP})

list(APPEND SMDATA_ARCH_MOVIE_TEXTURE_SRC
	"arch/MovieTexture/MovieTexture.cpp"
	"arch/MovieTexture/MovieTexture_FFMpeg.cpp"
	"arch/MovieTexture/MovieTexture_Generic.cpp"
	"arch/MovieTexture/MovieTexture_Null.cpp"
)

list(APPEND SMDATA_ARCH_MOVIE_TEXTURE_HPP
	"arch/MovieTexture/MovieTexture.h"
	"arch/MovieTexture/MovieTexture_FFMpeg.h"
	"arch/MovieTexture/MovieTexture_Generic.h"
	"arch/MovieTexture/MovieTexture_Null.h"
)

source_group("Arch Specific\\\\Movie Texture" FILES ${SMDATA_ARCH_MOVIE_TEXTURE_SRC} ${SMDATA_ARCH_MOVIE_TEXTURE_HPP})

list(APPEND SMDATA_ARCH_MEMORY_SRC
	"arch/MemoryCard/MemoryCardDriver.cpp"
)

list(APPEND SMDATA_ARCH_MEMORY_HPP
	"arch/MemoryCard/MemoryCardDriver.h"
	"arch/MemoryCard/MemoryCardDriver_Null.h"
)

if(WIN32)
	list(APPEND SMDATA_ARCH_MEMORY_SRC
		"arch/MemoryCard/MemoryCardDriverThreaded_Windows.cpp"
	)
	list(APPEND SMDATA_ARCH_MEMORY_HPP
		"arch/MemoryCard/MemoryCardDriverThreaded_Windows.h"
	)
elseif(APPLE)
	list(APPEND SMDATA_ARCH_MEMORY_SRC
		"arch/MemoryCard/MemoryCardDriverThreaded_MacOSX.cpp"
	)
	list(APPEND SMDATA_ARCH_MEMORY_HPP
		"arch/MemoryCard/MemoryCardDriverThreaded_MacOSX.h"
	)
else(UNIX)
	list(APPEND SMDATA_ARCH_MEMORY_SRC
		"arch/MemoryCard/MemoryCardDriverThreaded_Linux.cpp"
	)
	list(APPEND SMDATA_ARCH_MEMORY_HPP
		"arch/MemoryCard/MemoryCardDriverThreaded_Linux.h"
	)
endif(WIN32)

source_group("Arch Specific\\\\Memory Card" FILES ${SMDATA_ARCH_MEMORY_SRC} ${SMDATA_ARCH_MEMORY_HPP})

list(APPEND SMDATA_ARCH_LOWLEVEL_SRC
	"arch/LowLevelWindow/LowLevelWindow.cpp"
)
list(APPEND SMDATA_ARCH_LOWLEVEL_HPP
	"arch/LowLevelWindow/LowLevelWindow.h"
)

if(WIN32)
	list(APPEND SMDATA_ARCH_LOWLEVEL_SRC
		"arch/LowLevelWindow/LowLevelWindow_Win32.cpp"
	)
	list(APPEND SMDATA_ARCH_LOWLEVEL_HPP
		"arch/LowLevelWindow/LowLevelWindow_Win32.h"
	)
elseif(APPLE)
	list(APPEND SMDATA_ARCH_LOWLEVEL_SRC
		"arch/LowLevelWindow/LowLevelWindow_MacOSX.mm"
	)
	list(APPEND SMDATA_ARCH_LOWLEVEL_HPP
		"arch/LowLevelWindow/LowLevelWindow_MacOSX.h"
	)
else(UNIX)
	list(APPEND SMDATA_ARCH_LOWLEVEL_SRC
		"arch/LowLevelWindow/LowLevelWindow_X11.cpp"
	)
	list(APPEND SMDATA_ARCH_LOWLEVEL_HPP
		"arch/LowLevelWindow/LowLevelWindow_X11.h"
	)
endif(WIN32)

source_group("Arch Specific\\\\Low Level Window" FILES ${SMDATA_ARCH_LOWLEVEL_SRC} ${SMDATA_ARCH_LOWLEVEL_HPP})

list(APPEND SMDATA_ARCH_LOADING_SRC
	"arch/LoadingWindow/LoadingWindow.cpp"
)
list(APPEND SMDATA_ARCH_LOADING_HPP
	"arch/LoadingWindow/LoadingWindow.h"
)

if(WIN32)
	list(APPEND SMDATA_ARCH_LOADING_SRC
		"arch/LoadingWindow/LoadingWindow_Win32.cpp"
	)
	list(APPEND SMDATA_ARCH_LOADING_HPP
		"arch/LoadingWindow/LoadingWindow_Win32.h"
	)
else(WIN32)
	list(APPEND SMDATA_ARCH_LOADING_HPP
		"arch/LoadingWindow/LoadingWindow_Null.h"
	)
	if(APPLE)
		list(APPEND SMDATA_ARCH_LOADING_SRC
		"arch/LoadingWindow/LoadingWindow_MacOSX.mm"
	)
	list(APPEND SMDATA_ARCH_LOADING_HPP
		"arch/LoadingWindow/LoadingWindow_MacOSX.h"
	)
	else(APPLE)
		# TODO: GTK stuff for linux.
	endif(APPLE)
endif(WIN32)

source_group("Arch Specific\\\\Loading Window" FILES ${SMDATA_ARCH_LOADING_SRC} ${SMDATA_ARCH_LOADING_HPP})

list(APPEND SMDATA_ARCH_LIGHTS_SRC
	"arch/Lights/LightsDriver.cpp"
	"arch/Lights/LightsDriver_SystemMessage.cpp"
)
list(APPEND SMDATA_ARCH_LIGHTS_HPP
	"arch/Lights/LightsDriver.h"
	"arch/Lights/LightsDriver_SystemMessage.h"
)

# TODO: Confirm if Apple can use the export.
if(NOT APPLE)
	list(APPEND SMDATA_ARCH_LIGHTS_SRC
		"arch/Lights/LightsDriver_Export.cpp"
	)
	list(APPEND SMDATA_ARCH_LIGHTS_HPP
		"arch/Lights/LightsDriver_Export.h"
	)
	if(WIN32)
		list(APPEND SMDATA_ARCH_LIGHTS_SRC
			"arch/Lights/LightsDriver_Win32Parallel.cpp"
		)
		list(APPEND SMDATA_ARCH_LIGHTS_HPP
			"arch/Lights/LightsDriver_Win32Parallel.h"
		)
	else(WIN32)
		# TODO: Linux HAVE_PARALLEL_PORT
		list(APPEND SMDATA_ARCH_LIGHTS_SRC
			"arch/Lights/LightsDriver_LinuxParallel.cpp"
		)
		list(APPEND SMDATA_ARCH_LIGHTS_HPP
			"arch/Lights/LightsDriver_LinuxParallel.h"
		)
	endif(WIN32)
endif(NOT APPLE)

source_group("Arch Specific\\\\Lights" FILES ${SMDATA_ARCH_LIGHTS_SRC} ${SMDATA_ARCH_LIGHTS_HPP})

list(APPEND SMDATA_ARCH_INPUT_SRC
	"arch/InputHandler/InputHandler.cpp"
	"arch/InputHandler/InputHandler_MonkeyKeyboard.cpp"
)
list(APPEND SMDATA_ARCH_INPUT_HPP
	"arch/InputHandler/InputHandler.h"
	"arch/InputHandler/InputHandler_MonkeyKeyboard.h"
)

if(WIN32)
	list(APPEND SMDATA_ARCH_INPUT_SRC
		"arch/InputHandler/InputHandler_DirectInput.cpp"
		"arch/InputHandler/InputHandler_DirectInputHelper.cpp"
		"arch/InputHandler/InputHandler_Win32_MIDI.cpp"
		"arch/InputHandler/InputHandler_Win32_Para.cpp"
		"arch/InputHandler/InputHandler_Win32_Pump.cpp"
	)
	list(APPEND SMDATA_ARCH_INPUT_HPP
		"arch/InputHandler/InputHandler_DirectInput.h"
		"arch/InputHandler/InputHandler_DirectInputHelper.h"
		"arch/InputHandler/InputHandler_Win32_MIDI.h"
		"arch/InputHandler/InputHandler_Win32_Para.h"
		"arch/InputHandler/InputHandler_Win32_Pump.h"
	)
elseif(APPLE)
	list(APPEND SMDATA_ARCH_INPUT_SRC
		"arch/InputHandler_MacOSX_HID.cpp"
	)
	list(APPEND SMDATA_ARCH_INPUT_HPP
		"arch/InputHandler_MacOSX_HID.h"
	)
else(UNIX)
	list(APPEND SMDATA_ARCH_INPUT_SRC
		"arch/InputHandler/LinuxInputManager.cpp"
		"arch/InputHandler/InputHandler_Linux_Joystick.cpp"
		"arch/InputHandler/InputHandler_Linux_Event.cpp"
		"arch/InputHandler/InputHandler_Linux_PIUIO.cpp"
	)
	list(APPEND SMDATA_ARCH_INPUT_SRC
		"arch/InputHandler/LinuxInputManager.h"
		"arch/InputHandler/InputHandler_Linux_Joystick.h"
		"arch/InputHandler/InputHandler_Linux_Event.h"
		"arch/InputHandler/InputHandler_Linux_PIUIO.h"
	)
endif(WIN32)

source_group("Arch Specific\\\\Input Handler" FILES ${SMDATA_ARCH_INPUT_SRC} ${SMDATA_ARCH_INPUT_HPP})

list(APPEND SMDATA_ARCH_DIALOG_SRC
	"arch/Dialog/Dialog.cpp"
	"arch/Dialog/DialogDriver.cpp"
)
list(APPEND SMDATA_ARCH_DIALOG_HPP
	"arch/Dialog/Dialog.h"
	"arch/Dialog/DialogDriver.h"
)

if(WIN32)
	list(APPEND SMDATA_ARCH_DIALOG_SRC
		"arch/Dialog/DialogDriver_Win32.cpp"
	)
	list(APPEND SMDATA_ARCH_DIALOG_HPP
		"arch/Dialog/DialogDriver_Win32.h"
	)
elseif(APPLE)
	list(APPEND SMDATA_ARCH_DIALOG_SRC
		"arch/Dialog/DialogDriver_MacOSX.cpp"
	)
	list(APPEND SMDATA_ARCH_DIALOG_HPP
		"arch/Dialog/DialogDriver_MacOSX.h"
	)
endif(WIN32)

source_group("Arch Specific\\\\Dialog" FILES ${SMDATA_ARCH_DIALOG_SRC} ${SMDATA_ARCH_DIALOG_HPP})

list(APPEND SMDATA_ARCH_HOOKS_SRC
	"arch/ArchHooks/ArchHooks.cpp"
)
list(APPEND SMDATA_ARCH_HOOKS_HPP
	"arch/ArchHooks/ArchHooks.h"
)

if(NOT APPLE)
	list(APPEND SMDATA_ARCH_HOOKS_SRC
		"arch/ArchHooks/ArchHooksUtil.cpp"
	)
	if(WIN32)
		list(APPEND SMDATA_ARCH_HOOKS_SRC
			"arch/ArchHooks/ArchHooks_Win32.cpp"
			"arch/ArchHooks/ArchHooks_Win32Static.cpp"
		)
		list(APPEND SMDATA_ARCH_HOOKS_HPP
			"arch/ArchHooks/ArchHooks_Win32.h"
		)
	else(WIN32)
		list(APPEND SMDATA_ARCH_HOOKS_SRC
			"arch/ArchHooks/ArchHooks_Unix.cpp"
		)
		list(APPEND SMDATA_ARCH_HOOKS_HPP
			"arch/ArchHooks/ArchHooks_Unix.h"
		)
	endif(WIN32)
else(NOT APPLE)
	list(APPEND SMDATA_ARCH_HOOKS_SRC
		"arch/ArchHooks/ArchHooks_MacOSX.cpp"
	)
	list(APPEND SMDATA_ARCH_HOOKS_HPP
		"arch/ArchHooks/ArchHooks_MacOSX.h"
	)
endif(NOT APPLE)

source_group("Arch Specific\\\\Arch Hooks" FILES ${SMDATA_ARCH_HOOKS_SRC} ${SMDATA_ARCH_HOOKS_HPP})

list(APPEND SMDATA_ALL_ARCH_SRC
	${SMDATA_ARCH_SRC}
	${SMDATA_ARCH_HOOKS_SRC}
	${SMDATA_ARCH_DIALOG_SRC}
	${SMDATA_ARCH_INPUT_SRC}
	${SMDATA_ARCH_LIGHTS_SRC}
	${SMDATA_ARCH_LOADING_SRC}
	${SMDATA_ARCH_LOWLEVEL_SRC}
	${SMDATA_ARCH_MEMORY_SRC}
	${SMDATA_ARCH_MOVIE_TEXTURE_SRC}
	${SMDATA_ARCH_SOUND_SRC}
	${SMDATA_ARCH_THREADS_SRC}
)

list(APPEND SMDATA_ALL_ARCH_HPP
	${SMDATA_ARCH_HPP}
	${SMDATA_ARCH_HOOKS_HPP}
	${SMDATA_ARCH_DIALOG_HPP}
	${SMDATA_ARCH_INPUT_HPP}
	${SMDATA_ARCH_LIGHTS_HPP}
	${SMDATA_ARCH_LOADING_HPP}
	${SMDATA_ARCH_LOWLEVEL_HPP}
	${SMDATA_ARCH_MEMORY_HPP}
	${SMDATA_ARCH_MOVIE_TEXTURE_HPP}
	${SMDATA_ARCH_SOUND_HPP}
	${SMDATA_ARCH_THREADS_HPP}
)


source_group("Generated Files" FILES ${CMAKE_CURRENT_BINARY_DIR}/verdata.cpp)

list(APPEND SMDATA_ALL_FILES_SRC
	${SMDATA_GLOBAL_FILES_SRC}
	${SMDATA_ALL_ARCH_SRC}
	${CMAKE_CURRENT_BINARY_DIR}/verdata.cpp
)

list(APPEND SMDATA_ALL_FILES_HPP
	${SMDATA_ALL_ARCH_HPP}
	${SMDATA_GLOBAL_FILES_HPP}
)

if (WIN32)
	add_definitions(/D WINDOWS)
endif(WIN32)

add_library("StepMania-Data" ${SMDATA_ALL_FILES_SRC} ${SMDATA_ALL_FILES_HPP})

target_link_libraries("StepMania-Data"
	"glew"
	"lua-5.1"
	"pcre"
	"png"
	"tomcrypt"
	"tommath"
)

set_property(TARGET "StepMania-Data" PROPERTY FOLDER "Internal Libraries")
