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
#include <cerrno>
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
	
	if (parts.size() == 0)
		return;

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
        if (parts.size() == 3)
		{
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
			{
				if (ResolveConditional(parts[2]))
					rm_rf(dir + '/' + file);
				(*mHandleFile)(file, dir, mPath);
			}
			else
			{				
				if (IsADirectory((dir == "/" ? dir : dir + "/" ) + file))
				{
					CStringArray list, dirs;
					int fd = open(".", O_RDONLY, 0);

					chdir(dir);                
					FileListingForDirectoryWithIgnore(file, list, dirs,
														mIgnore);
					fchdir(fd);
					close(fd);
					if (list.size() == 0) //empty dir or ignored dir
						(*mHandleFile)(file, dir, mPath);
					for (unsigned i=0; i<list.size(); ++i)
						(*mHandleFile)(list[i], dir, mPath);
				}
				else
					(*mHandleFile)(file, dir, mPath);
			}
		}
		else if (parts.size() == 4)
		{
			CString iFile, iDir, fFile, fDir;
			
			if (parts[1][0] == '/')
			{
				iFile = parts[1].substr(1);
				iDir = "/";
			}
			else
			{
				iFile = parts[1];
				iDir = mCWD;
			}
			if (parts[2][0] == '/')
			{
				fFile = parts[2].substr(1);
				fFile = "/";
			}
			else
			{
				fFile = parts[2];
				fDir = mCWD;
			}
			if (mInstalling)
			{
				if (ResolveConditional(parts[3]))
					rm_rf(fDir + '/' + fFile);
				(*mHandleFile)(fFile, fDir, mPath);
			}
			else
			{
				// Now the fun part
				char temp[] = "/tmp/tmpXXXXXX";
				CString dir;
				size_t pos = fFile.find_last_of('/');
				char arg[] = "-R";
				char tool[] = "/bin/cp";
				
				mkdtemp(temp);
				dir.Format("%s/%s/%s", temp, fDir == mCWD ? "." : fDir.c_str(),
						   pos == fFile.npos ? "" :
						   fFile.substr(0, pos).c_str());
				if (mkdir_p(dir))
				{
					fprintf(stderr, "Couldn't create directory: %s\n",
							dir.c_str());
					exit(-1);
				}
				if (CallTool(tool, arg, (iDir + "/" + iFile).c_str(),
							 dir.c_str(), NULL))
				{
					fprintf(stderr, "Couldn't copy file(s).\n");
					exit(-1);
				}
				
				if (IsADirectory((iDir == "/" ? iDir : iDir + "/") + iFile))
				{
					CStringArray list, dirs;
					int fd = open(".", O_RDONLY, 0);
					
					chdir(dir + "/" + fDir);
					FileListingForDirectoryWithIgnore(fFile, list, dirs,
													  mIgnore);
					fchdir(fd);
					close(fd);
					if (list.size() == 0)
						(*mHandleFile)(fFile, temp, mPath);
					for (unsigned i=0; i<list.size(); ++i)
						(*mHandleFile)(list[i], temp, mPath);
				}
				else
					(*mHandleFile)(fFile, temp, mPath);
				rm_rf(temp);
			}
		}
		else
			goto error;

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
            mReturnStack.push(nextLine);
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
        mReturnStack.push(nextLine);
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
		
		rm_rf(path);
        
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

#pragma mark ECHO
    if (parts[0] == "ECHO")
    {
        if (parts.size() != 3)
            goto error;
        (mEcho)(parts[1], ResolveConditional(parts[2]));
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
