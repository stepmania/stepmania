# Prep options that are needed for each platform.

# This option quiets warnings that are a part of external projects.
option(WITH_EXTERNAL_WARNINGS
       "Build with warnings for all components, not just StepMania." OFF)

# This option is not yet working, but will likely default to ON in the future.
option(WITH_LTO
       "Build with Link Time Optimization (LTO)/Whole Program Optimization."
       OFF)

# This option handles if we use SSE2 processing.
option(WITH_SSE2 "Build with SSE2 Optimizations." ON)

# Turn this on to set this to a specific release mode.
option(WITH_FULL_RELEASE "Build as a proper, full release." OFF)

# Turn this on to include Club Fantastic songs
option(WITH_CLUB_FANTASTIC "Include Club Fantastic songs." OFF)

# Turn this on to compile tomcrypt with no assembly data. This is a portable
# mode.
option(WITH_PORTABLE_TOMCRYPT
       "Build with assembly/free tomcrypt, making it portable." ON)

# Turn this on to not use the ROLC assembly featurs of tomcrypt. If
# WITH_PORTABLE_TOMCRYPT is ON, this will automatically have no effect.
option(
  WITH_NO_ROLC_TOMCRYPT
  "Build without the ROLC assembly instructions for tomcrypt."
  OFF)

# Turn this option on to log every segment added or removed.
option(WITH_LOGGING_TIMING_DATA
       "Build with logging all Add and Erase Segment calls." OFF)

if(NOT MSVC)
  # Change this number to utilize a different number of jobs for building
  # FFMPEG.
  option(WITH_FFMPEG_JOBS "Build FFMPEG with this many jobs." 2)
else()
  # Turn this option on to enable using the Texture Font Generator.
  option(
    WITH_TEXTURE_GENERATOR
    "Build with the Texture Font Generator. Ensure the MFC library is installed."
    OFF)
endif()

if(LINUX)
  option(WITH_PROFILING "Build with Profiling Support." OFF)
  option(WITH_GLES2 "Build with OpenGL ES 2.0 Support." ON)
  option(WITH_GTK3 "Build with GTK3 Support." ON)
  option(WITH_PARALLEL_PORT "Build with Parallel Lights I/O Support." OFF)
  option(WITH_CRASH_HANDLER "Build with Crash Handler Support." ON)
  option(WITH_XINERAMA
         "Build using libXinerama to query for monitor numbers (if available)."
         ON)
  option(WITH_ALSA "Build with ALSA support" ON)
  option(WITH_PULSEAUDIO "Build with PulseAudio support" ON)
  option(WITH_JACK "Build with JACK support" OFF)
  option(WITH_XRANDR "Build with Xrandr support" ON)
  option(WITH_LIBXTST "Build with libXtst support" ON)
  option(WITH_X11 "Build with X11 support" ON)
endif()

option(WITH_MINIMAID "Build with Minimaid support." ON)

