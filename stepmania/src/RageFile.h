#ifndef RageFile_H
#define RageFile_H

/*
-----------------------------------------------------------------------------
 Class: RageFile

 Desc: Encapsulates C and C++ file classes to deal with arch-specific oddities.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
        Steve Checkoway
-----------------------------------------------------------------------------
*/

#include <fstream>
#include <cstdio>
using namespace std;	// using "std::ifstream" causes problems below in VC6.  Why?!?

// call FixSlashes on any path that came from the user
void FixSlashesInPlace( CString &sPath );
CString FixSlashes( CString sPath );

void CollapsePath( CString &sPath );


class RageFile
{
private:
    FILE    *mFP;
    CString mPath;
    
public:
    RageFile() : mPath("") { mFP = NULL; }
    RageFile(const CString& path, const char *mode = "r") { Open(path, mode); }
    ~RageFile() { Close(); }
    
    bool Open(const CString& path, const char *mode = "r");
    void Close();
    
    bool IsOpen() { return (mFP == NULL); }
    bool AtEOF() { return !!feof(mFP); }
    int GetError() { return ferror(mFP); }
    FILE *GetFilePointer() { return mFP; }
    
    long Tell() { return ftell(mFP); }
    bool Seek(long offset, int origin = SEEK_CUR) { return !fseek(mFP, offset, origin); }
    void Rewind() { rewind(mFP); }
    
    // GetLine() strips new lines
    CString GetLine();
    bool PutString(const CString& string) { return fputs(string, mFP) >= 0; }
    size_t Read(void *buffer, size_t bytes);
    size_t Write(const void *buffer, size_t bytes);
};

#endif
