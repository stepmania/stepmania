/*
 *  Processor.cpp
 *  stepmania
 *
 *  Created by Steve Checkoway on Sun Sep 07 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

using namespace std;

#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include "StdString.h"
#include "Processor.h"

void split(const CString& Source, const CString& Deliminator, vector<CString>& AddIt)
{
    unsigned startpos = 0;

    do {
        unsigned pos = Source.find(Deliminator, startpos);
        if (pos == Source.npos) pos=Source.size();

        CString AddCString = Source.substr(startpos, pos-startpos);
        AddIt.push_back(AddCString);

        startpos=pos+Deliminator.size();
    } while (startpos <= Source.size());
}

bool IsAVar(const CString& var)
{
    return var[0] == '$';
}

CString StripVar(const CString& var)
{
    if (!IsAVar(var))
        return var;
    return var.substr(1, var.npos);
}

void Processor::ProcessLine(const CString& line, unsigned& nextLine)
{
    CStringArray parts;

    split(line, ":", parts);

    if (!mInstalling)
    {
        if (parts[0] == "FILE")
        {
            if (parts.size() != 3)
                goto error;
            (*mHandleFile)(parts[1], mPath, true);
        }
        ++nextLine;
        return;
    }

    if (parts[0] == "LABEL")
    {
        if (parts.size() != 2)
            goto error;
        if (mDoGoto && parts[1] == mLabel)
        {
            mDoGoto = false;
            mLabel = "";
        }
        mLabels[parts[1]] = ++nextLine; // set mLabel to be the next line
        return;
    }
    
    if (mDoGoto)
    {
        ++nextLine;
        return;
    }

    if (parts[0] == "GOTO")
    {
        if (parts.size() != 2)
            goto error;
        if (parts[1] == "")
            goto error;
        mLabel = parts[1];
        if (mLabels.find(mLabel) != mLabels.end())
        {
            mReturnStack.push(nextLine + 1);
            nextLine = mLabels[mLabel];
            mLabel = "";
            return;
        }
        mDoGoto = true;
        mReturnStack.push(++nextLine);
        return;
    }

    if (parts[0] == "IF")
    {
        if (parts.size() != 4)
            goto error;
        if (ResolveConditional(parts[1]))
            mLabel = parts[2];
        else
            mLabel = parts[3];
        if (mLabel == "")
        {
            ++nextLine;
            return;
        }
        mReturnStack.push(nextLine + 1);
        if (mLabels.find(mLabel) != mLabels.end())
        {
            nextLine = mLabels[mLabel];
            mLabel = "";
            return;
        }
        mDoGoto = true;
        ++nextLine;
        return;
    }

    if (parts[0] == "FILE")
    {
        if (parts.size() != 3)
            goto error;
        bool overwrite = ResolveConditional(parts[2]);
        (*mHandleFile)(parts[1], mPath, overwrite);

        ++nextLine;
        return;
    }

    if (parts[0] == "RETURN")
    {
        if (mReturnStack.empty())
            nextLine = unsigned(-1); // quite large
        else
        {
            nextLine = mReturnStack.top();
            mReturnStack.pop();
        }
        return;
    }

    if (parts[0] == "GETPATH")
    {
        if (parts.size() != 3)
            goto error;
        mVars[parts[2]] = (*mGetPath)(parts[1]);
        ++nextLine;
        return;
    }

    if (parts[0] == "SETVAR")
    {
        if (parts.size() < 3)
            goto error;
        CString temp = "";
        for (unsigned i=2; i<parts.size(); ++i)
            temp += parts[i];
        mVars[parts[1]] = temp;
        ++nextLine;
        return;
    }

    if (parts[0] == "MKDIR")
    {
        if (parts.size() != 2)
            goto error;
        CString path = mCWD + ResolveVar(parts[1]);
        if (mkdir(path, 777))
            (*mError)("%s\n", strerror(errno));
        ++nextLine;
        return;
    }

    if (parts[0] == "CD")
    {
        if (parts.size() != 2)
            goto error;
        CString path = ResolveVar(parts[1]);
        if (path[0] == '/')
            mCWD = path;
        else
            mCWD += "/" + path;
        struct stat st;
        if (stat(mCWD, &st))
            (*mError)("%s\n", strerror(errno));
        if (!(st.st_mode & S_IFDIR))
            (*mError)("%s is not a directory.\n", mCWD.c_str());
        ++nextLine;
        return;
    }

    if (parts[0] == "RM")
    {
        if (parts.size() != 2)
            goto error;
        CString path = ResolveVar(parts[1]);
        if (path[0] != '/')
            path = mCWD + "/" + path;
        CString command = "rm -rf '" + path + "'";
        system(command); // lazy
        ++nextLine;
        return;
    }

error:
    (*mError)("%s is an invalid command", line.c_str());
    ++nextLine;
}

const CString& Processor::ResolveVar(const CString& var)
{
    if (!IsAVar(var))
        return var;
    return mVars[StripVar(var)];
}

bool Processor::ResolveConditional(const CString& cond)
{
    if (!IsAVar(cond))
        return cond != "";
    return mConditionals[StripVar(cond)];
}

void Processor::DefaultError(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    exit(-1);
}
