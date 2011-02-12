/** @brief TimingData - Holds data for translating beats<->seconds. */

#ifndef TIMING_DATA_H
#define TIMING_DATA_H

#include "NoteTypes.h"
#include "PrefsManager.h"
struct lua_State;

/** @brief Compare a TimingData segment's properties with one another. */
#define COMPARE(x) if(x!=other.x) return false;

/**
 * @brief Identifies when a song changes its BPM.
 */
struct BPMSegment 
{
	/**
	 * @brief Creates a simple BPM Segment with default values.
	 *
	 * It is best to override the values as soon as possible.
	 */
	BPMSegment() : m_iStartRow(-1), m_fBPS(-1.0f) { }
	/**
	 * @brief Creates a BPM Segment with the specified starting row and beats per second.
	 * @param s the starting row of this segment.
	 * @param b the beats per second to be turned into beats per minute.
	 */
	BPMSegment( int s, float b ) { m_iStartRow = max( 0, s ); SetBPM( b ); }
	/**
	 * @brief The row in which the BPMSegment activates.
	 */
	int m_iStartRow;
	/**
	 * @brief The BPS to use when this row is reached.
	 */
	float m_fBPS;
	
	/**
	 * @brief Converts the BPS to a BPM.
	 * @param f The BPM.
	 */
	void SetBPM( float f ) { m_fBPS = f / 60.0f; }
	/**
	 * @brief Retrieves the BPM from the BPS.
	 * @return the BPM.
	 */
	float GetBPM() const { return m_fBPS * 60.0f; }

	/**
	 * @brief Compares two BPMSegments to see if they are equal to each other.
	 * @param other the other BPMSegment to compare to.
	 * @return the equality of the two segments.
	 */
	bool operator==( const BPMSegment &other ) const
	{
		COMPARE( m_iStartRow );
		COMPARE( m_fBPS );
		return true;
	}
	/**
	 * @brief Compares two BPMSegments to see if they are not equal to each other.
	 * @param other the other BPMSegment to compare to.
	 * @return the inequality of the two segments.
	 */
	bool operator!=( const BPMSegment &other ) const { return !operator==(other); }
	/**
	 * @brief Compares two BPMSegments to see if one is less than the other.
	 * @param other the other BPMSegment to compare to.
	 * @return the truth/falsehood of if the first is less than the second.
	 */
	bool operator<( const BPMSegment &other ) const
	{
		return m_iStartRow < other.m_iStartRow || 
		( m_iStartRow == other.m_iStartRow && m_fBPS < other.m_fBPS );
	}
	/**
	 * @brief Compares two BPMSegments to see if one is less than or equal to the other.
	 * @param other the other BPMSegment to compare to.
	 * @return the truth/falsehood of if the first is less or equal to than the second.
	 */
	bool operator<=( const BPMSegment &other ) const
	{
		return ( operator<(other) || operator==(other) );
	}
	/**
	 * @brief Compares two BPMSegments to see if one is greater than the other.
	 * @param other the other BPMSegment to compare to.
	 * @return the truth/falsehood of if the first is greater than the second.
	 */
	bool operator>( const BPMSegment &other ) const { return !operator<=(other); }
	/**
	 * @brief Compares two BPMSegments to see if one is greater than or equal to the other.
	 * @param other the other BPMSegment to compare to.
	 * @return the truth/falsehood of if the first is greater than or equal to the second.
	 */
	bool operator>=( const BPMSegment &other ) const { return !operator<(other); }
};
/**
 * @brief Identifies when a song has a stop or a delay.
 *
 * It is hopeful that stops and delays can be made into their own segments at some point.
 */
struct StopSegment 
{
	/**
	 * @brief Creates a simple Stop Segment with default values.
	 *
	 * It is best to override the values as soon as possible.
	 */
	StopSegment() : m_iStartRow(-1), m_fStopSeconds(-1.0f), m_bDelay(false)  { }
	/**
	 * @brief Creates a Stop Segment at the specified row for the specified length of time.
	 *
	 * This will not create a dedicated delay segment. Use the third constructor for
	 * making delays.
	 * @param s the starting row of this segment.
	 * @param f the length of time to pause the note scrolling.
	 */
	StopSegment( int s, float f ) {
		m_iStartRow = max( 0, s );
		m_fStopSeconds = PREFSMAN->m_bQuirksMode ? f : max( 0.0f, f );
		m_bDelay = false;
	}
	/**
	 * @brief Creates a Stop (or Delay) Segment at the specified row for the specified length of time.
	 * @param s the starting row of this segment.
	 * @param f the length of time to pause the note scrolling.
	 * @param d the flag that makes this Stop Segment a Delay Segment.
	 */
	StopSegment( int s, float f, bool d ) {
		m_iStartRow = max( 0, s );
		m_fStopSeconds = PREFSMAN->m_bQuirksMode ? f : max( 0.0f, f );
		m_bDelay = d;
	}
	/**
	 * @brief The row in which the StopSegment activates.
	 */
	int m_iStartRow;
	/**
	 * @brief The amount of time to complete the pause at the given row.
	 */
	float m_fStopSeconds;
	/**
	 * @brief If true, the Stop Segment is treated as a Delay Segment, similar to the Pump It Up series.
	 */
	bool m_bDelay;
	/**
	 * @brief Compares two StopSegments to see if they are equal to each other.
	 * @param other the other StopSegment to compare to.
	 * @return the equality of the two segments.
	 */
	bool operator==( const StopSegment &other ) const
	{
		COMPARE( m_iStartRow );
		COMPARE( m_fStopSeconds );
		COMPARE( m_bDelay );
		return true;
	}
	/**
	 * @brief Compares two StopSegments to see if they are not equal to each other.
	 * @param other the other StopSegment to compare to.
	 * @return the inequality of the two segments.
	 */
	bool operator!=( const StopSegment &other ) const { return !operator==(other); }
	/**
	 * @brief Compares two StopSegments to see if one is less than the other.
	 *
	 * It should be observed that Delay Segments have to come before Stop Segments.
	 * Otherwise, it will act like a Stop Segment with extra time from the Delay at
	 * the same row.
	 * @param other the other StopSegment to compare to.
	 * @return the truth/falsehood of if the first is less than the second.
	 */
	bool operator<( const StopSegment &other ) const
	{
		return ( m_iStartRow < other.m_iStartRow ) ||
		( m_iStartRow == other.m_iStartRow && 
		 ( m_bDelay && !other.m_bDelay || m_fStopSeconds < other.m_fStopSeconds ));
	}
	/**
	 * @brief Compares two StopSegments to see if one is less than or equal to the other.
	 * @param other the other StopSegment to compare to.
	 * @return the truth/falsehood of if the first is less or equal to than the second.
	 */
	bool operator<=( const StopSegment &other ) const
	{
		return ( operator<(other) || operator==(other) );
	}
	/**
	 * @brief Compares two StopSegments to see if one is greater than the other.
	 *
	 * Similar to the less than operator function, stops must come after delays
	 * to avoid rendering the point of delays pointless.
	 * @param other the other StopSegment to compare to.
	 * @return the truth/falsehood of if the first is greater than the second.
	 */
	bool operator>( const StopSegment &other ) const { return !operator<=(other); }
	/**
	 * @brief Compares two StopSegments to see if one is greater than or equal to the other.
	 * @param other the other StopSegment to compare to.
	 * @return the truth/falsehood of if the first is greater than or equal to the second.
	 */
	bool operator>=( const StopSegment &other ) const { return !operator<(other); }
};

/**
 * @brief Identifies when a song changes its time signature.
 *
 * This only supports simple time signatures. The upper number (called the numerator here, though this isn't
 * properly a fraction) is the number of beats per measure. The lower number (denominator here)
 * is the note value representing one beat. */
struct TimeSignatureSegment 
{
	/**
	 * @brief Creates a simple Time Signature Segment with default values.
	 */
	TimeSignatureSegment() : m_iStartRow(-1), m_iNumerator(4), m_iDenominator(4)  { }
	/**
	 * @brief Creates a Time Signature Segment at the given row with a supplied numerator.
	 * @param r the starting row of the segment.
	 * @param n the numerator for the segment.
	 */
	TimeSignatureSegment( int r, int n ) {
		m_iStartRow = max( 0, r );
		m_iNumerator = n;
		m_iDenominator = 4;
	}
	/**
	 * @brief Creates a Time Signature Segment at the given row with a supplied numerator & denominator.
	 * @param r the starting row of the segment.
	 * @param n the numerator for the segment.
	 * @param d the denonimator for the segment.
	 */
	TimeSignatureSegment( int r, int n, int d ) {
		m_iStartRow = max( 0, r );
		m_iNumerator = n;
		m_iDenominator = d;
	}
	/**
	 * @brief The row in which the TimeSignatureSegment activates.
	 */
	int m_iStartRow;
	/**
	 * @brief The numerator of the TimeSignatureSegment.
	 */
	int m_iNumerator;
	/**
	 * @brief The denominator of the TimeSignatureSegment.
	 */
	int m_iDenominator;

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
	int GetNoteRowsPerMeasure() const { return BeatToNoteRow(1) * 4 * m_iNumerator / m_iDenominator; }
	/**
	 * @brief Compares two TimeSignatureSegments to see if they are equal to each other.
	 * @param other the other TimeSignatureSegment to compare to.
	 * @return the equality of the two segments.
	 */
	bool operator==( const TimeSignatureSegment &other ) const
	{
		COMPARE( m_iStartRow );
		COMPARE( m_iNumerator );
		COMPARE( m_iDenominator );
		return true;
	}
	/**
	 * @brief Compares two TimeSignatureSegments to see if they are not equal to each other.
	 * @param other the other TimeSignatureSegment to compare to.
	 * @return the inequality of the two segments.
	 */
	bool operator!=( const TimeSignatureSegment &other ) const { return !operator==(other); }
	/**
	 * @brief Compares two TimeSignatureSegments to see if one is less than the other.
	 * @param other the other TimeSignatureSegment to compare to.
	 * @return the truth/falsehood of if the first is less than the second.
	 */
	bool operator<( const TimeSignatureSegment &other ) const
	{ 
		return m_iStartRow < other.m_iStartRow ||
		( m_iStartRow == other.m_iStartRow && 
		 ( m_iNumerator < other.m_iNumerator || 
		  ( m_iNumerator == other.m_iNumerator && m_iDenominator < other.m_iDenominator )));
	}
	/**
	 * @brief Compares two TimeSignatureSegments to see if one is less than or equal to the other.
	 * @param other the other TimeSignatureSegment to compare to.
	 * @return the truth/falsehood of if the first is less or equal to than the second.
	 */
	bool operator<=( const TimeSignatureSegment &other ) const
	{
		return ( operator<(other) || operator==(other) );
	}
	/**
	 * @brief Compares two TimeSignatureSegments to see if one is greater than the other.
	 * @param other the other TimeSignatureSegment to compare to.
	 * @return the truth/falsehood of if the first is greater than the second.
	 */
	bool operator>( const TimeSignatureSegment &other ) const { return !operator<=(other); }
	/**
	 * @brief Compares two TimeSignatureSegments to see if one is greater than or equal to the other.
	 * @param other the other TimeSignatureSegment to compare to.
	 * @return the truth/falsehood of if the first is greater than or equal to the second.
	 */
	bool operator>= (const TimeSignatureSegment &other ) const { return !operator<(other); }
};

/**
 * @brief Identifies when a song needs to warp to a new beat.
 *
 * A warp segment is used to replicate the effects of Negative BPMs without
 * abusing negative BPMs. Negative BPMs should be converted to warp segments.
 * WarpAt=WarpTo is the format, where both are in beats. (Technically they're
 * both rows though.) */
struct WarpSegment
{
	/**
	 * @brief Creates a simple Warp Segment with default values.
	 *
	 * It is best to override the values as soon as possible.
	 */
	WarpSegment() : m_iStartRow(-1), m_fWarpBeats(-1) { }
	/**
	 * @brief Creates a Warp Segment with the specified starting row and row to warp to.
	 * @param s the starting row of this segment.
	 * @param r the row to warp to.
	 */
	WarpSegment( int s, int r )
	{
		m_iStartRow = max( 0, s );
		m_fWarpBeats = max( 0, NoteRowToBeat( r ) );
	}
	/**
	 * @brief Creates a Warp Segment with the specified starting row and beat to warp to.
	 * @param s the starting row of this segment.
	 * @param b the beat to warp to.
	 */
	WarpSegment( int s, float b ){ m_iStartRow = max( 0, s ); m_fWarpBeats = max( 0, b ); }
	/**
	 * @brief The row in which the WarpSegment activates.
	 */
	int m_iStartRow;
	/**
	 * @brief The beat to warp to.
	 */
	float m_fWarpBeats;
	/**
	 * @brief Compares two WarpSegments to see if they are equal to each other.
	 * @param other the other WarpSegment to compare to.
	 * @return the equality of the two segments.
	 */
	bool operator==( const WarpSegment &other ) const
	{
		COMPARE( m_iStartRow );
		COMPARE( m_fWarpBeats );
		return true;
	}
	/**
	 * @brief Compares two WarpSegments to see if they are not equal to each other.
	 * @param other the other WarpSegment to compare to.
	 * @return the inequality of the two segments.
	 */
	bool operator!=( const WarpSegment &other ) const { return !operator==(other); }
	/**
	 * @brief Compares two WarpSegments to see if one is less than the other.
	 * @param other the other WarpSegment to compare to.
	 * @return the truth/falsehood of if the first is less than the second.
	 */
	bool operator<( const WarpSegment &other ) const
	{ 
		return m_iStartRow < other.m_iStartRow ||
		( m_iStartRow == other.m_iStartRow && m_fWarpBeats < other.m_fWarpBeats );
	}
	/**
	 * @brief Compares two WarpSegments to see if one is less than or equal to the other.
	 * @param other the other WarpSegment to compare to.
	 * @return the truth/falsehood of if the first is less or equal to than the second.
	 */
	bool operator<=( const WarpSegment &other ) const
	{
		return ( operator<(other) || operator==(other) );
	}
	/**
	 * @brief Compares two WarpSegments to see if one is greater than the other.
	 * @param other the other WarpSegment to compare to.
	 * @return the truth/falsehood of if the first is greater than the second.
	 */
	bool operator>( const WarpSegment &other ) const { return !operator<=(other); }
	/**
	 * @brief Compares two WarpSegments to see if one is greater than or equal to the other.
	 * @param other the other WarpSegment to compare to.
	 * @return the truth/falsehood of if the first is greater than or equal to the second.
	 */
	bool operator>=( const WarpSegment &other ) const { return !operator<(other); }
};

/**
 * @brief Identifies when a chart is to have a different tickcount value for hold notes.
 * 
 * A tickcount segment is used to better replicate the checkpoint hold
 * system used by various based video games. The number is used to 
 * represent how many ticks can be counted in one beat.
 */
struct TickcountSegment
{
	/**
	 * @brief Creates a simple Tickcount Segment with default values.
	 *
	 * It is best to override the values as soon as possible.
	 */
	TickcountSegment() : m_iStartRow(-1), m_iTicks(2) { }
	/**
	 * @brief Creates a Tickcount Segment with the specified starting row and beats per second.
	 * @param s the starting row of this segment.
	 * @param t the amount of ticks counted per beat.
	 */
	TickcountSegment( int s, int t ){ m_iStartRow = max( 0, s ); m_iTicks = max( 1, t ); }
	/**
	 * @brief The row in which the TickcountSegment activates.
	 */
	int m_iStartRow;
	/**
	 * @brief The amount of ticks counted per beat.
	 */
	int m_iTicks;
	
	/**
	 * @brief Compares two TickcountSegments to see if they are equal to each other.
	 * @param other the other TickcountSegment to compare to.
	 * @return the equality of the two segments.
	 */
	bool operator==( const TickcountSegment &other ) const
	{
		COMPARE( m_iStartRow );
		COMPARE( m_iTicks );
		return true;
	}
	/**
	 * @brief Compares two TickcountSegments to see if they are not equal to each other.
	 * @param other the other TickcountSegment to compare to.
	 * @return the inequality of the two segments.
	 */
	bool operator!=( const TickcountSegment &other ) const { return !operator==(other); }
	/**
	 * @brief Compares two TickcountSegments to see if one is less than the other.
	 * @param other the other TickcountSegment to compare to.
	 * @return the truth/falsehood of if the first is less than the second.
	 */
	bool operator<( const TickcountSegment &other ) const { return m_iStartRow < other.m_iStartRow; }
	/**
	 * @brief Compares two TickcountSegments to see if one is less than or equal to the other.
	 * @param other the other TickcountSegment to compare to.
	 * @return the truth/falsehood of if the first is less or equal to than the second.
	 */
	bool operator<=( const TickcountSegment &other ) const
	{
		return ( operator<(other) || operator==(other) );
	}
	/**
	 * @brief Compares two TickcountSegments to see if one is greater than the other.
	 * @param other the other TickcountSegment to compare to.
	 * @return the truth/falsehood of if the first is greater than the second.
	 */
	bool operator>( const TickcountSegment &other ) const { return !operator<=(other); }
	/**
	 * @brief Compares two TickcountSegments to see if one is greater than or equal to the other.
	 * @param other the other TickcountSegment to compare to.
	 * @return the truth/falsehood of if the first is greater than or equal to the second.
	 */
	bool operator>=( const TickcountSegment &other ) const { return !operator<(other); }
};
/**
 * @brief Houses all of the TimingData functions.
 */
class TimingData
{
public:
	TimingData();

	void GetActualBPM( float &fMinBPMOut, float &fMaxBPMOut ) const;
	float GetBPMAtBeat( float fBeat ) const;
	void SetBPMAtRow( int iNoteRow, float fBPM );
	void SetBPMAtBeat( float fBeat, float fBPM ) { SetBPMAtRow( BeatToNoteRow(fBeat), fBPM ); }
	void SetStopAtRow( int iNoteRow, float fSeconds );
	void SetStopAtRow( int iNoteRow, float fSeconds, bool bDelay ); // (sm-ssc)
	void SetStopAtBeat( float fBeat, float fSeconds ) { SetStopAtRow( BeatToNoteRow(fBeat), fSeconds ); }
	void SetStopAtBeat( float fBeat, float fSeconds, bool bDelay ) { SetStopAtRow( BeatToNoteRow(fBeat), fSeconds, bDelay ); } // (sm-ssc)
	float GetStopAtRow( int iNoteRow, bool &bDelayOut ) const;
	float GetStopAtRow( int iRow ) const;
	float GetStopAtBeat( float fBeat ) const { return GetStopAtRow( BeatToNoteRow(fBeat) ); }
	float GetDelayAtRow( int iRow ) const;
	float GetDelayAtBeat( float fBeat ) const { return GetDelayAtRow( BeatToNoteRow(fBeat) ); }
	void SetTimeSignatureAtRow( int iRow, int iNumerator, int iDenominator );
	void SetTimeSignatureAtBeat( float fBeat, int iNumerator, int iDenominator ) { SetTimeSignatureAtRow( BeatToNoteRow(fBeat), iNumerator, iDenominator ); }
	void SetTimeSignatureNumeratorAtRow( int iRow, int iNumerator );
	void SetTimeSignatureNumeratorAtBeat( float fBeat, int iNumerator ) { SetTimeSignatureNumeratorAtRow( BeatToNoteRow(fBeat), iNumerator); }
	void SetTimeSignatureDenominatorAtRow( int iRow, int iDenominator );
	void SetTimeSignatureDenominatorAtBeat( float fBeat, int iDenominator ) { SetTimeSignatureDenominatorAtRow( BeatToNoteRow(fBeat), iDenominator); }
	int GetWarpToRow( int iWarpBeginRow ) const;
	void SetDelayAtRow( int iNoteRow, float fSeconds ); // sm-ssc
	void SetTickcountAtRow( int iRow, int iTicks );
	void SetTickcountAtBeat( float fBeat, int iTicks ) { SetTickcountAtRow( BeatToNoteRow( fBeat ), iTicks ); }
	int GetTickcountAtRow( int iRow ) const;
	int GetTickcountAtBeat( float fBeat ) const { return GetTickcountAtRow( BeatToNoteRow(fBeat) ); }
	void MultiplyBPMInBeatRange( int iStartIndex, int iEndIndex, float fFactor );
	void AddBPMSegment( const BPMSegment &seg );
	void AddStopSegment( const StopSegment &seg );
	void AddTimeSignatureSegment( const TimeSignatureSegment &seg );
	void AddWarpSegment( const WarpSegment &seg );
	void AddTickcountSegment( const TickcountSegment &seg );
	int GetBPMSegmentIndexAtBeat( float fBeat ) const;
	int GetTimeSignatureSegmentIndexAtRow( int iRow ) const;
	int GetTimeSignatureSegmentIndexAtBeat( float fBeat ) const { return GetTimeSignatureSegmentIndexAtRow( BeatToNoteRow(fBeat) ); }
	TimeSignatureSegment& GetTimeSignatureSegmentAtRow( int iRow );
	TimeSignatureSegment& GetTimeSignatureSegmentAtBeat( float fBeat ) { return GetTimeSignatureSegmentAtRow( BeatToNoteRow(fBeat) ); }
	int GetTimeSignatureNumeratorAtRow( int iRow );
	int GetTimeSignatureNumeratorAtBeat( float fBeat ) { return GetTimeSignatureNumeratorAtRow( BeatToNoteRow(fBeat) ); }
	int GetTimeSignatureDenominatorAtRow( int iRow );
 	int GetTimeSignatureDenominatorAtBeat( float fBeat ) { return GetTimeSignatureDenominatorAtRow( BeatToNoteRow(fBeat) ); }
	BPMSegment& GetBPMSegmentAtBeat( float fBeat );
	int GetTickcountSegmentIndexAtRow( int iRow ) const;
	int GetTickcountSegmentIndexAtBeat( float fBeat ) const { return GetTickcountSegmentIndexAtRow( BeatToNoteRow(fBeat) ); }
	TickcountSegment& GetTickcountSegmentAtRow( int iRow );
	TickcountSegment& GetTickcountSegmentAtBeat( float fBeat ) { return GetTickcountSegmentAtRow( BeatToNoteRow(fBeat) ); }
	void NoteRowToMeasureAndBeat( int iNoteRow, int &iMeasureIndexOut, int &iBeatIndexOut, int &iRowsRemainder ) const;

	void GetBeatAndBPSFromElapsedTime( float fElapsedTime, float &fBeatOut, float &fBPSOut, bool &bFreezeOut, bool &bDelayOut, int &iWarpBeginOut, float &fWarpLengthOut ) const;
	float GetBeatFromElapsedTime( float fElapsedTime ) const	// shortcut for places that care only about the beat
	{
		float fBeat, fThrowAway, fThrowAway2;
		bool bThrowAway, bThrowAway2;
		int iThrowAway;
		GetBeatAndBPSFromElapsedTime( fElapsedTime, fBeat, fThrowAway, bThrowAway, bThrowAway2, iThrowAway, fThrowAway2 );
		return fBeat;
	}
	float GetElapsedTimeFromBeat( float fBeat ) const;

	void GetBeatAndBPSFromElapsedTimeNoOffset( float fElapsedTime, float &fBeatOut, float &fBPSOut, bool &bFreezeOut, bool &bDelayOut, int &iWarpBeginOut, float &iWarpLengthOut ) const;
	float GetBeatFromElapsedTimeNoOffset( float fElapsedTime ) const	// shortcut for places that care only about the beat
	{
		float fBeat, fThrowAway, fThrowAway2;
		bool bThrowAway, bThrowAway2;
		int iThrowAway;
		GetBeatAndBPSFromElapsedTimeNoOffset( fElapsedTime, fBeat, fThrowAway, bThrowAway, bThrowAway2, iThrowAway, fThrowAway2 );
		return fBeat;
	}
	float GetElapsedTimeFromBeatNoOffset( float fBeat ) const;

	bool HasBpmChanges() const;
	bool HasStops() const;
	bool HasWarps() const;

	bool operator==( const TimingData &other )
	{
		COMPARE( m_BPMSegments.size() );
		for( unsigned i=0; i<m_BPMSegments.size(); i++ )
			COMPARE( m_BPMSegments[i] );
		COMPARE( m_StopSegments.size() );
		for( unsigned i=0; i<m_StopSegments.size(); i++ )
			COMPARE( m_StopSegments[i] );
		COMPARE( m_WarpSegments.size() );
		for( unsigned i=0; i<m_WarpSegments.size(); i++ )
			COMPARE( m_WarpSegments[i] );
		COMPARE( m_vTimeSignatureSegments.size() );
		for( unsigned i=0; i<m_vTimeSignatureSegments.size(); i++)
			COMPARE( m_vTimeSignatureSegments[i] );
		COMPARE( m_TickcountSegments.size() );
		for( unsigned i=0; i<m_TickcountSegments.size(); i++ )
			COMPARE( m_TickcountSegments[i] );
		COMPARE( m_fBeat0OffsetInSeconds );
		return true;
	}
	bool operator!=( const TimingData &other ) { return !operator==(other); }

	void ScaleRegion( float fScale = 1, int iStartRow = 0, int iEndRow = MAX_NOTE_ROW );
	void InsertRows( int iStartRow, int iRowsToAdd );
	void DeleteRows( int iStartRow, int iRowsToDelete );

	// Lua
	void PushSelf( lua_State *L );

	RString					m_sFile;		// informational only
	// All of the following vectors must be sorted before gameplay.
	vector<BPMSegment>		m_BPMSegments;
	vector<StopSegment>		m_StopSegments;
	vector<TimeSignatureSegment>	m_vTimeSignatureSegments;
	vector<WarpSegment>		m_WarpSegments;
	vector<TickcountSegment>	m_TickcountSegments;
	float	m_fBeat0OffsetInSeconds;
	bool	m_bHasNegativeBpms; // only used for Lua bindings in Song (to be moved to TimingData later)
};

#undef COMPARE

#endif

/**
 * @file
 * @author Chris Danford, Glenn Maynard (c) 2001-2004 
 * 
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
