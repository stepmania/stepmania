dnl AX_CHECK_LIB_USING_HEADER(lib, function, header, [action-if-found], [action-if-not-found], [other-libraries])
dnl As AC_CHECK_LIB except "function" must be a fully valid function call and "header" the header file in which that function is defined. The benefit is that this can find functions whose symbols have been decorated, e.g. a lot of MinGW stuff.

AC_DEFUN([AX_CHECK_LIB_USING_HEADER],
[
dnl	# On Windows OpenGL functions' library symbols don't match their function names. So we have to do it the hard way.
	
	AC_MSG_CHECKING(for $2 in $1 using $3)
	
	AC_LANG_PUSH(C)
	
	LIBS_SAVE=$LIBS
	LIBS="$LIBS -l$1 $6"
	AC_LINK_IFELSE(
		[AC_LANG_PROGRAM([#include <$3>], [$2])],
		[AC_MSG_RESULT(yes)
		$4],
		[AC_MSG_RESULT(no)
		$5]
	)
	LIBS=$LIBS_SAVE
	
	AC_LANG_POP(C)
])