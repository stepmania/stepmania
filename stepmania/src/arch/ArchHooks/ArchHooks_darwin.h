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
#include <queue>

class ArchHooks_darwin : public ArchHooks {
private:
    queue<CString> crashLog;
    vector<CString> staticLog;
    CString additionalLog;

public:
    ArchHooks_darwin();
    void Log(CString str, bool important);
    void AdditionalLog(CString str) { additionalLog = str; }
    void MessageBoxOK(CString sMessage, CString ID = "");
    MessageBoxResult MessageBoxAbortRetryIgnore(CString sMessage, CString ID = "");
};

#undef ARCH_HOOKS
#define ARCH_HOOKS ArchHooks_darwin
    
#endif /* ARCH_HOOKS_DARWIN_H */