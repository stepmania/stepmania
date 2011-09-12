#ifndef TIMING_SEGMENTS_H
#define TIMING_SEGMENTS_H

#include "NoteTypes.h" // Converting rows to beats and vice~versa.

enum TimingSegmentType
{
	SEGMENT_BPM,
	SEGMENT_STOP,
	SEGMENT_DELAY,
	SEGMENT_TIME_SIG,
	SEGMENT_WARP,
	SEGMENT_LABEL,
	SEGMENT_TICKCOUNT,
	SEGMENT_COMBO,
	SEGMENT_SPEED,
	SEGMENT_SCROLL,
	SEGMENT_FAKE,
	NUM_TimingSegmentType,
	TimingSegmentType_Invalid,
};

const RString& TimingSegmentTypeToString( TimingSegmentType tst );

const int ROW_INVALID = -1;

/**
 * @brief The base timing segment for make glorious benefit wolfman
 */
struct TimingSegment
{
	virtual TimingSegmentType GetType() const
	{
		return TimingSegmentType_Invalid;
	}

	TimingSegment( int iRow = ROW_INVALID ) : m_iStartRow(iRow) { }
	TimingSegment( float fBeat ) : m_iStartRow(ToNoteRow(fBeat)) { }

	TimingSegment(const TimingSegment &other) :
		m_iStartRow( other.GetRow() ) { }

	virtual ~TimingSegment() { }

	/**
	 * @brief Scales itself.
	 * @param start Starting row
	 * @param length Length in rows
	 * @param newLength The new length in rows
	 */
	virtual void Scale( int start, int length, int newLength );

	int GetRow() const { return m_iStartRow; }
	void SetRow( int iRow ) { m_iStartRow = iRow; }

	float GetBeat() const { return NoteRowToBeat(m_iStartRow); }
	void SetBeat( float fBeat ) { SetRow( BeatToNoteRow(fBeat) ); }

	virtual RString ToString(int dec) const
	{
		return FloatToString(GetBeat());
	}

	bool operator<( const TimingSegment &other ) const
	{
		return GetRow() < other.GetRow();
	}

	virtual bool operator==( const TimingSegment &other ) const
	{
		return GetRow() == other.GetRow();
	}

	virtual bool operator!=( const TimingSegment &other ) const
	{
		return !operator==(other);
	}

private:
	/** @brief The row in which this segment activates. */
	int m_iStartRow;
};

/**
 * @brief Identifies when a whole region of arrows is to be ignored.
 *
 * FakeSegments are similar to the Fake Tap Notes in that the contents
 * inside are neither for nor against the player. They can be useful for
 * mission modes, in conjunction with WarpSegments, or perhaps other
 * uses not thought up at the time of this comment. Unlike the Warp
 * Segments, these are not magically jumped over: instead, these are
 * drawn normally.
 *
 * These were inspired by the Pump It Up series. */
struct FakeSegment : public TimingSegment
{
	TimingSegmentType GetType() const { return SEGMENT_FAKE; }

	FakeSegment() : TimingSegment(), m_iLengthRows(-1) { }

	FakeSegment( int iStartRow, int iLengthRows ) :
		TimingSegment(iStartRow), m_iLengthRows(iLengthRows) { }

	FakeSegment( int iStartRow, float fBeats ) :
		TimingSegment(iStartRow), m_iLengthRows(ToNoteRow(fBeats)) { }

	FakeSegment( const FakeSegment &other ) :
		TimingSegment( other.GetRow() ),
		m_iLengthRows( other.GetLengthRows() ) { }

	int GetLengthRows() const { return m_iLengthRows; }
	float GetLengthBeats() const { return ToBeat(m_iLengthRows); }
	float GetLength() const { return GetLengthBeats(); } // compatibility

	void SetLength( int iRows ) { m_iLengthRows = ToNoteRow(iRows); }
	void SetLength( float fBeats ) { m_iLengthRows = ToNoteRow(fBeats); }

	void Scale( int start, int length, int newLength );

	RString ToString( int dec ) const;
private:
	/** @brief The number of rows the FakeSegment is alive for. */
	int m_iLengthRows;
};

/**
 * @brief Identifies when a song needs to warp to a new beat.
 *
 * A warp segment is used to replicate the effects of Negative BPMs without
 * abusing negative BPMs. Negative BPMs should be converted to warp segments.
 * WarpAt=WarpToRelative is the format, where both are in beats.
 * (Technically they're both rows though.) */
struct WarpSegment : public TimingSegment
{
	TimingSegmentType GetType() const { return SEGMENT_WARP; }

	WarpSegment() : TimingSegment(), m_iLengthRows(0) { }

	WarpSegment( const WarpSegment &other ) :
		TimingSegment( other.GetRow() ),
		m_iLengthRows( other.GetLengthRows() ) { }

	WarpSegment( int iStartRow, int iLengthRows ) :
		TimingSegment(iStartRow), m_iLengthRows(iLengthRows) { }

	WarpSegment( int iStartRow, float fBeats ) :
		TimingSegment(iStartRow), m_iLengthRows(ToNoteRow(fBeats)) { }

	int GetLengthRows() const { return m_iLengthRows; }
	float GetLengthBeats() const { return ToBeat(m_iLengthRows); }
	float GetLength() const { return GetLengthBeats(); } // compatibility

	void SetLength( int iRows ) { m_iLengthRows = ToNoteRow(iRows); }
	void SetLength( float fBeats ) { m_iLengthRows = ToNoteRow(fBeats); }

	void Scale( int start, int length, int newLength );

	RString ToString( int dec ) const;
private:
	/** @brief The number of rows the WarpSegment will warp past. */
	int m_iLengthRows;
};

/**
 * @brief Identifies when a chart is to have a different tickcount value
 * for hold notes.
 *
 * A tickcount segment is used to better replicate the checkpoint hold
 * system used by various based video games. The number is used to
 * represent how many ticks can be counted in one beat.
 */

/** @brief The default amount of ticks per beat. */
const unsigned DEFAULT_TICK_COUNT = 4;

struct TickcountSegment : public TimingSegment
{
	TimingSegmentType GetType() const { return SEGMENT_TICKCOUNT; }

	TickcountSegment( int iStartRow = ROW_INVALID, int iTicks = DEFAULT_TICK_COUNT ) :
		TimingSegment(iStartRow), m_iTicksPerBeat(iTicks) { }

	TickcountSegment( const TickcountSegment &other ) :
		TimingSegment( other.GetRow() ),
		m_iTicksPerBeat( other.GetTicks() ) { }

	int GetTicks() const { return m_iTicksPerBeat; }
	void SetTicks( int iTicks ) { m_iTicksPerBeat = iTicks; }

	RString ToString( int dec ) const;
private:
	/** @brief The amount of hold checkpoints counted per beat */
	int m_iTicksPerBeat;
};

/**
 * @brief Identifies when a chart is to have a different combo multiplier value.
 *
 * Admitedly, this would primarily be used for mission mode style charts. However,
 * it can have its place during normal gameplay.
 */
struct ComboSegment : public TimingSegment
{
	TimingSegmentType GetType() const { return SEGMENT_COMBO; }

	ComboSegment( int iStartRow = ROW_INVALID, int iCombo = 1, int iMissCombo = 1 ) :
		TimingSegment(iStartRow), m_iCombo(iCombo),
		m_iMissCombo(iMissCombo) { }

	ComboSegment(const ComboSegment &other) :
		TimingSegment( other.GetRow() ),
		m_iCombo( other.GetCombo() ),
		m_iMissCombo( other.GetMissCombo() ) { }

	int GetCombo() const { return m_iCombo; }
	int GetMissCombo() const { return m_iMissCombo; }

	void SetCombo( int iCombo ) { m_iCombo = iCombo; }
	void SetMissCombo( int iCombo ) { m_iMissCombo = iCombo; }

	RString ToString( int dec ) const;
private:
	/** @brief The amount the combo increases at this point. */
	int m_iCombo;

	/** @brief The amount of miss combos given at this point. */
	int m_iMissCombo;
};


/**
 * @brief Identifies when a chart is entering a different section.
 *
 * This is meant for helping to identify different sections of a chart
 * versus relying on measures and beats alone.
 */
struct LabelSegment : public TimingSegment
{
	TimingSegmentType GetType() const { return SEGMENT_LABEL; }

	LabelSegment( int iStartRow = ROW_INVALID, const RString& sLabel = RString() ) :
		TimingSegment(), m_sLabel(sLabel) { }

	LabelSegment(const LabelSegment &other) :
		TimingSegment( other.GetRow() ),
		m_sLabel( other.GetLabel() ) { }

	const RString& GetLabel() const { return m_sLabel; }
	void SetLabel( const RString& sLabel ) { m_sLabel.assign(sLabel); }

	RString ToString( int dec ) const;
private:
	/** @brief The label/section name for this point. */
	RString m_sLabel;
};

/**
 * @brief Identifies when a song changes its BPM.
 */
struct BPMSegment : public TimingSegment
{
	TimingSegmentType GetType() const { return SEGMENT_BPM; }

	// note that this takes a BPM, not a BPS (compatibility)
	BPMSegment( int iStartRow = ROW_INVALID, float fBPM = 0.0f ) :
		TimingSegment(iStartRow) { SetBPM(fBPM); }

	BPMSegment( const BPMSegment &other ) :
		TimingSegment( other.GetRow() ),
		m_fBPS( other.GetBPS() ) { }

	float GetBPS() const { return m_fBPS; }
	float GetBPM() const { return m_fBPS * 60.0f; }

	void SetBPS( float fBPS ) { m_fBPS = fBPS; }
	void SetBPM( float fBPM ) { m_fBPS = fBPM / 60.0f; }

	RString ToString( int dec ) const;
private:
	/** @brief The number of beats per second within this BPMSegment. */
	float m_fBPS;
};

/**
 * @brief Identifies when a song changes its time signature.
 *
 * This only supports simple time signatures. The upper number
 * (called the numerator here, though this isn't properly a
 * fraction) is the number of beats per measure. The lower number
 * (denominator here) is the note value representing one beat. */
struct TimeSignatureSegment : public TimingSegment
{
	TimingSegmentType GetType() const { return SEGMENT_TIME_SIG; }

	TimeSignatureSegment( int iStartRow = ROW_INVALID,
	  int iNum = 4, int iDenom = 4 ) :
		TimingSegment(iStartRow), m_iNumerator(iNum),
		m_iDenominator(iDenom) { }

	TimeSignatureSegment( const TimeSignatureSegment &other ) :
		TimingSegment( other.GetRow() ),
		m_iNumerator( other.GetNum() ),
		m_iDenominator( other.GetDen() ) { }

	int GetNum() const { return m_iNumerator; }
	void SetNum( int num ) { m_iNumerator = num; }

	int GetDen() const { return m_iDenominator; }
	void SetDen( int den ) { m_iDenominator = den; }

	RString ToString( int dec ) const;

	/**
	 * @brief Retrieve the number of note rows per measure within the TimeSignatureSegment.
	 *
	 * With BeatToNoteRow(1) rows per beat, then we should have BeatToNoteRow(1)*m_iNumerator
	 * beats per measure. But if we assume that every BeatToNoteRow(1) rows is a quarter note,
	 * and we want the beats to be 1/m_iDenominator notes, then we should have
	 * BeatToNoteRow(1)*4 is rows per whole note and thus BeatToNoteRow(1)*4/m_iDenominator is
	 * rows per beat. Multiplying by m_iNumerator gives rows per measure.
	 * @returns the number of note rows per measure.
	 */
	int GetNoteRowsPerMeasure() const
	{
		return BeatToNoteRow(1) * 4 * m_iNumerator / m_iDenominator;
	}

private:
	int m_iNumerator, m_iDenominator;
};

/**
 * @brief Identifies when the arrow scroll changes.
 *
 * SpeedSegments take a Player's scrolling BPM (Step's BPM * speed mod),
 * and then multiplies it with the percentage value. No matter the player's
 * speed mod, the ratio will be the same. Unlike forced attacks, these
 * cannot be turned off at a set time: reset it by setting the percentage
 * back to 1.
 *
 * These were inspired by the Pump It Up series. */
struct SpeedSegment : public TimingSegment
{
	TimingSegmentType GetType() const { return SEGMENT_SPEED; }

	/** @brief The type of unit used for segment scaling. */
	enum BaseUnit { UNIT_BEATS, UNIT_SECONDS };

	SpeedSegment( int iStartRow = ROW_INVALID, float fRatio = 1.0f,
	  float fDelay = 0.0f, BaseUnit unit = UNIT_BEATS ) :
		TimingSegment(iStartRow), m_fRatio(fRatio), m_fDelay(fDelay),
		m_Unit(unit) { }

	SpeedSegment(const SpeedSegment &other) :
		TimingSegment( other.GetRow() ),
		m_fRatio( other.GetRatio() ),
		m_fDelay( other.GetDelay() ),
		m_Unit( other.GetUnit() ) { }

	float GetRatio() const { return m_fRatio; }
	void SetRatio( float fRatio ) { m_fRatio = fRatio; }

	float GetDelay() const { return m_fDelay; }
	void SetDelay( float fDelay ) { m_fDelay = fDelay; }

	BaseUnit GetUnit() const { return m_Unit; }
	void SetUnit( BaseUnit unit ) { m_Unit = unit; }

	void Scale( int start, int length, int newLength );

	RString ToString( int dec ) const;
private:
	/** @brief The percentage by which the Player's BPM is multiplied. */
	float m_fRatio;

	/**
	 * @brief The number of beats or seconds to wait before applying.
	 * A value of 0 means this is immediate. */
	float m_fDelay;

	/** @brief The mode that this segment uses for the math.  */
	BaseUnit m_Unit;
};

/**
 * @brief Identifies when the chart scroll changes.
 *
 * ScrollSegments adjusts the scrolling speed of the note field.
 * Unlike forced attacks, these cannot be turned off at a set time:
 * reset it by setting the precentage back to 1.
 *
 * These were inspired by the Pump It Up series. */
struct ScrollSegment : public TimingSegment
{
	TimingSegmentType GetType() const { return SEGMENT_SCROLL; }

	ScrollSegment( int iStartRow = ROW_INVALID, float fRatio = 1.0f ) :
		TimingSegment(iStartRow), m_fRatio(fRatio) { }

	ScrollSegment(const ScrollSegment &other) :
		TimingSegment( other.GetRow() ),
		m_fRatio( other.GetRatio() ) { }

	float GetRatio() const { return m_fRatio; }
	void SetRatio( float fRatio ) { m_fRatio = fRatio; }

	RString ToString( int dec ) const;
private:
	/** @brief The percentage by which the chart's scroll rate is multiplied. */
	float m_fRatio;
};

/**
 * @brief Identifies when a song has a stop, DDR/ITG style.
 */
struct StopSegment : public TimingSegment
{
	TimingSegmentType GetType() const { return SEGMENT_STOP; }

	StopSegment( int iStartRow = ROW_INVALID, float fSeconds = 0.0f ) :
		TimingSegment(iStartRow), m_fSeconds(fSeconds) { }

	StopSegment (const StopSegment &other) :
		TimingSegment( other.GetRow() ),
		m_fSeconds( other.GetPause() ) { }

	float GetPause() const { return m_fSeconds; }
	void SetPause( float fSeconds ) { m_fSeconds = fSeconds; }

	RString ToString( int dec ) const;
private:
	/** @brief The number of seconds to pause at the segment's row. */
	float m_fSeconds;
};

/**
 * @brief Identifies when a song has a delay, or pump style stop.
 */
struct DelaySegment : public TimingSegment
{
	TimingSegmentType GetType() const { return SEGMENT_DELAY; }

	DelaySegment( int iStartRow, float fSeconds ) :
		TimingSegment(iStartRow), m_fSeconds(fSeconds) { }

	DelaySegment( const DelaySegment &other ) :
		TimingSegment( other.GetRow() ),
		m_fSeconds( other.GetPause() ) { }

	float GetPause() const { return m_fSeconds; }
	void SetPause( float fSeconds ) { m_fSeconds = fSeconds; }

	RString ToString( int dec ) const;
private:
	/** @brief The number of seconds to pause at the segment's row. */
	float m_fSeconds;
};


#endif

/**
 * @file
 * @author Jason Felds (c) 2011
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
