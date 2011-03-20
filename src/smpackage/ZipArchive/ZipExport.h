#if !defined (ZIP_EXPORT_H)
#define ZIP_API_H

#if defined (ZIP_HAS_DLL)
#  if (ZIP_HAS_DLL == 1)
#    if defined (ZIP_BUILD_DLL)
#      define ZIP_API __declspec (dllexport)
#    else
#      define ZIP_API  __declspec (dllimport)
#    endif /* ZIP_BUILD_DLL */
#  else
#    define ZIP_API
#  endif   /* ! ZIP_HAS_DLL == 1 */
#else
#  define ZIP_API
#endif     /* ZIP_HAS_DLL */

#endif     /* ZIP_EXPORT_H */
