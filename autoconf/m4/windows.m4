AC_DEFUN([SM_WINDOWS],
[
	# Go for the default install path if $WINSDK_PATH is unset
dnl	if test -z $WINSDK_PATH; then
dnl		AC_MSG_WARN(WINSDK_PATH not set. Assuming defaults.)
dnl		WINSDK_PATH="C:/Program Files/Microsoft SDKs/Windows/v7.1"
dnl	fi
	AM_CPPFLAGS="-I$WINSDK_PATH/Include $AM_CPPFLAGS"
	AM_LDFLAGS="-I$WINSDK_PATH/Lib $AM_LDFLAGS"
	AC_CHECK_HEADER(windows.h, , AC_MSG_ERROR(Windows SDK not found.))
dnl _WIN32_WINNT 0x503 = Windows XP SP3
	AC_CHECK_DECL(PBS_MARQUEE, , [AC_MSG_ERROR(w32api headers too old. This is usually caused by trying to use old (official) MinGW instead of MinGW-W64.)], [#include <windows.h>
	#include <CommCtrl.h>])
])