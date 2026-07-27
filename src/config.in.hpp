#ifndef CONFIG_HPP_
#define CONFIG_HPP_

/* Auto-generated config.h file powered by cmake. */

/* Defined to 1 if <alloca.h> is found. */
#cmakedefine HAVE_ALLOCA_H 1

/* Defined to 1 if <dirent.h> is found. */
#cmakedefine HAVE_DIRENT_H 1

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

/* Defined to 1 if the underlying system provides the stricmp function. */
#cmakedefine HAVE_STRICMP 1

/* Defined to 1 if the underlying system provides the _stricmp function. */
#cmakedefine HAVE__STRICMP 1

/* Defined to 1 if the underlying system provides the strcasecmp function. */
#cmakedefine HAVE_STRCASECMP 1

/* Defined to 1 if the underlying system provides the strtof function. */
#cmakedefine HAVE_STRTOF 1

/* Defined to 1 if the underlying system provides the M_PI constant. */
#cmakedefine HAVE_M_PI 1

/* Defined to 1 if the underlying system provides the posix_fadvise function. */
#cmakedefine HAVE_POSIX_FADVISE 1

/* Defined to 1 if the underlying system provides the pthread_mutex_timedlock function. */
#cmakedefine HAVE_PTHREAD_MUTEX_TIMEDLOCK 1

/* Defined to 1 if the underlying system provides the pthread_cond_timedwait function. */
#cmakedefine HAVE_PTHREAD_COND_TIMEDWAIT 1

/* Provide a fallback if pid_t is not defined. */
#cmakedefine HAVE_SIZEOF_PID_T 1
#if !defined(HAVE_SIZEOF_PID_T)
typedef int pid_t;
#endif

/* Defined to 1 if logging timing segment additions and removals. */
#cmakedefine WITH_LOGGING_TIMING_DATA 1

#if defined(__GNUC__)
/** @brief Define a macro to tell the compiler that a function has printf()
 * semantics, to aid warning output. */
#define PRINTF(a,b) __attribute__((format(__printf__,a,b)))
#define CONST_FUNCTION __attribute__((const))
#else
/** @brief A dummy define to keep things going smoothly. */
#define PRINTF(a,b)
/** @brief A dummy define to keep things going smoothly. */
#define CONST_FUNCTION
#endif

/* Ensure we have a function that acts like a size limited sprintf. */
#if defined(HAVE_SNPRINTF)
#elif defined(HAVE__SNPRINTF)
#define snprintf _snprintf
#else
#error "No size limited sprintf function available. Aborting."
#endif

/* Ensure we have a function that acts like a case insensitive string comparison. */
#if defined(HAVE_STRCASECMP)
#elif defined(HAVE__STRICMP)
#define strcasecmp _stricmp
#elif defined(HAVE_STRICMP)
#define strcasecmp stricmp
#else
#error "No case insensitive string comparison function available. Aborting."
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
#endif
#endif

/* Ensure we have a function for converting a string to a float. */
#if !defined(HAVE_STRTOF)
inline float strtof( const char *s, char **se ) { return (float) strtod( s, se ); }
#endif

#if !defined(HAVE_M_PI)
/* This is only called if the math header files don't include it: stating it here is fine. */
#define M_PI 3.1415926535897932384626433832795
#endif

/* Define standard endianness macros, if they're missing. */
#if defined(HAVE_ENDIAN_H)
#include <endian.h>
#elif defined(HAVE_MACHINE_ENDIAN_H)
#include <machine/endian.h>
#else

#endif

#endif
