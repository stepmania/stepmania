#ifndef ARCH_HOOKS_NONE_H
#define ARCH_HOOKS_NONE_H

#include "ArchHooks.h"
class ArchHooks_none: public ArchHooks
{
};

#undef ARCH_HOOKS
#define ARCH_HOOKS ArchHooks_none

#endif
/*
 * Copyright (c) 2002-2003 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
