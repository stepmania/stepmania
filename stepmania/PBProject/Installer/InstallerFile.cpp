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
