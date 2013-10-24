AC_DEFUN([SM_OPENGL_TRY_LIB],
[
dnl	# On Windows OpenGL functions' library symbols don't match their function names. So we have to do it the hard way.
	
	AC_MSG_CHECKING(for glBegin in $1)
	
	AC_LANG_PUSH(C)
	
	LIBS_SAVE=$LIBS
	LIBS="$LIBS $1"
	AC_LINK_IFELSE(
		[AC_LANG_PROGRAM([#include <GL/gl.h>], [glBegin(GL_POINTS)])],
		[AC_MSG_RESULT(yes)
		$2],
		AC_MSG_RESULT(no)
	)
	LIBS=$LIBS_SAVE
	
	AC_LANG_POP(C)
])

AC_DEFUN([SM_OPENGL],
[
	AC_LANG_PUSH(C)

	AC_CHECK_HEADER("GL/gl.h", ,
	AC_MSG_ERROR("OpenGL headers not found."), [$GL_INCLUDES])
	
	GL_LIBS=""
	
	SM_OPENGL_TRY_LIB(-lGL, [GL_LIBS="-lGL"])
	
	if test x$GL_LIBS = "x"; then
	SM_OPENGL_TRY_LIB(-lopengl32, [GL_LIBS="-lopengl32"])
	fi
	if test x$GL_LIBS = "x"; then
			AC_MSG_ERROR("No usable OpenGL library found.")
	fi
])