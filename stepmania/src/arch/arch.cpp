/*
 * This file provides functions to create driver objects.
 */

#include "../stdafx.h"

#include "arch.h"

/* Load default drivers. */
#include "arch_default.h"

/* Override them with arch-specific drivers, as available. */
#if defined(WIN32)
#include "arch_Win32.h"
#endif

LoadingWindow *MakeLoadingWindow() { return new ARCH_LOADING_WINDOW; }

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
