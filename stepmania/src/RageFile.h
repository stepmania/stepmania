/*
 * RageFile: high-level file abstraction
 */

#ifndef RAGE_FILE_H
#define RAGE_FILE_H

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
		STREAMED		= 0x4,

		/* Flush the file to disk on close.  Combined with not streaming, this results
		 * in very safe writes, but is slow. */
		SLOW_FLUSH		= 0x8
	};

    RageFile();
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
	bool IsGood() const { return IsOpen() && !AtEOF() && m_Error.empty(); }

	int Tell() const { return m_FilePos; }
	int Seek( int offset );
	int GetFileSize() const;
    
	/* Raw I/O: */
	int Read( void *buffer, size_t bytes );
	int Read( CString &buffer, int bytes = -1 );
	int Write( const void *buffer, size_t bytes );
	int Write( const CString& string ) { return Write( string.data(), string.size() ); }
	int Flush();

	/* These are just here to make wrappers (eg. vorbisfile, SDL_rwops) easier. */
	int Write( const void *buffer, size_t bytes, int nmemb );
	int Read( void *buffer, size_t bytes, int nmemb );
	int Seek( int offset, int whence );

	/* Line-based I/O: */
	int GetLine( CString &out );
	int PutLine( const CString &str );

protected:
	void SetError( const CString &err ) { m_Error = err; } /* called by RageFileObj::SetError */
	
private:
	int FillBuf();
	void ResetBuf();

	RageFileObj *m_File;
	CString	m_Path;
	int		m_Mode;
	
	CString	m_Error;
	bool	m_EOF;
	int		m_FilePos;

	enum { BSIZE = 1024 };
	char	m_Buffer[BSIZE];
	char	*m_pBuf;
	int		m_BufAvail;
};

/* Convenience wrappers for reading binary files. */
namespace FileReading
{
	/* On error, these set sError to the error message.  If sError is already
	 * non-empty, nothing happens. */
	void ReadBytes( RageFile &f, void *buf, int size, CString &sError );
	uint8_t read_8( RageFile &f, CString &sError );
	int16_t read_16_le( RageFile &f, CString &sError );
	uint16_t read_u16_le( RageFile &f, CString &sError );
	int32_t read_32_le( RageFile &f, CString &sError );
	uint32_t read_u32_le( RageFile &f, CString &sError );
};

#endif
