#include "global.h"
#include "TimingData.h"
#include "PrefsManager.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "NoteTypes.h"
#include "Foreach.h"
#include <float.h>


TimingData::TimingData(float fOffset) : 
	m_fBeat0OffsetInSeconds(fOffset)
{
	// allTimingSegments[SEGMENT_BPM] = new vector<BPMSegment>();
}

TimingData::~TimingData()
{
	for (unsigned i = 0; i < NUM_TimingSegmentTypes; i++)
	{
		this->allTimingSegments[i].clear();
	}
	delete [] this->allTimingSegments;
}

void TimingData::GetActualBPM( float &fMinBPMOut, float &fMaxBPMOut, float highest ) const
{
	fMinBPMOut = FLT_MAX;
	fMaxBPMOut = 0;
	const vector<TimingSegment *> &bpms = this->allTimingSegments[SEGMENT_BPM];
	for (unsigned i = 0; i < bpms.size(); i++)
	{
		BPMSegment *seg = static_cast<BPMSegment *>(bpms[i]);
		const float fBPM = seg->GetBPM();
		fMaxBPMOut = clamp(max( fBPM, fMaxBPMOut ), 0, highest);
		fMinBPMOut = min( fBPM, fMinBPMOut );
	}
}

void TimingData::AddSegment(TimingSegmentType tst, TimingSegment * seg)
{
	vector<TimingSegment *> &segs = this->allTimingSegments[tst];
	// Unsure if this uses the proper comparison.
	segs.insert(upper_bound(segs.begin(), segs.end(), seg), seg);
}

// TODO: Find a way to combine all of these SetAtRows to one.

/* Change an existing BPM segment, merge identical segments together or insert a new one. */
void TimingData::SetBPMAtRow( int iNoteRow, float fBPM )
{
	unsigned i;
	vector<TimingSegment *> &bpms = this->allTimingSegments[SEGMENT_BPM];
	for( i=0; i<bpms.size(); i++ )
		if( bpms[i]->GetRow() >= iNoteRow )
			break;

	BPMSegment *bs = static_cast<BPMSegment *>(bpms[i]);
	if( i == bpms.size() || bs->GetRow() != iNoteRow )
	{
		// There is no BPMSegment at the specified beat.  If the BPM being set differs
		// from the last BPMSegment's BPM, create a new BPMSegment.
		if (i == 0 ||
			fabsf(static_cast<BPMSegment *>(bpms[i-1])->GetBPM() - fBPM) > 1e-5f )
			AddSegment( SEGMENT_BPM, new BPMSegment(iNoteRow, fBPM) );
	}
	else	// BPMSegment being modified is m_BPMSegments[i]
	{
		if (i > 0 &&
			fabsf(static_cast<BPMSegment *>(bpms[i-1])->GetBPM() - fBPM) < 1e-5f )
			bpms.erase( bpms.begin()+i, bpms.begin()+i+1 );
		else
			bs->SetBPM(fBPM);
	}
}

void TimingData::SetStopAtRow( int iRow, float fSeconds, bool bDelay )
{
	unsigned i;
	vector<TimingSegment *> &stops = this->allTimingSegments[SEGMENT_STOP_DELAY];
	for( i=0; i<stops.size(); i++ )
		if (stops[i]->GetRow() == iRow &&
			static_cast<StopSegment *>(stops[i])->GetDelay() == bDelay )
			break;

	StopSegment *ss = static_cast<StopSegment *>(stops[i]);
	if( i == stops.size() )	// there is no Stop/Delay Segment at the current beat
	{
		// create a new StopSegment
		if( fSeconds > 0 )
		{
			AddSegment( SEGMENT_STOP_DELAY, new StopSegment(iRow, fSeconds, bDelay) );
		}
	}
	else	// StopSegment being modified is m_StopSegments[i]
	{
		if( fSeconds > 0 )
		{
			ss->SetPause(fSeconds);
		}
		else
			stops.erase( stops.begin()+i, stops.begin()+i+1 );
	}
}

void TimingData::SetTimeSignatureAtRow( int iRow, int iNumerator, int iDenominator )
{
	unsigned i;
	vector<TimingSegment *> &tSigs = this->allTimingSegments[SEGMENT_TIME_SIG];
	for( i = 0; i < tSigs.size(); i++ )
	{
		if( tSigs[i]->GetRow() >= iRow)
			break; // We found our segment.
	}
	
	TimeSignatureSegment *ts = static_cast<TimeSignatureSegment *>(tSigs[i]);
	if ( i == tSigs.size() || ts->GetRow() != iRow )
	{
		// No specific segment here: place one if it differs.
		if (i == 0 || 
		   (static_cast<TimeSignatureSegment *>(tSigs[i-1])->GetNum() != iNumerator ||
			static_cast<TimeSignatureSegment *>(tSigs[i-1])->GetDen() != iDenominator ) )
			AddSegment( SEGMENT_TIME_SIG, new TimeSignatureSegment(iRow, iNumerator, iDenominator) );
	}
	else	// TimeSignatureSegment being modified is m_vTimeSignatureSegments[i]
	{
		if (i > 0 &&
			static_cast<TimeSignatureSegment *>(tSigs[i-1])->GetNum() == iNumerator &&
			static_cast<TimeSignatureSegment *>(tSigs[i-1])->GetDen() == iDenominator )
			tSigs.erase( tSigs.begin()+i, tSigs.begin()+i+1 );
		else
		{
			ts->SetNum(iNumerator);
			ts->SetDen(iDenominator);
		}
	}
}

void TimingData::SetTimeSignatureNumeratorAtRow( int iRow, int iNumerator )
{
	this->SetTimeSignatureAtRow(iRow,
								iNumerator,
								GetTimeSignatureSegmentAtRow(iRow).GetDen());
}

void TimingData::SetTimeSignatureDenominatorAtRow( int iRow, int iDenominator )
{
	this->SetTimeSignatureAtRow(iRow,
								GetTimeSignatureSegmentAtRow(iRow).GetNum(),
								iDenominator);
}

void TimingData::SetWarpAtRow( int iRow, float fNew )
{
	unsigned i;
	vector<TimingSegment *> &warps = this->allTimingSegments[SEGMENT_WARP];
	for( i=0; i<warps.size(); i++ )
		if( warps[i]->GetRow() == iRow )
			break;
	bool valid = iRow > 0 && fNew > 0;
	if( i == warps.size() )
	{
		if( valid )
		{
			AddSegment( SEGMENT_WARP, new WarpSegment(iRow, fNew) );
		}
	}
	else
	{
		if( valid )
		{
			static_cast<WarpSegment *>(warps[i])->SetLength(fNew);
		}
		else
			warps.erase( warps.begin()+i, warps.begin()+i+1 );
	}
}

/* Change an existing Tickcount segment, merge identical segments together or insert a new one. */
void TimingData::SetTickcountAtRow( int iRow, int iTicks )
{
	unsigned i;
	vector<TimingSegment *> &ticks = this->allTimingSegments[SEGMENT_TICKCOUNT];
	for( i=0; i<ticks.size(); i++ )
		if( ticks[i]->GetRow() >= iRow )
			break;

	TickcountSegment *ts = static_cast<TickcountSegment *>(ticks[i]);
	if( i == ticks.size() || ts->GetRow() != iRow )
	{
		// No TickcountSegment here. Make a new segment if required.
		if (i == 0 ||
			static_cast<TickcountSegment *>(ticks[i-1])->GetTicks() != iTicks )
			AddSegment( SEGMENT_TICKCOUNT, new TickcountSegment(iRow, iTicks ) );
	}
	else	// TickcountSegment being modified is m_TickcountSegments[i]
	{
		if (i > 0 &&
			static_cast<TickcountSegment *>(ticks[i-1])->GetTicks() == iTicks )
			ticks.erase( ticks.begin()+i, ticks.begin()+i+1 );
		else
			ts->SetTicks(iTicks);
	}
}

void TimingData::SetComboAtRow( int iRow, int iCombo, int iMiss )
{
	unsigned i;
	for( i=0; i<m_ComboSegments.size(); i++ )
		if( m_ComboSegments[i].GetRow() >= iRow )
			break;
	
	if( i == m_ComboSegments.size() || m_ComboSegments[i].GetRow() != iRow )
	{
		if(i == 0 || m_ComboSegments[i-1].GetCombo() != iCombo ||
		   m_ComboSegments[i-1].GetMissCombo() != iMiss)
			AddComboSegment( ComboSegment(iRow, iCombo ) );
	}
	else
	{
		if(i > 0 && m_ComboSegments[i-1].GetCombo() == iCombo &&
		   m_ComboSegments[i-1].GetMissCombo() == iMiss)
			m_ComboSegments.erase( m_ComboSegments.begin()+i, m_ComboSegments.begin()+i+1 );
		else
		{
			m_ComboSegments[i].SetCombo(iCombo);
			m_ComboSegments[i].SetMissCombo(iMiss);
		}
	}
}

void TimingData::SetHitComboAtRow(int iRow, int iCombo)
{
	this->SetComboAtRow(iRow,
						iCombo,
						this->GetComboSegmentAtRow(iRow).GetMissCombo());
}

void TimingData::SetMissComboAtRow(int iRow, int iMiss)
{
	this->SetComboAtRow(iRow,
						this->GetComboSegmentAtRow(iRow).GetCombo(),
						iMiss);
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
	return m_ComboSegments[this->GetComboSegmentIndexAtRow(iNoteRow)].GetCombo();
}

int TimingData::GetMissComboAtRow(int iNoteRow) const
{
	return m_ComboSegments[this->GetComboSegmentIndexAtRow(iNoteRow)].GetMissCombo();
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

float TimingData::GetPreviousBPMSegmentBeatAtRow( int iRow ) const
{
	float backup = -1;
	for (unsigned i = 0; i < m_BPMSegments.size(); i++ )
	{
		if( m_BPMSegments[i].GetRow() >= iRow )
		{
			break;
		}
		backup = m_BPMSegments[i].GetBeat();
	}
	return (backup > -1) ? backup : NoteRowToBeat(iRow);
}

float TimingData::GetNextBPMSegmentBeatAtRow( int iRow ) const
{
	for (unsigned i = 0; i < m_BPMSegments.size(); i++ )
	{
		if( m_BPMSegments[i].GetRow() <= iRow )
		{
			continue;
		}
		return m_BPMSegments[i].GetBeat();
	}
	return NoteRowToBeat(iRow);
}

float TimingData::GetPreviousStopSegmentBeatAtRow( int iRow ) const
{
	float backup = -1;
	for (unsigned i = 0; i < m_StopSegments.size(); i++ )
	{
		const StopSegment &s = m_StopSegments[i];
		if( s.GetRow() >= iRow )
		{
			break;
		}
		if (!s.GetDelay())
			backup = s.GetBeat();
	}
	return (backup > -1) ? backup : NoteRowToBeat(iRow);
}

float TimingData::GetNextStopSegmentBeatAtRow( int iRow ) const
{
	for (unsigned i = 0; i < m_StopSegments.size(); i++ )
	{
		const StopSegment &s = m_StopSegments[i];
		if( s.GetRow() <= iRow )
		{
			continue;
		}
		if (!s.GetDelay())
			return s.GetBeat();
	}
	return NoteRowToBeat(iRow);
}

float TimingData::GetPreviousDelaySegmentBeatAtRow( int iRow ) const
{
	float backup = -1;
	for (unsigned i = 0; i < m_StopSegments.size(); i++ )
	{
		const StopSegment &s = m_StopSegments[i];
		if( s.GetRow() >= iRow )
		{
			break;
		}
		if (s.GetDelay())
			backup = s.GetBeat();
	}
	return (backup > -1) ? backup : NoteRowToBeat(iRow);
}

float TimingData::GetNextDelaySegmentBeatAtRow( int iRow ) const
{
	for (unsigned i = 0; i < m_StopSegments.size(); i++ )
	{
		const StopSegment &s = m_StopSegments[i];
		if( s.GetRow() <= iRow )
		{
			continue;
		}
		if (s.GetDelay())
			return s.GetBeat();
	}
	return NoteRowToBeat(iRow);
}

float TimingData::GetPreviousTimeSignatureSegmentBeatAtRow( int iRow ) const
{
	float backup = -1;
	for (unsigned i = 0; i < m_vTimeSignatureSegments.size(); i++ )
	{
		if( m_vTimeSignatureSegments[i].GetRow() >= iRow )
		{
			break;
		}
		backup = m_vTimeSignatureSegments[i].GetBeat();
	}
	return (backup > -1) ? backup : NoteRowToBeat(iRow);
}

float TimingData::GetNextTimeSignatureSegmentBeatAtRow( int iRow ) const
{
	for (unsigned i = 0; i < m_vTimeSignatureSegments.size(); i++ )
	{
		if( m_vTimeSignatureSegments[i].GetRow() <= iRow )
		{
			continue;
		}
		return m_vTimeSignatureSegments[i].GetBeat();
	}
	return NoteRowToBeat(iRow);
}


float TimingData::GetPreviousTickcountSegmentBeatAtRow( int iRow ) const
{
	float backup = -1;
	for (unsigned i = 0; i < m_TickcountSegments.size(); i++ )
	{
		if( m_TickcountSegments[i].GetRow() >= iRow )
		{
			break;
		}
		backup = m_TickcountSegments[i].GetBeat();
	}
	return (backup > -1) ? backup : NoteRowToBeat(iRow);
}

float TimingData::GetNextTickcountSegmentBeatAtRow( int iRow ) const
{
	for (unsigned i = 0; i < m_TickcountSegments.size(); i++ )
	{
		if( m_TickcountSegments[i].GetRow() <= iRow )
		{
			continue;
		}
		return m_TickcountSegments[i].GetBeat();
	}
	return NoteRowToBeat(iRow);
}

float TimingData::GetPreviousComboSegmentBeatAtRow( int iRow ) const
{
	float backup = -1;
	for (unsigned i = 0; i < m_ComboSegments.size(); i++ )
	{
		if( m_ComboSegments[i].GetRow() >= iRow )
		{
			break;
		}
		backup = m_ComboSegments[i].GetBeat();
	}
	return (backup > -1) ? backup : NoteRowToBeat(iRow);
}

float TimingData::GetNextComboSegmentBeatAtRow( int iRow ) const
{
	for (unsigned i = 0; i < m_ComboSegments.size(); i++ )
	{
		if( m_ComboSegments[i].GetRow() <= iRow )
		{
			continue;
		}
		return m_ComboSegments[i].GetBeat();
	}
	return NoteRowToBeat(iRow);
}



float TimingData::GetPreviousWarpSegmentBeatAtRow( int iRow ) const
{
	float backup = -1;
	for (unsigned i = 0; i < m_WarpSegments.size(); i++ )
	{
		if( m_WarpSegments[i].GetRow() >= iRow )
		{
			break;
		}
		backup = m_WarpSegments[i].GetBeat();
	}
	return (backup > -1) ? backup : NoteRowToBeat(iRow);
}

float TimingData::GetNextWarpSegmentBeatAtRow( int iRow ) const
{
	for (unsigned i = 0; i < m_WarpSegments.size(); i++ )
	{
		if( m_WarpSegments[i].GetRow() <= iRow )
		{
			continue;
		}
		return m_WarpSegments[i].GetBeat();
	}
	return NoteRowToBeat(iRow);
}

float TimingData::GetPreviousFakeSegmentBeatAtRow( int iRow ) const
{
	float backup = -1;
	for (unsigned i = 0; i < m_FakeSegments.size(); i++ )
	{
		if( m_FakeSegments[i].GetRow() >= iRow )
		{
			break;
		}
		backup = m_FakeSegments[i].GetBeat();
	}
	return (backup > -1) ? backup : NoteRowToBeat(iRow);
}

float TimingData::GetNextFakeSegmentBeatAtRow( int iRow ) const
{
	for (unsigned i = 0; i < m_FakeSegments.size(); i++ )
	{
		if( m_FakeSegments[i].GetRow() <= iRow )
		{
			continue;
		}
		return m_FakeSegments[i].GetBeat();
	}
	return NoteRowToBeat(iRow);
}

float TimingData::GetPreviousSpeedSegmentBeatAtRow( int iRow ) const
{
	float backup = -1;
	for (unsigned i = 0; i < m_SpeedSegments.size(); i++ )
	{
		if( m_SpeedSegments[i].GetRow() >= iRow )
		{
			break;
		}
		backup = m_SpeedSegments[i].GetBeat();
	}
	return (backup > -1) ? backup : NoteRowToBeat(iRow);
}

float TimingData::GetNextSpeedSegmentBeatAtRow( int iRow ) const
{
	for (unsigned i = 0; i < m_SpeedSegments.size(); i++ )
	{
		if( m_SpeedSegments[i].GetRow() <= iRow )
		{
			continue;
		}
		return m_SpeedSegments[i].GetBeat();
	}
	return NoteRowToBeat(iRow);
}

float TimingData::GetPreviousScrollSegmentBeatAtRow( int iRow ) const
{
	float backup = -1;
	for (unsigned i = 0; i < m_ScrollSegments.size(); i++ )
	{
		if( m_ScrollSegments[i].GetRow() >= iRow )
		{
			break;
		}
		backup = m_ScrollSegments[i].GetBeat();
	}
	return (backup > -1) ? backup : NoteRowToBeat(iRow);
}

float TimingData::GetNextScrollSegmentBeatAtRow( int iRow ) const
{
	for (unsigned i = 0; i < m_ScrollSegments.size(); i++ )
	{
		if( m_ScrollSegments[i].GetRow() <= iRow )
		{
			continue;
		}
		return m_ScrollSegments[i].GetBeat();
	}
	return NoteRowToBeat(iRow);
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
	float fBPS = GetBPMAtRow(0) / 60.0f;
	
	float bIsWarping = false;
	float fWarpDestination = 0;
	
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
	float fBPS = GetBPMAtRow(0) / 60.0f;
	
	float bIsWarping = false;
	float fWarpDestination = 0;
	
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
	float fOutBeat = 0;
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
	
	int length = iEndIndex - iStartIndex;
	int newLength = lrintf( fScale * length );
	
	for ( unsigned i = 0; i < m_BPMSegments.size(); i++ )
		m_BPMSegments[i].Scale( iStartIndex, length, newLength );
	
	for( unsigned i = 0; i < m_StopSegments.size(); i++ )
		m_StopSegments[i].Scale( iStartIndex, length, newLength );
	
	for( unsigned i = 0; i < m_vTimeSignatureSegments.size(); i++ )
		m_vTimeSignatureSegments[i].Scale( iStartIndex, length, newLength );
	
	for( unsigned i = 0; i < m_WarpSegments.size(); i++ )
		m_WarpSegments[i].Scale( iStartIndex, length, newLength );
	
	for ( unsigned i = 0; i < m_TickcountSegments.size(); i++ )
		m_TickcountSegments[i].Scale( iStartIndex, length, newLength );
	
	for ( unsigned i = 0; i < m_ComboSegments.size(); i++ )
		m_ComboSegments[i].Scale( iStartIndex, length, newLength );
	
	for ( unsigned i = 0; i < m_LabelSegments.size(); i++ )
		m_LabelSegments[i].Scale( iStartIndex, length, newLength );
	
	for ( unsigned i = 0; i < m_SpeedSegments.size(); i++ )
		m_SpeedSegments[i].Scale( iStartIndex, length, newLength );
	
	for( unsigned i = 0; i < m_FakeSegments.size(); i++ )
		m_FakeSegments[i].Scale( iStartIndex, length, newLength );
	
	for( unsigned i = 0; i < m_ScrollSegments.size(); i++ )
		m_ScrollSegments[i].Scale( iStartIndex, length, newLength );
	
	// adjust BPM changes to preserve timing
	if( bAdjustBPM )
	{
		int iNewEndIndex = iStartIndex + newLength;
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
	for (unsigned i = 0; i < NUM_TimingSegmentTypes; i++)
	{
		vector<TimingSegment *> &segs = this->allTimingSegments[i];
		for (unsigned j = 0; j < segs.size(); j++)
		{
			TimingSegment *seg = segs[j];
			if (seg->GetRow() < iStartRow)
				continue;
			seg->SetRow(seg->GetRow() + iRowsToAdd);
		}
	}

	if( iStartRow == 0 )
	{
		/* If we're shifting up at the beginning, we just shifted up the first
		 * BPMSegment. That segment must always begin at 0. */
		vector<TimingSegment *> &bpms = this->allTimingSegments[SEGMENT_BPM];
		ASSERT_M( bpms.size() > 0, "There must be at least one BPM Segment in the chart!" );
		bpms[0]->SetRow(0);
	}
}

// Delete BPMChanges and StopSegments in [iStartRow,iRowsToDelete), and shift down.
void TimingData::DeleteRows( int iStartRow, int iRowsToDelete )
{
	/* Remember the BPM at the end of the region being deleted. */
	float fNewBPM = this->GetBPMAtBeat( NoteRowToBeat(iStartRow+iRowsToDelete) );

	/* We're moving rows up. Delete any BPM changes and stops in the region
	 * being deleted. */
	for (unsigned i = 0; i < NUM_TimingSegmentTypes; i++)
	{
		vector<TimingSegment *> &segs = this->allTimingSegments[i];
		for (unsigned j = 0; j < segs.size(); j++)
		{
			TimingSegment *seg = segs[j];
			// Before deleted region:
			if (seg->GetRow() < iStartRow)
				continue;
			
			// Inside deleted region:
			if (seg->GetRow() < iStartRow + iRowsToDelete)
			{
				segs.erase(segs.begin()+j, segs.begin()+j+1);
				--j;
				continue;
			}
			
			// After deleted regions:
			seg->SetRow(seg->GetRow() - iRowsToDelete);
		}
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
		return 1.0f;
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
		ComboSegment seg(0, 1, 1);
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
	return this->allTimingSegments[SEGMENT_BPM].size()>1;
}

bool TimingData::HasStops() const
{
	return this->allTimingSegments[SEGMENT_STOP_DELAY].size()>0;
}

bool TimingData::HasWarps() const
{
	return this->allTimingSegments[SEGMENT_WARP].size()>0;
}

bool TimingData::HasFakes() const
{
	return this->allTimingSegments[SEGMENT_FAKE].size()>0;
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
	static int GetWarps( T* p, lua_State *L )
	{
		vector<RString> vWarps;
		vector<TimingSegment *> &warps = p->allTimingSegments[SEGMENT_WARP];
		for (unsigned i = 0; i < warps.size(); i++)
		{
			WarpSegment *seg = static_cast<WarpSegment *>(warps[i]);
			const float length = seg->GetLength();
			const float beat = seg->GetBeat();
			vWarps.push_back( ssprintf("%f=%f", beat, length) );
		}
		LuaHelpers::CreateTableFromArray(vWarps, L);
		return 1;
	}
	static int GetFakes( T* p, lua_State *L )
	{
		vector<RString> vFakes;
		vector<TimingSegment *> &fakes = p->allTimingSegments[SEGMENT_FAKE];
		for (unsigned i = 0; i < fakes.size(); i++)
		{
			FakeSegment *seg = static_cast<FakeSegment *>(fakes[i]);
			const float length = seg->GetLength();
			const float beat = seg->GetBeat();
			vFakes.push_back( ssprintf("%f=%f", beat, length) );
		}
		LuaHelpers::CreateTableFromArray(vFakes, L);
		return 1;
	}
	static int GetScrolls( T* p, lua_State *L )
	{
		vector<RString> vScrolls;
		vector<TimingSegment *> &scrolls = p->allTimingSegments[SEGMENT_SCROLL];
		for (unsigned i = 0; i < scrolls.size(); i++)
		{
			ScrollSegment *seg = static_cast<ScrollSegment *>(scrolls[i]);
			const float ratio = seg->GetRatio();
			const float beat = seg->GetBeat();
			vScrolls.push_back( ssprintf("%f=%f", beat, ratio) );
		}
		LuaHelpers::CreateTableFromArray(vScrolls, L);
		return 1;
	}
	static int GetSpeeds( T* p, lua_State *L )
	{
		vector<RString> vSpeeds;
		vector<TimingSegment *> &speeds = p->allTimingSegments[SEGMENT_SPEED];
		for (unsigned i = 0; i < speeds.size(); i++)
		{
			SpeedSegment *seg = static_cast<SpeedSegment *>(speeds[i]);
			const float length = seg->GetLength();
			const float ratio = seg->GetRatio();
			const unsigned short unit = seg->GetUnit();
			const float beat = seg->GetBeat();
			vSpeeds.push_back( ssprintf("%f=%f=%f=%uh", beat, ratio, length, unit) );
		}
		LuaHelpers::CreateTableFromArray(vSpeeds, L);
		return 1;
	}
	static int GetTimeSignatures( T* p, lua_State *L )
	{
		vector<RString> vTimes;
		vector<TimingSegment *> &tSigs = p->allTimingSegments[SEGMENT_TIME_SIG];
		for (unsigned i = 0; i < tSigs.size(); i++)
		{
			TimeSignatureSegment *seg = static_cast<TimeSignatureSegment *>(tSigs[i]);
			const int numerator = seg->GetNum();
			const int denominator = seg->GetDen();
			const float beat = seg->GetBeat();
			vTimes.push_back( ssprintf("%f=%d=%d", beat, numerator, denominator) );
		}
		LuaHelpers::CreateTableFromArray(vTimes, L);
		return 1;
	}
	static int GetCombos( T* p, lua_State *L )
	{
		vector<RString> vCombos;
		vector<TimingSegment *> &combos = p->allTimingSegments[SEGMENT_COMBO];
		for (unsigned i = 0; i < combos.size(); i++)
		{
			ComboSegment *seg = static_cast<ComboSegment *>(combos[i]);
			const int combo = seg->GetCombo();
			const int miss = seg->GetMissCombo();
			const float beat = seg->GetBeat();
			vCombos.push_back( ssprintf("%f=%d=%d", beat, combo, miss) );
		}
		LuaHelpers::CreateTableFromArray(vCombos, L);
		return 1;
	}
	static int GetTickcounts( T* p, lua_State *L )
	{
		vector<RString> vTicks;
		vector<TimingSegment *> &ticks = p->allTimingSegments[SEGMENT_TICKCOUNT];
		for (unsigned i = 0; i < ticks.size(); i++)
		{
			TickcountSegment *seg = static_cast<TickcountSegment *>(ticks[i]);
			const int tick = seg->GetTicks();
			const float beat = seg->GetBeat();
			vTicks.push_back( ssprintf("%f=%d", beat, tick) );
		}
		LuaHelpers::CreateTableFromArray(vTicks, L);
		return 1;
	}
	static int GetStops( T* p, lua_State *L )
	{
		vector<RString> vStops;
		vector<TimingSegment *> &stops = p->allTimingSegments[SEGMENT_STOP_DELAY];
		for (unsigned i = 0; i < stops.size(); i++)
		{
			StopSegment *seg = static_cast<StopSegment *>(stops[i]);
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
		vector<TimingSegment *> &stops = p->allTimingSegments[SEGMENT_STOP_DELAY];
		for (unsigned i = 0; i < stops.size(); i++)
		{
			StopSegment *seg = static_cast<StopSegment *>(stops[i]);
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
		vector<TimingSegment *> &bpms = p->allTimingSegments[SEGMENT_BPM];
		for (unsigned i = 0; i < bpms.size(); i++)
		{
			BPMSegment *seg = static_cast<BPMSegment *>(bpms[i]);
			const float fBPM = seg->GetBPM();
			vBPMs.push_back( fBPM );
		}

		LuaHelpers::CreateTableFromArray(vBPMs, L);
		return 1;
	}
	static int GetLabels( T* p, lua_State *L )
	{
		vector<RString> vLabels;
		vector<TimingSegment *> &labels = p->allTimingSegments[SEGMENT_LABEL];
		for (unsigned i = 0; i < labels.size(); i++)
		{
			LabelSegment *seg = static_cast<LabelSegment *>(labels[i]);
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
		vector<TimingSegment *> &bpms = p->allTimingSegments[SEGMENT_BPM];
		for (unsigned i = 0; i < bpms.size(); i++)
		{
			BPMSegment *seg = static_cast<BPMSegment *>(bpms[i]);
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
		ADD_METHOD( GetWarps );
		ADD_METHOD( GetFakes );
		ADD_METHOD( GetTimeSignatures );
		ADD_METHOD( GetTickcounts );
		ADD_METHOD( GetSpeeds );
		ADD_METHOD( GetScrolls );
		ADD_METHOD( GetCombos );
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
