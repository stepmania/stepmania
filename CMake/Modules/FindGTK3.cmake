# Use pkg-config to find installed gtk+3 if available
#
# Once found, the following are defined:
#  GTK3_FOUND
#  GTK3_INCLUDE_DIRS
#  GTK3_LIBRARIES

include(FindPkgConfig)
pkg_check_modules(GTK3 gtk+-3.0)
