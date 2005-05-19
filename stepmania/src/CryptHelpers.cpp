#include "global.h"
#include "CryptHelpers.h"

// crypt headers
#include "crypto51/files.h"



void RageFileStore::StoreInitialize(const NameValuePairs &parameters)
{
	const char *fileName;
	if (parameters.GetValue("InputFileName", fileName))
	{
		if( !m_file.Open(fileName, RageFile::READ) )
			throw OpenErr(fileName);
	}
	else
	{
		ASSERT(0);
	}
	m_waiting = false;
}

unsigned long RageFileStore::MaxRetrievable() const
{
	if( !m_file.IsGood() )
		return 0;
	
	return m_file.GetFileSize() - m_file.Tell();
}

unsigned int RageFileStore::TransferTo2(BufferedTransformation &target, unsigned long &transferBytes, const std::string &channel, bool blocking)
{
	if( !m_file.IsGood() )
	{
		transferBytes = 0;
		return 0;
	}
	
	unsigned long size=transferBytes;
	transferBytes = 0;
	
	if (m_waiting)
		goto output;
	
	while( size && !m_file.AtEOF() )
	{
		{
			unsigned int spaceSize = 1024;
			m_space = HelpCreatePutSpace(target, channel, 1, (unsigned int)STDMIN(size, (unsigned long)UINT_MAX), spaceSize);
			
			m_len = m_file.Read( (char *)m_space, STDMIN(size, (unsigned long)spaceSize));
			if( m_len == -1 )
				throw ReadErr( m_file );
		}
		unsigned int blockedBytes;
output:
		blockedBytes = target.ChannelPutModifiable2(channel, m_space, m_len, 0, blocking);
		m_waiting = blockedBytes > 0;
		if (m_waiting)
			return blockedBytes;
		size -= m_len;
		transferBytes += m_len;
	}
	
	return 0;
}


unsigned int RageFileStore::CopyRangeTo2(BufferedTransformation &target, unsigned long &begin, unsigned long end, const std::string &channel, bool blocking) const
{
	if( !m_file.IsGood() )
		return 0;
	
	if (begin == 0 && end == 1)
	{
		int current = m_file.Tell();
		byte result;
		m_file.Read( &result, 1 );
		m_file.Seek( current );
		if( m_file.AtEOF() )
			return 0;

		unsigned int blockedBytes = target.ChannelPut(channel, byte(result), blocking);
		begin += 1-blockedBytes;
		return blockedBytes;
	}
	
	// TODO: figure out what happens on cin
	// (What does that mean?)
	int current = m_file.Tell();
	int newPosition = current + (streamoff)begin;
	
	if( newPosition >= m_file.GetFileSize() )
		return 0;	// don't try to seek beyond the end of file

	m_file.Seek(newPosition);
	try
	{
		assert(!m_waiting);
		unsigned long copyMax = end-begin;
		unsigned int blockedBytes = const_cast<RageFileStore *>(this)->TransferTo2(target, copyMax, channel, blocking);
		begin += copyMax;
		if (blockedBytes)
		{
			const_cast<RageFileStore *>(this)->m_waiting = false;
			return blockedBytes;
		}
	}
	catch(...)
	{
		m_file.ClearError();
		m_file.Seek(current);
		throw;
	}
	m_file.ClearError();
	m_file.Seek(current);
	
	return 0;
}

/*
 * (c) 2001-2004 Chris Danford
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
