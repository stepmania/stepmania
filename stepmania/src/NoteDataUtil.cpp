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
		for( int i=iMeasureStartIndex; i<=iMeasureLastIndex; i++ )	// for each index in this measure
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
				case '2': tn = TAP_ORIGINAL_HOLD_HEAD;		break;
				case '3': tn = TAP_ORIGINAL_HOLD_TAIL;		break;
//				case 'm':
				// Don't be loose with the definition.  Use only 'M' since
				// that's what we've been writing to disk.  -Chris
				case 'M': tn = TAP_ORIGINAL_MINE;			break;
				case 'A': tn = TAP_ORIGINAL_ATTACK;			break;
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
				if( tn.type != TapNote::empty )
					out.SetTapNote( iTrack, iIndex, tn );

				iTrack++;
			}
		}
	}
	out.Convert2sAnd3sToHoldNotes();
}

void NoteDataUtil::GetSMNoteDataString( const NoteData &in_, CString &notes_out )
{
	//
	// Get note data
	//
	NoteData in;
	in.To2sAnd3s( in_ );

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
				case TapNote::hold_head:	c = '2'; break;
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

	NoteData Original;
	Original.To4s( in );

	out.Config( in );
	out.SetNumTracks( iNewNumTracks );


	int iCurTrackOffset = 0;
	int iTrackOffsetMin = 0;
	int iTrackOffsetMax = abs( iNewNumTracks - Original.GetNumTracks() );
	int bOffsetIncreasing = true;

	int iLastRow = Original.GetLastRow();

	int iLastMeasure = 0;
	for( int r=0; r<=iLastRow; )
	{
		// copy notes in this measure
		for( int t=0; t<Original.GetNumTracks(); t++ )
		{
			int iOldTrack = t;
			int iNewTrack = (iOldTrack + iCurTrackOffset) % iNewNumTracks;
			const TapNote &tn = Original.GetTapNote( iOldTrack, r );
			out.SetTapNote( iNewTrack, r, tn );
		}
		r++;

		const int iMeasure = r / ROWS_PER_MEASURE;
		if( iMeasure / 4 != iLastMeasure / 4 ) // adjust sliding window every 4 measures
		{
			// See if there is a hold crossing the beginning of this measure
			bool bHoldCrossesThisMeasure = false;

			if( r )
			for( int t=0; t<Original.GetNumTracks(); t++ )
			{
				if( Original.GetTapNote(t,r).type == TapNote::hold &&
					Original.GetTapNote(t,r-1).type == TapNote::hold )
				{
					bHoldCrossesThisMeasure = true;
					break;
				}
			}

			// adjust offset
			if( !bHoldCrossesThisMeasure )
			{
				iCurTrackOffset += bOffsetIncreasing ? 1 : -1;
				if( iCurTrackOffset == iTrackOffsetMin  ||  iCurTrackOffset == iTrackOffsetMax )
					bOffsetIncreasing ^= true;
				CLAMP( iCurTrackOffset, iTrackOffsetMin, iTrackOffsetMax );
			}
		}

		iLastMeasure = iMeasure;
	}

	out.Convert4sToHoldNotes();
}

void NoteDataUtil::LoadOverlapped( const NoteData &input, NoteData &out, int iNewNumTracks )
{
	out.SetNumTracks( iNewNumTracks );

	NoteData in;
	in.To4s( input );

	/* Keep track of the last source track that put a tap into each destination track,
	 * and the row of that tap.  Then, if two rows are trying to put taps into the
	 * same row within the shift threshold, shift the newcomer source row. */
	int LastSourceTrack[MAX_NOTE_TRACKS];
	int LastSourceRow[MAX_NOTE_TRACKS];
	int DestRow[MAX_NOTE_TRACKS];
	
	for( int tr = 0; tr < MAX_NOTE_TRACKS; ++tr )
	{
		LastSourceTrack[tr] = -1;
		LastSourceRow[tr] = -9999;
		DestRow[tr] = tr;
		wrap( DestRow[tr], iNewNumTracks );
	}

	const int ShiftThreshold = BeatToNoteRow(1);

	const int iLastRow = in.GetLastRow();
	for( int row = 0; row <= iLastRow; ++row )
	{
		for ( int i = 0; i < in.GetNumTracks(); i++ )
		{
			const int iTrackFrom = i;
			int &iTrackTo = DestRow[i];

			const TapNote &tnFrom = in.GetTapNote( iTrackFrom, row );
			if( tnFrom.type == TapNote::empty )
				continue;

			if( LastSourceTrack[iTrackTo] != iTrackFrom &&
				row - LastSourceRow[iTrackTo] < ShiftThreshold )
			{
				/* This destination track is in use by a different source track.  Use the
				 * least-recently-used track. */
				for( int DestTrack = 0; DestTrack < iNewNumTracks; ++DestTrack )
					if( LastSourceRow[DestTrack] < LastSourceRow[iTrackTo] )
						iTrackTo = DestTrack;
			}

			/* If it's still in use, then we just don't have an available track. */
			if( LastSourceTrack[iTrackTo] != iTrackFrom &&
				row - LastSourceRow[iTrackTo] < ShiftThreshold )
				continue;

			LastSourceTrack[iTrackTo] = iTrackFrom;
			LastSourceRow[iTrackTo] = row;
			out.SetTapNote( iTrackTo, row, tnFrom );
		}
	}

	out.Convert4sToHoldNotes();
}

void NoteDataUtil::LoadTransformedLights( const NoteData &in, NoteData &out, int iNewNumTracks )
{
	// reset all notes
	out.Init();

	NoteData Original;
	Original.To4s( in );

	out.Config(in);
	out.SetNumTracks( iNewNumTracks );

	int iLastRow = Original.GetLastRow();
	for( int r=0; r <= iLastRow; ++r )
	{
		if( Original.IsRowEmpty( r ) )
			continue;

		/* Enable every track in the output.  If there are any hold notes in the source
		 * on this row, output hold notes; otherwise tap notes.  This is to 1: prevent
		 * converting everything to hold notes, and 2: so if it's written to an SM, we
		 * don't write huge streams of tap notes. */
		bool bHoldNoteOnThisRow = false;
		for( int t=0; t<Original.GetNumTracks(); t++ )
			if( Original.GetTapNote(t,r).type == TapNote::hold )
				bHoldNoteOnThisRow = true;

		for( int t=0; t<out.GetNumTracks(); t++ )
		{
			TapNote tn = bHoldNoteOnThisRow ? TAP_ORIGINAL_HOLD : TAP_ORIGINAL_TAP;
			out.SetTapNote( t, r, tn );
		}
	}

	out.Convert4sToHoldNotes();
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
		case RADAR_NUM_JUMPS:			out[rc] = (float) in.GetNumDoubles();				break;
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

	const int BEAT_WINDOW = 8;

	for( int i=0; i<=int(fLastBeat); i+=BEAT_WINDOW )
	{
		int iNumNotesThisWindow = in.GetNumTapNotes((float)i,(float)i+BEAT_WINDOW) + in.GetNumHoldNotes((float)i,(float)i+BEAT_WINDOW);
		float fDensityThisWindow = iNumNotesThisWindow/(float)BEAT_WINDOW;
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
	int iNumDoubles = in.GetNumDoubles();
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

void NoteDataUtil::RemoveHoldNotes(NoteData &in, float fStartBeat, float fEndBeat)
{
	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );

	// turn all the HoldNotes into TapNotes
	for( int i=in.GetNumHoldNotes()-1; i>=0; i-- )	// iterate backwards so we can delete
	{
		const HoldNote hn = in.GetHoldNote( i );
		if( !hn.RangeOverlaps(iStartIndex,iEndIndex) )
			continue;	// skip

		in.RemoveHoldNote( i );
		in.SetTapNote( hn.iTrack, hn.iStartRow, TAP_ORIGINAL_TAP );
	}
}


void NoteDataUtil::RemoveSimultaneousNotes(NoteData &in, int iMaxSimultaneous, float fStartBeat, float fEndBeat)
{
	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );

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
			if( in.GetTapNote(t,r).type == TapNote::tap )
			{
				in.SetTapNote( t, r, TAP_EMPTY );
				iTracksToRemove--;
			}
			else if( in.GetTapNote(t,r).type == TapNote::hold_head )
			{
				int i;
				for( i=0; i<in.GetNumHoldNotes(); i++ )
				{
					const HoldNote& hn = in.GetHoldNote(i);
					if( hn.iStartRow == r )
						break;
				}
				ASSERT( i<in.GetNumHoldNotes() );	// if we hit this, there was a hold head with no hold
				in.RemoveHoldNote( i );
				iTracksToRemove--;
			}
		}
	}
}

void NoteDataUtil::RemoveJumps( NoteData &inout, float fStartBeat, float fEndBeat )
{
	RemoveSimultaneousNotes(inout,1);
}

void NoteDataUtil::RemoveHands( NoteData &inout, float fStartBeat, float fEndBeat )
{
	RemoveSimultaneousNotes(inout,2);
}

void NoteDataUtil::RemoveQuads( NoteData &inout, float fStartBeat, float fEndBeat )
{
	RemoveSimultaneousNotes(inout,3);
}

void NoteDataUtil::RemoveMines(NoteData &inout, float fStartBeat, float fEndBeat )
{
	int iRowStart = BeatToNoteRow(fStartBeat);
	int iRowEnd = BeatToNoteRow(fEndBeat);
	for( int t=0; t<inout.GetNumTracks(); t++ )
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( inout, t, r, iRowStart, iRowEnd ) 
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

			int iShuffleSeed = GAMESTATE->m_iRoundSeed;
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
	/* We already did the normal shuffling code above, which did a good job
	 * of shuffling HoldNotes without creating impossible patterns.
	 * Now, go in and shuffle the TapNotes per-row.
	 *
	 * This is only called by NoteDataUtil::Turn.  "in" is in 4s, and iStartIndex
	 * and iEndIndex are in range. */
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, r, iStartIndex, iEndIndex )
	{
		for( int t1=0; t1<inout.GetNumTracks(); t1++ )
		{
			const TapNote &tn1 = inout.GetTapNote(t1, r);
			if( tn1.type == TapNote::hold )
				continue;

			// a tap that is not part of a hold
			// probe for a spot to swap with
			while( 1 )
			{
				const int t2 = rand() % inout.GetNumTracks();
				const TapNote &tn2 = inout.GetTapNote(t2, r);
				if( tn2.type == TapNote::hold )	// a tap that is not part of a hold
					continue;

				// swap
				inout.SetTapNote(t1, r, tn2);
				inout.SetTapNote(t2, r, tn1);
				break;
			}
		}
	}
}


void NoteDataUtil::Turn( NoteData &inout, StepsType st, TrackMapping tt, float fStartBeat, float fEndBeat )
{
	int iTakeFromTrack[MAX_NOTE_TRACKS];	// New track "t" will take from old track iTakeFromTrack[t]
	GetTrackMapping( st, tt, inout.GetNumTracks(), iTakeFromTrack );

	if( fEndBeat == -1 )
		fEndBeat = inout.GetLastBeat();

	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );

	/* Clamp to known-good ranges. */
	iStartIndex = max( iStartIndex, 0 );
	iEndIndex = min( iEndIndex, inout.GetLastRow() );

	NoteData tempNoteData;
	tempNoteData.To4s( inout );
	NoteData tempNoteDataOut;	// write into here as we tranform
	tempNoteDataOut.Config( inout );

	// transform notes
	for( int t=0; t<inout.GetNumTracks(); t++ )
		FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( tempNoteData, r, iStartIndex, iEndIndex ) 			
			tempNoteDataOut.SetTapNote( t, r, tempNoteData.GetTapNote(iTakeFromTrack[t], r) );

	if( tt == super_shuffle )
		SuperShuffleTaps( tempNoteDataOut, iStartIndex, iEndIndex ); /* expects 4s */

	inout.From4s( tempNoteDataOut );
}

void NoteDataUtil::Backwards( NoteData &inout )
{
	inout.ConvertHoldNotesTo4s();
	int max_row = inout.GetLastRow();
	for( int t=0; t<inout.GetNumTracks(); t++ )
	{
		/* XXX: This is wrong.  Is it worth fixing?  (When is "Backwards" useful?) */
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( inout, t, r, 0, max_row/2 )
		{
			int iRowEarlier = r;
			int iRowLater = max_row-r;

			const TapNote &tnEarlier = inout.GetTapNote(t, iRowEarlier);
			const TapNote &tnLater = inout.GetTapNote(t, iRowLater);
			inout.SetTapNote(t, iRowEarlier, tnLater);
			inout.SetTapNote(t, iRowLater, tnEarlier);
		}
	}
	inout.Convert4sToHoldNotes();
}

void NoteDataUtil::SwapSides( NoteData &inout )
{
	inout.ConvertHoldNotesTo4s();
	/* XXX: This is broken. */
	for( int t=0; t<inout.GetNumTracks()/2; t++ )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK( inout, t, r )
		{
			int iTrackEarlier = t;
			int iTrackLater = t + inout.GetNumTracks()/2 + inout.GetNumTracks()%2;

			// swap
			const TapNote &tnEarlier = inout.GetTapNote(iTrackEarlier, r);
			const TapNote &tnLater = inout.GetTapNote(iTrackLater, r);
			inout.SetTapNote(iTrackEarlier, r, tnLater);
			inout.SetTapNote(iTrackLater, r, tnEarlier);
		}
	}
	inout.Convert4sToHoldNotes();
}

void NoteDataUtil::Little(NoteData &inout, float fStartBeat, float fEndBeat)
{
	if( fEndBeat == -1 )
		fEndBeat = inout.GetLastBeat();

	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );


	// filter out all non-quarter notes
	for( int t=0; t<inout.GetNumTracks(); t++ ) 
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( inout, t, i, iStartIndex, iEndIndex )
			if( i % ROWS_PER_BEAT != 0 )
					inout.SetTapNote( t, i, TAP_EMPTY );

	for( int i=inout.GetNumHoldNotes()-1; i>=0; i-- )
		if( fmodf(inout.GetHoldNote(i).GetStartBeat(),1) != 0 )	// doesn't start on a beat
			inout.RemoveHoldNote( i );
}

void NoteDataUtil::Wide( NoteData &inout, float fStartBeat, float fEndBeat )
{
	// Make all all quarter notes into jumps.

	inout.ConvertHoldNotesTo4s();

	/* Start on an even beat. */
	fStartBeat = Quantize( fStartBeat, 2.0f );

	const int first_row = BeatToNoteRow( fStartBeat );
	const int last_row = min( BeatToNoteRow(fEndBeat), inout.GetLastRow() );

	for( int i=first_row; i<last_row; i+=ROWS_PER_BEAT*2 ) // every even beat
	{
		if( inout.GetNumTapNonEmptyTracks(i) != 1 )
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

	inout.Convert4sToHoldNotes();
}

void NoteDataUtil::Big( NoteData &inout, float fStartBeat, float fEndBeat )
{
	InsertIntelligentTaps(inout,1.0f,0.5f,1.0f,false,fStartBeat,fEndBeat);	// add 8ths between 4ths
}

void NoteDataUtil::Quick( NoteData &inout, float fStartBeat, float fEndBeat )
{
	InsertIntelligentTaps(inout,0.5f,0.25f,1.0f,false,fStartBeat,fEndBeat);	// add 16ths between 8ths
}

// Due to popular request by people annoyed with the "new" implementation of Quick, we now have
// this BMR-izer for your steps.  Use with caution.
void NoteDataUtil::BMRize( NoteData &inout, float fStartBeat, float fEndBeat )
{
	Big( inout, fStartBeat, fEndBeat );
	Quick( inout, fStartBeat, fEndBeat );
}

void NoteDataUtil::Skippy( NoteData &inout, float fStartBeat, float fEndBeat )
{
	InsertIntelligentTaps(inout,1.0f,0.75f,1.0f,true,fStartBeat,fEndBeat);	// add 16ths between 4ths
}

void NoteDataUtil::InsertIntelligentTaps( 
	NoteData &inout, 
	float fWindowSizeBeats, 
	float fInsertOffsetBeats, 
	float fWindowStrideBeats, 
	bool bSkippy, 
	float fStartBeat, 
	float fEndBeat )
{
	ASSERT( fInsertOffsetBeats <= fWindowSizeBeats );
	ASSERT( fWindowSizeBeats <= fWindowStrideBeats );
	inout.ConvertHoldNotesTo4s();

	bool bRequireNoteAtBeginningOfWindow = !bSkippy;
	bool bRequireNoteAtEndOfWindow = true;

	/* Start on a multiple of fBeatInterval. */
	fStartBeat = Quantize( fStartBeat, fWindowStrideBeats );

	// Insert a beat in the middle of every fBeatInterval.
	const int first_row = BeatToNoteRow( fStartBeat );
	const int last_row = min( BeatToNoteRow(fEndBeat), inout.GetLastRow() );

	const int rows_per_window = BeatToNoteRow( fWindowSizeBeats );
	const int rows_per_stride = BeatToNoteRow( fWindowStrideBeats );
	const int insert_row_offset = BeatToNoteRow( fInsertOffsetBeats );

	for( int i=first_row; i<last_row; i+=rows_per_stride ) 
	{
		int iRowEarlier = i;
		int iRowLater = i + rows_per_window;
		int iRowToAdd = i + insert_row_offset;
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
		FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, j, iRowEarlier+1, iRowLater-1 )
			bNoteInMiddle = true;
		if( bNoteInMiddle )
			continue;

		// add a note deterministically somewhere on a track different from the two surrounding notes
		int iTrackOfNoteEarlier = -1;
		bool bEarlierHasNonEmptyTrack = inout.GetTapFirstNonEmptyTrack( iRowEarlier, iTrackOfNoteEarlier );
		int iTrackOfNoteLater = -1;
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

	inout.Convert4sToHoldNotes();
}

void NoteDataUtil::AddMines( NoteData &inout, float fStartBeat, float fEndBeat )
{
	const int first_row = BeatToNoteRow( fStartBeat );
	const int last_row = min( BeatToNoteRow(fEndBeat), inout.GetLastRow() );

	//
	// Change whole rows at a time to be tap notes.  Otherwise, it causes
	// major problems for our scoring system. -Chris
	//

	int iRowCount = 0;
	int iPlaceEveryRows = 6;
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, r, first_row, last_row )
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
	for( int i=0; i<inout.GetNumHoldNotes(); i++ )
	{
		HoldNote &hn = inout.GetHoldNote(i);
		float fHoldEndBeat = hn.GetEndBeat();
		float fMineBeat = fHoldEndBeat+0.5f;
		int iMineRow = BeatToNoteRow( fMineBeat );

		if( iMineRow < first_row || iMineRow > last_row )
			continue;

		// Only place a mines if there's not another step nearby
		int iMineRangeBegin = BeatToNoteRow( fMineBeat-0.5f ) + 1;
		int iMineRangeEnd = BeatToNoteRow( fMineBeat+0.5f ) - 1;
		if( !inout.IsRangeEmpty(hn.iTrack, iMineRangeBegin, iMineRangeEnd) )
			continue;
	
		// Add a mine right after the hold end.
		inout.SetTapNote( hn.iTrack, iMineRow, TAP_ADDITION_MINE );

		// Convert all notes inout this row to mines.
		for( int t=0; t<inout.GetNumTracks(); t++ )
			if( inout.GetTapNote(t,iMineRow).type == TapNote::tap )
				inout.SetTapNote(t,iMineRow,TAP_ADDITION_MINE);

		iRowCount = 0;
	}
}

void NoteDataUtil::Echo( NoteData &inout, float fStartBeat, float fEndBeat )
{
	// add 8th note tap "echos" after all taps
	int iEchoTrack = -1;

	fStartBeat = Quantize( fStartBeat, 0.5 );

	const int first_row = BeatToNoteRow( fStartBeat );
	const int last_row = min( BeatToNoteRow(fEndBeat), inout.GetLastRow() );
	const int rows_per_interval = BeatToNoteRow( 0.5 );

	// window is one beat wide and slides 1/2 a beat at a time
	for( int r=first_row; r<last_row; r+=rows_per_interval ) 
	{
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

void NoteDataUtil::Planted( NoteData &inout, float fStartBeat, float fEndBeat )
{
	ConvertTapsToHolds( inout, 1, fStartBeat, fEndBeat );
}
void NoteDataUtil::Floored( NoteData &inout, float fStartBeat, float fEndBeat )
{
	ConvertTapsToHolds( inout, 2, fStartBeat, fEndBeat );
}
void NoteDataUtil::Twister( NoteData &inout, float fStartBeat, float fEndBeat )
{
	ConvertTapsToHolds( inout, 3, fStartBeat, fEndBeat );
}
void NoteDataUtil::ConvertTapsToHolds( NoteData &inout, int iSimultaneousHolds, float fStartBeat, float fEndBeat )
{
	// Convert all taps to freezes.
	const int first_row = BeatToNoteRow( fStartBeat );
	const int last_row = min( BeatToNoteRow(fEndBeat), inout.GetLastRow() );
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, r, first_row, last_row )
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
				FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, next_row, r+1, last_row )
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
				float fStartBeat = NoteRowToBeat(r);
				float fEndBeat = NoteRowToBeat(r2);

				// If the steps end in a tap, convert that tap
				// to a hold that lasts for at least one beat.
				if( r2==r+1 )
					fEndBeat = fStartBeat+1;

				inout.AddHoldNote( HoldNote(t,BeatToNoteRow(fStartBeat),BeatToNoteRow(fEndBeat)) );
				iTrackAddedThisRow++;
			}
dont_add_hold:
			;
		}
	}

}

void NoteDataUtil::Stomp( NoteData &inout, StepsType st, float fStartBeat, float fEndBeat )
{
	// Make all non jumps with ample space around them into jumps.

	const int first_row = BeatToNoteRow( fStartBeat );
	const int last_row = min( BeatToNoteRow(fEndBeat), inout.GetLastRow() );

	int iTrackMapping[MAX_NOTE_TRACKS];
	GetTrackMapping( st, stomp, inout.GetNumTracks(), iTrackMapping );

	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, r, first_row, last_row )
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
				for( int r2=iRowWindowBegin+1; r2<=iRowWindowEnd-1; r2++ )
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

void NoteDataUtil::SnapToNearestNoteType( NoteData &inout, NoteType nt1, NoteType nt2, float fBeginBeat, float fEndBeat )
{
	// nt2 is optional and should be NOTE_TYPE_INVALID if it is not used

	float fSnapInterval1, fSnapInterval2;
	switch( nt1 )
	{
		case NOTE_TYPE_4TH:		fSnapInterval1 = 1/1.0f;	break;
		case NOTE_TYPE_8TH:		fSnapInterval1 = 1/2.0f;	break;
		case NOTE_TYPE_12TH:	fSnapInterval1 = 1/3.0f;	break;
		case NOTE_TYPE_16TH:	fSnapInterval1 = 1/4.0f;	break;
		case NOTE_TYPE_24TH:	fSnapInterval1 = 1/6.0f;	break;
		case NOTE_TYPE_32ND:	fSnapInterval1 = 1/8.0f;	break;
		case NOTE_TYPE_48TH:	fSnapInterval1 = 1/12.0f;	break;
		case NOTE_TYPE_64TH:	fSnapInterval1 = 1/16.0f;	break;
		default:	ASSERT( false );						return;
	}

	switch( nt2 )
	{
		case NOTE_TYPE_4TH:		fSnapInterval2 = 1/1.0f;	break;
		case NOTE_TYPE_8TH:		fSnapInterval2 = 1/2.0f;	break;
		case NOTE_TYPE_12TH:	fSnapInterval2 = 1/3.0f;	break;
		case NOTE_TYPE_16TH:	fSnapInterval2 = 1/4.0f;	break;
		case NOTE_TYPE_24TH:	fSnapInterval2 = 1/6.0f;	break;
		case NOTE_TYPE_32ND:	fSnapInterval2 = 1/8.0f;	break;
		case NOTE_TYPE_48TH:	fSnapInterval2 = 1/12.0f;	break;
		case NOTE_TYPE_64TH:	fSnapInterval2 = 1/16.0f;	break;
		case NOTE_TYPE_INVALID:	fSnapInterval2 = 10000;	break;	// nothing will ever snap to this.  That's what we want!
		default:	ASSERT( false );						return;
	}

	int rowBegin = BeatToNoteRow( fBeginBeat );
	int rowEnd = BeatToNoteRow( fEndBeat );

	inout.ConvertHoldNotesTo2sAnd3s();

	// iterate over all TapNotes in the interval and snap them
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, i, rowBegin, rowEnd )
	{
		int iOldIndex = i;
		float fOldBeat = NoteRowToBeat( iOldIndex );
		float fNewBeat1 = Quantize( fOldBeat, fSnapInterval1 );
		float fNewBeat2 = Quantize( fOldBeat, fSnapInterval2 );

		bool bNewBeat1IsCloser = fabsf(fNewBeat1-fOldBeat) < fabsf(fNewBeat2-fOldBeat);
		float fNewBeat = bNewBeat1IsCloser ? fNewBeat1 : fNewBeat2;
		int iNewIndex = BeatToNoteRow( fNewBeat );

		for( int c=0; c<inout.GetNumTracks(); c++ )
		{
			if( iOldIndex == iNewIndex )
				continue;

			TapNote tnNew = inout.GetTapNote(c, iOldIndex);
			if( tnNew.type == TapNote::empty )
				continue;

			inout.SetTapNote(c, iOldIndex, TAP_EMPTY);

			const TapNote &tnOld = inout.GetTapNote(c, iNewIndex);
			if( tnNew.type == TapNote::tap &&
				(tnOld.type == TapNote::hold_head || tnOld.type == TapNote::hold_tail) )
				continue; // HoldNotes override TapNotes

			/* If two hold tnNew boundaries are getting snapped together,
			 * merge them. */
			if( (tnNew.type == TapNote::hold_head && tnOld.type == TapNote::hold_tail) ||
				(tnNew.type == TapNote::hold_tail && tnOld.type == TapNote::hold_head) )
				tnNew = TAP_EMPTY;
			
			inout.SetTapNote(c, iNewIndex, tnNew );
		}
	}

	inout.Convert2sAnd3sToHoldNotes();
}


void NoteDataUtil::CopyLeftToRight( NoteData &inout )
{
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
}

void NoteDataUtil::CopyRightToLeft( NoteData &inout )
{
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
}

void NoteDataUtil::ClearLeft( NoteData &inout )
{
	inout.ConvertHoldNotesTo4s();
	for( int t=0; t<inout.GetNumTracks()/2; t++ )
		FOREACH_NONEMPTY_ROW_IN_TRACK( inout, t, r )
			inout.SetTapNote(t, r, TAP_EMPTY);
	inout.Convert4sToHoldNotes();
}

void NoteDataUtil::ClearRight( NoteData &inout )
{
	inout.ConvertHoldNotesTo4s();
	for( int t=(inout.GetNumTracks()+1)/2; t<inout.GetNumTracks(); t++ )
		FOREACH_NONEMPTY_ROW_IN_TRACK( inout, t, r )
			inout.SetTapNote(t, r, TAP_EMPTY);
	inout.Convert4sToHoldNotes();
}

void NoteDataUtil::CollapseToOne( NoteData &inout )
{
	inout.ConvertHoldNotesTo2sAnd3s();
	FOREACH_NONEMPTY_ROW_ALL_TRACKS( inout, r )
		for( int t=0; t<inout.GetNumTracks(); t++ )
			if( inout.GetTapNote(t,r).type != TapNote::empty )
			{
				TapNote tn = inout.GetTapNote(t,r);
				inout.SetTapNote(t, r, TAP_EMPTY);
				inout.SetTapNote(0, r, tn);
			}
	inout.Convert2sAnd3sToHoldNotes();
}

void NoteDataUtil::CollapseLeft( NoteData &inout )
{
	inout.ConvertHoldNotesTo2sAnd3s();
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
	inout.Convert2sAnd3sToHoldNotes();
}

void NoteDataUtil::ShiftLeft( NoteData &inout )
{
	inout.ConvertHoldNotesTo4s();
	FOREACH_NONEMPTY_ROW_ALL_TRACKS( inout, r )
	{
		for( int t=0; t<inout.GetNumTracks()-1; t++ )	// inout.GetNumTracks()-1 times
		{
			int iTrackEarlier = t;
			int iTrackLater = (t+1) % inout.GetNumTracks();

			// swap
			TapNote tnEarlier = inout.GetTapNote(iTrackEarlier, r);
			TapNote tnLater = inout.GetTapNote(iTrackLater, r);
			inout.SetTapNote(iTrackEarlier, r, tnLater);
			inout.SetTapNote(iTrackLater, r, tnEarlier);
		}
	}
	inout.Convert4sToHoldNotes();
}

void NoteDataUtil::ShiftRight( NoteData &inout )
{
	inout.ConvertHoldNotesTo4s();
	FOREACH_NONEMPTY_ROW_ALL_TRACKS( inout, r )
	{
		for( int t=inout.GetNumTracks()-1; t>0; t-- )	// inout.GetNumTracks()-1 times
		{
			int iTrackEarlier = t;
			int iTrackLater = (t+1) % inout.GetNumTracks();

			// swap
			TapNote tnEarlier = inout.GetTapNote(iTrackEarlier, r);
			TapNote tnLater = inout.GetTapNote(iTrackLater, r);
			inout.SetTapNote(iTrackEarlier, r, tnLater);
			inout.SetTapNote(iTrackLater, r, tnEarlier);
		}
	}
	inout.Convert4sToHoldNotes();
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
	int iLastRow = inout.GetLastRow();
	for( int r=0; r<=iLastRow; r++ )
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

			NoteDataUtil::TransformNoteData( nd, po, st, fStartBeat, fEndBeat );
		}
	}
}

void NoteDataUtil::TransformNoteData( NoteData &nd, const PlayerOptions &po, StepsType st, float fStartBeat, float fEndBeat )
{
	// Apply remove transforms before others so that we don't go removing
	// notes we just inserted.
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_LITTLE] )		NoteDataUtil::Little(nd, fStartBeat, fEndBeat);
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_NOHOLDS] )	NoteDataUtil::RemoveHoldNotes(nd, fStartBeat, fEndBeat);
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_NOMINES] )	NoteDataUtil::RemoveMines(nd, fStartBeat, fEndBeat);
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_NOJUMPS] )	NoteDataUtil::RemoveJumps(nd, fStartBeat, fEndBeat);
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_NOHANDS] )	NoteDataUtil::RemoveHands(nd, fStartBeat, fEndBeat);
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_NOQUADS] )	NoteDataUtil::RemoveQuads(nd, fStartBeat, fEndBeat);

	// Apply inserts.
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_BIG] )		NoteDataUtil::Big(nd, fStartBeat, fEndBeat);
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_QUICK] )		NoteDataUtil::Quick(nd, fStartBeat, fEndBeat);
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_BMRIZE] )		NoteDataUtil::BMRize(nd, fStartBeat, fEndBeat);

	// Skippy will still add taps to places that the other 
	// AddIntelligentTaps above won't.
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_SKIPPY] )		NoteDataUtil::Skippy(nd, fStartBeat, fEndBeat);

	// These aren't affects by the above inserts very much.
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_MINES] )		NoteDataUtil::AddMines(nd, fStartBeat, fEndBeat);
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_ECHO]	)		NoteDataUtil::Echo(nd, fStartBeat, fEndBeat);

	// Jump-adding transforms aren't much affected by additional taps.
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_WIDE] )		NoteDataUtil::Wide(nd, fStartBeat, fEndBeat);
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_STOMP] )		NoteDataUtil::Stomp(nd, st, fStartBeat, fEndBeat);

	// Transforms that add holds go last.  If they went first, most tap-adding 
	// transforms wouldn't do anything because tap-adding transforms skip areas 
	// where there's a hold.
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_PLANTED] )	NoteDataUtil::Planted(nd, fStartBeat, fEndBeat);
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_FLOORED] )	NoteDataUtil::Floored(nd, fStartBeat, fEndBeat);
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_TWISTER] )	NoteDataUtil::Twister(nd, fStartBeat, fEndBeat);

	// Apply turns and shuffles last to that they affect inserts.
	if( po.m_bTurns[PlayerOptions::TURN_MIRROR] )			NoteDataUtil::Turn( nd, st, NoteDataUtil::mirror, fStartBeat, fEndBeat );
	if( po.m_bTurns[PlayerOptions::TURN_LEFT] )				NoteDataUtil::Turn( nd, st, NoteDataUtil::left, fStartBeat, fEndBeat );
	if( po.m_bTurns[PlayerOptions::TURN_RIGHT] )			NoteDataUtil::Turn( nd, st, NoteDataUtil::right, fStartBeat, fEndBeat );
	if( po.m_bTurns[PlayerOptions::TURN_SHUFFLE] )			NoteDataUtil::Turn( nd, st, NoteDataUtil::shuffle, fStartBeat, fEndBeat );
	if( po.m_bTurns[PlayerOptions::TURN_SUPER_SHUFFLE] )	NoteDataUtil::Turn( nd, st, NoteDataUtil::super_shuffle, fStartBeat, fEndBeat );
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
		nd.SetTapAttackNote( iTrack, BeatToNoteRow(fBeat), szAttacks[rand()%ARRAYSIZE(szAttacks)], 15 );
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

// added to fix things in the editor - make sure that you're working off data that is 
// either in 4s or in 2s and 3s.
void NoteDataUtil::ScaleRegion( NoteData &nd, float fScale, float fStartBeat, float fEndBeat)
{
	ASSERT( fScale > 0 );
	ASSERT( fStartBeat < fEndBeat );
	ASSERT( fStartBeat >= 0 );

	NoteData temp1, temp2;
	temp1.Config( nd );
	temp2.Config( nd );

	const int iFirstRowAtEndOfRegion = min( nd.GetLastRow(), BeatToNoteRowNotRounded(fEndBeat) );
	const int iScaledFirstRowAfterRegion = (int)((fStartBeat + (fEndBeat - fStartBeat) * fScale) * ROWS_PER_BEAT);

	if( fStartBeat != 0 )
		temp1.CopyRange( nd, 0, BeatToNoteRowNotRounded(fStartBeat) );
	if( nd.GetLastRow() > iFirstRowAtEndOfRegion )
		temp1.CopyRange( nd, iFirstRowAtEndOfRegion, nd.GetLastRow(), iScaledFirstRowAfterRegion);
	temp2.CopyRange( nd, BeatToNoteRowNotRounded(fStartBeat), iFirstRowAtEndOfRegion );
	nd.ClearAll();

	for( int r=0; r<=temp2.GetLastRow(); r++ )
	{
		for( int t=0; t<temp2.GetNumTracks(); t++ )
		{
			TapNote tn = temp2.GetTapNote( t, r );
			if( tn.type != TapNote::empty )
			{
				temp2.SetTapNote( t, r, TAP_EMPTY );

				int new_row = int(r*fScale + fStartBeat*ROWS_PER_BEAT);
				temp1.SetTapNote( t, new_row, tn );
			}
		}
	}

	nd.CopyAll( temp1 );
}

void NoteDataUtil::ShiftRows( NoteData &nd, float fStartBeat, float fBeatsToShift )
{
	NoteData temp;
	temp.SetNumTracks( nd.GetNumTracks() );
	int iTakeFromRow=0;
	int iPasteAtRow;

	iTakeFromRow = BeatToNoteRow( fStartBeat );
	iPasteAtRow = BeatToNoteRow( fStartBeat );

	if( fBeatsToShift > 0 )	// add blank rows
		iPasteAtRow += BeatToNoteRow( fBeatsToShift );
	else	// delete rows
		iTakeFromRow += BeatToNoteRow( -fBeatsToShift );

	temp.CopyRange( nd, iTakeFromRow, nd.GetLastRow() );
	nd.ClearRange( min(iTakeFromRow,iPasteAtRow), nd.GetLastRow()  );
	nd.CopyRange( temp, 0, temp.GetLastRow(), iPasteAtRow );		
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
