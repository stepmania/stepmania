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
    typedef void (*handleFileFunc)(const CString& file, const CString& dir,
								   const CString& archivePath);
    typedef const CString (*getPathFunc)(const CString& ID);
    typedef void (*errorFunc)(const char *fmt, ...);
    typedef bool (*askFunc)(const CString& question);
    typedef void (*echoFunc)(const CString& message, bool loud);

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
	echoFunc mEcho;
    bool mInstalling;

    const CString& ResolveVar(const CString& var);
    bool ResolveConditional(const CString& cond);
    static void DefaultError(const char *fmt, ...);
public:
    Processor(CString& path, handleFileFunc f, getPathFunc p, askFunc a, bool i);
    ~Processor();
    void ProcessLine(const CString& line, unsigned& nextLine);
    void SetErrorFunc(errorFunc f) { mError = f; }
    void SetEchoFunc(echoFunc f) { mEcho = f; }
};

#endif

/*
 * (c) 2003 Steve Checkoway
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
