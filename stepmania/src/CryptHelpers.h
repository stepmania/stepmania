#ifndef CryptHelpers_H
#define CryptHelpers_H

#include "RageFile.h"

// crypt headers
#include "crypto51/files.h"
#include "crypto51/filters.h"
#include "crypto51/cryptlib.h"

using namespace CryptoPP;

//! .
class RageFileStore : public Store, private FilterPutSpaceHelper
{
public:
	class Err : public Exception
	{
	public:
		Err(const std::string &s) : Exception(IO_ERROR, s) {}
	};
	class OpenErr : public Err {public: OpenErr(const std::string &filename) : Err("FileStore: error opening file for reading: " + filename) {}};
	struct ReadErr : public Err { ReadErr(const RageFile &f) : Err("RageFileStore(" + f.GetPath() + "): read error: " + f.GetError() ) {}};

	RageFileStore() {}
	RageFileStore(const char *filename)
		{StoreInitialize(MakeParameters("InputFileName", filename));}

	unsigned long MaxRetrievable() const;
	unsigned int TransferTo2(BufferedTransformation &target, unsigned long &transferBytes, const std::string &channel=NULL_CHANNEL, bool blocking=true);
	unsigned int CopyRangeTo2(BufferedTransformation &target, unsigned long &begin, unsigned long end=ULONG_MAX, const std::string &channel=NULL_CHANNEL, bool blocking=true) const;

private:
	void StoreInitialize(const NameValuePairs &parameters);
	
	mutable RageFile m_file;	// mutable because reading from a file is not a const operation
	byte *m_space;
	int m_len;
	bool m_waiting;
};

//! .
class RageFileSource : public SourceTemplate<RageFileStore>
{
public:
	typedef FileStore::Err Err;
	typedef FileStore::OpenErr OpenErr;
	typedef FileStore::ReadErr ReadErr;

	RageFileSource(BufferedTransformation *attachment = NULL)
		: SourceTemplate<RageFileStore>(attachment) {}
	RageFileSource(std::istream &in, bool pumpAll, BufferedTransformation *attachment = NULL)
		: SourceTemplate<RageFileStore>(attachment) {SourceInitialize(pumpAll, MakeParameters("InputStreamPointer", &in));}
	RageFileSource(const char *filename, bool pumpAll, BufferedTransformation *attachment = NULL, bool binary=true)
		: SourceTemplate<RageFileStore>(attachment) {SourceInitialize(pumpAll, MakeParameters("InputFileName", filename)("InputBinaryMode", binary));}
};


//! .
class RageFileSink : public Sink
{
public:
	class Err : public Exception
	{
	public:
		Err(const std::string &s) : Exception(IO_ERROR, s) {}
	};
	class OpenErr : public Err {public: OpenErr(const std::string &filename) : Err("FileSink: error opening file for writing: " + filename) {}};
	struct WriteErr : public Err { WriteErr(const RageFile &f) : Err("RageFileSink(" + f.GetPath() + "): write error: " + f.GetError()) {}};

	RageFileSink() {}
	RageFileSink(const char *filename, bool binary=true)
		{IsolatedInitialize(MakeParameters("OutputFileName", filename)("OutputBinaryMode", binary));}

	void IsolatedInitialize(const NameValuePairs &parameters);
	unsigned int Put2(const byte *inString, unsigned int length, int messageEnd, bool blocking);
	bool IsolatedFlush(bool hardFlush, bool blocking);

private:
	RageFile m_file;
};

#endif

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
