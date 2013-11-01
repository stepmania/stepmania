/* Since VS generates WindowsResources.rc we can't edit it directly. */
#if defined(HAVE_WINRES_H)
#include <winres.h> // Just in case
#else
#include <afxres.h>
#endif
