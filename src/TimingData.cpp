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

struct ts_less : binary_function <TimingSegment *, TimingSegment *, bool> {
	bool operator() (const TimingSegment *x, const TimingSegment *y) const {
		return (*x) < (*y);
	}
};

void TimingData::AddSegment(TimingSegmentType tst, TimingSegment * seg)
{
	vector<TimingSegment *> &segs = this->allTimingSegments[tst];
	// Unsure if this uses the proper comparison.
	segs.insert(upper_bound(segs.begin(), segs.end(), seg, ts_less()), seg);
}

int TimingData::GetSegmentIndexAtRow(TimingSegmentType tst,
									 int row, bool isDelay) const
{
	const vector<TimingSegment *> &segs = this->allTimingSegments[tst];
	unsigned i = 0;
	for (; i < segs.size() - 1; i++)
	{
		TimingSegment *seg = segs[i+1];
		if (seg->GetRow() > row)
		{
			// put conditions here for individual segments.
			if (tst == SEGMENT_STOP_DELAY &&
				static_cast<StopSegment *>(seg)->GetDelay() != isDelay)
				continue;
			break;
		}
	}
	return static_cast<int>(i);
}

float TimingData::GetNextSegmentBeatAtRow(TimingSegmentType tst,
										  int row, bool isDelay) const
{
	const vector<TimingSegment *> segs = this->allTimingSegments[tst];
	for (unsigned i = 0; i < segs.size(); i++ )
	{
		if( segs[i]->GetRow() <= row )
		{
			continue;
		}
		if (tst != SEGMENT_STOP_DELAY ||
			static_cast<StopSegment *>(segs[i])->GetDelay() == isDelay)
			return segs[i]->GetBeat();
	}
	return NoteRowToBeat(row);
}

float TimingData::GetPreviousSegmentBeatAtRow(TimingSegmentType tst,
											  int row, bool isDelay) const
{
	float backup = -1;
	const vector<TimingSegment *> segs = this->allTimingSegments[tst];
	for (unsigned i = 0; i < segs.size(); i++ )
	{
		if( segs[i]->GetRow() >= row )
		{
			break;
		}
		if (tst != SEGMENT_STOP_DELAY ||
			static_cast<StopSegment *>(segs[i])->GetDelay() == isDelay)
			backup = segs[i]->GetBeat();
	}
	return (backup > -1) ? backup : NoteRowToBeat(row);
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
								GetTimeSignatureSegmentAtRow(iRow)->GetDen());
}

void TimingData::SetTimeSignatureDenominatorAtRow( int iRow, int iDenominator )
{
	this->SetTimeSignatureAtRow(iRow,
								GetTimeSignatureSegmentAtRow(iRow)->GetNum(),
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
	LOG->Trace( "TimingData::SetTickcountAtRow( '%i' , '%i' )", iRow, iTicks );

	unsigned i = 0;
	vector<TimingSegment *> &ticks = this->allTimingSegments[SEGMENT_TICKCOUNT];
	for( i=0; i<ticks.size(); i++ )
		if( ticks[i]->GetRow() >= iRow )
			break;

	// this thing causes an invalid parameter crash
	TickcountSegment *ts = static_cast<TickcountSegment *>(ticks[i]);
	LOG->Trace( "Done with that thing again" );

	if( i == ticks.size() || ts->GetRow() != iRow )
	{
		LOG->Trace( "New TickSegment" );
		// No TickcountSegment here. Make a new segment if required.
		if (i == 0 ||
			static_cast<TickcountSegment *>(ticks[i-1])->GetTicks() != iTicks )
			AddSegment( SEGMENT_TICKCOUNT, new TickcountSegment(iRow, iTicks ) );
	}
	else	// TickcountSegment being modified is m_TickcountSegments[i]
	{
		LOG->Trace( "Editing TickSegment" );
		if (i > 0 &&
			static_cast<TickcountSegment *>(ticks[i-1])->GetTicks() == iTicks )
			ticks.erase( ticks.begin()+i, ticks.begin()+i+1 );
		else
			ts->SetTicks(iTicks);
	}

	LOG->Trace( "DONE" );
}

void TimingData::SetComboAtRow( int iRow, int iCombo, int iMiss )
{
	unsigned i;
	vector<TimingSegment *> &combos = this->allTimingSegments[SEGMENT_COMBO];
	for( i=0; i<combos.size(); i++ )
		if( combos[i]->GetRow() >= iRow )
			break;
	
	ComboSegment *cs = static_cast<ComboSegment *>(combos[i]);
	
	if( i == combos.size() || cs->GetRow() != iRow )
	{
		if (i == 0 ||
			static_cast<ComboSegment *>(combos[i-1])->GetCombo() != iCombo ||
			static_cast<ComboSegment *>(combos[i-1])->GetMissCombo() != iMiss)
			AddSegment( SEGMENT_COMBO, new ComboSegment(iRow, iCombo, iMiss ) );
	}
	else
	{
		if (i > 0 &&
			static_cast<ComboSegment *>(combos[i-1])->GetCombo() == iCombo &&
			static_cast<ComboSegment *>(combos[i-1])->GetMissCombo() == iMiss)
			combos.erase( combos.begin()+i, combos.begin()+i+1 );
		else
		{
			cs->SetCombo(iCombo);
			cs->SetMissCombo(iMiss);
		}
	}
}

void TimingData::SetHitComboAtRow(int iRow, int iCombo)
{
	this->SetComboAtRow(iRow,
						iCombo,
						this->GetComboSegmentAtRow(iRow)->GetMissCombo());
}

void TimingData::SetMissComboAtRow(int iRow, int iMiss)
{
	this->SetComboAtRow(iRow,
						this->GetComboSegmentAtRow(iRow)->GetCombo(),
						iMiss);
}

void TimingData::SetLabelAtRow( int iRow, const RString sLabel )
{
	unsigned i;
	vector<TimingSegment *> &labels = this->allTimingSegments[SEGMENT_LABEL];
	for( i=0; i<labels.size(); i++ )
		if( labels[i]->GetRow() >= iRow )
			break;
	
	LabelSegment *ls = static_cast<LabelSegment *>(labels[i]);
	
	if( i == labels.size() || ls->GetRow() != iRow )
	{
		if (i == 0 ||
			static_cast<LabelSegment *>(labels[i-1])->GetLabel() != sLabel )
			AddSegment( SEGMENT_LABEL, new LabelSegment(iRow, sLabel ) );
	}
	else
	{
		if (i > 0 &&
			( static_cast<LabelSegment *>(labels[i-1])->GetLabel() == sLabel ||
			 sLabel == "" ) )
			labels.erase( labels.begin()+i, labels.begin()+i+1 );
		else
			ls->SetLabel(sLabel);
	}
}

void TimingData::SetSpeedAtRow( int iRow, float fPercent, float fWait, unsigned short usMode )
{
	unsigned i;
	vector<TimingSegment *> &speeds = this->allTimingSegments[SEGMENT_SPEED];
	for( i = 0; i < speeds.size(); i++ )
	{
		if( speeds[i]->GetRow() >= iRow)
			break;
	}
	
	SpeedSegment *ss = static_cast<SpeedSegment *>(speeds[i]);
	
	if ( i == speeds.size() || ss->GetRow() != iRow )
	{
		// the core mod itself matters the most for comparisons.
		if (i == 0 ||
			static_cast<SpeedSegment *>(speeds[i-1])->GetRatio() != fPercent )
			AddSegment( SEGMENT_SPEED, new SpeedSegment(iRow, fPercent, fWait, usMode) );
	}
	else
	{
		// The others aren't compared: only the mod itself matters.
		if (i > 0 &&
			static_cast<SpeedSegment *>(speeds[i-1])->GetRatio() == fPercent )
			speeds.erase( speeds.begin()+i, speeds.begin()+i+1 );
		else
		{
			ss->SetRatio(fPercent);
			ss->SetLength(fWait);
			ss->SetUnit(usMode);
		}
	}
}

void TimingData::SetScrollAtRow( int iRow, float fPercent )
{
	unsigned i;
	vector<TimingSegment *> &scrolls = this->allTimingSegments[SEGMENT_SCROLL];
	for( i = 0; i < scrolls.size(); i++ )
	{
		if( scrolls[i]->GetRow() >= iRow)
			break;
	}
	
	ScrollSegment *ss = static_cast<ScrollSegment *>(scrolls[i]);
	if ( i == scrolls.size() || ss->GetRow() != iRow )
	{
		// the core mod itself matters the most for comparisons.
		if (i == 0 ||
			static_cast<ScrollSegment *>(scrolls[i-1])->GetRatio() != fPercent )
			AddSegment( SEGMENT_SCROLL, new ScrollSegment(iRow, fPercent) );
	}
	else
	{
		// The others aren't compared: only the mod itself matters.
		if (i > 0 &&
			static_cast<ScrollSegment *>(scrolls[i-1])->GetRatio() == fPercent )
			scrolls.erase( scrolls.begin()+i, scrolls.begin()+i+1 );
		else
		{
			ss->SetRatio(fPercent);
		}
	}
}

void TimingData::SetFakeAtRow( int iRow, float fNew )
{
	unsigned i;
	vector<TimingSegment *> &fakes = this->allTimingSegments[SEGMENT_FAKE];
	for( i=0; i<fakes.size(); i++ )
		if( fakes[i]->GetRow() == iRow )
			break;
	bool valid = iRow > 0 && fNew > 0;
	if( i == fakes.size() )
	{
		if( valid )
		{
			AddSegment( SEGMENT_FAKE, new FakeSegment(iRow, fNew) );
		}
	}
	else
	{
		if( valid )
		{
			static_cast<FakeSegment *>(fakes[i])->SetLength(fNew);
		}
		else
			fakes.erase( fakes.begin()+i, fakes.begin()+i+1 );
	}
}

void TimingData::SetSpeedPercentAtRow( int iRow, float fPercent )
{
	SetSpeedAtRow( iRow, 
		      fPercent, 
		      GetSpeedSegmentAtRow( iRow )->GetLength(),
		      GetSpeedSegmentAtRow( iRow )->GetUnit());
}

void TimingData::SetSpeedWaitAtRow( int iRow, float fWait )
{
	SetSpeedAtRow( iRow, 
		      GetSpeedSegmentAtRow( iRow )->GetRatio(),
		      fWait,
		      GetSpeedSegmentAtRow( iRow )->GetUnit());
}

void TimingData::SetSpeedModeAtRow( int iRow, unsigned short usMode )
{
	SetSpeedAtRow( iRow, 
		      GetSpeedSegmentAtRow( iRow )->GetRatio(),
		      GetSpeedSegmentAtRow( iRow )->GetLength(),
		      usMode );
}


float TimingData::GetStopAtRow( int iNoteRow, bool bDelay ) const
{
	const vector<TimingSegment *> &stops = this->allTimingSegments[SEGMENT_STOP_DELAY];
	for( unsigned i=0; i<stops.size(); i++ )
	{
		const StopSegment *s = static_cast<StopSegment *>(stops[i]);
		if( s->GetDelay() == bDelay && s->GetRow() == iNoteRow )
		{
			return s->GetPause();
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
	const vector<TimingSegment *> &c = this->allTimingSegments[SEGMENT_COMBO];
	const int index = this->GetSegmentIndexAtRow(SEGMENT_COMBO, iNoteRow);
	return static_cast<ComboSegment *>(c[index])->GetCombo();
}

int TimingData::GetMissComboAtRow(int iNoteRow) const
{
	const vector<TimingSegment *> &c = this->allTimingSegments[SEGMENT_COMBO];
	const int index = this->GetSegmentIndexAtRow(SEGMENT_COMBO, iNoteRow);
	return static_cast<ComboSegment *>(c[index])->GetMissCombo();
}

RString TimingData::GetLabelAtRow( int iRow ) const
{
	const vector<TimingSegment *> &l = this->allTimingSegments[SEGMENT_LABEL];
	const int index = this->GetSegmentIndexAtRow(SEGMENT_LABEL, iRow);
	return static_cast<LabelSegment *>(l[index])->GetLabel();
}

float TimingData::GetWarpAtRow( int iWarpRow ) const
{
	const vector<TimingSegment *> &warps = this->allTimingSegments[SEGMENT_WARP];
	for( unsigned i=0; i<warps.size(); i++ )
	{
		if( warps[i]->GetRow() == iWarpRow )
		{
			return static_cast<WarpSegment *>(warps[i])->GetLength();
		}
	}
	return 0;
}

float TimingData::GetSpeedPercentAtRow( int iRow )
{
	return GetSpeedSegmentAtRow( iRow )->GetRatio();
}

float TimingData::GetSpeedWaitAtRow( int iRow )
{
	return GetSpeedSegmentAtRow( iRow )->GetLength();
}

unsigned short TimingData::GetSpeedModeAtRow( int iRow )
{
	return GetSpeedSegmentAtRow( iRow )->GetUnit();
}

float TimingData::GetScrollAtRow( int iRow )
{
	return GetScrollSegmentAtRow( iRow )->GetRatio();
}

float TimingData::GetFakeAtRow( int iFakeRow ) const
{
	const vector<TimingSegment *> &fakes = this->allTimingSegments[SEGMENT_FAKE];
	for( unsigned i=0; i<fakes.size(); i++ )
	{
		if( fakes[i]->GetRow() == iFakeRow )
		{
			return static_cast<FakeSegment *>(fakes[i])->GetLength();
		}
	}
	return 0;
}

// Multiply the BPM in the range [fStartBeat,fEndBeat) by fFactor.
void TimingData::MultiplyBPMInBeatRange( int iStartIndex, int iEndIndex, float fFactor )
{
	// Change all other BPM segments in this range.
	vector<TimingSegment *> &bpms = this->allTimingSegments[SEGMENT_BPM];
	for( unsigned i=0; i<bpms.size(); i++ )
	{
		BPMSegment *bs = static_cast<BPMSegment *>(bpms[i]);
		const int iStartIndexThisSegment = bs->GetRow();
		const bool bIsLastBPMSegment = i == bpms.size()-1;
		const int iStartIndexNextSegment = bIsLastBPMSegment ? INT_MAX : bpms[i+1]->GetRow();

		if( iStartIndexThisSegment <= iStartIndex && iStartIndexNextSegment <= iStartIndex )
			continue;

		/* If this BPM segment crosses the beginning of the range,
		 * split it into two. */
		if( iStartIndexThisSegment < iStartIndex && iStartIndexNextSegment > iStartIndex )
		{
			
			BPMSegment * b = new BPMSegment(iStartIndexNextSegment,
											bs->GetBPS());
			bpms.insert(bpms.begin()+i+1, b);

			/* Don't apply the BPM change to the first half of the segment we
			 * just split, since it lies outside the range. */
			continue;
		}

		// If this BPM segment crosses the end of the range, split it into two.
		if( iStartIndexThisSegment < iEndIndex && iStartIndexNextSegment > iEndIndex )
		{
			BPMSegment * b = new BPMSegment(iEndIndex,
											bs->GetBPS());
			bpms.insert(bpms.begin()+i+1, b);
		}
		else if( iStartIndexNextSegment > iEndIndex )
			continue;

		bs->SetBPM(bs->GetBPM() * fFactor);
	}
}

float TimingData::GetBPMAtRow( int iNoteRow ) const
{
	unsigned i;
	const vector<TimingSegment *> &bpms = this->allTimingSegments[SEGMENT_BPM];
	for( i=0; i<bpms.size()-1; i++ )
		if( bpms[i+1]->GetRow() > iNoteRow )
			break;
	return static_cast<BPMSegment *>(bpms[i])->GetBPM();
}

bool TimingData::IsWarpAtRow( int iNoteRow ) const
{
	const vector<TimingSegment *> &warps = this->allTimingSegments[SEGMENT_WARP];
	if( warps.empty() )
		return false;
	
	int i = GetSegmentIndexAtRow( SEGMENT_WARP, iNoteRow );
	const WarpSegment *s = static_cast<WarpSegment *>(warps[i]);
	float beatRow = NoteRowToBeat(iNoteRow);
	if( s->GetBeat() <= beatRow && beatRow < (s->GetBeat() + s->GetLength() ) )
	{
		// Allow stops inside warps to allow things like stop, warp, stop, warp, stop, and so on.
		if( this->allTimingSegments[SEGMENT_STOP_DELAY].empty() )
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
	const vector<TimingSegment *> &fakes = this->allTimingSegments[SEGMENT_FAKE];
	if( fakes.empty() )
		return false;
	
	int i = GetSegmentIndexAtRow( SEGMENT_FAKE, iNoteRow );
	const FakeSegment *s = static_cast<FakeSegment *>(fakes[i]);
	float beatRow = NoteRowToBeat(iNoteRow);
	if( s->GetBeat() <= beatRow && beatRow < ( s->GetBeat() + s->GetLength() ) )
	{
		return true;
	}
	return false;
}

BPMSegment* TimingData::GetBPMSegmentAtRow( int iNoteRow )
{
	vector<TimingSegment *> &bpms = this->allTimingSegments[SEGMENT_BPM];
	static BPMSegment empty;
	if( bpms.empty() )
		return new BPMSegment();

	int i = GetSegmentIndexAtRow( SEGMENT_BPM, iNoteRow );
	return static_cast<BPMSegment *>(bpms[i]);
}

TimeSignatureSegment* TimingData::GetTimeSignatureSegmentAtRow( int iRow )
{
	unsigned i;
	vector<TimingSegment *> &tSigs = this->allTimingSegments[SEGMENT_TIME_SIG];
	for( i=0; i<tSigs.size()-1; i++ )
		if( tSigs[i+1]->GetRow() > iRow )
			break;
	return static_cast<TimeSignatureSegment *>(tSigs[i]);
}

SpeedSegment* TimingData::GetSpeedSegmentAtRow( int iRow )
{
	unsigned i;
	vector<TimingSegment *> &speeds = this->allTimingSegments[SEGMENT_SPEED];
	for( i=0; i<speeds.size()-1; i++ )
		if( speeds[i+1]->GetRow() > iRow )
			break;
	return static_cast<SpeedSegment *>(speeds[i]);
}

ScrollSegment* TimingData::GetScrollSegmentAtRow( int iRow )
{
	unsigned i;
	vector<TimingSegment *> &scrolls = this->allTimingSegments[SEGMENT_SCROLL];
	for( i=0; i<scrolls.size()-1; i++ )
		if( scrolls[i+1]->GetRow() > iRow )
			break;
	return static_cast<ScrollSegment *>(scrolls[i]);
}

int TimingData::GetTimeSignatureNumeratorAtRow( int iRow )
{
	return GetTimeSignatureSegmentAtRow( iRow )->GetNum();
}

int TimingData::GetTimeSignatureDenominatorAtRow( int iRow )
{
	return GetTimeSignatureSegmentAtRow( iRow )->GetDen();
}

ComboSegment* TimingData::GetComboSegmentAtRow( int iRow )
{
	unsigned i;
	vector<TimingSegment *> &combos = this->allTimingSegments[SEGMENT_COMBO];
	for( i=0; i<combos.size()-1; i++ )
		if( combos[i+1]->GetRow() > iRow )
			break;
	return static_cast<ComboSegment *>(combos[i]);
}

LabelSegment* TimingData::GetLabelSegmentAtRow( int iRow )
{
	unsigned i;
	vector<TimingSegment *> &labels = this->allTimingSegments[SEGMENT_LABEL];
	for( i=0; i<labels.size()-1; i++ )
		if( labels[i+1]->GetRow() > iRow )
			break;
	return static_cast<LabelSegment *>(labels[i]);
}

StopSegment* TimingData::GetStopSegmentAtRow( int iNoteRow, bool bDelay )
{
	vector<TimingSegment *> &stops = this->allTimingSegments[SEGMENT_STOP_DELAY];
	if( stops.empty() )
		return new StopSegment();
	
	int i = GetSegmentIndexAtRow( SEGMENT_STOP_DELAY, iNoteRow, bDelay );
	return static_cast<StopSegment *>(stops[i]);
}

WarpSegment* TimingData::GetWarpSegmentAtRow( int iRow )
{
	vector<TimingSegment *> &warps = this->allTimingSegments[SEGMENT_WARP];
	if( warps.empty() )
		return new WarpSegment();
	
	int i = GetSegmentIndexAtRow( SEGMENT_WARP, iRow );
	return static_cast<WarpSegment *>(warps[i]);
}

FakeSegment* TimingData::GetFakeSegmentAtRow( int iRow )
{
	vector<TimingSegment *> &fakes = this->allTimingSegments[SEGMENT_FAKE];
	if( fakes.empty() )
		return new FakeSegment();
	
	int i = GetSegmentIndexAtRow( SEGMENT_FAKE, iRow );
	return static_cast<FakeSegment *>(fakes[i]);
}

TickcountSegment* TimingData::GetTickcountSegmentAtRow( int iRow )
{
	vector<TimingSegment *> &ticks = this->allTimingSegments[SEGMENT_TICKCOUNT];
	if( ticks.empty() )
		return new TickcountSegment();
	
	int i = GetSegmentIndexAtRow( SEGMENT_TICKCOUNT, iRow );
	return static_cast<TickcountSegment *>(ticks[i]);
}

int TimingData::GetTickcountAtRow( int iRow ) const
{
	const vector<TimingSegment *> &ticks = this->allTimingSegments[SEGMENT_TICKCOUNT];
	const int index = GetSegmentIndexAtRow( SEGMENT_TICKCOUNT, iRow );
	return static_cast<TickcountSegment *>(ticks[index])->GetTicks();
}

bool TimingData::DoesLabelExist( RString sLabel ) const
{
	const vector<TimingSegment *> &labels = this->allTimingSegments[SEGMENT_LABEL];
	for (unsigned i = 0; i < labels.size(); i++)
	{
		if (static_cast<LabelSegment *>(labels[i])->GetLabel() == sLabel)
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
	const vector<TimingSegment *> * segs = this->allTimingSegments;
	vector<TimingSegment *>::const_iterator itBPMS = segs[SEGMENT_BPM].begin();
	vector<TimingSegment *>::const_iterator itWS   = segs[SEGMENT_WARP].begin();
	vector<TimingSegment *>::const_iterator itSS   = segs[SEGMENT_STOP_DELAY].begin();
	
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
		if (itBPMS != segs[SEGMENT_BPM].end() && 
			(*itBPMS)->GetRow() < iEventRow )
		{
			iEventRow = (*itBPMS)->GetRow();
			iEventType = FOUND_BPM_CHANGE;
		}
		if (itSS != segs[SEGMENT_STOP_DELAY].end() &&
			(*itSS)->GetRow() < iEventRow )
		{
			iEventRow = (*itSS)->GetRow();
			iEventType = FOUND_STOP;
		}
		if (itWS != segs[SEGMENT_WARP].end() &&
			(*itWS)->GetRow() < iEventRow )
		{
			iEventRow = (*itWS)->GetRow();
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
			fBPS = static_cast<BPMSegment *>(*itBPMS)->GetBPS();
			itBPMS ++;
			break;
		case FOUND_STOP:
			{
				const StopSegment *ss = static_cast<StopSegment *>(*itSS);
				fTimeToNextEvent = ss->GetPause();
				fNextEventTime   = fLastTime + fTimeToNextEvent;
				const bool bIsDelay = ss->GetDelay();
				if ( fElapsedTime < fNextEventTime )
				{
					bFreezeOut = !bIsDelay;
					bDelayOut  = bIsDelay;
					fBeatOut   = ss->GetBeat();
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
				const WarpSegment *ws = static_cast<WarpSegment *>(*itWS);
				float fWarpSum = ws->GetLength() + ws->GetBeat();
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
	const vector<TimingSegment *> * segs = this->allTimingSegments;
	vector<TimingSegment *>::const_iterator itBPMS = segs[SEGMENT_BPM].begin();
	vector<TimingSegment *>::const_iterator itWS   = segs[SEGMENT_WARP].begin();
	vector<TimingSegment *>::const_iterator itSS   = segs[SEGMENT_STOP_DELAY].begin();
	
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
		if (itBPMS != segs[SEGMENT_BPM].end() &&
			(*itBPMS)->GetRow() < iEventRow )
		{
			iEventRow = (*itBPMS)->GetRow();
			iEventType = FOUND_BPM_CHANGE;
		}
		if (itSS != segs[SEGMENT_STOP_DELAY].end() &&
			static_cast<StopSegment *>(*itSS)->GetDelay() &&
			(*itSS)->GetRow() < iEventRow ) // delays (come before marker)
		{
			iEventRow = (*itSS)->GetRow();
			iEventType = FOUND_STOP;
		}
		if( BeatToNoteRow(fBeat) < iEventRow )
		{
			iEventRow = BeatToNoteRow(fBeat);
			iEventType = FOUND_MARKER;
		}
		if (itSS != segs[SEGMENT_STOP_DELAY].end() &&
			!static_cast<StopSegment *>(*itSS)->GetDelay() &&
			(*itSS)->GetRow() < iEventRow ) // stops (come after marker)
		{
			iEventRow = (*itSS)->GetRow();
			iEventType = FOUND_STOP;
		}
		if (itWS != segs[SEGMENT_WARP].end() &&
			(*itWS)->GetRow() < iEventRow )
		{
			iEventRow = (*itWS)->GetRow();
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
			fBPS = static_cast<BPMSegment *>(*itBPMS)->GetBPS();
			itBPMS ++;
			break;
		case FOUND_STOP:
			fTimeToNextEvent = static_cast<StopSegment *>(*itSS)->GetPause();
			fNextEventTime   = fLastTime + fTimeToNextEvent;
			fLastTime = fNextEventTime;
			itSS ++;
			break;
		case FOUND_MARKER:
			return fLastTime;	
		case FOUND_WARP:
			{
				bIsWarping = true;
				WarpSegment *ws = static_cast<WarpSegment *>(*itWS);
				float fWarpSum = ws->GetLength() + ws->GetBeat();
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
	const vector<TimingSegment *> &scrolls = this->allTimingSegments[SEGMENT_SCROLL];
	vector<TimingSegment *>::const_iterator it = scrolls.begin(), end = scrolls.end();
	float fOutBeat = 0;
	for( it = it + 1; it != end; it++ )
	{
		ASSERT((*(it-1))->GetBeat() <= (*it)->GetBeat());
	}
	it = scrolls.begin();
	for( ; it != end; it++ )
	{
		if( it+1 == end || fBeat <= (*(it+1))->GetBeat() )
		{
			fOutBeat += ( fBeat - (*it)->GetBeat() ) *
				static_cast<ScrollSegment *>(*it)->GetRatio();
			break;
		}
		else
		{
			fOutBeat += ( (*(it+1))->GetBeat() - (*it)->GetBeat() ) *
				static_cast<ScrollSegment *>(*it)->GetRatio();
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
	
	// TODO: Confirm this works as intended.
	for (unsigned i = 0; i < NUM_TimingSegmentType; i++)
	{
		vector<TimingSegment *> &segs = this->allTimingSegments[i];
		for (unsigned j = 0; j < segs.size(); j++)
		{
			segs[i][j].Scale(iStartIndex, length, newLength);
		}
	}
	
	// adjust BPM changes to preserve timing
	if( bAdjustBPM )
	{
		int iNewEndIndex = iStartIndex + newLength;
		float fEndBPMBeforeScaling = GetBPMAtRow(iNewEndIndex);
		vector<TimingSegment *> &bpms = this->allTimingSegments[SEGMENT_BPM];
		
		// adjust BPM changes "between" iStartIndex and iNewEndIndex
		for ( unsigned i = 0; i < bpms.size(); i++ )
		{
			BPMSegment *bpm = static_cast<BPMSegment *>(bpms[i]);
			const int iSegStart = bpm->GetRow();
			if( iSegStart <= iStartIndex )
				continue;
			else if( iSegStart >= iNewEndIndex )
				continue;
			else
				bpm->SetBPM( bpm->GetBPM() * fScale );
		}
		
		// set BPM at iStartIndex and iNewEndIndex.
		SetBPMAtRow( iStartIndex, GetBPMAtRow(iStartIndex) * fScale );
		SetBPMAtRow( iNewEndIndex, fEndBPMBeforeScaling );
	}
	
}

void TimingData::InsertRows( int iStartRow, int iRowsToAdd )
{
	for (unsigned i = 0; i < NUM_TimingSegmentType; i++)
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
	for (unsigned i = 0; i < NUM_TimingSegmentType; i++)
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
	
	const vector<TimingSegment *> &speeds = this->allTimingSegments[SEGMENT_SPEED];
	if( speeds.size() == 0 )
		return 1.0f;

	const int index = GetSegmentIndexAtBeat( SEGMENT_SPEED, fSongBeat );
	
	const SpeedSegment *seg = static_cast<SpeedSegment *>(speeds[index]);
	float fStartBeat = seg->GetBeat();
	float fStartTime = GetElapsedTimeFromBeat( fStartBeat ) - GetDelayAtBeat( fStartBeat );
	float fEndTime;
	float fCurTime = fMusicSeconds;
	
	if( seg->GetUnit() == 1 ) // seconds
	{
		fEndTime = fStartTime + seg->GetLength();
	}
	else
	{
		fEndTime = GetElapsedTimeFromBeat( fStartBeat + seg->GetLength() ) 
		- GetDelayAtBeat( fStartBeat + seg->GetLength() );
	}
	
	SpeedSegment *first = static_cast<SpeedSegment *>(speeds[0]);
	
	if( ( index == 0 && first->GetLength() > 0.0 ) && fCurTime < fStartTime )
	{
		return 1.0f;
	}
	else if( fEndTime >= fCurTime && ( index > 0 || first->GetLength() > 0.0 ) )
	{
		const float fPriorSpeed = (index == 0 ?
								   1 :
								   static_cast<SpeedSegment *>(speeds[index - 1])->GetRatio() );
		float fTimeUsed = fCurTime - fStartTime;
		float fDuration = fEndTime - fStartTime;
		float fRatioUsed = fDuration == 0.0 ? 1 : fTimeUsed / fDuration;
		
		float fDistance = fPriorSpeed - seg->GetRatio();
		float fRatioNeed = fRatioUsed * -fDistance;
		return (fPriorSpeed + fRatioNeed);
	}
	else 
	{
		return seg->GetRatio();
	}

}

void TimingData::TidyUpData()
{
	// If there are no BPM segments, provide a default.
	vector<TimingSegment *> *segs = this->allTimingSegments;
	if( segs[SEGMENT_BPM].empty() )
	{
		LOG->UserLog( "Song file", m_sFile, "has no BPM segments, default provided." );

		AddSegment( SEGMENT_BPM, new BPMSegment(0, 60) );
	}

	// Make sure the first BPM segment starts at beat 0.
	if( segs[SEGMENT_BPM][0]->GetRow() != 0 )
		segs[SEGMENT_BPM][0]->SetRow(0);

	// If no time signature specified, assume 4/4 time for the whole song.
	if( segs[SEGMENT_TIME_SIG].empty() )
	{
		segs[SEGMENT_TIME_SIG].push_back( new TimeSignatureSegment(0, 4, 4) );
	}
	
	// Likewise, if no tickcount signature is specified, assume 4 ticks
	//per beat for the entire song. The default of 4 is chosen more
	//for compatibility with the main Pump series than anything else.
	if( segs[SEGMENT_TICKCOUNT].empty() )
	{
		segs[SEGMENT_TICKCOUNT].push_back( new TickcountSegment(0, 4) );
	}
	
	// Have a default combo segment of one just in case.
	if( segs[SEGMENT_COMBO].empty() )
	{
		segs[SEGMENT_COMBO].push_back( new ComboSegment(0, 1, 1) );
	}
	
	// Have a default label segment just in case.
	if( segs[SEGMENT_LABEL].empty() )
	{
		segs[SEGMENT_LABEL].push_back( new LabelSegment(0, "Song Start") );
	}
	
	// Always be sure there is a starting speed.
	if( segs[SEGMENT_SPEED].empty() )
	{
		segs[SEGMENT_SPEED].push_back( new SpeedSegment(0, 1, 0) );
	}
	
	// Always be sure there is a starting scrolling factor.
	if( segs[SEGMENT_SCROLL].empty() )
	{
		segs[SEGMENT_SCROLL].push_back( new ScrollSegment(0, 1) );
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
	const vector<TimingSegment *> &speeds = this->allTimingSegments[SEGMENT_SPEED];
	return (speeds.size()>1 || 
			static_cast<SpeedSegment *>(speeds[0])->GetRatio() != 1);
}

bool TimingData::HasScrollChanges() const
{
	const vector<TimingSegment *> &scrolls = this->allTimingSegments[SEGMENT_SCROLL];
	return (scrolls.size()>1 || 
			static_cast<ScrollSegment *>(scrolls[0])->GetRatio() != 1);
}

void TimingData::NoteRowToMeasureAndBeat( int iNoteRow, int &iMeasureIndexOut, int &iBeatIndexOut, int &iRowsRemainder ) const
{
	iMeasureIndexOut = 0;
	const vector<TimingSegment *> &tSigs = this->allTimingSegments[SEGMENT_TIME_SIG];
	for (unsigned i = 0; i < tSigs.size(); i++)
	{
		TimeSignatureSegment *curSig = static_cast<TimeSignatureSegment *>(tSigs[i]);
		int iSegmentEndRow = (i + 1 == tSigs.size()) ? INT_MAX : curSig->GetRow();
	
		int iRowsPerMeasureThisSegment = curSig->GetNoteRowsPerMeasure();

		if( iNoteRow >= curSig->GetRow() )
		{
			// iNoteRow lands in this segment
			int iNumRowsThisSegment = iNoteRow - curSig->GetRow();
			int iNumMeasuresThisSegment = (iNumRowsThisSegment) / iRowsPerMeasureThisSegment;	// don't round up
			iMeasureIndexOut += iNumMeasuresThisSegment;
			iBeatIndexOut = iNumRowsThisSegment / iRowsPerMeasureThisSegment;
			iRowsRemainder = iNumRowsThisSegment % iRowsPerMeasureThisSegment;
			return;
		}
		else
		{
			// iNoteRow lands after this segment
			int iNumRowsThisSegment = iSegmentEndRow - curSig->GetRow();
			int iNumMeasuresThisSegment = (iNumRowsThisSegment + iRowsPerMeasureThisSegment - 1)
				/ iRowsPerMeasureThisSegment;	// round up
			iMeasureIndexOut += iNumMeasuresThisSegment;
		}
	}

	ASSERT(0);
	return;
}

vector<RString> TimingData::ToVectorString(TimingSegmentType tst,
										   bool isDelay, int dec) const
{
	const vector<TimingSegment *> segs = this->allTimingSegments[tst];
	vector<RString> ret;
	
	for (unsigned i = 0; i < segs.size(); i++)
	{
		if (tst == SEGMENT_STOP_DELAY)
		{
			StopSegment *seg = static_cast<StopSegment *>(segs[i]);
			if (seg->GetDelay() != isDelay)
				continue;
		}
		ret.push_back(segs[i]->ToString(dec));
	}
	return ret;
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
		LuaHelpers::CreateTableFromArray(p->ToVectorString(SEGMENT_WARP), L);
		return 1;
	}
	static int GetFakes( T* p, lua_State *L )
	{
		LuaHelpers::CreateTableFromArray(p->ToVectorString(SEGMENT_FAKE), L);
		return 1;
	}
	static int GetScrolls( T* p, lua_State *L )
	{
		LuaHelpers::CreateTableFromArray(p->ToVectorString(SEGMENT_SCROLL), L);
		return 1;
	}
	static int GetSpeeds( T* p, lua_State *L )
	{
		LuaHelpers::CreateTableFromArray(p->ToVectorString(SEGMENT_SPEED), L);
		return 1;
	}
	static int GetTimeSignatures( T* p, lua_State *L )
	{
		LuaHelpers::CreateTableFromArray(p->ToVectorString(SEGMENT_TIME_SIG), L);
		return 1;
	}
	static int GetCombos( T* p, lua_State *L )
	{
		LuaHelpers::CreateTableFromArray(p->ToVectorString(SEGMENT_COMBO), L);
		return 1;
	}
	static int GetTickcounts( T* p, lua_State *L )
	{
		LuaHelpers::CreateTableFromArray(p->ToVectorString(SEGMENT_TICKCOUNT), L);
		return 1;
	}
	static int GetStops( T* p, lua_State *L )
	{
		LuaHelpers::CreateTableFromArray(p->ToVectorString(SEGMENT_STOP_DELAY, false), L);
		return 1;
	}
	static int GetDelays( T* p, lua_State *L )
	{
		LuaHelpers::CreateTableFromArray(p->ToVectorString(SEGMENT_STOP_DELAY, true), L);
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
		LuaHelpers::CreateTableFromArray(p->ToVectorString(SEGMENT_LABEL), L);
		return 1;
	}
	static int GetBPMsAndTimes( T* p, lua_State *L )
	{
		LuaHelpers::CreateTableFromArray(p->ToVectorString(SEGMENT_BPM), L);
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
