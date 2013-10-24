AC_DEFUN([SM_BZIP],
[
        AC_REQUIRE([SM_STATIC])
		
dnl		# XXX: AC_CHECK_LIB doesn't work correctly on MinGW for BZip.
dnl		# Hardcoding it because I'm so fed up with this shit.
		if test "$host_os" != "mingw32"; then
			AC_CHECK_LIB(bz2, BZ2_bzCompressInit, have_bzip=yes, have_bzip=no)
		fi
        AC_CHECK_HEADER(bzlib.h, have_bzip_header=yes, have_bzip_header=no)
        AC_ARG_WITH(static-bzip, AS_HELP_STRING([--with-static-bzip],[Statically link bzip]), with_static_bzip=$withval, with_static_bzip=no)

        if test "$with_static_bzip" = "yes"; then
                LIB_PRE=$START_STATIC
                LIB_POST=$END_STATIC
        fi

        if test "$have_bzip_header" = "no"; then
                have_bzip=no
        fi

        if test "$have_bzip" = "no"; then
                echo "*** bzip is required to build StepMania; please make sure"
                echo "*** that libbz2 is installed to continue the installation process."
                exit 0;
        fi

        LIBS="$LIBS $LIB_PRE -lbz2 $LIB_POST"

        LIB_PRE=
        LIB_POST=
])
