#include "global.h"
#include "TimingData.h"
#include "PrefsManager.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "NoteTypes.h"
#include "Foreach.h"
#include <float.h>


TimingData::TimingData() : 
	m_fBeat0OffsetInSeconds(0), 
	m_bHasNegativeBpms(false)
{
}

TimingData::TimingData(float fOffset) : 
	m_fBeat0OffsetInSeconds(fOffset),
	m_bHasNegativeBpms(false)
{	
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

void TimingData::AddSpeedSegment( const SpeedSegment &seg )
{
	m_SpeedSegments.insert( upper_bound(m_SpeedSegments.begin(), m_SpeedSegments.end(), seg), seg );
}

void TimingData::AddScrollSegment( const ScrollSegment &seg )
{
	m_ScrollSegments.insert( upper_bound(m_ScrollSegments.begin(), m_ScrollSegments.end(), seg), seg );
}

void TimingData::AddFakeSegment( const FakeSegment &seg )
{
	m_FakeSegments.insert( upper_bound(m_FakeSegments.begin(), m_FakeSegments.end(), seg), seg );
}

/* Change an existing BPM segment, merge identical segments together or insert a new one. */
void TimingData::SetBPMAtRow( int iNoteRow, float fBPM )
{
	unsigned i;
	for( i=0; i<m_BPMSegments.size(); i++ )
		if( m_BPMSegments[i].GetRow() >= iNoteRow )
			break;

	if( i == m_BPMSegments.size() || m_BPMSegments[i].GetRow() != iNoteRow )
	{
		// There is no BPMSegment at the specified beat.  If the BPM being set differs
		// from the last BPMSegment's BPM, create a new BPMSegment.
		if( i == 0 || fabsf(m_BPMSegments[i-1].GetBPM() - fBPM) > 1e-5f )
			AddBPMSegment( BPMSegment(iNoteRow, fBPM) );
	}
	else	// BPMSegment being modified is m_BPMSegments[i]
	{
		if( i > 0  &&  fabsf(m_BPMSegments[i-1].GetBPM() - fBPM) < 1e-5f )
			m_BPMSegments.erase( m_BPMSegments.begin()+i, m_BPMSegments.begin()+i+1 );
		else
			m_BPMSegments[i].SetBPM(fBPM);
	}
}

void TimingData::SetStopAtRow( int iRow, float fSeconds, bool bDelay )
{
	unsigned i;
	for( i=0; i<m_StopSegments.size(); i++ )
		if( m_StopSegments[i].GetRow() == iRow && m_StopSegments[i].GetDelay() == bDelay )
			break;

	if( i == m_StopSegments.size() )	// there is no Stop/Delay Segment at the current beat
	{
		// create a new StopSegment
		if( fSeconds > 0 )
		{
			AddStopSegment( StopSegment(iRow, fSeconds, bDelay) );
		}
	}
	else	// StopSegment being modified is m_StopSegments[i]
	{
		if( fSeconds > 0 )
		{
			m_StopSegments[i].SetPause(fSeconds);
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
		if( m_vTimeSignatureSegments[i].GetRow() >= iRow)
			break; // We found our segment.
	}
	
	if ( i == m_vTimeSignatureSegments.size() || m_vTimeSignatureSegments[i].GetRow() != iRow )
	{
		// No specific segmeent here: place one if it differs.
		if( i == 0 || 
		   ( m_vTimeSignatureSegments[i-1].GetNum() != iNumerator
		    || m_vTimeSignatureSegments[i-1].GetDen() != iDenominator ) )
			AddTimeSignatureSegment( TimeSignatureSegment(iRow, iNumerator, iDenominator) );
	}
	else	// TimeSignatureSegment being modified is m_vTimeSignatureSegments[i]
	{
		if( i > 0  && m_vTimeSignatureSegments[i-1].GetNum() == iNumerator
		   && m_vTimeSignatureSegments[i-1].GetDen() == iDenominator )
			m_vTimeSignatureSegments.erase( m_vTimeSignatureSegments.begin()+i,
						       m_vTimeSignatureSegments.begin()+i+1 );
		else
		{
			m_vTimeSignatureSegments[i].SetNum(iNumerator);
			m_vTimeSignatureSegments[i].SetDen(iDenominator);
		}
	}
}

void TimingData::SetTimeSignatureNumeratorAtRow( int iRow, int iNumerator )
{
	SetTimeSignatureAtRow( iRow, iNumerator, GetTimeSignatureSegmentAtBeat( NoteRowToBeat( iRow ) ).GetDen() );
}

void TimingData::SetTimeSignatureDenominatorAtRow( int iRow, int iDenominator )
{
	SetTimeSignatureAtRow( iRow, GetTimeSignatureSegmentAtBeat( NoteRowToBeat( iRow ) ).GetNum(), iDenominator );
}

void TimingData::SetWarpAtRow( int iRow, float fNew )
{
	unsigned i;
	for( i=0; i<m_WarpSegments.size(); i++ )
		if( m_WarpSegments[i].GetRow() == iRow )
			break;
	bool valid = iRow > 0 && fNew > 0;
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
			m_WarpSegments[i].SetLength(fNew);
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
		if( m_TickcountSegments[i].GetRow() >= iRow )
			break;

	if( i == m_TickcountSegments.size() || m_TickcountSegments[i].GetRow() != iRow )
	{
		// No TickcountSegment here. Make a new segment if required.
		if( i == 0 || m_TickcountSegments[i-1].GetTicks() != iTicks )
			AddTickcountSegment( TickcountSegment(iRow, iTicks ) );
	}
	else	// TickcountSegment being modified is m_TickcountSegments[i]
	{
		if( i > 0  && m_TickcountSegments[i-1].GetTicks() == iTicks )
			m_TickcountSegments.erase( m_TickcountSegments.begin()+i, m_TickcountSegments.begin()+i+1 );
		else
			m_TickcountSegments[i].SetTicks(iTicks);
	}
}

void TimingData::SetComboAtRow( int iRow, int iCombo )
{
	unsigned i;
	for( i=0; i<m_ComboSegments.size(); i++ )
		if( m_ComboSegments[i].GetRow() >= iRow )
			break;
	
	if( i == m_ComboSegments.size() || m_ComboSegments[i].GetRow() != iRow )
	{
		if( i == 0 || m_ComboSegments[i-1].GetCombo() != iCombo )
			AddComboSegment( ComboSegment(iRow, iCombo ) );
	}
	else
	{
		if( i > 0 && m_ComboSegments[i-1].GetCombo() == iCombo )
			m_ComboSegments.erase( m_ComboSegments.begin()+i, m_ComboSegments.begin()+i+1 );
		else
			m_ComboSegments[i].SetCombo(iCombo);
	}
}

void TimingData::SetLabelAtRow( int iRow, const RString sLabel )
{
	unsigned i;
	for( i=0; i<m_LabelSegments.size(); i++ )
		if( m_LabelSegments[i].GetRow() >= iRow )
			break;
	
	if( i == m_LabelSegments.size() || m_LabelSegments[i].GetRow() != iRow )
	{
		if( i == 0 || m_LabelSegments[i-1].GetLabel() != sLabel )
			AddLabelSegment( LabelSegment(iRow, sLabel ) );
	}
	else
	{
		if( i > 0 && ( m_LabelSegments[i-1].GetLabel() == sLabel || sLabel == "" ) )
			m_LabelSegments.erase( m_LabelSegments.begin()+i, m_LabelSegments.begin()+i+1 );
		else
			m_LabelSegments[i].SetLabel(sLabel);
	}
}

void TimingData::SetSpeedAtRow( int iRow, float fPercent, float fWait, unsigned short usMode )
{
	unsigned i;
	for( i = 0; i < m_SpeedSegments.size(); i++ )
	{
		if( m_SpeedSegments[i].GetRow() >= iRow)
			break;
	}
	
	if ( i == m_SpeedSegments.size() || m_SpeedSegments[i].GetRow() != iRow )
	{
		// the core mod itself matters the most for comparisons.
		if( i == 0 || m_SpeedSegments[i-1].GetRatio() != fPercent )
			AddSpeedSegment( SpeedSegment(iRow, fPercent, fWait, usMode) );
	}
	else
	{
		// The others aren't compared: only the mod itself matters.
		if( i > 0  && m_SpeedSegments[i-1].GetRatio() == fPercent )
			m_SpeedSegments.erase( m_SpeedSegments.begin()+i,
					       m_SpeedSegments.begin()+i+1 );
		else
		{
			m_SpeedSegments[i].SetRatio(fPercent);
			m_SpeedSegments[i].SetLength(fWait);
			m_SpeedSegments[i].SetUnit(usMode);
		}
	}
}

void TimingData::SetScrollAtRow( int iRow, float fPercent )
{
	unsigned i;
	for( i = 0; i < m_ScrollSegments.size(); i++ )
	{
		if( m_ScrollSegments[i].GetRow() >= iRow)
			break;
	}
	
	if ( i == m_ScrollSegments.size() || m_ScrollSegments[i].GetRow() != iRow )
	{
		// the core mod itself matters the most for comparisons.
		if( i == 0 || m_ScrollSegments[i-1].GetRatio() != fPercent )
			AddScrollSegment( ScrollSegment(iRow, fPercent) );
	}
	else
	{
		// The others aren't compared: only the mod itself matters.
		if( i > 0  && m_ScrollSegments[i-1].GetRatio() == fPercent )
			m_ScrollSegments.erase( m_ScrollSegments.begin()+i,
					       m_ScrollSegments.begin()+i+1 );
		else
		{
			m_ScrollSegments[i].SetRatio(fPercent);
		}
	}
}

void TimingData::SetFakeAtRow( int iRow, float fNew )
{
	unsigned i;
	for( i=0; i<m_FakeSegments.size(); i++ )
		if( m_FakeSegments[i].GetRow() == iRow )
			break;
	bool valid = iRow > 0 && fNew > 0;
	if( i == m_FakeSegments.size() )
	{
		if( valid )
		{
			AddFakeSegment( FakeSegment(iRow, fNew) );
		}
	}
	else
	{
		if( valid )
		{
			m_FakeSegments[i].SetLength(fNew);
		}
		else
			m_FakeSegments.erase( m_FakeSegments.begin()+i, m_FakeSegments.begin()+i+1 );
	}
}

void TimingData::SetSpeedPercentAtRow( int iRow, float fPercent )
{
	SetSpeedAtRow( iRow, 
		      fPercent, 
		      GetSpeedSegmentAtBeat( NoteRowToBeat( iRow ) ).GetLength(),
		      GetSpeedSegmentAtBeat( NoteRowToBeat( iRow ) ).GetUnit());
}

void TimingData::SetSpeedWaitAtRow( int iRow, float fWait )
{
	SetSpeedAtRow( iRow, 
		      GetSpeedSegmentAtBeat( NoteRowToBeat( iRow ) ).GetRatio(),
		      fWait,
		      GetSpeedSegmentAtBeat( NoteRowToBeat( iRow ) ).GetUnit());
}

void TimingData::SetSpeedModeAtRow( int iRow, unsigned short usMode )
{
	SetSpeedAtRow( iRow, 
		      GetSpeedSegmentAtBeat( NoteRowToBeat( iRow ) ).GetRatio(),
		      GetSpeedSegmentAtBeat( NoteRowToBeat( iRow ) ).GetLength(),
		      usMode );
}


float TimingData::GetStopAtRow( int iNoteRow, bool bDelay ) const
{
	for( unsigned i=0; i<m_StopSegments.size(); i++ )
	{
		const StopSegment &s = m_StopSegments[i];
		if( s.GetDelay() == bDelay && s.GetRow() == iNoteRow )
		{
			return s.GetPause();
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
	return m_ComboSegments[GetComboSegmentIndexAtRow( iNoteRow )].GetCombo();
}

RString TimingData::GetLabelAtRow( int iRow ) const
{
	return m_LabelSegments[GetLabelSegmentIndexAtRow( iRow )].GetLabel();
}

float TimingData::GetWarpAtRow( int iWarpRow ) const
{
	for( unsigned i=0; i<m_WarpSegments.size(); i++ )
	{
		if( m_WarpSegments[i].GetRow() == iWarpRow )
		{
			return m_WarpSegments[i].GetLength();
		}
	}
	return 0;
}

float TimingData::GetSpeedPercentAtRow( int iRow )
{
	return GetSpeedSegmentAtRow( iRow ).GetRatio();
}

float TimingData::GetSpeedWaitAtRow( int iRow )
{
	return GetSpeedSegmentAtRow( iRow ).GetLength();
}

unsigned short TimingData::GetSpeedModeAtRow( int iRow )
{
	return GetSpeedSegmentAtRow( iRow ).GetUnit();
}

float TimingData::GetScrollAtRow( int iRow )
{
	return GetScrollSegmentAtRow( iRow ).GetRatio();
}

float TimingData::GetFakeAtRow( int iFakeRow ) const
{
	for( unsigned i=0; i<m_FakeSegments.size(); i++ )
	{
		if( m_FakeSegments[i].GetRow() == iFakeRow )
		{
			return m_FakeSegments[i].GetLength();
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
		const int iStartIndexThisSegment = m_BPMSegments[i].GetRow();
		const bool bIsLastBPMSegment = i==m_BPMSegments.size()-1;
		const int iStartIndexNextSegment = bIsLastBPMSegment ? INT_MAX : m_BPMSegments[i+1].GetRow();

		if( iStartIndexThisSegment <= iStartIndex && iStartIndexNextSegment <= iStartIndex )
			continue;

		/* If this BPM segment crosses the beginning of the range,
		 * split it into two. */
		if( iStartIndexThisSegment < iStartIndex && iStartIndexNextSegment > iStartIndex )
		{
			BPMSegment b = m_BPMSegments[i];
			b.SetRow(iStartIndexNextSegment);
			m_BPMSegments.insert( m_BPMSegments.begin()+i+1, b );

			/* Don't apply the BPM change to the first half of the segment we
			 * just split, since it lies outside the range. */
			continue;
		}

		// If this BPM segment crosses the end of the range, split it into two.
		if( iStartIndexThisSegment < iEndIndex && iStartIndexNextSegment > iEndIndex )
		{
			BPMSegment b = m_BPMSegments[i];
			b.SetRow(iEndIndex);
			m_BPMSegments.insert( m_BPMSegments.begin()+i+1, b );
		}
		else if( iStartIndexNextSegment > iEndIndex )
			continue;

		m_BPMSegments[i].SetBPM(m_BPMSegments[i].GetBPM() * fFactor);
	}
}

float TimingData::GetBPMAtRow( int iNoteRow ) const
{
	unsigned i;
	for( i=0; i<m_BPMSegments.size()-1; i++ )
		if( m_BPMSegments[i+1].GetRow() > iNoteRow )
			break;
	return m_BPMSegments[i].GetBPM();
}

int TimingData::GetBPMSegmentIndexAtRow( int iNoteRow ) const
{
	unsigned i;
	for( i=0; i<m_BPMSegments.size()-1; i++ )
		if( m_BPMSegments[i+1].GetRow() > iNoteRow )
			break;
	return static_cast<int>(i);
}

int TimingData::GetStopSegmentIndexAtRow( int iNoteRow, bool bDelay ) const
{
	unsigned i;
	for( i=0; i<m_StopSegments.size()-1; i++ )
	{
		const StopSegment& s = m_StopSegments[i+1];
		if( s.GetRow() > iNoteRow && s.GetDelay() == bDelay )
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
		if( s.GetRow() > iNoteRow )
			break;
	}
	return static_cast<int>(i);
}

int TimingData::GetFakeSegmentIndexAtRow( int iNoteRow ) const
{
	unsigned i;
	for( i=0; i<m_FakeSegments.size()-1; i++ )
	{
		const FakeSegment& s = m_FakeSegments[i+1];
		if( s.GetRow() > iNoteRow )
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
	float beatRow = NoteRowToBeat(iNoteRow);
	if( s.GetBeat() <= beatRow && beatRow < (s.GetBeat() + s.GetLength() ) )
	{
		// Allow stops inside warps to allow things like stop, warp, stop, warp, stop, and so on.
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

bool TimingData::IsFakeAtRow( int iNoteRow ) const
{
	if( m_FakeSegments.empty() )
		return false;
	
	int i = GetFakeSegmentIndexAtRow( iNoteRow );
	const FakeSegment& s = m_FakeSegments[i];
	float beatRow = NoteRowToBeat(iNoteRow);
	if( s.GetBeat() <= beatRow && beatRow < ( s.GetBeat() + s.GetLength() ) )
	{
		return true;
	}
	return false;
}

int TimingData::GetTimeSignatureSegmentIndexAtRow( int iRow ) const
{
	unsigned i;
	for (i=0; i < m_vTimeSignatureSegments.size() - 1; i++ )
		if( m_vTimeSignatureSegments[i+1].GetRow() > iRow )
			break;
	return static_cast<int>(i);
}

int TimingData::GetComboSegmentIndexAtRow( int iRow ) const
{
	unsigned i;
	for( i=0; i<m_ComboSegments.size()-1; i++ )
	{
		const ComboSegment& s = m_ComboSegments[i+1];
		if( s.GetRow() > iRow )
			break;
	}
	return static_cast<int>(i);
}

int TimingData::GetLabelSegmentIndexAtRow( int iRow ) const
{
	unsigned i;
	for( i=0; i<m_LabelSegments.size()-1; i++ )
	{
		const LabelSegment& s = m_LabelSegments[i+1];
		if( s.GetRow() > iRow )
			break;
	}
	return static_cast<int>(i);
}

int TimingData::GetSpeedSegmentIndexAtRow( int iRow ) const
{
	unsigned i;
	for (i=0; i < m_SpeedSegments.size() - 1; i++ )
		if( m_SpeedSegments[i+1].GetRow() > iRow )
			break;
	return static_cast<int>(i);
}

int TimingData::GetScrollSegmentIndexAtRow( int iRow ) const
{
	unsigned i;
	for (i=0; i < m_ScrollSegments.size() - 1; i++ )
		if( m_ScrollSegments[i+1].GetRow() > iRow )
			break;
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
		if( m_vTimeSignatureSegments[i+1].GetRow() > iRow )
			break;
	return m_vTimeSignatureSegments[i];
}

SpeedSegment& TimingData::GetSpeedSegmentAtRow( int iRow )
{
	unsigned i;
	for( i=0; i<m_SpeedSegments.size()-1; i++ )
		if( m_SpeedSegments[i+1].GetRow() > iRow )
			break;
	return m_SpeedSegments[i];
}

ScrollSegment& TimingData::GetScrollSegmentAtRow( int iRow )
{
	unsigned i;
	for( i=0; i<m_ScrollSegments.size()-1; i++ )
		if( m_ScrollSegments[i+1].GetRow() > iRow )
			break;
	return m_ScrollSegments[i];
}

int TimingData::GetTimeSignatureNumeratorAtRow( int iRow )
{
	return GetTimeSignatureSegmentAtRow( iRow ).GetNum();
}

int TimingData::GetTimeSignatureDenominatorAtRow( int iRow )
{
	return GetTimeSignatureSegmentAtRow( iRow ).GetDen();
}

ComboSegment& TimingData::GetComboSegmentAtRow( int iRow )
{
	unsigned i;
	for( i=0; i<m_ComboSegments.size()-1; i++ )
		if( m_ComboSegments[i+1].GetRow() > iRow )
			break;
	return m_ComboSegments[i];
}

LabelSegment& TimingData::GetLabelSegmentAtRow( int iRow )
{
	unsigned i;
	for( i=0; i<m_LabelSegments.size()-1; i++ )
		if( m_LabelSegments[i+1].GetRow() > iRow )
			break;
	return m_LabelSegments[i];
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

FakeSegment& TimingData::GetFakeSegmentAtRow( int iRow )
{
	static FakeSegment empty;
	if( m_FakeSegments.empty() )
		return empty;
	
	int i = GetFakeSegmentIndexAtRow( iRow );
	return m_FakeSegments[i];
}

int TimingData::GetTickcountSegmentIndexAtRow( int iRow ) const
{
	unsigned i;
	for (i=0; i < m_TickcountSegments.size() - 1; i++ )
		if( m_TickcountSegments[i+1].GetRow() > iRow )
			break;
	return static_cast<int>(i);
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
	return m_TickcountSegments[GetTickcountSegmentIndexAtRow( iRow )].GetTicks();
}

float TimingData::GetPreviousLabelSegmentBeatAtRow( int iRow ) const
{
	float backup = -1;
	for (unsigned i = 0; i < m_LabelSegments.size(); i++ )
	{
		if( m_LabelSegments[i].GetRow() >= iRow )
		{
			break;
		}
		backup = m_LabelSegments[i].GetBeat();
	}
	return (backup > -1) ? backup : NoteRowToBeat(iRow);
}

float TimingData::GetNextLabelSegmentBeatAtRow( int iRow ) const
{
	for (unsigned i = 0; i < m_LabelSegments.size(); i++ )
	{
		if( m_LabelSegments[i].GetRow() <= iRow )
		{
			continue;
		}
		return m_LabelSegments[i].GetBeat();
	}
	return NoteRowToBeat(iRow);
}

bool TimingData::DoesLabelExist( RString sLabel ) const
{
	FOREACH_CONST( LabelSegment, m_LabelSegments, seg )
	{
		if( seg->GetLabel() == sLabel )
			return true;
	}
	return false;
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
		if( itBPMS != m_BPMSegments.end() && itBPMS->GetRow() < iEventRow )
		{
			iEventRow = itBPMS->GetRow();
			iEventType = FOUND_BPM_CHANGE;
		}
		if( itSS != m_StopSegments.end() && itSS->GetRow() < iEventRow )
		{
			iEventRow = itSS->GetRow();
			iEventType = FOUND_STOP;
		}
		if( itWS != m_WarpSegments.end() && itWS->GetRow() < iEventRow )
		{
			iEventRow = itWS->GetRow();
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
			fBPS = itBPMS->GetBPS();
			itBPMS ++;
			break;
		case FOUND_STOP:
			{
				fTimeToNextEvent = itSS->GetPause();
				fNextEventTime   = fLastTime + fTimeToNextEvent;
				const bool bIsDelay = itSS->GetDelay();
				if ( fElapsedTime < fNextEventTime )
				{
					bFreezeOut = !bIsDelay;
					bDelayOut  = bIsDelay;
					fBeatOut   = itSS->GetBeat();
					fBPSOut    = fBPS;
					return;
				}
				fLastTime = fNextEventTime;
				itSS ++;
			}
			break;
		case FOUND_WARP:
			{
				bIsWarping = true;
				float fWarpSum = itWS->GetLength() + itWS->GetBeat();
				if( fWarpSum > fWarpDestination )
				{
					fWarpDestination = fWarpSum;
				}
				iWarpBeginOut = iEventRow;
				fWarpDestinationOut = fWarpDestination;
				itWS ++;
				break;
			}
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
		if( itBPMS != m_BPMSegments.end() && itBPMS->GetRow() < iEventRow )
		{
			iEventRow = itBPMS->GetRow();
			iEventType = FOUND_BPM_CHANGE;
		}
		if( itSS != m_StopSegments.end() && itSS->GetDelay() && itSS->GetRow() < iEventRow ) // delays (come before marker)
		{
			iEventRow = itSS->GetRow();
			iEventType = FOUND_STOP;
		}
		if( BeatToNoteRow(fBeat) < iEventRow )
		{
			iEventRow = BeatToNoteRow(fBeat);
			iEventType = FOUND_MARKER;
		}
		if( itSS != m_StopSegments.end() && !itSS->GetDelay() && itSS->GetRow() < iEventRow ) // stops (come after marker)
		{
			iEventRow = itSS->GetRow();
			iEventType = FOUND_STOP;
		}
		if( itWS != m_WarpSegments.end() && itWS->GetRow() < iEventRow )
		{
			iEventRow = itWS->GetRow();
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
			fBPS = itBPMS->GetBPS();
			itBPMS ++;
			break;
		case FOUND_STOP:
			fTimeToNextEvent = itSS->GetPause();
			fNextEventTime   = fLastTime + fTimeToNextEvent;
			fLastTime = fNextEventTime;
			itSS ++;
			break;
		case FOUND_MARKER:
			return fLastTime;	
		case FOUND_WARP:
			{
				bIsWarping = true;
				float fWarpSum = itWS->GetLength() + itWS->GetBeat();
				if( fWarpSum > fWarpDestination )
				{
					fWarpDestination = fWarpSum;
				}
				itWS ++;
				break;
			}
		}
		iLastRow = iEventRow;
	}
	
	// won't reach here, unless BeatToNoteRow(fBeat == INT_MAX) (impossible)
	
}

float TimingData::GetDisplayedBeat( float fBeat ) const
{
	vector<ScrollSegment>::const_iterator it = m_ScrollSegments.begin(), end = m_ScrollSegments.end();
	float fOutBeat = 0.0;
	for( ; it != end; it++ )
	{
		if( it+1 == end || fBeat <= (it+1)->GetBeat() )
		{
			fOutBeat += ( fBeat - (it)->GetBeat() ) * (it)->GetRatio();
			break;
		}
		else
		{
			fOutBeat += ( (it+1)->GetBeat() - (it)->GetBeat() ) * (it)->GetRatio();
		}
	}
	return fOutBeat;
}

void TimingData::ScaleRegion( float fScale, int iStartIndex, int iEndIndex, bool bAdjustBPM )
{
	ASSERT( fScale > 0 );
	ASSERT( iStartIndex >= 0 );
	ASSERT( iStartIndex < iEndIndex );
	
	for ( unsigned i = 0; i < m_BPMSegments.size(); i++ )
	{
		BPMSegment &b = m_BPMSegments[i];
		const int iSegStart = b.GetRow();
		if( iSegStart < iStartIndex )
			continue;
		else if( iSegStart > iEndIndex )
			b.SetRow( b.GetRow() + lrintf( (iEndIndex - iStartIndex) * (fScale - 1) ) );
		else
			b.SetRow( lrintf( (iSegStart - iStartIndex) * fScale ) + iStartIndex );
	}
	
	for( unsigned i = 0; i < m_StopSegments.size(); i++ )
	{
		StopSegment &s = m_StopSegments[i];
		const int iSegStartRow = s.GetRow();
		if( iSegStartRow < iStartIndex )
			continue;
		else if( iSegStartRow > iEndIndex )
			s.SetRow(s.GetRow() + lrintf((iEndIndex - iStartIndex) * (fScale - 1)));
		else
			s.SetRow(lrintf((iSegStartRow - iStartIndex) * fScale) + iStartIndex);
	}
	
	for( unsigned i = 0; i < m_vTimeSignatureSegments.size(); i++ )
	{
		TimeSignatureSegment &t = m_vTimeSignatureSegments[i];
		const int iSegStartRow = t.GetRow();
		if( iSegStartRow < iStartIndex )
			continue;
		else if( iSegStartRow > iEndIndex )
			t.SetRow(t.GetRow() + lrintf((iEndIndex - iStartIndex) * (fScale - 1)));
		else
			t.SetRow(lrintf((iSegStartRow - iStartIndex) * fScale) + iStartIndex);
	}
	
	for( unsigned i = 0; i < m_WarpSegments.size(); i++ )
	{
		WarpSegment &w = m_WarpSegments[i];
		const int iSegStartRow = w.GetRow();
		const int iSegEndRow = iSegStartRow + BeatToNoteRow( w.GetLength() );
		if( iSegEndRow >= iStartIndex )
		{
			if( iSegEndRow > iEndIndex )
				w.SetLength(w.GetLength() +
					    NoteRowToBeat(lrintf((iEndIndex - iStartIndex) * (fScale - 1))));
			else
				w.SetLength(NoteRowToBeat(lrintf((iSegEndRow - iStartIndex) * fScale)));
		}
		if( iSegStartRow < iStartIndex )
			continue;
		else if( iSegStartRow > iEndIndex )
			w.SetRow(w.GetRow() + lrintf((iEndIndex - iStartIndex) * (fScale - 1)));
		else
			w.SetRow(lrintf((iSegStartRow - iStartIndex) * fScale) + iStartIndex);
	}
	
	for ( unsigned i = 0; i < m_TickcountSegments.size(); i++ )
	{
		TickcountSegment &t = m_TickcountSegments[i];
		const int iSegStart = t.GetRow();
		if( iSegStart < iStartIndex )
			continue;
		else if( iSegStart > iEndIndex )
			t.SetRow(t.GetRow() + lrintf( (iEndIndex - iStartIndex) * (fScale - 1) ));
		else
			t.SetRow(lrintf( (iSegStart - iStartIndex) * fScale ) + iStartIndex);
	}
	
	for ( unsigned i = 0; i < m_ComboSegments.size(); i++ )
	{
		ComboSegment &c = m_ComboSegments[i];
		const int iSegStart = c.GetRow();
		if( iSegStart < iStartIndex )
			continue;
		else if( iSegStart > iEndIndex )
			c.SetRow(c.GetRow() + lrintf( (iEndIndex - iStartIndex) * (fScale - 1) ));
		else
			c.SetRow(lrintf( (iSegStart - iStartIndex) * fScale ) + iStartIndex);
	}
	
	for ( unsigned i = 0; i < m_LabelSegments.size(); i++ )
	{
		LabelSegment &l = m_LabelSegments[i];
		const int iSegStart = l.GetRow();
		if( iSegStart < iStartIndex )
			continue;
		else if( iSegStart > iEndIndex )
			l.SetRow(l.GetRow() + lrintf( (iEndIndex - iStartIndex) * (fScale - 1) ));
		else
			l.SetRow(lrintf( (iSegStart - iStartIndex) * fScale ) + iStartIndex);
	}
	
	for ( unsigned i = 0; i < m_SpeedSegments.size(); i++ )
	{
		SpeedSegment &s = m_SpeedSegments[i];
		const int iSegStart = s.GetRow();
		if( iSegStart < iStartIndex )
			continue;
		else if( iSegStart > iEndIndex )
			s.SetRow(s.GetRow() + lrintf( (iEndIndex - iStartIndex) * (fScale - 1) ));
		else
			s.SetRow(lrintf( (iSegStart - iStartIndex) * fScale ) + iStartIndex);
	}
	
	for( unsigned i = 0; i < m_FakeSegments.size(); i++ )
	{
		FakeSegment &f = m_FakeSegments[i];
		const int iSegStartRow = f.GetRow();
		const int iSegEndRow = iSegStartRow + BeatToNoteRow( f.GetLength() );
		if( iSegEndRow >= iStartIndex )
		{
			if( iSegEndRow > iEndIndex )
				f.SetLength(f.GetLength() 
					    + NoteRowToBeat(lrintf((iEndIndex - iStartIndex) * (fScale - 1))));
			else
				f.SetLength(NoteRowToBeat(lrintf((iSegEndRow - iStartIndex) * fScale)));
		}
		if( iSegStartRow < iStartIndex )
			continue;
		else if( iSegStartRow > iEndIndex )
			f.SetRow(f.GetRow() 
				 + lrintf((iEndIndex - iStartIndex) * (fScale - 1)));
		else
			f.SetRow(lrintf((iSegStartRow - iStartIndex) * fScale) + iStartIndex);
	}
	
	for( unsigned i = 0; i < m_ScrollSegments.size(); i++ )
	{
		ScrollSegment &s = m_ScrollSegments[i];
		const int iSegStartRow = s.GetRow();
		if( iSegStartRow < iStartIndex )
			continue;
		else if( iSegStartRow > iEndIndex )
			s.SetRow(s.GetRow() + lrintf((iEndIndex - iStartIndex) * (fScale - 1)));
		else
			s.SetRow(lrintf((iSegStartRow - iStartIndex) * fScale) + iStartIndex);
	}
	
	// adjust BPM changes to preserve timing
	if( bAdjustBPM )
	{
		int iNewEndIndex = lrintf( (iEndIndex - iStartIndex) * fScale ) + iStartIndex;
		float fEndBPMBeforeScaling = GetBPMAtRow(iNewEndIndex);
		
		// adjust BPM changes "between" iStartIndex and iNewEndIndex
		for ( unsigned i = 0; i < m_BPMSegments.size(); i++ )
		{
			const int iSegStart = m_BPMSegments[i].GetRow();
			if( iSegStart <= iStartIndex )
				continue;
			else if( iSegStart >= iNewEndIndex )
				continue;
			else
				m_BPMSegments[i].SetBPM( m_BPMSegments[i].GetBPM() * fScale );
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
		if( bpm.GetRow() < iStartRow )
			continue;
		bpm.SetRow( bpm.GetRow() + iRowsToAdd );
	}

	for( unsigned i = 0; i < m_StopSegments.size(); i++ )
	{
		StopSegment &stop = m_StopSegments[i];
		if( stop.GetRow() < iStartRow )
			continue;
		stop.SetRow(stop.GetRow() + iRowsToAdd);
	}
	
	for( unsigned i = 0; i < m_WarpSegments.size(); i++ )
	{
		WarpSegment &warp = m_WarpSegments[i];
		if( warp.GetRow() < iStartRow )
			continue;
		warp.SetRow(warp.GetRow() + iRowsToAdd);
	}
	
	for( unsigned i = 0; i < m_vTimeSignatureSegments.size(); i++ )
	{
		TimeSignatureSegment &time = m_vTimeSignatureSegments[i];
		if( time.GetRow() < iStartRow )
			continue;
		time.SetRow(time.GetRow() + iRowsToAdd);
	}
	
	for( unsigned i = 0; i < m_TickcountSegments.size(); i++ )
	{
		TickcountSegment &tick = m_TickcountSegments[i];
		if( tick.GetRow() < iStartRow )
			continue;
		tick.SetRow(tick.GetRow() + iRowsToAdd);
	}
	
	for( unsigned i = 0; i < m_ComboSegments.size(); i++ )
	{
		ComboSegment &comb = m_ComboSegments[i];
		if( comb.GetRow() < iStartRow )
			continue;
		comb.SetRow(comb.GetRow() + iRowsToAdd);
	}
	for( unsigned i = 0; i < m_LabelSegments.size(); i++ )
	{
		LabelSegment &labl = m_LabelSegments[i];
		if( labl.GetRow() < iStartRow )
			continue;
		labl.SetRow(labl.GetRow() + iRowsToAdd);
	}
	
	for( unsigned i = 0; i < m_SpeedSegments.size(); i++ )
	{
		SpeedSegment &sped = m_SpeedSegments[i];
		if( sped.GetRow() < iStartRow )
			continue;
		sped.SetRow(sped.GetRow() + iRowsToAdd);
	}
	
	for( unsigned i = 0; i < m_FakeSegments.size(); i++ )
	{
		FakeSegment &fake = m_FakeSegments[i];
		if( fake.GetRow() < iStartRow )
			continue;
		fake.SetRow(fake.GetRow() + iRowsToAdd);
	}
	
	for( unsigned i = 0; i < m_ScrollSegments.size(); i++ )
	{
		ScrollSegment &scrl = m_ScrollSegments[i];
		if( scrl.GetRow() < iStartRow )
			continue;
		scrl.SetRow(scrl.GetRow() + iRowsToAdd);
	}

	if( iStartRow == 0 )
	{
		/* If we're shifting up at the beginning, we just shifted up the first
		 * BPMSegment. That segment must always begin at 0. */
		ASSERT( m_BPMSegments.size() > 0 );
		m_BPMSegments[0].SetRow(0);
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
		if( bpm.GetRow() < iStartRow )
			continue;

		// Inside deleted region:
		if( bpm.GetRow() < iStartRow+iRowsToDelete )
		{
			m_BPMSegments.erase( m_BPMSegments.begin()+i, m_BPMSegments.begin()+i+1 );
			--i;
			continue;
		}

		// After deleted region:
		bpm.SetRow( bpm.GetRow() - iRowsToDelete );
	}

	for( unsigned i = 0; i < m_StopSegments.size(); i++ )
	{
		StopSegment &stop = m_StopSegments[i];
		int keyRow = stop.GetRow();
		// Before deleted region:
		if( keyRow < iStartRow )
			continue;

		// Inside deleted region:
		if( keyRow < iStartRow+iRowsToDelete )
		{
			m_StopSegments.erase( m_StopSegments.begin()+i, m_StopSegments.begin()+i+1 );
			--i;
			continue;
		}

		// After deleted region:
		stop.SetRow(keyRow - iRowsToDelete);
	}
	
	for( unsigned i = 0; i < m_WarpSegments.size(); i++ )
	{
		WarpSegment &warp = m_WarpSegments[i];
		int keyRow = warp.GetRow();
		if( keyRow < iStartRow )
			continue;
		
		if( keyRow < iStartRow+iRowsToDelete )
		{
			m_WarpSegments.erase( m_WarpSegments.begin()+i, m_WarpSegments.begin()+i+1 );
			--i;
			continue;
		}
		
		warp.SetRow(keyRow - iRowsToDelete);
	}
	
	for( unsigned i = 0; i < m_vTimeSignatureSegments.size(); i++ )
	{
		TimeSignatureSegment &time = m_vTimeSignatureSegments[i];
		int keyRow = time.GetRow();
		// Before deleted region:
		if( keyRow < iStartRow )
			continue;
		
		// Inside deleted region:
		if( keyRow < iStartRow+iRowsToDelete )
		{
			m_vTimeSignatureSegments.erase( 
				m_vTimeSignatureSegments.begin()+i, 
				m_vTimeSignatureSegments.begin()+i+1 );
			--i;
			continue;
		}
		
		// After deleted region:
		
		
		time.SetRow(keyRow - iRowsToDelete);
	}
	
	for( unsigned i = 0; i < m_TickcountSegments.size(); i++ )
	{
		TickcountSegment &tick = m_TickcountSegments[i];
		int keyRow = tick.GetRow();
		// Before deleted region:
		if( keyRow < iStartRow )
			continue;
		
		// Inside deleted region:
		if( keyRow < iStartRow+iRowsToDelete )
		{
			m_TickcountSegments.erase( m_TickcountSegments.begin()+i, m_TickcountSegments.begin()+i+1 );
			--i;
			continue;
		}
		
		// After deleted region:
		tick.SetRow(keyRow - iRowsToDelete);
	}
	
	for( unsigned i = 0; i < m_ComboSegments.size(); i++ )
	{
		ComboSegment &comb = m_ComboSegments[i];
		int keyRow = comb.GetRow();
		// Before deleted region:
		if( keyRow < iStartRow )
			continue;
		
		// Inside deleted region:
		if( keyRow < iStartRow+iRowsToDelete )
		{
			m_ComboSegments.erase( m_ComboSegments.begin()+i, m_ComboSegments.begin()+i+1 );
			--i;
			continue;
		}
		
		// After deleted region:
		comb.SetRow(keyRow - iRowsToDelete);
	}
	
	for( unsigned i = 0; i < m_LabelSegments.size(); i++ )
	{
		LabelSegment &labl = m_LabelSegments[i];
		int keyRow = labl.GetRow();
		if( keyRow < iStartRow )
			continue;
		
		if( keyRow < iStartRow+iRowsToDelete )
		{
			m_LabelSegments.erase( m_LabelSegments.begin()+i, m_LabelSegments.begin()+i+1 );
			--i;
			continue;
		}
		labl.SetRow(keyRow - iRowsToDelete);
	}
	
	for( unsigned i = 0; i < m_SpeedSegments.size(); i++ )
	{
		SpeedSegment &sped = m_SpeedSegments[i];
		int keyRow = sped.GetRow();
		if( keyRow < iStartRow )
			continue;
		
		if( keyRow < iStartRow+iRowsToDelete )
		{
			m_SpeedSegments.erase( m_SpeedSegments.begin()+i, m_SpeedSegments.begin()+i+1 );
			--i;
			continue;
		}
		sped.SetRow(keyRow - iRowsToDelete);
	}
	
	for( unsigned i = 0; i < m_FakeSegments.size(); i++ )
	{
		FakeSegment &fake = m_FakeSegments[i];
		int keyRow = fake.GetRow();
		if( keyRow < iStartRow )
			continue;
		
		if( keyRow < iStartRow+iRowsToDelete )
		{
			m_FakeSegments.erase( m_FakeSegments.begin()+i, m_FakeSegments.begin()+i+1 );
			--i;
			continue;
		}
		
		fake.SetRow(keyRow - iRowsToDelete);
	}
	
	for( unsigned i = 0; i < m_ScrollSegments.size(); i++ )
	{
		ScrollSegment &scrl = m_ScrollSegments[i];
		int keyRow = scrl.GetRow();
		if( keyRow < iStartRow )
			continue;
		
		if( keyRow < iStartRow+iRowsToDelete )
		{
			m_ScrollSegments.erase( m_ScrollSegments.begin()+i, m_ScrollSegments.begin()+i+1 );
			--i;
			continue;
		}
		scrl.SetRow(keyRow - iRowsToDelete);
	}

	this->SetBPMAtRow( iStartRow, fNewBPM );
}

float TimingData::GetDisplayedSpeedPercent( float fSongBeat, float fMusicSeconds ) const
{
	/* HACK: Somehow we get called into this function when there is no
	 * TimingData to work with. This seems to happen the most upon
	 * leaving the editor. Still, cover our butts in case this instance
	 * isn't existing. */
	if (!this) return 1.0f;
	if( m_SpeedSegments.size() == 0 )
		return 1.0f;

	const int index = GetSpeedSegmentIndexAtBeat( fSongBeat );
	
	const SpeedSegment &seg = m_SpeedSegments[index];
	float fStartBeat = seg.GetBeat();
	float fStartTime = GetElapsedTimeFromBeat( fStartBeat ) - GetDelayAtBeat( fStartBeat );
	float fEndTime;
	float fCurTime = fMusicSeconds;
	
	if( seg.GetUnit() == 1 ) // seconds
	{
		fEndTime = fStartTime + seg.GetLength();
	}
	else
	{
		fEndTime = GetElapsedTimeFromBeat( fStartBeat + seg.GetLength() ) 
		- GetDelayAtBeat( fStartBeat + seg.GetLength() );
	}
	
	if( ( index == 0 && m_SpeedSegments[0].GetLength() > 0.0 ) && fCurTime < fStartTime )
	{
		return 1.0;
	}
	else if( fEndTime >= fCurTime && ( index > 0 || m_SpeedSegments[0].GetLength() > 0.0 ) )
	{
		const float fPriorSpeed = ( index == 0 ? 1 : m_SpeedSegments[index - 1].GetRatio() );
		float fTimeUsed = fCurTime - fStartTime;
		float fDuration = fEndTime - fStartTime;
		float fRatioUsed = fDuration == 0.0 ? 1 : fTimeUsed / fDuration;
		
		float fDistance = fPriorSpeed - seg.GetRatio();
		float fRatioNeed = fRatioUsed * -fDistance;
		return (fPriorSpeed + fRatioNeed);
	}
	else 
	{
		return seg.GetRatio();
	}

}

void TimingData::TidyUpData()
{
	// If there are no BPM segments, provide a default.
	if( m_BPMSegments.empty() )
	{
		LOG->UserLog( "Song file", m_sFile, "has no BPM segments, default provided." );

		AddBPMSegment( BPMSegment(0, 60) );
	}

	// Make sure the first BPM segment starts at beat 0.
	if( m_BPMSegments[0].GetRow() != 0 )
		m_BPMSegments[0].SetRow(0);

	// If no time signature specified, assume 4/4 time for the whole song.
	if( m_vTimeSignatureSegments.empty() )
	{
		TimeSignatureSegment seg(0, 4, 4);
		m_vTimeSignatureSegments.push_back( seg );
	}
	
	// Likewise, if no tickcount signature is specified, assume 2 ticks
	//per beat for the entire song. The default of 2 is chosen more
	//for compatibility with the Pump Pro series than anything else.
	if( m_TickcountSegments.empty() )
	{
		TickcountSegment seg(0, 2);
		m_TickcountSegments.push_back( seg );
	}
	
	// Have a default combo segment of one just in case.
	if( m_ComboSegments.empty() )
	{
		ComboSegment seg(0, 1);
		m_ComboSegments.push_back( seg );
	}
	
	// Have a default label segment just in case.
	if( m_LabelSegments.empty() )
	{
		LabelSegment seg(0, "Song Start");
		m_LabelSegments.push_back( seg );
	}
	
	// Always be sure there is a starting speed.
	if( m_SpeedSegments.empty() )
	{
		SpeedSegment seg(0, 1, 0);
		m_SpeedSegments.push_back( seg );
	}
	
	// Always be sure there is a starting scrolling factor.
	if( m_ScrollSegments.empty() )
	{
		ScrollSegment seg(0, 1);
		m_ScrollSegments.push_back( seg );
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

bool TimingData::HasWarps() const
{
	return m_WarpSegments.size()>0;
}

bool TimingData::HasFakes() const
{
	return m_FakeSegments.size()>0;
}

bool TimingData::HasSpeedChanges() const
{
	return m_SpeedSegments.size()>1 || m_SpeedSegments[0].GetRatio() != 1;
}

bool TimingData::HasScrollChanges() const
{
	return m_ScrollSegments.size()>1 || m_ScrollSegments[0].GetRatio() != 1;
}

void TimingData::NoteRowToMeasureAndBeat( int iNoteRow, int &iMeasureIndexOut, int &iBeatIndexOut, int &iRowsRemainder ) const
{
	iMeasureIndexOut = 0;

	FOREACH_CONST( TimeSignatureSegment, m_vTimeSignatureSegments, iter )
	{
		vector<TimeSignatureSegment>::const_iterator next = iter;
		next++;
		int iSegmentEndRow = (next == m_vTimeSignatureSegments.end()) ? INT_MAX : next->GetRow();

		int iRowsPerMeasureThisSegment = iter->GetNoteRowsPerMeasure();

		if( iNoteRow >= iter->GetRow() )
		{
			// iNoteRow lands in this segment
			int iNumRowsThisSegment = iNoteRow - iter->GetRow();
			int iNumMeasuresThisSegment = (iNumRowsThisSegment) / iRowsPerMeasureThisSegment;	// don't round up
			iMeasureIndexOut += iNumMeasuresThisSegment;
			iBeatIndexOut = iNumRowsThisSegment / iRowsPerMeasureThisSegment;
			iRowsRemainder = iNumRowsThisSegment % iRowsPerMeasureThisSegment;
			return;
		}
		else
		{
			// iNoteRow lands after this segment
			int iNumRowsThisSegment = iSegmentEndRow - iter->GetRow();
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
	static int HasFakes( T* p, lua_State *L )		{ lua_pushboolean(L, p->HasFakes()); return 1; }
	static int HasSpeedChanges( T* p, lua_State *L )	{ lua_pushboolean(L, p->HasSpeedChanges()); return 1; }
	static int HasScrollChanges( T* p, lua_State *L )	{ lua_pushboolean(L, p->HasScrollChanges()); return 1; }
	static int GetStops( T* p, lua_State *L )
	{
		vector<RString> vStops;
		FOREACH_CONST( StopSegment, p->m_StopSegments, seg )
		{
			const float fStartBeat = seg->GetBeat();
			const float fStopLength = seg->GetPause();
			if(!seg->GetDelay())
				vStops.push_back( ssprintf("%f=%f", fStartBeat, fStopLength) );
		}

		LuaHelpers::CreateTableFromArray(vStops, L);
		return 1;
	}
	static int GetDelays( T* p, lua_State *L )
	{
		vector<RString> vDelays;
		FOREACH_CONST( StopSegment, p->m_StopSegments, seg )
		{
			const float fStartBeat = seg->GetBeat();
			const float fStopLength = seg->GetPause();
			if(seg->GetDelay())
				vDelays.push_back( ssprintf("%f=%f", fStartBeat, fStopLength) );
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
	static int GetLabels( T* p, lua_State *L )
	{
		vector<RString> vLabels;
		FOREACH_CONST( LabelSegment, p->m_LabelSegments, seg )
		{
			const float fStartRow = seg->GetBeat();
			const RString sLabel = seg->GetLabel();
			vLabels.push_back( ssprintf("%f=%s", fStartRow, sLabel.c_str()) );
		}
		LuaHelpers::CreateTableFromArray(vLabels, L);
		return 1;
	}
	static int GetBPMsAndTimes( T* p, lua_State *L )
	{
		vector<RString> vBPMs;
		FOREACH_CONST( BPMSegment, p->m_BPMSegments, seg )
		{
			const float fStartRow = seg->GetBeat();
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
	static int HasNegativeBPMs( T* p, lua_State *L )		{ lua_pushboolean(L, p->HasWarps()); return 1; }
	// formerly in Song.cpp in sm-ssc private beta 1.x:
	static int GetBPMAtBeat( T* p, lua_State *L )		{ lua_pushnumber(L, p->GetBPMAtBeat(FArg(1))); return 1; }
	static int GetBeatFromElapsedTime( T* p, lua_State *L )	{ lua_pushnumber(L, p->GetBeatFromElapsedTime(FArg(1))); return 1; }
	static int GetElapsedTimeFromBeat( T* p, lua_State *L )	{ lua_pushnumber(L, p->GetElapsedTimeFromBeat(FArg(1))); return 1; }

	LunaTimingData()
	{
		ADD_METHOD( HasStops );
		ADD_METHOD( HasBPMChanges );
		ADD_METHOD( HasWarps );
		ADD_METHOD( HasFakes );
		ADD_METHOD( HasSpeedChanges );
		ADD_METHOD( HasScrollChanges );
		ADD_METHOD( GetStops );
		ADD_METHOD( GetDelays );
		ADD_METHOD( GetBPMs );
		ADD_METHOD( GetLabels );
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
