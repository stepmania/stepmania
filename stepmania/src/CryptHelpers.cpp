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
	if( !m_file.IsOpen() )
		return 0;

	return m_file.GetFileSize() - m_file.Tell();
}

unsigned int RageFileStore::TransferTo2(BufferedTransformation &target, unsigned long &transferBytes, const std::string &channel, bool blocking)
{
	if( !m_file.IsOpen() )
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

	if (!m_file.AtEOF())
		throw ReadErr();

	return 0;
}


unsigned int RageFileStore::CopyRangeTo2(BufferedTransformation &target, unsigned long &begin, unsigned long end, const std::string &channel, bool blocking) const
{
	if( !m_file.IsOpen() )
		return 0;

	if (begin == 0 && end == 1)
	{
		int current = m_file.Tell();
		byte result;
		m_file.Read( &result, 1 );
		if (m_file.AtEOF())	// GCC workaround: 2.95.2 doesn't have char_traits<char>::eof()
		{
			m_file.Seek( current );
			return 0;
		}
		else
		{
			unsigned int blockedBytes = target.ChannelPut(channel, byte(result), blocking);
			begin += 1-blockedBytes;
			m_file.Seek( current );
			return blockedBytes;
		}
	}

	// TODO: figure out what happens on cin
	int current = m_file.Tell();
	int endPosition = m_file.GetFileSize();
	m_file.Seek( endPosition );
	int newPosition = current + (streamoff)begin;

	if (newPosition >= endPosition)
	{
		m_file.Seek(current);
		return 0;	// don't try to seek beyond the end of file
	}
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

void RageFileSink::IsolatedInitialize(const NameValuePairs &parameters)
{
	const char *fileName;
	if (parameters.GetValue("OutputFileName", fileName))
	{
        // does this do anything other than cause g++ to emit a warning?
		ios::openmode binary = parameters.GetValueWithDefault("OutputBinaryMode", true) ? ios::binary : ios::openmode(0);
		if( !m_file.Open( fileName, RageFile::WRITE ) )	// trucates existing data
			throw OpenErr(fileName);
	}
	else
	{
		ASSERT(0);
	}
}

bool RageFileSink::IsolatedFlush(bool hardFlush, bool blocking)
{
	if (!m_file.IsOpen())
		throw Err("FileSink: output stream not opened");

	m_file.Flush();
	if( !m_file.GetError().empty() )
		throw WriteErr();

	return false;
}

unsigned int RageFileSink::Put2(const byte *inString, unsigned int length, int messageEnd, bool blocking)
{
	if (!m_file.IsOpen())
		throw Err("FileSink: output stream not opened");

	m_file.Write((const char *)inString, length);

	if (messageEnd)
		m_file.Flush();

	if( !m_file.GetError().empty() )
		throw WriteErr();

	return 0;
}

