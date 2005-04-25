#include "global.h"
#include "NoteDataUtil.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "PlayerOptions.h"
#include "song.h"
#include "GameState.h"
#include "RadarValues.h"
#include "Foreach.h"

NoteType NoteDataUtil::GetSmallestNoteTypeForMeasure( const NoteData &n, int iMeasureIndex )
{
	const int iMeasureStartIndex = iMeasureIndex * ROWS_PER_MEASURE;
	const int iMeasureLastIndex = (iMeasureIndex+1) * ROWS_PER_MEASURE - 1;

	// probe to find the smallest note type
	NoteType nt;
	for( nt=(NoteType)0; nt<NUM_NOTE_TYPES; nt=NoteType(nt+1) )		// for each NoteType, largest to largest
	{
		float fBeatSpacing = NoteTypeToBeat( nt );
		int iRowSpacing = int(roundf( fBeatSpacing * ROWS_PER_BEAT ));

		bool bFoundSmallerNote = false;
		// for each index in this measure
		FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( n, i, iMeasureStartIndex, iMeasureLastIndex )
		{
			if( i % iRowSpacing == 0 )
				continue;	// skip
			
			if( !n.IsRowEmpty(i) )
			{
				bFoundSmallerNote = true;
				break;
			}
		}

		if( bFoundSmallerNote )
			continue;	// searching the next NoteType
		else
			break;	// stop searching.  We found the smallest NoteType
	}

	if( nt == NUM_NOTE_TYPES )	// we didn't find one
		return NOTE_TYPE_INVALID;	// well-formed notes created in the editor should never get here
	else
		return nt;
}

void NoteDataUtil::LoadFromSMNoteDataString( NoteData &out, CString sSMNoteData )
{
	//
	// Load note data
	//

	/* Clear notes, but keep the same number of tracks. */
	int iNumTracks = out.GetNumTracks();
	out.Init();
	out.SetNumTracks( iNumTracks );

	// strip comments out of sSMNoteData
	while( sSMNoteData.Find("//") != -1 )
	{
		int iIndexCommentStart = sSMNoteData.Find("//");
		int iIndexCommentEnd = sSMNoteData.Find("\n", iIndexCommentStart);
		if( iIndexCommentEnd == -1 )	// comment doesn't have an end?
			sSMNoteData.erase( iIndexCommentStart, 2 );
		else
			sSMNoteData.erase( iIndexCommentStart, iIndexCommentEnd-iIndexCommentStart );
	}

	CStringArray asMeasures;
	split( sSMNoteData, ",", asMeasures, true );	// ignore empty is important
	for( unsigned m=0; m<asMeasures.size(); m++ )	// foreach measure
	{
		CString &sMeasureString = asMeasures[m];
		TrimLeft(sMeasureString);
		TrimRight(sMeasureString);

		CStringArray asMeasureLines;
		split( sMeasureString, "\n", asMeasureLines, true );	// ignore empty is important

		for( unsigned l=0; l<asMeasureLines.size(); l++ )
		{
			CString &sMeasureLine = asMeasureLines[l];
			TrimLeft(sMeasureLine);
			TrimRight(sMeasureLine);

			const float fPercentIntoMeasure = l/(float)asMeasureLines.size();
			const float fBeat = (m + fPercentIntoMeasure) * BEATS_PER_MEASURE;
			const int iIndex = BeatToNoteRow( fBeat );

//			if( m_iNumTracks != sMeasureLine.GetLength() )
//				RageException::Throw( "Actual number of note columns (%d) is different from the StepsType (%d).", m_iNumTracks, sMeasureLine.GetLength() );

			const char *p = sMeasureLine;
			int iTrack = 0;
			while( iTrack < iNumTracks && *p )
			{
				TapNote tn;
				char ch = *p;
				
				switch( ch )
				{
				case '0': tn = TAP_EMPTY;					break;
				case '1': tn = TAP_ORIGINAL_TAP;			break;
				case '2':
				case '4':
					switch( ch )
					{
					case '2':	tn = TAP_ORIGINAL_HOLD_HEAD;	break;
					case '4':	tn = TAP_ORIGINAL_ROLL_HEAD;	break;
					default:	ASSERT(0);
					}

					/* Set the hold note to have infinite length.  We'll clamp it when
					 * we hit the tail. */
					tn.iDuration = MAX_NOTE_ROW;
					break;
				case '3':
				{
					/* This is the end of a hold.  Search for the beginning. */
					int iHeadRow;
					if( !out.IsHoldNoteAtBeat( iTrack, iIndex, &iHeadRow ) )
					{
						LOG->Warn( "Unmatched 3 in \"%s\"", sMeasureLine.c_str() );
					}
					else
					{
						TapNote head_tap = out.GetTapNote( iTrack, iHeadRow );
						head_tap.iDuration = iIndex - iHeadRow;
						out.SetTapNote( iTrack, iHeadRow, head_tap );
					}

					/* This won't write tn, but keep parsing normally anyway. */
					break;
				}
//				case 'm':
				// Don't be loose with the definition.  Use only 'M' since
				// that's what we've been writing to disk.  -Chris
				case 'M': tn = TAP_ORIGINAL_MINE;			break;
				// case 'A': tn = TAP_ORIGINAL_ATTACK;			break;
				case 'K': tn = TAP_ORIGINAL_AUTO_KEYSOUND;	break;
				default: 
					/* Invalid data.  We don't want to assert, since there might
					 * simply be invalid data in an .SM, and we don't want to die
					 * due to invalid data.  We should probably check for this when
					 * we load SM data for the first time ... */
					// ASSERT(0); 
					tn = TAP_EMPTY;
					break;
				}
				
				p++;
				
#if 0
				// look for optional attack info (e.g. "{tipsy,50% drunk:15.2}")
				if( *p == '{' )
				{
					p++;

					char szModifiers[256] = "";
					float fDurationSeconds = 0;
					if( sscanf( p, "%255[^:]:%f}", szModifiers, &fDurationSeconds ) == 2 )	// not fatal if this fails due to malformed data
					{
						tn.type = TapNote::attack;
						tn.sAttackModifiers = szModifiers;
		 				tn.fAttackDurationSeconds = fDurationSeconds;
					}

					// skip past the '}'
					while( *p )
					{
						if( *p == '}' )
						{
							p++;
							break;
						}
						p++;
					}
				}
#endif

				// look for optional keysound index (e.g. "[123]")
				if( *p == '[' )
				{
					p++;
					int iKeysoundIndex = 0;
					if( 1 == sscanf( p, "%d]", &iKeysoundIndex ) )	// not fatal if this fails due to malformed data
					{
						tn.bKeysound = true;
		 				tn.iKeysoundIndex = iKeysoundIndex;
					}

					// skip past the ']'
					while( *p )
					{
						if( *p == ']' )
						{
							p++;
							break;
						}
						p++;
					}
				}

				/* Optimization: if we pass TAP_EMPTY, NoteData will do a search
				 * to remove anything in this position.  We know that there's nothing
				 * there, so avoid the search. */
				if( tn.type != TapNote::empty && ch != '3' )
					out.SetTapNote( iTrack, iIndex, tn );

				iTrack++;
			}
		}
	}

	/* Make sure we don't have any hold notes that didn't find a tail. */
	for( int t=0; t<out.GetNumTracks(); t++ )
	{
		NoteData::iterator begin = out.begin( t );
		NoteData::iterator end = out.end( t );
		while( begin != end )
		{
			NoteData::iterator next = Increment( begin );
			const TapNote &tn = begin->second;
			if( tn.type == TapNote::hold_head && tn.iDuration == MAX_NOTE_ROW )
			{
				int iRow = begin->first;
				LOG->Warn( "Unmatched 2 at beat %f", NoteRowToBeat(iRow) );
				out.RemoveTapNote( t, begin );
			}

			begin = next;
		}
	}
}

void NoteDataUtil::InsertHoldTails( NoteData &inout )
{
	for( int t=0; t < inout.GetNumTracks(); t++ )
	{
		NoteData::iterator begin = inout.begin(t), end = inout.end(t);

		for( ; begin != end; ++begin )
		{
			int iRow = begin->first;
			const TapNote &tn = begin->second;
			if( tn.type != TapNote::hold_head )
				continue;

			TapNote tail = tn;
			tail.type = TapNote::hold_tail;

			/* If iDuration is 0, we'd end up overwriting the head with the tail
				* (and invalidating our iterator).  Empty hold notes aren't valid. */
			ASSERT( tn.iDuration != 0 );

			inout.SetTapNote( t, iRow + tn.iDuration, tail );
		}
	}
}

void NoteDataUtil::GetSMNoteDataString( const NoteData &in_, CString &notes_out )
{
	//
	// Get note data
	//
	NoteData in( in_ );
	InsertHoldTails( in );

	float fLastBeat = in.GetLastBeat();
	int iLastMeasure = int( fLastBeat/BEATS_PER_MEASURE );

	CString &sRet = notes_out;

	sRet = "";
	
	for( int m=0; m<=iLastMeasure; m++ )	// foreach measure
	{
		if( m )
			sRet.append( 1, ',' );

		NoteType nt = GetSmallestNoteTypeForMeasure( in, m );
		int iRowSpacing;
		if( nt == NOTE_TYPE_INVALID )
			iRowSpacing = 1;
		else
			iRowSpacing = int(roundf( NoteTypeToBeat(nt) * ROWS_PER_BEAT ));
			// (verify first)
			// iRowSpacing = BeatToNoteRow( NoteTypeToBeat(nt) );
		sRet += ssprintf("  // measure %d\n", m+1);

		const int iMeasureStartRow = m * ROWS_PER_MEASURE;
		const int iMeasureLastRow = (m+1) * ROWS_PER_MEASURE - 1;

		for( int r=iMeasureStartRow; r<=iMeasureLastRow; r+=iRowSpacing )
		{
			for( int t=0; t<in.GetNumTracks(); t++ )
			{
				const TapNote &tn = in.GetTapNote(t, r);
				char c;
				switch( tn.type )
				{
				case TapNote::empty:		c = '0'; break;
				case TapNote::tap:			c = '1'; break;
				case TapNote::hold_head:
					switch( tn.subType )
					{
					case TapNote::hold_head_hold:	c = '2'; break;
					case TapNote::hold_head_roll:	c = '4'; break;
					default:	ASSERT(0);
					}
					break;
				case TapNote::hold_tail:	c = '3'; break;
				case TapNote::mine:			c = 'M'; break;
				case TapNote::attack:		c = 'A'; break;
				case TapNote::autoKeysound:	c = 'K'; break;
				default: 
					FAIL_M( ssprintf("tn %i", tn.type) );	// invalid enum value
				}
				sRet.append(1, c);

				if( tn.type == TapNote::attack )
				{
					sRet.append( ssprintf("{%s:%.2f}",tn.sAttackModifiers.c_str(), tn.fAttackDurationSeconds) );
				}
				if( tn.bKeysound )
				{
					sRet.append( ssprintf("[%d]",tn.iKeysoundIndex) );
				}
			}
			
			sRet.append(1, '\n');
		}
	}
}

void NoteDataUtil::LoadTransformedSlidingWindow( const NoteData &in, NoteData &out, int iNewNumTracks )
{
	// reset all notes
	out.Init();

	if( in.GetNumTracks() > iNewNumTracks )
	{
		/* Use a different algorithm for reducing tracks. */
		LoadOverlapped( in, out, iNewNumTracks );
		return;
	}

	out.SetNumTracks( iNewNumTracks );

	if( in.GetNumTracks() == 0 )
		return;	// nothing to do and don't AV below

	int iCurTrackOffset = 0;
	int iTrackOffsetMin = 0;
	int iTrackOffsetMax = abs( iNewNumTracks - in.GetNumTracks() );
	int bOffsetIncreasing = true;

	int iLastMeasure = 0;
	int iMeasuresSinceChange = 0;
	FOREACH_NONEMPTY_ROW_ALL_TRACKS( in, r )
	{
		const int iMeasure = r / ROWS_PER_MEASURE;
		if( iMeasure != iLastMeasure )
			++iMeasuresSinceChange;

		if( iMeasure != iLastMeasure && iMeasuresSinceChange >= 4 ) // adjust sliding window every 4 measures at most
		{
			// See if there is a hold crossing the beginning of this measure
			bool bHoldCrossesThisMeasure = false;

			for( int t=0; t<in.GetNumTracks(); t++ )
			{
				if( in.IsHoldNoteAtBeat( t, r-1 ) &&
				    in.IsHoldNoteAtBeat( t, r ) )
				{
					bHoldCrossesThisMeasure = true;
					break;
				}
			}

			// adjust offset
			if( !bHoldCrossesThisMeasure )
			{
				iMeasuresSinceChange = 0;
				iCurTrackOffset += bOffsetIncreasing ? 1 : -1;
				if( iCurTrackOffset == iTrackOffsetMin  ||  iCurTrackOffset == iTrackOffsetMax )
					bOffsetIncreasing ^= true;
				CLAMP( iCurTrackOffset, iTrackOffsetMin, iTrackOffsetMax );
			}
		}

		iLastMeasure = iMeasure;

		// copy notes in this measure
		for( int t=0; t<in.GetNumTracks(); t++ )
		{
			int iOldTrack = t;
			int iNewTrack = (iOldTrack + iCurTrackOffset) % iNewNumTracks;
			const TapNote &tn = in.GetTapNote( iOldTrack, r );
			out.SetTapNote( iNewTrack, r, tn );
		}
	}
}

void NoteDataUtil::LoadOverlapped( const NoteData &in, NoteData &out, int iNewNumTracks )
{
	out.SetNumTracks( iNewNumTracks );

	/* Keep track of the last source track that put a tap into each destination track,
	 * and the row of that tap.  Then, if two rows are trying to put taps into the
	 * same row within the shift threshold, shift the newcomer source row. */
	int LastSourceTrack[MAX_NOTE_TRACKS];
	int LastSourceRow[MAX_NOTE_TRACKS];
	int DestRow[MAX_NOTE_TRACKS];
	
	for( int tr = 0; tr < MAX_NOTE_TRACKS; ++tr )
	{
		LastSourceTrack[tr] = -1;
		LastSourceRow[tr] = -MAX_NOTE_ROW;
		DestRow[tr] = tr;
		wrap( DestRow[tr], iNewNumTracks );
	}

	const int ShiftThreshold = BeatToNoteRow(1);

	FOREACH_NONEMPTY_ROW_ALL_TRACKS( in, row )
	{
		for( int iTrackFrom = 0; iTrackFrom < in.GetNumTracks(); ++iTrackFrom )
		{
			const TapNote &tnFrom = in.GetTapNote( iTrackFrom, row );
			if( tnFrom.type == TapNote::empty )
				continue;

			/* If this is a hold note, find the end. */
			int iEndIndex = row;
			if( tnFrom.type == TapNote::hold_head )
				iEndIndex = row + tnFrom.iDuration;

			int &iTrackTo = DestRow[iTrackFrom];
			if( LastSourceTrack[iTrackTo] != iTrackFrom )
			{
				if( iEndIndex - LastSourceRow[iTrackTo] < ShiftThreshold )
				{
					/* This destination track is in use by a different source track.  Use the
					 * least-recently-used track. */
					for( int DestTrack = 0; DestTrack < iNewNumTracks; ++DestTrack )
						if( LastSourceRow[DestTrack] < LastSourceRow[iTrackTo] )
							iTrackTo = DestTrack;
				}

				/* If it's still in use, then we just don't have an available track. */
				if( iEndIndex - LastSourceRow[iTrackTo] < ShiftThreshold )
					continue;
			}

			LastSourceTrack[iTrackTo] = iTrackFrom;
			LastSourceRow[iTrackTo] = iEndIndex;
			out.SetTapNote( iTrackTo, row, tnFrom );
			if( tnFrom.type == TapNote::hold_head )
			{
				TapNote tnTail = in.GetTapNote( iTrackFrom, iEndIndex );
				out.SetTapNote( iTrackTo, iEndIndex, tnTail );
			}
		}
	}
}

int FindLongestOverlappingHoldNoteForAnyTrack( const NoteData &in, int iRow )
{
	int iMaxTailRow = -1;
	for( int t=0; t<in.GetNumTracks(); t++ )
	{
		for( int t=0; t<in.GetNumTracks(); t++ )
		{
			const TapNote &tn = in.GetTapNote( t, iRow );
			if( tn.type == TapNote::hold_head )
				iMaxTailRow = max( iMaxTailRow, iRow + tn.iDuration );
		}
	}

	return iMaxTailRow;
}

void NoteDataUtil::LoadTransformedLights( const NoteData &in, NoteData &out, int iNewNumTracks )
{
	// reset all notes
	out.Init();

	out.SetNumTracks( iNewNumTracks );

	FOREACH_NONEMPTY_ROW_ALL_TRACKS( in, r )
	{
		/* If any row starts a hold note, find the end of the hold note, and keep searching
		 * until we've extended to the end of the latest overlapping hold note. */
		int iHoldStart = r;
		int iHoldEnd = -1;
		while(1)
		{
			int iMaxTailRow = FindLongestOverlappingHoldNoteForAnyTrack( in, r );
			if( iMaxTailRow == -1 )
				break;
			iHoldEnd = iMaxTailRow;
			r = iMaxTailRow;
		}

		if( iHoldEnd != -1 )
		{
			/* If we found a hold note, add it to all tracks. */
			for( int t=0; t<in.GetNumTracks(); t++ )
				out.AddHoldNote( t, iHoldStart, iHoldEnd, TAP_ORIGINAL_HOLD_HEAD );
			continue;
		}

		if( in.IsRowEmpty( r ) )
			continue;

		/* Enable every track in the output. */
		for( int t=0; t<out.GetNumTracks(); t++ )
			out.SetTapNote( t, r, TAP_ORIGINAL_TAP );
	}
}

void NoteDataUtil::GetRadarValues( const NoteData &in, float fSongSeconds, RadarValues& out )
{
	// The for loop and the assert are used to ensure that all fields of 
	// RadarValue get set in here.
	FOREACH_RadarCategory( rc )
	{
		switch( rc )
		{
		case RADAR_STREAM:				out[rc] = GetStreamRadarValue( in, fSongSeconds );	break;	
		case RADAR_VOLTAGE:				out[rc] = GetVoltageRadarValue( in, fSongSeconds );	break;
		case RADAR_AIR:					out[rc] = GetAirRadarValue( in, fSongSeconds );		break;
		case RADAR_FREEZE:				out[rc] = GetFreezeRadarValue( in, fSongSeconds );	break;
		case RADAR_CHAOS:				out[rc] = GetChaosRadarValue( in, fSongSeconds );	break;
		case RADAR_NUM_TAPS_AND_HOLDS:	out[rc] = (float) in.GetNumRowsWithTapOrHoldHead();	break;
		case RADAR_NUM_JUMPS:			out[rc] = (float) in.GetNumJumps();					break;
		case RADAR_NUM_HOLDS:			out[rc] = (float) in.GetNumHoldNotes();				break;
		case RADAR_NUM_MINES:			out[rc] = (float) in.GetNumMines();					break;
		case RADAR_NUM_HANDS:			out[rc] = (float) in.GetNumHands();					break;
		default:	ASSERT(0);
		}
	}
}

float NoteDataUtil::GetStreamRadarValue( const NoteData &in, float fSongSeconds )
{
	if( !fSongSeconds )
		return 0.0f;
	// density of steps
	int iNumNotes = in.GetNumTapNotes() + in.GetNumHoldNotes();
	float fNotesPerSecond = iNumNotes/fSongSeconds;
	float fReturn = fNotesPerSecond / 7;
	return min( fReturn, 1.0f );
}

float NoteDataUtil::GetVoltageRadarValue( const NoteData &in, float fSongSeconds )
{
	if( !fSongSeconds )
		return 0.0f;

	const float fLastBeat = in.GetLastBeat();
	const float fAvgBPS = fLastBeat / fSongSeconds;

	// peak density of steps
	float fMaxDensitySoFar = 0;

	const float BEAT_WINDOW = 8;
	const int BEAT_WINDOW_ROWS = BeatToNoteRow( BEAT_WINDOW );

	for( int i=0; i<=int(fLastBeat); i+=BEAT_WINDOW_ROWS )
	{
		int iNumNotesThisWindow = in.GetNumTapNotes( i, i+BEAT_WINDOW_ROWS ) + in.GetNumHoldNotes( i, i+BEAT_WINDOW_ROWS );
		float fDensityThisWindow = iNumNotesThisWindow / BEAT_WINDOW;
		fMaxDensitySoFar = max( fMaxDensitySoFar, fDensityThisWindow );
	}

	float fReturn = fMaxDensitySoFar*fAvgBPS/10;
	return min( fReturn, 1.0f );
}

float NoteDataUtil::GetAirRadarValue( const NoteData &in, float fSongSeconds )
{
	if( !fSongSeconds )
		return 0.0f;
	// number of doubles
	int iNumDoubles = in.GetNumJumps();
	float fReturn = iNumDoubles / fSongSeconds;
	return min( fReturn, 1.0f );
}

float NoteDataUtil::GetFreezeRadarValue( const NoteData &in, float fSongSeconds )
{
	if( !fSongSeconds )
		return 0.0f;
	// number of hold steps
	float fReturn = in.GetNumHoldNotes() / fSongSeconds;
	return min( fReturn, 1.0f );
}

float NoteDataUtil::GetChaosRadarValue( const NoteData &in, float fSongSeconds )
{
	if( !fSongSeconds )
		return 0.0f;
	// count number of triplets or 16ths
	int iNumChaosNotes = 0;

	FOREACH_NONEMPTY_ROW_ALL_TRACKS( in, r )
	{
		if( GetNoteType(r) >= NOTE_TYPE_12TH )
			iNumChaosNotes++;
	}

	float fReturn = iNumChaosNotes / fSongSeconds * 0.5f;
	return min( fReturn, 1.0f );
}

void NoteDataUtil::RemoveHoldNotes( NoteData &in, int iStartIndex, int iEndIndex )
{
	// turn all the HoldNotes into TapNotes
	for( int t=0; t<in.GetNumTracks(); ++t )
	{
		NoteData::iterator begin, end;
		in.GetTapNoteRangeInclusive( t, iStartIndex, iEndIndex, begin, end );
		for( ; begin != end; ++begin )
		{
			if( begin->second.type != TapNote::hold_head )
				continue;
			begin->second.type = TapNote::tap;
		}
	}
}


void NoteDataUtil::RemoveSimultaneousNotes( NoteData &in, int iMaxSimultaneous, int iStartIndex, int iEndIndex )
{
	// turn all the HoldNotes into TapNotes
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( in, r, iStartIndex, iEndIndex )
	{
		set<int> viTracksHeld;
		in.GetTracksHeldAtRow( r, viTracksHeld );

		// remove the first tap note or the first hold note that starts on this row
		int iTotalTracksPressed = in.GetNumTracksWithTap(r) + viTracksHeld.size();
		int iTracksToRemove = max( 0, iTotalTracksPressed - iMaxSimultaneous );
		for( int t=0; iTracksToRemove>0 && t<in.GetNumTracks(); t++ )
		{
			const TapNote &tn = in.GetTapNote(t,r);
			if( tn.type == TapNote::tap )
			{
				in.SetTapNote( t, r, TAP_EMPTY );
				iTracksToRemove--;
			}
			else if( tn.type == TapNote::hold_head )
			{
				int iStartTrack;
				if( !in.IsHoldNoteAtBeat( t, r, &iStartTrack ) )
					continue;

				in.SetTapNote( t, iStartTrack, TAP_EMPTY );
				iTracksToRemove--;
			}
		}
	}
}

void NoteDataUtil::RemoveJumps( NoteData &inout, int iStartIndex, int iEndIndex )
{
	RemoveSimultaneousNotes( inout, 1, iStartIndex, iEndIndex );
}

void NoteDataUtil::RemoveHands( NoteData &inout, int iStartIndex, int iEndIndex )
{
	RemoveSimultaneousNotes( inout, 2, iStartIndex, iEndIndex );
}

void NoteDataUtil::RemoveQuads( NoteData &inout, int iStartIndex, int iEndIndex )
{
	RemoveSimultaneousNotes( inout, 3, iStartIndex, iEndIndex );
}

void NoteDataUtil::RemoveMines( NoteData &inout, int iStartIndex, int iEndIndex )
{
	for( int t=0; t<inout.GetNumTracks(); t++ )
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( inout, t, r, iStartIndex, iEndIndex ) 
			if( inout.GetTapNote(t,r).type == TapNote::mine )
				inout.SetTapNote( t, r, TAP_EMPTY );
}

void NoteDataUtil::RemoveAllButOneTap( NoteData &inout, int row )
{
	if(row < 0) return;

	int track;
	for( track = 0; track < inout.GetNumTracks(); ++track )
	{
		if( inout.GetTapNote(track, row).type == TapNote::tap )
			break;
	}

	track++;

	for( ; track < inout.GetNumTracks(); ++track )
	{
		if( inout.GetTapNote(track, row).type == TapNote::tap )
			inout.SetTapNote(track, row, TAP_EMPTY );
	}
}

static void GetTrackMapping( StepsType st, NoteDataUtil::TrackMapping tt, int NumTracks, int *iTakeFromTrack )
{
	// Identity transform for cases not handled below.
	for( int t = 0; t < MAX_NOTE_TRACKS; ++t )
		iTakeFromTrack[t] = t;

	switch( tt )
	{
	case NoteDataUtil::left:
	case NoteDataUtil::right:
		// Is there a way to do this withoutn handling each StepsType? -Chris
		switch( st )
		{
		case STEPS_TYPE_DANCE_SINGLE:
		case STEPS_TYPE_DANCE_DOUBLE:
		case STEPS_TYPE_DANCE_COUPLE:
			iTakeFromTrack[0] = 2;
			iTakeFromTrack[1] = 0;
			iTakeFromTrack[2] = 3;
			iTakeFromTrack[3] = 1;
			iTakeFromTrack[4] = 6;
			iTakeFromTrack[5] = 4;
			iTakeFromTrack[6] = 7;
			iTakeFromTrack[7] = 5;
			break;
		case STEPS_TYPE_DANCE_SOLO:
			iTakeFromTrack[0] = 5;
			iTakeFromTrack[1] = 4;
			iTakeFromTrack[2] = 0;
			iTakeFromTrack[3] = 3;
			iTakeFromTrack[4] = 1;
			iTakeFromTrack[5] = 2;
			break;
		case STEPS_TYPE_PUMP_SINGLE:
		case STEPS_TYPE_PUMP_COUPLE:
			iTakeFromTrack[0] = 3;
			iTakeFromTrack[1] = 4;
			iTakeFromTrack[2] = 2;
			iTakeFromTrack[3] = 0;
			iTakeFromTrack[4] = 1;
			iTakeFromTrack[5] = 8;
			iTakeFromTrack[6] = 9;
			iTakeFromTrack[7] = 7;
			iTakeFromTrack[8] = 5;
			iTakeFromTrack[9] = 6;
			break;
		case STEPS_TYPE_PUMP_HALFDOUBLE:
			iTakeFromTrack[0] = 2;
			iTakeFromTrack[1] = 0;
			iTakeFromTrack[2] = 1;
			iTakeFromTrack[3] = 3;
			iTakeFromTrack[4] = 4;
			iTakeFromTrack[5] = 5;
			break;
		case STEPS_TYPE_PUMP_DOUBLE:
			iTakeFromTrack[0] = 8;
			iTakeFromTrack[1] = 9;
			iTakeFromTrack[2] = 7;
			iTakeFromTrack[3] = 5;
			iTakeFromTrack[4] = 6;
			iTakeFromTrack[5] = 3;
			iTakeFromTrack[6] = 4;
			iTakeFromTrack[7] = 2;
			iTakeFromTrack[8] = 0;
			iTakeFromTrack[9] = 1;
			break;
		default: break;
		}

		if( tt == NoteDataUtil::right )
		{
			/* Invert. */
			int iTrack[MAX_NOTE_TRACKS];
			memcpy( iTrack, iTakeFromTrack, sizeof(iTrack) );
			for( int t = 0; t < MAX_NOTE_TRACKS; ++t )
			{
				const int to = iTrack[t];
				iTakeFromTrack[to] = t;
			}
		}

		break;
	case NoteDataUtil::mirror:
		for( int t=0; t<NumTracks; t++ )
			iTakeFromTrack[t] = NumTracks-t-1;
		break;
	case NoteDataUtil::shuffle:
	case NoteDataUtil::super_shuffle:		// use shuffle code to mix up HoldNotes without creating impossible patterns
		{
			// TRICKY: Shuffle so that both player get the same shuffle mapping
			// in the same round.
			int iOrig[MAX_NOTE_TRACKS];
			memcpy( iOrig, iTakeFromTrack, sizeof(iOrig) );

			int iShuffleSeed = GAMESTATE->m_iStageSeed;
			do {
				RandomGen rnd( iShuffleSeed );
				random_shuffle( &iTakeFromTrack[0], &iTakeFromTrack[NumTracks], rnd );
				iShuffleSeed++;
			}
			while ( !memcmp( iOrig, iTakeFromTrack, sizeof(iOrig) ) );
		}
		break;
	case NoteDataUtil::stomp:
		switch( st )
		{
		case STEPS_TYPE_DANCE_SINGLE:
		case STEPS_TYPE_DANCE_COUPLE:
			iTakeFromTrack[0] = 3;
			iTakeFromTrack[1] = 2;
			iTakeFromTrack[2] = 1;
			iTakeFromTrack[3] = 0;
			iTakeFromTrack[4] = 7;
			iTakeFromTrack[5] = 6;
			iTakeFromTrack[6] = 5;
			iTakeFromTrack[7] = 4;
			break;
		case STEPS_TYPE_DANCE_DOUBLE:
			iTakeFromTrack[0] = 1;
			iTakeFromTrack[1] = 0;
			iTakeFromTrack[2] = 3;
			iTakeFromTrack[3] = 2;
			iTakeFromTrack[4] = 5;
			iTakeFromTrack[5] = 4;
			iTakeFromTrack[6] = 7;
			iTakeFromTrack[7] = 6;
			break;
		default: 
			break;
		}
		break;
	default:
		ASSERT(0);
	}
}

static void SuperShuffleTaps( NoteData &inout, int iStartIndex, int iEndIndex )
{
	/*
	 * We already did the normal shuffling code above, which did a good job
	 * of shuffling HoldNotes without creating impossible patterns.
	 * Now, go in and shuffle the TapNotes per-row.
	 *
	 * This is only called by NoteDataUtil::Turn.
	 */
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, r, iStartIndex, iEndIndex )
	{
		for( int t1=0; t1<inout.GetNumTracks(); t1++ )
		{
			const TapNote tn1 = inout.GetTapNote(t1, r);
			switch( tn1.type )
			{
			case TapNote::empty:
			case TapNote::hold_head:
			case TapNote::hold_tail:
			case TapNote::autoKeysound:
				continue;	// skip
			case TapNote::tap:
			case TapNote::mine:
			case TapNote::attack:
				break;	// shuffle this
			default:
				ASSERT(0);
			}

#if _DEBUG
			ASSERT_M( !inout.IsHoldNoteAtBeat(t1,r), ssprintf("There is a tap.type = %d inside of a hold at row %d", tn1.type, r) );
#endif

			// Probe for a spot to swap with.
			set<int> vTriedTracks;
			for( int i=0; i<4; i++ )	// probe max 4 times
			{
				int t2 = rand() % inout.GetNumTracks();
				if( vTriedTracks.find(t2) != vTriedTracks.end() )	// already tried this track
					continue;	// skip
				vTriedTracks.insert( t2 );

				// swapping with ourself is a no-op
				if( t1 == t2 )
					break;	// done swapping

				const TapNote tn2 = inout.GetTapNote(t2, r);
				switch( tn2.type )
				{
				case TapNote::hold_head:
				case TapNote::hold_tail:
				case TapNote::autoKeysound:
					continue;	// don't swap with these
				case TapNote::empty:
				case TapNote::tap:
				case TapNote::mine:
				case TapNote::attack:
					break;	// ok to swap with this
				default:
					ASSERT(0);
				}

				// don't swap into the middle of a hold note
				if( inout.IsHoldNoteAtBeat(t2,r) )
					continue;

				// do the swap
				inout.SetTapNote(t1, r, tn2);
				inout.SetTapNote(t2, r, tn1);
				
				break;	// done swapping
			}
		}
	}
}


void NoteDataUtil::Turn( NoteData &inout, StepsType st, TrackMapping tt, int iStartIndex, int iEndIndex )
{
	int iTakeFromTrack[MAX_NOTE_TRACKS];	// New track "t" will take from old track iTakeFromTrack[t]
	GetTrackMapping( st, tt, inout.GetNumTracks(), iTakeFromTrack );

	NoteData tempNoteData;
	tempNoteData.LoadTransformed( inout, inout.GetNumTracks(), iTakeFromTrack );

	if( tt == super_shuffle )
		SuperShuffleTaps( tempNoteData, iStartIndex, iEndIndex );

	inout.CopyAll( tempNoteData );
}

void NoteDataUtil::Backwards( NoteData &inout )
{
	NoteData out;
	out.SetNumTracks( inout.GetNumTracks() );

	int max_row = inout.GetLastRow();
	for( int t=0; t<inout.GetNumTracks(); t++ )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( inout, t, r, 0, max_row )
		{
			int iRowEarlier = r;
			int iRowLater = max_row-r;

			TapNote tnEarlier = inout.GetTapNote(t, iRowEarlier);
			if( tnEarlier.type == TapNote::hold_head )
				iRowLater -= tnEarlier.iDuration;

			out.SetTapNote(t, iRowLater, tnEarlier);
		}
	}

	inout = out;
}

void NoteDataUtil::SwapSides( NoteData &inout )
{
	int iOriginalTrackToTakeFrom[MAX_NOTE_TRACKS];
	for( int t = 0; t < inout.GetNumTracks()/2; ++t )
	{
		int iTrackEarlier = t;
		int iTrackLater = t + inout.GetNumTracks()/2 + inout.GetNumTracks()%2;
		iOriginalTrackToTakeFrom[iTrackEarlier] = iTrackLater;
		iOriginalTrackToTakeFrom[iTrackLater] = iTrackEarlier;
	}

	NoteData orig( inout );
	inout.LoadTransformed( orig, orig.GetNumTracks(), iOriginalTrackToTakeFrom );
}

void NoteDataUtil::Little( NoteData &inout, int iStartIndex, int iEndIndex )
{
	// filter out all non-quarter notes
	for( int t=0; t<inout.GetNumTracks(); t++ ) 
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( inout, t, i, iStartIndex, iEndIndex )
		{
			if( i % ROWS_PER_BEAT == 0 )
				continue;

			inout.SetTapNote( t, i, TAP_EMPTY );
		}
	}
}

// Make all all quarter notes into jumps.
void NoteDataUtil::Wide( NoteData &inout, int iStartIndex, int iEndIndex )
{
	/* Start on an even beat. */
	iStartIndex = Quantize( iStartIndex, BeatToNoteRow(2.0f) );

	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, i, iStartIndex, iEndIndex )
	{
		if( i % BeatToNoteRow(2.0f) != 0 )
			continue;	// even beats only

		bool bHoldNoteAtBeat = false;
		for( int t = 0; !bHoldNoteAtBeat && t < inout.GetNumTracks(); ++t )
			if( inout.IsHoldNoteAtBeat(t, i) )
				bHoldNoteAtBeat = true;
		if( bHoldNoteAtBeat )
			continue;	// skip.  Don't place during holds

		if( inout.GetNumTracksWithTap(i) != 1 )
			continue;	// skip

		bool bSpaceAroundIsEmpty = true;	// no other notes with a 1/8th of this row
		FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, j, i-ROWS_PER_BEAT/2+1, i+ROWS_PER_BEAT/2 )
			if( j!=i  &&  inout.GetNumTapNonEmptyTracks(j) > 0 )
			{
				bSpaceAroundIsEmpty = false;
				break;
			}
				
		if( !bSpaceAroundIsEmpty )
			continue;	// skip

		// add a note determinitsitcally
		int iBeat = (int)roundf( NoteRowToBeat(i) );
		int iTrackOfNote = inout.GetFirstTrackWithTap(i);
		int iTrackToAdd = iTrackOfNote + (iBeat%5)-2;	// won't be more than 2 tracks away from the existing note
		CLAMP( iTrackToAdd, 0, inout.GetNumTracks()-1 );
		if( iTrackToAdd == iTrackOfNote )
			iTrackToAdd++;
		CLAMP( iTrackToAdd, 0, inout.GetNumTracks()-1 );
		if( iTrackToAdd == iTrackOfNote )
			iTrackToAdd--;
		CLAMP( iTrackToAdd, 0, inout.GetNumTracks()-1 );

		if( inout.GetTapNote(iTrackToAdd, i).type != TapNote::empty )
		{
			iTrackToAdd = (iTrackToAdd+1) % inout.GetNumTracks();
		}
		inout.SetTapNote(iTrackToAdd, i, TAP_ADDITION_TAP);
	}
}

void NoteDataUtil::Big( NoteData &inout, int iStartIndex, int iEndIndex )
{
	// add 8ths between 4ths
	InsertIntelligentTaps( inout,BeatToNoteRow(1.0f), BeatToNoteRow(0.5f), BeatToNoteRow(1.0f), false,iStartIndex,iEndIndex );
}

void NoteDataUtil::Quick( NoteData &inout, int iStartIndex, int iEndIndex )
{
	// add 16ths between 8ths
	InsertIntelligentTaps( inout, BeatToNoteRow(0.5f), BeatToNoteRow(0.25f), BeatToNoteRow(1.0f), false,iStartIndex,iEndIndex );
}

// Due to popular request by people annoyed with the "new" implementation of Quick, we now have
// this BMR-izer for your steps.  Use with caution.
void NoteDataUtil::BMRize( NoteData &inout, int iStartIndex, int iEndIndex )
{
	Big( inout, iStartIndex, iEndIndex );
	Quick( inout, iStartIndex, iEndIndex );
}

void NoteDataUtil::Skippy( NoteData &inout, int iStartIndex, int iEndIndex )
{
	// add 16ths between 4ths
	InsertIntelligentTaps( inout, BeatToNoteRow(1.0f), BeatToNoteRow(0.75f),BeatToNoteRow(1.0f), true,iStartIndex,iEndIndex );
}

void NoteDataUtil::InsertIntelligentTaps( 
	NoteData &inout, 
	int iWindowSizeRows, 
	int iInsertOffsetRows, 
	int iWindowStrideRows, 
	bool bSkippy, 
	int iStartIndex,
	int iEndIndex )
{
	ASSERT( iInsertOffsetRows <= iWindowSizeRows );
	ASSERT( iWindowSizeRows <= iWindowStrideRows );

	bool bRequireNoteAtBeginningOfWindow = !bSkippy;
	bool bRequireNoteAtEndOfWindow = true;

	/* Start on a multiple of fBeatInterval. */
	iStartIndex = Quantize( iStartIndex, iWindowStrideRows );

	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, i, iStartIndex, iEndIndex )
	{
		// Insert a beat in the middle of every fBeatInterval.
		if( i % iWindowStrideRows != 0 )
			continue;	// even beats only

		int iRowEarlier = i;
		int iRowLater = i + iWindowSizeRows;
		int iRowToAdd = i + iInsertOffsetRows;
		// following two lines have been changed because the behavior of treating hold-heads
		// as different from taps doesn't feel right, and because we need to check
		// against TAP_ADDITION with the BMRize mod.
		if( bRequireNoteAtBeginningOfWindow )
			if( inout.GetNumTapNonEmptyTracks(iRowEarlier)!=1 || inout.GetNumTracksWithTapOrHoldHead(iRowEarlier)!=1 )
				continue;
		if( bRequireNoteAtEndOfWindow )
			if( inout.GetNumTapNonEmptyTracks(iRowLater)!=1 || inout.GetNumTracksWithTapOrHoldHead(iRowLater)!=1 )
				continue;
		// there is a 4th and 8th note surrounding iRowBetween
		
		// don't insert a new note if there's already one within this interval
		bool bNoteInMiddle = false;
		for( int t = 0; t < inout.GetNumTracks(); ++t )
			if( inout.IsHoldNoteAtBeat(t, iRowEarlier+1) )
				bNoteInMiddle = true;
		FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, j, iRowEarlier+1, iRowLater-1 )
			bNoteInMiddle = true;
		if( bNoteInMiddle )
			continue;

		// add a note deterministically somewhere on a track different from the two surrounding notes
		int iTrackOfNoteEarlier = -1;
		bool bEarlierHasNonEmptyTrack = inout.GetTapFirstNonEmptyTrack( iRowEarlier, iTrackOfNoteEarlier );
		int iTrackOfNoteLater = -1;
		inout.GetTapFirstNonEmptyTrack( iRowLater, iTrackOfNoteLater );
		int iTrackOfNoteToAdd = 0;
		if( bSkippy  &&
			iTrackOfNoteEarlier != iTrackOfNoteLater )	// Don't make skips on the same note
		{
			if( bEarlierHasNonEmptyTrack )
			{
				iTrackOfNoteToAdd = iTrackOfNoteEarlier;
				goto done_looking_for_track_to_add;
			}
		}

		// try to choose a track between the earlier and later notes
		if( abs(iTrackOfNoteEarlier-iTrackOfNoteLater) >= 2 )
		{
			iTrackOfNoteToAdd = min(iTrackOfNoteEarlier,iTrackOfNoteLater)+1;
			goto done_looking_for_track_to_add;
		}
		
		// try to choose a track just to the left
		if( min(iTrackOfNoteEarlier,iTrackOfNoteLater)-1 >= 0 )
		{
			iTrackOfNoteToAdd = min(iTrackOfNoteEarlier,iTrackOfNoteLater)-1;
			goto done_looking_for_track_to_add;
		}

		// try to choose a track just to the right
		if( max(iTrackOfNoteEarlier,iTrackOfNoteLater)+1 < inout.GetNumTracks() )
		{
			iTrackOfNoteToAdd = max(iTrackOfNoteEarlier,iTrackOfNoteLater)+1;
			goto done_looking_for_track_to_add;
		}

done_looking_for_track_to_add:
		inout.SetTapNote(iTrackOfNoteToAdd, iRowToAdd, TAP_ADDITION_TAP);
	}
}
#if 0
class TrackIterator
{
public:
	TrackIterator();

	/* If called, iterate only over [iStart,iEnd]. */
	void SetRange( int iStart, int iEnd )
	{
	}

	/* If called, pay attention to iTrack only. */
	void SetTrack( iTrack );

	/* Extend iStart and iEnd to include hold notes overlapping the boundaries.  Call SetRange()
	 * and SetTrack() first. */
	void HoldInclusive();

	/* Reduce iStart and iEnd to exclude hold notes overlapping the boundaries.  Call SetRange()
	 * and SetTrack() first. */
	void HoldExclusive();

	/* If called, keep the iterator around.  This results in much faster iteration.  If used,
	 * ensure that the current row will always remain valid.  SetTrack() must be called first. */
	void Fast();

	/* Retrieve an iterator for the current row.  SetTrack() must be called first (but Fast()
	 * does not). */
	TapNote::iterator Get();

	int GetRow() const { return m_iCurrentRow; }
	bool Prev();
	bool Next();

private:
	int m_iStart, m_iEnd;
	int m_iTrack;

	bool m_bFast;

	int m_iCurrentRow;

	NoteData::iterator m_Iterator;

	/* m_bFast only: */
	NoteData::iterator m_Begin, m_End;
};

bool TrackIterator::Next()
{
	if( m_bFast )
	{
		if( m_Iterator == 

	}

}

TrackIterator::TrackIterator()
{
	m_iStart = 0;
	m_iEnd = MAX_NOTE_ROW;
	m_iTrack = -1;
}
#endif

void NoteDataUtil::AddMines( NoteData &inout, int iStartIndex, int iEndIndex )
{
	//
	// Change whole rows at a time to be tap notes.  Otherwise, it causes
	// major problems for our scoring system. -Chris
	//

	int iRowCount = 0;
	int iPlaceEveryRows = 6;
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, r, iStartIndex, iEndIndex )
	{
		iRowCount++;

		// place every 6 or 7 rows
		// XXX: What is "6 or 7" derived from?  Can we calculate that in a way
		// that won't break if ROWS_PER_MEASURE changes?
		if( iRowCount>=iPlaceEveryRows )
		{
			for( int t=0; t<inout.GetNumTracks(); t++ )
				if( inout.GetTapNote(t,r).type == TapNote::tap )
					inout.SetTapNote(t,r,TAP_ADDITION_MINE);
			
			iRowCount = 0;
			if( iPlaceEveryRows == 6 )
				iPlaceEveryRows = 7;
			else
				iPlaceEveryRows = 6;
		}
	}

	// Place mines right after hold so player must lift their foot.
	for( int iTrack=0; iTrack<inout.GetNumTracks(); ++iTrack )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( inout, iTrack, r, iStartIndex, iEndIndex )
		{
			const TapNote &tn = inout.GetTapNote( iTrack, r );
			if( tn.type != TapNote::hold_head )
				continue;

			int iMineRow = r + tn.iDuration + BeatToNoteRow(0.5f);
			if( iMineRow < iStartIndex || iMineRow > iEndIndex )
				continue;

			// Only place a mines if there's not another step nearby
			int iMineRangeBegin = iMineRow - BeatToNoteRow( 0.5f ) + 1;
			int iMineRangeEnd = iMineRow + BeatToNoteRow( 0.5f ) - 1;
			if( !inout.IsRangeEmpty(iTrack, iMineRangeBegin, iMineRangeEnd) )
				continue;
		
			// Add a mine right after the hold end.
			inout.SetTapNote( iTrack, iMineRow, TAP_ADDITION_MINE );

			// Convert all notes in this row to mines.
			for( int t=0; t<inout.GetNumTracks(); t++ )
				if( inout.GetTapNote(t,iMineRow).type == TapNote::tap )
					inout.SetTapNote(t,iMineRow,TAP_ADDITION_MINE);

			iRowCount = 0;
		}
	}
}

void NoteDataUtil::Echo( NoteData &inout, int iStartIndex, int iEndIndex )
{
	// add 8th note tap "echos" after all taps
	int iEchoTrack = -1;

	const int rows_per_interval = BeatToNoteRow( 0.5f );
	iStartIndex = Quantize( iStartIndex, rows_per_interval );

	/* Clamp iEndIndex to the last real tap note.  Otherwise, we'll keep adding
	 * echos of our echos all the way up to MAX_TAP_ROW. */
	iEndIndex = min( iEndIndex, inout.GetLastRow() )+1;

	// window is one beat wide and slides 1/2 a beat at a time
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, r, iStartIndex, iEndIndex )
	{
		if( r % rows_per_interval != 0 )
			continue;	// 8th notes only

		const int iRowWindowBegin = r;
		const int iRowWindowEnd = r + rows_per_interval*2;

		const int iFirstTapInRow = inout.GetFirstTrackWithTap(iRowWindowBegin);
		if( iFirstTapInRow != -1 )
			iEchoTrack = iFirstTapInRow;

		if( iEchoTrack==-1 )
			continue;	// don't lay

		// don't insert a new note if there's already a tap within this interval
		bool bTapInMiddle = false;
		FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, r2, iRowWindowBegin+1, iRowWindowEnd-1 )
			bTapInMiddle = true;
		if( bTapInMiddle )
			continue;	// don't lay


		const int iRowEcho = r + rows_per_interval;
		{
			set<int> viTracks;
			inout.GetTracksHeldAtRow( iRowEcho, viTracks );

			// don't lay if holding 2 already
			if( viTracks.size() >= 2 )
				continue;	// don't lay
			
			// don't lay echos on top of a HoldNote
			if( find(viTracks.begin(),viTracks.end(),iEchoTrack) != viTracks.end() )
				continue;	// don't lay
		}

		inout.SetTapNote( iEchoTrack, iRowEcho, TAP_ADDITION_TAP );
	}
}

void NoteDataUtil::Planted( NoteData &inout, int iStartIndex, int iEndIndex )
{
	ConvertTapsToHolds( inout, 1, iStartIndex, iEndIndex );
}
void NoteDataUtil::Floored( NoteData &inout, int iStartIndex, int iEndIndex )
{
	ConvertTapsToHolds( inout, 2, iStartIndex, iEndIndex );
}
void NoteDataUtil::Twister( NoteData &inout, int iStartIndex, int iEndIndex )
{
	ConvertTapsToHolds( inout, 3, iStartIndex, iEndIndex );
}
void NoteDataUtil::ConvertTapsToHolds( NoteData &inout, int iSimultaneousHolds, int iStartIndex, int iEndIndex )
{
	// Convert all taps to freezes.
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, r, iStartIndex, iEndIndex )
	{
		int iTrackAddedThisRow = 0;
		for( int t=0; t<inout.GetNumTracks(); t++ )
		{
			if( iTrackAddedThisRow > iSimultaneousHolds )
				break;

			if( inout.GetTapNote(t,r).type == TapNote::tap )
			{
				// Find the ending row for this hold
				int iTapsLeft = iSimultaneousHolds;

				int r2 = r+1;
				FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, next_row, r+1, iEndIndex )
				{
					r2 = next_row;

					// If there are two taps in a row on the same track, 
					// don't convert the earlier one to a hold.
					if( inout.GetTapNote(t,r2).type != TapNote::empty )
						goto dont_add_hold;

					set<int> tracksDown;
					inout.GetTracksHeldAtRow( r2, tracksDown );
					inout.GetTapNonEmptyTracks( r2, tracksDown );
					iTapsLeft -= tracksDown.size();
					if( iTapsLeft == 0 )
						break;	// we found the ending row for this hold
					else if( iTapsLeft < 0 )
						goto dont_add_hold;
				}

				// If the steps end in a tap, convert that tap
				// to a hold that lasts for at least one beat.
				if( r2 == r+1 )
					r2 = r+BeatToNoteRow(1);

				inout.AddHoldNote( t, r, r2, TAP_ORIGINAL_HOLD_HEAD );
				iTrackAddedThisRow++;
			}
dont_add_hold:
			;
		}
	}

}

void NoteDataUtil::Stomp( NoteData &inout, StepsType st, int iStartIndex, int iEndIndex )
{
	// Make all non jumps with ample space around them into jumps.
	int iTrackMapping[MAX_NOTE_TRACKS];
	GetTrackMapping( st, stomp, inout.GetNumTracks(), iTrackMapping );

	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, r, iStartIndex, iEndIndex )
	{
		if( inout.GetNumTracksWithTap(r) != 1 )
			continue;	// skip

		for( int t=0; t<inout.GetNumTracks(); t++ )
		{
			if( inout.GetTapNote(t, r).type == TapNote::tap )	// there is a tap here
			{
				// Look to see if there is enough empty space on either side of the note
				// to turn this into a jump.
				int iRowWindowBegin = r - BeatToNoteRow(0.5);
				int iRowWindowEnd = r + BeatToNoteRow(0.5);

				bool bTapInMiddle = false;
				FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, r2, iRowWindowBegin+1, iRowWindowEnd-1 )
					if( inout.IsThereATapAtRow(r2) && r2 != r )	// don't count the note we're looking around
					{
						bTapInMiddle = true;
						break;
					}
				if( bTapInMiddle )
					continue;

				// don't convert to jump if there's a hold here
				int iNumTracksHeld = inout.GetNumTracksHeldAtRow(r);
				if( iNumTracksHeld >= 1 )
					continue;

				int iOppositeTrack = iTrackMapping[t];
				inout.SetTapNote( iOppositeTrack, r, TAP_ADDITION_TAP );
			}
		}
	}		
}

void NoteDataUtil::SnapToNearestNoteType( NoteData &inout, NoteType nt1, NoteType nt2, int iStartIndex, int iEndIndex )
{
	// nt2 is optional and should be NOTE_TYPE_INVALID if it is not used

	float fSnapInterval1 = NoteTypeToBeat( nt1 );
	float fSnapInterval2 = 10000; // nothing will ever snap to this.  That's what we want!
	if( nt2 != NOTE_TYPE_INVALID )
		fSnapInterval2 = NoteTypeToBeat( nt2 );

	// iterate over all TapNotes in the interval and snap them
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, iOldIndex, iStartIndex, iEndIndex )
	{
		int iNewIndex1 = Quantize( iOldIndex, BeatToNoteRow(fSnapInterval1) );
		int iNewIndex2 = Quantize( iOldIndex, BeatToNoteRow(fSnapInterval2) );

		bool bNewBeat1IsCloser = abs(iNewIndex1-iOldIndex) < abs(iNewIndex2-iOldIndex);
		int iNewIndex = bNewBeat1IsCloser? iNewIndex1 : iNewIndex2;

		for( int c=0; c<inout.GetNumTracks(); c++ )
		{
			TapNote tnNew = inout.GetTapNote(c, iOldIndex);
			if( tnNew.type == TapNote::empty )
				continue;

			inout.SetTapNote(c, iOldIndex, TAP_EMPTY);

			if( tnNew.type == TapNote::tap && inout.IsHoldNoteAtBeat(c, iNewIndex) )
				continue; // HoldNotes override TapNotes

			if( tnNew.type == TapNote::hold_head )
			{
				/* Quantize the duration.  If the result is empty, just discard the hold. */
				tnNew.iDuration = Quantize( tnNew.iDuration, BeatToNoteRow(fSnapInterval1) );
				if( tnNew.iDuration == 0 )
					continue;

				/* We might be moving a hold note downwards, or extending its duration
				 * downwards.  Make sure there isn't anything else in the new range. */
				inout.ClearRangeForTrack( iNewIndex, iNewIndex+tnNew.iDuration+1, c );
			}
			
			inout.SetTapNote( c, iNewIndex, tnNew );
		}
	}
}


void NoteDataUtil::CopyLeftToRight( NoteData &inout )
{
	/* XXX
	inout.ConvertHoldNotesTo4s();
	for( int t=0; t<inout.GetNumTracks()/2; t++ )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK( inout, t, r )
		{
			int iTrackEarlier = t;
			int iTrackLater = inout.GetNumTracks()-1-t;

			const TapNote &tnEarlier = inout.GetTapNote(iTrackEarlier, r);
			inout.SetTapNote(iTrackLater, r, tnEarlier);
		}
	}
	inout.Convert4sToHoldNotes();
*/
}

void NoteDataUtil::CopyRightToLeft( NoteData &inout )
{
	/* XXX
	inout.ConvertHoldNotesTo4s();
	for( int t=0; t<inout.GetNumTracks()/2; t++ )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK( inout, t, r )
		{
			int iTrackEarlier = t;
			int iTrackLater = inout.GetNumTracks()-1-t;

			TapNote tnLater = inout.GetTapNote(iTrackLater, r);
			inout.SetTapNote(iTrackEarlier, r, tnLater);
		}
	}
	inout.Convert4sToHoldNotes();
*/
}

void NoteDataUtil::ClearLeft( NoteData &inout )
{
	for( int t=0; t<inout.GetNumTracks()/2; t++ )
		inout.ClearRangeForTrack( 0, MAX_NOTE_ROW, t );
}

void NoteDataUtil::ClearRight( NoteData &inout )
{
	for( int t=(inout.GetNumTracks()+1)/2; t<inout.GetNumTracks(); t++ )
		inout.ClearRangeForTrack( 0, MAX_NOTE_ROW, t );
}

void NoteDataUtil::CollapseToOne( NoteData &inout )
{
	FOREACH_NONEMPTY_ROW_ALL_TRACKS( inout, r )
		for( int t=0; t<inout.GetNumTracks(); t++ )
			if( inout.GetTapNote(t,r).type != TapNote::empty )
			{
				TapNote tn = inout.GetTapNote(t,r);
				inout.SetTapNote(t, r, TAP_EMPTY);
				inout.SetTapNote(0, r, tn);
			}
}

void NoteDataUtil::CollapseLeft( NoteData &inout )
{
	FOREACH_NONEMPTY_ROW_ALL_TRACKS( inout, r )
	{
		int iNumTracksFilled = 0;
		for( int t=0; t<inout.GetNumTracks(); t++ )
		{
			if( inout.GetTapNote(t,r).type != TapNote::empty )
			{
				TapNote tn = inout.GetTapNote(t,r);
				inout.SetTapNote(t, r, TAP_EMPTY);
				if( iNumTracksFilled < inout.GetNumTracks() )
				{
					inout.SetTapNote(iNumTracksFilled, r, tn);
					++iNumTracksFilled;
				}
			}
		}
	}
}

void NoteDataUtil::ShiftTracks( NoteData &inout, int iShiftBy )
{
	int iOriginalTrackToTakeFrom[MAX_NOTE_TRACKS];
	for( int i = 0; i < inout.GetNumTracks(); ++i )
	{
		int iFrom = i-iShiftBy;
		wrap( iFrom, inout.GetNumTracks() );
		iOriginalTrackToTakeFrom[i] = iFrom;
	}

	NoteData orig( inout );
	inout.LoadTransformed( orig, orig.GetNumTracks(), iOriginalTrackToTakeFrom );
}

void NoteDataUtil::ShiftLeft( NoteData &inout )
{
	ShiftTracks( inout, -1 );
}

void NoteDataUtil::ShiftRight( NoteData &inout )
{
	ShiftTracks( inout, +1 );
}


struct ValidRow
{
	StepsType st;
	bool bValidMask[MAX_NOTE_TRACKS];
};
#define T true
#define f false
const ValidRow g_ValidRows[] = 
{
	{ STEPS_TYPE_DANCE_DOUBLE, { T,T,T,T,f,f,f,f } },
	{ STEPS_TYPE_DANCE_DOUBLE, { f,T,T,T,T,f,f,f } },
	{ STEPS_TYPE_DANCE_DOUBLE, { f,f,f,T,T,T,T,f } },
	{ STEPS_TYPE_DANCE_DOUBLE, { f,f,f,f,T,T,T,T } },
};

void NoteDataUtil::FixImpossibleRows( NoteData &inout, StepsType st )
{
	vector<const ValidRow*> vpValidRowsToCheck;
	for( unsigned i=0; i<ARRAYSIZE(g_ValidRows); i++ )
	{
		if( g_ValidRows[i].st == st )
			vpValidRowsToCheck.push_back( &g_ValidRows[i] );
	}

	// bail early if there's nothing to validate against
	if( vpValidRowsToCheck.empty() )
		return;

	// each row must pass at least one valid mask
	FOREACH_NONEMPTY_ROW_ALL_TRACKS( inout, r )
	{
		// only check rows with jumps
		if( inout.GetNumTapNonEmptyTracks(r) < 2 )
			continue;

		bool bPassedOneMask = false;
		for( unsigned i=0; i<vpValidRowsToCheck.size(); i++ )
		{
			const ValidRow &vr = *vpValidRowsToCheck[i];
			if( NoteDataUtil::RowPassesValidMask(inout,r,vr.bValidMask) )
			{
				bPassedOneMask = true;
				break;
			}
		}

		if( !bPassedOneMask )
			RemoveAllButOneTap( inout, r );
	}
}

bool NoteDataUtil::RowPassesValidMask( NoteData &inout, int row, const bool bValidMask[] )
{
	for( int t=0; t<inout.GetNumTracks(); t++ )
	{
		if( !bValidMask[t] && inout.GetTapNote(t,row).type != TapNote::empty )
			return false;
	}

	return true;
}

void NoteDataUtil::ConvertAdditionsToRegular( NoteData &inout )
{
	for( int t=0; t<inout.GetNumTracks(); t++ )
		FOREACH_NONEMPTY_ROW_IN_TRACK( inout, t, r )
			if( inout.GetTapNote(t,r).source == TapNote::addition )
			{
				TapNote tn = inout.GetTapNote(t,r);
				tn.source = TapNote::original;
				inout.SetTapNote( t, r, tn );
			}
}

void NoteDataUtil::TransformNoteData( NoteData &nd, const AttackArray &aa, StepsType st, Song* pSong )
{
	FOREACH_CONST( Attack, aa, a )
	{
		PlayerOptions po;
		po.FromString( a->sModifier );
		if( po.ContainsTransformOrTurn() )
		{
			float fStartBeat, fEndBeat;
			a->GetAttackBeats( pSong, NULL, fStartBeat, fEndBeat );

			NoteDataUtil::TransformNoteData( nd, po, st, BeatToNoteRow(fStartBeat), BeatToNoteRow(fEndBeat) );
		}
	}
}

void NoteDataUtil::TransformNoteData( NoteData &nd, const PlayerOptions &po, StepsType st, int iStartIndex, int iEndIndex )
{
	// Apply remove transforms before others so that we don't go removing
	// notes we just inserted.
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_LITTLE] )		NoteDataUtil::Little( nd, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_NOHOLDS] )	NoteDataUtil::RemoveHoldNotes( nd, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_NOMINES] )	NoteDataUtil::RemoveMines( nd, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_NOJUMPS] )	NoteDataUtil::RemoveJumps( nd, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_NOHANDS] )	NoteDataUtil::RemoveHands( nd, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_NOQUADS] )	NoteDataUtil::RemoveQuads( nd, iStartIndex, iEndIndex );

	// Apply inserts.
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_BIG] )		NoteDataUtil::Big( nd, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_QUICK] )		NoteDataUtil::Quick( nd, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_BMRIZE] )		NoteDataUtil::BMRize( nd, iStartIndex, iEndIndex );

	// Skippy will still add taps to places that the other 
	// AddIntelligentTaps above won't.
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_SKIPPY] )		NoteDataUtil::Skippy( nd, iStartIndex, iEndIndex );

	// These aren't affects by the above inserts very much.
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_MINES] )		NoteDataUtil::AddMines( nd, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_ECHO]	)		NoteDataUtil::Echo( nd, iStartIndex, iEndIndex );

	// Jump-adding transforms aren't much affected by additional taps.
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_WIDE] )		NoteDataUtil::Wide( nd, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_STOMP] )		NoteDataUtil::Stomp( nd, st, iStartIndex, iEndIndex );

	// Transforms that add holds go last.  If they went first, most tap-adding 
	// transforms wouldn't do anything because tap-adding transforms skip areas 
	// where there's a hold.
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_PLANTED] )	NoteDataUtil::Planted( nd, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_FLOORED] )	NoteDataUtil::Floored( nd, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_TWISTER] )	NoteDataUtil::Twister( nd, iStartIndex, iEndIndex );

	// Apply turns and shuffles last to that they affect inserts.
	if( po.m_bTurns[PlayerOptions::TURN_MIRROR] )			NoteDataUtil::Turn( nd, st, NoteDataUtil::mirror, iStartIndex, iEndIndex );
	if( po.m_bTurns[PlayerOptions::TURN_LEFT] )				NoteDataUtil::Turn( nd, st, NoteDataUtil::left, iStartIndex, iEndIndex );
	if( po.m_bTurns[PlayerOptions::TURN_RIGHT] )			NoteDataUtil::Turn( nd, st, NoteDataUtil::right, iStartIndex, iEndIndex );
	if( po.m_bTurns[PlayerOptions::TURN_SHUFFLE] )			NoteDataUtil::Turn( nd, st, NoteDataUtil::shuffle, iStartIndex, iEndIndex );
	if( po.m_bTurns[PlayerOptions::TURN_SUPER_SHUFFLE] )	NoteDataUtil::Turn( nd, st, NoteDataUtil::super_shuffle, iStartIndex, iEndIndex );
}

void NoteDataUtil::AddTapAttacks( NoteData &nd, Song* pSong )
{
	// throw an attack in every 30 seconds

	const char* szAttacks[3] =
	{
		"2x",
		"drunk",
		"dizzy",
	};

	for( float sec=15; sec<pSong->m_fMusicLengthSeconds; sec+=30 )
	{
		float fBeat = pSong->GetBeatFromElapsedTime( sec );
		int iBeat = (int)fBeat;
		int iTrack = iBeat % nd.GetNumTracks();	// deterministically calculates track
		TapNote tn(
			TapNote::attack,
			TapNote::SubType_invalid,
			TapNote::original, 
			szAttacks[rand()%ARRAYSIZE(szAttacks)],
			15.0f, 
			false,
			0 );
		nd.SetTapNote( iTrack, BeatToNoteRow(fBeat), tn );
	}
}

#if 0 // undo this if ScaleRegion breaks more things than it fixes
void NoteDataUtil::Scale( NoteData &nd, float fScale )
{
	ASSERT( fScale > 0 );

	NoteData temp;
	temp.CopyAll( &nd );
	nd.ClearAll();

	for( int r=0; r<=temp.GetLastRow(); r++ )
	{
		for( int t=0; t<temp.GetNumTracks(); t++ )
		{
			TapNote tn = temp.GetTapNote( t, r );
			if( tn != TAP_EMPTY )
			{
				temp.SetTapNote( t, r, TAP_EMPTY );

				int new_row = int(r*fScale);
				nd.SetTapNote( t, new_row, tn );
			}
		}
	}
}
#endif

void NoteDataUtil::ScaleRegion( NoteData &nd, float fScale, int iStartIndex, int iEndIndex )
{
	ASSERT( fScale > 0 );
	ASSERT( iStartIndex < iEndIndex );
	ASSERT( iStartIndex >= 0 );

	NoteData temp1, temp2;
	temp1.SetNumTracks( nd.GetNumTracks() );
	temp2.SetNumTracks( nd.GetNumTracks() );

	if( iStartIndex != 0 )
		temp1.CopyRange( nd, 0, iStartIndex );
	if( iEndIndex != MAX_NOTE_ROW )
	{
		const int iScaledFirstRowAfterRegion = int(iStartIndex + (iEndIndex - iStartIndex) * fScale);
		temp1.CopyRange( nd, iEndIndex, MAX_NOTE_ROW, iScaledFirstRowAfterRegion );
	}
	temp2.CopyRange( nd, iStartIndex, iEndIndex );
	nd.ClearAll();

	for( int t=0; t<temp2.GetNumTracks(); t++ )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK( temp2, t, r )
		{
			TapNote tn = temp2.GetTapNote( t, r );
			if( tn.type != TapNote::empty )
			{
				temp2.SetTapNote( t, r, TAP_EMPTY );

				int new_row = int(r*fScale + iStartIndex);
				temp1.SetTapNote( t, new_row, tn );
			}
		}
	}

	nd.CopyAll( temp1 );
}

void NoteDataUtil::ShiftRows( NoteData &nd, int iStartIndex, int iRowsToShift )
{
	int iTakeFromRow = iStartIndex;
	int iPasteAtRow = iStartIndex;

	if( iRowsToShift > 0 )	// add blank rows
		iPasteAtRow += iRowsToShift;
	else	// delete rows
		iTakeFromRow -= iRowsToShift;

	NoteData temp;
	temp.SetNumTracks( nd.GetNumTracks() );
	temp.CopyRange( nd, iTakeFromRow, MAX_NOTE_ROW );
	nd.ClearRange( min(iTakeFromRow,iPasteAtRow), MAX_NOTE_ROW );
	nd.CopyRange( temp, 0, MAX_NOTE_ROW, iPasteAtRow );		
}

void NoteDataUtil::RemoveAllTapsOfType( NoteData& ndInOut, TapNote::Type typeToRemove )
{
	for( int t=0; t<ndInOut.GetNumTracks(); t++ )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK( ndInOut, t, row )
		{
			if( ndInOut.GetTapNote(t, row).type == typeToRemove )
				ndInOut.SetTapNote( t, row, TAP_EMPTY );
		}
	}
}

void NoteDataUtil::RemoveAllTapsExceptForType( NoteData& ndInOut, TapNote::Type typeToKeep )
{
	for( int t=0; t<ndInOut.GetNumTracks(); t++ )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK( ndInOut, t, row )
		{
			if( ndInOut.GetTapNote(t, row).type != typeToKeep )
				ndInOut.SetTapNote( t, row, TAP_EMPTY );
		}
	}
}

int NoteDataUtil::GetNumUsedTracks( const NoteData& in )
{
	for( int t=0; t<in.GetNumTracks(); t++ )
	{
		bool bHasAnyTapsInTrack = false;
		FOREACH_NONEMPTY_ROW_IN_TRACK( in, t, row )
		{
			bHasAnyTapsInTrack = true;
			break;
		}
		if( !bHasAnyTapsInTrack )
			return t;
	}
	return in.GetNumTracks();
}


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
