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
    return crashedFramePtr;
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
    CFStringRef OK = CFStringCreateWithCString(NULL, "OK", kCFStringEncodingASCII);
    struct AlertStdCFStringAlertParamRec params = {kStdCFStringAlertVersionOne, true, false, OK, NULL, NULL,
        kAlertStdAlertOKButton, NULL, kWindowAlertPositionParentWindowScreen, NULL};
    DialogRef dialog;
    SInt16 unused;

    CreateStandardAlert(kAlertStopAlert, error, NULL, &params, &dialog);
    AutoSizeDialog(dialog);
    RunStandardAlert(dialog, NULL, &unused);
    CFRelease(OK);
    CFRelease(error);
}
