/* TimingData - Holds data for translating beats<->seconds. */

#ifndef TIMING_DATA_H
#define TIMING_DATA_H

#include "NoteTypes.h"
#include "PrefsManager.h"
struct lua_State;

#define COMPARE(x) if(x!=other.x) return false;

struct BPMSegment 
{
	BPMSegment() : m_iStartRow(-1), m_fBPS(-1.0f) { }
	BPMSegment( int s, float b ) { m_iStartRow = max( 0, s ); SetBPM( b ); }
	int m_iStartRow;
	float m_fBPS;

	void SetBPM( float f ) { m_fBPS = f / 60.0f; }
	float GetBPM() const { return m_fBPS * 60.0f; }

	bool operator==( const BPMSegment &other ) const
	{
		COMPARE( m_iStartRow );
		COMPARE( m_fBPS );
		return true;
	}
	bool operator!=( const BPMSegment &other ) const { return !operator==(other); }
	bool operator<( const BPMSegment &other ) const { return m_iStartRow < other.m_iStartRow; }
	bool operator<=( const BPMSegment &other ) const
	{
		return ( operator<(other) || operator==(other) );
	}
	bool operator>( const BPMSegment &other ) const { return m_iStartRow > other.m_iStartRow; }
	bool operator>=( const BPMSegment &other ) const
	{
		return ( operator>(other) || operator==(other) );
	}
};

struct StopSegment 
{
	StopSegment() : m_iStartRow(-1), m_fStopSeconds(-1.0f), m_bDelay(false)  { }
	StopSegment( int s, float f ) {
		m_iStartRow = max( 0, s );
		m_fStopSeconds = PREFSMAN->m_bQuirksMode ? f : max( 0.0f, f );
		m_bDelay = false; // no delay by default
	}
	StopSegment( int s, float f, bool d ) {
		m_iStartRow = max( 0, s );
		m_fStopSeconds = PREFSMAN->m_bQuirksMode ? f : max( 0.0f, f );
		m_bDelay = d;
	}
	int m_iStartRow;
	float m_fStopSeconds;
	bool m_bDelay; // if true, treat this stop as a Pump delay instead

	bool operator==( const StopSegment &other ) const
	{
		COMPARE( m_iStartRow );
		COMPARE( m_fStopSeconds );
		COMPARE( m_bDelay );
		return true;
	}
	bool operator!=( const StopSegment &other ) const { return !operator==(other); }
	// Delays need to come before stops to not render them pointless.
	bool operator<( const StopSegment &other ) const
	{
		return ( m_iStartRow < other.m_iStartRow ) ||
		( m_iStartRow == other.m_iStartRow && m_bDelay && !other.m_bDelay );
	}
	bool operator<=( const StopSegment &other ) const
	{
		return ( operator<(other) || operator==(other) );
	}
	bool operator>( const StopSegment &other ) const
	{
		return ( m_iStartRow > other.m_iStartRow ) ||
		( m_iStartRow == other.m_iStartRow && !m_bDelay && other.m_bDelay );
	}
	bool operator>=( const StopSegment &other ) const
	{
		return ( operator>(other) || operator==(other) );
	}
};

/* This only supports simple time signatures. The upper number (called the numerator here, though this isn't
 * properly a fraction) is the number of beats per measure. The lower number (denominator here)
 * is the note value representing one beat. */
struct TimeSignatureSegment 
{
	TimeSignatureSegment() : m_iStartRow(-1), m_iNumerator(4), m_iDenominator(4)  { }
	TimeSignatureSegment( int r, int n ) {
		m_iStartRow = max( 0, r );
		m_iNumerator = n;
		m_iDenominator = 64; // Hope we don't need this many.
	}
	TimeSignatureSegment( int r, int n, int d ) {
		m_iStartRow = max( 0, r );
		m_iNumerator = n;
		m_iDenominator = d;
	}
	
	int m_iStartRow;
	int m_iNumerator;
	int m_iDenominator;

	/* With BeatToNoteRow(1) rows per beat, then we should have BeatToNoteRow(1)*m_iNumerator
	 * beats per measure. But if we assume that every BeatToNoteRow(1) rows is a quarter note,
	 * and we want the beats to be 1/m_iDenominator notes, then we should have
	 * BeatToNoteRow(1)*4 is rows per whole note and thus BeatToNoteRow(1)*4/m_iDenominator is
	 * rows per beat. Multiplying by m_iNumerator gives rows per measure. */
	int GetNoteRowsPerMeasure() const { return BeatToNoteRow(1) * 4 * m_iNumerator / m_iDenominator; }

	bool operator==( const TimeSignatureSegment &other ) const
	{
		COMPARE( m_iStartRow );
		COMPARE( m_iNumerator );
		COMPARE( m_iDenominator );
		return true;
	}
	bool operator!=( const TimeSignatureSegment &other ) const { return !operator==(other); }
	bool operator<( const TimeSignatureSegment &other ) const { return m_iStartRow < other.m_iStartRow; }
	bool operator<= (const TimeSignatureSegment &other ) const
	{
		return ( operator<(other) || operator==(other) );
	}
	bool operator>( const TimeSignatureSegment &other ) const { return m_iStartRow > other.m_iStartRow; }
	bool operator>= (const TimeSignatureSegment &other ) const
	{
		return ( operator>(other) || operator==(other) );
	}
};

/* A warp segment is used to replicate the effects of Negative BPMs without
 * abusing negative BPMs. Negative BPMs should be converted to warp segments.
 * WarpAt=WarpTo is the format, where both are in beats. (Technically they're
 * both rows though.) */
struct WarpSegment
{
	WarpSegment() : m_iStartRow(-1), m_fWarpBeats(-1) { }
	WarpSegment( int s, float b ){ m_iStartRow = max( 0, s ); m_fWarpBeats = max( 0, b ); }
	int m_iStartRow;
	float m_fWarpBeats;

	bool operator==( const WarpSegment &other ) const
	{
		COMPARE( m_iStartRow );
		COMPARE( m_fWarpBeats );
		return true;
	}
	bool operator!=( const WarpSegment &other ) const { return !operator==(other); }
	bool operator<( const WarpSegment &other ) const { return m_iStartRow < other.m_iStartRow; }
	bool operator<=( const WarpSegment &other ) const
	{
		return ( operator<(other) || operator==(other) );
	}
	bool operator>( const WarpSegment &other ) const { return m_iStartRow > other.m_iStartRow; }
	bool operator>=( const WarpSegment &other ) const
	{
		return ( operator>(other) || operator==(other) );
	}
};

/*
 * A tickcount segment is used to better replicate the checkpoint hold
 * system used by various based video games. The number is used to 
 * represent how many ticks can be counted in one beat.
 */
struct TickcountSegment
{
	TickcountSegment() : m_iStartRow(-1), m_iTicks(2) { }
	TickcountSegment( int s, int t ){ m_iStartRow = max( 0, s ); m_iTicks = max( 1, t ); }
	int m_iStartRow;
	int m_iTicks;
	
	bool operator==( const TickcountSegment &other ) const
	{
		COMPARE( m_iStartRow );
		COMPARE( m_iTicks );
		return true;
	}
	bool operator!=( const TickcountSegment &other ) const { return !operator==(other); }
	bool operator<( const TickcountSegment &other ) const { return m_iStartRow < other.m_iStartRow; }
	bool operator<=( const TickcountSegment &other ) const
	{
		return ( operator<(other) || operator==(other) );
	}
	bool operator>( const TickcountSegment &other ) const { return m_iStartRow > other.m_iStartRow; }
	bool operator>=( const TickcountSegment &other ) const
	{
		return ( operator>(other) || operator==(other) );
	}
};

class TimingData
{
public:
	TimingData();

	void GetActualBPM( float &fMinBPMOut, float &fMaxBPMOut ) const;
	float GetBPMAtBeat( float fBeat ) const;
	void SetBPMAtRow( int iNoteRow, float fBPM );
	void SetBPMAtBeat( float fBeat, float fBPM ) { SetBPMAtRow( BeatToNoteRow(fBeat), fBPM ); }
	void SetStopAtRow( int iNoteRow, float fSeconds );
	void SetStopAtRow( int iNoteRow, float fSeconds, bool bDelay ); // (sm-ssc)
	void SetStopAtBeat( float fBeat, float fSeconds ) { SetStopAtRow( BeatToNoteRow(fBeat), fSeconds ); }
	void SetStopAtBeat( float fBeat, float fSeconds, bool bDelay ) { SetStopAtRow( BeatToNoteRow(fBeat), fSeconds, bDelay ); } // (sm-ssc)
	float GetStopAtRow( int iNoteRow, bool &bDelayOut ) const;
	float GetStopAtRow( int iRow ) const;
	float GetStopAtBeat( float fBeat ) const { return GetStopAtRow( BeatToNoteRow(fBeat) ); }
	float GetDelayAtRow( int iRow ) const;
	float GetDelayAtBeat( float fBeat ) const { return GetDelayAtRow( BeatToNoteRow(fBeat) ); }
	void SetTimeSignatureAtRow( int iRow, int iNumerator, int iDenominator );
	void SetTimeSignatureAtBeat( float fBeat, int iNumerator, int iDenominator ) { SetTimeSignatureAtRow( BeatToNoteRow(fBeat), iNumerator, iDenominator ); }
	void SetTimeSignatureNumeratorAtRow( int iRow, int iNumerator );
	void SetTimeSignatureNumeratorAtBeat( float fBeat, int iNumerator ) { SetTimeSignatureNumeratorAtRow( BeatToNoteRow(fBeat), iNumerator); }
	void SetTimeSignatureDenominatorAtRow( int iRow, int iDenominator );
	void SetTimeSignatureDenominatorAtBeat( float fBeat, int iDenominator ) { SetTimeSignatureDenominatorAtRow( BeatToNoteRow(fBeat), iDenominator); }
	int GetWarpToRow( int iWarpBeginRow ) const;
	void SetDelayAtRow( int iNoteRow, float fSeconds ); // sm-ssc
	void SetTickcountAtRow( int iRow, int iTicks );
	void SetTickcountAtBeat( float fBeat, int iTicks ) { SetTickcountAtRow( BeatToNoteRow( fBeat ), iTicks ); }
	int GetTickcountAtRow( int iRow ) const;
	int GetTickcountAtBeat( float fBeat ) const { return GetTickcountAtRow( BeatToNoteRow(fBeat) ); }
	void MultiplyBPMInBeatRange( int iStartIndex, int iEndIndex, float fFactor );
	void AddBPMSegment( const BPMSegment &seg );
	void AddStopSegment( const StopSegment &seg );
	void AddTimeSignatureSegment( const TimeSignatureSegment &seg );
	void AddWarpSegment( const WarpSegment &seg );
	void AddTickcountSegment( const TickcountSegment &seg );
	int GetBPMSegmentIndexAtBeat( float fBeat ) const;
	int GetTimeSignatureSegmentIndexAtRow( int iRow ) const;
	int GetTimeSignatureSegmentIndexAtBeat( float fBeat ) const { return GetTimeSignatureSegmentIndexAtRow( BeatToNoteRow(fBeat) ); }
	TimeSignatureSegment& GetTimeSignatureSegmentAtRow( int iRow );
	TimeSignatureSegment& GetTimeSignatureSegmentAtBeat( float fBeat ) { return GetTimeSignatureSegmentAtRow( BeatToNoteRow(fBeat) ); }
	int GetTimeSignatureNumeratorAtRow( int iRow );
	int GetTimeSignatureNumeratorAtBeat( float fBeat ) { return GetTimeSignatureNumeratorAtRow( BeatToNoteRow(fBeat) ); }
	int GetTimeSignatureDenominatorAtRow( int iRow );
 	int GetTimeSignatureDenominatorAtBeat( float fBeat ) { return GetTimeSignatureDenominatorAtRow( BeatToNoteRow(fBeat) ); }
	BPMSegment& GetBPMSegmentAtBeat( float fBeat );
	int GetTickcountSegmentIndexAtRow( int iRow ) const;
	int GetTickcountSegmentIndexAtBeat( float fBeat ) const { return GetTickcountSegmentIndexAtRow( BeatToNoteRow(fBeat) ); }
	TickcountSegment& GetTickcountSegmentAtRow( int iRow );
	TickcountSegment& GetTickcountSegmentAtBeat( float fBeat ) { return GetTickcountSegmentAtRow( BeatToNoteRow(fBeat) ); }
	void NoteRowToMeasureAndBeat( int iNoteRow, int &iMeasureIndexOut, int &iBeatIndexOut, int &iRowsRemainder ) const;

	void GetBeatAndBPSFromElapsedTime( float fElapsedTime, float &fBeatOut, float &fBPSOut, bool &bFreezeOut, bool &bDelayOut, int &iWarpBeginOut, float &fWarpLengthOut ) const;
	float GetBeatFromElapsedTime( float fElapsedTime ) const	// shortcut for places that care only about the beat
	{
		float fBeat, fThrowAway, fThrowAway2;
		bool bThrowAway, bThrowAway2;
		int iThrowAway;
		GetBeatAndBPSFromElapsedTime( fElapsedTime, fBeat, fThrowAway, bThrowAway, bThrowAway2, iThrowAway, fThrowAway2 );
		return fBeat;
	}
	float GetElapsedTimeFromBeat( float fBeat ) const;

	void GetBeatAndBPSFromElapsedTimeNoOffset( float fElapsedTime, float &fBeatOut, float &fBPSOut, bool &bFreezeOut, bool &bDelayOut, int &iWarpBeginOut, float &iWarpLengthOut ) const;
	float GetBeatFromElapsedTimeNoOffset( float fElapsedTime ) const	// shortcut for places that care only about the beat
	{
		float fBeat, fThrowAway, fThrowAway2;
		bool bThrowAway, bThrowAway2;
		int iThrowAway;
		GetBeatAndBPSFromElapsedTimeNoOffset( fElapsedTime, fBeat, fThrowAway, bThrowAway, bThrowAway2, iThrowAway, fThrowAway2 );
		return fBeat;
	}
	float GetElapsedTimeFromBeatNoOffset( float fBeat ) const;

	bool HasBpmChanges() const;
	bool HasStops() const;
	bool HasWarps() const;

	bool operator==( const TimingData &other )
	{
		COMPARE( m_BPMSegments.size() );
		for( unsigned i=0; i<m_BPMSegments.size(); i++ )
			COMPARE( m_BPMSegments[i] );
		COMPARE( m_StopSegments.size() );
		for( unsigned i=0; i<m_StopSegments.size(); i++ )
			COMPARE( m_StopSegments[i] );
		COMPARE( m_WarpSegments.size() );
		for( unsigned i=0; i<m_WarpSegments.size(); i++ )
			COMPARE( m_WarpSegments[i] );
		COMPARE( m_vTimeSignatureSegments.size() );
		for( unsigned i=0; i<m_vTimeSignatureSegments.size(); i++)
			COMPARE( m_vTimeSignatureSegments[i] );
		COMPARE( m_TickcountSegments.size() );
		for( unsigned i=0; i<m_TickcountSegments.size(); i++ )
			COMPARE( m_TickcountSegments[i] );
		COMPARE( m_fBeat0OffsetInSeconds );
		return true;
	}
	bool operator!=( const TimingData &other ) { return !operator==(other); }

	void ScaleRegion( float fScale = 1, int iStartRow = 0, int iEndRow = MAX_NOTE_ROW );
	void InsertRows( int iStartRow, int iRowsToAdd );
	void DeleteRows( int iStartRow, int iRowsToDelete );

	// Lua
	void PushSelf( lua_State *L );

	RString					m_sFile;		// informational only
	// All of the following vectors must be sorted before gameplay.
	vector<BPMSegment>		m_BPMSegments;
	vector<StopSegment>		m_StopSegments;
	vector<TimeSignatureSegment>	m_vTimeSignatureSegments;
	vector<WarpSegment>		m_WarpSegments;
	vector<TickcountSegment>	m_TickcountSegments;
	float	m_fBeat0OffsetInSeconds;
	bool	m_bHasNegativeBpms; // only used for Lua bindings in Song (to be moved to TimingData later)
};

#undef COMPARE

#endif

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
