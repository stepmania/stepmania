AC_DEFUN([SM_WINDOWS],
[
dnl We are a GUI program...
	LDFLAGS="$LDFLAGS -mwindows"
	
dnl Windows-specific libs. It'd be difficult and of little value to test for these.
	LIBS="$LIBS -lole32 -lDsound -lwinmm -ldxguid -lDinput8 -lHid -lDbghelp -lWs2_32 -lVersion -lSetupapi"

	AM_CPPFLAGS="-I$WINSDK_PATH/Include $AM_CPPFLAGS"
	AM_LDFLAGS="-I$WINSDK_PATH/Lib $AM_LDFLAGS"
	AC_CHECK_HEADER(windows.h, , AC_MSG_ERROR(Windows SDK not found.))
	AC_CHECK_DECL(PBS_MARQUEE, , [AC_MSG_ERROR(w32api headers too old. This is usually caused by trying to use old (official) MinGW instead of MinGW-W64.)], [#include <windows.h>
	#include <CommCtrl.h>])
	
	AC_CHECK_DECL(DXGetErrorString,
		,
		use_dxerr9=yes,
		[#include <windows.h>
		#include <dxerr.h>])
	
	if test "$use_dxerr9" = "yes"; then
		LIBS="$LIBS -ldxerr9"
		AC_DEFINE([USE_DXERR9], [], [Use old dxerr9.])
		AC_CHECK_DECL(DXGetErrorString9, 
			,
			AC_MSG_ERROR("Couldn't find DXGetErrorString in dxerr.h or DXGetErrorString9 in dxerr9.h."),
			[#include <windows.h>
			#include <dxerr9.h>])
	else
		LIBS="$LIBS -ldxerr"
	fi
])