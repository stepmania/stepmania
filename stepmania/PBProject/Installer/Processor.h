/*
 *  Processor.h
 *  stepmania
 *
 *  Created by Steve Checkoway on Sun Sep 07 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

#ifndef PROCESSOR_H
#define PROCESSOR_H

using namespace std;
#include <stack>
#include <map>
#include <set>
#include "StdString.h"

class Processor
{
private:
    typedef void (*handleFileFunc)(const CString& file, const CString& dir, const CString& archivePath,
                                   bool overwrite);
    typedef const CString (*getPathFunc)(const CString& ID);
    typedef void (*errorFunc)(const char *fmt, ...);
    typedef bool (*askFunc)(const CString& question);
    typedef bool (*authFunc)(void);
    typedef void (*echoFunc)(const CString& message, bool loud);
    typedef void (*privFunc)(bool privileged);

    stack<unsigned> mReturnStack;
    bool mDoGoto;
    CString mLabel;
    map<CString, bool> mConditionals;
    map<CString, CString> mVars;
    map<CString, unsigned> mLabels;
    set<CString> mIgnore;
    CString mPath;
    CString mCWD;
    handleFileFunc mHandleFile;
    getPathFunc mGetPath;
    askFunc mAsk;
    errorFunc mError;
    authFunc mAuth;
    echoFunc mEcho;
    privFunc mPriv;
    bool mInstalling;

    const CString& ResolveVar(const CString& var);
    bool ResolveConditional(const CString& cond);
    static void DefaultError(const char *fmt, ...);
public:
    Processor(CString& path, handleFileFunc f, getPathFunc p, askFunc a, bool i);
    ~Processor();
    void ProcessLine(const CString& line, unsigned& nextLine);
    void SetErrorFunc(errorFunc f) { mError = f; }
    void SetAuthFunc(authFunc f) { mAuth = f; }
    void SetEchoFunc(echoFunc f) { mEcho = f; }
    void SetPrivFunc(privFunc f) { mPriv = f; }
};

#endif
