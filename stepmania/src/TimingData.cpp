/* Holds data for translating beats<->seconds. */

#include "global.h"
#include "TimingData.h"
#include "PrefsManager.h"

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
	// This function is a nightmare.  Don't even try to understand it. :-)

	fElapsedTime += PREFSMAN->m_fGlobalOffsetSeconds;

	fElapsedTime += m_fBeat0OffsetInSeconds;


	for( unsigned i=0; i<m_BPMSegments.size(); i++ ) // foreach BPMSegment
	{
		float fStartBeatThisSegment = m_BPMSegments[i].m_fStartBeat;
		bool bIsLastBPMSegment = i==m_BPMSegments.size()-1;
		float fStartBeatNextSegment = bIsLastBPMSegment ? 40000/*inf*/ : m_BPMSegments[i+1].m_fStartBeat; 
		float fBeatsInThisSegment = fStartBeatNextSegment - fStartBeatThisSegment;
		float fBPM = m_BPMSegments[i].m_fBPM;
		float fBPS = fBPM / 60.0f;

		// calculate the number of seconds in this segment
		float fSecondsInThisSegment =  fBeatsInThisSegment / fBPS;
		unsigned j;
		for( j=0; j<m_StopSegments.size(); j++ )	// foreach freeze
		{
			if( fStartBeatThisSegment <= m_StopSegments[j].m_fStartBeat  &&  m_StopSegments[j].m_fStartBeat < fStartBeatNextSegment )	
			{
				// this freeze lies within this BPMSegment
				fSecondsInThisSegment += m_StopSegments[j].m_fStopSeconds;
			}
		}


		if( !bIsLastBPMSegment && fElapsedTime > fSecondsInThisSegment )
		{
			// this BPMSegement is NOT the current segment
			fElapsedTime -= fSecondsInThisSegment;
			continue;
		}

		// this BPMSegment IS the current segment

		float fBeatEstimate = fStartBeatThisSegment + fElapsedTime*fBPS;

		for( j=0; j<m_StopSegments.size(); j++ )	// foreach freeze
		{
			if( fStartBeatThisSegment > m_StopSegments[j].m_fStartBeat  ||
				m_StopSegments[j].m_fStartBeat > fStartBeatNextSegment )	
				continue;

			// this freeze lies within this BPMSegment

			if( m_StopSegments[j].m_fStartBeat > fBeatEstimate )
				break;

			fElapsedTime -= m_StopSegments[j].m_fStopSeconds;
			// re-estimate
			fBeatEstimate = fStartBeatThisSegment + fElapsedTime*fBPS;
			if( fBeatEstimate < m_StopSegments[j].m_fStartBeat )
			{
				fBeatOut = m_StopSegments[j].m_fStartBeat;
				fBPSOut = fBPS;
				bFreezeOut = true;
				return;
			}
		}

		fBeatOut = fBeatEstimate;
		fBPSOut = fBPS;
		bFreezeOut = false;
		return;
	}
}


float TimingData::GetElapsedTimeFromBeat( float fBeat ) const
{
	float fElapsedTime = 0;
	fElapsedTime -= PREFSMAN->m_fGlobalOffsetSeconds;
	fElapsedTime -= m_fBeat0OffsetInSeconds;

	for( unsigned j=0; j<m_StopSegments.size(); j++ )	// foreach freeze
	{
		if( m_StopSegments[j].m_fStartBeat >= fBeat )
			break;
		fElapsedTime += m_StopSegments[j].m_fStopSeconds;
	}

	for( unsigned i=0; i<m_BPMSegments.size(); i++ ) // foreach BPMSegment
	{
		const float fStartBeatThisSegment = m_BPMSegments[i].m_fStartBeat;
		const bool bIsLastBPMSegment = i==m_BPMSegments.size()-1;
		const float fStartBeatNextSegment = bIsLastBPMSegment ? 40000/*inf*/ : m_BPMSegments[i+1].m_fStartBeat; 
		const float fBPS = m_BPMSegments[i].m_fBPM / 60.0f;
		const float fBeatsInThisSegment = fStartBeatNextSegment - fStartBeatThisSegment;

		fElapsedTime += min(fBeat, fBeatsInThisSegment) / fBPS;
		fBeat -= fBeatsInThisSegment;
		
		if( fBeat <= 0 )
			return fElapsedTime;
	}

	// This assert fires if you leave the game running overnight with the 
	// the menu timer off and the music wheel sitting on one song.
	ASSERT(0);
	return fElapsedTime;
}

/*
 * Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
 *	Chris Danford
 *	Glenn Maynard
 */
