#ifndef ARCH_HOOKS_DARWIN_H
#define ARCH_HOOKS_DARWIN_H
/*
 *  ArchHooks_darwin.h
 *  stepmania
 *
 *  Created by Steve Checkoway on Tue Jul 15 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

#include "ArchHooks.h"
#include <SDL.h>
#include <SDL_thread.h>
#include <queue>

class ArchHooks_darwin : public ArchHooks
{
public:
    ArchHooks_darwin();
    ~ArchHooks_darwin() { }
    void DumpDebugInfo();
protected:
    void MessageBoxErrorPrivate(CString sError, CString ID);
	void MessageBoxOKPrivate(CString sMessage, CString ID);
    MessageBoxResult MessageBoxAbortRetryIgnorePrivate(CString sMessage, CString ID);
};

#undef ARCH_HOOKS
#define ARCH_HOOKS ArchHooks_darwin
    
#endif /* ARCH_HOOKS_DARWIN_H */
