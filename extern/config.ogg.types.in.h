#ifndef __CONFIG_TYPES_H__
#define __CONFIG_TYPES_H__

/* CMake powered version of ogg's config types validation. */

#cmakedefine HAVE_INTTYPES_H 1
#cmakedefine HAVE_STDINT_H 1
#cmakedefine HAVE_SYS_TYPES_H 1

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#endif

#if defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#if defined(HAVE_SYS_TYPES_H)
#include <sys/types.h>
#endif

#cmakedefine SIZEOF_INT16_T ${SIZEOF_INT16_T}
#cmakedefine SIZEOF_UINT16_T ${SIZEOF_UINT16_T}
#cmakedefine SIZEOF_U_INT16_T ${SIZEOF_U_INT16_T}
#cmakedefine SIZEOF_INT32_T ${SIZEOF_INT32_T}
#cmakedefine SIZEOF_UINT32_T ${SIZEOF_UINT32_T}
#cmakedefine SIZEOF_U_INT32_T ${SIZEOF_U_INT32_T}
#cmakedefine SIZEOF_INT64_T ${SIZEOF_INT64_T}
#cmakedefine SIZEOF_SHORT ${SIZEOF_SHORT}
#cmakedefine SIZEOF_INT ${SIZEOF_INT}
#cmakedefine SIZEOF_UNSIGNED_SHORT ${SIZEOF_UNSIGNED_SHORT}
#cmakedefine SIZEOF_UNSIGNED_INT ${SIZEOF_UNSIGNED_INT}
#cmakedefine SIZEOF_LONG ${SIZEOF_LONG}
#cmakedefine SIZEOF_UNSIGNED_LONG ${SIZEOF_UNSIGNED_LONG}
#cmakedefine SIZEOF_LONG_LONG ${SIZEOF_LONG_LONG}

/* Ensure we have one 16-bit type. */
#if defined(SIZEOF_INT16_T) && SIZEOF_INT16_T == 2
typedef int16_t ogg_int16_t;
#elif defined(SIZEOF_SHORT) && SIZEOF_SHORT == 2
typedef short ogg_int16_t;
#elif defined(SIZEOF_INT) && SIZEOF_INT == 2
typedef int ogg_int16_t;
#else
#error "No valid 16-bit types found."
#endif

#if defined(SIZEOF_UINT16_T) && SIZEOF_UINT16_T == 2
typedef uint16_t ogg_uint16_t;
#elif defined(SIZEOF_UNSIGNED_SHORT) && SIZEOF_UNSIGNED_SHORT == 2
typedef unsigned short ogg_uint16_t;
#elif defined(SIZEOF_UNSIGNED_INT) && SIZEOF_UNSIGNED_INT == 2
typedef unsigned int ogg_uint16_t;
#elif defined(SIZEOF_U_INT16_T) && SIZEOF_U_INT16_T == 2
typedef u_int16_t ogg_uint16_t;
#else
#error "No valid unsigned 16-bit types found."
#endif

#if defined(SIZEOF_INT32_T) && SIZEOF_INT32_T == 4
typedef int32_t ogg_int32_t;
#elif defined(SIZEOF_SHORT) && SIZEOF_SHORT == 4
typedef short ogg_int32_t;
#elif defined(SIZEOF_INT) && SIZEOF_INT == 4
typedef int ogg_int32_t;
#elif defined(SIZEOF_LONG) && SIZEOF_LONG == 4
typedef long ogg_int32_t;
#else
#error "No valid 32-bit types found."
#endif

#if defined(SIZEOF_UINT32_T) && SIZEOF_UINT32_T == 4
typedef uint32_t ogg_uint32_t;
#elif defined(SIZEOF_UNSIGNED_SHORT) && SIZEOF_UNSIGNED_SHORT == 4
typedef unsigned short ogg_uint32_t;
#elif defined(SIZEOF_UNSIGNED_INT) && SIZEOF_UNSIGNED_INT == 4
typedef unsigned int ogg_uint32_t;
#elif defined(SIZEOF_UNSIGNED_LONG) && SIZEOF_UNSIGNED_LONG == 4
typedef unsigned long ogg_uint32_t;
#elif defined(SIZEOF_U_INT16_T) && SIZEOF_U_INT32_T == 4
typedef u_int32_t ogg_uint32_t;
#else
#error "No valid unsigned 32-bit types found."
#endif

#if defined(SIZEOF_INT64_T) && SIZEOF_INT64_T == 8
typedef int64_t ogg_int64_t;
#elif defined(SIZEOF_INT) && SIZEOF_INT == 8
typedef int ogg_int64_t;
#elif defined(SIZEOF_LONG) && SIZEOF_LONG == 8
typedef long ogg_int64_t;
#elif defined(SIZEOF_LONG_LONG) && SIZEOF_LONG_LONG == 8
typedef long long ogg_int64_t;
#else
#error "No valid 64-bit types found."
#endif

#endif