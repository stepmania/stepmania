#ifndef RAGE_FILE_H
#define RAGE_FILE_H

/*
-----------------------------------------------------------------------------
 Class: RageFile

 Desc: Encapsulates C and C++ file classes to deal with arch-specific oddities.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Steve Checkoway
	Glenn Maynard
-----------------------------------------------------------------------------
*/

// call FixSlashes on any path that came from the user
void FixSlashesInPlace( CString &sPath );
CString FixSlashes( CString sPath );
void CollapsePath( CString &sPath );

class RageFileObj;

class RageFile
{
	friend class RageFileObj;

public:
	enum OpenMode { READ, WRITE };
    RageFile();
    RageFile( const CString& path, OpenMode mode = READ );
    ~RageFile() { Close(); }

	const CString &GetRealPath() const { return m_Path; }
	CString GetPath() const;
    
    bool Open( const CString& path, OpenMode mode = READ );
    void Close();
    
	bool IsOpen() const { return m_File != NULL; }
	bool AtEOF() const { return m_EOF; }
	CString GetError() const { return m_Error; }
	void ClearError() { m_Error = ""; }
    
	int Tell() const { return m_FilePos; }
	int Seek( int offset );
	int SeekCur( int offset );
	int GetFileSize();
	void Rewind();
    
	/* Raw I/O: */
	int Write( const void *buffer, size_t bytes );
	int Write( const CString& string ) { return Write( string.data(), string.size() ); }
	int Read( void *buffer, size_t bytes );
	int Read( const CString &buffer, size_t bytes );

	/* These are just here to make wrappers (eg. vorbisfile, SDL_rwops) easier. */
	int Write( const void *buffer, size_t bytes, int nmemb );
	int Read( void *buffer, size_t bytes, int nmemb );
	int Seek( int offset, int whence );

	/* Line-based I/O: */
    CString GetLine();
	int GetLine( CString &out );
	int PutLine( const CString &str );

protected:
	void SetError( const CString &err ) { m_Error = err; } /* called by RageFileObj::SetError */
	
private:
	void ResetBuf();

	RageFileObj *m_File;
	CString	m_Path;
	OpenMode m_Mode;
	
	CString	m_Error;
	bool	m_EOF;
	int		m_FilePos;

	enum { BSIZE = 256 };
	char	m_Buffer[BSIZE];
	char	*m_pBuf;
	int		m_BufUsed;
};

#endif
