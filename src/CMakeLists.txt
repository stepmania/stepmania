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
	.
	../extern/ffmpeg/include
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

list(APPEND SMDATA_ALL_ARCH_SRC
	${SMDATA_ARCH_SRC}
	${SMDATA_ARCH_MOVIE_TEXTURE_SRC}
	${SMDATA_ARCH_SOUND_SRC}
	${SMDATA_ARCH_THREADS_SRC}
)

list(APPEND SMDATA_ALL_ARCH_HPP
	${SMDATA_ARCH_HPP}
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

set_property(TARGET "StepMania-Data" PROPERTY FOLDER "Internal Libraries")