/*
 *  InstallerFile.h
 *  stepmania
 *
 *  Created by Steve Checkoway on Sun Sep 07 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

#ifndef INSTALLER_FILE_H
#define INSTALLER_FILE_H

#include <cstdio>
#include <vector>

class InstallerFile {
private:
    CString mPath;
    vector<CString> mLines;

    void ReadBinary(const char *buf, unsigned len);
    void ReadText(const char *buf, unsigned len);

public:
    InstallerFile(CString path) : mPath(path) { }
    bool ReadFile(const CString& path = "");
    bool WriteFile(const CString& path);
    CString GetLine(unsigned line);
    unsigned GetNumLines() { return mLines.size(); }
};

#endif
