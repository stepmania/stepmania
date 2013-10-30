/* Since VS generates WindowsResources.rc we can't edit it directly. */
#if defined(_MSC_VER)
#include <winres.h> // Just in case
#else
#include <afxres.h>
#endif