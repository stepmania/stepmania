#include <sys/types.h>
#include <sys/stat.h>
#include <cerrno>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include "Util.h"

void split(const CString& Source, const CString& Deliminator, vector<CString>& AddIt)
{
    unsigned startpos = 0;

    do
    {
        unsigned pos = Source.find(Deliminator, startpos);
        if (pos == Source.npos) pos=Source.size();

        CString AddCString = Source.substr(startpos, pos-startpos);
        AddIt.push_back(AddCString);

        startpos=pos+Deliminator.size();
    }
    while (startpos <= Source.size());
}

bool IsADirectory(const CString& path)
{
    struct stat s;
    if (lstat(path, &s))
        return false;
    return ((s.st_mode & S_IFDIR) == S_IFDIR) &&
           ((s.st_mode & S_IFLNK) != S_IFLNK);
}

bool DoesFileExist(const CString& path)
{
    struct stat s;
    return !stat(path, &s);
}

CString LastPathComponent(const CString& path)
{
    unsigned pos = path.rfind('/');

    if (pos == path.npos)
        pos = 0;
    return path.substr(pos+1);
}

void FileListingForDirectoryWithIgnore(const CString& path, CStringArray& list, CStringArray& dirs,
                                       const set<CString>& ignore, bool ignoreDot)
{
    DIR *dir = opendir(path);
    dirent *entry;

    if (!dir)
        return;

    while ((entry = readdir(dir)))
    {
        if (ignoreDot && entry->d_name[0] == '.')
            continue;
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;
        if (ignore.find(LastPathComponent(path)) != ignore.end())
            continue;
        CString temp = path + "/";

        temp += entry->d_name;
        if (IsADirectory(temp))
        {
            dirs.push_back(temp);
            FileListingForDirectoryWithIgnore(temp, list, dirs, ignore, ignoreDot);
        }
        else
            list.push_back(temp);
    }
    closedir(dir);
}

int mkdir_p(const CString& path)
{
    CStringArray components;
    int ret = 0;
    int fd = open(".", O_RDONLY, 0);

    split(path, "/", components);

    if (path[0] == '/')
        chdir("/");
    for (unsigned i=0; i<components.size(); ++i)
    {
        if (components[i] == "")
            continue;
        if ((ret = mkdir(components[i], 0777)))
        {
            if (errno != EEXIST)
                goto err;
            else
                ret = 0;
        }
        chdir(components[i]);
    }

err:
    if (ret)
        fprintf(stderr, "%s\n", strerror(errno));
    fchdir(fd);
    close(fd);
    return ret;
}

int MakeAllButLast(const CString& path)
{
	size_t i = path.find_last_of("/");
	
	if (i == path.npos)
		return 0;
	CString dir = path.substr(0, i);
	printf("%s -> %s\n", path.c_str(), dir.c_str());
	return mkdir_p(dir);
}

void rm_rf(const CString& path)
{
	printf("deleting %s\n", path.c_str());
	if (IsADirectory(path))
	{
		CStringArray files, dirs;
		FileListingForDirectoryWithIgnore(path, files, dirs, set<CString>(), false);
		for (unsigned i=0; i<files.size(); ++i)
		{
			if (unlink(files[i]))
				fputs(strerror(errno), stderr);
		}
		
		while (!dirs.empty())
		{
			if (rmdir(dirs.back()))
				fputs(strerror(errno), stderr);
			dirs.pop_back();
		}
		if (rmdir(path))
			fputs(strerror(errno), stderr);
	}
	else if (DoesFileExist(path))
	{
		if (unlink(path))
			fputs(strerror(errno), stderr);
	}
	assert(!DoesFileExist(path));
}

int CallTool3(bool blocking, int fd_in, int fd_out, int fd_err, const char *path, char *const *arguments)
{
    if (!DoesFileExist(path))
        throw CString("The tool \"") + path + "\" does not exist.";
    pid_t pid = fork();

    switch (pid)
    {
        case -1: /* error */
            _exit(-1);
        case 0: /* child */
            if (fd_in > -1 && fd_in != 0)
			{
                dup2(fd_in, 0);
				close(fd_in);
			}
            
            if (fd_out > -1 && fd_in != 1)
			{
                dup2(fd_out, 1);
				close(fd_out);
			}
                
            if (fd_err > -1 && fd_in != 2)
			{
                dup2(fd_err, 2);
				close(fd_err);
			}
                
            execv(path, arguments);
            _exit(-1);
    }
    /* parent */
    if (!blocking)
        return 0;

    int status, returnStatus;

    waitpid(pid, &status, 0);
	returnStatus = WEXITSTATUS(status);
    return returnStatus;
}

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
