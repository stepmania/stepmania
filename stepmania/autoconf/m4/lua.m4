AC_DEFUN(SM_LUA, [

AC_CHECK_PROGS(LUA_CONFIG, [lua-config50 lua-config], "")
if test "$LUA_CONFIG" != ""; then
	LUA_CFLAGS=`$LUA_CONFIG --include`
	LUA_LIBS=`$LUA_CONFIG --static`
else
	if test "$LIB_LUA" = ""; then
		AC_CHECK_LIB(lua, lua_open, LIB_LUA=-llua)
	fi
	if test "$LIB_LUA" = ""; then
		AC_CHECK_LIB(lua50, lua_open, LIB_LUA=-llua50)
	fi
	if test "$LIB_LUA_LIB" = ""; then
		AC_CHECK_LIB(lualib, luaopen_base, LIB_LUA_LIB=-llualib, , [$LIB_LUA])
	fi
	if test "$LIB_LUA_LIB" = ""; then
		AC_CHECK_LIB(lualib50, luaopen_base, LIB_LUA_LIB=-llualib50, , [$LIB_LUA])
	fi
	if test "$LIB_LUA" = ""; then
		echo
		echo "*** liblua is required to build StepMania; please make sure that"
		echo "*** it is installed to continue the installation process."
		exit 0;
	fi
	if test "$LIB_LUA_LIB" = ""; then
		echo
		echo "*** liblualib is required to build StepMania; please make sure that"
		echo "*** it is installed to continue the installation process."
		exit 0;
	fi
	LUA_CFLAGS=
	LUA_LIBS="$LIB_LUA $LIB_LUA_LIB"
fi
AC_SUBST(LUA_CFLAGS)
AC_SUBST(LUA_LIBS)

])
