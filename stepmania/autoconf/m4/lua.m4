AC_DEFUN(SM_LUA, [

AC_CHECK_PROGS(LUA_CONFIG, [lua-config50 lua-config], "")
if test "$LUA_CONFIG" != ""; then
	LUA_CFLAGS=`lua-config --include`
	LUA_LIBS=`lua-config --static`
else
	LUA_CFLAGS=
	LUA_LIBS=
	if test "$LIB_LUA" = ""; then
		AC_CHECK_LIB(lualib, lua_open, LIB_LUA=-llua)
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
	if test "$LIB_LUA" = "" -o "$LIB_LUA_LIB" = ""; then
		echo
		echo "*** liblua is required to build StepMania; please"
		echo "*** make sure that liblua is installed to continue"
		echo "*** the installation process."
		exit 0;
	fi
fi
AC_SUBST(LUA_CFLAGS)
AC_SUBST(LUA_LIBS)

])
