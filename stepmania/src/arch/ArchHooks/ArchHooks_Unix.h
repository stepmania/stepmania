#ifndef ARCH_HOOKS_UNIX_H
#define ARCH_HOOKS_UNIX_H

#include "ArchHooks.h"
class ArchHooks_Unix: public ArchHooks
{
public:
    ArchHooks_Unix();
};

#undef ARCH_HOOKS
#define ARCH_HOOKS ArchHooks_Unix

#endif
/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
