/* TimingData - Holds data for translating beats<->seconds. */

#ifndef TIMING_DATA_H
#define TIMING_DATA_H

struct BPMSegment 
{
	BPMSegment() { m_fStartBeat = m_fBPM = -1; };
	BPMSegment( float s, float b ) { m_fStartBeat = s; m_fBPM = b; };
	float m_fStartBeat;
	float m_fBPM;
};

struct StopSegment 
{
	StopSegment() { m_fStartBeat = m_fStopSeconds = -1; };
	StopSegment( float s, float f ) { m_fStartBeat = s; m_fStopSeconds = f; };
	float m_fStartBeat;
	float m_fStopSeconds;
};

class TimingData
{
public:
	TimingData();

	void GetActualBPM( float &fMinBPMOut, float &fMaxBPMOut ) const;
	float GetBPMAtBeat( float fBeat ) const;
	void SetBPMAtBeat( float fBeat, float fBPM );
	void AddBPMSegment( const BPMSegment &seg );
	void AddStopSegment( const StopSegment &seg );
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
	bool HasBpmChangesOrStops() const;

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
	void ScaleRegion( float fScale = 1, float fStartBeat = 0, float fEndBeat = 99999 );
	void ShiftRows( float fStartBeat, float fBeatsToShift );

	CString						m_sFile;		// informational only
	vector<BPMSegment>			m_BPMSegments;	// this must be sorted before gameplay
	vector<StopSegment>			m_StopSegments;	// this must be sorted before gameplay
	float	m_fBeat0OffsetInSeconds;
};

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
