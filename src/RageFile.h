/* RageFile - High-level file access. */

#ifndef RAGE_FILE_H
#define RAGE_FILE_H

#include "RageFileBasic.h"

#include <cstddef>
#include <cstdint>

struct lua_State;

/**
 * @brief High-level file access.
 *
 * This is the high-level interface, which interfaces with RageFileObj
 * implementations and RageFileManager. */
class RageFile: public RageFileBasic
{
public:
	enum
	{
		READ		= 0x1,
		WRITE		= 0x2,

		/* Always write directly to the destination file; don't do a safe write. (for logs) */
		STREAMED	= 0x4,

		/* Flush the file to disk on close.  Combined with not streaming, this results
		 * in very safe writes, but is slow. */
		SLOW_FLUSH	= 0x8
	};

	RageFile();
	~RageFile() { Close(); }
	RageFile( const RageFile &cpy );
	RageFile *Copy() const;

	/*
	 * Use GetRealPath to get the path this file was opened with; use that if you
	 * want a path that will probably get you the same file again.
	 *
	 * GetPath can be overridden by drivers.  Use it to get a path for display;
	 * it may give more information, such as the name of the archive the file
	 * is in.  It has no parsable meaning.
	 */
	const RString &GetRealPath() const { return m_Path; }
	RString GetPath() const;

	bool Open( const RString& path, int mode = READ );
	void Close();
	bool IsOpen() const { return m_File != nullptr; }
	int GetMode() const { return m_Mode; }

	bool AtEOF() const;
	RString GetError() const;
	void ClearError();

	int Tell() const;
	int Seek( int offset );
	int GetFileSize() const;
	int GetFD();

	/* Raw I/O: */
	int Read( void *buffer, std::size_t bytes );
	int Read( RString &buffer, int bytes = -1 );
	int Write( const void *buffer, std::size_t bytes );
	int Write( const RString& string ) { return Write( string.data(), string.size() ); }
	int Flush();

	/* These are just here to make wrappers (eg. vorbisfile, SDL_rwops) easier. */
	int Write( const void *buffer, std::size_t bytes, int nmemb );
	int Read( void *buffer, std::size_t bytes, int nmemb );
	int Seek( int offset, int whence );

	/* Line-based I/O: */
	int GetLine( RString &out );
	int PutLine( const RString &str );

	void EnableCRC32( bool on=true );
	bool GetCRC32( std::uint32_t *iRet );

	// Lua
	virtual void PushSelf( lua_State *L );
private:
	void SetError( const RString &err );

	RageFileBasic *m_File;
	RString	m_Path;
	RString	m_sError;
	int		m_Mode;

	// Swallow up warnings. If they must be used, define them.
	RageFile& operator=(const RageFile& rhs);
};

/** @brief Convenience wrappers for reading binary files. */
namespace FileReading
{
	/* On error, these set sError to the error message.  If sError is already
	 * non-empty, nothing happens. */
	void ReadBytes( RageFileBasic &f, void *buf, int size, RString &sError );
	void SkipBytes( RageFileBasic &f, int size, RString &sError );
	void Seek( RageFileBasic &f, int iOffset, RString &sError );
	RString ReadString( RageFileBasic &f, int size, RString &sError );
	std::uint8_t read_8( RageFileBasic &f, RString &sError );
	std::int16_t read_16_le( RageFileBasic &f, RString &sError );
	std::uint16_t read_u16_le( RageFileBasic &f, RString &sError );
	std::int32_t read_32_le( RageFileBasic &f, RString &sError );
	std::uint32_t read_u32_le( RageFileBasic &f, RString &sError );
};

#endif

/*
 * Copyright (c) 2003-2005 Glenn Maynard, Chris Danford
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
