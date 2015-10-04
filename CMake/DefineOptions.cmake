# Prep options that are needed for each platform.

# This option allows for networking support with StepMania.
option(WITH_NETWORKING "Build with networking support." ON)

# This option allows for additional version information to be built-in.
option(WITH_VERSION_INFO "Build with version information." ON)

# This option quiets warnings that are a part of external projects.
option(WITH_EXTERNAL_WARNINGS "Build with warnings for all components, not just StepMania." OFF)

# This option is not yet working, but will likely default to ON in the future.
option(WITH_LTO "Build with Link Time Optimization (LTO)/Whole Program Optimization." OFF)

# This option handles if we use SSE2 processing.
option(WITH_SSE2 "Build with SSE2 Optimizations." ON)

# This option may go away in the future: if it does, JPEG will always be required.
option(WITH_JPEG "Build with JPEG Image Support." ON)

# Turn this on to set this to a specific release mode.
option(WITH_FULL_RELEASE "Build as a proper, full release." OFF)

# Turn this on to compile tomcrypt with no assembly data. This is a portable mode.
option(WITH_PORTABLE_TOMCRYPT "Build with assembly/free tomcrypt, making it portable." ON)

# Turn this on to not use the ROLC assembly featurs of tomcrypt.
# If WITH_PORTABLE_TOMCRYPT is ON, this will automatically have no effect.
option(WITH_NO_ROLC_TOMCRYPT "Build without the ROLC assembly instructions for tomcrypt. (Ignored by Apple builds)" OFF)

# Turn this option off to not use the GPL exclusive components.
option(WITH_GPL_LIBS "Build with GPL libraries." ON)

# Turn this option off to disable using WAV files with the game.
# Note that it is recommended to keep this on.
option(WITH_WAV "Build with WAV Support." ON)

# Turn this option off to disable using MP3 files with the game.
option(WITH_MP3 "Build with MP3 Support." ON)

# Turn this option off to disable using OGG files with the game.
option(WITH_OGG "Build with OGG/Vorbis Support." ON)

if(NOT MSVC)
  option(WITH_FFMPEG "Build with FFMPEG." ON)
endif()

if(WIN32)
  option(WITH_MINIMAID "Build with Minimaid Lights Support." ON)
elseif(LINUX)
    # Builder beware: later versions of ffmpeg may break!
    option(WITH_SYSTEM_FFMPEG "Build with the system's FFMPEG." OFF)
    option(WITH_CRYSTALHD_DISABLED "Build FFMPEG without Crystal HD support." OFF)
    option(WITH_TTY "Build with Linux TTY Input Support." OFF)
    option(WITH_PROFILING "Build with Profiling Support." OFF)
    option(WITH_GLES2 "Build with OpenGL ES 2.0 Support." ON)
    option(WITH_GTK2 "Build with GTK2 Support." ON)
    option(WITH_PARALLEL_PORT "Build with Parallel Lights I/O Support." OFF)
    option(WITH_CRASH_HANDLER "Build with Crash Handler Support." ON)
endif()
