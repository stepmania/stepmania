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
 * Copyright (c) 2001-2004 by the person(s) listed below.  All rights reserved.
 *	Chris Danford
 *	Glenn Maynard
 */
