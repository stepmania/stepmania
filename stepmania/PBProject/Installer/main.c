/*
 *  main.c
 *  stepmania
 *
 *  Created by Steve Checkoway on Mon Sep 08 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

#include <Carbon/Carbon.h>

enum
{
    kMacOSX_10_2 = 0x1020
};

extern int NSApplicationMain(int argc, const char *argv[]);

int main(int argc, const char *argv[])
{
    long response;
    OSErr err = Gestalt(gestaltSystemVersion, &response);
    CFStringRef error = NULL;
    
    if (err != noErr)
        error = CFStringCreateWithCString(NULL, "Couldn't determine OS version.", kCFStringEncodingASCII);
    else if (response < kMacOSX_10_2)
        error = CFStringCreateWithCString(NULL, "StepMania will not run on any version of Mac OS X 10.1.x "
                                          "or earlier.", kCFStringEncodingASCII);
    if (error)
    {
        CFStringRef OK = CFSTR("OK");
        struct AlertStdCFStringAlertParamRec params = {kStdCFStringAlertVersionOne, TRUE, FALSE, OK, NULL, NULL,
            kAlertStdAlertOKButton, NULL, kWindowAlertPositionParentWindowScreen, NULL};
        DialogRef dialog;
        SInt16 result;

        CreateStandardAlert(kAlertStopAlert, error, NULL, &params, &dialog);
        AutoSizeDialog(dialog);
        RunStandardAlert(dialog, NULL, &result);
        CFRelease(error);
        
        exit(0);
    }
    
    return NSApplicationMain(argc, argv);
}
