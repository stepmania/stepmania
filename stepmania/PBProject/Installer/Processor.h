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
    typedef void (*handleFileFunc)(const CString& file, const CString& dir, const CString& archivePath,
                                   bool overwrite);
    typedef const CString (*getPathFunc)(const CString& ID);
    typedef void (*errorFunc)(const char *fmt, ...);
    typedef bool (*askFunc)(const CString& question);

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
    askFunc mAsk;
    errorFunc mError;
    bool mInstalling;

    const CString& ResolveVar(const CString& var);
    bool ResolveConditional(const CString& cond);
    static void DefaultError(const char *fmt, ...);
public:
    Processor(CString& path, handleFileFunc f, getPathFunc p, askFunc a, bool i)
        : mDoGoto(false), mPath(path), mCWD("."), mHandleFile(f), mGetPath(p), mAsk(a),
        mError(Processor::DefaultError), mInstalling(i) { mConditionals["INSTALLING"] = i; }
    void ProcessLine(const CString& line, unsigned& nextLine);
    void SetErrorFunc(errorFunc f) { mError = f; }
};

#endif
