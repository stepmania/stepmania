/* TimingData - Holds data for translating beats<->seconds. */

#ifndef TIMING_DATA_H
#define TIMING_DATA_H

#include "NoteTypes.h"

#define COMPARE(x) if(x!=other.x) return false;

struct BPMSegment 
{
	BPMSegment() { m_iStartIndex = -1; m_fBPS = -1; }
	BPMSegment( int s, float b ) { m_iStartIndex = s; m_fBPS = b/60.0f; }
	int m_iStartIndex;
	float m_fBPS;

	void SetBPM( float f );
	float GetBPM() const;

	bool operator==( const BPMSegment &other ) const
	{
		COMPARE( m_iStartIndex );
		COMPARE( m_fBPS );
		return true;
	}
	bool operator!=( const BPMSegment &other ) const { return !operator==(other); }
};

struct StopSegment 
{
	StopSegment() { m_fStopSeconds = -1; m_iStartRow = -1; }
	StopSegment( int s, float f ) { m_iStartRow = s; m_fStopSeconds = f; }
	int m_iStartRow;
	float m_fStopSeconds;

	bool operator==( const StopSegment &other ) const
	{
		COMPARE( m_iStartRow );
		COMPARE( m_fStopSeconds );
		return true;
	}
	bool operator!=( const StopSegment &other ) const { return !operator==(other); }
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
	void SetStopAtBeat( float fBeat, float fSeconds ) { SetStopAtRow( BeatToNoteRow(fBeat), fSeconds ); }
	void MultiplyBPMInBeatRange( int iStartIndex, int iEndIndex, float fFactor );
	void AddBPMSegment( const BPMSegment &seg );
	void AddStopSegment( const StopSegment &seg );
	int GetBPMSegmentIndexAtBeat( float fBeat );
	BPMSegment& GetBPMSegmentAtBeat( float fBeat );

	void GetBeatAndBPSFromElapsedTime( float fElapsedTime, float &fBeatOut, float &fBPSOut, bool &bFreezeOut ) const;
	float GetBeatFromElapsedTime( float fElapsedTime ) const	// shortcut for places that care only about the beat
	{
		float fBeat, fThrowAway;
		bool bThrowAway;
		GetBeatAndBPSFromElapsedTime( fElapsedTime, fBeat, fThrowAway, bThrowAway );
		return fBeat;
	}
	float GetElapsedTimeFromBeat( float fBeat ) const;
	bool HasBpmChanges() const;
	bool HasStops() const;

	bool operator==( const TimingData &other )
	{
		COMPARE( m_BPMSegments.size() );
		for( unsigned i=0; i<m_BPMSegments.size(); i++ )
			COMPARE( m_BPMSegments[i] );
		COMPARE( m_StopSegments.size() );
		for( unsigned i=0; i<m_StopSegments.size(); i++ )
			COMPARE( m_StopSegments[i] );
		COMPARE( m_fBeat0OffsetInSeconds );
		return true;
	}
	bool operator!=( const TimingData &other ) { return !operator==(other); }

	// used for editor fix - expand/contract needs to move BPMSegments
	// and StopSegments that land during/after the edited range.
	// in addition, we need to be able to shift them otherwise as well
	// (for example, adding/removing rows should move all following 
	// segments as necessary)
	// NOTE: How do we want to handle deleting rows that have a BPM change
	// or a stop?  I'd like to think we should move them to the first row
	// of the range that was deleted (say if rows 1680-1728 are deleted, and
	// a BPM change or a stop occurs at row 1704, we'll move it to row
	// 1680).
	void ScaleRegion( float fScale = 1, int iStartRow = 0, int iEndRow = MAX_NOTE_ROW );
	void ShiftRows( int iStartRow, int iRowsToShift );

	CString						m_sFile;		// informational only
	vector<BPMSegment>			m_BPMSegments;	// this must be sorted before gameplay
	vector<StopSegment>			m_StopSegments;	// this must be sorted before gameplay
	float	m_fBeat0OffsetInSeconds;
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
