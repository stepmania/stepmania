/*
 *  ArchHooks_darwin.cpp
 *  stepmania
 *
 *  Created by Steve Checkoway on Tue Jul 15 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

#include "global.h"
#include "PrefsManager.h"
#include "ArchHooks_darwin.h"

ArchHooks_darwin::ArchHooks_darwin() {

}

void ArchHooks_darwin::Log(CString str, bool important) {

}

void ArchHooks_darwin::MessageBoxOK(CString sMessage, CString ID) {

}

ArchHooks::MessageBoxResult ArchHooks_darwin::MessageBoxAbortRetryIgnore(CString sMessage, CString ID) {
    return ignore;
}