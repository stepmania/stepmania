/*
 *  BuildInstaller.cpp
 *  stepmania
 *
 *  Created by Steve Checkoway on Sun Sep 07 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

using namespace std;

#include <unistd.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stack>
#include <map>
#include <cerrno>
#include <cassert>
#include "StdString.h"
#include "InstallerFile.h"
#include "Processor.h"
#include "Util.h"

void HandleFile(const CString& file, const CString& dir,
				const CString& archivePath)
{
	CString from = dir + '/' + file;
	CString to = archivePath + '/' + file;
	char copy[] = "/bin/cp";
	char arg[] = "-R";
	
	MakeAllButLast(to);
	if (CallTool(copy, arg, from.c_str(), to.c_str(), NULL))
	{
		fprintf(stderr, "Couldn't copy file from \"%s\" to \"%s\".",
				from.c_str(), to.c_str());
		exit(-10);
	}
}

const CString GetPath(const CString& ID)
{
    if (ID == "install path")
    {
        char cwd[MAXPATHLEN];
        char path[MAXPATHLEN];
        char *ptr;
        size_t temp;

        getwd(cwd);
        printf("Enter a path (relative or absolute) to the install files\n"
               "The current working directory is: %s\n"
               "> ", cwd);
        ptr = fgets(path, MAXPATHLEN, stdin);
        if ((temp = strlen(path)))
        {
            if (path[temp - 1] == '\n')
                path[temp - 1] = '\000';
        }
        assert(ptr);
        while (*ptr != '\000' && (*ptr == ' ' || *ptr == '\t'))
            ++ptr;
        if (*ptr == '/')
            return ptr;
        return CString(cwd) + "/" + CString(ptr);       
    }
    
    fprintf(stderr, "Unknown path command, return `.'\n");
    return ".";
}

void PrintUsage(int err)
{
    printf("usage: BuildInstaller config [dir]\n\n"
           "config:  The configuration file for the installer.\n"
           "dir:     The output directory. It is created if it doesn't exist\n");
    exit(err);
}

int main(int argc, char *argv[])
{
    CString inFile, outDir;

    switch (argc)
    {
        case 1:
            printf("BuildInstaller needs to be run with at least one argument.\n");
            PrintUsage(1);

        case 2:
            inFile = argv[1];
            outDir = ".";
            break;

        case 3:
            inFile = argv[1];
            outDir = argv[2];
            break;

        default:
            printf("BuildInstaller takes at most 2 arguments.\n");
            PrintUsage(2);
    }

    if (inFile == "help" || inFile == "-h" || inFile == "--help")
        PrintUsage(0);

    outDir += "/files";

    InstallerFile config(inFile);
    unsigned nextLine = 0;

    if (!config.ReadFile())
    {
        printf("Couldn't read config file, \"%s\".\n", inFile.c_str());
        return 3;
    }
    
    if (mkdir_p(outDir))
    {
        printf("The mkdir_p call failed.\n");
        return 4;
    }

	CString archivePath = outDir;

    Processor p(archivePath, HandleFile, GetPath, NULL, false);

    while (nextLine < config.GetNumLines())
        p.ProcessLine(config.GetLine(nextLine), nextLine);

    if (!config.WriteFile(outDir + "/config"))
    {
        printf("%s\n", strerror(errno));
        return 7;
    }

    return 0;
}
