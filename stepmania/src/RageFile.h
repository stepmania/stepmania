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

class RageFileObj;

class RageFile
{
	friend class RageFileObj;

public:
	enum
	{
		READ			= 0x1,
		WRITE			= 0x2,

		/* Always write directly to the destination file; don't do a safe write. (for logs) */
		STREAMED		= 0x4
	};

    RageFile();
    RageFile( const CString& path, int mode = READ );
    ~RageFile() { Close(); }
	RageFile( const RageFile &cpy );

	/* Use GetRealPath to get the path this file was opened with; use that if you
	 * want a path that will probably get you the same file again.
	 *
	 * GetPath can be overridden by drivers.  Use it to get a path for display;
	 * it may give more information, such as the name of the archive the file
	 * is in.  It has no parsable meaning. */
	const CString &GetRealPath() const { return m_Path; }
	CString GetPath() const;
    
    bool Open( const CString& path, int mode = READ );
    void Close();
    
	bool IsOpen() const { return m_File != NULL; }
	int GetOpenMode() const { return m_Mode; }
	bool AtEOF() const { return m_EOF; }
	CString GetError() const { return m_Error; }
	void ClearError() { m_Error = ""; }

	int Tell() const { return m_FilePos; }
	int Seek( int offset );
	int SeekCur( int offset );
	int GetFileSize();
	void Rewind();
    
	/* Raw I/O: */
	int Read( void *buffer, size_t bytes );
	int Read( CString &buffer, size_t bytes );
	int Write( const void *buffer, size_t bytes );
	int Write( const CString& string ) { return Write( string.data(), string.size() ); }
	int Flush();

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
	int		m_Mode;
	
	CString	m_Error;
	bool	m_EOF;
	int		m_FilePos;

	enum { BSIZE = 256 };
	char	m_Buffer[BSIZE];
	char	*m_pBuf;
	int		m_BufUsed;
};

#endif
