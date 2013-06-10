AC_DEFUN([SM_ZLIB],
[
	AC_REQUIRE([SM_STATIC])
	AC_CHECK_LIB(z, inflate, have_libz=yes, have_libz=no)
	AC_CHECK_HEADER(zlib.h, have_libz_header=yes, have_libz_header=no)
	AC_ARG_WITH(static-zlib, AS_HELP_STRING([--with-static-zlib],[Statically link zlib]), with_static_zlib=$withval, with_static_zlib=no)

	if test "$with_static_zlib" = "yes"; then
		LIB_PRE=$START_STATIC
		LIB_POST=$END_STATIC
	fi

	if test "$have_libz_header" = "no"; then
		have_libz=no
	fi

	if test "$have_libz" = "no"; then
		echo "*** zlib is required to build StepMania; please make sure"
		echo "*** that zlib is installed to continue the installation process."
		exit 0;
	fi

	LIBS="$LIBS $LIB_PRE -lz $LIB_POST"

	LIB_PRE=
	LIB_POST=
])

