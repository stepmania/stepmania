#ifndef CONFIG_HPP_
#define CONFIG_HPP_

/* Auto-generated config.h file powered by cmake. */

/* Include necessary items from the rage project. */
#include "RageConfig.hpp"

/* Defined to 1 if <alloca.h> is found. */
#cmakedefine HAVE_ALLOCA_H 1

/* Defined to 1 if <dirent.h> is found. */
#cmakedefine HAVE_DIRENT_H 1

/* Defined to 1 if <inttypes.h> is found. */
#cmakedefine HAVE_INTTYPES_H 1

/* Defined to 1 if <stdint.h> is found. */
#cmakedefine HAVE_STDINT_H 1

/* Defined to 1 if <endian.h> is found. */
#cmakedefine HAVE_ENDIAN_H 1

/* Defined to 1 if <sys/endian.h> is found. */
#cmakedefine HAVE_SYS_ENDIAN_H 1

/* Defined to 1 if <machine/endian.h> is found. */
#cmakedefine HAVE_MACHINE_ENDIAN_H 1

/* Defined to 1 if <sys/param.h> is found. */
#cmakedefine HAVE_SYS_PARAM_H 1

/* Defined to 1 if <sys/utsname.h> is found. */
#cmakedefine HAVE_SYS_UTSNAME_H 1

/* Defined to 1 if <fcntl.h> is found. */
#cmakedefine HAVE_FCNTL_H 1

/* Defined to 1 if <unistd.h> is found. */
#cmakedefine HAVE_UNISTD_H 1

/* Defined to 1 if the underlying system provides the _mkdir function. */
#cmakedefine HAVE__MKDIR 1

/* Defined to 1 if the underlying system provides the mkdir function. */
#cmakedefine HAVE_MKDIR 1

/* Defined to 1 if the underlying system provides the _snprintf function. */
#cmakedefine HAVE__SNPRINTF 1

/* Defined to 1 if the underlying system provides the snprintf function. */
#cmakedefine HAVE_SNPRINTF 1

/* Defined to 1 if the underlying system provides the strtof function. */
#cmakedefine HAVE_STRTOF 1

/* Defined to 1 if the underlying system is using X11. */
#cmakedefine X11_FOUND 1

/* Defined to 1 if the underlying system provides the posix_fadvise function. */
#cmakedefine HAVE_POSIX_FADVISE 1

/* Defined to 1 if the underlying system provides the pthread_mutex_timedlock function. */
#cmakedefine HAVE_PTHREAD_MUTEX_TIMEDLOCK 1

/* Defined to 1 if the underlying system provides the pthread_cond_timedwait function. */
#cmakedefine HAVE_PTHREAD_COND_TIMEDWAIT 1

/* Provide a fallback if intptr_t is not defined. */
#cmakedefine HAVE_SIZEOF_INTPTR_T 1
#if !defined(HAVE_SIZEOF_INTPTR_T)
typedef unsigned int intptr_t;
#endif

/* Provide a fallback if pid_t is not defined. */
#cmakedefine HAVE_SIZEOF_PID_T 1
#if !defined(HAVE_SIZEOF_PID_T)
typedef int pid_t;
#endif

/* Provide a fallback if size_t is not defined. */
#cmakedefine HAVE_SIZEOF_SIZE_T 1
#if !defined(HAVE_SIZEOF_SIZE_T)
typedef unsigned long size_t;
#endif

/* Provide a fallback if ssize_t is not defined. */
#cmakedefine HAVE_SIZEOF_SSIZE_T 1
#if !defined(HAVE_SIZEOF_SSIZE_T)
typedef long ssize_t;
#endif

/* Defined to 1 if the underlying system uses little endian. */
#cmakedefine ENDIAN_LITTLE 1

/* Defined to 1 if the underlying system uses big endian. */
#cmakedefine ENDIAN_BIG 1

/* Defined to 1 if compiling with WAV support. */
#cmakedefine HAS_WAV 1

/* Defined to 1 if compiling with MP3 support. */
#cmakedefine HAS_MP3 1

/* Defined to 1 if compiling with OGG support. */
#cmakedefine HAS_OGG 1

/* Defined to 1 if building on a windows system, and thus uses the windows loading window. */
#cmakedefine NEED_WINDOWS_LOADING_WINDOW 1

/* Defined to 1 if logging timing segment additions and removals. */
#cmakedefine WITH_LOGGING_TIMING_DATA 1

/* Defined to 1 if the PBS_MARQUEE symbol was found. */
#cmakedefine HAVE_PBS_MARQUEE 1

/* Defined to 1 if the PBM_SETMARQUEE symbol was found. */
#cmakedefine HAVE_PBM_SETMARQUEE 1

#if defined(NEED_WINDOWS_LOADING_WINDOW)
#if !defined(HAVE_PBS_MARQUEE)
#define PBS_MARQUEE 0x08
#endif
#if !defined(HAVE_PBM_SETMARQUEE)
#define PBM_SETMARQUEE (WM_USER+10)
#endif
#endif

/* Ensure we have a function that acts like a size limited sprintf. */
#if defined(HAVE_SNPRINTF)
#elif defined(HAVE__SNPRINTF)
#define snprintf _snprintf
#else
#error "No size limited sprintf function available. Aborting."
#endif

/* Ensure we have a function that can create a directory on the file system. */
#if defined(HAVE__MKDIR)
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#if !defined(HAVE_MKDIR)
#error "No create directory function available. Aborting."
#else
#include <sys/types.h>
#include <sys/stat.h>
#if defined(__MINGW32__) || defined(__MINGW64__)
#define mkdir(path, mode) mkdir(path)
#endif
#endif
#endif

/* Ensure we have a function for converting a string to a float. */
#if !defined(HAVE_STRTOF)
inline float strtof( const char *s, char **se ) { return (float) strtod( s, se ); }
#endif

/* Ensure we have an endianness defined. */
#if !defined(ENDIAN_LITTLE) && !defined(ENDIAN_BIG)
#error "Neither ENDIAN_LITTLE nor ENDIAN_BIG defined. Aborting."
#endif

/* Define standard endianness macros, if they're missing. */
#if defined(HAVE_ENDIAN_H)
#include <endian.h>
#elif defined(HAVE_MACHINE_ENDIAN_H)
#include <machine/endian.h>
#else
/** @brief The macro for little endian order. */
#define LITTLE_ENDIAN 1234
/** @brief The macro for big endian order. */
#define BIG_ENDIAN 4321
#if defined(ENDIAN_LITTLE)
#define BYTE_ORDER LITTLE_ENDIAN
#elif defined(ENDIAN_BIG)
#define BYTE_ORDER BIG_ENDIAN
#endif

#endif

#endif
