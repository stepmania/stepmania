/*
 *  Util.h
 *  stepmania
 *
 *  Created by Steve Checkoway on Mon Sep 15 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

using namespace std;
#include "StdString.h"
#include <vector>
#include <set>

void split(const CString& Source, const CString& Deliminator, vector<CString>& AddIt);
bool IsADirectory(const CString& path);
bool DoesFileExist(const CString& path);
inline CString LastPathComponent(const CString& path);
void FileListingForDirectoryWithIgnore(const CString& path, CStringArray& list, CStringArray& dirs,
                                       const set<CString>& ignore, bool ignoreDot = true);
int mkdir_p(const CString& path);
int CallTool3(bool blocking, int fd_in, int fd_out, int fd_err, const char *path, char *const *arguments);
inline int CallTool(char *path, ...) { return CallTool3(true, -1, -1, -1, path, &path); }
inline int CallTool2(bool blocking, int fd_in, int fd_out, int fd_err, char *path, ...)
{ return CallTool3(blocking, fd_in, fd_out, fd_err, path, &path); }
