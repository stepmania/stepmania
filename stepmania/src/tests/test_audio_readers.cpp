#include "global.h"
#include "RageLog.h"
#include "RageSoundReader_MP3.h"
       #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>
#include "RageTimer.h"

#include "RageInputDevice.h"

void ReadData( SoundReader *snd,
	       int ms,		/* start */
	       char *buf,	/* out */
	       int bytes )
{
	if( ms != -1 )
		snd->SetPosition_Accurate( ms );
	int got = snd->Read( buf, bytes );
	ASSERT( got == bytes );
}
	       
void find( const char *haystack, int hs, const char *needle, int ns )
{
	for( int i = 0; i <= hs-ns; ++i )
	{
		if( !memcmp(haystack+i, needle, ns) )
		{
			printf("xx %i\n", i);
			return;
		}
	}
}

void dump( const char *fn, const char *buf, int size )
{
	int fd = open( "out.raw", O_WRONLY|O_CREAT|O_TRUNC );
	ASSERT( fd != -1 );
	write( fd, buf, size );
	close( fd );
}

void dump( const char *buf, int size )
{
	for( int i = 0; i < size; ++i )
		printf("%-2.2x ", ((unsigned char*)buf)[i]);
	printf("\n\n");
}

bool test_read( SoundReader *snd, const char *expected_data, int bytes )
{
	bytes = (bytes/4) * 4;
	char buf[bytes];
	int got = snd->Read( buf, bytes );
	ASSERT( got == bytes );

	return !memcmp(expected_data, buf, bytes);
}


bool test_file2( const char *fn )
{
	CString error;
	SoundReader *snd = SoundReader_FileReader::OpenFile( fn, error );
	
	if( snd == NULL )
	{
		LOG->Trace( "File '%s' failed to open: %s", fn, error.c_str() );
		return false;
	}

	float len = snd->GetLength();
	printf("%f\n", len);

	delete snd;
	return true;
}

bool must_be_eof( SoundReader *snd )
{
	char buf[4];
	int got = snd->Read( buf, 4 );
	return got == 0;
}


bool test_file( const char *fn, float expected_fix )
{
	LOG->Trace("Testing: %s", fn );
	CString error;
	SoundReader *s = SoundReader_FileReader::OpenFile( fn, error );
	
	if( s == NULL )
	{
		LOG->Trace( "File '%s' failed to open: %s", fn, error.c_str() );
		return false;
	}
	SoundReader *snd = s->Copy();
	delete s;

	{
		/* The fix value shouldn't change much; give a leeway of about 2ms. */
		const float Fix = snd->GetOffsetFix();
		const float Error = fabs(Fix-expected_fix);
		if( Error > 0.02f )
			LOG->Warn( "Fix error too high (%f, exp %f)", Fix, expected_fix );
	}

	/* Read the first second of the file.  Do this without calling any
	 * seek functions. */
	const int one_second=snd->GetSampleRate()*2*2;
	char data[one_second];
	memset( data, 0x42, sizeof(data) );
	ReadData( snd, -1, data, sizeof(data) );

	/* Make sure we're getting non-null data. */
	{
		bool all_null=true;
		bool all_42=true;

		for( unsigned i = 0; i < sizeof(data); ++i )
		{
			if( data[i] != 0 )
				all_null=false;
			if( data[i] != 0x42 )
				all_42=false;

		}
		
		if( all_null || all_42 )
		{
			LOG->Warn( "'%s': sanity check failed (%i %i)", fn, all_null, all_42 );
			return false;
		}
	}

	/* Read to EOF, discarding the data. */
	while(1)
	{
		char buf[4096];
		int got = snd->Read( buf, sizeof(buf) );
		if( got < int(sizeof(buf)) )
			break;
	}
	
	/* Now, make sure reading after an EOF returns another EOF. */
	if( !must_be_eof(snd) )
	{
		LOG->Warn("Fail: Reading past EOF didn't EOF");
		return false;
	}

	if( !must_be_eof(snd) )
	{
		LOG->Warn("Fail: Reading past EOF twice didn't EOF");
		return false;
	}
	


	/* SetPosition_Accurate(0) must always reset properly. */
	snd->SetPosition_Accurate( 0 );
	if( !test_read( snd, data, one_second ) )
	{
		LOG->Warn("Fail: rewind didn't work");
		return false;
	}

	/* SetPosition_Fast(0) must always reset properly.   */
	snd->SetPosition_Fast( 0 );
	if( !test_read( snd, data, one_second ) )
	{
		LOG->Warn("Fail: rewind didn't work");
		return false;
	}

	/* Seek to 1ms and make sure it gives us the correct data. */
	snd->SetPosition_Accurate( 1 );
	if( !test_read( snd, data + one_second * 1/1000, one_second * 1/1000 ) )
	{
		LOG->Warn("Fail: seek(1) didn't work");
		return false;
	}

	/* Seek to 500ms and make sure it gives us the correct data. */
	snd->SetPosition_Accurate( 500 );
	if( !test_read( snd, data+one_second * 500/1000, one_second * 500/1000 ) )
	{
		LOG->Warn("Fail: seek(500) didn't work");
		return false;
	}

	/* Make sure seeking past end of file returns 0. */
	int ret2 = snd->SetPosition_Fast( 10000000 );
	if( ret2 != 0 )
	{
		LOG->Warn( "Fail: SetPosition_Fast(1000000) returned %i instead of 0", ret2 );
		return false;
	}

	/* Make sure that reading after a seek past EOF returns EOF. */
	if( !must_be_eof(snd) )
	{
		LOG->Warn("Fail: SetPosition_Fast EOF didn't EOF");
		return false;
	}

	ret2 = snd->SetPosition_Accurate( 10000000 );
	if( ret2 != 0 )
	{
		LOG->Warn( "Fail: SetPosition_Accurate(1000000) returned %i instead of 0", ret2 );
		return false;
	}

	if( !must_be_eof(snd) )
	{
		LOG->Warn("Fail: SetPosition_Fast EOF didn't EOF");
		return false;
	}
	
	/* Seek to a spot that lies exactly on a TOC entry, to check the
	 * case that SetPosition_hard doesn't actually have to do anything. */

	/* Check SetPosition_Accurate consistency:
	 * 
	 * Reopen the file from scratch, seek to 100ms, read some data, do some
	 * operations that would result in the internal TOC being filled (seek
	 * to the end), then re-read the data at 100ms and make sure it's the same. */
	
	delete snd;
	snd = SoundReader_FileReader::OpenFile( fn, error );
	
	if( snd == NULL )
	{
		LOG->Trace( "File '%s' failed to open: %s", fn, error.c_str() );
		return false;
	}

	snd->SetPosition_Accurate( 100 );
	ReadData( snd, 100, data, one_second/10 );

	snd->SetPosition_Accurate( 10000 );
	snd->SetPosition_Accurate( 100 );
	if( !test_read( snd, data, one_second/10 ) )
	{
		LOG->Warn("Fail: rewind didn't work");
		return false;
	}

	return true;
}
int main()
{
	LOG			= new RageLog();
	struct {
		const char *fn;
		float expected_fix;
	} files[] = {
		{ "afronoia.wav",				0 },
		{ "test.wav",					0 },
		{ "test.ogg",					0 },
		{ "first frame corrupt.mp3",	0.052f },
		{ "LAME cbr.mp3",				0 },
		{ "LAME vbr with xing tag.mp3",	0 },
		{ "LAME cbr with id3v2 tag.mp3", 0 },
		{ "LAME vbr with xing and id3v2 tag.mp3",	0 },
		{ NULL,							0 }
	};
	
	for( int i = 0; files[i].fn; ++i )
	{
		if( !test_file( files[i].fn, files[i].expected_fix ) )
			exit(1);
		test_file2( files[i].fn );
	}

	exit(0);
}

