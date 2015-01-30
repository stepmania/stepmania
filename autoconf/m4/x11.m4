AC_DEFUN([SM_X11],
[
	AC_PATH_X

	XCFLAGS=
	XLIBS=

	if test "$no_x" != "yes"; then
	if test -n "$x_includes"; then
		XCFLAGS="-I$x_includes"
	fi

	if test -n "$x_libraries"; then
		XLIBS="-L$x_libraries"
	fi

	XLIBS+="-lX11"

	if test -n "$x_includes"; then
		# See if we can compile X applications without using $XCFLAGS.
		AC_MSG_CHECKING(if $XCFLAGS is really necessary)
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <X11/Xos.h>]], [[]])],[XCFLAGS=
		AC_MSG_RESULT(no)],[AC_MSG_RESULT(yes)])
	fi

	# Check for libXtst.
	AC_CHECK_LIB(Xtst, XTestQueryExtension, 
		XLIBS="$XLIBS -lXtst"
		[AC_DEFINE(HAVE_LIBXTST, 1, [libXtst available])],
		,
		[$XLIBS])
	AC_DEFINE(HAVE_X11, 1, [X11 libraries present])
	fi

	# Check for Xxf86vm
	AC_CHECK_LIB(Xxf86vm, XF86VidModeSwitchToMode,
		have_xf86vm=yes,
		have_xf86vm=no,
		[$XLIBS])
	AC_CHECK_HEADER(X11/extensions/xf86vmode.h, have_xf86vm_header=yes, have_xf86vm_header=no, [#include <X11/Xlib.h>])

	if test "$have_xf86vm_header" = "no"; then
		have_xf86vm=no
	fi

	if test "$have_xf86vm" = "no"; then
		if test "$unix" = "yes"; then
			AC_MSG_ERROR("Couldn't find X11 libraries.")
		else
			no_x=yes
		fi
	else
		XLIBS="$XLIBS -lXxf86vm"
	fi

	AM_CONDITIONAL(HAVE_X11, test "$no_x" != "yes")

	AC_SUBST(XCFLAGS)
	AC_SUBST(XLIBS)
])
