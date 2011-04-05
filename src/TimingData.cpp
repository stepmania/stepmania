#include "global.h"
#include "TimingData.h"
#include "PrefsManager.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "NoteTypes.h"
#include "Foreach.h"
#include <float.h>


TimingData::TimingData()
{
	m_fBeat0OffsetInSeconds = 0;
	m_bHasNegativeBpms = false;
}

void TimingData::GetActualBPM( float &fMinBPMOut, float &fMaxBPMOut ) const
{
	fMinBPMOut = FLT_MAX;
	fMaxBPMOut = 0;
	FOREACH_CONST( BPMSegment, m_BPMSegments, seg )
	{
		const float fBPM = seg->GetBPM();
		fMaxBPMOut = max( fBPM, fMaxBPMOut );
		fMinBPMOut = min( fBPM, fMinBPMOut );
	}
}


void TimingData::AddBPMSegment( const BPMSegment &seg )
{
	m_BPMSegments.insert( upper_bound(m_BPMSegments.begin(), m_BPMSegments.end(), seg), seg );
}

void TimingData::AddStopSegment( const StopSegment &seg )
{
	m_StopSegments.insert( upper_bound(m_StopSegments.begin(), m_StopSegments.end(), seg), seg );
}

void TimingData::AddTimeSignatureSegment( const TimeSignatureSegment &seg )
{
	m_vTimeSignatureSegments.insert( upper_bound(m_vTimeSignatureSegments.begin(), m_vTimeSignatureSegments.end(), seg), seg );
}

void TimingData::AddWarpSegment( const WarpSegment &seg )
{
	m_WarpSegments.insert( upper_bound(m_WarpSegments.begin(), m_WarpSegments.end(), seg), seg );
}

void TimingData::AddTickcountSegment( const TickcountSegment &seg )
{
	m_TickcountSegments.insert( upper_bound(m_TickcountSegments.begin(), m_TickcountSegments.end(), seg), seg );
}

void TimingData::AddComboSegment( const ComboSegment &seg )
{
	m_ComboSegments.insert( upper_bound(m_ComboSegments.begin(), m_ComboSegments.end(), seg), seg );
}

void TimingData::AddLabelSegment( const LabelSegment &seg )
{
	m_LabelSegments.insert( upper_bound(m_LabelSegments.begin(), m_LabelSegments.end(), seg), seg );
}

/* Change an existing BPM segment, merge identical segments together or insert a new one. */
void TimingData::SetBPMAtRow( int iNoteRow, float fBPM )
{
	float fBPS = fBPM / 60.0f;
	unsigned i;
	for( i=0; i<m_BPMSegments.size(); i++ )
		if( m_BPMSegments[i].m_iStartRow >= iNoteRow )
			break;

	if( i == m_BPMSegments.size() || m_BPMSegments[i].m_iStartRow != iNoteRow )
	{
		// There is no BPMSegment at the specified beat.  If the BPM being set differs
		// from the last BPMSegment's BPM, create a new BPMSegment.
		if( i == 0 || fabsf(m_BPMSegments[i-1].m_fBPS - fBPS) > 1e-5f )
			AddBPMSegment( BPMSegment(iNoteRow, fBPM) );
	}
	else	// BPMSegment being modified is m_BPMSegments[i]
	{
		if( i > 0  &&  fabsf(m_BPMSegments[i-1].m_fBPS - fBPS) < 1e-5f )
			m_BPMSegments.erase( m_BPMSegments.begin()+i, m_BPMSegments.begin()+i+1 );
		else
			m_BPMSegments[i].m_fBPS = fBPS;
	}
}

void TimingData::SetStopAtRow( int iRow, float fSeconds, bool bDelay )
{
	unsigned i;
	for( i=0; i<m_StopSegments.size(); i++ )
		if( m_StopSegments[i].m_iStartRow == iRow && m_StopSegments[i].m_bDelay == bDelay )
			break;

	if( i == m_StopSegments.size() )	// there is no Stop/Delay Segment at the current beat
	{
		// create a new StopSegment
		if( fSeconds > 0 || PREFSMAN->m_bQuirksMode )
		{
			AddStopSegment( StopSegment(iRow, fSeconds, bDelay) );
		}
	}
	else	// StopSegment being modified is m_StopSegments[i]
	{
		if( fSeconds > 0 || PREFSMAN->m_bQuirksMode )
		{
			m_StopSegments[i].m_fStopSeconds = fSeconds;
			//m_StopSegments[i].m_bDelay = bDelay; // use this?
		}
		else
			m_StopSegments.erase( m_StopSegments.begin()+i, m_StopSegments.begin()+i+1 );
	}
}

void TimingData::SetTimeSignatureAtRow( int iRow, int iNumerator, int iDenominator )
{
	unsigned i;
	for( i = 0; i < m_vTimeSignatureSegments.size(); i++ )
	{
		if( m_vTimeSignatureSegments[i].m_iStartRow >= iRow)
			break; // We found our segment.
	}
	
	if ( i == m_vTimeSignatureSegments.size() || m_vTimeSignatureSegments[i].m_iStartRow != iRow )
	{
		// No specific segmeent here: place one if it differs.
		if( i == 0 || 
		   ( m_vTimeSignatureSegments[i-1].m_iNumerator != iNumerator
		    || m_vTimeSignatureSegments[i-1].m_iDenominator != iDenominator ) )
			AddTimeSignatureSegment( TimeSignatureSegment(iRow, iNumerator, iDenominator) );
	}
	else	// TimeSignatureSegment being modified is m_vTimeSignatureSegments[i]
	{
		if( i > 0  && m_vTimeSignatureSegments[i-1].m_iNumerator == iNumerator
		   && m_vTimeSignatureSegments[i-1].m_iDenominator == iDenominator )
			m_vTimeSignatureSegments.erase( m_vTimeSignatureSegments.begin()+i,
						       m_vTimeSignatureSegments.begin()+i+1 );
		else
		{
			m_vTimeSignatureSegments[i].m_iNumerator = iNumerator;
			m_vTimeSignatureSegments[i].m_iDenominator = iDenominator;
		}
	}
}

void TimingData::SetTimeSignatureNumeratorAtRow( int iRow, int iNumerator )
{
	SetTimeSignatureAtRow( iRow, iNumerator, GetTimeSignatureSegmentAtBeat( NoteRowToBeat( iRow ) ).m_iDenominator );
}

void TimingData::SetTimeSignatureDenominatorAtRow( int iRow, int iDenominator )
{
	SetTimeSignatureAtRow( iRow, GetTimeSignatureSegmentAtBeat( NoteRowToBeat( iRow ) ).m_iNumerator, iDenominator );
}

void TimingData::SetWarpAtRow( int iRow, float fNew )
{
	unsigned i;
	for( i=0; i<m_WarpSegments.size(); i++ )
		if( m_WarpSegments[i].m_iStartRow == iRow )
			break;
	bool valid = iRow > 0 && NoteRowToBeat(iRow) < fNew;
	if( i == m_WarpSegments.size() )
	{
		if( valid )
		{
			AddWarpSegment( WarpSegment(iRow, fNew) );
		}
	}
	else
	{
		if( valid )
		{
			m_WarpSegments[i].m_fEndBeat = fNew;
		}
		else
			m_WarpSegments.erase( m_WarpSegments.begin()+i, m_WarpSegments.begin()+i+1 );
	}
}

/* Change an existing Tickcount segment, merge identical segments together or insert a new one. */
void TimingData::SetTickcountAtRow( int iRow, int iTicks )
{
	unsigned i;
	for( i=0; i<m_TickcountSegments.size(); i++ )
		if( m_TickcountSegments[i].m_iStartRow >= iRow )
			break;

	if( i == m_TickcountSegments.size() || m_TickcountSegments[i].m_iStartRow != iRow )
	{
		// No TickcountSegment here. Make a new segment if required.
		if( i == 0 || m_TickcountSegments[i-1].m_iTicks != iTicks )
			AddTickcountSegment( TickcountSegment(iRow, iTicks ) );
	}
	else	// TickcountSegment being modified is m_TickcountSegments[i]
	{
		if( i > 0  && m_TickcountSegments[i-1].m_iTicks == iTicks )
			m_TickcountSegments.erase( m_TickcountSegments.begin()+i, m_TickcountSegments.begin()+i+1 );
		else
			m_TickcountSegments[i].m_iTicks = iTicks;
	}
}

void TimingData::SetComboAtRow( int iRow, int iCombo )
{
	unsigned i;
	for( i=0; i<m_ComboSegments.size(); i++ )
		if( m_ComboSegments[i].m_iStartRow >= iRow )
			break;
	
	if( i == m_ComboSegments.size() || m_ComboSegments[i].m_iStartRow != iRow )
	{
		if( i == 0 || m_ComboSegments[i-1].m_iCombo != iCombo )
			AddComboSegment( ComboSegment(iRow, iCombo ) );
	}
	else
	{
		if( i > 0 && m_ComboSegments[i-1].m_iCombo == iCombo )
			m_ComboSegments.erase( m_ComboSegments.begin()+i, m_ComboSegments.begin()+i+1 );
		else
			m_ComboSegments[i].m_iCombo = iCombo;
	}
}

float TimingData::GetStopAtRow( int iNoteRow, bool bDelay ) const
{
	for( unsigned i=0; i<m_StopSegments.size(); i++ )
	{
		if( m_StopSegments[i].m_bDelay == bDelay && m_StopSegments[i].m_iStartRow == iNoteRow )
		{
			return m_StopSegments[i].m_fStopSeconds;
		}
	}
	return 0;
}

float TimingData::GetStopAtRow( int iRow ) const
{
	return GetStopAtRow( iRow, false );
}


float TimingData::GetDelayAtRow( int iRow ) const
{
	return GetStopAtRow( iRow, true );
}

int TimingData::GetComboAtRow( int iNoteRow ) const
{
	return m_ComboSegments[GetComboSegmentIndexAtRow( iNoteRow )].m_iCombo;
}

float TimingData::GetWarpAtRow( int iWarpRow ) const
{
	for( unsigned i=0; i<m_WarpSegments.size(); i++ )
	{
		if( m_WarpSegments[i].m_iStartRow == iWarpRow )
		{
			return m_WarpSegments[i].m_fEndBeat;
		}
	}
	return 0;
}

// Multiply the BPM in the range [fStartBeat,fEndBeat) by fFactor.
void TimingData::MultiplyBPMInBeatRange( int iStartIndex, int iEndIndex, float fFactor )
{
	// Change all other BPM segments in this range.
	for( unsigned i=0; i<m_BPMSegments.size(); i++ )
	{
		const int iStartIndexThisSegment = m_BPMSegments[i].m_iStartRow;
		const bool bIsLastBPMSegment = i==m_BPMSegments.size()-1;
		const int iStartIndexNextSegment = bIsLastBPMSegment ? INT_MAX : m_BPMSegments[i+1].m_iStartRow;

		if( iStartIndexThisSegment <= iStartIndex && iStartIndexNextSegment <= iStartIndex )
			continue;

		/* If this BPM segment crosses the beginning of the range,
		 * split it into two. */
		if( iStartIndexThisSegment < iStartIndex && iStartIndexNextSegment > iStartIndex )
		{
			BPMSegment b = m_BPMSegments[i];
			b.m_iStartRow = iStartIndexNextSegment;
			m_BPMSegments.insert( m_BPMSegments.begin()+i+1, b );

			/* Don't apply the BPM change to the first half of the segment we
			 * just split, since it lies outside the range. */
			continue;
		}

		// If this BPM segment crosses the end of the range, split it into two.
		if( iStartIndexThisSegment < iEndIndex && iStartIndexNextSegment > iEndIndex )
		{
			BPMSegment b = m_BPMSegments[i];
			b.m_iStartRow = iEndIndex;
			m_BPMSegments.insert( m_BPMSegments.begin()+i+1, b );
		}
		else if( iStartIndexNextSegment > iEndIndex )
			continue;

		m_BPMSegments[i].m_fBPS = m_BPMSegments[i].m_fBPS * fFactor;
	}
}

float TimingData::GetBPMAtRow( int iNoteRow ) const
{
	unsigned i;
	for( i=0; i<m_BPMSegments.size()-1; i++ )
		if( m_BPMSegments[i+1].m_iStartRow > iNoteRow )
			break;
	return m_BPMSegments[i].GetBPM();
}

int TimingData::GetBPMSegmentIndexAtRow( int iNoteRow ) const
{
	unsigned i;
	for( i=0; i<m_BPMSegments.size()-1; i++ )
		if( m_BPMSegments[i+1].m_iStartRow > iNoteRow )
			break;
	return static_cast<int>(i);
}

int TimingData::GetStopSegmentIndexAtRow( int iNoteRow, bool bDelay ) const
{
	unsigned i;
	for( i=0; i<m_StopSegments.size()-1; i++ )
	{
		const StopSegment& s = m_StopSegments[i+1];
		if( s.m_iStartRow > iNoteRow && s.m_bDelay == bDelay )
			break;
	}
	return static_cast<int>(i);
}

int TimingData::GetWarpSegmentIndexAtRow( int iNoteRow ) const
{
	unsigned i;
	for( i=0; i<m_WarpSegments.size()-1; i++ )
	{
		const WarpSegment& s = m_WarpSegments[i+1];
		if( s.m_iStartRow > iNoteRow )
			break;
	}
	return static_cast<int>(i);
}

bool TimingData::IsWarpAtRow( int iNoteRow ) const
{
	if( m_WarpSegments.empty() )
		return false;
	
	int i = GetWarpSegmentIndexAtRow( iNoteRow );
	const WarpSegment& s = m_WarpSegments[i];
	if( s.m_iStartRow <= iNoteRow && iNoteRow < BeatToNoteRow(s.m_fEndBeat) )
	{
		if( m_StopSegments.empty() )
		{
			return true;
		}
		if( GetStopAtRow(iNoteRow) != 0.0f || GetDelayAtRow(iNoteRow) != 0.0f )
		{
			return false;
		}
		return true;
	}
	return false;
}

int TimingData::GetTimeSignatureSegmentIndexAtRow( int iRow ) const
{
	unsigned i;
	for (i=0; i < m_vTimeSignatureSegments.size() - 1; i++ )
		if( m_vTimeSignatureSegments[i+1].m_iStartRow > iRow )
			break;
	return static_cast<int>(i);
}

int TimingData::GetComboSegmentIndexAtRow( int iRow ) const
{
	unsigned i;
	for( i=0; i<m_ComboSegments.size()-1; i++ )
	{
		const ComboSegment& s = m_ComboSegments[i+1];
		if( s.m_iStartRow > iRow )
			break;
	}
	return static_cast<int>(i);
}

BPMSegment& TimingData::GetBPMSegmentAtRow( int iNoteRow )
{
	static BPMSegment empty;
	if( m_BPMSegments.empty() )
		return empty;

	int i = GetBPMSegmentIndexAtRow( iNoteRow );
	return m_BPMSegments[i];
}

TimeSignatureSegment& TimingData::GetTimeSignatureSegmentAtRow( int iRow )
{
	unsigned i;
	for( i=0; i<m_vTimeSignatureSegments.size()-1; i++ )
		if( m_vTimeSignatureSegments[i+1].m_iStartRow > iRow )
			break;
	return m_vTimeSignatureSegments[i];
}

int TimingData::GetTimeSignatureNumeratorAtRow( int iRow )
{
	return GetTimeSignatureSegmentAtRow( iRow ).m_iNumerator;
}

int TimingData::GetTimeSignatureDenominatorAtRow( int iRow )
{
	return GetTimeSignatureSegmentAtRow( iRow ).m_iDenominator;
}

ComboSegment& TimingData::GetComboSegmentAtRow( int iRow )
{
	unsigned i;
	for( i=0; i<m_ComboSegments.size()-1; i++ )
		if( m_ComboSegments[i+1].m_iStartRow > iRow )
			break;
	return m_ComboSegments[i];
}

StopSegment& TimingData::GetStopSegmentAtRow( int iNoteRow, bool bDelay )
{
	static StopSegment empty;
	if( m_StopSegments.empty() )
		return empty;
	
	int i = GetStopSegmentIndexAtRow( iNoteRow, bDelay );
	return m_StopSegments[i];
}

WarpSegment& TimingData::GetWarpSegmentAtRow( int iRow )
{
	static WarpSegment empty;
	if( m_WarpSegments.empty() )
		return empty;
	
	int i = GetWarpSegmentIndexAtRow( iRow );
	return m_WarpSegments[i];
}

int TimingData::GetTickcountSegmentIndexAtRow( int iRow ) const
{
	int i;
	for (i=0; i < (int)(m_TickcountSegments.size()) - 1; i++ )
		if( m_TickcountSegments[i+1].m_iStartRow > iRow )
			break;
	return i;
}

TickcountSegment& TimingData::GetTickcountSegmentAtRow( int iRow )
{
	static TickcountSegment empty;
	if( m_TickcountSegments.empty() )
		return empty;
	
	int i = GetTickcountSegmentIndexAtBeat( iRow );
	return m_TickcountSegments[i];
}

int TimingData::GetTickcountAtRow( int iRow ) const
{
	return m_TickcountSegments[GetTickcountSegmentIndexAtRow( iRow )].m_iTicks;
}

void TimingData::GetBeatAndBPSFromElapsedTime( float fElapsedTime, float &fBeatOut, float &fBPSOut, bool &bFreezeOut, bool &bDelayOut, int &iWarpBeginOut, float &fWarpLengthOut ) const
{
	fElapsedTime += PREFSMAN->m_fGlobalOffsetSeconds;

	GetBeatAndBPSFromElapsedTimeNoOffset( fElapsedTime, fBeatOut, fBPSOut, bFreezeOut, bDelayOut, iWarpBeginOut, fWarpLengthOut );
}

enum
{
	FOUND_WARP,
	FOUND_WARP_DESTINATION,
	FOUND_BPM_CHANGE,
	FOUND_STOP,
	FOUND_MARKER,
	NOT_FOUND
};

void TimingData::GetBeatAndBPSFromElapsedTimeNoOffset( float fElapsedTime, float &fBeatOut, float &fBPSOut, bool &bFreezeOut, bool &bDelayOut, int &iWarpBeginOut, float &fWarpDestinationOut ) const
{
	
	vector<BPMSegment>::const_iterator  itBPMS = m_BPMSegments.begin();
	vector<WarpSegment>::const_iterator itWS   = m_WarpSegments.begin();
	vector<StopSegment>::const_iterator itSS   = m_StopSegments.begin();
	
	bFreezeOut = false;
	bDelayOut = false;
	
	iWarpBeginOut = -1;
	
	int iLastRow = 0;
	float fLastTime = -m_fBeat0OffsetInSeconds;
	float fBPS = GetBPMAtRow(0) / 60.0;
	
	float bIsWarping = false;
	float fWarpDestination = 0.0;
	
	for( ;; )
	{
		int iEventRow = INT_MAX;
		int iEventType = NOT_FOUND;
		if( bIsWarping && BeatToNoteRow(fWarpDestination) < iEventRow )
		{
			iEventRow = BeatToNoteRow(fWarpDestination);
			iEventType = FOUND_WARP_DESTINATION;
		}
		if( itBPMS != m_BPMSegments.end() && itBPMS->m_iStartRow < iEventRow )
		{
			iEventRow = itBPMS->m_iStartRow;
			iEventType = FOUND_BPM_CHANGE;
		}
		if( itSS != m_StopSegments.end() && itSS->m_iStartRow < iEventRow )
		{
			iEventRow = itSS->m_iStartRow;
			iEventType = FOUND_STOP;
		}
		if( itWS != m_WarpSegments.end() && itWS->m_iStartRow < iEventRow )
		{
			iEventRow = itWS->m_iStartRow;
			iEventType = FOUND_WARP;
		}
		if( iEventType == NOT_FOUND )
		{
			break;
		}
		float fTimeToNextEvent = bIsWarping ? 0 : NoteRowToBeat( iEventRow - iLastRow ) / fBPS;
		float fNextEventTime   = fLastTime + fTimeToNextEvent;
		if ( fElapsedTime < fNextEventTime )
		{
			break;
		}
		fLastTime = fNextEventTime;
		switch( iEventType )
		{
		case FOUND_WARP_DESTINATION:
			bIsWarping = false;
			break;
		case FOUND_BPM_CHANGE:
			fBPS = itBPMS->m_fBPS;
			itBPMS ++;
			break;
		case FOUND_STOP:
			{
				fTimeToNextEvent = itSS->m_fStopSeconds;
				fNextEventTime   = fLastTime + fTimeToNextEvent;
				const bool bIsDelay = itSS->m_bDelay;
				if ( fElapsedTime < fNextEventTime )
				{
					bFreezeOut = !bIsDelay;
					bDelayOut  = bIsDelay;
					fBeatOut   = NoteRowToBeat( itSS->m_iStartRow );
					fBPSOut    = fBPS;
					return;
				}
				fLastTime = fNextEventTime;
				itSS ++;
			}
			break;
		case FOUND_WARP:
			bIsWarping = true;
			if( itWS->m_fEndBeat > fWarpDestination )
			{
				fWarpDestination = itWS->m_fEndBeat;
			}
			iWarpBeginOut = iEventRow;
			fWarpDestinationOut = fWarpDestination;
			itWS ++;
			break;
		}
		iLastRow = iEventRow;
	}
	
	fBeatOut = NoteRowToBeat( iLastRow ) + (fElapsedTime - fLastTime) * fBPS;
	fBPSOut = fBPS;
	
}




float TimingData::GetElapsedTimeFromBeat( float fBeat ) const
{
	return TimingData::GetElapsedTimeFromBeatNoOffset( fBeat ) - PREFSMAN->m_fGlobalOffsetSeconds;
}

float TimingData::GetElapsedTimeFromBeatNoOffset( float fBeat ) const
{

	vector<BPMSegment>::const_iterator  itBPMS = m_BPMSegments.begin();
	vector<WarpSegment>::const_iterator itWS   = m_WarpSegments.begin();
	vector<StopSegment>::const_iterator itSS   = m_StopSegments.begin();
	
	int iLastRow = 0;
	float fLastTime = -m_fBeat0OffsetInSeconds;
	float fBPS = GetBPMAtRow(0) / 60.0;
	
	float bIsWarping = false;
	float fWarpDestination = 0.0;
	
	for( ;; )
	{
		int iEventRow = INT_MAX;
		int iEventType = NOT_FOUND;
		if( bIsWarping && BeatToNoteRow(fWarpDestination) < iEventRow )
		{
			iEventRow = BeatToNoteRow(fWarpDestination);
			iEventType = FOUND_WARP_DESTINATION;
		}
		if( itBPMS != m_BPMSegments.end() && itBPMS->m_iStartRow < iEventRow )
		{
			iEventRow = itBPMS->m_iStartRow;
			iEventType = FOUND_BPM_CHANGE;
		}
		if( itSS != m_StopSegments.end() && itSS->m_bDelay && itSS->m_iStartRow < iEventRow ) // delays (come before marker)
		{
			iEventRow = itSS->m_iStartRow;
			iEventType = FOUND_STOP;
		}
		if( BeatToNoteRow(fBeat) < iEventRow )
		{
			iEventRow = BeatToNoteRow(fBeat);
			iEventType = FOUND_MARKER;
		}
		if( itSS != m_StopSegments.end() && !itSS->m_bDelay && itSS->m_iStartRow < iEventRow ) // stops (come after marker)
		{
			iEventRow = itSS->m_iStartRow;
			iEventType = FOUND_STOP;
		}
		if( itWS != m_WarpSegments.end() && itWS->m_iStartRow < iEventRow )
		{
			iEventRow = itWS->m_iStartRow;
			iEventType = FOUND_WARP;
		}
		float fTimeToNextEvent = bIsWarping ? 0 : NoteRowToBeat( iEventRow - iLastRow ) / fBPS;
		float fNextEventTime   = fLastTime + fTimeToNextEvent;
		fLastTime = fNextEventTime;
		switch( iEventType )
		{
		case FOUND_WARP_DESTINATION:
			bIsWarping = false;
			break;
		case FOUND_BPM_CHANGE:
			fBPS = itBPMS->m_fBPS;
			itBPMS ++;
			break;
		case FOUND_STOP:
			fTimeToNextEvent = itSS->m_fStopSeconds;
			fNextEventTime   = fLastTime + fTimeToNextEvent;
			fLastTime = fNextEventTime;
			itSS ++;
			break;
		case FOUND_MARKER:
			return fLastTime;	
		case FOUND_WARP:
			bIsWarping = true;
			if( itWS->m_fEndBeat > fWarpDestination )
			{
				fWarpDestination = itWS->m_fEndBeat;
			}
			itWS ++;
			break;
		}
		iLastRow = iEventRow;
	}
	
	// won't reach here, unless BeatToNoteRow(fBeat == INT_MAX) (impossible)
	
}

void TimingData::ScaleRegion( float fScale, int iStartIndex, int iEndIndex, bool bAdjustBPM )
{
	ASSERT( fScale > 0 );
	ASSERT( iStartIndex >= 0 );
	ASSERT( iStartIndex < iEndIndex );
	
	for ( unsigned i = 0; i < m_BPMSegments.size(); i++ )
	{
		const int iSegStart = m_BPMSegments[i].m_iStartRow;
		if( iSegStart < iStartIndex )
			continue;
		else if( iSegStart > iEndIndex )
			m_BPMSegments[i].m_iStartRow += lrintf( (iEndIndex - iStartIndex) * (fScale - 1) );
		else
			m_BPMSegments[i].m_iStartRow = lrintf( (iSegStart - iStartIndex) * fScale ) + iStartIndex;
	}
	
	for( unsigned i = 0; i < m_StopSegments.size(); i++ )
	{
		const int iSegStartRow = m_StopSegments[i].m_iStartRow;
		if( iSegStartRow < iStartIndex )
			continue;
		else if( iSegStartRow > iEndIndex )
			m_StopSegments[i].m_iStartRow += lrintf((iEndIndex - iStartIndex) * (fScale - 1));
		else
			m_StopSegments[i].m_iStartRow = lrintf((iSegStartRow - iStartIndex) * fScale) + iStartIndex;
	}
	
	for( unsigned i = 0; i < m_WarpSegments.size(); i++ )
	{
		const int iSegStartRow = m_WarpSegments[i].m_iStartRow;
		const int iSegEndRow = BeatToNoteRow( m_WarpSegments[i].m_fEndBeat );
		if( iSegEndRow >= iStartIndex )
		{
			if( iSegEndRow > iEndIndex )
				m_WarpSegments[i].m_fEndBeat += NoteRowToBeat(lrintf((iEndIndex - iStartIndex) * (fScale - 1)));
			else
				m_WarpSegments[i].m_fEndBeat = NoteRowToBeat(lrintf((iSegEndRow - iStartIndex) * fScale) + iStartIndex);
		}
		if( iSegStartRow < iStartIndex )
			continue;
		else if( iSegStartRow > iEndIndex )
			m_WarpSegments[i].m_iStartRow += lrintf((iEndIndex - iStartIndex) * (fScale - 1));
		else
			m_WarpSegments[i].m_iStartRow = lrintf((iSegStartRow - iStartIndex) * fScale) + iStartIndex;
	}
	
	// adjust BPM changes to preserve timing
	if( bAdjustBPM )
	{
		int iNewEndIndex = lrintf( (iEndIndex - iStartIndex) * fScale ) + iStartIndex;
		float fEndBPMBeforeScaling = GetBPMAtRow(iNewEndIndex);
		
		// adjust BPM changes "between" iStartIndex and iNewEndIndex
		for ( unsigned i = 0; i < m_BPMSegments.size(); i++ )
		{
			const int iSegStart = m_BPMSegments[i].m_iStartRow;
			if( iSegStart <= iStartIndex )
				continue;
			else if( iSegStart >= iNewEndIndex )
				continue;
			else
				m_BPMSegments[i].m_fBPS *= fScale;
		}
		
		// set BPM at iStartIndex and iNewEndIndex.
		SetBPMAtRow( iStartIndex, GetBPMAtRow(iStartIndex) * fScale );
		SetBPMAtRow( iNewEndIndex, fEndBPMBeforeScaling );
		
	}
	
}

void TimingData::InsertRows( int iStartRow, int iRowsToAdd )
{
	for( unsigned i = 0; i < m_BPMSegments.size(); i++ )
	{
		BPMSegment &bpm = m_BPMSegments[i];
		if( bpm.m_iStartRow < iStartRow )
			continue;
		bpm.m_iStartRow += iRowsToAdd;
	}

	for( unsigned i = 0; i < m_StopSegments.size(); i++ )
	{
		StopSegment &stop = m_StopSegments[i];
		if( stop.m_iStartRow < iStartRow )
			continue;
		stop.m_iStartRow += iRowsToAdd;
	}
	
	for( unsigned i = 0; i < m_WarpSegments.size(); i++ )
	{
		WarpSegment &warp = m_WarpSegments[i];
		if( BeatToNoteRow(warp.m_fEndBeat) >= iStartRow )
			warp.m_fEndBeat += NoteRowToBeat(iRowsToAdd);
		if( warp.m_iStartRow < iStartRow )
			continue;
		warp.m_iStartRow += iRowsToAdd;
	}
	
	for( unsigned i = 0; i < m_vTimeSignatureSegments.size(); i++ )
	{
		TimeSignatureSegment &time = m_vTimeSignatureSegments[i];
		if( time.m_iStartRow < iStartRow )
			continue;
		time.m_iStartRow += iRowsToAdd;
	}
	
	for( unsigned i = 0; i < m_TickcountSegments.size(); i++ )
	{
		TickcountSegment &tick = m_TickcountSegments[i];
		if( tick.m_iStartRow < iStartRow )
			continue;
		tick.m_iStartRow += iRowsToAdd;
	}
	
	for( unsigned i = 0; i < m_ComboSegments.size(); i++ )
	{
		ComboSegment &comb = m_ComboSegments[i];
		if( comb.m_iStartRow < iStartRow )
			continue;
		comb.m_iStartRow += iRowsToAdd;
	}

	if( iStartRow == 0 )
	{
		/* If we're shifting up at the beginning, we just shifted up the first
		 * BPMSegment. That segment must always begin at 0. */
		ASSERT( m_BPMSegments.size() > 0 );
		m_BPMSegments[0].m_iStartRow = 0;
	}
}

// Delete BPMChanges and StopSegments in [iStartRow,iRowsToDelete), and shift down.
void TimingData::DeleteRows( int iStartRow, int iRowsToDelete )
{
	/* Remember the BPM at the end of the region being deleted. */
	float fNewBPM = this->GetBPMAtBeat( NoteRowToBeat(iStartRow+iRowsToDelete) );

	/* We're moving rows up. Delete any BPM changes and stops in the region
	 * being deleted. */
	for( unsigned i = 0; i < m_BPMSegments.size(); i++ )
	{
		BPMSegment &bpm = m_BPMSegments[i];

		// Before deleted region:
		if( bpm.m_iStartRow < iStartRow )
			continue;

		// Inside deleted region:
		if( bpm.m_iStartRow < iStartRow+iRowsToDelete )
		{
			m_BPMSegments.erase( m_BPMSegments.begin()+i, m_BPMSegments.begin()+i+1 );
			--i;
			continue;
		}

		// After deleted region:
		bpm.m_iStartRow -= iRowsToDelete;
	}

	for( unsigned i = 0; i < m_StopSegments.size(); i++ )
	{
		StopSegment &stop = m_StopSegments[i];

		// Before deleted region:
		if( stop.m_iStartRow < iStartRow )
			continue;

		// Inside deleted region:
		if( stop.m_iStartRow < iStartRow+iRowsToDelete )
		{
			m_StopSegments.erase( m_StopSegments.begin()+i, m_StopSegments.begin()+i+1 );
			--i;
			continue;
		}

		// After deleted region:
		stop.m_iStartRow -= iRowsToDelete;
	}
	
	for( unsigned i = 0; i < m_WarpSegments.size(); i++ )
	{
		WarpSegment &warp = m_WarpSegments[i];
		
		if( BeatToNoteRow(warp.m_fEndBeat) >= iStartRow )
			warp.m_fEndBeat = max( NoteRowToBeat(iStartRow), warp.m_fEndBeat - NoteRowToBeat(iRowsToDelete) );
		
		if( warp.m_iStartRow < iStartRow )
			continue;
		
		if( warp.m_iStartRow < iStartRow+iRowsToDelete )
		{
			m_WarpSegments.erase( m_WarpSegments.begin()+i, m_WarpSegments.begin()+i+1 );
			--i;
			continue;
		}
		
		warp.m_iStartRow -= iRowsToDelete;
	}
	
	for( unsigned i = 0; i < m_vTimeSignatureSegments.size(); i++ )
	{
		TimeSignatureSegment &time = m_vTimeSignatureSegments[i];
		
		// Before deleted region:
		if( time.m_iStartRow < iStartRow )
			continue;
		
		// Inside deleted region:
		if( time.m_iStartRow < iStartRow+iRowsToDelete )
		{
			m_vTimeSignatureSegments.erase( 
				m_vTimeSignatureSegments.begin()+i, 
				m_vTimeSignatureSegments.begin()+i+1 );
			--i;
			continue;
		}
		
		// After deleted region:
		
		
		time.m_iStartRow -= iRowsToDelete;
	}
	
	for( unsigned i = 0; i < m_TickcountSegments.size(); i++ )
	{
		TickcountSegment &tick = m_TickcountSegments[i];
		
		// Before deleted region:
		if( tick.m_iStartRow < iStartRow )
			continue;
		
		// Inside deleted region:
		if( tick.m_iStartRow < iStartRow+iRowsToDelete )
		{
			m_TickcountSegments.erase( m_TickcountSegments.begin()+i, m_TickcountSegments.begin()+i+1 );
			--i;
			continue;
		}
		
		// After deleted region:
		tick.m_iStartRow -= iRowsToDelete;
	}
	
	for( unsigned i = 0; i < m_ComboSegments.size(); i++ )
	{
		ComboSegment &comb = m_ComboSegments[i];
		
		// Before deleted region:
		if( comb.m_iStartRow < iStartRow )
			continue;
		
		// Inside deleted region:
		if( comb.m_iStartRow < iStartRow+iRowsToDelete )
		{
			m_ComboSegments.erase( m_ComboSegments.begin()+i, m_ComboSegments.begin()+i+1 );
			--i;
			continue;
		}
		
		// After deleted region:
		comb.m_iStartRow -= iRowsToDelete;
	}

	this->SetBPMAtRow( iStartRow, fNewBPM );
}

bool TimingData::HasBpmChanges() const
{
	return m_BPMSegments.size()>1;
}

bool TimingData::HasStops() const
{
	return m_StopSegments.size()>0;
}

bool TimingData::HasWarps() const
{
	return m_WarpSegments.size()>0;
}

void TimingData::NoteRowToMeasureAndBeat( int iNoteRow, int &iMeasureIndexOut, int &iBeatIndexOut, int &iRowsRemainder ) const
{
	iMeasureIndexOut = 0;

	FOREACH_CONST( TimeSignatureSegment, m_vTimeSignatureSegments, iter )
	{
		vector<TimeSignatureSegment>::const_iterator next = iter;
		next++;
		int iSegmentEndRow = (next == m_vTimeSignatureSegments.end()) ? INT_MAX : next->m_iStartRow;

		int iRowsPerMeasureThisSegment = iter->GetNoteRowsPerMeasure();

		if( iNoteRow >= iter->m_iStartRow )
		{
			// iNoteRow lands in this segment
			int iNumRowsThisSegment = iNoteRow - iter->m_iStartRow;
			int iNumMeasuresThisSegment = (iNumRowsThisSegment) / iRowsPerMeasureThisSegment;	// don't round up
			iMeasureIndexOut += iNumMeasuresThisSegment;
			iBeatIndexOut = iNumRowsThisSegment / iRowsPerMeasureThisSegment;
			iRowsRemainder = iNumRowsThisSegment % iRowsPerMeasureThisSegment;
			return;
		}
		else
		{
			// iNoteRow lands after this segment
			int iNumRowsThisSegment = iSegmentEndRow - iter->m_iStartRow;
			int iNumMeasuresThisSegment = (iNumRowsThisSegment + iRowsPerMeasureThisSegment - 1) / iRowsPerMeasureThisSegment;	// round up
			iMeasureIndexOut += iNumMeasuresThisSegment;
		}
	}

	ASSERT(0);
	return;
}


// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the TimingData. */ 
class LunaTimingData: public Luna<TimingData>
{
public:
	static int HasStops( T* p, lua_State *L )		{ lua_pushboolean(L, p->HasStops()); return 1; }
	static int HasBPMChanges( T* p, lua_State *L )		{ lua_pushboolean(L, p->HasBpmChanges()); return 1; }
	static int HasWarps( T* p, lua_State *L )		{ lua_pushboolean(L, p->HasWarps()); return 1; }
	static int GetStops( T* p, lua_State *L )
	{
		vector<RString> vStops;
		FOREACH_CONST( StopSegment, p->m_StopSegments, seg )
		{
			const float fStartRow = NoteRowToBeat(seg->m_iStartRow);
			const float fStopLength = seg->m_fStopSeconds;
			if(!seg->m_bDelay)
				vStops.push_back( ssprintf("%f=%f", fStartRow, fStopLength) );
		}

		LuaHelpers::CreateTableFromArray(vStops, L);
		return 1;
	}
	static int GetDelays( T* p, lua_State *L )
	{
		vector<RString> vDelays;
		FOREACH_CONST( StopSegment, p->m_StopSegments, seg )
		{
			const float fStartRow = NoteRowToBeat(seg->m_iStartRow);
			const float fStopLength = seg->m_fStopSeconds;
			if(seg->m_bDelay)
				vDelays.push_back( ssprintf("%f=%f", fStartRow, fStopLength) );
		}

		LuaHelpers::CreateTableFromArray(vDelays, L);
		return 1;
	}
	static int GetBPMs( T* p, lua_State *L )
	{
		vector<float> vBPMs;
		FOREACH_CONST( BPMSegment, p->m_BPMSegments, seg )
		{
			const float fBPM = seg->GetBPM();
			vBPMs.push_back( fBPM );
		}

		LuaHelpers::CreateTableFromArray(vBPMs, L);
		return 1;
	}
	static int GetBPMsAndTimes( T* p, lua_State *L )
	{
		vector<RString> vBPMs;
		FOREACH_CONST( BPMSegment, p->m_BPMSegments, seg )
		{
			const float fStartRow = NoteRowToBeat(seg->m_iStartRow);
			const float fBPM = seg->GetBPM();
			vBPMs.push_back( ssprintf("%f=%f", fStartRow, fBPM) );
		}

		LuaHelpers::CreateTableFromArray(vBPMs, L);
		return 1;
	}
	static int GetActualBPM( T* p, lua_State *L )
	{
		// certainly there's a better way to do it than this? -aj
		float fMinBPM, fMaxBPM;
		p->GetActualBPM( fMinBPM, fMaxBPM );
		vector<float> fBPMs;
		fBPMs.push_back( fMinBPM );
		fBPMs.push_back( fMaxBPM );
		LuaHelpers::CreateTableFromArray(fBPMs, L);
		return 1;
	}
	static int HasNegativeBPMs( T* p, lua_State *L )		{ lua_pushboolean(L, p->m_bHasNegativeBpms); return 1; }
	// formerly in Song.cpp in sm-ssc private beta 1.x:
	static int GetBPMAtBeat( T* p, lua_State *L )		{ lua_pushnumber(L, p->GetBPMAtBeat(FArg(1))); return 1; }
	static int GetBeatFromElapsedTime( T* p, lua_State *L )	{ lua_pushnumber(L, p->GetBeatFromElapsedTime(FArg(1))); return 1; }
	static int GetElapsedTimeFromBeat( T* p, lua_State *L )	{ lua_pushnumber(L, p->GetElapsedTimeFromBeat(FArg(1))); return 1; }

	LunaTimingData()
	{
		ADD_METHOD( HasStops );
		ADD_METHOD( HasBPMChanges );
		ADD_METHOD( HasWarps );
		ADD_METHOD( GetStops );
		ADD_METHOD( GetDelays );
		ADD_METHOD( GetBPMs );
		ADD_METHOD( GetBPMsAndTimes );
		ADD_METHOD( GetActualBPM );
		ADD_METHOD( HasNegativeBPMs );
		// formerly in Song.cpp in sm-ssc private beta 1.x:
		ADD_METHOD( GetBPMAtBeat );
		ADD_METHOD( GetBeatFromElapsedTime );
		ADD_METHOD( GetElapsedTimeFromBeat );
	}
};

LUA_REGISTER_CLASS( TimingData )
// lua end

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
