#include "global.h"
#include "TimingData.h"
#include "PrefsManager.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "NoteTypes.h"
#include <float.h>

void BPMSegment::SetBPM( float f )
{
	m_fBPS = f / 60.0f;
}

float BPMSegment::GetBPM() const
{
	return m_fBPS * 60.0f;
}

TimingData::TimingData()
{
	m_fBeat0OffsetInSeconds = 0;
}

static int CompareBPMSegments(const BPMSegment &seg1, const BPMSegment &seg2)
{
	return seg1.m_iStartIndex < seg2.m_iStartIndex;
}

void SortBPMSegmentsArray( vector<BPMSegment> &arrayBPMSegments )
{
	sort( arrayBPMSegments.begin(), arrayBPMSegments.end(), CompareBPMSegments );
}

static int CompareStopSegments(const StopSegment &seg1, const StopSegment &seg2)
{
	return seg1.m_iStartRow < seg2.m_iStartRow;
}

void SortStopSegmentsArray( vector<StopSegment> &arrayStopSegments )
{
	sort( arrayStopSegments.begin(), arrayStopSegments.end(), CompareStopSegments );
}

void TimingData::GetActualBPM( float &fMinBPMOut, float &fMaxBPMOut ) const
{
	fMinBPMOut = FLT_MAX;
	fMaxBPMOut = 0;
	for( unsigned i=0; i<m_BPMSegments.size(); i++ ) 
	{
		const BPMSegment &seg = m_BPMSegments[i];
		fMaxBPMOut = max( seg.m_fBPS * 60.0f, fMaxBPMOut );
		fMinBPMOut = min( seg.m_fBPS * 60.0f, fMinBPMOut );
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

/* Change an existing BPM segment, merge identical segments together or insert a new one. */
void TimingData::SetBPMAtBeat( float fBeat, float fBPM )
{
	int iNoteRow = BeatToNoteRow( fBeat );
	float fBPS = fBPM / 60.0f;
	unsigned i;
	for( i=0; i<m_BPMSegments.size(); i++ )
		if( m_BPMSegments[i].m_iStartIndex >= iNoteRow )
			break;

	if( i == m_BPMSegments.size() || m_BPMSegments[i].m_iStartIndex != iNoteRow )
	{
		// There is no BPMSegment at the specified beat.  If the BPM being set differs
		// from the last BPMSegment's BPM, create a new BPMSegment.
		if( i == 0 || fabsf(m_BPMSegments[i-1].m_fBPS - fBPS) > 1e-5f )
			AddBPMSegment( BPMSegment(iNoteRow, fBPM) );
	}
	else	// BPMSegment being modified is m_BPMSegments[i]
	{
		if( i > 0  &&  fabsf(m_BPMSegments[i-1].m_fBPS - fBPS) < 1e-5f )
			m_BPMSegments.erase( m_BPMSegments.begin()+i,
										  m_BPMSegments.begin()+i+1);
		else
			m_BPMSegments[i].m_fBPS = fBPS;
	}
}

void TimingData::SetStopAtBeat( float fBeat, float fSeconds )
{
	int iRow = BeatToNoteRow( fBeat );
	unsigned i;
	for( i=0; i<m_StopSegments.size(); i++ )
		if( m_StopSegments[i].m_iStartRow == iRow )
			break;

	if( i == m_StopSegments.size() )	// there is no BPMSegment at the current beat
	{
		// create a new StopSegment
		if( fSeconds > 0 )
			AddStopSegment( StopSegment(iRow, fSeconds) );
	}
	else	// StopSegment being modified is m_StopSegments[i]
	{
		if( fSeconds > 0 )
			m_StopSegments[i].m_fStopSeconds = fSeconds;
		else
			m_StopSegments.erase( m_StopSegments.begin()+i, m_StopSegments.begin()+i+1 );
	}
}

/* Multiply the BPM in the range [fStartBeat,fEndBeat) by fFactor. */
void TimingData::MultiplyBPMInBeatRange( int iStartIndex, int iEndIndex, float fFactor )
{
	/* Change all other BPM segments in this range. */
	for( unsigned i=0; i<m_BPMSegments.size(); i++ )
	{
		const int iStartIndexThisSegment = m_BPMSegments[i].m_iStartIndex;
		const bool bIsLastBPMSegment = i==m_BPMSegments.size()-1;
		const int iStartIndexNextSegment = bIsLastBPMSegment ? INT_MAX : m_BPMSegments[i+1].m_iStartIndex;

		if( iStartIndexThisSegment <= iStartIndex && iStartIndexNextSegment <= iStartIndex )
			continue;

		/* If this BPM segment crosses the beginning of the range, split it into two. */
		if( iStartIndexThisSegment < iStartIndex && iStartIndexNextSegment > iStartIndex )
		{
			BPMSegment b = m_BPMSegments[i];
			b.m_iStartIndex = iStartIndexNextSegment;
			m_BPMSegments.insert( m_BPMSegments.begin()+i+1, b );

			/* Don't apply the BPM change to the first half of the segment we just split,
			 * since it lies outside the range. */
			continue;
		}

		/* If this BPM segment crosses the end of the range, split it into two. */
		if( iStartIndexThisSegment < iEndIndex && iStartIndexNextSegment > iEndIndex )
		{
			BPMSegment b = m_BPMSegments[i];
			b.m_iStartIndex = iEndIndex;
			m_BPMSegments.insert( m_BPMSegments.begin()+i+1, b );
		}
		else if( iStartIndexNextSegment > iEndIndex )
			continue;

		m_BPMSegments[i].m_fBPS = m_BPMSegments[i].m_fBPS * fFactor;
	}
}

float TimingData::GetBPMAtBeat( float fBeat ) const
{
	int iIndex = BeatToNoteRow( fBeat );
	unsigned i;
	for( i=0; i<m_BPMSegments.size()-1; i++ )
		if( m_BPMSegments[i+1].m_iStartIndex > iIndex )
			break;
	return m_BPMSegments[i].GetBPM();
}

int TimingData::GetBPMSegmentIndexAtBeat( float fBeat )
{
	int iIndex = BeatToNoteRow( fBeat );
	int i;
	for( i=0; i<(int)(m_BPMSegments.size())-1; i++ )
		if( m_BPMSegments[i+1].m_iStartIndex > iIndex )
			break;
	return i;
}

BPMSegment& TimingData::GetBPMSegmentAtBeat( float fBeat )
{
	static BPMSegment empty;
	if( m_BPMSegments.empty() )
	{
		empty = BPMSegment();
		return empty;
	}
	
	int i = GetBPMSegmentIndexAtBeat( fBeat );
	return m_BPMSegments[i];
}

void TimingData::GetBeatAndBPSFromElapsedTime( float fElapsedTime, float &fBeatOut, float &fBPSOut, bool &bFreezeOut ) const
{
//	LOG->Trace( "GetBeatAndBPSFromElapsedTime( fElapsedTime = %f )", fElapsedTime );

	fElapsedTime += PREFSMAN->m_fGlobalOffsetSeconds;

	fElapsedTime += m_fBeat0OffsetInSeconds;


	for( unsigned i=0; i<m_BPMSegments.size(); i++ ) // foreach BPMSegment
	{
		const int iStartRowThisSegment = m_BPMSegments[i].m_iStartIndex;
		const float fStartBeatThisSegment = NoteRowToBeat( iStartRowThisSegment );
		const bool bIsFirstBPMSegment = i==0;
		const bool bIsLastBPMSegment = i==m_BPMSegments.size()-1;
		const int iStartRowNextSegment = bIsLastBPMSegment ? MAX_NOTE_ROW : m_BPMSegments[i+1].m_iStartIndex; 
		const float fStartBeatNextSegment = NoteRowToBeat( iStartRowNextSegment );
		const float fBPS = m_BPMSegments[i].m_fBPS;

		for( unsigned j=0; j<m_StopSegments.size(); j++ )	// foreach freeze
		{
			if( !bIsFirstBPMSegment && iStartRowThisSegment >= m_StopSegments[j].m_iStartRow )
				continue;
			if( !bIsLastBPMSegment && m_StopSegments[j].m_iStartRow > iStartRowNextSegment )
				continue;

				// this freeze lies within this BPMSegment
			const int iRowsBeatsSinceStartOfSegment = m_StopSegments[j].m_iStartRow - iStartRowThisSegment;
			const float fBeatsSinceStartOfSegment = NoteRowToBeat(iRowsBeatsSinceStartOfSegment);
			const float fFreezeStartSecond = fBeatsSinceStartOfSegment / fBPS;
			
			if( fFreezeStartSecond >= fElapsedTime )
				break;

			// the freeze segment is <= current time
			fElapsedTime -= m_StopSegments[j].m_fStopSeconds;

			if( fFreezeStartSecond >= fElapsedTime )
			{
				/* The time lies within the stop. */
				fBeatOut = NoteRowToBeat(m_StopSegments[j].m_iStartRow);
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

	int iRow = BeatToNoteRow(fBeat);
	for( unsigned j=0; j<m_StopSegments.size(); j++ )	// foreach freeze
	{
		/* The exact beat of a stop comes before the stop, not after, so use >=, not >. */
		if( m_StopSegments[j].m_iStartRow >= iRow )
			break;
		fElapsedTime += m_StopSegments[j].m_fStopSeconds;
	}

	for( unsigned i=0; i<m_BPMSegments.size(); i++ ) // foreach BPMSegment
	{
		const bool bIsLastBPMSegment = i==m_BPMSegments.size()-1;
		const float fBPS = m_BPMSegments[i].m_fBPS;

		if( bIsLastBPMSegment )
		{
			fElapsedTime += NoteRowToBeat( iRow ) / fBPS;
		}
		else
		{
			const int iStartIndexThisSegment = m_BPMSegments[i].m_iStartIndex;
			const int iStartIndexNextSegment = m_BPMSegments[i+1].m_iStartIndex; 
			const int iRowsInThisSegment = min( iStartIndexNextSegment - iStartIndexThisSegment, iRow );
			fElapsedTime += NoteRowToBeat( iRowsInThisSegment ) / fBPS;
			iRow -= iRowsInThisSegment;
		}
		
		if( iRow <= 0 )
			return fElapsedTime;
	}

	return fElapsedTime;
}

void TimingData::ScaleRegion( float fScale, int iStartIndex, int iEndIndex )
{
	ASSERT( fScale > 0 );
	ASSERT( iStartIndex >= 0 );
	ASSERT( iStartIndex < iEndIndex );

	unsigned ix = 0;

	for ( ix = 0; ix < m_BPMSegments.size(); ix++ )
	{
		const int iSegStart = m_BPMSegments[ix].m_iStartIndex;
		if( iSegStart < iStartIndex )
			continue;
		else if( iSegStart > iEndIndex )
			m_BPMSegments[ix].m_iStartIndex += lrintf( (iEndIndex - iStartIndex) * (fScale - 1) );
		else
			m_BPMSegments[ix].m_iStartIndex = lrintf( (iSegStart - iStartIndex) * fScale ) + iStartIndex;
	}

	for( ix = 0; ix < m_StopSegments.size(); ix++ )
	{
		const int iSegStartRow = m_StopSegments[ix].m_iStartRow;
		if( iSegStartRow < iStartIndex )
			continue;
		else if( iSegStartRow > iEndIndex )
			m_StopSegments[ix].m_iStartRow += lrintf((iEndIndex - iStartIndex) * (fScale - 1));
		else
			m_StopSegments[ix].m_iStartRow = lrintf((iSegStartRow - iStartIndex) * fScale) + iStartIndex;
	}
}

void TimingData::ShiftRows( int iStartRow, int iRowsToShift )
{
	unsigned ix = 0;

	for( ix = 0; ix < m_BPMSegments.size(); ix++ )
	{
		int &iSegStart = m_BPMSegments[ix].m_iStartIndex;
		if( iSegStart < iStartRow )
			continue;

		iSegStart += iRowsToShift;
		iSegStart = max( iSegStart, iStartRow );
	}

	for( ix = 0; ix < m_StopSegments.size(); ix++ )
	{
		int &iSegStartRow = m_StopSegments[ix].m_iStartRow;
		if( iSegStartRow < iStartRow )
			continue;
		iSegStartRow += iRowsToShift;
		iSegStartRow = max( iSegStartRow, iStartRow );
	}
}

bool TimingData::HasBpmChanges() const
{
	return m_BPMSegments.size()>1;
}

bool TimingData::HasStops() const
{
	return m_StopSegments.size()>0;
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
