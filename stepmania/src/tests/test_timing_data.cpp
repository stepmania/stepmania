#include "global.h"
#include "RageLog.h"
#include "RageFile.h"
#include "RageUtil.h"
#include "RageUtil_FileDB.h"
#include "PrefsManager.h"
#include "RageFileManager.h"
#include "TimingData.h"

void run()
{
#define CHECK(call, exp) \
{ \
	float ret = call; \
	if( call != exp ) { \
		LOG->Warn( "Line %i: Got %f, expected %f", __LINE__, ret, exp); \
		return; \
	} \
}

	TimingData test;
	test.AddBPMSegment( BPMSegment(0, 60) );
	
	/* First, trivial sanity checks. */
	CHECK( test.GetBeatFromElapsedTime(60), 60.0f );
	CHECK( test.GetElapsedTimeFromBeat(60), 60.0f );

	/* The first BPM segment extends backwards in time. */
	CHECK( test.GetBeatFromElapsedTime(-60), -60.0f );
	CHECK( test.GetElapsedTimeFromBeat(-60), -60.0f );
	
	CHECK( test.GetBeatFromElapsedTime(100000), 100000.0f );
	CHECK( test.GetElapsedTimeFromBeat(100000), 100000.0f );
	CHECK( test.GetBeatFromElapsedTime(-100000), -100000.0f );
	CHECK( test.GetElapsedTimeFromBeat(-100000), -100000.0f );
	
	CHECK( test.GetBPMAtBeat(0), 60.0f );
	CHECK( test.GetBPMAtBeat(100000), 60.0f );
	CHECK( test.GetBPMAtBeat(-100000), 60.0f );

	/* 120BPM at beat 10: */
	test.AddBPMSegment( BPMSegment(10, 120) );
	CHECK( test.GetBPMAtBeat(9.99), 60.0f );
	CHECK( test.GetBPMAtBeat(10), 120.0f );

	CHECK( test.GetBeatFromElapsedTime(9), 9.0f );
	CHECK( test.GetBeatFromElapsedTime(10), 10.0f );
	CHECK( test.GetBeatFromElapsedTime(10.5), 11.0f );

	CHECK( test.GetElapsedTimeFromBeat(9), 9.0f );
	CHECK( test.GetElapsedTimeFromBeat(10), 10.0f );
	CHECK( test.GetElapsedTimeFromBeat(11), 10.5f );

	/* Add a 5-second stop at beat 10. */
	test.AddStopSegment( StopSegment(10, 5) );

	/* The stop shouldn't affect GetBPMAtBeat at all. */
	CHECK( test.GetBPMAtBeat(9.99), 60.0f );
	CHECK( test.GetBPMAtBeat(10), 120.0f );

	CHECK( test.GetBeatFromElapsedTime(9), 9.0f );
	CHECK( test.GetBeatFromElapsedTime(10), 10.0f );
	CHECK( test.GetBeatFromElapsedTime(12), 10.0f );
	CHECK( test.GetBeatFromElapsedTime(14), 10.0f );
	CHECK( test.GetBeatFromElapsedTime(15), 10.0f );
	CHECK( test.GetBeatFromElapsedTime(15.5), 11.0f );
	
	CHECK( test.GetElapsedTimeFromBeat(9), 9.0f );
	CHECK( test.GetElapsedTimeFromBeat(10), 10.0f );
	CHECK( test.GetElapsedTimeFromBeat(11), 15.5f );

	/* Add a 2-second stop at beat 5 and a 5-second stop at beat 15. */
	test.m_StopSegments.clear();
	test.AddStopSegment( StopSegment(5, 2) );
	test.AddStopSegment( StopSegment(15, 5) );
	CHECK( test.GetBPMAtBeat(9.99), 60.0f );
	CHECK( test.GetBPMAtBeat(10), 120.0f );

	CHECK( test.GetBeatFromElapsedTime(1), 1.0f );
	CHECK( test.GetBeatFromElapsedTime(2), 2.0f );
	CHECK( test.GetBeatFromElapsedTime(5), 5.0f ); // stopped
	CHECK( test.GetBeatFromElapsedTime(6), 5.0f ); // stopped
	CHECK( test.GetBeatFromElapsedTime(7), 5.0f ); // stop finished
	CHECK( test.GetBeatFromElapsedTime(8), 6.0f );
	CHECK( test.GetBeatFromElapsedTime(12), 10.0f ); // bpm changes to 120
	CHECK( test.GetBeatFromElapsedTime(13), 12.0f );
	CHECK( test.GetBeatFromElapsedTime(14), 14.0f );
	CHECK( test.GetBeatFromElapsedTime(14.5f), 15.0f ); // stopped
	CHECK( test.GetBeatFromElapsedTime(15), 15.0f ); // stopped
	CHECK( test.GetBeatFromElapsedTime(17), 15.0f ); // stopped
	CHECK( test.GetBeatFromElapsedTime(19.5f), 15.0f ); // stop finished
	CHECK( test.GetBeatFromElapsedTime(20), 16.0f );

	CHECK( test.GetElapsedTimeFromBeat(1), 1.0f );
	CHECK( test.GetElapsedTimeFromBeat(2), 2.0f );
	CHECK( test.GetElapsedTimeFromBeat(5), 5.0f ); // stopped
	CHECK( test.GetElapsedTimeFromBeat(6), 8.0f );
	CHECK( test.GetElapsedTimeFromBeat(10), 12.0f ); // bpm changes to 120
	CHECK( test.GetElapsedTimeFromBeat(12), 13.0f );
	CHECK( test.GetElapsedTimeFromBeat(14), 14.0f );
	CHECK( test.GetElapsedTimeFromBeat(15.0f), 14.5f ); // stopped
	CHECK( test.GetElapsedTimeFromBeat(16), 20.0f );

RageTimer foobar;
	/* We can look up the time of any given beat, then look up the beat of that
	 * time and get the original value.  (We can't do this in reverse; the beat
	 * doesn't move during stop segments.) */
int q = 0;
	for( float f = -10; f < 250; f += 0.002 )
	{
		++q;
//		const float t = test.GetElapsedTimeFromBeat( f );
		const float b = test.GetBeatFromElapsedTime( f );

		/* b == f */
	
//		if( fabsf(b-f) > 0.001 )
//		{
//			LOG->Warn( "%f != %f", b, f );
//			return;
//		}
	}
LOG->Trace("... %i in %f", q, foobar.GetDeltaTime());

	TimingData test2;
	test2.AddBPMSegment( BPMSegment(0, 60) );
	test2.AddStopSegment( StopSegment(0, 1) );
	CHECK( test2.GetBeatFromElapsedTime(-1), -1.0f );
	CHECK( test2.GetBeatFromElapsedTime(0), 0.0f );
	CHECK( test2.GetBeatFromElapsedTime(1), 0.0f );
	CHECK( test2.GetBeatFromElapsedTime(2), 1.0f );
	CHECK( test2.GetElapsedTimeFromBeat(-1), -1.0f );
	CHECK( test2.GetElapsedTimeFromBeat(0), 0.0f );
	CHECK( test2.GetElapsedTimeFromBeat(1), 2.0f );
	CHECK( test2.GetElapsedTimeFromBeat(2), 3.0f );
}

int main( int argc, char *argv[] )
{
	FILEMAN			= new RageFileManager( argv[0] );
	FILEMAN->Mount( "dir", ".", "" );
	LOG                     = new RageLog();
	PREFSMAN		= new PrefsManager; // TimingData needs PREFSMAN; it probably shouldn't
	LOG->SetShowLogOutput( true );
	LOG->SetFlushing( true );

	run();
	
	delete PREFSMAN;
	delete LOG;
	delete FILEMAN;

	exit(0);
}
