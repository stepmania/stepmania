#include "global.h"
#include "RageLog.h"
#include "RageFile.h"
#include "RageUtil.h"
#include "RageUtil_FileDB.h"
#include "test_misc.h"

bool CreateTestFiles = false;

void CreateBinaryTestFile( RageFile &f, int size )
{
	char c = 0;
	while( size-- )
	{
		f.Write( ssprintf("%c", c) );
		++c;
	}
}

void CreateBinaryTestFile( CString fn, int size )
{
	RageFile f;
	f.Open( fn, RageFile::WRITE );
	CreateBinaryTestFile( f, size );
	f.Close();

	if( !f.Open(fn) )
	{
		LOG->Warn("CreateBinaryTestFile: error reopening %s: %s", fn.c_str(), f.GetError().c_str() );
		return;
	}

	int test = f.GetFileSize();
	if( test != size )
		LOG->Warn("CreateBinaryTestFile: f.GetFileSize ret %i, exp %i", test, size );
}


bool CheckData( const CString &buf, int offset )
{
	char c = offset%256;
	
	for( unsigned i = 0; i < buf.size(); ++i )
	{
		if( buf[i] != c )
			return false;
		++c;
	}
	return true;
}

int TestBinaryRead( RageFile &test, int size, bool AllowEOF, int ExpectedFilePos, int TestDataPos )
{
	char buf[4096];
	ASSERT( size <= (int) sizeof(buf) );
	int ret = test.Read( buf, size );
	if( ret != size )
	{
		if( AllowEOF )
			return 0;
		LOG->Warn("FAIL");
		return -1;
	}

	if( !CheckData( CString(buf, ret), TestDataPos ) )
	{
		LOG->Warn("TestBinaryRead: check failed (size %i, %i,%i)",
				size, ExpectedFilePos, TestDataPos );
		return -1;
	}

	if( test.Tell() != ExpectedFilePos+ret )
	{
		LOG->Warn( "Simple Tell() failed (got %i, expected %i)", test.Tell(), ret );
		return -1;
	}
	return 1;
}

CString MakeTextTestLine( int line, int size )
{
	CString ret = ssprintf( "%02i", line );
	ret.append( CString( size-ret.size(), 'x' ) );
	return ret;
}

bool CheckTextData( const CString &buf, int LineSize, int size )
{
	return buf == MakeTextTestLine( LineSize, size );
}

void TestText( int LineSize, bool DOS )
{
	const unsigned NumLines = 50;
	
	CString filename = "test.text";
	if( DOS )
		filename += ".DOS";
	filename += ssprintf( ".%i", LineSize );
	if( CreateTestFiles )
	{
		RageFile f;
		f.Open( filename, RageFile::WRITE );

		for( unsigned line = 0; line < NumLines; ++line )
		{
			const CString TestLine = MakeTextTestLine( line, LineSize );
			f.Write( TestLine );
			if( DOS )
				f.Write( "\r" );
			f.Write( "\n" );
		}
		
		/* Write a binary test block at the end. */
		CreateBinaryTestFile( f, 4096 );

		/* Write a text line, with no NL. */
		f.Write( "final" );

		return;
	}

	RageFile test;
	if( !test.Open(filename) )
	{
		LOG->Warn( "Open \"%s\" failed: %s", filename.c_str(), test.GetError().c_str() );
		exit(1);
	}
		
	int ExpectedPos = 0;
	for( unsigned line = 0; line < NumLines; ++line )
	{
		CString buf;
		if( test.GetLine( buf ) <= 0 )
		{
			LOG->Warn("Text read failure, line %i, size %i, DOS %i",
					line, LineSize, DOS );
			return;
		}

		/* If we successfully got data, we should not be at EOF. */
		if( test.AtEOF() )
		{
			LOG->Warn("Unexpected EOF in text read, line %i, size %i, DOS %i",
					line, LineSize, DOS );
			return;
		}

		if( !CheckTextData( buf, line, LineSize ) )
		{
			LOG->Warn("Text read check failure, line %i, size %i, DOS %i: '%s'",
					line, LineSize, DOS, buf.c_str() );
			return;
		}
		ExpectedPos += buf.size()+1;
		if( DOS )
			++ExpectedPos;
	}
	if( ExpectedPos != test.Tell() )
	{
		LOG->Warn("Text read pos failure, size %i, DOS %i: exp %i, got %i",
				LineSize, DOS, ExpectedPos, test.Tell() );
		return;
	}

	/* Test the binary block.  Do it in a couple reads, to force it to read from
	 * the buffer and the file more than once. */
	TestBinaryRead( test, 1024, true, test.Tell(), 1024*0 );
	TestBinaryRead( test, 1024, true, test.Tell(), 1024*1 );
	TestBinaryRead( test, 1024, true, test.Tell(), 1024*2 );
	TestBinaryRead( test, 1024, true, test.Tell(), 1024*3 );

	{
		CString buf;
		if( test.GetLine( buf ) <= 0 )
		{
			LOG->Warn( "Unexpected EOF in final line read, DOS %i", DOS );
			return;
		}

		if( buf != "final" )
		{
			LOG->Warn( "Final text read check failure, DOS %i: '%s'",
					DOS, buf.c_str() );
			return;
		}
		if( test.AtEOF() )
		{
			LOG->Warn( "Unexpected EOF (2) in final text read, DOS %i", DOS );
			return;
		}
	}

	{
		CString buf;
		if( test.GetLine( buf ) != 0 )
		{
			LOG->Warn( "Expected EOF in final text read, but didn't get it" );
			return;
		}

		if( !test.AtEOF() )
		{
			LOG->Warn( "Expected EOF (2) in final text read, but didn't get it" );
			return;
		}
	}
}

void TestText()
{
	/* Test with these line sizes.  The internal buffer size is 256, so test line sizes
	 * around there carefully. */
	int sizes[] = {
		4,
		7,
		100,
		200,
		254,
		255,
		256,
		257,
		258,
		500,
		-1
	};

	for( int s = 0; sizes[s] != -1; ++s )
	{
		TestText( sizes[s], false );
		TestText( sizes[s], true );
	}
}

void TestSeek( bool relative )
{
	const CString TestLine = "Hello World\n";
	/* Print a couple kb of text.  Make sure this is big enough that the binary
	 * test data after it won't start in the buffer; if that happens, the seek
	 * will be optimized out. */
	CString junk;
	for( int lines = 0; lines < 128; ++lines )
		junk += "XXXXXXXXX\n";

	if( CreateTestFiles )
	{
		/* Output a line of text, followed by some junk, followed by binary
		 * test data. */
		RageFile f;
		f.Open( "test.seek", RageFile::WRITE );
		f.Write( TestLine );
		f.Write( junk );
		CreateBinaryTestFile( f, 1024 );
		return;
	}
		
	/* Read the line of text.  This will result in some of the junk being
	 * read into the buffer. */
	RageFile test;
	test.Open("test.seek");
	CString line;
	test.GetLine( line );

	/* Run the absolute seek test twice. */
	for( int pass = 0; pass < 2; ++pass )
	{
		/* Seek to the binary test data. */
		if( relative && pass > 0 )
			break;

		int ret;
		if( relative )
			ret = test.Seek( test.Tell() + junk.size() );
		else
			ret = test.Seek( TestLine.size()+junk.size() );

		if( ret != int(TestLine.size()+junk.size()) )
		{
			LOG->Warn( "Seek(%i) failed: got %i", TestLine.size()+junk.size(), ret );
			return;
		}

		/* Check the binary test data (making sure we don't get any junk). */
		TestBinaryRead( test, 256, false, test.Tell(), 256*0 );
		TestBinaryRead( test, 256, false, test.Tell(), 256*1 );
		TestBinaryRead( test, 256, false, test.Tell(), 256*2 );
		TestBinaryRead( test, 256, false, test.Tell(), 256*3 );

		/* Check EOF. */
		char c;
		ret = test.Read( &c, 1 );
		if( ret != 0 )
		{
			LOG->Warn( "Read(1) failed: got %i, expected 0", ret );
			return;
		}
		if( !test.AtEOF() )
		{
			LOG->Warn( "AtEOF returned false, expected true");
			return;
		}
	}

	test.Seek( 5 );
	if( test.AtEOF() )
	{
		LOG->Warn( "AtEOF returned true, expected false");
		return;
	}

	/* Make sure a read after seeking far past EOF returns 0. */
	test.Seek( 9999999 );
	char c;
	int ret = test.Read( &c, 1 );
	if( ret != 0 )
	{
		LOG->Warn( "Read(1) failed: got %i, expected 0", ret );
		return;
	}
	if( !test.AtEOF() )
	{
		LOG->Warn( "AtEOF returned false, expected true");
		return;
	}
}



#include "RageFileDriverMemory.h"
#include "RageFileDriverDeflate.h"
void CheckDeflate( RageFileObjMem &data, int iSize, unsigned iInputCRC, int iBlockSize )
{
	data.Seek(0);

	CString foo2;

	RageFileObjInflate infl( &data, iSize );
	infl.EnableCRC32();
	int iGot = 0;
	while( iGot < iSize )
	{
		int iRet = infl.Read( foo2, iBlockSize );
		if( iRet == 0 )
		{
			LOG->Warn( "Premature EOF (iSize %i, iBlockSize %i)", iSize, iBlockSize );
			return;
		}
			
		iGot += iRet;
	}

	unsigned iOutputCRC;
	if( !infl.GetCRC32(&iOutputCRC) )
	{
		LOG->Warn( "Inflate GetCRC32 failed" );
		return;
	}
	if( iInputCRC != iOutputCRC )
	{
		LOG->Warn( "Expected CRC %08x, got %08x", iInputCRC, iOutputCRC );
		return;
	}

	if( iSize != iGot )
	{
		LOG->Warn( "Expected %i, got %i", iSize, iGot );
		return;
	}
}

void TestDeflate()
{
	RageFileObjMem data;
	RageFileObjDeflate defl( &data );
	defl.EnableCRC32();

	int iSize = 0;
	while( iSize < 1024*512 )
		iSize += defl.Write("test foo bar fah");
	defl.Flush();

	unsigned iOutputCRC;
	if( !defl.GetCRC32(&iOutputCRC) )
	{
		LOG->Warn( "Deflate GetCRC32 failed" );
		return;
	}

	CheckDeflate( data, iSize, iOutputCRC, 1 );
	CheckDeflate( data, iSize, iOutputCRC, 1024 );
	CheckDeflate( data, iSize, iOutputCRC, 1024*4 );
	CheckDeflate( data, iSize, iOutputCRC, iSize-1 );
	CheckDeflate( data, iSize, iOutputCRC, iSize );
}

int main( int argc, char *argv[] )
{
	test_handle_args( argc, argv );

	optind = 0; // force getopt to reset
	while( 1 )
	{
		opterr = 0;
		int opt = getopt( argc, argv, "c" );
		if( opt == -1 )
			break;
		switch( opt )
		{
		case 'c':
			CreateTestFiles = true;
			break;
		}

	}

	test_init();

	if( GetCommandlineArgument("zlib") )
	{
		TestDeflate();
		test_deinit();
		return 0;
	}
			
	if( CreateTestFiles )
		CreateBinaryTestFile( "test.binary", 4096 );
	else
	{
		/* Test binary reading. */
		RageFile test;
		if( !test.Open("test.binary") )
		{
			LOG->Warn( "Open \"test.binary\" failed: %s", test.GetError().c_str() );
			exit(1);
		}

		TestBinaryRead( test, 256, false, 0, 0 );

		/* Test simple re-opening. */
		test.Close();
		test.Open("test.binary");

		/* We should now be back at 0. */
		if( test.Tell() != 0 )
		{
			LOG->Warn( "Simple Tell() after reopen failed (got %i, expected 0)", test.Tell() );
			exit(1);
		}

		/* Test reading further, with various block sizes. */
		while( 1 )
		{
			if( TestBinaryRead( test, 85, true, test.Tell(), test.Tell() ) == 0 )
				break;
		}
	}

	/* Test text reading. */
	TestText();

	/* Test seeking. */
	TestSeek( true );
	TestSeek( false );

	test_deinit();

	exit(0);
}

