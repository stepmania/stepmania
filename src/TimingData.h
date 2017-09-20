#ifndef TIMING_DATA_H
#define TIMING_DATA_H

#include "NoteTypes.h"
#include "TimingSegments.h"
#include "PrefsManager.h"
#include <limits> // max float
struct lua_State;

/** @brief Compare a TimingData segment's properties with one another. */
#define COMPARE(x) if(this->x!=other.x) return false;

/* convenience functions to handle static casting */
template<class T>
inline T ToDerived( const TimingSegment *t, TimingSegmentType tst )
{
	ASSERT_M( t && tst == t->GetType(),
		fmt::sprintf("type mismatch (expected %s, got %s)",
		TimingSegmentTypeToString(tst).c_str(),
		TimingSegmentTypeToString(t->GetType()).c_str() ) );

	return static_cast<T>( t );
}

#define TimingSegmentToXWithName(Seg, SegName, SegType) \
	inline const Seg* To##SegName( const TimingSegment *t ) \
	{ \
		ASSERT( t->GetType() == SegType ); \
		return static_cast<const Seg*>( t ); \
	} \
	inline Seg* To##SegName( TimingSegment *t ) \
	{ \
		ASSERT( t->GetType() == SegType ); \
		return static_cast<Seg*>( t ); \
	}

#define TimingSegmentToX(Seg, SegType) \
	TimingSegmentToXWithName(Seg##Segment, Seg, SEGMENT_##SegType)

/* ToBPM(TimingSegment*), ToTimeSignature(TimingSegment*), etc. */
TimingSegmentToX( BPM, BPM );
TimingSegmentToX( Stop, STOP );
TimingSegmentToX( Delay, DELAY );
TimingSegmentToX( TimeSignature, TIME_SIG );
TimingSegmentToX( Warp, WARP );
TimingSegmentToX( Label, LABEL );
TimingSegmentToX( Tickcount, TICKCOUNT );
TimingSegmentToX( Combo, COMBO );
TimingSegmentToX( Speed, SPEED );
TimingSegmentToX( Scroll, SCROLL );
TimingSegmentToX( Fake, FAKE );

#undef TimingSegmentToXWithName
#undef TimingSegmentToX

/**
 * @brief Holds data for translating beats<->seconds.
 */
class TimingData
{
public:
	/**
	 * @brief Sets up initial timing data with a defined offset.
	 * @param fOffset the offset from the 0th beat. */
	TimingData( float fOffset = 0 );
	~TimingData();

	void Copy( const TimingData &other );
	void Clear();
	bool IsSafeFullTiming();

	TimingData( const TimingData &cpy ) { Copy(cpy); }
	TimingData& operator=( const TimingData &cpy ) { Copy(cpy); return *this; }

	// Let's try a different method for optimizing GetBeatFromElapsedTime.
	// Each timing segment is like a line segment.
	// GetBeat/GetElapsedTime finds the segment at the given time, then
	// linearly interpolates between its endpoints for the result.
	// This should be faster than stepping forward from a known start point.
	// RequestLookup should be called before gameplay starts, so that the lookup
	// tables are populated.  ReleaseLookup should be called after gameplay
	// finishes so that memory isn't wasted.
	// RequestLookup actually tracks a requester count and only builds the
	// lookup table if it hasn't already been requested.
	// PrepareLookup is internal, for updating the lookup table directly.
	// -Kyz
	struct LineSegment
	{
		float start_beat;
		float start_second;
		float end_beat;
		float end_second;
		// The expand modifier needs the second in a special form that doesn't
		// increase during stops. -Kyz
		float start_expand_second;
		float end_expand_second;
		// bps needed for SongPosition.
		float bps;
		TimingSegment* time_segment;

		void set_for_next()
		{
		start_beat= end_beat;
			start_second= end_second;
			start_expand_second= end_expand_second;
			time_segment= nullptr;
		}
	};
	// displayed_beat_entry is for optimizing GetDisplayedBeat, which is used
	// by scroll segments.
	struct displayed_beat_entry
	{
		float beat;
		float displayed_beat;
		float velocity;
	};
	private:
	mutable std::vector<LineSegment> m_line_segments;
	mutable std::map<float, std::vector<LineSegment*> > m_segments_by_beat;
	mutable std::map<float, std::vector<LineSegment*> > m_segments_by_second;

	void PrepareLineLookup(int search_mode, float search_time,
		LineSegment* search_ret) const;
	void ReleaseLineLookup() const;
	float GetLineBeatFromSecond(float second) const;
	float GetLineSecondFromBeat(float beat) const;

	mutable std::map<float, displayed_beat_entry> m_displayed_beat_lookup;
	// m_lookup_requester_count exists to track how many things have requested
	// the lookup tables be prepared, so unrelated parts of code don't have to
	// check for them.
	// The lookup tables are only created if m_lookup_requester_count is 0 when
	// PrepareLookup is called, and only released when it reaches 0 again.
	// -Kyz
	mutable int m_lookup_requester_count;

	void ReleaseDisplayedBeatLookup() const;
	void ReleaseLookupInternal() const;

	public:
	float GetExpandSeconds(float second) const;

	struct DetailedTimeInfo
	{
		float second;
		float beat;
		float bps_out;
		float warp_dest_out;
		int warp_begin_out;
		bool freeze_out;
		bool delay_out;
	DetailedTimeInfo() :second(0), beat(0), bps_out(0), warp_dest_out(0),
			warp_begin_out(-1), freeze_out(false), delay_out(false) {}
	};

	void RequestLookup() const;
	void PrepareLookup() const;
	void ReleaseLookup() const;
	void DumpLookupTables() const;
	int get_lookup_requester_count() { return m_lookup_requester_count; }

	int GetSegmentIndexAtRow(TimingSegmentType tst, int row) const;
	int GetSegmentIndexAtBeat(TimingSegmentType tst, float beat) const
	{
		return GetSegmentIndexAtRow( tst, BeatToNoteRow(beat) );
	}

	float GetNextSegmentBeatAtRow(TimingSegmentType tst, int row) const;
	float GetNextSegmentBeatAtBeat(TimingSegmentType tst, float beat) const
	{
		return GetNextSegmentBeatAtRow( tst, BeatToNoteRow(beat) );
	}

	float GetPreviousSegmentBeatAtRow(TimingSegmentType tst, int row) const;
	float GetPreviousSegmentBeatAtBeat(TimingSegmentType tst, float beat) const
	{
		return GetPreviousSegmentBeatAtRow( tst, BeatToNoteRow(beat) );
	}

	bool empty() const;

	void CopyRange(int start_row, int end_row, TimingSegmentType copy_type, int dest_row, TimingData& dest) const;
	void ShiftRange(int start_row, int end_row, TimingSegmentType shift_type, int shift_amount);
	void ClearRange(int start_row, int end_row, TimingSegmentType clear_type);
	/**
	 * @brief Gets the actual BPM of the song,
	 * while respecting a limit.
	 *
	 * The high limit is due to the implementation of mMods.
	 * @param fMinBPMOut the minimium specified BPM.
	 * @param fMaxBPMOut the maximum specified BPM.
	 * @param highest the highest allowed max BPM.
	 */
    void GetActualBPM( float &fMinBPMOut, float &fMaxBPMOut, float highest = std::numeric_limits<float>::max() ) const;

	/**
	 * @brief Retrieve the TimingSegment at the specified row.
	 * @param iNoteRow the row that has a TimingSegment.
	 * @param tst the TimingSegmentType requested.
	 * @return the segment in question.
	 */
	const TimingSegment* GetSegmentAtRow( int iNoteRow, TimingSegmentType tst ) const;
	TimingSegment* GetSegmentAtRow( int iNoteRow, TimingSegmentType tst );

	/**
	 * @brief Retrieve the TimingSegment at the given beat.
	 * @param fBeat the beat that has a TimingSegment.
	 * @param tst the TimingSegmentType requested.
	 * @return the segment in question.
	 */
	const TimingSegment* GetSegmentAtBeat( float fBeat, TimingSegmentType tst ) const
	{
		return GetSegmentAtRow( BeatToNoteRow(fBeat), tst );
	}
	TimingSegment* GetSegmentAtBeat( float fBeat, TimingSegmentType tst )
	{
		return const_cast<TimingSegment*>( const_cast<const TimingData*>(this)->GetSegmentAtBeat(fBeat, tst) );
	}

	#define DefineSegmentWithName(Seg, SegName, SegType) \
		const Seg* Get##Seg##AtRow( int iNoteRow ) const \
		{ \
			const TimingSegment *t = GetSegmentAtRow( iNoteRow, SegType ); \
			return To##SegName( t ); \
		} \
		Seg* Get##Seg##AtRow( int iNoteRow ) \
		{ \
			return const_cast<Seg*> (((const TimingData*)this)->Get##Seg##AtRow(iNoteRow) ); \
		} \
		const Seg* Get##Seg##AtBeat( float fBeat ) const \
		{ \
			return Get##Seg##AtRow( BeatToNoteRow(fBeat) ); \
		} \
		Seg* Get##Seg##AtBeat( float fBeat ) \
		{ \
			return const_cast<Seg*> (((const TimingData*)this)->Get##Seg##AtBeat(fBeat) ); \
		} \
		void AddSegment( const Seg &seg ) \
		{ \
			AddSegment( &seg ); \
		}

	// "XXX: this comment (and quote mark) exists so nano won't
	// display the rest of this file as one giant string

	// (TimeSignature,TIME_SIG) -> (TimeSignatureSegment,SEGMENT_TIME_SIG)
	#define DefineSegment(Seg, SegType ) \
		DefineSegmentWithName( Seg##Segment, Seg, SEGMENT_##SegType )

	DefineSegment( BPM, BPM );
	DefineSegment( Stop, STOP );
	DefineSegment( Delay, DELAY );
	DefineSegment( Warp, WARP );
	DefineSegment( Label, LABEL );
	DefineSegment( Tickcount, TICKCOUNT );
	DefineSegment( Combo, COMBO );
	DefineSegment( Speed, SPEED );
	DefineSegment( Scroll, SCROLL );
	DefineSegment( Fake, FAKE );
	DefineSegment( TimeSignature, TIME_SIG );

	#undef DefineSegmentWithName
	#undef DefineSegment

	/* convenience aliases (Set functions are deprecated) */
	float GetBPMAtRow( int iNoteRow ) const { return GetBPMSegmentAtRow(iNoteRow)->GetBPM(); }
	float GetBPMAtBeat( float fBeat ) const { return GetBPMAtRow( BeatToNoteRow(fBeat) ); }
	void SetBPMAtRow( int iNoteRow, float fBPM ) { AddSegment( BPMSegment(iNoteRow, fBPM) ); }
	void SetBPMAtBeat( float fBeat, float fBPM ) { SetBPMAtRow( BeatToNoteRow(fBeat), fBPM ); }

	float GetStopAtRow( int iNoteRow ) const { return GetStopSegmentAtRow(iNoteRow)->GetPause(); }
	float GetStopAtBeat( float fBeat ) const { return GetStopAtRow( BeatToNoteRow(fBeat) ); }
	void SetStopAtRow( int iNoteRow, float fSeconds ) { AddSegment( StopSegment(iNoteRow, fSeconds) ); }
	void SetStopAtBeat( float fBeat, float fSeconds ) { SetStopAtRow( BeatToNoteRow(fBeat), fSeconds ); }

	float GetDelayAtRow( int iNoteRow ) const { return GetDelaySegmentAtRow(iNoteRow)->GetPause(); }
	float GetDelayAtBeat( float fBeat ) const { return GetDelayAtRow( BeatToNoteRow(fBeat) ); }
	void SetDelayAtRow( int iNoteRow, float fSeconds ) { AddSegment( DelaySegment(iNoteRow, fSeconds) ); }
	void SetDelayAtBeat( float fBeat, float fSeconds ) { SetDelayAtRow( BeatToNoteRow(fBeat), fSeconds ); }

	void SetTimeSignatureAtRow( int iNoteRow, int iNum, int iDen )
	{
		AddSegment( TimeSignatureSegment(iNoteRow, iNum, iDen) );
	}

	void SetTimeSignatureAtBeat( float fBeat, int iNum, int iDen )
	{
		SetTimeSignatureAtRow( BeatToNoteRow(fBeat), iNum, iDen );
	}

	float GetWarpAtRow( int iNoteRow ) const { return GetWarpSegmentAtRow(iNoteRow)->GetLength(); }
	float GetWarpAtBeat( float fBeat ) const { return GetWarpAtRow( BeatToNoteRow(fBeat) ); }
	/* Note: fLength is in beats, not rows */
	void SetWarpAtRow( int iRow, float fLength ) { AddSegment( WarpSegment(iRow, fLength) ); }
	void SetWarpAtBeat( float fBeat, float fLength ) { AddSegment( WarpSegment(BeatToNoteRow(fBeat), fLength) ); }

	int GetTickcountAtRow( int iNoteRow ) const { return GetTickcountSegmentAtRow(iNoteRow)->GetTicks(); }
	int GetTickcountAtBeat( float fBeat ) const { return GetTickcountAtRow( BeatToNoteRow(fBeat) ); }
	void SetTickcountAtRow( int iNoteRow, int iTicks ) { AddSegment( TickcountSegment(iNoteRow, iTicks) ); }
	void SetTickcountAtBeat( float fBeat, int iTicks ) { SetTickcountAtRow( BeatToNoteRow( fBeat ), iTicks ); }

	int GetComboAtRow( int iNoteRow ) const { return GetComboSegmentAtRow(iNoteRow)->GetCombo(); }
	int GetComboAtBeat( float fBeat ) const { return GetComboAtRow( BeatToNoteRow(fBeat) ); }
	int GetMissComboAtRow( int iNoteRow ) const { return GetComboSegmentAtRow(iNoteRow)->GetMissCombo(); }
	int GetMissComboAtBeat( float fBeat ) const { return GetMissComboAtRow( BeatToNoteRow(fBeat) ); }

	const std::string& GetLabelAtRow( int iNoteRow ) const { return GetLabelSegmentAtRow(iNoteRow)->GetLabel(); }
	const std::string& GetLabelAtBeat( float fBeat ) const { return GetLabelAtRow( BeatToNoteRow(fBeat) ); }
	void SetLabelAtRow( int iNoteRow, const std::string& sLabel ) { AddSegment( LabelSegment(iNoteRow,sLabel) ); }
	void SetLabelAtBeat( float fBeat, const std::string sLabel ) { SetLabelAtRow( BeatToNoteRow( fBeat ), sLabel ); }
	bool DoesLabelExist( const std::string& sLabel ) const;

	float GetSpeedPercentAtRow( int iNoteRow ) const { return GetSpeedSegmentAtRow(iNoteRow)->GetRatio(); }
	float GetSpeedPercentAtBeat( float fBeat ) const { return GetSpeedPercentAtRow( BeatToNoteRow(fBeat) ); }

	float GetSpeedWaitAtRow( int iNoteRow ) const { return GetSpeedSegmentAtRow(iNoteRow)->GetDelay(); }
	float GetSpeedWaitAtBeat( float fBeat ) const { return GetSpeedWaitAtRow( BeatToNoteRow(fBeat) ); }

	// XXX: is there any point to having specific unit types?
	SpeedSegment::BaseUnit GetSpeedModeAtRow( int iNoteRow ) const { return GetSpeedSegmentAtRow(iNoteRow)->GetUnit(); }
	SpeedSegment::BaseUnit GetSpeedModeAtBeat( float fBeat ) { return GetSpeedModeAtRow( BeatToNoteRow(fBeat) ); }

	void SetSpeedAtRow( int iNoteRow, float fPercent, float fWait, SpeedSegment::BaseUnit unit )
	{
		AddSegment( SpeedSegment(iNoteRow, fPercent, fWait, unit) );
	}

	void SetSpeedAtBeat( float fBeat, float fPercent, float fWait, SpeedSegment::BaseUnit unit )
	{
		SetSpeedAtRow( BeatToNoteRow(fBeat), fPercent, fWait, unit );
	}

	void SetSpeedPercentAtRow( int iNoteRow, float fPercent )
	{
		const SpeedSegment* seg = GetSpeedSegmentAtRow(iNoteRow);
		SetSpeedAtRow( iNoteRow, fPercent, seg->GetDelay(), seg->GetUnit() );
	}

	void SetSpeedWaitAtRow( int iNoteRow, float fWait )
	{
		const SpeedSegment* seg = GetSpeedSegmentAtRow(iNoteRow);
		SetSpeedAtRow( iNoteRow, seg->GetRatio(), fWait, seg->GetUnit() );
	}

	void SetSpeedModeAtRow( int iNoteRow, SpeedSegment::BaseUnit unit )
	{
		const SpeedSegment* seg = GetSpeedSegmentAtRow(iNoteRow);
		SetSpeedAtRow( iNoteRow, seg->GetRatio(), seg->GetDelay(), unit );
	}

	void SetSpeedPercentAtBeat( float fBeat, float fPercent ) { SetSpeedPercentAtRow( BeatToNoteRow(fBeat), fPercent); }
	void SetSpeedWaitAtBeat( float fBeat, float fWait ) { SetSpeedWaitAtRow( BeatToNoteRow(fBeat), fWait); }
	void SetSpeedModeAtBeat( float fBeat, SpeedSegment::BaseUnit unit ) { SetSpeedModeAtRow( BeatToNoteRow(fBeat), unit); }

	float GetDisplayedSpeedPercent( float fBeat, float fMusicSeconds ) const;


	float GetScrollAtRow( int iNoteRow ) const { return GetScrollSegmentAtRow(iNoteRow)->GetRatio(); }
	float GetScrollAtBeat( float fBeat ) { return GetScrollAtRow( BeatToNoteRow(fBeat) ); }

	void SetScrollAtRow( int iNoteRow, float fPercent ) { AddSegment( ScrollSegment(iNoteRow, fPercent) ); }
	void SetScrollAtBeat( float fBeat, float fPercent ) { SetScrollAtRow( BeatToNoteRow(fBeat), fPercent ); }

	float GetFakeAtRow( int iRow ) const { return GetFakeSegmentAtRow(iRow)->GetLength(); }
	float GetFakeAtBeat( float fBeat ) const { return GetFakeAtRow( BeatToNoteRow( fBeat ) ); }

	bool IsWarpAtRow( int iRow ) const;
	bool IsWarpAtBeat( float fBeat ) const { return IsWarpAtRow( BeatToNoteRow( fBeat ) ); }
	bool IsFakeAtRow( int iRow ) const;
	bool IsFakeAtBeat( float fBeat ) const { return IsFakeAtRow( BeatToNoteRow( fBeat ) ); }

	/**
	 * @brief Determine if this notes on this row can be judged.
	 * @param row the row to focus on.
	 * @return true if the row can be judged, false otherwise. */
	bool IsJudgableAtRow( int row ) const { return !IsWarpAtRow(row) && !IsFakeAtRow(row); }
	bool IsJudgableAtBeat( float beat ) const { return IsJudgableAtRow( BeatToNoteRow( beat ) ); }

	void MultiplyBPMInBeatRange( int iStartIndex, int iEndIndex, float fFactor );

	void NoteRowToMeasureAndBeat( int iNoteRow, int &iMeasureIndexOut, int &iBeatIndexOut, int &iRowsRemainder ) const;

	void GetDetailedInfoForSecond(DetailedTimeInfo& args) const;
	float GetBeatFromElapsedTime(float second) const;
	float GetElapsedTimeFromBeat(float beat) const;

	void GetDetailedInfoForSecondNoOffset(DetailedTimeInfo& args) const;
	float GetBeatFromElapsedTimeNoOffset(float second) const;
	float GetElapsedTimeFromBeatNoOffset(float beat) const;
	float GetDisplayedBeat(float beat) const;

	bool HasBpmChanges() const { return GetTimingSegments(SEGMENT_BPM).size() > 1; }
	bool HasStops() const { return !GetTimingSegments(SEGMENT_STOP).empty(); }
	bool HasDelays() const { return !GetTimingSegments(SEGMENT_DELAY).empty(); }
	bool HasWarps() const { return !GetTimingSegments(SEGMENT_WARP).empty(); }
	bool HasFakes() const { return !GetTimingSegments(SEGMENT_FAKE).empty(); }

	bool HasSpeedChanges() const;
	bool HasScrollChanges() const;

	/**
	 * @brief Compare two sets of timing data to see if they are equal.
	 * @param other the other TimingData.
	 * @return the equality or lack thereof of the two TimingData.
	 */
	bool operator==( const TimingData &other )
	{
		FOREACH_ENUM( TimingSegmentType, tst )
		{
			const std::vector<TimingSegment*> &us = m_avpTimingSegments[tst];
			const std::vector<TimingSegment*> &them = other.m_avpTimingSegments[tst];

			// optimization: check vector sizes before contents
			if( us.size() != them.size() )
				return false;

			for( unsigned i = 0; i < us.size(); ++i )
			{
				/* UGLY: since TimingSegment's comparison compares base data,
				 * and the derived versions only compare derived data, we must
				 * manually call each. */
				if( !(*us[i]).TimingSegment::operator==(*them[i]) )
					return false;
				if( !(*us[i]).operator==(*them[i]) )
					return false;
			}
		}

		COMPARE( m_fBeat0OffsetInSeconds );
		return true;
	}

	/**
	 * @brief Compare two sets of timing data to see if they are not equal.
	 * @param other the other TimingData.
	 * @return the inequality or lack thereof of the two TimingData.
	 */
	bool operator!=( const TimingData &other ) { return !operator==(other); }

	void ScaleRegion( float fScale = 1, int iStartRow = 0, int iEndRow = MAX_NOTE_ROW, bool bAdjustBPM = false );
	void InsertRows( int iStartRow, int iRowsToAdd );
	void DeleteRows( int iStartRow, int iRowsToDelete );

	void SortSegments( TimingSegmentType tst );

	const std::vector<TimingSegment*> &GetTimingSegments( TimingSegmentType tst ) const
	{
		return const_cast<TimingData *>(this)->GetTimingSegments(tst);
	}
	std::vector<TimingSegment *> &GetTimingSegments( TimingSegmentType tst )
	{
		return m_avpTimingSegments[tst];
	}

	/**
	 * @brief Tidy up the timing data, e.g. provide default BPMs, labels, tickcounts.
	 * @param allowEmpty true if completely empty TimingData should be left
	 *                   alone, false if it should be changed
	 */
	void TidyUpData(bool allowEmpty);

	// Lua
	void PushSelf( lua_State *L );

	/**
	 * @brief The file of the song/steps that use this TimingData.
	 *
	 * This is for informational purposes only.
	 */
	std::string					m_sFile;

	/** @brief The initial offset of a song. */
	void adjust_offset(float amount) { set_offset(m_fBeat0OffsetInSeconds + amount); }
	void set_offset(float off);
	float get_offset() const;

	// XXX: this breaks encapsulation. get rid of it ASAP
	std::vector<std::string> ToVectorString(TimingSegmentType tst, int dec = 6) const;
protected:
	float	m_fBeat0OffsetInSeconds;
	// don't call this directly; use the derived-type overloads.
	void AddSegment( const TimingSegment *seg );

	// All of the following vectors must be sorted before gameplay.
	std::vector<TimingSegment *> m_avpTimingSegments[NUM_TimingSegmentType];
};

#undef COMPARE

#endif

/**
 * @file
 * @author Chris Danford, Glenn Maynard (c) 2001-2004
 * @section LICENSE
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
