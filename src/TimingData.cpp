#include "global.h"
#include "TimingData.h"
#include "PrefsManager.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "NoteTypes.h"
#include "Foreach.h"
#include <float.h>

TimingSegment* GetSegmentAtRow( int iNoteRow, TimingSegmentType tst );

TimingData::TimingData(float fOffset) : m_fBeat0OffsetInSeconds(fOffset)
{
}

void TimingData::Copy( const TimingData& cpy )
{
	/* de-allocate any old pointers we had */
	Clear();

	m_fBeat0OffsetInSeconds = cpy.m_fBeat0OffsetInSeconds;
	m_sFile = cpy.m_sFile;

	FOREACH_TimingSegmentType( tst )
	{
		const vector<TimingSegment*> &vpSegs = cpy.m_avpTimingSegments[tst];

		for( unsigned i = 0; i < vpSegs.size(); ++i )
			AddSegment( vpSegs[i] );
	}
}

void TimingData::Clear()
{
	/* Delete all pointers owned by this TimingData. */
	FOREACH_TimingSegmentType( tst )
	{
		vector<TimingSegment*> &vSegs = m_avpTimingSegments[tst];
		for( unsigned i = 0; i < vSegs.size(); ++i )
		{
			SAFE_DELETE( vSegs[i] );
		}

		vSegs.clear();
	}
}

TimingData::~TimingData()
{
	Clear();
}

bool TimingData::empty() const
{
	FOREACH_TimingSegmentType( tst )
		if( !GetTimingSegments(tst).empty() )
			return false;

	return true;
}

TimingData TimingData::CopyRange(int startRow, int endRow) const
{
	TimingData ret;

	FOREACH_TimingSegmentType( tst )
	{
		const vector<TimingSegment*> &vSegs = GetTimingSegments(tst);

		for (unsigned i = 0; i < vSegs.size(); i++)
		{
			const TimingSegment *seg = vSegs[i];
			int row = seg->GetRow();

			if (row >= startRow && row < endRow)
			{
				TimingSegment *cpy = seg->Copy();

				// offset rows as though startRow were beat 0.
				cpy->SetRow(seg->GetRow() - startRow);
				ret.AddSegment(cpy);
			}
		}
	}

	return ret;
}

void TimingData::GetActualBPM( float &fMinBPMOut, float &fMaxBPMOut, float highest ) const
{
	fMinBPMOut = FLT_MAX;
	fMaxBPMOut = 0;
	const vector<TimingSegment*> &bpms = GetTimingSegments(SEGMENT_BPM);

	for (unsigned i = 0; i < bpms.size(); i++)
	{
		const float fBPM = ToBPM(bpms[i])->GetBPM();
		fMaxBPMOut = clamp(max( fBPM, fMaxBPMOut ), 0, highest);
		fMinBPMOut = min( fBPM, fMinBPMOut );
	}
}

float TimingData::GetNextSegmentBeatAtRow(TimingSegmentType tst, int row) const
{
	const vector<TimingSegment *> segs = GetTimingSegments(tst);
	for (unsigned i = 0; i < segs.size(); i++ )
	{
		if( segs[i]->GetRow() <= row )
		{
			continue;
		}
		return segs[i]->GetBeat();
	}
	return NoteRowToBeat(row);
}

float TimingData::GetPreviousSegmentBeatAtRow(TimingSegmentType tst, int row) const
{
	float backup = -1;
	const vector<TimingSegment *> segs = GetTimingSegments(tst);
	for (unsigned i = 0; i < segs.size(); i++ )
	{
		if( segs[i]->GetRow() >= row )
		{
			break;
		}
		backup = segs[i]->GetBeat();
	}
	return (backup > -1) ? backup : NoteRowToBeat(row);
}

static const int INVALID_INDEX = -1;

int TimingData::GetSegmentIndexAtRow(TimingSegmentType tst, int iRow ) const
{
	const vector<TimingSegment*> &vSegs = GetTimingSegments(tst);

	if( vSegs.empty() )
		return INVALID_INDEX;

	int min = 0, max = vSegs.size() - 1;
	int l = min, r = max;
	while( l <= r )
	{
		int m = ( l + r ) / 2;
		if( ( m == min || vSegs[m]->GetRow() <= iRow ) && ( m == max || iRow < vSegs[m + 1]->GetRow() ) )
		{
			return m;
		}
		else if( vSegs[m]->GetRow() <= iRow )
		{
			l = m + 1;
		}
		else
		{
			r = m - 1;
		}
	}
	
	// iRow is before the first segment of type tst
	return INVALID_INDEX;
}

struct ts_less : binary_function <TimingSegment*, TimingSegment*, bool>
{
	bool operator() (const TimingSegment *x, const TimingSegment *y) const
	{
		return (*x) < (*y);
	}
};

// Multiply the BPM in the range [fStartBeat,fEndBeat) by fFactor.
void TimingData::MultiplyBPMInBeatRange( int iStartIndex, int iEndIndex, float fFactor )
{
	// Change all other BPM segments in this range.
	vector<TimingSegment *> &bpms = m_avpTimingSegments[SEGMENT_BPM];
	for( unsigned i=0; i<bpms.size(); i++ )
	{
		BPMSegment *bs = ToBPM(bpms[i]);
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

bool TimingData::IsWarpAtRow( int iNoteRow ) const
{
	const vector<TimingSegment *> &warps = GetTimingSegments(SEGMENT_WARP);
	if( warps.empty() )
		return false;

	int i = GetSegmentIndexAtRow( SEGMENT_WARP, iNoteRow );
	if (i == -1)
	{
		return false;
	}
	const WarpSegment *s = ToWarp(warps[i]);
	float beatRow = NoteRowToBeat(iNoteRow);
	if( s->GetBeat() <= beatRow && beatRow < (s->GetBeat() + s->GetLength() ) )
	{
		// Allow stops inside warps to allow things like stop, warp, stop, warp, stop, and so on.
		if( GetTimingSegments(SEGMENT_STOP).empty() && GetTimingSegments(SEGMENT_DELAY).empty() )
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
	const vector<TimingSegment *> &fakes = GetTimingSegments(SEGMENT_FAKE);
	if( fakes.empty() )
		return false;

	int i = GetSegmentIndexAtRow( SEGMENT_FAKE, iNoteRow );
	if (i == -1)
	{
		return false;
	}
	const FakeSegment *s = ToFake(fakes[i]);
	float beatRow = NoteRowToBeat(iNoteRow);
	if( s->GetBeat() <= beatRow && beatRow < ( s->GetBeat() + s->GetLength() ) )
	{
		return true;
	}
	return false;
}

/* DummySegments: since our model relies on being able to get a segment at will,
 * whether one exists or not, we have a bunch of dummies to return if there is
 * no segment. It's kind of kludgy, but when we have functions making
 * indiscriminate calls to get segments at arbitrary rows, I think it's the
 * best solution we've got for now.
 *
 * Note that types whose SegmentEffectAreas are "Indefinite" are NULL here,
 * because they should never need to be used; we always have at least one such
 * segment in the TimingData, and if not, we'll crash anyway. -- vyhd */
static const TimingSegment* DummySegments[NUM_TimingSegmentType] =
{
	NULL, // BPMSegment
	new StopSegment,
	new DelaySegment,
	NULL, // TimeSignatureSegment
	new WarpSegment,
	NULL, // LabelSegment
	NULL, // TickcountSegment
	NULL, // ComboSegment
	NULL, // SpeedSegment
	NULL, // ScrollSegment
	new FakeSegment
};

const TimingSegment* TimingData::GetSegmentAtRow( int iNoteRow, TimingSegmentType tst ) const
{
	const vector<TimingSegment*> &vSegments = GetTimingSegments(tst);

	if( vSegments.empty() )
		return DummySegments[tst];

	int index = GetSegmentIndexAtRow( tst, iNoteRow );
	const TimingSegment *seg = vSegments[index];

	switch( seg->GetEffectType() )
	{
		case SegmentEffectType_Indefinite:
		{
			// this segment is in effect at this row
			return seg;
		}
		default:
		{
			// if the returned segment isn't exactly on this row,
			// we don't want it, return a dummy instead
			if( seg->GetRow() == iNoteRow )
				return seg;
			else
				return DummySegments[tst];
		}
	}

	FAIL_M("Could not find timing segment for row");
}

TimingSegment* GetSegmentAtRow( int iNoteRow, TimingSegmentType tst )
{
	return const_cast<TimingSegment*>( GetSegmentAtRow(iNoteRow, tst) );
}

static void EraseSegment( vector<TimingSegment*> &vSegs, int index, TimingSegment *cur )
{
#ifdef DEBUG
	LOG->Trace( "EraseSegment(%d, %p)", index, cur );
	cur->DebugPrint();
#endif

	vSegs.erase( vSegs.begin() + index );
	SAFE_DELETE( cur );
}

// NOTE: the pointer we're passed is a reference to a temporary,
// so we must deep-copy it (with ::Copy) for new allocations.
void TimingData::AddSegment( const TimingSegment *seg )
{
#ifdef DEBUG
	LOG->Trace( "AddSegment( %s )", TimingSegmentTypeToString(seg->GetType()).c_str() );
	seg->DebugPrint();
#endif

	TimingSegmentType tst = seg->GetType();
	vector<TimingSegment*> &vSegs = m_avpTimingSegments[tst];

	// OPTIMIZATION: if this is our first segment, push and return.
	if( vSegs.empty() )
	{
		vSegs.push_back( seg->Copy() );
		return;
	}

	int index = GetSegmentIndexAtRow( tst, seg->GetRow() );
	ASSERT( index != INVALID_INDEX );
	TimingSegment *cur = vSegs[index];

	bool bIsNotable = seg->IsNotable();
	bool bOnSameRow = seg->GetRow() == cur->GetRow();

	// ignore changes that are zero and don't overwrite an existing segment
	if( !bIsNotable && !bOnSameRow )
		return;

	switch( seg->GetEffectType() )
	{
		case SegmentEffectType_Row:
		case SegmentEffectType_Range:
		{
			// if we're overwriting a change with a non-notable
			// one, take it to mean deleting the existing segment
			if( bOnSameRow && !bIsNotable )
			{
				EraseSegment( vSegs, index, cur );
				return;
			}

			break;
		}
		case SegmentEffectType_Indefinite:
		{
			TimingSegment *prev = cur;

			// get the segment before last; if we're on the same
			// row, get the segment in effect before 'cur'
			if( bOnSameRow && index > 0 )
				prev = vSegs[index - 1];

			// if true, this is redundant segment change
			if( (*prev) == (*seg) )
			{
				if( prev != cur )
					EraseSegment( vSegs, index, cur );
				return;
			}

			break;
		}
	}

	// the segment at or before this row is equal to the new one; ignore it
	if( bOnSameRow && (*cur) == (*seg) )
	{
#if defined(DEBUG)
		LOG->Trace( "equals previous segment, ignoring" );
#endif
		return;
	}

	// Copy() the segment (which allocates a new segment), assign it
	// to the position of the old one, then delete the old pointer.
	TimingSegment *cpy = seg->Copy();

	if( bOnSameRow )
	{
		// delete the existing pointer and replace it
		SAFE_DELETE( cur );
		vSegs[index] = cpy;
	}
	else
	{
		// copy and insert a new segment
		vector<TimingSegment*>::iterator it;
		it = upper_bound( vSegs.begin(), vSegs.end(), cpy, ts_less() );
		vSegs.insert( it, cpy );
	}
}

bool TimingData::DoesLabelExist( const RString& sLabel ) const
{
	const vector<TimingSegment *> &labels = GetTimingSegments(SEGMENT_LABEL);
	for (unsigned i = 0; i < labels.size(); i++)
	{
		if (ToLabel(labels[i])->GetLabel() == sLabel)
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
	FOUND_DELAY,
	FOUND_STOP_DELAY, // we have these two on the same row.
	FOUND_MARKER,
	NOT_FOUND
};

void TimingData::GetBeatAndBPSFromElapsedTimeNoOffset( float fElapsedTime, float &fBeatOut, float &fBPSOut, bool &bFreezeOut, bool &bDelayOut, int &iWarpBeginOut, float &fWarpDestinationOut ) const
{
	const vector<TimingSegment *> * segs = m_avpTimingSegments;
	vector<TimingSegment *>::const_iterator itBPMS = segs[SEGMENT_BPM].begin();
	vector<TimingSegment *>::const_iterator itWS   = segs[SEGMENT_WARP].begin();
	vector<TimingSegment *>::const_iterator itSS   = segs[SEGMENT_STOP].begin();
	vector<TimingSegment *>::const_iterator itDS   = segs[SEGMENT_DELAY].begin();

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
		if (itDS != segs[SEGMENT_DELAY].end() &&
			(*itDS)->GetRow() < iEventRow)
		{
			iEventRow = (*itDS)->GetRow();
			iEventType = FOUND_DELAY;
		}
		if (itSS != segs[SEGMENT_STOP].end() &&
			(*itSS)->GetRow() < iEventRow ) // && iEventType != FOUND_DELAY )
		{
			int tmpRow = iEventRow;
			iEventRow = (*itSS)->GetRow();
			iEventType = (tmpRow == iEventRow) ? FOUND_STOP_DELAY : FOUND_STOP;
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
				fBPS = ToBPM(*itBPMS)->GetBPS();
				itBPMS ++;
				break;
			case FOUND_DELAY:
			case FOUND_STOP_DELAY:
			{
				const DelaySegment *ss = ToDelay(*itDS);
				fTimeToNextEvent = ss->GetPause();
				fNextEventTime   = fLastTime + fTimeToNextEvent;
				if ( fElapsedTime < fNextEventTime )
				{
					bFreezeOut = false;
					bDelayOut  = true;
					fBeatOut   = ss->GetBeat();
					fBPSOut    = fBPS;
					return;
				}
				fLastTime = fNextEventTime;
				itDS ++;
				if (iEventType == FOUND_DELAY)
					break;
			}
			case FOUND_STOP:
			{
				const StopSegment *ss = ToStop(*itSS);
				fTimeToNextEvent = ss->GetPause();
				fNextEventTime   = fLastTime + fTimeToNextEvent;
				if ( fElapsedTime < fNextEventTime )
				{
					bFreezeOut = true;
					bDelayOut  = false;
					fBeatOut   = ss->GetBeat();
					fBPSOut    = fBPS;
					return;
				}
				fLastTime = fNextEventTime;
				itSS ++;
				break;
			}
			case FOUND_WARP:
			{
				bIsWarping = true;
				const WarpSegment *ws = ToWarp(*itWS);
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
	const vector<TimingSegment *> * segs = m_avpTimingSegments;
	vector<TimingSegment *>::const_iterator itBPMS = segs[SEGMENT_BPM].begin();
	vector<TimingSegment *>::const_iterator itWS   = segs[SEGMENT_WARP].begin();
	vector<TimingSegment *>::const_iterator itSS   = segs[SEGMENT_STOP].begin();
	vector<TimingSegment *>::const_iterator itDS   = segs[SEGMENT_DELAY].begin();
	
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
		if (itDS != segs[SEGMENT_DELAY].end() &&
			(*itDS)->GetRow() < iEventRow ) // delays (come before marker)
		{
			iEventRow = (*itDS)->GetRow();
			iEventType = FOUND_DELAY;
		}
		if( BeatToNoteRow(fBeat) < iEventRow )
		{
			iEventRow = BeatToNoteRow(fBeat);
			iEventType = FOUND_MARKER;
		}
		if (itSS != segs[SEGMENT_STOP].end() &&
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
			fBPS = ToBPM(*itBPMS)->GetBPS();
			itBPMS ++;
			break;
		case FOUND_STOP:
			fTimeToNextEvent = ToStop(*itSS)->GetPause();
			fNextEventTime   = fLastTime + fTimeToNextEvent;
			fLastTime = fNextEventTime;
			itSS ++;
			break;
		case FOUND_DELAY:
			fTimeToNextEvent = ToDelay(*itDS)->GetPause();
			fNextEventTime   = fLastTime + fTimeToNextEvent;
			fLastTime = fNextEventTime;
			itDS ++;
			break;
		case FOUND_MARKER:
			return fLastTime;	
		case FOUND_WARP:
			{
				bIsWarping = true;
				WarpSegment *ws = ToWarp(*itWS);
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
	float fOutBeat = 0;
	unsigned i;
	const vector<TimingSegment *> &scrolls = m_avpTimingSegments[SEGMENT_SCROLL];
	for( i=0; i<scrolls.size()-1; i++ )
	{
		if( scrolls[i+1]->GetBeat() > fBeat )
			break;
		fOutBeat += (scrolls[i+1]->GetBeat() - scrolls[i]->GetBeat()) * ToScroll(scrolls[i])->GetRatio();
	}
	fOutBeat += (fBeat - scrolls[i]->GetBeat()) * ToScroll(scrolls[i])->GetRatio();
	return fOutBeat;
}

void TimingData::ScaleRegion( float fScale, int iStartIndex, int iEndIndex, bool bAdjustBPM )
{
	ASSERT( fScale > 0 );
	ASSERT( iStartIndex >= 0 );
	ASSERT( iStartIndex < iEndIndex );

	int length = iEndIndex - iStartIndex;
	int newLength = lrintf( fScale * length );

	FOREACH_TimingSegmentType( tst )
		for (unsigned j = 0; j < m_avpTimingSegments[tst].size(); j++)
			m_avpTimingSegments[tst][j]->Scale(iStartIndex, length, newLength);

	// adjust BPM changes to preserve timing
	if( bAdjustBPM )
	{
		int iNewEndIndex = iStartIndex + newLength;
		float fEndBPMBeforeScaling = GetBPMAtRow(iNewEndIndex);
		vector<TimingSegment *> &bpms = m_avpTimingSegments[SEGMENT_BPM];

		// adjust BPM changes "between" iStartIndex and iNewEndIndex
		for ( unsigned i = 0; i < bpms.size(); i++ )
		{
			BPMSegment *bpm = ToBPM(bpms[i]);
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
	FOREACH_TimingSegmentType( tst )
	{
		vector<TimingSegment *> &segs = m_avpTimingSegments[tst];
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
		vector<TimingSegment *> &bpms = m_avpTimingSegments[SEGMENT_BPM];
		ASSERT_M( bpms.size() > 0, "There must be at least one BPM Segment in the chart!" );
		bpms[0]->SetRow(0);
	}
}

// Delete BPMChanges and StopSegments in [iStartRow,iRowsToDelete), and shift down.
void TimingData::DeleteRows( int iStartRow, int iRowsToDelete )
{
	/* Remember the BPM at the end of the region being deleted. */
	float fNewBPM = GetBPMAtBeat( NoteRowToBeat(iStartRow+iRowsToDelete) );

	/* We're moving rows up. Delete any BPM changes and stops in the region
	 * being deleted. */
	FOREACH_TimingSegmentType( tst )
	{
		vector<TimingSegment *> &segs = m_avpTimingSegments[tst];
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

	SetBPMAtRow( iStartRow, fNewBPM );
}

float TimingData::GetDisplayedSpeedPercent( float fSongBeat, float fMusicSeconds ) const
{
	/* HACK: Somehow we get called into this function when there is no
	 * TimingData to work with. This seems to happen the most upon
	 * leaving the editor. Still, cover our butts in case this instance
	 * isn't existing. */
	/* ...but force a crash, so debuggers will catch it and stop here.
	 * That'll make us keep this bug in mind. -- vyhd */
	if( !this )
	{
		DEBUG_ASSERT( this );
		return 1.0f;
	}

	const vector<TimingSegment *> &speeds = GetTimingSegments(SEGMENT_SPEED);
	if( speeds.size() == 0 )
	{
#ifdef DEBUG
		LOG->Trace("No speed segments");
#endif
		return 1.0f;
	}

	const int index = GetSegmentIndexAtBeat( SEGMENT_SPEED, fSongBeat );

	const SpeedSegment *seg = ToSpeed(speeds[index]);
	float fStartBeat = seg->GetBeat();
	float fStartTime = GetElapsedTimeFromBeat( fStartBeat ) - GetDelayAtBeat( fStartBeat );
	float fEndTime;
	float fCurTime = fMusicSeconds;

	if( seg->GetUnit() == SpeedSegment::UNIT_SECONDS )
	{
		fEndTime = fStartTime + seg->GetDelay();
	}
	else
	{
		fEndTime = GetElapsedTimeFromBeat( fStartBeat + seg->GetDelay() )
			- GetDelayAtBeat( fStartBeat + seg->GetDelay() );
	}

	SpeedSegment *first = ToSpeed(speeds[0]);

	if( ( index == 0 && first->GetDelay() > 0.0 ) && fCurTime < fStartTime )
	{
		return 1.0f;
	}
	else if( fEndTime >= fCurTime && ( index > 0 || first->GetDelay() > 0.0 ) )
	{
		const float fPriorSpeed = (index == 0) ? 1 :
			ToSpeed(speeds[index-1])->GetRatio();

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

void TimingData::TidyUpData(bool allowEmpty)
{
	// Empty TimingData is used to implement steps with no timing of their
	// own.  Don't override this.
	if( allowEmpty && empty() )
		return;

	// If there are no BPM segments, provide a default.
	vector<TimingSegment *> *segs = m_avpTimingSegments;
	if( segs[SEGMENT_BPM].empty() )
	{
		LOG->UserLog( "Song file", m_sFile, "has no BPM segments, default provided." );
		AddSegment( BPMSegment(0, 60) );
	}

	// Make sure the first BPM segment starts at beat 0.
	if( segs[SEGMENT_BPM][0]->GetRow() != 0 )
		segs[SEGMENT_BPM][0]->SetRow(0);

	// If no time signature specified, assume default time for the whole song.
	if( segs[SEGMENT_TIME_SIG].empty() )
		AddSegment( TimeSignatureSegment(0) );

	// Likewise, if no tickcount signature is specified, assume 4 ticks
	// per beat for the entire song. The default of 4 is chosen more
	// for compatibility with the main Pump series than anything else.
	// (TickcountSegment's constructor handles that now. -- vyhd)
	if( segs[SEGMENT_TICKCOUNT].empty() )
		AddSegment( TickcountSegment(0) );

	// Have a default combo segment of one just in case.
	if( segs[SEGMENT_COMBO].empty() )
		AddSegment( ComboSegment(0) );

	// Have a default label segment just in case.
	if( segs[SEGMENT_LABEL].empty() )
		AddSegment( LabelSegment(0, "Song Start") );

	// Always be sure there is a starting speed.
	if( segs[SEGMENT_SPEED].empty() )
		AddSegment( SpeedSegment(0) );

	// Always be sure there is a starting scrolling factor.
	if( segs[SEGMENT_SCROLL].empty() )
		AddSegment( ScrollSegment(0) );
}

void TimingData::SortSegments( TimingSegmentType tst )
{
	vector<TimingSegment*> &vSegments = m_avpTimingSegments[tst];
	sort( vSegments.begin(), vSegments.end() );
}

bool TimingData::HasSpeedChanges() const
{
	const vector<TimingSegment *> &speeds = GetTimingSegments(SEGMENT_SPEED);
	return (speeds.size()>1 || ToSpeed(speeds[0])->GetRatio() != 1);
}

bool TimingData::HasScrollChanges() const
{
	const vector<TimingSegment *> &scrolls = GetTimingSegments(SEGMENT_SCROLL);
	return (scrolls.size()>1 || ToScroll(scrolls[0])->GetRatio() != 1);
}

void TimingData::NoteRowToMeasureAndBeat( int iNoteRow, int &iMeasureIndexOut, int &iBeatIndexOut, int &iRowsRemainder ) const
{
	iMeasureIndexOut = 0;
	const vector<TimingSegment *> &tSigs = GetTimingSegments(SEGMENT_TIME_SIG);
	for (unsigned i = 0; i < tSigs.size(); i++)
	{
		TimeSignatureSegment *curSig = ToTimeSignature(tSigs[i]);
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

	FAIL_M("Failed to get measure and beat for note row");
}

vector<RString> TimingData::ToVectorString(TimingSegmentType tst, int dec) const
{
	const vector<TimingSegment *> segs = GetTimingSegments(tst);
	vector<RString> ret;

	for (unsigned i = 0; i < segs.size(); i++)
	{
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
	static int HasDelays( T* p, lua_State *L )		{ lua_pushboolean(L, p->HasDelays()); return 1; }
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
		LuaHelpers::CreateTableFromArray(p->ToVectorString(SEGMENT_STOP), L);
		return 1;
	}
	static int GetDelays( T* p, lua_State *L )
	{
		LuaHelpers::CreateTableFromArray(p->ToVectorString(SEGMENT_DELAY), L);
		return 1;
	}
	static int GetBPMs( T* p, lua_State *L )
	{
		vector<float> vBPMs;
		const vector<TimingSegment *> &bpms = p->GetTimingSegments(SEGMENT_BPM);

		for (unsigned i = 0; i < bpms.size(); i++)
			vBPMs.push_back( ToBPM(bpms[i])->GetBPM() );

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
		ADD_METHOD( HasDelays );
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
