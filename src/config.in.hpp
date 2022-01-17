#ifndef CONFIG_HPP_
#define CONFIG_HPP_

/* Auto-generated config.h file powered by cmake. */

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

/* Defined to 1 if the underlying system provides the stricmp function. */
#cmakedefine HAVE_STRICMP 1

/* Defined to 1 if the underlying system provides the _stricmp function. */
#cmakedefine HAVE__STRICMP 1

/* Defined to 1 if the underlying system provides the strcasecmp function. */
#cmakedefine HAVE_STRCASECMP 1

/* Defined to 1 if the underlying system provides the powf function. */
#cmakedefine HAVE_POWF 1

/* Defined to 1 if the underlying system provides the sqrtf function. */
#cmakedefine HAVE_SQRTF 1

/* Defined to 1 if the underlying system provides the sinf function. */
#cmakedefine HAVE_SINF 1

/* Defined to 1 if the underlying system provides the tanf function. */
#cmakedefine HAVE_TANF 1

/* Defined to 1 if the underlying system provides the cos function. */
#cmakedefine HAVE_COSF 1

/* Defined to 1 if the underlying system provides the acos function. */
#cmakedefine HAVE_ACOSF 1

/* Defined to 1 if the underlying system provides the truncf function. */
#cmakedefine HAVE_TRUNCF 1

/* Defined to 1 if the underlying system provides the roundf function. */
#cmakedefine HAVE_ROUNDF 1

/* Defined to 1 if the underlying system provides the lrintf function. */
#cmakedefine HAVE_LRINTF 1

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
#include <type_traits>
typedef std::make_signed<size_t>::type ssize_t;
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
#if defined(__MINGW32__) || defined(__MINGW64__)
#define mkdir(path, mode) mkdir(path)
#endif
#endif
#endif

/* Ensure we have a function for raising a number by a power to a float. */
#if !defined(HAVE_POWF)
inline float powf( float x, float y ) CONST_FUNCTION;
float powf( float x, float y ) { return float( pow(double(x), double(y)) ); }
#endif

/* Ensure we have a function for getting the square root of a number to a float. */
#if !defined(HAVE_SQRTF)
inline float sqrtf( float x ) CONST_FUNCTION;
float sqrtf( float x ) { return float( sqrt(double(x)) ); }
#endif

/* Ensure we have a function for getting the sin of a number to a float. */
#if !defined(HAVE_SINF)
inline float sinf( float x ) CONST_FUNCTION;
float sinf( float x ) { return float( sin(double(x)) ); }
#endif

/* Ensure we have a function for getting the tan of a number to a float. */
#if !defined(HAVE_TANF)
inline float tanf( float x ) CONST_FUNCTION;
float tanf( float x ) { return float( tan(double(x)) ); }
#endif

/* Ensure we have a function for getting the cos of a number to a float. */
#if !defined(HAVE_COSF)
inline float cosf( float x ) CONST_FUNCTION;
float cosf( float x ){ return float( cos(double(x)) ); }
#endif

/* Ensure we have a function for getting the arc cos of a number to a float. */
#if !defined(HAVE_ACOSF)
inline float acosf( float x ) CONST_FUNCTION;
float acosf( float x ) { return float( acos(double(x)) ); }
#endif

/* Ensure we have a function for truncating a number to a float. */
#if !defined(HAVE_TRUNCF)
inline float truncf( float f ) CONST_FUNCTION;
float truncf( float f ) { return float( int(f) ); }
#endif

/* Ensure we have a function for rounding a number to a float. */
#if !defined(HAVE_ROUNDF)
inline float roundf( float f ) CONST_FUNCTION;
float roundf( float f ) { if( f < 0.0f ) return truncf( f-0.5f ); return truncf( f+0.5f ); }
#endif

/* Ensure we have a function for converting a string to a float. */
#if !defined(HAVE_STRTOF)
inline float strtof( const char *s, char **se ) { return (float) strtod( s, se ); }
#endif

#if !defined(HAVE_LRINTF)
#if defined(_MSC_VER)
inline long lrintf( float f )
{
	int retval;

	_asm fld f;
	_asm fistp retval;

	return retval;
}
#else
#define lrintf(x) ((int)rint(x))
#endif
#endif

#if !defined(HAVE_M_PI)
/* This is only called if the math header files don't include it: stating it here is fine. */
#define M_PI 3.1415926535897932384626433832795
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
