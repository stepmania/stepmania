/*
 * RageFile: high-level file abstraction
 */

#ifndef RAGE_FILE_H
#define RAGE_FILE_H

class RageFileObj;


/* This is a simple file I/O interface.  Although most of these operations
 * are straightforward, there are several of them; most of the time, you'll
 * only want to implement RageFileObj. */
class RageBasicFile
{
public:
	virtual ~RageBasicFile() { }

	virtual CString GetError() const = 0;
	virtual void ClearError() = 0;
	virtual bool AtEOF() const = 0;

	/* Seek to the given absolute offset.  Return to the position actually
	 * seeked to; if the position given was beyond the end of the file, the
	 * return value will be the size of the file. */
	virtual int Seek( int iOffset ) = 0;
	virtual int Seek( int offset, int whence ) = 0;
	virtual int Tell() const = 0;

	/* Read at most iSize bytes into pBuf.  Return the number of bytes read,
	 * 0 on end of stream, or -1 on error.  Note that reading less than iSize
	 * does not necessarily mean that the end of the stream has been reached;
	 * keep reading until 0 is returned. */
	virtual int Read( void *pBuffer, size_t iBytes ) = 0;
	virtual int Read( CString &buffer, int bytes = -1 ) = 0;
	virtual int Read( void *buffer, size_t bytes, int nmemb ) = 0;

	/* Write iSize bytes of data from pBuf.  Return 0 on success, -1 on error. */
	virtual int Write( const void *pBuffer, size_t iBytes ) = 0;
	virtual int Write( const CString &sString ) = 0;
	virtual int Write( const void *buffer, size_t bytes, int nmemb ) = 0;

	/* Due to buffering, writing may not happen by the end of a Write() call, so not
	 * all errors may be returned by it.  Data will be flushed when the stream (or its
	 * underlying object) is destroyed, but errors can no longer be returned.  Call
	 * Flush() to flush pending data, in order to check for errors. */
	virtual int Flush() = 0;

	virtual int GetLine( CString &out ) = 0;
	virtual int PutLine( const CString &str ) = 0;

	virtual int GetFileSize() const = 0;
};

/* This is the high-level interface, which interfaces with RageFileObj implementations
 * and RageFileManager. */
class RageFile: public RageBasicFile
{
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

	bool AtEOF() const;
	CString GetError() const;
	void ClearError();
	bool IsGood() const { return IsOpen() && !AtEOF() && GetError().empty(); }

	int Tell() const;
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
	void SetError( const CString &err );
	
private:
	RageFileObj *m_File;
	CString	m_Path;
	CString	m_sError;
	int		m_Mode;
};

/* Convenience wrappers for reading binary files. */
namespace FileReading
{
	/* On error, these set sError to the error message.  If sError is already
	 * non-empty, nothing happens. */
	void ReadBytes( RageBasicFile &f, void *buf, int size, CString &sError );
	uint8_t read_8( RageBasicFile &f, CString &sError );
	int16_t read_16_le( RageBasicFile &f, CString &sError );
	uint16_t read_u16_le( RageBasicFile &f, CString &sError );
	int32_t read_32_le( RageBasicFile &f, CString &sError );
	uint32_t read_u32_le( RageBasicFile &f, CString &sError );
};

#endif
