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
	class ReadErr : public Err {public: ReadErr() : Err("FileStore: error reading file") {}};

	RageFileStore() {}
	RageFileStore(const char *filename)
		{StoreInitialize(MakeParameters("InputFileName", filename));}

	unsigned long MaxRetrievable() const;
	unsigned int TransferTo2(BufferedTransformation &target, unsigned long &transferBytes, const std::string &channel=NULL_CHANNEL, bool blocking=true);
	unsigned int CopyRangeTo2(BufferedTransformation &target, unsigned long &begin, unsigned long end=ULONG_MAX, const std::string &channel=NULL_CHANNEL, bool blocking=true) const;

private:
	void StoreInitialize(const NameValuePairs &parameters);
	
	mutable RageFile m_file;	// mutable so that we can call RageFile::GetFileSize()
	byte *m_space;
	unsigned int m_len;
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
	class WriteErr : public Err {public: WriteErr() : Err("FileSink: error writing file") {}};

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
