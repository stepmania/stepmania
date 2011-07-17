#ifndef TIMING_SEGMENTS_H
#define TIMING_SEGMENTS_H

#include "NoteTypes.h" // Converting rows to beats and vice~versa.

enum TimingSegmentType
{
	SEGMENT_BPM,
	SEGMENT_STOP_DELAY,
	// uncomment the below two when stops and delays don't share one.
	// SEGMENT_STOP,
	// SEGMENT_DELAY,
	SEGMENT_TIME_SIG,
	SEGMENT_WARP,
	SEGMENT_LABEL,
	SEGMENT_TICKCOUNT,
	SEGMENT_COMBO,
	SEGMENT_SPEED,
	SEGMENT_SCROLL,
	SEGMENT_FAKE,
	NUM_TimingSegmentTypes
};

/**
 * @brief The base timing segment for all of the changing glory.
 */
struct TimingSegment
{

	/** @brief Set up a TimingSegment with default values. */
	TimingSegment():
		startingRow(-1) {};

	/**
	 * @brief Set up a TimingSegment with specified values.
	 * @param s the starting row / beat. */
	TimingSegment(int   s): startingRow(ToNoteRow(s)) {}
	TimingSegment(float s): startingRow(ToNoteRow(s)) {}

	TimingSegment(const TimingSegment &other):
		startingRow(other.GetRow()) {};
	
	virtual ~TimingSegment();
	
	/**
	 * @brief Scales itself.
	 * @param start Starting row
	 * @param length Length in rows
	 * @param newLength The new length in rows
	 */
	virtual void Scale( int start, int length, int newLength );
	
	/**
	 * @brief Set the starting row of the BaseTimingSegment.
	 *
	 * This is virtual to allow other segments to implement validation
	 * as required by them.
	 * @param s the supplied row. */
	virtual void SetRow( const int s );
	
	/**
	 * @brief Set the starting beat of the BaseTimingSegment.
	 *
	 * @param s the supplied beat. */
	void SetBeat( const float s );
	
	/**
	 * @brief Get the starting row of the BaseTimingSegment.
	 * @return the starting row. */
	int GetRow() const;
	
	/**
	 * @brief Get the starting beat of the BaseTimingSegment.
	 * @return the starting beat. */
	float GetBeat() const;
	
	virtual TimingSegmentType GetType() const = 0;
	
	/**
	 * @brief Compares two DrivedSegments to see if one is less than the other.
	 * @param other the other TimingSegments to compare to.
	 * @return the truth/falsehood of if the first is less than the second.
	 *
	 * This is virtual to allow other segments to implement comparison
	 * as required by them.
	 */
	bool operator<( const TimingSegment &other ) const
	{ 
		return this->GetRow() < other.GetRow();
	};
	
	/**
	 * @brief Compares two DrivedSegments to see if they are equal to each other.
	 * @param other the other FakeSegment to compare to.
	 * @return the equality of the two segments.
	 *
	 * This is virtual to allow other segments to implement comparison
	 * as required by them.
	 */
	bool operator==( const TimingSegment &other ) const
	{
		return !this->operator<(other) && 
		!other.operator<(*static_cast<const TimingSegment *>(this));
	};	
	/**
	 * @brief Compares two DrivedSegments to see if they are not equal to each other.
	 * @param other the other DrivedSegments to compare to.
	 * @return the inequality of the two segments.
	 */
	bool operator!=( const TimingSegment &other ) const { return !this->operator==(other); };
	/**
	 * @brief Compares two DrivedSegments to see if one is less than or equal to the other.
	 * @param other the other DrivedSegments to compare to.
	 * @return the truth/falsehood of if the first is less or equal to than the second.
	 */
	bool operator<=( const TimingSegment &other ) const { return !this->operator>(other); };
	/**
	 * @brief Compares two DrivedSegments to see if one is greater than the other.
	 * @param other the other DrivedSegments to compare to.
	 * @return the truth/falsehood of if the first is greater than the second.
	 */
	bool operator>( const TimingSegment &other ) const
	{
		return other.operator<(*static_cast<const TimingSegment *>(this));
	};
	/**
	 * @brief Compares two DrivedSegments to see if one is greater than or equal to the other.
	 * @param other the other DrivedSegments to compare to.
	 * @return the truth/falsehood of if the first is greater than or equal to the second.
	 */
	bool operator>=( const TimingSegment &other ) const { return !this->operator<(other); };

private:
	/** @brief The row in which this segment activates. */
	int startingRow;
	
	/** @brief The specific type of segment this is. */
	TimingSegmentType segType;
	
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
	/**
	 * @brief Create a simple Fake Segment with default values.
	 *
	 * It is best to override the values as soon as possible.
	 */
	FakeSegment():
		TimingSegment(-1), lengthBeats(-1) {};
	
	/**
	 * @brief Create a copy of another Fake Segment.
	 * @param other the other fake segment
	 */
	FakeSegment(const FakeSegment &other):
		TimingSegment(other.GetRow()),
		lengthBeats(other.GetLength()) {};

	/**
	 * @brief Create a Fake Segment with the specified values.
	 * @param s the starting row of this segment.
	 * @param r the number of rows this segment lasts.
	 */
	template <typename StartType, typename LengthType>
	FakeSegment( StartType s, LengthType r ):
		TimingSegment(max((StartType)0, s)), 
		lengthBeats(ToBeat(max((LengthType)0, r))) {};
	
	/**
	 * @brief Get the length in beats of the FakeSegment.
	 * @return the length in beats. */
	float GetLength() const;
	
	/**
	 * @brief Set the length in beats of the FakeSegment.
	 * @param b the length in beats. */
	void SetLength(const float b);
	
	void Scale( int start, int length, int newLength );
	
	/**
	 * @brief Compares two FakeSegments to see if one is less than the other.
	 * @param other the other FakeSegment to compare to.
	 * @return the truth/falsehood of if the first is less than the second.
	 */
	bool operator<( const FakeSegment &other ) const;
	
	TimingSegmentType GetType() const { return SEGMENT_FAKE; }
	
private:
	/**
	 * @brief The number of beats the FakeSegment is alive for.
	 */
	float lengthBeats;
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
	/**
	 * @brief Create a simple Warp Segment with default values.
	 *
	 * It is best to override the values as soon as possible.
	 */
	WarpSegment():
		TimingSegment(-1), lengthBeats(-1) {};
	
	/**
	 * @brief Create a copy of another Warp Segment.
	 * @param other the other warp segment
	 */
	WarpSegment(const WarpSegment &other):
		TimingSegment(other.GetRow()),
		lengthBeats(other.GetLength()) {};

	/**
	 * @brief Create a Warp Segment with the specified values.
	 * @param s the starting row of this segment.
	 * @param r the number of rows this segment lasts.
	 */
	template <typename StartType, typename LengthType>
	WarpSegment( StartType s, LengthType r ):
		TimingSegment(s),
		lengthBeats(ToBeat(max((LengthType)0, r))) {};

	/**
	 * @brief Get the length in beats of the WarpSegment.
	 * @return the length in beats. */
	float GetLength() const;
	
	/**
	 * @brief Set the length in beats of the WarpSegment.
	 * @param b the length in beats. */
	void SetLength(const float b);
	
	void Scale( int start, int length, int newLength );
	
	/*
	 * @brief Compares two WarpSegments to see if one is less than the other.
	 * @param other the other WarpSegment to compare to.
	 * @return the truth/falsehood of if the first is less than the second.
	 */
	bool operator<( const WarpSegment &other ) const;
	
	TimingSegmentType GetType() const { return SEGMENT_WARP; }
private:
	/**
	 * @brief The number of beats the FakeSegment is alive for.
	 */
	float lengthBeats;
};

/**
 * @brief Identifies when a chart is to have a different tickcount value 
 * for hold notes.
 * 
 * A tickcount segment is used to better replicate the checkpoint hold
 * system used by various based video games. The number is used to 
 * represent how many ticks can be counted in one beat.
 */
struct TickcountSegment : public TimingSegment
{
	/**
	 * @brief Creates a simple Tickcount Segment with default values.
	 *
	 * It is best to override the values as soon as possible.
	 */
	TickcountSegment():
		TimingSegment(-1), ticks(4) {};
	
	/**
	 * @brief Create a copy of another Tickcount Segment.
	 * @param other the other tickcount segment
	 */
	TickcountSegment(const TickcountSegment &other):
		TimingSegment(other.GetRow()),
		ticks(other.GetTicks()) {};

	/**
	 * @brief Creates a TickcountSegment with specified values.
	 * @param s the starting row / beat. */
	template <typename StartType>
	TickcountSegment( StartType s ):
		TimingSegment(max((StartType)0, s)), ticks(4) {};
	
	/**
	 * @brief Creates a TickcountSegment with specified values.
	 * @param s the starting row / beat.
	 * @param t the amount of ticks counted per beat. */
	template <typename StartType>
	TickcountSegment( StartType s, int t ):
		TimingSegment(max((StartType)0, s)), ticks(max(0, t)) {};
	
	/**
	 * @brief Get the number of ticks in this TickcountSegment.
	 * @return the tickcount. */
	int GetTicks() const;
	
	/**
	 * @brief Set the number of ticks in this TickcountSegment.
	 * @param i the tickcount. */
	void SetTicks(const int i);
	
	/**
	 * @brief Compares two TickcountSegments to see if one is less than the other.
	 * @param other the other TickcountSegment to compare to.
	 * @return the truth/falsehood of if the first is less than the second.
	 */
	bool operator<( const TickcountSegment &other ) const;
	
	TimingSegmentType GetType() const { return SEGMENT_TICKCOUNT; }
	
private:
	/**
	 * @brief The amount of ticks counted per beat.
	 */
	int ticks;
};

/**
 * @brief Identifies when a chart is to have a different combo multiplier value.
 * 
 * Admitedly, this would primarily be used for mission mode style charts. However,
 * it can have its place during normal gameplay.
 */
struct ComboSegment : public TimingSegment
{
	/**
	 * @brief Creates a simple Combo Segment with default values.
	 *
	 * It is best to override the values as soon as possible.
	 */
	ComboSegment() : 
		TimingSegment(-1), combo(1), missCombo(1) { }

	ComboSegment(const ComboSegment &other) : 
		TimingSegment(other.GetRow()),
		combo(other.GetCombo()),
		missCombo(other.GetMissCombo()) {};

	/**
	 * @brief Creates a Combo Segment with the specified values.
	 * @param s the starting row / beat of this segment.
	 * @param t the amount the combo increases on a succesful hit.
	 */
	template <typename StartType>
	ComboSegment( StartType s, int t ):
		TimingSegment(max((StartType)0, s)),
		combo(max(0,t)), missCombo(max(0,t)) {}
	
	/**
	 * @brief Creates a Combo Segment with the specified values.
	 * @param s the starting row / beat of this segment.
	 * @param t the amount the combo increases on a succesful hit.
	 * @param m the amount the miss combo increases on missing.
	 */
	template <typename StartType>
	ComboSegment(StartType s, int t, int m):
		TimingSegment(max((StartType)0, s)),
		combo(max(0,t)), missCombo(max(0,m)) {}
	
	/**
	 * @brief Get the combo in this ComboSegment.
	 * @return the combo. */
	int GetCombo() const;
	
	/**
	 * @brief Get the miss combo in this ComboSegment.
	 * @return the miss combo. */
	int GetMissCombo() const;
	
	/**
	 * @brief Set the combo in this ComboSegment.
	 * @param i the combo. */
	void SetCombo(const int i);
	
	/**
	 * @brief Set the miss combo in this ComboSegment.
	 * @param i the miss combo. */
	void SetMissCombo(const int i);
	
	/**
	 * @brief Compares two ComboSegments to see if one is less than the other.
	 * @param other the other ComboSegment to compare to.
	 * @return the truth/falsehood of if the first is less than the second.
	 */
	bool operator<( const ComboSegment &other ) const;
	
	TimingSegmentType GetType() const { return SEGMENT_COMBO; }
private:
	/**
	 * @brief The amount the combo increases at this point.
	 */
	int combo;
	/** @brief The amount of miss combos given at this point. */
	int missCombo;
};


/**
 * @brief Identifies when a chart is entering a different section.
 * 
 * This is meant for helping to identify different sections of a chart
 * versus relying on measures and beats alone.
 */
struct LabelSegment : public TimingSegment
{
	/**
	 * @brief Creates a simple Label Segment with default values.
	 *
	 * It is best to override the values as soon as possible.
	 */
	LabelSegment() : 
		TimingSegment(-1), label("") { }

	LabelSegment(const LabelSegment &other) :
		TimingSegment(other.GetRow()),
		label(other.GetLabel()) {};

	/**
	 * @brief Creates a Label Segment with the specified values.
	 * @param s the starting row / beat of this segment.
	 * @param l the label for this section.
	 */
	template <typename StartType>
	LabelSegment( StartType s, RString l ):
		TimingSegment(max((StartType)0, s)),
		label(l) {}

	/**
	 * @brief Get the label in this LabelSegment.
	 * @return the label. */
	RString GetLabel() const;
	
	/**
	 * @brief Set the label in this LabelSegment.
	 * @param l the label. */
	void SetLabel(const RString l);
	
	/**
	 * @brief Compares two LabelSegments to see if one is less than the other.
	 * @param other the other LabelSegment to compare to.
	 * @return the truth/falsehood of if the first is less than the second.
	 */
	bool operator<( const LabelSegment &other ) const;
	
	TimingSegmentType GetType() const { return SEGMENT_LABEL; }

private:
	/**
	 * @brief The label/section name for this point.
	 */
	RString label;
};



/**
 * @brief Identifies when a song changes its BPM.
 */
struct BPMSegment : public TimingSegment
{
	/**
	 * @brief Creates a simple BPM Segment with default values.
	 *
	 * It is best to override the values as soon as possible.
	 */
	BPMSegment() :
		TimingSegment(-1), bps(-1.0f) { }
	
	BPMSegment(const BPMSegment &other) :
		TimingSegment(other.GetRow()),
		bps(other.GetBPS()) {};

	/**
	 * @brief Creates a BPM Segment with the specified starting row and beats per second.
	 * @param s the starting row / beat of this segment.
	 * @param b the beats per minute to be turned into beats per second.
	 */
	template <typename StartType>
	BPMSegment( StartType s, float bpm ):
		TimingSegment(max((StartType)0, s)), bps(0.0f) { SetBPM(bpm); }

	/**
	 * @brief Get the label in this LabelSegment.
	 * @return the label. */
	float GetBPM() const;
	
	/**
	 * @brief Set the label in this LabelSegment.
	 * @param l the label. */
	void SetBPM(const float bpm);
	
	/**
	 * @brief Get the label in this LabelSegment.
	 * @return the label. */
	float GetBPS() const;
	
	/**
	 * @brief Set the label in this LabelSegment.
	 * @param l the label. */
	void SetBPS(const float newBPS);
	
	/**
	 * @brief Compares two LabelSegments to see if one is less than the other.
	 * @param other the other LabelSegment to compare to.
	 * @return the truth/falsehood of if the first is less than the second.
	 */
	bool operator<( const BPMSegment &other ) const;
	
	TimingSegmentType GetType() const { return SEGMENT_BPM; }

private:
	/**
	 * @brief The label/section name for this point.
	 */
	float bps;
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
	/**
	 * @brief Creates a simple Time Signature Segment with default values.
	 */
	TimeSignatureSegment():
		TimingSegment(-1),
		numerator(4), denominator(4)  { }
		
	TimeSignatureSegment(const TimeSignatureSegment &other) :
		TimingSegment(other.GetRow()),
		numerator(other.GetNum()),
		denominator(other.GetDen()) {};
	/**
	 * @brief Creates a Time Signature Segment with supplied values.
	 *
	 * The denominator will be 4 if this is called.
	 * @param s the starting row / beat of the segment.
	 * @param n the numerator for the segment.
	 */
	template <typename StartType>
	TimeSignatureSegment( StartType s, int n ):
		TimingSegment(max((StartType)0, s)),
		numerator(max(1, n)), denominator(4) {}
	/**
	 * @brief Creates a Time Signature Segment with supplied values.
	 * @param s the starting row of the segment.
	 * @param n the numerator for the segment.
	 * @param d the denonimator for the segment.
	 */
	template <typename StartType>
	TimeSignatureSegment( StartType s, int n, int d ):
		TimingSegment(max((StartType)0, s)),
		numerator(max(1, n)), denominator(max(1, d)) {}
	
	/**
	 * @brief Get the numerator in this TimeSignatureSegment.
	 * @return the numerator. */
	int GetNum() const;
	
	/**
	 * @brief Set the numerator in this TimeSignatureSegment.
	 * @param i the numerator. */
	void SetNum(const int i);
	
	/**
	 * @brief Get the denominator in this TimeSignatureSegment.
	 * @return the denominator. */
	int GetDen() const;
	
	/**
	 * @brief Set the denominator in this TimeSignatureSegment.
	 * @param i the denominator. */
	void SetDen(const int i);
	
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
	int GetNoteRowsPerMeasure() const;

	/**
	 * @brief Compares two TimeSignatureSegments to see if one is less than the other.
	 * @param other the other TimeSignatureSegment to compare to.
	 * @return the truth/falsehood of if the first is less than the second.
	 */
	bool operator<( const TimeSignatureSegment &other ) const;
	
	TimingSegmentType GetType() const { return SEGMENT_TIME_SIG; }
private:
	/**
	 * @brief The numerator of the TimeSignatureSegment.
	 */
	int numerator;
	/**
	 * @brief The denominator of the TimeSignatureSegment.
	 */
	int denominator;
};

/**
 * @brief Identifies when the arrow scroll changes.
 *
 * SpeedSegments take a Player's scrolling BPM (Step's BPM * speed mod),
 * and then multiplies it with the percentage value. No matter the player's
 * speed mod, the ratio will be the same. Unlike forced attacks, these
 * cannot be turned off at a set time: reset it by setting the precentage
 * back to 1.
 *
 * These were inspired by the Pump It Up series. */
struct SpeedSegment : public TimingSegment
{
	/** @brief Sets up the SpeedSegment with default values. */
	SpeedSegment():
		TimingSegment(0), 
	ratio(1), length(0), unit(0) {}
	
	SpeedSegment(const SpeedSegment &other) :
		TimingSegment(other.GetRow()),
		ratio(other.GetRatio()),
		length(other.GetLength()),
		unit(other.GetUnit()) {};

	/**
	 * @brief Sets up the SpeedSegment with specified values.
	 * @param s The row / beat this activates.
	 * @param p The percentage to use. */
	template <typename StartType>
	SpeedSegment( StartType s, float p): 
		TimingSegment(max((StartType)0, s)), 
	ratio(p), length(0), unit(0) {}
	
	/**
	 * @brief Sets up the SpeedSegment with specified values.
	 * @param s The row / beat this activates.
	 * @param p The percentage to use.
	 * @param w The number of beats to wait. */
	template <typename StartType>
	SpeedSegment(StartType s, float p, float w):
		TimingSegment(max((StartType)0, s)),
	ratio(p), length(w), unit(0) {}

	
	/**
	 * @brief Sets up the SpeedSegment with specified values.
	 * @param s The row / beat this activates.
	 * @param p The percentage to use.
	 * @param w The number of beats/seconds to wait.
	 * @param k The mode used for the wait variable. */
	template <typename StartType>
	SpeedSegment(StartType s, float p, float w, unsigned short k): 
		TimingSegment(max((StartType)0, s)),
		ratio(p), length(w), unit(k) {}
	
	/**
	 * @brief Get the ratio in this SpeedSegment.
	 * @return the ratio. */
	float GetRatio() const;
	
	/**
	 * @brief Set the ratio in this SpeedSegment.
	 * @param i the ratio. */
	void SetRatio(const float i);
	
	/**
	 * @brief Get the length in this SpeedSegment.
	 * @return the length. */
	float GetLength() const;
	
	/**
	 * @brief Set the length in this SpeedSegment.
	 * @param i the length. */
	void SetLength(const float i);
	
	/**
	 * @brief Get the unit in this SpeedSegment.
	 * @return the unit. */
	unsigned short GetUnit() const;
	
	/**
	 * @brief Set the unit in this SpeedSegment.
	 * @param i the unit. */
	void SetUnit(const unsigned short i);
	
	/**
	 * @brief Set the unit in this SpeedSegment.
	 *
	 * This one is offered for quicker compatibility.
	 * @param i the unit. */
	void SetUnit(const int i);
	
	void Scale( int start, int length, int newLength );

	/**
	 * @brief Compares two SpeedSegments to see if one is less than the other.
	 * @param other the other SpeedSegment to compare to.
	 * @return the truth/falsehood of if the first is less than the second.
	 */
	bool operator<( const SpeedSegment &other ) const;
	
	TimingSegmentType GetType() const { return SEGMENT_SPEED; }
private:
	/** @brief The percentage (ratio) to use when multiplying the Player's BPM. */
	float ratio;
	/** 
	 * @brief The number of beats or seconds to wait for the change to take place.
	 *
	 * A value of 0 means this is immediate. */
	float length;
	/**
	 * @brief The mode that this segment uses for the math.
	 *
	 * 0: beats
	 * 1: seconds
	 * other values are undetermined at this time, but we're prepared this way.
	 */
	unsigned short unit;
	
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
	/** @brief Sets up the ScrollSegment with default values. */
	ScrollSegment(): TimingSegment(0),
		ratio(1) {}
	
	/**
	 * @brief Sets up the ScrollSegment with specified values.
	 * @param s The row / beat this activates.
	 * @param p The percentage to use. */
	template <typename StartType>
	ScrollSegment( StartType s, float p): 
		TimingSegment(max((StartType)0, s)),
		ratio(p) {}
	
	ScrollSegment(const ScrollSegment &other) :
		TimingSegment(other.GetRow()),
		ratio(other.GetRatio()) {}
	
	/**
	 * @brief Get the ratio in this ScrollSegment.
	 * @return the ratio. */
	float GetRatio() const;
	
	/**
	 * @brief Set the ratio in this ScrollSegment.
	 * @param i the ratio. */
	void SetRatio(const float i);
	
	/**
	 * @brief Compares two ScrollSegment to see if one is less than the other.
	 * @param other the other ScrollSegment to compare to.
	 * @return the truth/falsehood of if the first is less than the second.
	 */
	bool operator<( const ScrollSegment &other ) const;
	
	TimingSegmentType GetType() const { return SEGMENT_SCROLL; }
private:
	/** @brief The ratio / percentage to use when multiplying the chart's scroll rate. */
	float ratio;
};

/**
 * @brief Identifies when a song has a stop or a delay.
 *
 * It is hopeful that stops and delays can be made into their own segments at some point.
 */
struct StopSegment : public TimingSegment
{
	/**
	 * @brief Creates a simple Stop Segment with default values.
	 *
	 * It is best to override the values as soon as possible.
	 */
	StopSegment() : TimingSegment(-1),
		pauseSeconds(-1.0f), isDelay(false) {}
	
	StopSegment (const StopSegment &other):
		TimingSegment(other.GetRow()),
		pauseSeconds(other.GetPause()),
		isDelay(other.GetDelay()) {}
	
	/**
	 * @brief Creates a Stop Segment with specified values.
	 *
	 * This will not create a dedicated delay segment.
	 * Use the third constructor for making delays.
	 * @param s the starting row / beat of this segment.
	 * @param f the length of time to pause the note scrolling.
	 */
	template <typename StartType>
	StopSegment( StartType s, float f ):
		TimingSegment(max((StartType)0, s)),
		pauseSeconds(f), isDelay(false) {}
	/**
	 * @brief Creates a Stop/Delay Segment with specified values.
	 * @param s the starting row / beat of this segment.
	 * @param f the length of time to pause the note scrolling.
	 * @param d the flag that makes this Stop Segment a Delay Segment.
	 */
	template <typename StartType>
	StopSegment( StartType s, float f, bool d ):
		TimingSegment(max((StartType)0, s)),
		pauseSeconds(f), isDelay(d) {}

	/**
	 * @brief Get the pause length in this StopSegment.
	 * @return the pause length. */
	float GetPause() const;
	
	/**
	 * @brief Set the pause length in this StopSegment.
	 * @param i the pause length. */
	void SetPause(const float i);
	
	/**
	 * @brief Get the behavior in this StopSegment.
	 * @return the behavior. */
	bool GetDelay() const;
	
	/**
	 * @brief Set the behavior in this StopSegment.
	 * @param i the behavior. */
	void SetDelay(const bool i);

	/**
	 * @brief Compares two StopSegments to see if one is less than the other.
	 *
	 * It should be observed that Delay Segments have to come before Stop Segments.
	 * Otherwise, it will act like a Stop Segment with extra time from the Delay at
	 * the same row.
	 * @param other the other StopSegment to compare to.
	 * @return the truth/falsehood of if the first is less than the second.
	 */
	bool operator<( const StopSegment &other ) const;
	
	TimingSegmentType GetType() const { return SEGMENT_STOP_DELAY; }
private:
	/**
	 * @brief The amount of time to complete the pause at the given row.
	 */
	float pauseSeconds;
	/**
	 * @brief How does this StopSegment behave?
	 *
	 * If true, the Stop Segment is treated as a Delay Segment, similar to the Pump It Up series.
	 * If false, this behaves similar to the DDR/ITG style games.
	 *
	 * TODO: Separate out DelaySegments in the future.
	 */
	bool isDelay;
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
