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
#include "archutils/Unix/SignalHandler.h"
#include <Carbon/Carbon.h>

void HandleSignal(int sig);
OSStatus HandleException(ExceptionInformation *theException);
void PrintSystemInformation(FILE *fp);

const unsigned maxLogMessages = 10; /* Follow the windows example */

static queue<CString> crashLog;
static vector<CString> staticLog;
static CString additionalLog;
static volatile unsigned handlingException;


ArchHooks_darwin::ArchHooks_darwin() {
    SignalHandler::OnClose(HandleSignal);
    InstallExceptionHandler(HandleException);
}

void ArchHooks_darwin::Log(CString str, bool important) {
    if (important)
        staticLog.push_back(str);
    
    /* Keep the crashLog to maxLogMessages */
    if (crashLog.size() == maxLogMessages)
        crashLog.pop();
    crashLog.push(str);
}

void ArchHooks_darwin::AdditionalLog(CString str) {
    additionalLog = str;
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

void HandleSignal(int sig) {
    FILE *fp = fopen("crashinfo.txt", "w");

    /* ***FIXME*** we need to track build number somehow */
    fprintf(fp,
            "StepMania crash report -- build unknown\n"
            "---------------------------------------\n\n"
            "Crash reason: Signal %d\n\n"
            "All stack trace information in ~/Library/Logs/CrashReporter/\n\n"
            "Static log:\n", sig);
    PrintSystemInformation(fp); //not written yet
    unsigned size = staticLog.size();
    for (unsigned i=0; i<size; ++i)
        fprintf(fp, "%s\n", staticLog[i].c_str());
    fprintf(fp, "\nPartial log:\n");
    while (!crashLog.empty()) {
        CString s = crashLog.front();
        fprintf(fp, "%s\n", s.c_str());
        crashLog.pop();
    }
    fprintf(fp, "\nAdditionalLog:\n%s\n\n-- End of report\n", additionalLog.c_str());
    fclose(fp);
}

#define CASE_CODE(code,addr) case k##code: strcpy((addr),#code); break

OSStatus HandleException(ExceptionInformation *theException) {
    if (handlingException)
        return noErr; /* Ignore it. */
    handlingException = true;
    
    FILE *fp = fopen("crashinfo.txt", "w");
    char code[31];

    switch (theException->theKind) {
        CASE_CODE(UnknownException, code);
        CASE_CODE(IllegalInstructionException, code);
        CASE_CODE(TrapException, code);
        CASE_CODE(AccessException, code);
        CASE_CODE(UnmappedMemoryException, code);
        CASE_CODE(ExcludedMemoryException, code);
        CASE_CODE(ReadOnlyMemoryException, code);
        CASE_CODE(UnresolvablePageFaultException, code);
        CASE_CODE(PrivilegeViolationException, code);
        CASE_CODE(TraceException, code);
        CASE_CODE(InstructionBreakpointException, code);
        CASE_CODE(DataBreakpointException, code);
        CASE_CODE(FloatingPointException, code);
        CASE_CODE(StackOverflowException, code);
    }
 /* ***FIXME*** we need to track build number somehow */
    fprintf(fp,
            "StepMania crash report -- build unknown\n"
            "---------------------------------------\n\n"
            "Crash reason: %s\n\n"
            "All stack trace information in ~/Library/Logs/CrashReporter/\n\n"
            "Static log:\n", code);
    PrintSystemInformation(fp); //not written yet
    unsigned size = staticLog.size();
    for (unsigned i=0; i<size; ++i)
        fprintf(fp, "%s\n", staticLog[i].c_str());
    fprintf(fp, "\nPartial log:\n");
    while (!crashLog.empty()) {
        CString s = crashLog.front();
        fprintf(fp, "%s\n", s.c_str());
        crashLog.pop();
    }
    fprintf(fp, "\nAdditionalLog:\n%s\n\n-- End of report\n", additionalLog.c_str());
    fclose(fp);
    _exit(-1);
    return -1;
}

void PrintSystemInformation(FILE *fp) {
}
    
