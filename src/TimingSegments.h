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
