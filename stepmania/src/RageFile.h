#ifndef RAGE_FILE_H
#define RAGE_FILE_H

/*
-----------------------------------------------------------------------------
 Class: RageFile

 Desc: Encapsulates C and C++ file classes to deal with arch-specific oddities.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Steve Checkoway
-----------------------------------------------------------------------------
*/

#include <cstdio>

// call FixSlashes on any path that came from the user
void FixSlashesInPlace( CString &sPath );
CString FixSlashes( CString sPath );

void CollapsePath( CString &sPath );


class RageFile
{
private:
    FILE    *mFP;
    CString mPath;

	enum { BSIZE = 256 };
	char	m_Buffer[BSIZE];
	char	*m_pBuf;
	int		m_BufUsed;

	void ResetBuf();
public:
	enum OpenMode { READ, WRITE };
    RageFile() : mPath("") { mFP = NULL; }
    RageFile( const CString& path, OpenMode mode = READ );
    ~RageFile() { Close(); }
    
    bool Open( const CString& path, OpenMode mode = READ );
    void Close();
    
    bool IsOpen() { return (mFP != NULL); }
    bool AtEOF() { return (feof(mFP) != 0); }
    int GetError() { return ferror(mFP); }
	void ClearError() { clearerr(mFP); }
    
    long Tell() { return ftell(mFP); }
    bool Seek(long offset) { return !fseek(mFP, offset, SEEK_SET); }
    void Rewind() { rewind(mFP); }
    
	/* Raw I/O: */
    // GetLine() strips new lines
	bool PutString(const CString& string) { return fputs(string, mFP) >= 0; }
	int Read(void *buffer, size_t bytes);
	int Write(const void *buffer, size_t bytes);
    int PutLine( const CString &str );

	/* Line-based I/O: */
    CString GetLine();
	void GetLine( CString &out );
};

#endif
