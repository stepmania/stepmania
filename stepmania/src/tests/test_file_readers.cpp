#include "global.h"
#include "RageLog.h"
#include "RageFile.h"
#include "RageUtil.h"
#include "RageUtil_FileDB.h"
#include "RageFileManager.h"

#include <unistd.h>

bool CreateTestFiles = false;

void CreateBinaryTestFile( FILE *out, int size )
{
	char c = 0;
	while( size-- )
	{
		fprintf( out, "%c", c );
		++c;
	}
}

void CreateBinaryTestFile( CString fn, int size )
{
	FILE *out = fopen( fn, "w+b" );
	CreateBinaryTestFile( out, size );
	fclose( out );

	RageFile f(fn);
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
		FILE *out = fopen( filename, "w+b" );

		for( unsigned line = 0; line < NumLines; ++line )
		{
			const CString TestLine = MakeTextTestLine( line, LineSize );
			fprintf( out, "%s", TestLine.c_str() );
			if( DOS )
				fprintf( out, "\r" );
			fprintf( out, "\n" );
		}
		
		/* Write a binary test block at the end. */
		CreateBinaryTestFile( out, 4096 );

		fclose( out );
		out = NULL;
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
	const CString junk = "XXXXXXXX";
	if( CreateTestFiles )
	{
		/* Output a line of text, followed by some junk, followed by binary
		 * test data. */
		FILE *out = fopen( "test.seek", "w+b" );
		fprintf( out, "%s", TestLine.c_str() );
		fprintf( out, "%s", junk.c_str() );
		CreateBinaryTestFile( out, 1024 );
		fclose( out );
		return;
	}
		
	/* Read the line of text.  This will result in some of the junk being
	 * read into the buffer. */
	RageFile test;
	test.Open("test.seek");
	CString line;
	test.GetLine( line );

	/* Seek to the binary test data. */
	int ret;
	if( relative )
		ret = test.SeekCur( junk.size() );
	else
		ret = test.Seek( TestLine.size()+junk.size() );

	if( ret != int(TestLine.size()+junk.size()) )
	{
		LOG->Warn( "%s(%i) failed: got %i", relative? "SeekCur":"Seek", TestLine.size()+junk.size(), ret );
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

	test.Seek( 5 );
	if( test.AtEOF() )
	{
		LOG->Warn( "AtEOF returned true, expected false");
		return;
	}
}



int main( int argc, char *argv[] )
{
	CString Driver = "dir", Root = ".";
	while( 1 )
	{
		int opt = getopt( argc, argv, "cd:r:h" );
		if( opt == -1 )
			break;
		switch( opt )
		{
		case 'c':
			CreateTestFiles = true;
			break;
		case 'd':
			Driver = optarg;
			break;
		case 'r':
			Root = optarg;
			break;

		case 'h':
			printf( "options:\n" );
			printf( " -c: create test files\n" );
			printf( " -d driver: choose file driver to test (default \"Dir\")\n" );
			printf( " -r root: set file driver root (default \".\")\n" );
			exit(1);
		}

	}

	FILEMAN = new RageFileManager( argv[0] );
	FILEMAN->Mount( Driver, Root, "" );

	LOG			= new RageLog();
	LOG->SetLogToDisk( false );
	LOG->SetShowLogOutput( true );
	LOG->SetFlushing( true );

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

	exit(0);
}

