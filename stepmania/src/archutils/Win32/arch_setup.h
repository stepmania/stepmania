#ifndef ARCH_SETUP_WINDOWS_H
#define ARCH_SETUP_WINDOWS_H

/* Fix Windows breakage. */

#define PATH_MAX _MAX_PATH

#include <direct.h> /* has stuff that should be in unistd.h */

#define getcwd _getcwd
#define wgetcwd _wgetcwd
#define chdir _chdir
#define wchdir _wchdir
#define alloca _alloca
#define stat _stat
#define lstat _stat
/* mkdir is missing the mode arg */
#define mkdir(p,m) mkdir(p)

#endif

