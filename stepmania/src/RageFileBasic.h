#ifndef RAGE_FILE_BASIC_H
#define RAGE_FILE_BASIC_H

/* This is a simple file I/O interface.  Although most of these operations
 * are straightforward, there are several of them; most of the time, you'll
 * only want to implement RageFileObj. */
class RageFileBasic
{
public:
	virtual ~RageFileBasic() { }

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

	/* This returns a descriptive path for the file, or "". */
	virtual CString GetDisplayPath() const { return ""; }

	virtual RageFileBasic *Copy() const = 0;

	virtual int GetLine( CString &out ) = 0;
	virtual int PutLine( const CString &str ) = 0;

	virtual int GetFileSize() const = 0;
};

class RageFileObj: public RageFileBasic
{
public:
	RageFileObj();
	virtual ~RageFileObj() { }

	virtual CString GetError() const { return m_sError; }
	virtual void ClearError() { SetError(""); }
	
	bool AtEOF() const { return m_bEOF; }

	int Seek( int iOffset );
	int Seek( int offset, int whence );
	int Tell() const { return m_iFilePos; }

	int Read( void *pBuffer, size_t iBytes );
	int Read( CString &buffer, int bytes = -1 );
	int Read( void *buffer, size_t bytes, int nmemb );

	int Write( const void *pBuffer, size_t iBytes );
	int Write( const CString &sString ) { return Write( sString.data(), sString.size() ); }
	int Write( const void *buffer, size_t bytes, int nmemb );

	int Flush();

	int GetLine( CString &out );
	int PutLine( const CString &str );

	virtual int GetFileSize() const = 0;
	virtual CString GetDisplayPath() const { return ""; }
	virtual RageFileBasic *Copy() const { FAIL_M( "Copying unimplemented" ); }

protected:
	virtual int SeekInternal( int iOffset ) { FAIL_M( "Seeking unimplemented" ); }
	virtual int ReadInternal( void *pBuffer, size_t iBytes ) = 0;
	virtual int WriteInternal( const void *pBuffer, size_t iBytes ) = 0;
	virtual int FlushInternal() { return 0; }

	void SetError( const CString &sError ) { m_sError = sError; }
	CString m_sError;

private:
	int FillBuf();
	void ResetBuf();

	bool m_bEOF;
	int m_iFilePos;

	enum { BSIZE = 1024 };
	char m_Buffer[BSIZE];
	char *m_pBuf;
	int  m_iBufAvail;
};

#endif

