AC_DEFUN([SM_ZLIB],
[
	AC_CHECK_LIB(z, inflate, have_libz=yes, have_libz=no)
	AC_CHECK_HEADER(zlib.h, have_libz_header=yes, have_libz_header=no)

	if test "$have_libz_header" = "no"; then
		have_libz=no
	fi

	if test "$have_libz" = "no"; then
		echo "*** zlib is required to build StepMania; please make sure"
		echo "*** that zlib is installed to continue the installation process."
		exit 0;
	fi

	LIBS="$LIBS -lz"
])

