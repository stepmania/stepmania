AC_DEFUN(SM_LUA, [

AC_CHECK_PROGS(LUA_CONFIG, [lua-config50 lua-config], "")
if test "$LUA_CONFIG" != ""; then
	LUA_CFLAGS=`$LUA_CONFIG --include`
	LUA_LIBS=`$LUA_CONFIG --static`

	old_LIBS=$LIBS
	LIBS="$LIBS $LUA_LIBS"
	
	AC_CHECK_FUNC(lua_open, , LUA_MISSING=yes)

	# If lua-config exists, we should at least have Lua; if it fails to build,
	# something other than it not being installed is wrong.
	if test "$LUA_MISSING" = "yes"; then
		echo
		echo "*** $LUA_CONFIG was found, but a Lua test program failed to build."
		echo "*** Please check your installation."
		exit 1;
	fi
	AC_CHECK_FUNC(luaopen_base, , LUA_LIB_MISSING=yes)

	LIBS="$old_LIBS"
else
	if test "$LIB_LUA" = ""; then
		AC_CHECK_LIB(lua, lua_open, LIB_LUA=-llua)
	fi
	if test "$LIB_LUA" = ""; then
		AC_CHECK_LIB(lua50, lua_open, LIB_LUA=-llua50)
	fi
	if test "$LIB_LUA_LIB" = ""; then
		AC_CHECK_LIB(lualib, luaopen_base, LIB_LUA_LIB=-llualib, , [$LIB_LUA -ldl])
	fi
	if test "$LIB_LUA_LIB" = ""; then
		AC_CHECK_LIB(lualib50, luaopen_base, LIB_LUA_LIB=-llualib50, , [$LIB_LUA -ldl])
	fi
	if test "$LIB_LUA" = ""; then
		LUA_MISSING=yes
	fi
	if test "$LIB_LUA_LIB" = ""; then
		LUA_LIB_MISSING=yes
	fi
	LUA_CFLAGS=
	LUA_LIBS="$LIB_LUA $LIB_LUA_LIB"
fi
if test "$LUA_MISSING" = "yes"; then
	echo
	echo "*** liblua is required to build StepMania; please make sure that"
	echo "*** it is installed to continue the installation process."
	exit 1;
fi
if test "$LUA_LIB_MISSING" = "yes"; then
	echo
	echo "*** liblualib is required to build StepMania; please make sure that"
	echo "*** it is installed to continue the installation process."
	exit 1;
fi

AC_SUBST(LUA_CFLAGS)
AC_SUBST(LUA_LIBS)

])
