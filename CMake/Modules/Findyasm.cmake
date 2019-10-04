# Try to find the yasm assembly program.

include(FindPackageHandleStandardArgs)

find_program(YASM_EXECUTABLE yasm
             HINTS $ENV{YASM_ROOT} ${YASM_ROOT}
             PATH_SUFFIXES bin)

find_package_handle_standard_args(yasm DEFAULT_MSG YASM_EXECUTABLE)

mark_as_advanced(YASM_EXECUTABLE)
