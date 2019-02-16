# Try to find the nasm assembly program.

include(FindPackageHandleStandardArgs)

find_program(NASM_EXECUTABLE nasm
             HINTS $ENV{NASM_ROOT} ${NASM_ROOT}
             PATH_SUFFIXES bin)

find_package_handle_standard_args(nasm DEFAULT_MSG NASM_EXECUTABLE)

mark_as_advanced(NASM_EXECUTABLE)
