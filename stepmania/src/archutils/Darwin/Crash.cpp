/*
 *  Crash.cpp
 *  stepmania
 *
 *  Created by Steve Checkoway on Thu Jul 31 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

#include "Crash.h"
#include "archutils/Unix/CrashHandler.h"
#include <Carbon/Carbon.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

static void *crashedFramePtr = NULL;

OSStatus HandleException(ExceptionInformation *theException)
{
    crashedFramePtr = (void *)theException->registerImage->R1.lo;
    CrashSignalHandler(theException->theKind);
    return -1;
}


void *GetCrashedFramePtr()
{
    return (crashedFramePtr ? crashedFramePtr : __builtin_return_address(2));
}

void InformUserOfCrash()
{
    CFStringRef error = CFStringCreateWithCString(NULL,
                                                  "StepMania has crashed.  Debug information has been output to\n"
                                                  "\n"
                                                  "    /tmp/crashinfo.txt\n"
                                                  "\n"
                                                  "Please report a bug at:\n"
                                                  "\n"
                                                  "    http://sourceforge.net/tracker/?func=add&group_id=37892&atid=421366",
                                                  kCFStringEncodingASCII);
    CFStringRef OK = CFStringCreateWithCString(NULL, "Open crashinfo", kCFStringEncodingASCII);
    CFStringRef Cancel = CFStringCreateWithCString(NULL, "Cancel", kCFStringEncodingASCII);
    struct AlertStdCFStringAlertParamRec params = {kStdCFStringAlertVersionOne, true, false, OK, Cancel, NULL,
        kAlertStdAlertOKButton, kAlertStdAlertCancelButton, kWindowAlertPositionParentWindowScreen, NULL};
    DialogRef dialog;
    SInt16 button;

    CreateStandardAlert(kAlertStopAlert, error, NULL, &params, &dialog);
    AutoSizeDialog(dialog);
    RunStandardAlert(dialog, NULL, &button);
    switch (button)
    {
        case kAlertStdAlertOKButton:
            if (system(NULL))
                system("open /tmp/crashinfo.txt");
            else
                SysBeep(30);
            break;
    }
    CFRelease(OK);
    CFRelease(Cancel);
    CFRelease(error);
}
