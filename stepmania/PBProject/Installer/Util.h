using namespace std;
#include "StdString.h"
#include <vector>
#include <set>

void split(const CString& Source, const CString& Deliminator, vector<CString>& AddIt);
bool IsADirectory(const CString& path);
bool DoesFileExist(const CString& path);
CString LastPathComponent(const CString& path);
void FileListingForDirectoryWithIgnore(const CString& path, CStringArray& list, CStringArray& dirs,
                                       const set<CString>& ignore, bool ignoreDot = true);
int mkdir_p(const CString& path);
int MakeAllButLast(const CString& path);
void rm_rf(const CString& path);
int CallTool3(bool blocking, int fd_in, int fd_out, int fd_err, const char *path, char *const *arguments);
inline int CallTool(char *path, ...) { return CallTool3(true, -1, -1, -1, path, &path); }
inline int CallTool2(bool blocking, int fd_in, int fd_out, int fd_err, char *path, ...)
{ return CallTool3(blocking, fd_in, fd_out, fd_err, path, &path); }

/*
 * (c) 2003-2004 Steve Checkoway
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
