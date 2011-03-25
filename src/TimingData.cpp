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

/*
void TimingData::SetWarpAtRow( int iRowAt, float fLengthBeats )
{
	// todo: code this -aj
}
*/

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

int TimingData::GetWarpToRow( int iWarpBeginRow ) const
{
	for( unsigned i=0; i<m_WarpSegments.size(); i++ )
	{
		if( m_WarpSegments[i].m_iStartRow == iWarpBeginRow )
		{
			return iWarpBeginRow + BeatToNoteRow(m_WarpSegments[i].m_fWarpBeats);
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
	return (int)i;
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
	return (int)i;
}

int TimingData::GetTimeSignatureSegmentIndexAtRow( int iRow ) const
{
	int i;
	for (i=0; i < (int)(m_vTimeSignatureSegments.size()) - 1; i++ )
		if( m_vTimeSignatureSegments[i+1].m_iStartRow > iRow )
			break;
	return i;
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

int TimingData::GetComboSegmentIndexAtRow( int iRow ) const
{
	unsigned i;
	for( i=0; i<m_ComboSegments.size()-1; i++ )
	{
		const ComboSegment& s = m_ComboSegments[i+1];
		if( s.m_iStartRow > iRow )
			break;
	}
	return (int)i;
}

BPMSegment& TimingData::GetBPMSegmentAtRow( int iNoteRow )
{
	static BPMSegment empty;
	if( m_BPMSegments.empty() )
		return empty;

	int i = GetBPMSegmentIndexAtRow( iNoteRow );
	return m_BPMSegments[i];
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

void TimingData::GetBeatAndBPSFromElapsedTimeNoOffset( float fElapsedTime, float &fBeatOut, float &fBPSOut, bool &bFreezeOut, bool &bDelayOut, int &iWarpBeginOut, float &fWarpLengthOut ) const
{
//	LOG->Trace( "GetBeatAndBPSFromElapsedTime( fElapsedTime = %f )", fElapsedTime );
	const float fTime = fElapsedTime;
	fElapsedTime += m_fBeat0OffsetInSeconds;

	for( unsigned i=0; i<m_BPMSegments.size(); i++ ) // foreach BPMSegment
	{
		const int iStartRowThisSegment = m_BPMSegments[i].m_iStartRow;
		const float fStartBeatThisSegment = NoteRowToBeat( iStartRowThisSegment );
		const bool bIsFirstBPMSegment = i==0;
		const bool bIsLastBPMSegment = i==m_BPMSegments.size()-1;
		const int iStartRowNextSegment = bIsLastBPMSegment ? MAX_NOTE_ROW : m_BPMSegments[i+1].m_iStartRow; 
		const float fStartBeatNextSegment = NoteRowToBeat( iStartRowNextSegment );
		const float fBPS = m_BPMSegments[i].m_fBPS;

		for( unsigned j=0; j<m_StopSegments.size(); j++ ) // foreach freeze
		{
			const bool bIsDelay = m_StopSegments[j].m_bDelay;
			if( !bIsFirstBPMSegment && iStartRowThisSegment >= m_StopSegments[j].m_iStartRow )
				continue;
			if( !bIsLastBPMSegment && m_StopSegments[j].m_iStartRow > iStartRowNextSegment )
				continue;

			// this freeze lies within this BPMSegment
			const int iRowsBeatsSinceStartOfSegment = m_StopSegments[j].m_iStartRow - iStartRowThisSegment;
			const float fBeatsSinceStartOfSegment = NoteRowToBeat(iRowsBeatsSinceStartOfSegment);
			const float fFreezeStartSecond = fBeatsSinceStartOfSegment / fBPS;

			// modified for delays
			if( !bIsDelay && fFreezeStartSecond >= fElapsedTime )
				break;
			if( bIsDelay && fFreezeStartSecond > fElapsedTime )
				break;

			// the freeze segment is <= current time
			fElapsedTime -= m_StopSegments[j].m_fStopSeconds;

			if( (fFreezeStartSecond >= fElapsedTime && !bIsDelay) ||
				(fFreezeStartSecond > fElapsedTime && bIsDelay) )
			{
				// The time lies within the stop.
				fBeatOut = NoteRowToBeat(m_StopSegments[j].m_iStartRow);
				fBPSOut = fBPS;
				bFreezeOut = !bIsDelay;
				bDelayOut = bIsDelay;
				//iWarpBeginOut = -1;
				//fWarpLengthOut = -1;
				return;
			}
		}

		// by this point we should have the warps in their own place.
		for( unsigned j=0; j<m_WarpSegments.size(); j++ ) // foreach warp
		{
			if( !bIsFirstBPMSegment && iStartRowThisSegment >= m_WarpSegments[j].m_iStartRow )
				continue;
			if( !bIsLastBPMSegment && m_WarpSegments[j].m_iStartRow > iStartRowNextSegment )
				continue;

			/*
			const int iRowsBeatsSinceStartOfSegment = m_WarpSegments[j].m_iStartRow - iStartRowThisSegment;
			const float fBeatsSinceStartOfSegment = NoteRowToBeat(iRowsBeatsSinceStartOfSegment);
			const float fWarpStartSecond = fBeatsSinceStartOfSegment / fBPS;
			*/

			// the freeze segment is <= current time
			//fElapsedTime -= m_WarpSegments[j].m_fWarpBeats;

			// this warp lies within this BPMSegment.
			/*
			if( fWarpStartSecond >= fElapsedTime )
			{
				// this WarpSegment IS the current segment.
				// don't know how to properly handle beatout -aj
				//fBeatOut = NoteRowToBeat(m_WarpSegments[j].m_iStartRow);
				fBeatOut = fStartBeatThisSegment + fElapsedTime*fBPS;
				fBPSOut = m_BPMSegments[i+1].m_fBPS;
				bFreezeOut = false;
				bDelayOut = false;
				iWarpBeginOut = m_WarpSegments[j].m_iStartRow;
				fWarpLengthOut = m_WarpSegments[j].m_fWarpBeats;
				return;
			}
			*/
		}

		const float fBeatsInThisSegment = fStartBeatNextSegment - fStartBeatThisSegment;
		const float fSecondsInThisSegment =  fBeatsInThisSegment / fBPS;
		//if(fBPS < 0.0f)
		/*
		if(fStartBeatThisSegment == 445.500f || fStartBeatThisSegment == 449.500)
		{
			LOG->Trace( ssprintf("segment (beat %f) beats: %f / seconds: %f / BPS: %f",fStartBeatThisSegment,fBeatsInThisSegment,fSecondsInThisSegment,fBPS) );
		}
		*/
		if( bIsLastBPMSegment || fElapsedTime <= fSecondsInThisSegment )
		{
			// this BPMSegment IS the current segment.
			fBeatOut = fStartBeatThisSegment + fElapsedTime*fBPS;
			fBPSOut = fBPS;
			bFreezeOut = false;
			bDelayOut = false;
			//iWarpBeginOut;
			//fWarpLengthOut;
			return;
		}

		// this BPMSegment is NOT the current segment.
		fElapsedTime -= fSecondsInThisSegment;
		// xxx: negative testing [aj]
		/*
		//if(fBPS < 0.0f)
		if( (fStartBeatNextSegment >= 445.490f && fStartBeatNextSegment <= 453.72f) || fBPS < 0.0f )
		{
			//LOG->Trace( ssprintf("beat %f is %f BPS (%f BPM)",fBeatOut,fBPSOut,fBPSOut*60.0f) );
			//LOG->Trace( ssprintf("start beat %f + elapsed time %f",fStartBeatThisSegment,fElapsedTime) );
			//LOG->Trace( ssprintf("elapsed time is now %f",fElapsedTime) );
		}
		*/
	}
	// If we get here, something has gone wrong. Is everything sorted?
	vector<BPMSegment> vBPMS = m_BPMSegments;
	vector<StopSegment> vSS = m_StopSegments;
	vector<WarpSegment> vWS = m_WarpSegments;
	vector<TimeSignatureSegment> vTSS = m_vTimeSignatureSegments;
	vector<TickcountSegment> vTS = m_TickcountSegments;
	vector<ComboSegment> vCS = m_ComboSegments;
	sort( vBPMS.begin(), vBPMS.end() );
	sort( vSS.begin(), vSS.end() );
	sort( vWS.begin(), vWS.end() );
	sort( vTSS.begin(), vTSS.end() );
	sort( vTS.begin(), vTS.end() );
	sort( vCS.begin(), vCS.end() );
	ASSERT_M( vBPMS == m_BPMSegments, "The BPM segments were not sorted!" );
	ASSERT_M( vSS == m_StopSegments, "The Stop segments were not sorted!" );
	ASSERT_M( vWS == m_WarpSegments, "The Warp segments were not sorted!" );
	ASSERT_M( vTSS == m_vTimeSignatureSegments, "The Time Signature segments were not sorted!" );
	ASSERT_M( vTS == m_TickcountSegments, "The Tickcount segments were not sorted!" );
	ASSERT_M( vCS == m_ComboSegments, "The Combo segments were not sorted!" );
	FAIL_M( ssprintf("Failed to find the appropriate segment for elapsed time %f.", fTime) );
}


float TimingData::GetElapsedTimeFromBeat( float fBeat ) const
{
	return TimingData::GetElapsedTimeFromBeatNoOffset( fBeat ) - PREFSMAN->m_fGlobalOffsetSeconds;
}

float TimingData::GetElapsedTimeFromBeatNoOffset( float fBeat ) const
{
	float fElapsedTime = 0;
	fElapsedTime -= m_fBeat0OffsetInSeconds;

	int iRow = BeatToNoteRow(fBeat);
	for( unsigned j=0; j<m_StopSegments.size(); j++ )	// foreach freeze
	{
		/* A traditional stop has the beat happening before the stop. (>=)
		 * A Pump delay acts differently: the pause is before the beat. (>)
		 */
		if( ( m_StopSegments[j].m_iStartRow >= iRow && !m_StopSegments[j].m_bDelay ) ||
			( m_StopSegments[j].m_iStartRow > iRow && m_StopSegments[j].m_bDelay ) )
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
			const int iStartIndexThisSegment = m_BPMSegments[i].m_iStartRow;
			const int iStartIndexNextSegment = m_BPMSegments[i+1].m_iStartRow; 
			const int iRowsInThisSegment = min( iStartIndexNextSegment - iStartIndexThisSegment, iRow );
			fElapsedTime += NoteRowToBeat( iRowsInThisSegment ) / fBPS;
			iRow -= iRowsInThisSegment;
		}
		
		if( iRow <= 0 )
			return fElapsedTime;
	}

	/*
	for( unsigned i=0; i<m_WarpSegments.size(); i++ ) // foreach WarpSegment
	{
		// todo: is this correct? -aj
		const int iWarpToRow = m_WarpSegments[i].m_iEndRow;
		fElapsedTime += NoteRowToBeat( iWarpToRow );
	}
	*/

	return fElapsedTime;
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
		if (warp.m_iStartRow < iStartRow )
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
