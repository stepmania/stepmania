/*
 *  ErrorDialog_darwin.cpp
 *  stepmania
 *
 *  Created by Steve Checkoway on Wed Jul 23 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

#include "global.h"
#include "ErrorDialog_darwin.h"
#include <Carbon/Carbon.h>

void ErrorDialog_darwin::ShowError() {
    CFStringRef error = CFStringCreateWithCString(NULL, GetErrorText().c_str(), kCFStringEncodingASCII);
    CFStringRef OK = CFStringCreateWithCString(NULL, "OK", kCFStringEncodingASCII);
    struct AlertStdCFStringAlertParamRec params = {kStdCFStringAlertVersionOne, true, false, OK, NULL, NULL,
        kAlertStdAlertOKButton, NULL, kWindowAlertPositionParentWindowScreen, NULL};
    DialogRef dialog;
    SInt16 unused;
    OSErr err;
    
    CreateStandardAlert(kAlertStopAlert, error, NULL, &params, &dialog);
    err = AutoSizeDialog(dialog);
    ASSERT(err == noErr);
    RunStandardAlert(dialog, NULL, &unused);
    CFRelease(error);
    CFRelease(OK);
}
