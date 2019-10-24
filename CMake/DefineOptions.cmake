# Prep options that are needed for each platform.

# This option allows for networking support with StepMania.
option(WITH_NETWORKING "Build with networking support." ON)

# This option quiets warnings that are a part of external projects.
option(WITH_EXTERNAL_WARNINGS
       "Build with warnings for all components, not just StepMania." OFF)

# This option is not yet working, but will likely default to ON in the future.
option(WITH_LTO
       "Build with Link Time Optimization (LTO)/Whole Program Optimization."
       OFF)

# This option handles if we use SSE2 processing.
option(WITH_SSE2 "Build with SSE2 Optimizations." ON)

# This option may go away in the future: if it does, JPEG will always be
# required.
option(WITH_JPEG "Build with JPEG Image Support." ON)

# Turn this on to set this to a specific release mode.
option(WITH_FULL_RELEASE "Build as a proper, full release." OFF)

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

# Turn this option off to not use the GPL exclusive components.
option(WITH_GPL_LIBS "Build with GPL libraries." ON)

# Turn this option off to disable using WAV files with the game. Note that it is
# recommended to keep this on.
option(WITH_WAV "Build with WAV Support." ON)

# Turn this option off to disable using MP3 files with the game.
option(WITH_MP3 "Build with MP3 Support." ON)

# Turn this option off to disable using OGG files with the game.
option(WITH_OGG "Build with OGG/Vorbis Support." ON)

# Turn this option on to log every segment added or removed.
option(WITH_LOGGING_TIMING_DATA
       "Build with logging all Add and Erase Segment calls." OFF)

option(WITH_SYSTEM_PNG "Build with system PNG library (may not work on 1.6+)"
       OFF)
option(WITH_SYSTEM_OGG "Build with system OGG libraries" OFF)
option(WITH_SYSTEM_GLEW "Build with system GLEW library" OFF)
option(WITH_SYSTEM_TOMMATH "Build with system libtommath" OFF)
option(WITH_SYSTEM_TOMCRYPT "Build with system libtomcrypt" OFF)
option(WITH_SYSTEM_MAD "Build with system libmad" OFF)
option(WITH_SYSTEM_JSONCPP "Build with system jsoncpp" OFF)
option(WITH_SYSTEM_JPEG "Build with system jpeglib" OFF)
option(WITH_SYSTEM_PCRE "Build with system PCRE" OFF)
option(WITH_SYSTEM_ZLIB "Build against system zlib" OFF)

option(WITH_SDL "Build with SDL" OFF)

if(NOT MSVC)
  # Turn this option off to disable using FFMEPG.
  option(WITH_FFMPEG "Build with FFMPEG." ON)
  # Change this number to utilize a different number of jobs for building
  # FFMPEG.
  option(WITH_FFMPEG_JOBS "Build FFMPEG with this many jobs." 2)
else()
  # Turn this option on to enable using the Texture Font Generator.
  option(
    WITH_TEXTURE_GENERATOR
    "Build with the Texture Font Generator. Ensure the MFC library is installed."
    OFF)
  # Turn this option off to use dynamic linking instead of static linking.
  option(WITH_STATIC_LINKING "Build StepMania with static linking." ON)
endif()

if(WIN32)
  option(WITH_MINIMAID "Build with Minimaid Lights Support." OFF)
  # Developer only option: connect to IRC to report the result. Only use with
  # build servers.
  option(WITH_IRC_POST_HOOK "Report via IRC of the success afterwards." OFF)
elseif(LINUX)
  # Builder beware: later versions of ffmpeg may break!
  option(WITH_SYSTEM_FFMPEG
         "Build with the system's FFMPEG, disabled build with bundled's FFMPEG"
         OFF)
  option(WITH_CRYSTALHD_DISABLED "Build FFMPEG without Crystal HD support." OFF)
  option(WITH_MINIMAID "Build with Minimaid Lights Support." OFF)
  option(WITH_TTY "Build with Linux TTY Input Support." OFF)
  option(WITH_PROFILING "Build with Profiling Support." OFF)
  option(WITH_GLES2 "Build with OpenGL ES 2.0 Support." ON)
  option(WITH_GTK2 "Build with GTK2 Support." ON)
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
