#include "global.h"
#include "TimingData.h"
#include "PrefsManager.h"
#include "GameState.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "NoteTypes.h"
#include <float.h>

static void EraseSegment(vector<TimingSegment*> &vSegs, int index, TimingSegment *cur);
static const int INVALID_INDEX = -1;

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

bool TimingData::IsSafeFullTiming()
{
	static vector<TimingSegmentType> needed_segments;
	if(needed_segments.empty())
	{
		needed_segments.push_back(SEGMENT_BPM);
		needed_segments.push_back(SEGMENT_TIME_SIG);
		needed_segments.push_back(SEGMENT_TICKCOUNT);
		needed_segments.push_back(SEGMENT_COMBO);
		needed_segments.push_back(SEGMENT_LABEL);
		needed_segments.push_back(SEGMENT_SPEED);
		needed_segments.push_back(SEGMENT_SCROLL);
	}
	for(size_t s= 0; s < needed_segments.size(); ++s)
	{
		if(m_avpTimingSegments[needed_segments[s]].empty())
		{
			return false;
		}
	}
	return true;
}

TimingData::~TimingData()
{
	Clear();
}

void TimingData::PrepareLookup()
{
	// If multiple players have the same timing data, then adding to the
	// lookups would probably cause FindEntryInLookup to return the wrong
	// thing.  So release the lookups. -Kyz
	ReleaseLookup();
	const unsigned int segments_per_lookup= 16;
	const vector<TimingSegment*>& bpms= m_avpTimingSegments[SEGMENT_BPM];
	const vector<TimingSegment*>& warps= m_avpTimingSegments[SEGMENT_WARP];
	const vector<TimingSegment*>& stops= m_avpTimingSegments[SEGMENT_STOP];
	const vector<TimingSegment*>& delays= m_avpTimingSegments[SEGMENT_DELAY];

	unsigned int total_segments= bpms.size() + warps.size() + stops.size() + delays.size();
	unsigned int lookup_entries= total_segments / segments_per_lookup;
	m_beat_start_lookup.reserve(lookup_entries);
	m_time_start_lookup.reserve(lookup_entries);
	for(unsigned int curr_segment= segments_per_lookup;
			curr_segment < total_segments; curr_segment+= segments_per_lookup)
	{
		GetBeatStarts beat_start;
		beat_start.last_time= -m_fBeat0OffsetInSeconds;
		GetBeatArgs args;
		args.elapsed_time= FLT_MAX;
		GetBeatInternal(beat_start, args, curr_segment);
		m_beat_start_lookup.push_back(lookup_item_t(args.elapsed_time, beat_start));

		GetBeatStarts time_start;
		time_start.last_time= -m_fBeat0OffsetInSeconds;
		float time= GetElapsedTimeInternal(time_start, FLT_MAX, curr_segment);
		m_time_start_lookup.push_back(lookup_item_t(NoteRowToBeat(time_start.last_row), time_start));
	}
	// If there are less than two entries, then FindEntryInLookup in lookup
	// will always decide there's no appropriate entry.  So clear the table.
	// -Kyz
	if(m_beat_start_lookup.size() < 2)
	{
		ReleaseLookup();
	}
	// DumpLookupTables();
}

void TimingData::ReleaseLookup()
{
	// According to The C++ Programming Language 3rd Ed., decreasing the size
	// of a vector doesn't actually free the memory it has allocated.  So this
	// small trick is required to actually free the memory. -Kyz
#define CLEAR_LOOKUP(lookup) \
	{ \
		lookup.clear(); \
		beat_start_lookup_t tmp= lookup; \
		lookup.swap(tmp); \
	}
	CLEAR_LOOKUP(m_beat_start_lookup);
	CLEAR_LOOKUP(m_time_start_lookup);
#undef CLEAR_LOOKUP
}

RString SegInfoStr(const vector<TimingSegment*>& segs, unsigned int index, const RString& name)
{
	if(index < segs.size())
	{
		return ssprintf("%s: %d at %d", name.c_str(), index, segs[index]->GetRow());
	}
	return ssprintf("%s: %d at end", name.c_str(), index);
}

void TimingData::DumpOneTable(const beat_start_lookup_t& lookup, const RString& name)
{
	const vector<TimingSegment*>& bpms= m_avpTimingSegments[SEGMENT_BPM];
	const vector<TimingSegment*>& warps= m_avpTimingSegments[SEGMENT_WARP];
	const vector<TimingSegment*>& stops= m_avpTimingSegments[SEGMENT_STOP];
	const vector<TimingSegment*>& delays= m_avpTimingSegments[SEGMENT_DELAY];
	LOG->Trace("%s lookup table:", name.c_str());
	for(size_t lit= 0; lit < lookup.size(); ++lit)
	{
		const lookup_item_t& item= lookup[lit];
		const GetBeatStarts& starts= item.second;
		LOG->Trace("%zu: %f", lit, item.first);
		RString str= ssprintf("  %s, %s, %s, %s,\n"
			"  last_row: %d, last_time: %.3f,\n"
			"  warp_destination: %.3f, is_warping: %d",
			SegInfoStr(bpms, starts.bpm, "bpm").c_str(),
			SegInfoStr(warps, starts.warp, "warp").c_str(),
			SegInfoStr(stops, starts.stop, "stop").c_str(),
			SegInfoStr(delays, starts.delay, "delay").c_str(),
			starts.last_row, starts.last_time, starts.warp_destination, starts.is_warping);
		LOG->Trace("%s", str.c_str());
	}
}

void TimingData::DumpLookupTables()
{
	LOG->Trace("Dumping timing data lookup tables for %s:", m_sFile.c_str());
	DumpOneTable(m_beat_start_lookup, "m_beat_start_lookup");
	DumpOneTable(m_time_start_lookup, "m_time_start_lookup");
	LOG->Trace("Finished dumping lookup tables for %s:", m_sFile.c_str());
}

TimingData::beat_start_lookup_t::const_iterator FindEntryInLookup(
	const TimingData::beat_start_lookup_t& lookup, float entry)
{
	if(lookup.empty())
	{
		return lookup.end();
	}
	size_t lower= 0;
	size_t upper= lookup.size()-1;
	if(lookup[lower].first > entry)
	{
		return lookup.end();
	}
	if(lookup[upper].first < entry)
	{
		// See explanation at the end of this function. -Kyz
		return lookup.begin() + upper - 1;
	}
	while(upper - lower > 1)
	{
		size_t next= (upper + lower) / 2;
		if(lookup[next].first > entry)
		{
			upper= next;
		}
		else if(lookup[next].first < entry)
		{
			lower= next;
		}
		else
		{
			lower= next;
			break;
		}
	}
	// If the time or beat being looked up is close enough to the starting
	// point that is returned, such as putting the time inside a stop or delay,
	// then it can make arrows unhittable.  So always return the entry before
	// the closest one to prevent that. -Kyz
	if(lower == 0)
	{
		return lookup.end();
	}
	return lookup.begin() + lower - 1;
}

bool TimingData::empty() const
{
	FOREACH_TimingSegmentType( tst )
		if( !GetTimingSegments(tst).empty() )
			return false;

	return true;
}

void TimingData::CopyRange(int start_row, int end_row,
	TimingSegmentType copy_type, int dest_row, TimingData& dest) const
{
	int row_offset= dest_row - start_row;
	FOREACH_TimingSegmentType(seg_type)
	{
		if(seg_type == copy_type || copy_type == TimingSegmentType_Invalid)
		{
			const vector<TimingSegment*>& segs= GetTimingSegments(seg_type);
			for(size_t i= 0; i < segs.size(); ++i)
			{
				if(segs[i]->GetRow() >= start_row && segs[i]->GetRow() <= end_row)
				{
					TimingSegment* copy= segs[i]->Copy();
					copy->SetRow(segs[i]->GetRow() + row_offset);
					dest.AddSegment(copy);
					// TimingSegment::Copy creates a new segment with new, and
					// AddSegment copies it again, so delete the temp. -Kyz
					delete copy;
				}
			}
		}
	}
}

void TimingData::ShiftRange(int start_row, int end_row,
	TimingSegmentType shift_type, int shift_amount)
{
	FOREACH_TimingSegmentType(seg_type)
	{
		if(seg_type == shift_type || shift_type == TimingSegmentType_Invalid)
		{
			vector<TimingSegment*>& segs= GetTimingSegments(seg_type);
			int first_row= min(start_row, start_row + shift_amount);
			int last_row= max(end_row, end_row + shift_amount);
			int first_affected= GetSegmentIndexAtRow(seg_type, first_row);
			int last_affected= GetSegmentIndexAtRow(seg_type, last_row);
			if(first_affected == INVALID_INDEX)
			{
				continue;
			}
			// Prance through the affected area twice.  The first time, changing
			// the rows of the segments, the second time removing segments that
			// have been run over by a segment being moved.  Attempts to combine
			// both operations into a single loop were error prone. -Kyz
			for(size_t i= first_affected; i <= last_affected && i < segs.size(); ++i)
			{
				int seg_row= segs[i]->GetRow();
				if(seg_row > 0 && seg_row >= start_row && seg_row <= end_row)
				{
					int dest_row= max(seg_row + shift_amount, 0);
					segs[i]->SetRow(dest_row);
				}
			}
#define ERASE_SEG(s) if(segs.size() > 1) { EraseSegment(segs, s, segs[s]); --i; --last_affected; erased= true; }
			for(size_t i= first_affected; i <= last_affected && i < segs.size(); ++i)
			{
				bool erased= false;
				int seg_row= segs[i]->GetRow();
				if(i < segs.size() - 1)
				{
					int next_row= segs[i+1]->GetRow();
					// This is a loop so that it will go back through and remove all
					// segments that were run over. -Kyz
					while(seg_row >= next_row && seg_row < start_row)
					{
						ERASE_SEG(i);
						if(i < segs.size())
						{
							seg_row= segs[i]->GetRow();
						}
						else
						{
							seg_row= -1;
						}
					}
				}
				if(!erased && i > 0)
				{
					int prev_row= segs[i-1]->GetRow();
					if(prev_row >= seg_row)
					{
						ERASE_SEG(i);
					}
				}
			}
#undef ERASE_SEG
		}
	}
}

void TimingData::ClearRange(int start_row, int end_row, TimingSegmentType clear_type)
{
	FOREACH_TimingSegmentType(seg_type)
	{
		if(seg_type == clear_type || clear_type == TimingSegmentType_Invalid)
		{
			vector<TimingSegment*>& segs= GetTimingSegments(seg_type);
			int first_affected= GetSegmentIndexAtRow(seg_type, start_row);
			int last_affected= GetSegmentIndexAtRow(seg_type, end_row);
			if(first_affected == INVALID_INDEX)
			{
				continue;
			}
			for(int index= last_affected; index >= first_affected; --index)
			{
				int seg_row= segs[index]->GetRow();
				if(segs.size() > 1 && seg_row > 0 && seg_row >= start_row &&
					seg_row <= end_row)
				{
					EraseSegment(segs, index, segs[index]);
				}
			}
		}
	}
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
 * Note that types whose SegmentEffectAreas are "Indefinite" are nullptr here,
 * because they should never need to be used; we always have at least one such
 * segment in the TimingData, and if not, we'll crash anyway. -- vyhd */
static const TimingSegment* DummySegments[NUM_TimingSegmentType] =
{
	nullptr, // BPMSegment
	new StopSegment,
	new DelaySegment,
	nullptr, // TimeSignatureSegment
	new WarpSegment,
	nullptr, // LabelSegment
	nullptr, // TickcountSegment
	nullptr, // ComboSegment
	nullptr, // SpeedSegment
	nullptr, // ScrollSegment
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

TimingSegment* TimingData::GetSegmentAtRow( int iNoteRow, TimingSegmentType tst )
{
	return const_cast<TimingSegment*>( static_cast<const TimingData*>(this)->GetSegmentAtRow(iNoteRow, tst) );
}

static void EraseSegment( vector<TimingSegment*> &vSegs, int index, TimingSegment *cur )
{
#ifdef WITH_LOGGING_TIMING_DATA
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
#ifdef WITH_LOGGING_TIMING_DATA
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
			{
				prev = vSegs[index - 1];
			}

			// If there is another segment after this one, it might become
			// redundant when this one is inserted.
			// If the next segment is redundant, we want to move its starting row
			// to the row the new segment is being added at instead of erasing it
			// and adding the new segment.
			// If the new segment is also redundant, erase the next segment because
			// that effectively moves it back to the prev segment. -Kyz
			if(static_cast<size_t>(index) < vSegs.size() - 1)
			{
				TimingSegment* next= vSegs[index + 1];
				if((*seg) == (*next))
				{
					// The segment after this new one is redundant.
					if((*seg) == (*prev))
					{
						// This new segment is redundant.  Erase the next segment and
						// ignore this new one.
						EraseSegment(vSegs, index + 1, next);
						if( prev != cur )
						{
							EraseSegment( vSegs, index, cur );
						}
						return;
					}
					else
					{
						// Move the next segment's start back to this row.
						next->SetRow(seg->GetRow());
						if( prev != cur )
						{
							EraseSegment( vSegs, index, cur );
						}
						return;
					}
				}
				else
				{
					// if true, this is redundant segment change
					if( (*prev) == (*seg) )
					{
						if( prev != cur )
						{
							EraseSegment( vSegs, index, cur );
						}
						return;
					}
				}
			}
			else
			{
				// if true, this is redundant segment change
				if( (*prev) == (*seg) )
				{
					if( prev != cur )
					{
						EraseSegment( vSegs, index, cur );
					}
					return;
				}
			}
			break;
		}
		default:
			break;
	}

	// the segment at or before this row is equal to the new one; ignore it
	if( bOnSameRow && (*cur) == (*seg) )
	{
#ifdef WITH_LOGGING_TIMING_DATA
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

void TimingData::GetBeatAndBPSFromElapsedTime(GetBeatArgs& args) const
{
	args.elapsed_time += GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate * PREFSMAN->m_fGlobalOffsetSeconds;
	GetBeatAndBPSFromElapsedTimeNoOffset(args);
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

void FindEvent(int& event_row, int& event_type,
	TimingData::GetBeatStarts& start, float beat, bool find_marker,
	const vector<TimingSegment*>& bpms, const vector<TimingSegment*>& warps,
	const vector<TimingSegment*>& stops, const vector<TimingSegment*>& delays)
{
	if(start.is_warping && BeatToNoteRow(start.warp_destination) < event_row)
	{
		event_row= BeatToNoteRow(start.warp_destination);
		event_type= FOUND_WARP_DESTINATION;
	}
	if(start.bpm < bpms.size() && bpms[start.bpm]->GetRow() < event_row)
	{
		event_row= bpms[start.bpm]->GetRow();
		event_type= FOUND_BPM_CHANGE;
	}
	if(start.delay < delays.size() && delays[start.delay]->GetRow() < event_row)
	{
		event_row= delays[start.delay]->GetRow();
		event_type= FOUND_DELAY;
	}
	if(find_marker && BeatToNoteRow(beat) < event_row)
	{
		event_row= BeatToNoteRow(beat);
		event_type= FOUND_MARKER;
	}
	if(start.stop < stops.size() && stops[start.stop]->GetRow() < event_row)
	{
		int tmp_row= event_row;
		event_row= stops[start.stop]->GetRow();
		event_type= (tmp_row == event_row) ? FOUND_STOP_DELAY : FOUND_STOP;
	}
	if(start.warp < warps.size() && warps[start.warp]->GetRow() < event_row)
	{
		event_row= warps[start.warp]->GetRow();
		event_type= FOUND_WARP;
	}
}

void TimingData::GetBeatInternal(GetBeatStarts& start, GetBeatArgs& args,
	unsigned int max_segment) const
{
	const vector<TimingSegment*>& bpms= m_avpTimingSegments[SEGMENT_BPM];
	const vector<TimingSegment*>& warps= m_avpTimingSegments[SEGMENT_WARP];
	const vector<TimingSegment*>& stops= m_avpTimingSegments[SEGMENT_STOP];
	const vector<TimingSegment*>& delays= m_avpTimingSegments[SEGMENT_DELAY];
	unsigned int curr_segment= start.bpm+start.warp+start.stop+start.delay;

	float bps= GetBPMAtRow(start.last_row) / 60.0f;
#define INC_INDEX(index) ++curr_segment; ++index;

	while(curr_segment < max_segment)
	{
		int event_row= INT_MAX;
		int event_type= NOT_FOUND;
		FindEvent(event_row, event_type, start, 0, false, bpms, warps, stops,
			delays);
		if(event_type == NOT_FOUND)
		{
			break;
		}
		float time_to_next_event= start.is_warping ? 0 :
			NoteRowToBeat(event_row - start.last_row) / bps;
		float next_event_time= start.last_time + time_to_next_event;
		if(args.elapsed_time < next_event_time)
		{
			break;
		}
		start.last_time= next_event_time;
		switch(event_type)
		{
			case FOUND_WARP_DESTINATION:
				start.is_warping= false;
				break;
			case FOUND_BPM_CHANGE:
				bps= ToBPM(bpms[start.bpm])->GetBPS();
				INC_INDEX(start.bpm);
				break;
			case FOUND_DELAY:
			case FOUND_STOP_DELAY:
				{
					const DelaySegment* ss= ToDelay(delays[start.delay]);
					time_to_next_event= ss->GetPause();
					next_event_time= start.last_time + time_to_next_event;
					if(args.elapsed_time < next_event_time)
					{
						args.freeze_out= false;
						args.delay_out= true;
						args.beat= ss->GetBeat();
						args.bps_out= bps;
						return;
					}
					start.last_time= next_event_time;
					INC_INDEX(start.delay);
					if(event_type == FOUND_DELAY)
					{
						break;
					}
				}
			case FOUND_STOP:
				{
					const StopSegment* ss= ToStop(stops[start.stop]);
					time_to_next_event= ss->GetPause();
					next_event_time= start.last_time + time_to_next_event;
					if(args.elapsed_time < next_event_time)
					{
						args.freeze_out= true;
						args.delay_out= false;
						args.beat= ss->GetBeat();
						args.bps_out= bps;
						return;
					}
					start.last_time= next_event_time;
					INC_INDEX(start.stop);
					break;
				}
			case FOUND_WARP:
				{
					start.is_warping= true;
					const WarpSegment* ws= ToWarp(warps[start.warp]);
					float warp_sum= ws->GetLength() + ws->GetBeat();
					if(warp_sum > start.warp_destination)
					{
						start.warp_destination= warp_sum;
					}
					args.warp_begin_out= event_row;
					args.warp_dest_out= start.warp_destination;
					INC_INDEX(start.warp);
					break;
				}
		}
		start.last_row= event_row;
	}
#undef INC_INDEX
	if(args.elapsed_time == FLT_MAX)
	{
		args.elapsed_time= start.last_time;
	}
	args.beat= NoteRowToBeat(start.last_row) +
		(args.elapsed_time - start.last_time) * bps;
	args.bps_out= bps;
}

void TimingData::GetBeatAndBPSFromElapsedTimeNoOffset(GetBeatArgs& args) const
{
	GetBeatStarts start;
	start.last_time= -m_fBeat0OffsetInSeconds;
	beat_start_lookup_t::const_iterator looked_up_start=
		FindEntryInLookup(m_beat_start_lookup, args.elapsed_time);
	if(looked_up_start != m_beat_start_lookup.end())
	{
		start= looked_up_start->second;
	}
	GetBeatInternal(start, args, INT_MAX);
}

float TimingData::GetElapsedTimeInternal(GetBeatStarts& start, float beat,
	unsigned int max_segment) const
{
	const vector<TimingSegment*>& bpms= m_avpTimingSegments[SEGMENT_BPM];
	const vector<TimingSegment*>& warps= m_avpTimingSegments[SEGMENT_WARP];
	const vector<TimingSegment*>& stops= m_avpTimingSegments[SEGMENT_STOP];
	const vector<TimingSegment*>& delays= m_avpTimingSegments[SEGMENT_DELAY];
	unsigned int curr_segment= start.bpm+start.warp+start.stop+start.delay;

	float bps= GetBPMAtRow(start.last_row) / 60.0f;
#define INC_INDEX(index) ++curr_segment; ++index;
	bool find_marker= beat < FLT_MAX;

	while(curr_segment < max_segment)
	{
		int event_row= INT_MAX;
		int event_type= NOT_FOUND;
		FindEvent(event_row, event_type, start, beat, find_marker, bpms, warps, stops,
			delays);
		float time_to_next_event= start.is_warping ? 0 :
			NoteRowToBeat(event_row - start.last_row) / bps;
		float next_event_time= start.last_time + time_to_next_event;
		start.last_time= next_event_time;
		switch(event_type)
		{
			case FOUND_WARP_DESTINATION:
				start.is_warping= false;
				break;
			case FOUND_BPM_CHANGE:
				bps= ToBPM(bpms[start.bpm])->GetBPS();
				INC_INDEX(start.bpm);
				break;
			case FOUND_STOP:
			case FOUND_STOP_DELAY:
				time_to_next_event= ToStop(stops[start.stop])->GetPause();
				next_event_time= start.last_time + time_to_next_event;
				start.last_time= next_event_time;
				INC_INDEX(start.stop);
				break;
			case FOUND_DELAY:
				time_to_next_event= ToDelay(delays[start.delay])->GetPause();
				next_event_time= start.last_time + time_to_next_event;
				start.last_time= next_event_time;
				INC_INDEX(start.delay);
				break;
			case FOUND_MARKER:
				return start.last_time;
			case FOUND_WARP:
				{
					start.is_warping= true;
					WarpSegment* ws= ToWarp(warps[start.warp]);
					float warp_sum= ws->GetLength() + ws->GetBeat();
					if(warp_sum > start.warp_destination)
					{
						start.warp_destination= warp_sum;
					}
					INC_INDEX(start.warp);
					break;
				}
		}
		start.last_row= event_row;
	}
#undef INC_INDEX
	return start.last_time;
}

float TimingData::GetElapsedTimeFromBeat( float fBeat ) const
{
	return TimingData::GetElapsedTimeFromBeatNoOffset( fBeat )
		- GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate * PREFSMAN->m_fGlobalOffsetSeconds;
}

float TimingData::GetElapsedTimeFromBeatNoOffset( float fBeat ) const
{
	GetBeatStarts start;
	start.last_time= -m_fBeat0OffsetInSeconds;
	beat_start_lookup_t::const_iterator looked_up_start=
		FindEntryInLookup(m_time_start_lookup, fBeat);
	if(looked_up_start != m_time_start_lookup.end())
	{
		start= looked_up_start->second;
	}
	GetElapsedTimeInternal(start, fBeat, INT_MAX);
	return start.last_time;
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

// Delete timing changes in [iStartRow, iStartRow + iRowsToDelete) and shift up.
void TimingData::DeleteRows( int iStartRow, int iRowsToDelete )
{
	FOREACH_TimingSegmentType( tst )
	{
		// Don't delete the indefinite segments that are still in effect
		// at the end row; rather, shift them so they start there.
		TimingSegment *tsEnd = GetSegmentAtRow(iStartRow + iRowsToDelete, tst);
		if (tsEnd != nullptr && tsEnd->GetEffectType() == SegmentEffectType_Indefinite &&
				iStartRow <= tsEnd->GetRow() &&
				tsEnd->GetRow() < iStartRow + iRowsToDelete)
		{
			// The iRowsToDelete will eventually be subtracted out
			LOG->Trace("Segment at row %d shifted to %d", tsEnd->GetRow(), iStartRow + iRowsToDelete);
			tsEnd->SetRow(iStartRow + iRowsToDelete);
		}

		// Now delete and shift up
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
		LOG->Trace("No speed segments found: using default value.");
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
	auto &segs = m_avpTimingSegments;
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

#define TIMING_DATA_RETURNS_NUMBERS THEME->GetMetricB("TimingData", "GetReturnsNumbers")

// This breaks encapsulation just as much as TimingData::ToVectorString does.
// But, it exists solely for the purpose of providing lua access, so it's as okay as all the other lua stuff that reaches past the encapsulation.
void TimingSegmentSetToLuaTable(TimingData* td, TimingSegmentType tst, lua_State *L);
void TimingSegmentSetToLuaTable(TimingData* td, TimingSegmentType tst, lua_State *L)
{
	const vector<TimingSegment*> segs= td->GetTimingSegments(tst);
	lua_createtable(L, segs.size(), 0);
	if(tst == SEGMENT_LABEL)
	{
		for(size_t i= 0; i < segs.size(); ++i)
		{
			lua_createtable(L, 2, 0);
			lua_pushnumber(L, segs[i]->GetBeat());
			lua_rawseti(L, -2, 1);
			lua_pushstring(L, (ToLabel(segs[i]))->GetLabel().c_str());
			lua_rawseti(L, -2, 2);
			lua_rawseti(L, -2, i+1);
		}
	}
	else
	{
		for(size_t i= 0; i < segs.size(); ++i)
		{
			vector<float> values= segs[i]->GetValues();
			lua_createtable(L, values.size()+1, 0);
			lua_pushnumber(L, segs[i]->GetBeat());
			lua_rawseti(L, -2, 1);
			for(size_t v= 0; v < values.size(); ++v)
			{
				lua_pushnumber(L, values[v]);
				lua_rawseti(L, -2, v+2);
			}
			lua_rawseti(L, -2, i+1);
		}
	}
}

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
#define GET_FUNCTION(get_name, segment_name) \
	static int get_name(T* p, lua_State* L) \
	{ \
		if(lua_toboolean(L, 1)) \
		{ \
			TimingSegmentSetToLuaTable(p, segment_name, L); \
		} \
		else \
		{ \
			LuaHelpers::CreateTableFromArray(p->ToVectorString(segment_name), L); \
		} \
		return 1; \
	}

	GET_FUNCTION(GetWarps, SEGMENT_WARP);
	GET_FUNCTION(GetFakes, SEGMENT_FAKE);
	GET_FUNCTION(GetScrolls, SEGMENT_SCROLL);
	GET_FUNCTION(GetSpeeds, SEGMENT_SPEED);
	GET_FUNCTION(GetTimeSignatures, SEGMENT_TIME_SIG);
	GET_FUNCTION(GetCombos, SEGMENT_COMBO);
	GET_FUNCTION(GetTickcounts, SEGMENT_TICKCOUNT);
	GET_FUNCTION(GetStops, SEGMENT_STOP);
	GET_FUNCTION(GetDelays, SEGMENT_DELAY);
	GET_FUNCTION(GetLabels, SEGMENT_LABEL);
	GET_FUNCTION(GetBPMsAndTimes, SEGMENT_BPM);
#undef GET_FUNCTION
	static int GetBPMs( T* p, lua_State *L )
	{
		vector<float> vBPMs;
		const vector<TimingSegment *> &bpms = p->GetTimingSegments(SEGMENT_BPM);

		for (unsigned i = 0; i < bpms.size(); i++)
			vBPMs.push_back( ToBPM(bpms[i])->GetBPM() );

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
