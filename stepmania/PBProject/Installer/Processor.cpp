/*
 *  Processor.cpp
 *  stepmania
 *
 *  Created by Steve Checkoway on Sun Sep 07 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

using namespace std;

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include "StdString.h"
#include "Processor.h"
#include "Util.h"

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

Processor::Processor(CString& path, handleFileFunc f, getPathFunc p, askFunc a, bool i)
: mDoGoto(false), mHandleFile(f), mGetPath(p), mAsk(a),
mError(Processor::DefaultError), mInstalling(i)
{
    char cwd[MAXPATHLEN];
    
    getwd(cwd);
    mCWD = cwd;
    mConditionals["INSTALLING"] = i;

    if (mInstalling)
    {
        char archivePath[] = "/tmp/installerXXXXXX.tar";
        int fd = mkstemps(archivePath, 4);

        if (fd == -1)
            throw CString("Couldn't create temporary file");

        mPath = archivePath;

        char toolPath[] = "/usr/bin/gunzip";

        if(CallTool2(true, -1, fd, -1, toolPath, "-c", path.c_str(), NULL))
        {
            unlink(mPath);
            throw CString("Archive file not handled correctly or not present.");
        }
    }
    else
        mPath = path;
}

Processor::~Processor()
{
    if (mInstalling)
        unlink(mPath);
}

void Processor::ProcessLine(const CString& line, unsigned& nextLine)
{
    CStringArray parts;

    split(line, ":", parts);
    nextLine++;

#pragma mark LABEL
    if (parts[0] == "LABEL")
    {
        printf("%u: %s\n", nextLine - 1, line.c_str());
        if (parts.size() != 2)
            goto error;
        if (mDoGoto && parts[1] == mLabel)
        {
            mDoGoto = false;
            mLabel = "";
        }
        mLabels[parts[1]] = nextLine;
        return;
    }
    
    if (mDoGoto)
        return;

    printf("%u: %s\n", nextLine - 1, line.c_str());

#pragma mark FILE
    if (parts[0] == "FILE")
    {
        if (parts.size() != 3)
            goto error;
        CString file, dir;
        if (parts[1][0] == '/')
        {
            file = parts[1].substr(1);
            dir = "/";
        }
        else
        {
            file = parts[1];
            dir = mCWD;
        }
        
        if (mInstalling)
            (*mHandleFile)(file, dir, mPath, ResolveConditional(parts[2]));
        else
        {
            CStringArray list, dirs;
            if (IsADirectory((dir == "/" ? dir : dir + "/" ) + file))
            {
                int fd = open(".", O_RDONLY, 0);

                chdir(dir);                
                FileListingForDirectoryWithIgnore(file, list, dirs, mIgnore);
                fchdir(fd);
                close(fd);
                if (list.size() == 0) //empty directory or ignored directory
                    (*mHandleFile)(file, dir, mPath, true);
                for (unsigned i=0; i<list.size(); ++i)
                    (*mHandleFile)(list[i], dir, mPath, true);
            }
            else
                (*mHandleFile)(file, dir, mPath, true);
        }

        return;
    }

#pragma mark GETPATH
    if (parts[0] == "GETPATH")
    {
        if (parts.size() != 3)
            goto error;
        mVars[parts[2]] = (*mGetPath)(parts[1]);
        return;
    }

#pragma mark CD
    if (parts[0] == "CD")
    {
        if (parts.size() != 3)
            goto error;
        if (!ResolveConditional(parts[2]))
            return;
        CString path = ResolveVar(parts[1]);
        if (path[0] == '/')
            mCWD = path;
        else
            mCWD += "/" + path;
        if (!IsADirectory(mCWD))
            (*mError)("%s is not a directory.\n", mCWD.c_str());
        return;
    }

#pragma mark IGNORE
    if (parts[0] == "IGNORE")
    {
        if (parts.size() != 2)
            goto error;
        mIgnore.insert(ResolveVar(parts[1]));
        return;
    }    
    
    if (!mInstalling)
        return;

#pragma mark GOTO
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
        mReturnStack.push(nextLine);
        return;
    }    

#pragma mark IF
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
        return;
    }

#pragma mark RETURN
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

#pragma mark SETVAR
    if (parts[0] == "SETVAR")
    {
        if (parts.size() < 3)
            goto error;
        CString temp = "";
        for (unsigned i=2; i<parts.size(); ++i)
            temp += ResolveVar(parts[i]);
        mVars[parts[1]] = temp;
        return;
    }

#pragma mark MKDIR
    if (parts[0] == "MKDIR")
    {
        if (parts.size() != 2)
            goto error;
        CString path = ResolveVar(parts[1]);
        if (path[0] != '/')
            path =  mCWD + "/" + path;
        if (mkdir_p(path))
            (*mError)(strerror(errno));
        return;
    }

#pragma mark RM
    if (parts[0] == "RM")
    {
        if (parts.size() != 2)
            goto error;
        CString path = ResolveVar(parts[1]);
        
        if (path[0] != '/')
            path = mCWD + "/" + path;

        if (IsADirectory(path))
        {
            CStringArray files, dirs;
            FileListingForDirectoryWithIgnore(path, files, dirs, set<CString>(), false);
            for (unsigned i=0; i<files.size(); ++i)
            {
                if (unlink(files[i]))
                    (*mError)("%s", strerror(errno));
            }

            while (!dirs.empty())
            {
                if (rmdir(dirs.back()))
                    (*mError)("%s", strerror(errno));
                dirs.pop_back();
            }
        }
        else if (DoesFileExist(path))
        {
            if (unlink(path))
                (*mError)("%s", strerror(errno));
        }
        
        return;
    }

#pragma mark ASK
    if (parts[0] == "ASK")
    {
        if (parts.size() != 3)
            goto error;
        mConditionals[parts[1]] = (*mAsk)(parts[2]);
        return;
    }

#pragma mark AUTH
    if (parts[0] == "AUTH")
    {
        if (parts.size() != 2)
            goto error;
        mConditionals[parts[1]] = (*mAuth)();
        return;
    }

#pragma mark ECHO
    if (parts[0] == "ECHO")
    {
        if (parts.size() != 3)
            goto error;
        (mEcho)(parts[1], ResolveConditional(parts[2]));
        return;
    }

#pragma mark PRIVILEGED
    if (parts[0] == "PRIVILEGED")
    {
        if (parts.size() != 2)
            goto error;
        (mPriv)(ResolveConditional(parts[1]));
        return;
    }

error:
    (*mError)("%s is an invalid command", line.c_str());
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
