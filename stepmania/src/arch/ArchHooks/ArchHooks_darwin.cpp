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
#include "RageLog.h"
#include <Carbon/Carbon.h>

const unsigned maxLogMessages = 10; /* Follow the windows example */

ArchHooks_darwin::ArchHooks_darwin() {
    /* Nothing for now. */
}

void ArchHooks_darwin::Log(CString str, bool important) {
    if (important)
        staticLog.push_back(str);
    
    /* Keep the crashLog to maxLogMessages */
    if (crashLog.size() == maxLogMessages)
        crashLog.pop();
    crashLog.push(str);
}

void ArchHooks_darwin::MessageBoxOK(CString sMessage, CString ID) {
    bool allowHush = ID != "";
    
    if (allowHush && MessageIsIgnored(ID))
        return;

    CFStringRef boxName = CFStringCreateWithCString(NULL, "Don't show again", kCFStringEncodingASCII);
    CFStringRef error = CFStringCreateWithCString(NULL, sMessage.c_str(), kCFStringEncodingASCII);
    CFStringRef OK = CFStringCreateWithCString(NULL, "OK", kCFStringEncodingASCII);
    struct AlertStdCFStringAlertParamRec params = {kStdCFStringAlertVersionOne, true, false, OK, NULL, NULL,
        kAlertStdAlertOKButton, NULL, kWindowAlertPositionParentWindowScreen, NULL};
    DialogRef dialog;
    CreateStandardAlert(kAlertNoteAlert, error, NULL, &params, &dialog);
    OSErr err;
    ControlRef box;
    SInt16 unused;
    
    if (allowHush) {
        Rect boxBounds = {20, 40, 300, 25}; /* again, whatever */
        CreateCheckBoxControl(GetDialogWindow(dialog), &boxBounds, boxName, 0, true, &box);
        GetBestControlRect(box, &boxBounds, &unused);
        OSStatus status = InsertDialogItem(dialog, 0, kCheckBoxDialogItem, (Handle)box, &boxBounds);
        ASSERT(status == noErr);
    }
    err = AutoSizeDialog(dialog);
    ASSERT(err == noErr);
    
    RunStandardAlert(dialog, NULL, &unused);
    if (allowHush && GetControl32BitValue(box)){
        LOG->Trace("Ignore dialog ID, \"%s\"", ID.c_str());
        IgnoreMessage(ID);
    }
    CFRelease(error);
    CFRelease(OK);
    CFRelease(boxName);
}

ArchHooks::MessageBoxResult ArchHooks_darwin::MessageBoxAbortRetryIgnore(CString sMessage, CString ID) {
    SInt16 button;
    /* I see no reason for an abort button */
    CFStringRef r = CFStringCreateWithCString(NULL, "Retry", kCFStringEncodingASCII);
    CFStringRef i = CFStringCreateWithCString(NULL, "Ignore", kCFStringEncodingASCII);
    CFStringRef error = CFStringCreateWithCString(NULL, sMessage.c_str(), kCFStringEncodingASCII);
    struct AlertStdCFStringAlertParamRec params = {kStdCFStringAlertVersionOne, true, false, r, i, NULL,
        kAlertStdAlertOKButton, kAlertStdAlertCancelButton, kWindowAlertPositionParentWindowScreen, NULL};
    DialogRef dialog;
    MessageBoxResult result;
    
    CreateStandardAlert(kAlertNoteAlert, error, NULL, &params, &dialog);
    OSErr err = AutoSizeDialog(dialog);
    ASSERT(err == noErr);
    RunStandardAlert(dialog, NULL, &button);
    
    switch (button) {
        case kAlertStdAlertOKButton:
            result =  retry;
            break;
        case kAlertStdAlertCancelButton:
            result =  ignore;
            break;
        default:
            ASSERT(0);
    }
    CFRelease(error);
    CFRelease(i);
    CFRelease(r);
    return result;
}    
    