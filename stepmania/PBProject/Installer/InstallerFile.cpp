/*
 *  InstallerFile.cpp
 *  stepmania
 *
 *  Created by Steve Checkoway on Sun Sep 07 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

using namespace std;

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <cassert>
#include <fcntl.h>
#include <cstdio>
#include "StdString.h"
#include "InstallerFile.h"

const unsigned maxLineLen = 1024;
const unsigned short magicNumber = 0xBAD1;

bool InstallerFile::ReadFile(const CString& path)
{
    const CString& file = (path == "" ? mPath : path);
    int fd = open(file, O_RDONLY, S_IROTH);
    if (fd == -1)
        return false;
    struct stat sb;

    if (fstat(fd, &sb) == -1)
        return false;


    unsigned fileLength = sb.st_size;
    
    if (fileLength <= 2)
        return false;
    fileLength += 1000; /* Just in case */
    
    char buf[fileLength];

    unsigned len = read(fd, buf, fileLength);

    close(fd);
    unsigned short temp = (buf[0] << 8) | (buf[1] & 0x00FF) ;

    if (temp == magicNumber)
        ReadBinary(buf + 2, len - 2);
    else
        ReadText(buf, len);
    
    return mLines.size();
}

void InstallerFile::ReadBinary(const char *buf, unsigned len)
{
    unsigned pos = 0;
    while (pos < len)
    {
        if (len < pos + 2)
            break;
        unsigned length = *(unsigned short *)buf;
        
        buf += 2;
        pos += 2;
        if (len < pos + length)
            break;
        mLines.push_back(CString(buf, length));
        buf += length;
        pos += length;
    }
}

void InstallerFile::ReadText(const char *buf, unsigned len)
{
    unsigned pos = 0;

    while (pos < len)
    {
        unsigned lineLen = 0;
        bool ignoreLine = false;
        char line[maxLineLen];
                
        while (lineLen < maxLineLen && pos < len)
        {
            ++pos;
            if (strspn(buf, "\r\n"))
            {
                ++buf;
                break;
            }
            if (!lineLen && strspn(buf, " \t")){
                ++buf;
                continue;
            }
            if (!lineLen && *buf == '#')
            {
                ignoreLine = true;
                ++buf;
            }
            else
                line[lineLen++] = *(buf++);
        }

        if (!lineLen || ignoreLine)
            continue;
        mLines.push_back(CString(line, lineLen));
    }
}

bool InstallerFile::WriteFile(const CString& path)
{
    /* Use buffered I/O */
    FILE *fp = fopen(path, "w");

    if (fp == NULL)
        return false;

    fwrite(&magicNumber, sizeof(magicNumber), 1, fp);

    for (unsigned i=0; i<mLines.size(); ++i)
    {
        CString& str = mLines[i];
        unsigned short length = str.length();
        fwrite(&length, 1, sizeof(length), fp);
        fwrite(str.c_str(), 1, length, fp);
    }

    return true;
}

CString InstallerFile::GetLine(unsigned line)
{
    if (line >= mLines.size())
        return "";
    return mLines[line];
}
