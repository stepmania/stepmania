#include "global.h"
#include "TimingData.h"
#include "PrefsManager.h"
#include "RageUtil.h"

TimingData::TimingData()
{
	m_fBeat0OffsetInSeconds = 0;
}

static int CompareBPMSegments(const BPMSegment &seg1, const BPMSegment &seg2)
{
	return seg1.m_fStartBeat < seg2.m_fStartBeat;
}

void SortBPMSegmentsArray( vector<BPMSegment> &arrayBPMSegments )
{
	sort( arrayBPMSegments.begin(), arrayBPMSegments.end(), CompareBPMSegments );
}

static int CompareStopSegments(const StopSegment &seg1, const StopSegment &seg2)
{
	return seg1.m_fStartBeat < seg2.m_fStartBeat;
}

void SortStopSegmentsArray( vector<StopSegment> &arrayStopSegments )
{
	sort( arrayStopSegments.begin(), arrayStopSegments.end(), CompareStopSegments );
}

void TimingData::GetActualBPM( float &fMinBPMOut, float &fMaxBPMOut ) const
{
	fMaxBPMOut = 0;
	fMinBPMOut = 100000;	// inf
	for( unsigned i=0; i<m_BPMSegments.size(); i++ ) 
	{
		const BPMSegment &seg = m_BPMSegments[i];
		fMaxBPMOut = max( seg.m_fBPM, fMaxBPMOut );
		fMinBPMOut = min( seg.m_fBPM, fMinBPMOut );
	}
}


void TimingData::AddBPMSegment( const BPMSegment &seg )
{
	m_BPMSegments.push_back( seg );
	SortBPMSegmentsArray( m_BPMSegments );
}

void TimingData::AddStopSegment( const StopSegment &seg )
{
	m_StopSegments.push_back( seg );
	SortStopSegmentsArray( m_StopSegments );
}

void TimingData::SetBPMAtBeat( float fBeat, float fBPM )
{
	unsigned i;
	for( i=0; i<m_BPMSegments.size(); i++ )
		if( m_BPMSegments[i].m_fStartBeat == fBeat )
			break;

	if( i == m_BPMSegments.size() )	// there is no BPMSegment at the current beat
	{
		// create a new BPMSegment
		AddBPMSegment( BPMSegment(fBeat, fBPM) );
	}
	else	// BPMSegment being modified is m_BPMSegments[i]
	{
		if( i > 0  &&  fabsf(m_BPMSegments[i-1].m_fBPM - fBPM) < 0.009f )
			m_BPMSegments.erase( m_BPMSegments.begin()+i,
										  m_BPMSegments.begin()+i+1);
		else
			m_BPMSegments[i].m_fBPM = fBPM;
	}
}


float TimingData::GetBPMAtBeat( float fBeat ) const
{
	unsigned i;
	for( i=0; i<m_BPMSegments.size()-1; i++ )
		if( m_BPMSegments[i+1].m_fStartBeat > fBeat )
			break;
	return m_BPMSegments[i].m_fBPM;
}

BPMSegment& TimingData::GetBPMSegmentAtBeat( float fBeat )
{
	unsigned i;
	for( i=0; i<m_BPMSegments.size()-1; i++ )
		if( m_BPMSegments[i+1].m_fStartBeat > fBeat )
			break;
	return m_BPMSegments[i];
}


void TimingData::GetBeatAndBPSFromElapsedTime( float fElapsedTime, float &fBeatOut, float &fBPSOut, bool &bFreezeOut ) const
{
//	LOG->Trace( "GetBeatAndBPSFromElapsedTime( fElapsedTime = %f )", fElapsedTime );

	fElapsedTime += PREFSMAN->m_fGlobalOffsetSeconds;

	fElapsedTime += m_fBeat0OffsetInSeconds;


	for( unsigned i=0; i<m_BPMSegments.size(); i++ ) // foreach BPMSegment
	{
		const float fStartBeatThisSegment = m_BPMSegments[i].m_fStartBeat;
		const bool bIsFirstBPMSegment = i==0;
		const bool bIsLastBPMSegment = i==m_BPMSegments.size()-1;
		const float fStartBeatNextSegment = bIsLastBPMSegment ? 40000/*inf*/ : m_BPMSegments[i+1].m_fStartBeat; 
		const float fBPS = m_BPMSegments[i].m_fBPM / 60.0f;

		for( unsigned j=0; j<m_StopSegments.size(); j++ )	// foreach freeze
		{
			if( !bIsFirstBPMSegment && fStartBeatThisSegment >= m_StopSegments[j].m_fStartBeat )
				continue;
			if( !bIsLastBPMSegment && m_StopSegments[j].m_fStartBeat > fStartBeatNextSegment )
				continue;

			// this freeze lies within this BPMSegment
			const float fBeatsSinceStartOfSegment = m_StopSegments[j].m_fStartBeat - fStartBeatThisSegment;
			const float fFreezeStartSecond = fBeatsSinceStartOfSegment / fBPS;
			
			if( fFreezeStartSecond >= fElapsedTime )
				break;

			// the freeze segment is <= current time
			fElapsedTime -= m_StopSegments[j].m_fStopSeconds;

			if( fFreezeStartSecond >= fElapsedTime )
			{
				/* The time lies within the stop. */
				fBeatOut = m_StopSegments[j].m_fStartBeat;
				fBPSOut = fBPS;
				bFreezeOut = true;
				return;
			}
		}

		const float fBeatsInThisSegment = fStartBeatNextSegment - fStartBeatThisSegment;
		const float fSecondsInThisSegment =  fBeatsInThisSegment / fBPS;
		if( bIsLastBPMSegment || fElapsedTime <= fSecondsInThisSegment )
		{
			// this BPMSegment IS the current segment
			fBeatOut = fStartBeatThisSegment + fElapsedTime*fBPS;
			fBPSOut = fBPS;
			bFreezeOut = false;
			return;
		}

		// this BPMSegment is NOT the current segment
		fElapsedTime -= fSecondsInThisSegment;
	}
}


float TimingData::GetElapsedTimeFromBeat( float fBeat ) const
{
	float fElapsedTime = 0;
	fElapsedTime -= PREFSMAN->m_fGlobalOffsetSeconds;
	fElapsedTime -= m_fBeat0OffsetInSeconds;

	for( unsigned j=0; j<m_StopSegments.size(); j++ )	// foreach freeze
	{
		/* The exact beat of a stop comes before the stop, not after, so use >=, not >. */
		if( m_StopSegments[j].m_fStartBeat >= fBeat )
			break;
		fElapsedTime += m_StopSegments[j].m_fStopSeconds;
	}

	for( unsigned i=0; i<m_BPMSegments.size(); i++ ) // foreach BPMSegment
	{
		const bool bIsLastBPMSegment = i==m_BPMSegments.size()-1;
		const float fBPS = m_BPMSegments[i].m_fBPM / 60.0f;

		if( bIsLastBPMSegment )
		{
			fElapsedTime += fBeat / fBPS;
		}
		else
		{
			const float fStartBeatThisSegment = m_BPMSegments[i].m_fStartBeat;
			const float fStartBeatNextSegment = m_BPMSegments[i+1].m_fStartBeat; 
			const float fBeatsInThisSegment = fStartBeatNextSegment - fStartBeatThisSegment;
			fElapsedTime += min(fBeat, fBeatsInThisSegment) / fBPS;
			fBeat -= fBeatsInThisSegment;
		}
		
		if( fBeat <= 0 )
			return fElapsedTime;
	}

	return fElapsedTime;
}

void TimingData::ScaleRegion( float fScale, float fStartBeat, float fEndBeat )
{
	ASSERT( fScale > 0 );
	ASSERT( fStartBeat >= 0 );
	ASSERT( fStartBeat < fEndBeat );

	unsigned ix = 0;

	for ( ix = 0; ix < m_BPMSegments.size(); ix++ )
	{
		const float fSegStart = m_BPMSegments[ix].m_fStartBeat;
		if( fSegStart < fStartBeat )
			continue;
		else if( fSegStart > fEndBeat )
			m_BPMSegments[ix].m_fStartBeat += (fEndBeat - fStartBeat) * (fScale - 1);
		else
			m_BPMSegments[ix].m_fStartBeat = (fSegStart - fStartBeat) * fScale + fStartBeat;
	}

	for( ix = 0; ix < m_StopSegments.size(); ix++ )
	{
		const float fSegStart = m_StopSegments[ix].m_fStartBeat;
		if( fSegStart < fStartBeat )
			continue;
		else if( fSegStart > fEndBeat )
			m_StopSegments[ix].m_fStartBeat += (fEndBeat - fStartBeat) * (fScale - 1);
		else
			m_StopSegments[ix].m_fStartBeat = (fSegStart - fStartBeat) * fScale + fStartBeat;
	}
}

void TimingData::ShiftRows( float fStartBeat, float fBeatsToShift )
{
	unsigned ix = 0;

	for( ix = 0; ix < m_BPMSegments.size(); ix++ )
	{
		float &fSegStart = m_BPMSegments[ix].m_fStartBeat;
		if( fSegStart < fStartBeat )
			continue;

		fSegStart += fBeatsToShift;
		fSegStart = max( fSegStart, fStartBeat );
	}

	for( ix = 0; ix < m_StopSegments.size(); ix++ )
	{
		float &fSegStart = m_StopSegments[ix].m_fStartBeat;
		if( fSegStart < fStartBeat )
			continue;
		fSegStart += fBeatsToShift;
		fSegStart = max( fSegStart, fStartBeat );
	}
}

bool TimingData::HasBpmChangesOrStops() const
{
	return m_BPMSegments.size()>1 || m_StopSegments.size()>0;
}

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
