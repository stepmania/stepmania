#ifndef ARCH_PLATFORM_H
#define ARCH_PLATFORM_H

#include "arch.h"

/* Load default drivers. */
#include "arch_default.h"

/* Override them with arch-specific drivers, as available. */
#if defined(LINUX)
#include "arch_linux.h"
#elif defined(DARWIN)
#include "arch_darwin.h"
#elif defined(_XBOX)
#include "arch_xbox.h"
#elif defined(_WINDOWS)
#include "arch_Win32.h"
#endif

#endif
