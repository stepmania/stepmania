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
#include "archutils/Darwin/Crash.h"
#include "archutils/Unix/CrashHandler.h"
#include "archutils/Unix/SignalHandler.h"
#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>

/* You would think that these would be defined somewhere. */
enum {
    kMacOSX_10_2 = 0x1020,
    kMacOSX_10_3 = 0x1030
};

OSStatus HandleException(ExceptionInformation *theException);

ArchHooks_darwin::ArchHooks_darwin()
{
    CrashHandlerHandleArgs();
    
    long response;
    CString error = "";
    OSErr err = Gestalt(gestaltSystemVersion, &response);
    
    if (err != noErr)
        error = "Gestalt call failed.";
    else if (response < kMacOSX_10_2)
        error = "StepMania is not supported on any version of Mac OS X "
            "other than OS X 10.2.x. This is not a bug.";
    if (error != "")
	{
        MessageBoxOK(error, "");
        exit(0);
    }
    InstallExceptionHandler(HandleException);
    err = EnterMovies();
    ASSERT(err == noErr);
}

#define CASE_GESTALT_M(str,code,result) case gestalt##code: str = result; break
#define CASE_GESTALT(str,code) CASE_GESTALT_M(str, code, #code)

void ArchHooks_darwin::DumpDebugInfo()
{
    CString systemVersion;
    long ram;
    long vRam;
    long processorSpeed;
    CString processor;
    long numProcessors;
    CString machine;
    CString primaryDisplayDriver = "unknown";
    CString dProvider = "";
    CString dDescription = "";
    CString dVersion = "";
    CString dDate = "";
    CString dDeviceID = "";
    char *temp;

    OSErr err = noErr;
    long code;

    /* Get system version */
    err = Gestalt(gestaltSystemVersion, &code);
    if (err == noErr) {
        systemVersion = "Mac OS X ";
        if (code >= kMacOSX_10_2 && code < kMacOSX_10_3) {
            systemVersion += "10.2.";
            asprintf(&temp, "%d", code - kMacOSX_10_2);
            systemVersion += temp;
            free(temp);
        } else {
            systemVersion += "10.3";
            int ssv = code - kMacOSX_10_3;
            if (ssv > 9)
                systemVersion += "+";
            else {
                asprintf(&temp, ".%d", ssv);
                systemVersion += temp;
                free(temp);
            }
        }
    } else
        systemVersion = "Unknown system version";
            
    /* Get memory */
    err = Gestalt(gestaltLogicalRAMSize, &vRam);
    if (err != noErr)
        vRam = 0;
    err = Gestalt(gestaltPhysicalRAMSize, &ram);
    if (err == noErr) {
        vRam -= ram;
        if (vRam < 0)
            vRam = 0;
        ram /= 1048576; /* 1048576 = 1024*1024 */
        vRam /= 1048576;
    } else {
        ram = 0;
        vRam = 0;
    }
        
    /* Get processor */
    numProcessors = MPProcessorsScheduled();
    err = Gestalt(gestaltNativeCPUtype, &code);
    if (err == noErr) {
        switch (code) {
            CASE_GESTALT_M(processor, CPU601, "601");
            CASE_GESTALT_M(processor, CPU603, "603");
            CASE_GESTALT_M(processor, CPU603e, "603e");
            CASE_GESTALT_M(processor, CPU603ev, "603ev");
            CASE_GESTALT_M(processor, CPU604, "604");
            CASE_GESTALT_M(processor, CPU604e, "604e");
            CASE_GESTALT_M(processor, CPU604ev, "604ev");
            CASE_GESTALT_M(processor, CPU750, "G3");
            CASE_GESTALT_M(processor, CPUG4, "G4");
            CASE_GESTALT_M(processor, CPUG47450, "G4");
            CASE_GESTALT_M(processor, CPUApollo, "G4 (Apollo)");
            CASE_GESTALT_M(processor, CPU750FX, "G3 (Sahara)");
            default:
                asprintf(&temp, "%d", code);
                processor = temp;
                free(temp);
        }
    } else
        processor = "unknown";
    err = Gestalt(gestaltProcClkSpeed, &processorSpeed);
    if (err != noErr)
        processorSpeed = 0;
    /* Get machine */
    err = Gestalt(gestaltMachineType, &code);
    if (err == noErr) {
        switch (code) {
            /* PowerMacs */
            CASE_GESTALT(machine, PowerMac4400);
            CASE_GESTALT(machine, PowerMac4400_160);
            CASE_GESTALT(machine, PowerMac5200);
            CASE_GESTALT(machine, PowerMac5400);
            CASE_GESTALT(machine, PowerMac5500);
            CASE_GESTALT(machine, PowerMac6100_60);
            CASE_GESTALT(machine, PowerMac6100_66);
            CASE_GESTALT(machine, PowerMac6200);
            CASE_GESTALT(machine, PowerMac6400);
            CASE_GESTALT(machine, PowerMac6500);
            CASE_GESTALT(machine, PowerMac7100_66);
            CASE_GESTALT(machine, PowerMac7100_80);
            CASE_GESTALT(machine, PowerMac7200);
            CASE_GESTALT(machine, PowerMac7300);
            CASE_GESTALT(machine, PowerMac7500);
            CASE_GESTALT(machine, PowerMac8100_80);
            CASE_GESTALT(machine, PowerMac8100_100);
            CASE_GESTALT(machine, PowerMac8100_110);
            CASE_GESTALT(machine, PowerMac8500);
            CASE_GESTALT(machine, PowerMac9500);
            /* upgrade cards */
            CASE_GESTALT(machine, PowerMacLC475);
            CASE_GESTALT(machine, PowerMacLC575);
            CASE_GESTALT(machine, PowerMacQuadra610);
            CASE_GESTALT(machine, PowerMacQuadra630);
            CASE_GESTALT(machine, PowerMacQuadra650);
            CASE_GESTALT(machine, PowerMacQuadra700);
            CASE_GESTALT(machine, PowerMacQuadra800);
            CASE_GESTALT(machine, PowerMacQuadra900);
            CASE_GESTALT(machine, PowerMacQuadra950);
            CASE_GESTALT(machine, PowerMacCentris610);
            CASE_GESTALT(machine, PowerMacCentris650);
            /* PowerBooks */
            CASE_GESTALT(machine, PowerBook1400);
            CASE_GESTALT(machine, PowerBook2400);
            CASE_GESTALT(machine, PowerBook3400);
            CASE_GESTALT(machine, PowerBook500PPCUpgrade);
            CASE_GESTALT(machine, PowerBookG3);
            CASE_GESTALT(machine, PowerBookG3Series);
            CASE_GESTALT(machine, PowerBookG3Series2);
            /* NewWorld */
            CASE_GESTALT(machine, PowerMacNewWorld);
            CASE_GESTALT(machine, PowerMacG3);
            default:
                asprintf(&temp, "%d", code);
                machine = temp;
                free(temp);
        }
    } else if (err == gestaltUndefSelectorErr ) {
        machine = "PowerMac";
        machine += processor;
    } else
        machine = "unknown machine";
    
    /* Get primary display driver */
    /* TODO */

    /* Send all of the information to the log */
    LOG->Info(machine.c_str());
    LOG->Info("Processor: %s (%d)", processor.c_str(), numProcessors);
    LOG->Info("%s", systemVersion.c_str());
    LOG->Info("Memory: %d MB total, %d MB swap", ram, vRam);
    LOG->Info("Primary display driver: %s\n"
              "Video Driver Information:\n"
              "Provider       :\t%s\n"
              "Description    :\t%s\n"
              "Version        :\t%s\n"
              "Date           :\t%s\n"
              "DeviceID       :\t%s\n", primaryDisplayDriver.c_str(), dProvider.c_str(),
              dDescription.c_str(), dVersion.c_str(), dDate.c_str(), dDeviceID.c_str());
}

void ArchHooks_darwin::MessageBoxOK(CString sMessage, CString ID)
{
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
    if (allowHush && GetControl32BitValue(box))
	{
        LOG->Trace("Ignore dialog ID, \"%s\"", ID.c_str());
        IgnoreMessage(ID);
    }
    CFRelease(error);
    CFRelease(OK);
    CFRelease(boxName);
}

ArchHooks::MessageBoxResult ArchHooks_darwin::MessageBoxAbortRetryIgnore(CString sMessage, CString ID)
{
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

inline void ArchHooks_darwin::Update(float delta) {
#pragma unused(delta)
    MoviesTask(NULL, 0);
    OSErr err = GetMoviesError();
    if (__builtin_expect(err, noErr))
        RageException::Throw("MoviesTask failed with error: %d", err);
}
