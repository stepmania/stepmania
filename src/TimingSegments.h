#ifndef TIMING_SEGMENTS_H
#define TIMING_SEGMENTS_H

#include "NoteTypes.h" // Converting rows to beats and vice~versa.

/** 
 * @brief Compare a TimingData segment's properties with one another.
 *
 * This will be removed once we start respecting public & private data.*/
#define COMPARE(x) if(x!=other.x) return false;

/**
 * @brief The general TimingSegment for all of the changing glory.
 *
 * Each segment is supposed to derive from this one. */
struct TimingSegment
{
	/** @brief Set up a TimingSegment with default values. */
	TimingSegment();
	/**
	 * @brief Set up a TimingSegment with specified values.
	 * @param s the starting row. */
	TimingSegment(int s);
	/**
	 * @brief Set up a TimingSegment with specified values.
	 * @param s the starting beat. */
	TimingSegment(float s);
	
	virtual ~TimingSegment();
	
	/**
	 * @brief Set the starting row of the TimingSegment.
	 *
	 * This is virtual to allow other segments to implement validation
	 * as required by them.
	 * @param s the supplied row. */
	virtual void SetRow( const int s );
	/**
	 * @brief Set the starting beat of the TimingSegment.
	 *
	 * This is virtual to allow other segments to implement validation
	 * as required by them.
	 * @param s the supplied beat. */
	virtual void SetBeat( const float s );
	/**
	 * @brief Get the starting row of the TimingSegment.
	 * @return the starting row. */
	int GetRow() const;
	/**
	 * @brief Get the starting beat of the TimingSegment.
	 * @return the starting beat. */
	float GetBeat() const;
	
private:
	/** @brief The row in which this segment activates. */
	int startingRow;
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
	FakeSegment();
	/**
	 * @brief Create a Fake Segment with the specified values.
	 * @param s the starting row of this segment.
	 * @param r the number of rows this segment lasts.
	 */
	FakeSegment( int s, int r );
	/**
	 * @brief Creates a Fake Segment with the specified values.
	 * @param s the starting row of this segment.
	 * @param b the number of beats this segment lasts.
	 */
	FakeSegment( int s, float b );
	/**
	 * @brief Create a Fake Segment with the specified values.
	 * @param s the starting beat in this segment.
	 * @param r the number of rows this segment lasts.
	 */
	FakeSegment( float s, int r );
	/**
	 * @brief Creates a Fake Segment with the specified values.
	 * @param s the starting beat of this segment.
	 * @param b the number of beats this segment lasts.
	 */
	FakeSegment( float s, float b );
	
	/**
	 * @brief Get the length in beats of the FakeSegment.
	 * @return the length in beats. */
	float GetLength() const;
	
	/**
	 * @brief Set the length in beats of the FakeSegment.
	 * @param b the length in beats. */
	void SetLength(const float b);
	
	/**
	 * @brief Compares two FakeSegments to see if they are equal to each other.
	 * @param other the other FakeSegment to compare to.
	 * @return the equality of the two segments.
	 */
	bool operator==( const FakeSegment &other ) const;
	/**
	 * @brief Compares two FakeSegments to see if they are not equal to each other.
	 * @param other the other FakeSegment to compare to.
	 * @return the inequality of the two segments.
	 */
	bool operator!=( const FakeSegment &other ) const;
	/**
	 * @brief Compares two FakeSegments to see if one is less than the other.
	 * @param other the other FakeSegment to compare to.
	 * @return the truth/falsehood of if the first is less than the second.
	 */
	bool operator<( const FakeSegment &other ) const;
	/**
	 * @brief Compares two FakeSegments to see if one is less than or equal to the other.
	 * @param other the other FakeSegment to compare to.
	 * @return the truth/falsehood of if the first is less or equal to than the second.
	 */
	bool operator<=( const FakeSegment &other ) const;
	/**
	 * @brief Compares two FakeSegments to see if one is greater than the other.
	 * @param other the other FakeSegment to compare to.
	 * @return the truth/falsehood of if the first is greater than the second.
	 */
	bool operator>( const FakeSegment &other ) const;
	/**
	 * @brief Compares two FakeSegments to see if one is greater than or equal to the other.
	 * @param other the other FakeSegment to compare to.
	 * @return the truth/falsehood of if the first is greater than or equal to the second.
	 */
	bool operator>=( const FakeSegment &other ) const;
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
	TickcountSegment();
	/**
	 * @brief Creates a TickcountSegment with specified values.
	 * @param s the starting row. */
	TickcountSegment(int s);
	/**
	 * @brief Creates a TickcountSegment with specified values.
	 * @param s the starting beat. */
	TickcountSegment(float s);
	/**
	 * @brief Creates a Tickcount Segment with the specified values.
	 * @param s the starting row.
	 * @param t the amount of ticks counted per beat.
	 */
	TickcountSegment( int s, int t );
	/**
	 * @brief Creates a Tickcount Segment with the specified values.
	 * @param s the starting beat.
	 * @param t the amount of ticks counted per beat.
	 */
	TickcountSegment( float s, int t );
	
	/**
	 * @brief Get the number of ticks in this TickcountSegment.
	 * @return the tickcount. */
	int GetTicks() const;
	
	/**
	 * @brief Set the number of ticks in this TickcountSegment.
	 * @param i the tickcount. */
	void SetTicks(const int i);
	
	/**
	 * @brief Compares two TickcountSegments to see if they are equal to each other.
	 * @param other the other TickcountSegment to compare to.
	 * @return the equality of the two segments.
	 */
	bool operator==( const TickcountSegment &other ) const;
	/**
	 * @brief Compares two TickcountSegments to see if they are not equal to each other.
	 * @param other the other TickcountSegment to compare to.
	 * @return the inequality of the two segments.
	 */
	bool operator!=( const TickcountSegment &other ) const;
	/**
	 * @brief Compares two TickcountSegments to see if one is less than the other.
	 * @param other the other TickcountSegment to compare to.
	 * @return the truth/falsehood of if the first is less than the second.
	 */
	bool operator<( const TickcountSegment &other ) const;
	/**
	 * @brief Compares two TickcountSegments to see if one is less than or equal to the other.
	 * @param other the other TickcountSegment to compare to.
	 * @return the truth/falsehood of if the first is less or equal to than the second.
	 */
	bool operator<=( const TickcountSegment &other ) const;
	/**
	 * @brief Compares two TickcountSegments to see if one is greater than the other.
	 * @param other the other TickcountSegment to compare to.
	 * @return the truth/falsehood of if the first is greater than the second.
	 */
	bool operator>( const TickcountSegment &other ) const;
	/**
	 * @brief Compares two TickcountSegments to see if one is greater than or equal to the other.
	 * @param other the other TickcountSegment to compare to.
	 * @return the truth/falsehood of if the first is greater than or equal to the second.
	 */
	bool operator>=( const TickcountSegment &other ) const;
	
private:
	/**
	 * @brief The amount of ticks counted per beat.
	 */
	int ticks;
};



#undef COMPARE

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
