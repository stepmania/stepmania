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
#include "StdString.h"

class Processor
{
private:
    typedef void (*handleFileFunc)(const char *file, const char *archivePath, bool overwrite);
    typedef const char *(*getPathFunc)(const char *id);
    typedef void (*errorFunc)(const char *fmt, ...);

    stack<unsigned> mReturnStack;
    bool mDoGoto;
    CString mLabel;
    map<CString, bool> mConditionals;
    map<CString, CString> mVars;
    map<CString, unsigned> mLabels;
    CString mPath;
    CString mCWD;
    handleFileFunc mHandleFile;
    getPathFunc mGetPath;
    errorFunc mError;
    bool mInstalling;

    const CString& ResolveVar(const CString& var);
    bool ResolveConditional(const CString& cond);
    static void DefaultError(const char *fmt, ...);
public:
    Processor(CString& path, handleFileFunc f, getPathFunc p, bool i)
        : mDoGoto(false), mPath(path), mCWD("."), mHandleFile(f), mGetPath(p),
        mError(Processor::DefaultError), mInstalling(i) { }
    void ProcessLine(const CString& line, unsigned& nextLine);
    void SetErrorFunc(errorFunc f) { mError = f; }
};

#endif
