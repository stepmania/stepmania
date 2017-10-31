#include "global.h"
#include "TimingData.h"
#include "RageMath.hpp"
#include "PrefsManager.h"
#include "GameState.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "NoteTypes.h"

using std::vector;

static void EraseSegment(vector<TimingSegment*> &vSegs, int index, TimingSegment *cur);
static const int INVALID_INDEX = -1;

TimingSegment* GetSegmentAtRow( int iNoteRow, TimingSegmentType tst );

// According to The C++ Programming Language 3rd Ed., decreasing the size
// of a vector doesn't actually free the memory it has allocated.  So this
// small trick is required to actually free the memory. -Kyz
template<typename T>
	void full_clear_container(T& container)
{
	container.clear();
	T tmp;
	container.swap(tmp);
}

struct lookup_updater_for_segment_add
{
	lookup_updater_for_segment_add(TimingData* para)
		:parent(para)
	{}
	~lookup_updater_for_segment_add()
	{
		// Updating the lookup in place seems complicated and bug prone.  So just
		// destroy it and rebuild it entirely.  Rebuilding doesn't take
		// substantial time, and AddSegment when the lookup exists should only
		// happen in edit mode, when the user changes a timing element.
		// TODO:  Create a multi-AddSegment that edit mode can use when pasting
		// chunks of timing data.
		// -Kyz
		if(parent->get_lookup_requester_count() > 0)
		{
			parent->PrepareLookup();
		}
	}
	TimingData* parent;
};

TimingData::TimingData(float fOffset)
	:m_lookup_requester_count(0), m_fBeat0OffsetInSeconds(fOffset)
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

		for (auto *seg: vpSegs)
		{
			AddSegment( seg );
		}
	}
	if(get_lookup_requester_count() > 0)
	{
		PrepareLookup();
	}
}

void TimingData::Clear()
{
	/* Delete all pointers owned by this TimingData. */
	FOREACH_TimingSegmentType( tst )
	{
		vector<TimingSegment*> &vSegs = m_avpTimingSegments[tst];
		for (auto *seg: vSegs)
		{
			Rage::safe_delete( seg );
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
	vector<TimingSegment *> *segs = m_avpTimingSegments;
	for(size_t s= 0; s < needed_segments.size(); ++s)
	{
		if(segs[needed_segments[s]].empty())
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

enum
{
	SEARCH_NONE,
	SEARCH_BEAT,
	SEARCH_SECOND
};

struct FindEventStatus
{
	unsigned int bpm;
	unsigned int warp;
	unsigned int stop;
	unsigned int delay;
	int last_row;
	float last_time;
	float warp_destination;
	bool is_warping;
	FindEventStatus() :bpm(0), warp(0), stop(0), delay(0), last_row(0),
		last_time(0), warp_destination(0), is_warping(false) {}
};

static void FindEvent(int& event_row, int& event_type,
	FindEventStatus& status, float beat, bool find_marker,
	const vector<TimingSegment*>& bpms, const vector<TimingSegment*>& warps,
	const vector<TimingSegment*>& stops, const vector<TimingSegment*>& delays)
{
	if(status.is_warping && BeatToNoteRow(status.warp_destination) < event_row)
	{
		event_row= BeatToNoteRow(status.warp_destination);
		event_type= FOUND_WARP_DESTINATION;
	}
	if(status.bpm < bpms.size() && bpms[status.bpm]->GetRow() < event_row)
	{
		event_row= bpms[status.bpm]->GetRow();
		event_type= FOUND_BPM_CHANGE;
	}
	if(status.delay < delays.size() && delays[status.delay]->GetRow() < event_row)
	{
		event_row= delays[status.delay]->GetRow();
		event_type= FOUND_DELAY;
	}
	if(find_marker && BeatToNoteRow(beat) < event_row)
	{
		event_row= BeatToNoteRow(beat);
		event_type= FOUND_MARKER;
	}
	if(status.stop < stops.size() && stops[status.stop]->GetRow() < event_row)
	{
		int tmp_row= event_row;
		event_row= stops[status.stop]->GetRow();
		event_type= (tmp_row == event_row) ? FOUND_STOP_DELAY : FOUND_STOP;
	}
	if(status.warp < warps.size() && warps[status.warp]->GetRow() < event_row)
	{
		event_row= warps[status.warp]->GetRow();
		event_type= FOUND_WARP;
	}
}

void TimingData::PrepareLineLookup(int search_mode, float search_time,
	LineSegment* search_ret) const
{
	auto bpms= GetTimingSegments(SEGMENT_BPM);
	auto stops= GetTimingSegments(SEGMENT_STOP);
	auto delays= GetTimingSegments(SEGMENT_DELAY);
	auto warps= GetTimingSegments(SEGMENT_WARP);
	if(search_mode == SEARCH_NONE)
	{
		m_line_segments.reserve(bpms.size() + stops.size() + delays.size() + warps.size());
	}
	FindEventStatus status;
#define SEARCH_NONE_CASE \
	case SEARCH_NONE: m_line_segments.push_back(next_line); break;
#define RETURN_IF_BEAT_COMPARE \
	if(next_line.end_beat > search_time) \
	{ \
		(*search_ret)= next_line; \
		return; \
	}
#define RETURN_IF_SECOND_COMPARE \
	if(next_line.end_second > search_time) \
	{ \
		(*search_ret)= next_line; \
		return; \
	}
	// Place an initial bpm segment in negative time before the song begins.
	// Without this, if there is a stop at beat 0, arrows will not move until
	// after beat 0 passes. -Kyz
	float first_bps= ToBPM(bpms[0])->GetBPS();
	LineSegment next_line= {
		-first_bps, -m_fBeat0OffsetInSeconds-1.f, 0.f, -m_fBeat0OffsetInSeconds,
		-m_fBeat0OffsetInSeconds-1.f, -m_fBeat0OffsetInSeconds,
		first_bps, bpms[0]};
	switch(search_mode)
	{
		SEARCH_NONE_CASE;
		case SEARCH_BEAT:
			RETURN_IF_BEAT_COMPARE;
			break;
		case SEARCH_SECOND:
			RETURN_IF_SECOND_COMPARE;
			break;
		default:
			break;
	}
	next_line.set_for_next();
	float spb= 1.f / next_line.bps;
	bool finished= false;
	// Placement order:
	//   warp
	//   delay
	//   stop
	//   bpm
	// Stop and delay segments can be placed as complete lines.
	// A warp needs to be broken into parts at every stop or delay that occurs
	// inside it.
	// When a warp occurs inside a warp, whichever has the greater destination
	// is used.
	// A bpm segment is placed when between two other segments.
	// -Kyz
	float const max_time= 16777216.f;
	TimingSegment* curr_bpm_segment= bpms[0];
	TimingSegment* curr_warp_segment= nullptr;
	while(!finished)
	{
		int event_row= std::numeric_limits<int>::max();;
		int event_type= NOT_FOUND;
		FindEvent(event_row, event_type, status, max_time, false,
			bpms, warps, stops, delays);
		if(event_type == NOT_FOUND)
		{
			next_line.end_beat= next_line.start_beat + 1.f;
			float seconds_change= next_line.start_second + spb;
			next_line.end_second= seconds_change;
			next_line.end_expand_second= next_line.start_expand_second + seconds_change;
			next_line.bps= ToBPM(curr_bpm_segment)->GetBPS();
			next_line.time_segment= curr_bpm_segment;
			switch(search_mode)
			{
				SEARCH_NONE_CASE;
				case SEARCH_BEAT:
				case SEARCH_SECOND:
					(*search_ret)= next_line;
					return;
					break;
				default:
					break;
			}
			finished= true;
			break;
		}
		if(status.is_warping)
		{
			// Don't place a line when encountering a warp inside a warp because
			// it should go straight to the end.
			if(event_type != FOUND_WARP)
			{
				next_line.end_beat= NoteRowToBeat(event_row);
				next_line.end_second= next_line.start_second;
				next_line.end_expand_second= next_line.start_expand_second;
				next_line.time_segment= curr_warp_segment;
				switch(search_mode)
				{
					SEARCH_NONE_CASE;
					case SEARCH_BEAT:
						RETURN_IF_BEAT_COMPARE;
						break;
					case SEARCH_SECOND:
						// A search for a second can't end inside a warp.
						break;
					default:
						break;
				}
				next_line.set_for_next();
			}
		}
		else
		{
			if(event_row > 0)
			{
				next_line.end_beat= NoteRowToBeat(event_row);
				float beats= next_line.end_beat - next_line.start_beat;
				float seconds= beats * spb;
				next_line.end_second= next_line.start_second + seconds;
				next_line.end_expand_second= next_line.start_expand_second + seconds;
				next_line.time_segment= curr_bpm_segment;
				switch(search_mode)
				{
					SEARCH_NONE_CASE;
					case SEARCH_BEAT:
						RETURN_IF_BEAT_COMPARE;
						break;
					case SEARCH_SECOND:
						RETURN_IF_SECOND_COMPARE;
						break;
					default:
						break;
				}
				next_line.set_for_next();
			}
		}
		switch(event_type)
		{
			case FOUND_WARP_DESTINATION:
				// Already handled in the is_warping condition above.
				status.is_warping= false;
				curr_warp_segment= nullptr;
				break;
			case FOUND_BPM_CHANGE:
				curr_bpm_segment= bpms[status.bpm];
				next_line.bps= ToBPM(curr_bpm_segment)->GetBPS();
				spb= 1.f / next_line.bps;
				++status.bpm;
				break;
			case FOUND_DELAY:
			case FOUND_STOP_DELAY:
				next_line.end_beat= next_line.start_beat;
				next_line.end_second= next_line.start_second +
					ToDelay(delays[status.delay])->GetPause();
				next_line.end_expand_second= next_line.start_expand_second;
				next_line.time_segment= delays[status.delay];
				switch(search_mode)
				{
					SEARCH_NONE_CASE;
					case SEARCH_BEAT:
						// Delay occurs before the beat, so the beat being searched for
						// can't be inside the delay.
						break;
					case SEARCH_SECOND:
						RETURN_IF_SECOND_COMPARE;
						break;
					default:
						break;
				}
				next_line.set_for_next();
				++status.delay;
				if(event_type == FOUND_DELAY)
				{
					break;
				}
			case FOUND_STOP:
				next_line.end_beat= next_line.start_beat;
				next_line.end_second= next_line.start_second +
					ToStop(stops[status.stop])->GetPause();
				next_line.end_expand_second= next_line.start_expand_second;
				next_line.time_segment= stops[status.stop];
				switch(search_mode)
				{
					SEARCH_NONE_CASE;
					case SEARCH_BEAT:
						// Stop occurs after the beat, so use this segment if the beat is
						// equal.
						if(next_line.end_beat == search_time)
						{
							(*search_ret)= next_line;
							return;
						}
						break;
					case SEARCH_SECOND:
						RETURN_IF_SECOND_COMPARE;
						break;
					default:
						break;
				}

				next_line.set_for_next();
				++status.stop;
				break;
			case FOUND_WARP:
				{
					status.is_warping= true;
					curr_warp_segment= warps[status.warp];
					WarpSegment* ws= ToWarp(warps[status.warp]);
					float warp_dest= ws->GetLength() + ws->GetBeat();
					if(warp_dest > status.warp_destination)
					{
						status.warp_destination= warp_dest;
					}
					++status.warp;
				}
				break;
			default:
				break;
		}
		status.last_row= event_row;
	}
#undef SEARCH_NONE_CASE
#undef RETURN_IF_BEAT_COMPARE
#undef RETURN_IF_SECOND_COMPARE
	ASSERT_M(search_mode == SEARCH_NONE, "PrepareLineLookup made it to the end while not in search_mode none.");
	// m_segments_by_beat and m_segments_by_second cannot be built in the
	// traversal above that builds m_line_segments because the vector
	// reallocates as it grows. -Kyz
	vector<LineSegment*>* curr_segments_by_beat= &(m_segments_by_beat[0.f]);
	vector<LineSegment*>* curr_segments_by_second= &(m_segments_by_second[-m_fBeat0OffsetInSeconds]);
	float curr_beat= 0.f;
	float curr_second= 0.f;
	for(size_t seg= 0; seg < m_line_segments.size(); ++seg)
	{
#define ADD_SEG(bors) \
		if(m_line_segments[seg].start_##bors > curr_##bors) \
		{ \
			curr_segments_by_##bors= &(m_segments_by_##bors[m_line_segments[seg].start_##bors]); \
			curr_##bors= m_line_segments[seg].start_##bors; \
		} \
		curr_segments_by_##bors->push_back(&m_line_segments[seg]);

		ADD_SEG(beat);
		ADD_SEG(second);
#undef ADD_SEG
	}
}

void TimingData::ReleaseLineLookup() const
{
	if(!m_line_segments.empty())
	{
		m_line_segments.clear();
		m_line_segments.shrink_to_fit();
		full_clear_container(m_segments_by_beat);
		full_clear_container(m_segments_by_second);
	}
}

TimingData::LineSegment const* FindLineSegment(
	std::map<float, std::vector<TimingData::LineSegment*> > const& sorted_segments,
	float time)
{
	ASSERT_M(!sorted_segments.empty(), "FindLineSegment called on empty sorted_segments.");
	auto seg_container= sorted_segments.lower_bound(time);
	// lower_bound returns the first element not less than the given key.
	// So if there is a segment at 1.0 and another at 2.0, and time is 1.5,
	// lower_bound returns the segment at 2.0.  Thus, whatever it returns needs
	// to be decremented. -Kyz
	if(seg_container != sorted_segments.begin() &&
		(seg_container == sorted_segments.end() ||
			seg_container->first > time))
	{
		--seg_container;
	}
	// Logically, if time is greater than seg_container->first, then we'll
	// be interpolating off of the last segment in seg_container.
	// Otherwise, they all have the same alt-time, so it doesn't matter which
	// we return.
	// -Kyz
	if(time > seg_container->first)
	{
		return seg_container->second.back();
	}
	return seg_container->second.front();
}

// Return the end time if the input dist is 0 sometimes:
//   A: Calculating beat from second
//      1. Segment is a warp.  Warp is instant, so return the end beat.
//      2. Segment is a delay or stop.  Start and end beat are the same.
//   B: Calculating second from beat
//      1. Segment is a warp.  Start and end second are the same.
//      2. Segment is a delay.  Delay happens before the beat, so return the
//         end second.
//      3. Segment is a stop.  Stop happens after the beat, so return the
//         start second.
//   A bpm segment does not have zero distance in either direction.

float TimingData::GetLineBeatFromSecond(float from) const
{
	LineSegment const* segment= FindLineSegment(m_segments_by_second, from);
	if(segment->start_second == segment->end_second)
	{
		return segment->end_beat;
	}
	return Rage::scale(from, segment->start_second, segment->end_second,
		segment->start_beat, segment->end_beat);
}

float TimingData::GetLineSecondFromBeat(float from) const
{
	LineSegment const* segment= FindLineSegment(m_segments_by_beat, from);
	if(segment->start_beat == segment->end_beat)
	{
		if(segment->time_segment->GetType() == SEGMENT_DELAY)
		{
			return segment->end_second;
		}
		return segment->start_second;
	}
	return Rage::scale(from, segment->start_beat, segment->end_beat,
		segment->start_second, segment->end_second);
}

float TimingData::GetExpandSeconds(float from) const
{
	LineSegment segment;
	if(!m_line_segments.empty())
	{
		segment= *FindLineSegment(m_segments_by_second, from);
	}
	else
	{
		const_cast<TimingData*>(this)->PrepareLineLookup(SEARCH_SECOND, from, &segment);
	}
	if(segment.start_second == segment.end_second ||
		segment.start_beat == segment.end_beat)
	{
		return segment.end_expand_second;
	}
	return Rage::scale(from, segment.start_second, segment.end_second,
		segment.start_expand_second, segment.end_expand_second);
}

void TimingData::RequestLookup() const
{
	++m_lookup_requester_count;
	if(m_lookup_requester_count > 1)
	{
		return;
	}
	PrepareLookup();
}

void TimingData::PrepareLookup() const
{
	// If by some mistake the old lookup table is still hanging around, adding
	// more entries would probably cause problems.  Release the lookups. -Kyz
	ReleaseLookupInternal();
	if(empty())
	{
		return;
	}
	PrepareLineLookup(SEARCH_NONE, 0.f, nullptr);

	const vector<TimingSegment*>* segs= m_avpTimingSegments;
	const vector<TimingSegment*>& scrolls= segs[SEGMENT_SCROLL];
	float displayed_beat= 0.0f;
	float last_real_beat= 0.0f;
	float last_ratio= 1.0f;
	for(auto&& scr : scrolls)
	{
		ScrollSegment* scroll= ToScroll(scr);
		float scroll_beat= scroll->GetBeat();
		float scroll_ratio= scroll->GetRatio();
		displayed_beat+= (scroll_beat - last_real_beat) * last_ratio;
		m_displayed_beat_lookup[scroll_beat]= {scroll_beat, displayed_beat, scroll_ratio};
		last_real_beat= scroll_beat;
		last_ratio= scroll_ratio;
	}

	// DumpLookupTables();
}

void TimingData::ReleaseLookup() const
{
	--m_lookup_requester_count;
	if(m_lookup_requester_count > 0)
	{
		return;
	}
	ReleaseLookupInternal();
}

void TimingData::ReleaseDisplayedBeatLookup() const
{
	if(!m_displayed_beat_lookup.empty())
	{
		full_clear_container(m_displayed_beat_lookup);
	}
}

void TimingData::ReleaseLookupInternal() const
{
	ReleaseDisplayedBeatLookup();
	ReleaseLineLookup();
}

std::string SegInfoStr(const vector<TimingSegment*>& segs, unsigned int index, const std::string& name)
{
	if(index < segs.size())
	{
		return fmt::sprintf("%s: %d at %d", name.c_str(), index, segs[index]->GetRow());
	}
	return fmt::sprintf("%s: %d at end", name.c_str(), index);
}

void TimingData::DumpLookupTables() const
{
	LOG->Trace("Dumping timing data lookup tables for %s:", m_sFile.c_str());
	// TODO: Write debugging dump code for line segment system.
	LOG->Trace("Finished dumping lookup tables for %s:", m_sFile.c_str());
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
			// TODO: Determine if range for will allow the copy to work.
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
	lookup_updater_for_segment_add lookup_me_senpai(this);
	FOREACH_TimingSegmentType(seg_type)
	{
		if(seg_type == shift_type || shift_type == TimingSegmentType_Invalid)
		{
			vector<TimingSegment*>& segs= GetTimingSegments(seg_type);
			int first_row= std::min(start_row, start_row + shift_amount);
			int last_row= std::max(end_row, end_row + shift_amount);
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
			for(size_t i= first_affected; i <= static_cast<size_t>(last_affected) && i < segs.size(); ++i)
			{
				int seg_row= segs[i]->GetRow();
				if(seg_row > 0 && seg_row >= start_row && seg_row <= end_row)
				{
					int dest_row= std::max(seg_row + shift_amount, 0);
					segs[i]->SetRow(dest_row);
				}
			}
#define ERASE_SEG(s) if(segs.size() > 1) { EraseSegment(segs, s, segs[s]); --i; --last_affected; erased= true; }
			for(size_t i= first_affected; i <= static_cast<size_t>(last_affected) && i < segs.size(); ++i)
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
	lookup_updater_for_segment_add lookup_me_senpai(this);
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
	fMinBPMOut = std::numeric_limits<float>::max();
	fMaxBPMOut = 0;
	const std::vector<TimingSegment*> &bpms = GetTimingSegments(SEGMENT_BPM);

	for (unsigned i = 0; i < bpms.size(); i++)
	{
		const float fBPM = ToBPM(bpms[i])->GetBPM();
		fMaxBPMOut = Rage::clamp(std::max( fBPM, fMaxBPMOut ), 0.f, highest);
		fMinBPMOut = std::min( fBPM, fMinBPMOut );
	}
}

float TimingData::GetNextSegmentBeatAtRow(TimingSegmentType tst, int row) const
{
	const std::vector<TimingSegment *> segs = GetTimingSegments(tst);
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

struct ts_less : std::binary_function <TimingSegment*, TimingSegment*, bool>
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
		const int iStartIndexNextSegment = bIsLastBPMSegment ? std::numeric_limits<int>::max() : bpms[i+1]->GetRow();

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
	LOG->Trace( "Erasing segment at index %d", index );
	cur->DebugPrint();
#endif

	vSegs.erase( vSegs.begin() + index );
	Rage::safe_delete( cur );
}

void TimingData::set_offset(float off)
{
	if(off != m_fBeat0OffsetInSeconds)
	{
		m_fBeat0OffsetInSeconds= off;
		if(get_lookup_requester_count() > 0)
		{
			PrepareLookup();
		}
	}
}

float TimingData::get_offset() const
{
	return m_fBeat0OffsetInSeconds;
}

// NOTE: the pointer we're passed is a reference to a temporary,
// so we must deep-copy it (with ::Copy) for new allocations.
void TimingData::AddSegment( const TimingSegment *seg )
{
#ifdef WITH_LOGGING_TIMING_DATA
	LOG->Trace( "AddSegment( %s )", TimingSegmentTypeToString(seg->GetType()).c_str() );
	seg->DebugPrint();
#endif
	// NEET trick: class that will trigger the lookup table rebuild when it is
	// destroyed.
	lookup_updater_for_segment_add lookup_me_senpai(this);

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
		Rage::safe_delete( cur );
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

bool TimingData::DoesLabelExist( const std::string& sLabel ) const
{
	const vector<TimingSegment *> &labels = GetTimingSegments(SEGMENT_LABEL);
	for (unsigned i = 0; i < labels.size(); i++)
	{
		if (ToLabel(labels[i])->GetLabel() == sLabel)
			return true;
	}
	return false;
}

void TimingData::GetDetailedInfoForSecond(DetailedTimeInfo& args) const
{
	args.second += GAMESTATE->get_hasted_music_rate() * PREFSMAN->m_fGlobalOffsetSeconds;
	GetDetailedInfoForSecondNoOffset(args);
}

float TimingData::GetBeatFromElapsedTime(float second) const
{
	float globoff= GAMESTATE->get_hasted_music_rate() * PREFSMAN->m_fGlobalOffsetSeconds;
	if(!m_line_segments.empty())
	{
		return GetLineBeatFromSecond(second + globoff);
	}
	else
	{
		LineSegment segment;
		const_cast<TimingData*>(this)->PrepareLineLookup(SEARCH_SECOND,
			second + globoff, &segment);
		if(segment.start_second == segment.end_second)
		{
			return segment.end_beat;
		}
		return Rage::scale(second, segment.start_second, segment.end_second,
			segment.start_beat, segment.end_beat);
	}
}

float TimingData::GetBeatFromElapsedTimeNoOffset(float second) const
{
	if(!m_line_segments.empty())
	{
		return GetLineBeatFromSecond(second);
	}
	else
	{
		LineSegment segment;
		const_cast<TimingData*>(this)->PrepareLineLookup(SEARCH_SECOND,
			second, &segment);
		if(segment.start_second == segment.end_second)
		{
			return segment.end_beat;
		}
		return Rage::scale(second, segment.start_second, segment.end_second,
			segment.start_beat, segment.end_beat);
	}
}

void TimingData::GetDetailedInfoForSecondNoOffset(DetailedTimeInfo& args) const
{
	LineSegment segment;
	if(!m_line_segments.empty())
	{
		segment= *FindLineSegment(m_segments_by_second, args.second);
	}
	else
	{
		const_cast<TimingData*>(this)->PrepareLineLookup(SEARCH_SECOND,
			args.second, &segment);
	}
	args.bps_out= segment.bps;
	switch(segment.time_segment->GetType())
	{
		case SEGMENT_STOP:
			args.freeze_out= true;
			break;
		case SEGMENT_DELAY:
			args.delay_out= true;
			break;
		default:
			break;
	}
	if(segment.start_second == segment.end_second)
	{
		args.beat= segment.end_beat;
	}
	else
	{
		args.beat= Rage::scale(args.second, segment.start_second,
			segment.end_second, segment.start_beat, segment.end_beat);
	}
}

float TimingData::GetElapsedTimeFromBeat(float beat) const
{
	return TimingData::GetElapsedTimeFromBeatNoOffset(beat)
		- GAMESTATE->get_hasted_music_rate() * PREFSMAN->m_fGlobalOffsetSeconds;
}

float TimingData::GetElapsedTimeFromBeatNoOffset(float beat) const
{
	if(!m_line_segments.empty())
	{
		return GetLineSecondFromBeat(beat);
	}
	else
	{
		LineSegment segment;
		const_cast<TimingData*>(this)->PrepareLineLookup(SEARCH_BEAT, beat, &segment);
		if(segment.start_beat == segment.end_beat)
		{
			if(segment.time_segment->GetType() == SEGMENT_DELAY)
			{
				return segment.end_second;
			}
			return segment.start_second;
		}
		return Rage::scale(beat, segment.start_beat, segment.end_beat,
			segment.start_second, segment.end_second);
	}
}

static float beat_relative_to_displayed_beat_entry(float beat, TimingData::displayed_beat_entry const& entry)
{
	return entry.displayed_beat + (entry.velocity * (beat - entry.beat));
}

float TimingData::GetDisplayedBeat(float beat) const
{
	if(!m_displayed_beat_lookup.empty())
	{
		auto entry= m_displayed_beat_lookup.lower_bound(beat);
		if(entry != m_displayed_beat_lookup.begin() &&
			(entry == m_displayed_beat_lookup.end() ||
				entry->first > beat))
		{
			--entry;
		}
		return beat_relative_to_displayed_beat_entry(beat, entry->second);
	}

	float fOutBeat = 0;
	unsigned i;
	const vector<TimingSegment *> &scrolls = m_avpTimingSegments[SEGMENT_SCROLL];
	for( i=0; i<scrolls.size()-1; i++ )
	{
		if( scrolls[i+1]->GetBeat() > beat )
			break;
		fOutBeat += (scrolls[i+1]->GetBeat() - scrolls[i]->GetBeat()) * ToScroll(scrolls[i])->GetRatio();
	}
	fOutBeat += (beat - scrolls[i]->GetBeat()) * ToScroll(scrolls[i])->GetRatio();
	return fOutBeat;
}

void TimingData::ScaleRegion( float fScale, int iStartIndex, int iEndIndex, bool bAdjustBPM )
{
	lookup_updater_for_segment_add lookup_me_senpai(this);
	ASSERT( fScale > 0 );
	ASSERT( iStartIndex >= 0 );
	ASSERT( iStartIndex < iEndIndex );

	int length = iEndIndex - iStartIndex;
	int newLength = std::lrint( fScale * length );

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
	lookup_updater_for_segment_add lookup_me_senpai(this);
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
	lookup_updater_for_segment_add lookup_me_senpai(this);
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
		int iSegmentEndRow = (i + 1 == tSigs.size()) ? std::numeric_limits<int>::max() : curSig->GetRow();

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

vector<std::string> TimingData::ToVectorString(TimingSegmentType tst, int dec) const
{
	const vector<TimingSegment *> segs = GetTimingSegments(tst);
	vector<std::string> ret;
	for (auto const *seg: segs)
	{
		ret.push_back(seg->ToString(dec));
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
