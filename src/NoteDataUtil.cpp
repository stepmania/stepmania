#include "global.h"
#include "NoteDataUtil.h"
#include "NoteData.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "PlayerOptions.h"
#include "PrefsManager.h"
#include "Song.h"
#include "Style.h"
#include "GameState.h"
#include "RadarValues.h"
#include "Foreach.h"
#include "TimingData.h"
#include <utility>

// TODO: Remove these constants that aren't time signature-aware
static const int BEATS_PER_MEASURE = 4;
static const int ROWS_PER_MEASURE = ROWS_PER_BEAT * BEATS_PER_MEASURE;

NoteType NoteDataUtil::GetSmallestNoteTypeForMeasure( const NoteData &nd, int iMeasureIndex )
{
	const int iMeasureStartIndex = iMeasureIndex * ROWS_PER_MEASURE;
	const int iMeasureEndIndex = (iMeasureIndex+1) * ROWS_PER_MEASURE;

	return NoteDataUtil::GetSmallestNoteTypeInRange( nd, iMeasureStartIndex, iMeasureEndIndex );
}

NoteType NoteDataUtil::GetSmallestNoteTypeInRange( const NoteData &n, int iStartIndex, int iEndIndex )
{
	// probe to find the smallest note type
	FOREACH_ENUM(NoteType, nt)
	{
		float fBeatSpacing = NoteTypeToBeat( nt );
		int iRowSpacing = lrintf( fBeatSpacing * ROWS_PER_BEAT );

		bool bFoundSmallerNote = false;
		// for each index in this measure
		FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( n, i, iStartIndex, iEndIndex )
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
			return nt;	// stop searching. We found the smallest NoteType
	}
	return NoteType_Invalid;	// well-formed notes created in the editor should never get here
}

static void LoadFromSMNoteDataStringWithPlayer( NoteData& out, const RString &sSMNoteData, int start,
						int len, PlayerNumber pn, int iNumTracks )
{
	/* Don't allocate memory for the entire string, nor per measure. Instead, use the in-place
	 * partial string split twice. By maintaining begin and end pointers to each measure line
	 * we can perform this without copying the string at all. */
	int size = -1;
	const int end = start + len;
	vector<pair<const char *, const char *> > aMeasureLines;
	for( unsigned m = 0; true; ++m )
	{
		/* XXX Ignoring empty seems wrong for measures. It means that ",,," is treated as
		 * "," where I would expect most people would want 2 empty measures. ",\n,\n,"
		 * would do as I would expect. */
		split( sSMNoteData, ",", start, size, end, true ); // Ignore empty is important.
		if( start == end )
		{
			break;
		}
		// Partial string split.
		int measureLineStart = start, measureLineSize = -1;
		const int measureEnd = start + size;

		aMeasureLines.clear();
		for(;;)
		{
			// Ignore empty is clearly important here.
			split( sSMNoteData, "\n", measureLineStart, measureLineSize, measureEnd, true );
			if( measureLineStart == measureEnd )
			{
				break;
			}
			//RString &line = sSMNoteData.substr( measureLineStart, measureLineSize );
			const char *beginLine = sSMNoteData.data() + measureLineStart;
			const char *endLine = beginLine + measureLineSize;

			while( beginLine < endLine && strchr("\r\n\t ", *beginLine) )
				++beginLine;
			while( endLine > beginLine && strchr("\r\n\t ", *(endLine - 1)) )
				--endLine;
			if( beginLine < endLine ) // nonempty
				aMeasureLines.push_back( pair<const char *, const char *>(beginLine, endLine) );
		}

		for( unsigned l=0; l<aMeasureLines.size(); l++ )
		{
			const char *p = aMeasureLines[l].first;
			const char *const beginLine = p;
			const char *const endLine = aMeasureLines[l].second;

			const float fPercentIntoMeasure = l/(float)aMeasureLines.size();
			const float fBeat = (m + fPercentIntoMeasure) * BEATS_PER_MEASURE;
			const int iIndex = BeatToNoteRow( fBeat );

			int iTrack = 0;
			while( iTrack < iNumTracks && p < endLine )
			{
				TapNote tn;
				char ch = *p;

				switch( ch )
				{
				case '0': tn = TAP_EMPTY;				break;
				case '1': tn = TAP_ORIGINAL_TAP;			break;
				case '2':
				case '4':
				// case 'N': // minefield
					tn = ch == '2' ? TAP_ORIGINAL_HOLD_HEAD : TAP_ORIGINAL_ROLL_HEAD;
					/*
					// upcoming code for minefields -aj
					switch(ch)
					{
					case '2': tn = TAP_ORIGINAL_HOLD_HEAD; break;
					case '4': tn = TAP_ORIGINAL_ROLL_HEAD; break;
					case 'N': tn = TAP_ORIGINAL_MINE_HEAD; break;
					}
					*/

					/* Set the hold note to have infinite length. We'll clamp
					 * it when we hit the tail. */
					tn.iDuration = MAX_NOTE_ROW;
					break;
				case '3':
				{
					// This is the end of a hold. Search for the beginning.
					int iHeadRow;
					if( !out.IsHoldNoteAtRow( iTrack, iIndex, &iHeadRow ) )
					{
						int n = intptr_t(endLine) - intptr_t(beginLine);
						LOG->Warn( "Unmatched 3 in \"%.*s\"", n, beginLine );
					}
					else
					{
						out.FindTapNote( iTrack, iHeadRow )->second.iDuration = iIndex - iHeadRow;
					}

					// This won't write tn, but keep parsing normally anyway.
					break;
				}
				//				case 'm':
				// Don't be loose with the definition.  Use only 'M' since
				// that's what we've been writing to disk.  -Chris
				case 'M': tn = TAP_ORIGINAL_MINE;			break;
				// case 'A': tn = TAP_ORIGINAL_ATTACK;			break;
				case 'K': tn = TAP_ORIGINAL_AUTO_KEYSOUND;		break;
				case 'L': tn = TAP_ORIGINAL_LIFT;			break;
				case 'F': tn = TAP_ORIGINAL_FAKE;			break;
				// case 'I': tn = TAP_ORIGINAL_ITEM;			break;
				default: 
					/* Invalid data. We don't want to assert, since there might
					 * simply be invalid data in an .SM, and we don't want to die
					 * due to invalid data. We should probably check for this when
					 * we load SM data for the first time ... */
					// FAIL_M("Invalid data in SM");
					tn = TAP_EMPTY;
					break;
				}

				p++;
				// We won't scan past the end of the line so these are safe to do.
#if 0
				// look for optional attack info (e.g. "{tipsy,50% drunk:15.2}")
				if( *p == '{' )
				{
					p++;

					char szModifiers[256] = "";
					float fDurationSeconds = 0;
					if( sscanf( p, "%255[^:]:%f}", szModifiers, &fDurationSeconds ) == 2 )	// not fatal if this fails due to malformed data
					{
						tn.type = TapNoteType_Attack;
						tn.sAttackModifiers = szModifiers;
		 				tn.fAttackDurationSeconds = fDurationSeconds;
					}

					// skip past the '}'
					while( p < endLine )
					{
						if( *(p++) == '}' )
							break;
					}
				}
#endif

				// look for optional keysound index (e.g. "[123]")
				if( *p == '[' )
				{
					p++;
					int iKeysoundIndex = 0;
					if( 1 == sscanf( p, "%d]", &iKeysoundIndex ) )	// not fatal if this fails due to malformed data
		 				tn.iKeysoundIndex = iKeysoundIndex;

					// skip past the ']'
					while( p < endLine )
					{
						if( *(p++) == ']' )
							break;
					}
				}

#if 0
				// look for optional item name (e.g. "<potion>"),
				// where the name in the <> is a Lua function defined elsewhere
				// (Data/ItemTypes.lua, perhaps?) -aj
				if( *p == '<' )
				{
					p++;

					// skip past the '>'
					while( p < endLine )
					{
						if( *(p++) == '>' )
							break;
					}
				}
#endif

				/* Optimization: if we pass TAP_EMPTY, NoteData will do a search
				 * to remove anything in this position.  We know that there's nothing
				 * there, so avoid the search. */
				if( tn.type != TapNoteType_Empty && ch != '3' )
				{
					tn.pn = pn;
					out.SetTapNote( iTrack, iIndex, tn );
				}

				iTrack++;
			}
		}
	}

	// Make sure we don't have any hold notes that didn't find a tail.
	for( int t=0; t<out.GetNumTracks(); t++ )
	{
		NoteData::iterator begin = out.begin( t );
		NoteData::iterator lEnd = out.end( t );
		while( begin != lEnd )
		{
			NoteData::iterator next = Increment( begin );
			const TapNote &tn = begin->second;
			if( tn.type == TapNoteType_HoldHead && tn.iDuration == MAX_NOTE_ROW )
			{
				int iRow = begin->first;
				LOG->UserLog( "", "", "While loading .sm/.ssc note data, there was an unmatched 2 at beat %f", NoteRowToBeat(iRow) );
				out.RemoveTapNote( t, begin );
			}

			begin = next;
		}
	}
	out.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::LoadFromSMNoteDataString( NoteData &out, const RString &sSMNoteData_, bool bComposite )
{
	// Load note data
	RString sSMNoteData;
	RString::size_type iIndexCommentStart = 0;
	RString::size_type iIndexCommentEnd = 0;
	RString::size_type origSize = sSMNoteData_.size();
	const char *p = sSMNoteData_.data();

	sSMNoteData.reserve( origSize );
	while( (iIndexCommentStart = sSMNoteData_.find("//", iIndexCommentEnd)) != RString::npos )
	{
		sSMNoteData.append( p, iIndexCommentStart - iIndexCommentEnd );
		p += iIndexCommentStart - iIndexCommentEnd;
		iIndexCommentEnd = sSMNoteData_.find( "\n", iIndexCommentStart );
		iIndexCommentEnd = (iIndexCommentEnd == RString::npos ? origSize : iIndexCommentEnd+1);
		p += iIndexCommentEnd - iIndexCommentStart;
	}
	sSMNoteData.append( p, origSize - iIndexCommentEnd );

	// Clear notes, but keep the same number of tracks.
	int iNumTracks = out.GetNumTracks();
	out.Init();
	out.SetNumTracks( iNumTracks );

	if( !bComposite )
	{
		LoadFromSMNoteDataStringWithPlayer( out, sSMNoteData, 0, sSMNoteData.size(),
						    PLAYER_INVALID, iNumTracks );
		return;
	}

	int start = 0, size = -1;

	vector<NoteData> vParts;
	FOREACH_PlayerNumber( pn )
	{
		// Split in place.
		split( sSMNoteData, "&", start, size, false );
		if( unsigned(start) == sSMNoteData.size() )
			break;
		vParts.push_back( NoteData() );
		NoteData &nd = vParts.back();

		nd.SetNumTracks( iNumTracks );
		LoadFromSMNoteDataStringWithPlayer( nd, sSMNoteData, start, size, pn, iNumTracks );
	}
	CombineCompositeNoteData( out, vParts );
	out.RevalidateATIs(vector<int>(), false);
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
			if( tn.type != TapNoteType_HoldHead )
				continue;

			TapNote tail = tn;
			tail.type = TapNoteType_HoldTail;

			/* If iDuration is 0, we'd end up overwriting the head with the tail
			 * (and invalidating our iterator). Empty hold notes aren't valid. */
			ASSERT( tn.iDuration != 0 );

			inout.SetTapNote( t, iRow + tn.iDuration, tail );
		}
	}
}

void NoteDataUtil::GetSMNoteDataString( const NoteData &in, RString &sRet )
{
	// Get note data
	vector<NoteData> parts;
	float fLastBeat = -1.0f;

	SplitCompositeNoteData( in, parts );

	FOREACH( NoteData, parts, nd )
	{
		InsertHoldTails( *nd );
		fLastBeat = max( fLastBeat, nd->GetLastBeat() );
	}

	int iLastMeasure = int( fLastBeat/BEATS_PER_MEASURE );

	sRet = "";
	FOREACH( NoteData, parts, nd )
	{
		if( nd != parts.begin() )
			sRet.append( "&\n" );
		for( int m = 0; m <= iLastMeasure; ++m ) // foreach measure
		{
			if( m )
				sRet.append( 1, ',' );
			sRet += ssprintf("  // measure %d\n", m);

			NoteType nt = GetSmallestNoteTypeForMeasure( *nd, m );
			int iRowSpacing;
			if( nt == NoteType_Invalid )
				iRowSpacing = 1;
			else
				iRowSpacing = lrintf( NoteTypeToBeat(nt) * ROWS_PER_BEAT );
			// (verify first)
			// iRowSpacing = BeatToNoteRow( NoteTypeToBeat(nt) );

			const int iMeasureStartRow = m * ROWS_PER_MEASURE;
			const int iMeasureLastRow = (m+1) * ROWS_PER_MEASURE - 1;

			for( int r=iMeasureStartRow; r<=iMeasureLastRow; r+=iRowSpacing )
			{
				for( int t = 0; t < nd->GetNumTracks(); ++t )
				{
					const TapNote &tn = nd->GetTapNote(t, r);
					char c;
					switch( tn.type )
					{
					case TapNoteType_Empty:			c = '0'; break;
					case TapNoteType_Tap:			c = '1'; break;
					case TapNoteType_HoldHead:
						switch( tn.subType )
						{
						case TapNoteSubType_Hold:	c = '2'; break;
						case TapNoteSubType_Roll:	c = '4'; break;
						//case TapNoteSubType_Mine:	c = 'N'; break;
						default:
							FAIL_M(ssprintf("Invalid tap note subtype: %i", tn.subType));
						}
						break;
					case TapNoteType_HoldTail:		c = '3'; break;
					case TapNoteType_Mine:			c = 'M'; break;
					case TapNoteType_Attack:			c = 'A'; break;
					case TapNoteType_AutoKeysound:	c = 'K'; break;
					case TapNoteType_Lift:			c = 'L'; break;
					case TapNoteType_Fake:			c = 'F'; break;
					default: 
						c = '\0';
						FAIL_M(ssprintf("Invalid tap note type: %i", tn.type));
					}
					sRet.append( 1, c );

					if( tn.type == TapNoteType_Attack )
					{
						sRet.append( ssprintf("{%s:%.2f}", tn.sAttackModifiers.c_str(),
								      tn.fAttackDurationSeconds) );
					}
					// hey maybe if we have TapNoteType_Item we can do things here.
					if( tn.iKeysoundIndex >= 0 )
						sRet.append( ssprintf("[%d]",tn.iKeysoundIndex) );
				}

				sRet.append( 1, '\n' );
			}
		}
	}
}

void NoteDataUtil::SplitCompositeNoteData( const NoteData &in, vector<NoteData> &out )
{
	if( !in.IsComposite() )
	{
		out.push_back( in );
		return;
	}

	FOREACH_PlayerNumber( pn )
	{
		out.push_back( NoteData() );
		out.back().SetNumTracks( in.GetNumTracks() );
	}

	for( int t = 0; t < in.GetNumTracks(); ++t )
	{
		for( NoteData::const_iterator iter = in.begin(t); iter != in.end(t); ++iter )
		{
			int row = iter->first;
			TapNote tn = iter->second;
			/*
			 XXX: This code is (hopefully) a temporary hack to make sure that
			 routine charts don't have any notes without players assigned to them.
			 I suspect this is due to a related bug that these problems were
			 occuring to begin with, but at this time, I am unsure how to deal with it.
			 Hopefully this hack can be removed soon. -- Jason "Wolfman2000" Felds
			 */
			const Style *curStyle = GAMESTATE->GetCurrentStyle(PLAYER_INVALID);
			if( (curStyle == NULL || curStyle->m_StyleType == StyleType_TwoPlayersSharedSides )
				&& int( tn.pn ) > NUM_PlayerNumber )
			{
				tn.pn = PLAYER_1;
			}
			unsigned index = int( tn.pn );

			ASSERT_M( index < NUM_PlayerNumber, ssprintf("We have a note not assigned to a player. The note in question is on beat %f, column %i.", NoteRowToBeat(row), t + 1) );
			tn.pn = PLAYER_INVALID;
			out[index].SetTapNote( t, row, tn );
		}
	}
}

void NoteDataUtil::CombineCompositeNoteData( NoteData &out, const vector<NoteData> &in )
{
	FOREACH_CONST( NoteData, in, nd )
	{
		const int iMaxTracks = min( out.GetNumTracks(), nd->GetNumTracks() );

		for( int track = 0; track < iMaxTracks; ++track )
		{
			for( NoteData::const_iterator i = nd->begin(track); i != nd->end(track); ++i )
			{
				int row = i->first;
				if( out.IsHoldNoteAtRow(track, i->first) )
					continue;
				if( i->second.type == TapNoteType_HoldHead )
					out.AddHoldNote( track, row, row + i->second.iDuration, i->second );
				else
					out.SetTapNote( track, row, i->second );
			}
		}
	}
	out.RevalidateATIs(vector<int>(), false);
}


void NoteDataUtil::LoadTransformedSlidingWindow( const NoteData &in, NoteData &out, int iNewNumTracks )
{
	// reset all notes
	out.Init();
	
	if( in.GetNumTracks() > iNewNumTracks )
	{
		// Use a different algorithm for reducing tracks.
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
				if( in.IsHoldNoteAtRow( t, r-1 ) &&
				    in.IsHoldNoteAtRow( t, r ) )
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
			TapNote tn = in.GetTapNote( iOldTrack, r );
			tn.pn= PLAYER_INVALID;
			out.SetTapNote( iNewTrack, r, tn );
		}
	}
	out.RevalidateATIs(vector<int>(), false);
}

void PlaceAutoKeysound( NoteData &out, int row, TapNote akTap )
{
	int iEmptyTrack = -1;
	int iEmptyRow = row;
	int iNewNumTracks = out.GetNumTracks();
	bool bFoundEmptyTrack = false;
	int iRowsToLook[3] = {0, -1, 1};
	
	for( int j = 0; j < 3; j ++ )
	{
		int r = iRowsToLook[j] + row;
		if( r < 0 )
			continue;
		for( int i = 0; i < iNewNumTracks; ++i )
		{
			if ( out.GetTapNote(i, r) == TAP_EMPTY && !out.IsHoldNoteAtRow(i, r) )
			{
				iEmptyTrack = i;
				iEmptyRow = r;
				bFoundEmptyTrack = true;
				break;
			}
		}
		if( bFoundEmptyTrack )
			break;
	}
	
	if( iEmptyTrack != -1 )
	{
		akTap.type = TapNoteType_AutoKeysound;
		out.SetTapNote( iEmptyTrack, iEmptyRow, akTap );
	}
}

void NoteDataUtil::LoadOverlapped( const NoteData &in, NoteData &out, int iNewNumTracks )
{
	out.SetNumTracks( iNewNumTracks );

	/* Keep track of the last source track that put a tap into each destination track,
	 * and the row of that tap. Then, if two rows are trying to put taps into the
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
			TapNote tnFrom = in.GetTapNote( iTrackFrom, row );
			if( tnFrom.type == TapNoteType_Empty || tnFrom.type == TapNoteType_AutoKeysound )
				continue;
			tnFrom.pn= PLAYER_INVALID;

			// If this is a hold note, find the end.
			int iEndIndex = row;
			if( tnFrom.type == TapNoteType_HoldHead )
				iEndIndex = row + tnFrom.iDuration;

			int &iTrackTo = DestRow[iTrackFrom];
			if( LastSourceTrack[iTrackTo] != iTrackFrom )
			{
				if( iEndIndex - LastSourceRow[iTrackTo] < ShiftThreshold )
				{
					/* This destination track is in use by a different source
					 * track. Use the least-recently-used track. */
					for( int DestTrack = 0; DestTrack < iNewNumTracks; ++DestTrack )
						if( LastSourceRow[DestTrack] < LastSourceRow[iTrackTo] )
							iTrackTo = DestTrack;
				}

				// If it's still in use, then we just don't have an available track.
				if( iEndIndex - LastSourceRow[iTrackTo] < ShiftThreshold )
				{
					// If it has a keysound, put it in autokeysound track.
					if( tnFrom.iKeysoundIndex >= 0 )
					{
						TapNote akTap = tnFrom;
						PlaceAutoKeysound( out, row, akTap );
					}
					continue;
				}
			}

			LastSourceTrack[iTrackTo] = iTrackFrom;
			LastSourceRow[iTrackTo] = iEndIndex;
			out.SetTapNote( iTrackTo, row, tnFrom );
			if( tnFrom.type == TapNoteType_HoldHead )
			{
				const TapNote &tnTail = in.GetTapNote( iTrackFrom, iEndIndex );
				out.SetTapNote( iTrackTo, iEndIndex, tnTail );
			}
		}
		
		// find empty track for autokeysounds in 2 next rows, so you can hear most autokeysounds
		for( int iTrackFrom = 0; iTrackFrom < in.GetNumTracks(); ++iTrackFrom )
		{
			const TapNote &tnFrom = in.GetTapNote( iTrackFrom, row );
			if( tnFrom.type != TapNoteType_AutoKeysound )
				continue;
			
			PlaceAutoKeysound( out, row, tnFrom );
		}
	}
	out.RevalidateATIs(vector<int>(), false);
}

int FindLongestOverlappingHoldNoteForAnyTrack( const NoteData &in, int iRow )
{
	int iMaxTailRow = -1;
	for( int t=0; t<in.GetNumTracks(); t++ )
	{
		const TapNote &tn = in.GetTapNote( t, iRow );
		if( tn.type == TapNoteType_HoldHead )
			iMaxTailRow = max( iMaxTailRow, iRow + tn.iDuration );
	}

	return iMaxTailRow;
}

// For every row in "in" with a tap or hold on any track, enable the specified tracks in "out".
void LightTransformHelper( const NoteData &in, NoteData &out, const vector<int> &aiTracks )
{
	for( unsigned i = 0; i < aiTracks.size(); ++i )
		ASSERT_M( aiTracks[i] < out.GetNumTracks(), ssprintf("%i, %i", aiTracks[i], out.GetNumTracks()) );

	FOREACH_NONEMPTY_ROW_ALL_TRACKS( in, r )
	{
		/* If any row starts a hold note, find the end of the hold note, and keep searching
		 * until we've extended to the end of the latest overlapping hold note. */
		int iHoldStart = r;
		int iHoldEnd = -1;
		for(;;)
		{
			int iMaxTailRow = FindLongestOverlappingHoldNoteForAnyTrack( in, r );
			if( iMaxTailRow == -1 )
			{
				break;
			}
			iHoldEnd = iMaxTailRow;
			r = iMaxTailRow;
		}

		if( iHoldEnd != -1 )
		{
			// If we found a hold note, add it to all tracks.
			for( unsigned i = 0; i < aiTracks.size(); ++i )
			{
				int t = aiTracks[i];
				out.AddHoldNote( t, iHoldStart, iHoldEnd, TAP_ORIGINAL_HOLD_HEAD );
			}
			continue;
		}

		if( in.IsRowEmpty(r) )
			continue;

		// Enable every track in the output.
		for( unsigned i = 0; i < aiTracks.size(); ++i )
		{
			int t = aiTracks[i];
			out.SetTapNote( t, r, TAP_ORIGINAL_TAP );
		}
	}
}

// For every track enabled in "in", enable all tracks in "out".
void NoteDataUtil::LoadTransformedLights( const NoteData &in, NoteData &out, int iNewNumTracks )
{
	if (PREFSMAN->m_bOITGStyleLights)
	{
		NoteData bass;
		bass.Init();

		// copy from the marquee data, but slim down the notes.
		// this makes it look more bass-ish and less like the original chart.
		bass.CopyAll(in);
		RemoveHoldNotes(bass);
		Little(bass);

		LoadTransformedLightsFromTwo(in, bass, out);
	}
	else
	{
	
		// reset all notes
		out.Init();

		out.SetNumTracks( iNewNumTracks );

		vector<int> aiTracks;
		for( int i = 0; i < out.GetNumTracks(); ++i )
			aiTracks.push_back( i );

		LightTransformHelper( in, out, aiTracks );
	}
}

// This transform is specific to StepsType_lights_cabinet.
#include "LightsManager.h" // for LIGHT_*
void NoteDataUtil::LoadTransformedLightsFromTwo( const NoteData &marquee, const NoteData &bass, NoteData &out )
{
	ASSERT( marquee.GetNumTracks() >= 4 );
	ASSERT( bass.GetNumTracks() >= 1 );

	/* For each track in "marquee", enable a track in the marquee lights.
	 * This will reinit out. */
	{
		NoteData transformed_marquee;
		transformed_marquee.CopyAll( marquee );
		Wide( transformed_marquee );

		const int iOriginalTrackToTakeFrom[NUM_CabinetLight] = { 0, 1, 2, 3, -1, -1 };
		out.LoadTransformed( transformed_marquee, NUM_CabinetLight, iOriginalTrackToTakeFrom );
	}

	// For each track in "bass", enable the bass lights.
	{
		vector<int> aiTracks;
		aiTracks.push_back( LIGHT_BASS_LEFT );
		aiTracks.push_back( LIGHT_BASS_RIGHT );
		LightTransformHelper( bass, out, aiTracks );
	}

	// Delete all mines.
	NoteDataUtil::RemoveMines( out );
}

// This kickbox_limb enum should not be used anywhere outside the
// Autogenkickbox function. -Kyz
enum kickbox_limb
{
	left_foot, left_fist, right_fist, right_foot, num_kickbox_limbs, invalid_limb= -1
};
void NoteDataUtil::AutogenKickbox(const NoteData& in, NoteData& out, const TimingData& timing, StepsType out_type, int nonrandom_seed)
{
	// Each limb has its own list of tracks it is used for.  This allows
	// abstract handling of the different styles.
	// By convention, the lower panels are pushed first.  This gives the upper
	// panels a higher index, which is mnemonically useful.
	vector<vector<int> > limb_tracks(num_kickbox_limbs);
	bool have_feet= true;
	switch(out_type)
	{
		case StepsType_kickbox_human:
			out.SetNumTracks(4);
			limb_tracks[left_foot].push_back(0);
			limb_tracks[left_fist].push_back(1);
			limb_tracks[right_fist].push_back(2);
			limb_tracks[right_foot].push_back(3);
			break;
		case StepsType_kickbox_quadarm:
			out.SetNumTracks(4);
			have_feet= false;
			limb_tracks[left_fist].push_back(1);
			limb_tracks[left_fist].push_back(0);
			limb_tracks[right_fist].push_back(2);
			limb_tracks[right_fist].push_back(3);
			break;
		case StepsType_kickbox_insect:
			out.SetNumTracks(6);
			limb_tracks[left_foot].push_back(0);
			limb_tracks[left_fist].push_back(2);
			limb_tracks[left_fist].push_back(1);
			limb_tracks[right_fist].push_back(3);
			limb_tracks[right_fist].push_back(4);
			limb_tracks[right_foot].push_back(5);
			break;
		case StepsType_kickbox_arachnid:
			out.SetNumTracks(8);
			limb_tracks[left_foot].push_back(0);
			limb_tracks[left_foot].push_back(1);
			limb_tracks[left_fist].push_back(3);
			limb_tracks[left_fist].push_back(2);
			limb_tracks[right_fist].push_back(4);
			limb_tracks[right_fist].push_back(5);
			limb_tracks[right_foot].push_back(7);
			limb_tracks[right_foot].push_back(6);
			break;
		DEFAULT_FAIL(out_type);
	}
	// prev_limb_panels keeps track of which panel in the track list the limb
	// hit last.
	vector<size_t> prev_limb_panels(num_kickbox_limbs, 0);
	vector<int> panel_repeat_counts(num_kickbox_limbs, 0);
	vector<int> panel_repeat_goals(num_kickbox_limbs, 0);
	RandomGen rnd(nonrandom_seed);
	kickbox_limb prev_limb_used= invalid_limb;
	// Kicks are only allowed if there is enough setup/recovery time.
	float kick_recover_time= GAMESTATE->GetAutoGenFarg(0);
	if(kick_recover_time <= 0.0f)
	{
		kick_recover_time= .25f;
	}
	float prev_note_time= -1.0f;
	int rows_done= 0;
#define RAND_FIST ((rnd() % 2) ? left_fist : right_fist)
#define RAND_FOOT ((rnd() % 2) ? left_foot : right_foot)
	FOREACH_NONEMPTY_ROW_ALL_TRACKS(in, r)
	{
		// Arbitrary:  Drop everything except taps and hold heads out entirely,
		// convert holds to tap just the head.
		bool has_valid_tapnote= false;
		for(int t= 0; t < in.GetNumTracks(); ++t)
		{
			const TapNote& tn= in.GetTapNote(t, r);
			if(tn.type == TapNoteType_Tap || tn.type == TapNoteType_HoldHead)
			{
				has_valid_tapnote= true;
				break;
			}
		}
		if(!has_valid_tapnote) { continue; }
		int next_row= r;
		bool next_has_valid= false;
		while(!next_has_valid)
		{
			if(!in.GetNextTapNoteRowForAllTracks(next_row))
			{
				next_row= -1;
				next_has_valid= true;
			}
			else
			{
				for(int t= 0; t < in.GetNumTracks(); ++t)
				{
					const TapNote& tn= in.GetTapNote(t, next_row);
					if(tn.type == TapNoteType_Tap || tn.type == TapNoteType_HoldHead)
					{
						next_has_valid= true;
						break;
					}
				}
			}
		}
		float this_note_time= timing.GetElapsedTimeFromBeat(NoteRowToBeat(r));
		float next_note_time= timing.GetElapsedTimeFromBeat(NoteRowToBeat(next_row));
		kickbox_limb this_limb= invalid_limb;
		switch(prev_limb_used)
		{
			case invalid_limb:
				// First limb is arbitrarily always a fist.
				this_limb= RAND_FIST;
				break;
			case left_foot:
			case right_foot:
				// Multiple kicks in a row are allowed if they're on the same foot.
				// Allow the last note to be a kick.
				// Switch feet if there's enough time.
				if(next_note_time - this_note_time > kick_recover_time * 2.0f)
				{
					this_limb= prev_limb_used == left_foot ? right_foot : left_foot;
				}
				else if((next_note_time - this_note_time > kick_recover_time * .5f ||
						next_note_time < 0.0f) && (rnd() % 2))
				{
					this_limb= prev_limb_used;
				}
				else
				{
					this_limb= RAND_FIST;
				}
				break;
			case left_fist:
			case right_fist:
				if(this_note_time - prev_note_time > kick_recover_time &&
					(next_note_time - this_note_time > kick_recover_time ||
						next_note_time < 0.0f) && have_feet)
				{
					this_limb= RAND_FOOT;
				}
				else
				{
					// Alternate fists.
					this_limb= prev_limb_used == left_fist ? right_fist : left_fist;
				}
				break;
			default:
				break;
		}
		size_t this_panel= prev_limb_panels[this_limb];
		if(panel_repeat_counts[this_limb] + 1 > panel_repeat_goals[this_limb])
		{
			// Use a different panel.
			this_panel= (this_panel + 1) % limb_tracks[this_limb].size();
			panel_repeat_counts[this_limb]= 0;
			panel_repeat_goals[this_limb]= (rnd() % 8) + 1;
		}
		out.SetTapNote(limb_tracks[this_limb][this_panel], r, TAP_ORIGINAL_TAP);
		++panel_repeat_counts[this_limb];
		prev_limb_panels[this_limb]= this_panel;
		prev_note_time= this_note_time;
		prev_limb_used= this_limb;
		++rows_done;
	}
	out.RevalidateATIs(vector<int>(), false);
}

struct recent_note
{
	int row;
	int track;
	recent_note()
		:row(0), track(0) {}
	recent_note(int r, int t)
		:row(r), track(t) {}
};

// CalculateRadarValues has to delay some stuff until a row ends, but can
// only detect a row ending when it hits the next note.  There isn't a note
// after the last row, so it also has to do the delayed stuff after exiting
// its loop.  So this state structure exists to be passed to a function that
// can be called from both places to do the work.  If this were Lua,
// DoRowEndRadarCalc would be a nested function. -Kyz
struct crv_state
{
	bool judgable;
	// hold_ends tracks where currently active holds will end, which is used
	// to count the number of hands. -Kyz
	vector<int> hold_ends;
	// num_holds_on_curr_row saves us the work of tracking where holds started
	// just to keep a jump of two holds from counting as a hand.
	int num_holds_on_curr_row;
	int num_notes_on_curr_row;

	crv_state()
		:judgable(false), num_holds_on_curr_row(0), num_notes_on_curr_row(0)
	{}
};

static void DoRowEndRadarCalc(crv_state& state, RadarValues& out)
{
	if(state.judgable)
	{
		if(state.num_notes_on_curr_row + (state.hold_ends.size() -
				state.num_holds_on_curr_row) >= 3)
		{
			++out[RadarCategory_Hands];
		}
	}
}

void NoteDataUtil::CalculateRadarValues( const NoteData &in, float fSongSeconds, RadarValues& out )
{
	// Anybody editing this function should also examine
	// NoteDataWithScoring::GetActualRadarValues to make sure it handles things
	// the same way.
	out.Zero();
	int curr_row= -1;
	// recent_notes is used to calculate the voltage.  Each element is the row
	// and track number of a tap note.  When the pair at the beginning is too
	// old, it's deleted.  This provides a way to have a rolling window
	// that scans for the peak step density. -Kyz
	vector<recent_note> recent_notes;
	NoteData::all_tracks_const_iterator curr_note=
		in.GetTapNoteRangeAllTracks(0, MAX_NOTE_ROW);
	TimingData* timing= GAMESTATE->GetProcessedTimingData();
	// total_taps exists because the stream calculation needs GetNumTapNotes,
	// but TapsAndHolds + Jumps + Hands would be inaccurate. -Kyz
	float total_taps= 0;
	const float voltage_window_beats= 8.0f;
	const int voltage_window= BeatToNoteRow(voltage_window_beats);
	size_t max_notes_in_voltage_window= 0;
	int num_chaos_rows= 0;
	crv_state state;

	while(!curr_note.IsAtEnd())
	{
		if(curr_note.Row() != curr_row)
		{
			DoRowEndRadarCalc(state, out);
			curr_row= curr_note.Row();
			state.num_notes_on_curr_row= 0;
			state.num_holds_on_curr_row= 0;
			state.judgable= timing->IsJudgableAtRow(curr_row);
			for(size_t n= 0; n < state.hold_ends.size(); ++n)
			{
				if(state.hold_ends[n] < curr_row)
				{
					state.hold_ends.erase(state.hold_ends.begin() + n);
					--n;
				}
			}
			for(size_t n= 0; n < recent_notes.size(); ++n)
			{
				if(recent_notes[n].row < curr_row - voltage_window)
				{
					recent_notes.erase(recent_notes.begin() + n);
					--n;
				}
				else
				{
					// recent_notes is kept sorted, so reaching the first note that
					// isn't old enough to remove means we're finished. -Kyz
					break;
				}
			}
			// GetChaosRadarValue did not care about whether a row is judgable.
			// So chaos is checked here. -Kyz
			if(GetNoteType(curr_row) >= NOTE_TYPE_12TH)
			{
				++num_chaos_rows;
			}
		}
		if(state.judgable)
		{
			switch(curr_note->type)
			{
				case TapNoteType_Tap:
				case TapNoteType_HoldHead:
					// Lifts have to be counted with taps for them to be added to max dp
					// correctly. -Kyz
				case TapNoteType_Lift:
					// HoldTails and Attacks are counted by IsTap.  But it doesn't
					// make sense to count HoldTails as hittable notes. -Kyz
				case TapNoteType_Attack:
					++out[RadarCategory_Notes];
					++state.num_notes_on_curr_row;
					++total_taps;
					recent_notes.push_back(
						recent_note(curr_row, curr_note.Track()));
					max_notes_in_voltage_window= max(recent_notes.size(),
						max_notes_in_voltage_window);
					// If there is one hold active, and one tap on this row, it does
					// not count as a jump.  Hands do need to count the number of
					// holds active though. -Kyz
					switch(state.num_notes_on_curr_row)
					{
						case 1:
							++out[RadarCategory_TapsAndHolds];
							break;
						case 2:
							++out[RadarCategory_Jumps];
							break;
						default:
							break;
					}
					if(curr_note->type == TapNoteType_HoldHead)
					{
						state.hold_ends.push_back(curr_row + curr_note->iDuration);
						++state.num_holds_on_curr_row;
						switch(curr_note->subType)
						{
							case TapNoteSubType_Hold:
								++out[RadarCategory_Holds];
								break;
							case TapNoteSubType_Roll:
								++out[RadarCategory_Rolls];
								break;
							default:
								break;
						}
					}
					else if(curr_note->type == TapNoteType_Lift)
					{
						++out[RadarCategory_Lifts];
					}
					break;
				case TapNoteType_Mine:
					++out[RadarCategory_Mines];
					break;
				case TapNoteType_Fake:
					++out[RadarCategory_Fakes];
					break;
				default:
					break;
			}
		}
		else
		{
			++out[RadarCategory_Fakes];
		}
		++curr_note;
	}
	DoRowEndRadarCalc(state, out);

	// Walking the notes complete, now assign any values that remain. -Kyz
	if(fSongSeconds > 0.0f)
	{
		out[RadarCategory_Stream]= (total_taps / fSongSeconds) / 7.0f;
		// As seen in GetVoltageRadarValue:  Don't use the timing data, just
		// pretend the beats are evenly spaced. -Kyz
		float avg_bps= in.GetLastBeat() / fSongSeconds;
		out[RadarCategory_Voltage]=
			((max_notes_in_voltage_window / voltage_window_beats) * avg_bps) /
			10.0f;
		out[RadarCategory_Air]= out[RadarCategory_Jumps] / fSongSeconds;
		out[RadarCategory_Freeze]= out[RadarCategory_Holds] / fSongSeconds;
		out[RadarCategory_Chaos]= num_chaos_rows / fSongSeconds * .5f;
	}
	// Sorry, there's not an assert here anymore for making sure all fields
	// are set.  There's a comment in the RadarCategory enum to direct
	// attention here when adding new categories. -Kyz
}

void NoteDataUtil::RemoveHoldNotes( NoteData &in, int iStartIndex, int iEndIndex )
{
	// turn all the HoldNotes into TapNotes
	for( int t=0; t<in.GetNumTracks(); ++t )
	{
		NoteData::TrackMap::iterator begin, end;
		in.GetTapNoteRangeInclusive( t, iStartIndex, iEndIndex, begin, end );
		for( ; begin != end; ++begin )
		{
			if( begin->second.type != TapNoteType_HoldHead ||
				begin->second.subType != TapNoteSubType_Hold )
				continue;
			begin->second.type = TapNoteType_Tap;
		}
	}
	in.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::ChangeRollsToHolds( NoteData &in, int iStartIndex, int iEndIndex )
{
	for( int t=0; t<in.GetNumTracks(); ++t )
	{
		NoteData::TrackMap::iterator begin, end;
		in.GetTapNoteRangeInclusive( t, iStartIndex, iEndIndex, begin, end );
		for( ; begin != end; ++begin )
		{
			if( begin->second.type != TapNoteType_HoldHead ||
				begin->second.subType != TapNoteSubType_Roll )
				continue;
			begin->second.subType = TapNoteSubType_Hold;
		}
	}
	in.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::ChangeHoldsToRolls( NoteData &in, int iStartIndex, int iEndIndex )
{
	for( int t=0; t<in.GetNumTracks(); ++t )
	{
		NoteData::TrackMap::iterator begin, end;
		in.GetTapNoteRangeInclusive( t, iStartIndex, iEndIndex, begin, end );
		for( ; begin != end; ++begin )
		{
			if( begin->second.type != TapNoteType_HoldHead ||
				begin->second.subType != TapNoteSubType_Hold )
				continue;
			begin->second.subType = TapNoteSubType_Roll;
		}
	}
	in.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::RemoveSimultaneousNotes( NoteData &in, int iMaxSimultaneous, int iStartIndex, int iEndIndex )
{
	// Remove tap and hold notes so no more than iMaxSimultaneous buttons are being held at any
	// given time.  Never touch data outside of the range given; if many hold notes are overlapping
	// iStartIndex, and we'd have to change those holds to obey iMaxSimultaneous, just do the best
	// we can without doing so.
	if( in.IsComposite() )
	{
		// Do this per part.
		vector<NoteData> vParts;
		
		SplitCompositeNoteData( in, vParts );
		FOREACH( NoteData, vParts, nd )
			RemoveSimultaneousNotes( *nd, iMaxSimultaneous, iStartIndex, iEndIndex );
		in.Init();
		in.SetNumTracks( vParts.front().GetNumTracks() );
		CombineCompositeNoteData( in, vParts );
	}
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( in, r, iStartIndex, iEndIndex )
	{
		set<int> viTracksHeld;
		in.GetTracksHeldAtRow( r, viTracksHeld );

		// remove the first tap note or the first hold note that starts on this row
		int iTotalTracksPressed = in.GetNumTracksWithTapOrHoldHead(r) + viTracksHeld.size();
		int iTracksToRemove = max( 0, iTotalTracksPressed - iMaxSimultaneous );
		for( int t=0; iTracksToRemove>0 && t<in.GetNumTracks(); t++ )
		{
			const TapNote &tn = in.GetTapNote(t,r);
			if( tn.type == TapNoteType_Tap || tn.type == TapNoteType_HoldHead )
			{
				in.SetTapNote( t, r, TAP_EMPTY );
				iTracksToRemove--;
			}
		}
	}
	in.RevalidateATIs(vector<int>(), false);
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

void NoteDataUtil::RemoveSpecificTapNotes(NoteData &inout, TapNoteType tn, int iStartIndex, int iEndIndex)
{
	for(int t=0; t<inout.GetNumTracks(); t++)
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE(inout, t, r, iStartIndex, iEndIndex)
		{
			if(inout.GetTapNote(t,r).type == tn)
			{
				inout.SetTapNote( t, r, TAP_EMPTY );
			}
		}
	}
	inout.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::RemoveMines(NoteData &inout, int iStartIndex, int iEndIndex)
{
	RemoveSpecificTapNotes(inout, TapNoteType_Mine, iStartIndex, iEndIndex);
}

void NoteDataUtil::RemoveLifts(NoteData &inout, int iStartIndex, int iEndIndex)
{
	RemoveSpecificTapNotes(inout, TapNoteType_Lift, iStartIndex, iEndIndex);
}

void NoteDataUtil::RemoveFakes(NoteData &inout, TimingData const& timing_data, int iStartIndex, int iEndIndex)
{
	RemoveSpecificTapNotes(inout, TapNoteType_Fake, iStartIndex, iEndIndex);
	for(int t=0; t<inout.GetNumTracks(); t++)
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE(inout, t, r, iStartIndex, iEndIndex)
		{
			if(!timing_data.IsJudgableAtRow(r))
			{
				inout.SetTapNote( t, r, TAP_EMPTY );
			}
		}
	}
	inout.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::RemoveAllButOneTap( NoteData &inout, int row )
{
	if(row < 0) return;

	int track;
	for( track = 0; track < inout.GetNumTracks(); ++track )
	{
		if( inout.GetTapNote(track, row).type == TapNoteType_Tap )
			break;
	}

	track++;

	for( ; track < inout.GetNumTracks(); ++track )
	{
		NoteData::iterator iter = inout.FindTapNote( track, row );
		if( iter != inout.end(track) && iter->second.type == TapNoteType_Tap )
			inout.RemoveTapNote( track, iter );
	}
	inout.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::RemoveAllButPlayer( NoteData &inout, PlayerNumber pn )
{
	for( int track = 0; track < inout.GetNumTracks(); ++track )
	{
		NoteData::iterator i = inout.begin( track );
		
		while( i != inout.end(track) )
		{
			if( i->second.pn != pn && i->second.pn != PLAYER_INVALID )
				inout.RemoveTapNote( track, i++ );
			else
				++i;
		}
	}
	inout.RevalidateATIs(vector<int>(), false);
}

// TODO: Perform appropriate matrix calculations for everything instead.
static void GetTrackMapping( StepsType st, NoteDataUtil::TrackMapping tt, int NumTracks, int *iTakeFromTrack )
{
	// Identity transform for cases not handled below.
	for( int t = 0; t < MAX_NOTE_TRACKS; ++t )
		iTakeFromTrack[t] = t;

	switch( tt )
	{
	case NoteDataUtil::left:
	case NoteDataUtil::right:
		// Is there a way to do this without handling each StepsType? -Chris
		switch( st )
		{
		case StepsType_dance_single:
		case StepsType_dance_double:
		case StepsType_dance_couple:
		case StepsType_dance_routine:
			iTakeFromTrack[0] = 2;
			iTakeFromTrack[1] = 0;
			iTakeFromTrack[2] = 3;
			iTakeFromTrack[3] = 1;
			iTakeFromTrack[4] = 6;
			iTakeFromTrack[5] = 4;
			iTakeFromTrack[6] = 7;
			iTakeFromTrack[7] = 5;
			break;
		case StepsType_dance_solo:
			iTakeFromTrack[0] = 5;
			iTakeFromTrack[1] = 4;
			iTakeFromTrack[2] = 0;
			iTakeFromTrack[3] = 3;
			iTakeFromTrack[4] = 1;
			iTakeFromTrack[5] = 2;
			break;
		case StepsType_pump_single:
		case StepsType_pump_couple:
			iTakeFromTrack[0] = 1;
			iTakeFromTrack[1] = 3;
			iTakeFromTrack[2] = 2;
			iTakeFromTrack[3] = 4;
			iTakeFromTrack[4] = 0;
			iTakeFromTrack[5] = 6;
			iTakeFromTrack[6] = 8;
			iTakeFromTrack[7] = 7;
			iTakeFromTrack[8] = 9;
			iTakeFromTrack[9] = 5;
			break;
		case StepsType_pump_halfdouble:
			iTakeFromTrack[0] = 2;
			iTakeFromTrack[1] = 0;
			iTakeFromTrack[2] = 1;
			iTakeFromTrack[3] = 3;
			iTakeFromTrack[4] = 4;
			iTakeFromTrack[5] = 5;
			break;
		case StepsType_pump_double:
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
			// Invert.
			int iTrack[MAX_NOTE_TRACKS];
			memcpy( iTrack, iTakeFromTrack, sizeof(iTrack) );
			for( int t = 0; t < MAX_NOTE_TRACKS; ++t )
			{
				const int to = iTrack[t];
				iTakeFromTrack[to] = t;
			}
		}

		break;
	case NoteDataUtil::backwards:
	{
		// If a Pump game type, treat differently. Otherwise, send to mirror.
		bool needsBackwards = true;
		switch (st)
		{
			case StepsType_pump_single:
			case StepsType_pump_couple:
			{
				iTakeFromTrack[0] = 3;
				iTakeFromTrack[1] = 4;
				iTakeFromTrack[2] = 2;
				iTakeFromTrack[3] = 0;
				iTakeFromTrack[4] = 1;
				iTakeFromTrack[5] = 8;
				iTakeFromTrack[6] = 9;
				iTakeFromTrack[7] = 2;
				iTakeFromTrack[8] = 5;
				iTakeFromTrack[9] = 6;
				break;
			}
			case StepsType_pump_double:
			case StepsType_pump_routine:
			{
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
			}
			case StepsType_pump_halfdouble:
			{
				iTakeFromTrack[0] = 5;
				iTakeFromTrack[1] = 3;
				iTakeFromTrack[2] = 4;
				iTakeFromTrack[3] = 1;
				iTakeFromTrack[4] = 2;
				iTakeFromTrack[5] = 0;
				break;
			}
			case StepsType_beat_single5:
			{
				// scratch is on right (cols 5 and 11) for 5-key
				iTakeFromTrack[0] = 4;
				iTakeFromTrack[1] = 3;
				iTakeFromTrack[2] = 2;
				iTakeFromTrack[3] = 1;
				iTakeFromTrack[4] = 0;
				iTakeFromTrack[5] = 5;
				iTakeFromTrack[6] = 10;
				iTakeFromTrack[7] = 9;
				iTakeFromTrack[8] = 8;
				iTakeFromTrack[9] = 7;
				iTakeFromTrack[10] = 6;
				iTakeFromTrack[11] = 11;
				break;
			}
			case StepsType_beat_single7:
			{
				// scratch is on left (cols 0 and 8) for 7-key
				iTakeFromTrack[0] = 0;
				iTakeFromTrack[1] = 7;
				iTakeFromTrack[2] = 6;
				iTakeFromTrack[3] = 5;
				iTakeFromTrack[4] = 4;
				iTakeFromTrack[5] = 3;
				iTakeFromTrack[6] = 2;
				iTakeFromTrack[7] = 1;
				iTakeFromTrack[8] = 8;
				iTakeFromTrack[9] = 15;
				iTakeFromTrack[10] = 14;
				iTakeFromTrack[11] = 13;
				iTakeFromTrack[12] = 12;
				iTakeFromTrack[13] = 11;
				iTakeFromTrack[14] = 10;
				iTakeFromTrack[15] = 9;
				break;
			}
			default:
				needsBackwards = false;
		}
		if (needsBackwards) break;
	}
	case NoteDataUtil::mirror:
		{
			for( int t=0; t<NumTracks; t++ )
				iTakeFromTrack[t] = NumTracks-t-1;
			break;
		}
	case NoteDataUtil::shuffle:
	case NoteDataUtil::super_shuffle:		// use shuffle code to mix up HoldNotes without creating impossible patterns
		{
			// TRICKY: Shuffle so that both player get the same shuffle mapping
			// in the same round. This is already achieved in beat mode.
			int iOrig[MAX_NOTE_TRACKS];
			memcpy( iOrig, iTakeFromTrack, sizeof(iOrig) );

			int iShuffleSeed = GAMESTATE->m_iStageSeed;
			do {
				RandomGen rnd( iShuffleSeed );
				// ignore turntable in beat mode
				switch(st) {
					case StepsType_beat_single5:
					{
						random_shuffle( &iTakeFromTrack[0], &iTakeFromTrack[5], rnd );
						random_shuffle( &iTakeFromTrack[6], &iTakeFromTrack[11], rnd );
						break;
					}
					case StepsType_beat_single7:
					{
						random_shuffle( &iTakeFromTrack[1], &iTakeFromTrack[8], rnd );
						random_shuffle( &iTakeFromTrack[9], &iTakeFromTrack[16], rnd );
						break;
					}
					default:
					{
						random_shuffle( &iTakeFromTrack[0], &iTakeFromTrack[NumTracks], rnd );
						break;
					}
				}
				iShuffleSeed++;
			}
			while ( !memcmp( iOrig, iTakeFromTrack, sizeof(iOrig) ) ); // shuffle again if shuffle managed to shuffle them in the same order
		}
		break;
	case NoteDataUtil::soft_shuffle:
		{
			// XXX: this is still pretty much a stub.

			// soft shuffle, as described at
			// http://www.stepmania.com/forums/showthread.php?t=19469

			/* one of the following at random:
			 *
			 * 0. No columns changed
			 * 1. Left and right columns swapped
			 * 2. Down and up columns swapped
			 * 3. Mirror (left and right swapped, down and up swapped)
			 * ----------------------------------------------------------------
			 * To extend it to handle all game types, it would pick each axis
			 * of symmetry the game type has and either flip it or not flip it.
			 *
			 * For instance, PIU singles has four axes:
			 * horizontal, vertical,
			 * diagonally top left to bottom right,
			 * diagonally bottom left to top right.
			 * (above text from forums) */

			// TRICKY: Shuffle so that both player get the same shuffle mapping
			// in the same round.

			int iShuffleSeed = GAMESTATE->m_iStageSeed;
			RandomGen rnd( iShuffleSeed );
			int iRandChoice = (rnd() % 4);

			// XXX: cases 1 and 2 only implemented for dance_*
			switch( iRandChoice )
			{
				case 1: // left and right mirror
				case 2: // up and down mirror
					switch( st )
					{
					case StepsType_dance_single:
						if( iRandChoice == 1 )
						{
							// left and right
							iTakeFromTrack[0] = 3;
							iTakeFromTrack[3] = 0;
						}
						if( iRandChoice == 2 )
						{
							// up and down
							iTakeFromTrack[1] = 2;
							iTakeFromTrack[2] = 1;
						}
						break;
					case StepsType_dance_double:
					case StepsType_dance_couple:
					case StepsType_dance_routine:
						if( iRandChoice == 1 )
						{
							// left and right
							iTakeFromTrack[0] = 3;
							iTakeFromTrack[3] = 0;
							iTakeFromTrack[4] = 7;
							iTakeFromTrack[7] = 4;
						}
						if( iRandChoice == 2 )
						{
							// up and down
							iTakeFromTrack[1] = 2;
							iTakeFromTrack[2] = 1;
							iTakeFromTrack[5] = 6;
							iTakeFromTrack[6] = 5;
						}
						break;
					// here be dragons (unchanged code)
					case StepsType_dance_solo:
						iTakeFromTrack[0] = 5;
						iTakeFromTrack[1] = 4;
						iTakeFromTrack[2] = 0;
						iTakeFromTrack[3] = 3;
						iTakeFromTrack[4] = 1;
						iTakeFromTrack[5] = 2;
						break;
					case StepsType_pump_single:
					case StepsType_pump_couple:
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
					case StepsType_pump_halfdouble:
						iTakeFromTrack[0] = 2;
						iTakeFromTrack[1] = 0;
						iTakeFromTrack[2] = 1;
						iTakeFromTrack[3] = 3;
						iTakeFromTrack[4] = 4;
						iTakeFromTrack[5] = 5;
						break;
					case StepsType_pump_double:
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
					case StepsType_kickbox_human:
						if(iRandChoice == 1)
						{
							iTakeFromTrack[0]= 3;
							iTakeFromTrack[3]= 0;
						}
						if(iRandChoice == 2)
						{
							iTakeFromTrack[1]= 2;
							iTakeFromTrack[2]= 1;
						}
						break;
					case StepsType_kickbox_quadarm:
						if(iRandChoice == 1)
						{
							iTakeFromTrack[0]= 1;
							iTakeFromTrack[1]= 0;
							iTakeFromTrack[2]= 3;
							iTakeFromTrack[3]= 2;
						}
						if(iRandChoice == 2)
						{
							iTakeFromTrack[1]= 2;
							iTakeFromTrack[2]= 1;
						}
						break;
					case StepsType_kickbox_insect:
						if(iRandChoice == 1)
						{
							iTakeFromTrack[1]= 2;
							iTakeFromTrack[2]= 1;
							iTakeFromTrack[3]= 4;
							iTakeFromTrack[4]= 3;
						}
						if(iRandChoice == 2)
						{
							iTakeFromTrack[1]= 4;
							iTakeFromTrack[2]= 3;
							iTakeFromTrack[3]= 2;
							iTakeFromTrack[4]= 1;
						}
						break;
					case StepsType_kickbox_arachnid:
						if(iRandChoice == 1)
						{
							iTakeFromTrack[0]= 1;
							iTakeFromTrack[1]= 0;
							iTakeFromTrack[2]= 3;
							iTakeFromTrack[3]= 2;
							iTakeFromTrack[4]= 5;
							iTakeFromTrack[5]= 4;
							iTakeFromTrack[6]= 7;
							iTakeFromTrack[7]= 6;
						}
						if(iRandChoice == 2)
						{
							iTakeFromTrack[0]= 6;
							iTakeFromTrack[1]= 7;
							iTakeFromTrack[2]= 4;
							iTakeFromTrack[3]= 5;
							iTakeFromTrack[4]= 2;
							iTakeFromTrack[5]= 3;
							iTakeFromTrack[6]= 0;
							iTakeFromTrack[7]= 1;
						}
						break;
					default: break;
					}
					break;
				case 3: // full mirror
					GetTrackMapping( st, NoteDataUtil::mirror, NumTracks, iTakeFromTrack );
					break;
				case 0:
				default:
					// case 0 and default are set by identity matrix above
					break;
			}
		}
		break;
	case NoteDataUtil::stomp:
		switch( st )
		{
		case StepsType_dance_single:
		case StepsType_dance_couple:
			iTakeFromTrack[0] = 3;
			iTakeFromTrack[1] = 2;
			iTakeFromTrack[2] = 1;
			iTakeFromTrack[3] = 0;
			iTakeFromTrack[4] = 7;
			iTakeFromTrack[5] = 6;
			iTakeFromTrack[6] = 5;
			iTakeFromTrack[7] = 4;
			break;
		case StepsType_dance_double:
		case StepsType_dance_routine:
			iTakeFromTrack[0] = 1;
			iTakeFromTrack[1] = 0;
			iTakeFromTrack[2] = 3;
			iTakeFromTrack[3] = 2;
			iTakeFromTrack[4] = 5;
			iTakeFromTrack[5] = 4;
			iTakeFromTrack[6] = 7;
			iTakeFromTrack[7] = 6;
			break;
		case StepsType_kickbox_human:
			iTakeFromTrack[0]= 1;
			iTakeFromTrack[1]= 0;
			iTakeFromTrack[2]= 3;
			iTakeFromTrack[3]= 2;
			break;
		case StepsType_kickbox_quadarm:
			iTakeFromTrack[0]= 1;
			iTakeFromTrack[1]= 0;
			iTakeFromTrack[2]= 3;
			iTakeFromTrack[3]= 2;
			break;
		case StepsType_kickbox_insect:
			iTakeFromTrack[0]= 1;
			iTakeFromTrack[1]= 4;
			iTakeFromTrack[2]= 3;
			iTakeFromTrack[3]= 2;
			iTakeFromTrack[4]= 1;
			iTakeFromTrack[5]= 4;
			break;
		case StepsType_kickbox_arachnid:
			iTakeFromTrack[0]= 5;
			iTakeFromTrack[1]= 4;
			iTakeFromTrack[2]= 5;
			iTakeFromTrack[3]= 4;
			iTakeFromTrack[4]= 3;
			iTakeFromTrack[5]= 2;
			iTakeFromTrack[6]= 1;
			iTakeFromTrack[7]= 0;
			break;
		default: 
			break;
		}
		break;
	case NoteDataUtil::swap_up_down:
		switch(st)
		{
			case StepsType_dance_single:
			case StepsType_dance_double:
			case StepsType_dance_couple:
			case StepsType_dance_routine:
				iTakeFromTrack[0]= 0;
				iTakeFromTrack[1]= 2;
				iTakeFromTrack[2]= 1;
				iTakeFromTrack[3]= 3;
				iTakeFromTrack[4]= 4;
				iTakeFromTrack[5]= 6;
				iTakeFromTrack[6]= 5;
				iTakeFromTrack[7]= 7;
				break;
			case StepsType_pump_single:
			case StepsType_pump_double:
			case StepsType_pump_couple:
			case StepsType_pump_routine:
				iTakeFromTrack[0]= 1;
				iTakeFromTrack[1]= 0;
				iTakeFromTrack[2]= 2;
				iTakeFromTrack[3]= 4;
				iTakeFromTrack[4]= 3;
				iTakeFromTrack[5]= 6;
				iTakeFromTrack[6]= 5;
				iTakeFromTrack[7]= 7;
				iTakeFromTrack[8]= 9;
				iTakeFromTrack[9]= 8;
				break;
			case StepsType_pump_halfdouble:
				iTakeFromTrack[0]= 0;
				iTakeFromTrack[1]= 2;
				iTakeFromTrack[2]= 1;
				iTakeFromTrack[3]= 4;
				iTakeFromTrack[4]= 3;
				iTakeFromTrack[5]= 5;
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
			const TapNote &tn1 = inout.GetTapNote( t1, r );
			switch( tn1.type )
			{
			case TapNoteType_Empty:
			case TapNoteType_HoldHead:
			case TapNoteType_HoldTail:
			case TapNoteType_AutoKeysound:
				continue;	// skip
			case TapNoteType_Tap:
			case TapNoteType_Mine:
			case TapNoteType_Attack:
			case TapNoteType_Lift:
			case TapNoteType_Fake:
				break;	// shuffle this
			DEFAULT_FAIL( tn1.type );
			}

			DEBUG_ASSERT_M( !inout.IsHoldNoteAtRow(t1,r), ssprintf("There is a tap.type = %d inside of a hold at row %d", tn1.type, r) );

			// Probe for a spot to swap with.
			set<int> vTriedTracks;
			for( int i=0; i<4; i++ )	// probe max 4 times
			{
				int t2 = RandomInt( inout.GetNumTracks() );
				if( vTriedTracks.find(t2) != vTriedTracks.end() )	// already tried this track
					continue;	// skip
				vTriedTracks.insert( t2 );

				// swapping with ourself is a no-op
				if( t1 == t2 )
					break;	// done swapping

				const TapNote &tn2 = inout.GetTapNote( t2, r );
				switch( tn2.type )
				{
				case TapNoteType_HoldHead:
				case TapNoteType_HoldTail:
				case TapNoteType_AutoKeysound:
					continue;	// don't swap with these
				case TapNoteType_Empty:
				case TapNoteType_Tap:
				case TapNoteType_Mine:
				case TapNoteType_Attack:
				case TapNoteType_Lift:
				case TapNoteType_Fake:
					break;	// ok to swap with this
				DEFAULT_FAIL( tn2.type );
				}

				// don't swap into the middle of a hold note
				if( inout.IsHoldNoteAtRow( t2,r ) )
					continue;

				// do the swap
				const TapNote tnTemp = tn1;
				inout.SetTapNote( t1, r, tn2 );
				inout.SetTapNote( t2, r, tnTemp );
				
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
	inout.RevalidateATIs(vector<int>(), false);
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

			const TapNote &tnEarlier = inout.GetTapNote( t, iRowEarlier );
			if( tnEarlier.type == TapNoteType_HoldHead )
				iRowLater -= tnEarlier.iDuration;

			out.SetTapNote( t, iRowLater, tnEarlier );
		}
	}

	inout.swap( out );
	inout.RevalidateATIs(vector<int>(), false);
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
	inout.RevalidateATIs(vector<int>(), false);
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
	inout.RevalidateATIs(vector<int>(), false);
}

// Make all quarter notes into jumps.
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
			if( inout.IsHoldNoteAtRow(t, i) )
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
		int iBeat = lrintf( NoteRowToBeat(i) );
		int iTrackOfNote = inout.GetFirstTrackWithTap(i);
		int iTrackToAdd = iTrackOfNote + (iBeat%5)-2;	// won't be more than 2 tracks away from the existing note
		CLAMP( iTrackToAdd, 0, inout.GetNumTracks()-1 );
		if( iTrackToAdd == iTrackOfNote )
			iTrackToAdd++;
		CLAMP( iTrackToAdd, 0, inout.GetNumTracks()-1 );
		if( iTrackToAdd == iTrackOfNote )
			iTrackToAdd--;
		CLAMP( iTrackToAdd, 0, inout.GetNumTracks()-1 );

		if( inout.GetTapNote(iTrackToAdd, i).type != TapNoteType_Empty  &&  inout.GetTapNote(iTrackToAdd, i).type != TapNoteType_Fake )
		{
			iTrackToAdd = (iTrackToAdd+1) % inout.GetNumTracks();
		}
		inout.SetTapNote(iTrackToAdd, i, TAP_ADDITION_TAP);
	}
	inout.RevalidateATIs(vector<int>(), false);
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
			if( inout.IsHoldNoteAtRow(t, iRowEarlier+1) )
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
			iTrackOfNoteEarlier != iTrackOfNoteLater &&   // Don't make skips on the same note
			bEarlierHasNonEmptyTrack )
		{
			iTrackOfNoteToAdd = iTrackOfNoteEarlier;
		}
		else if( abs(iTrackOfNoteEarlier-iTrackOfNoteLater) >= 2 )
		{
			// try to choose a track between the earlier and later notes
			iTrackOfNoteToAdd = min(iTrackOfNoteEarlier,iTrackOfNoteLater)+1;
		}
		else if( min(iTrackOfNoteEarlier,iTrackOfNoteLater)-1 >= 0 )
		{
			// try to choose a track just to the left
			iTrackOfNoteToAdd = min(iTrackOfNoteEarlier,iTrackOfNoteLater)-1;
		}
		else if( max(iTrackOfNoteEarlier,iTrackOfNoteLater)+1 < inout.GetNumTracks() )
		{
			// try to choose a track just to the right
			iTrackOfNoteToAdd = max(iTrackOfNoteEarlier,iTrackOfNoteLater)+1;
		}

		inout.SetTapNote(iTrackOfNoteToAdd, iRowToAdd, TAP_ADDITION_TAP);
	}
	inout.RevalidateATIs(vector<int>(), false);
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
		if( m_Iterator == XXX )
			;

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
	// Change whole rows at a time to be tap notes.  Otherwise, it causes
	// major problems for our scoring system. -Chris

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
				if( inout.GetTapNote(t,r).type == TapNoteType_Tap )
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
			if( tn.type != TapNoteType_HoldHead )
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
				if( inout.GetTapNote(t,iMineRow).type == TapNoteType_Tap )
					inout.SetTapNote(t,iMineRow,TAP_ADDITION_MINE);

			iRowCount = 0;
		}
	}
	inout.RevalidateATIs(vector<int>(), false);
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
	inout.RevalidateATIs(vector<int>(), false);
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

			if( inout.GetTapNote(t,r).type == TapNoteType_Tap )
			{
				// Find the ending row for this hold
				int iTapsLeft = iSimultaneousHolds;

				int r2 = r+1;
				bool addHold = true;
				FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, next_row, r+1, iEndIndex )
				{
					r2 = next_row;

					// If there are two taps in a row on the same track, 
					// don't convert the earlier one to a hold.
					if( inout.GetTapNote(t,r2).type != TapNoteType_Empty )
					{
						addHold = false;
						break;
					}

					set<int> tracksDown;
					inout.GetTracksHeldAtRow( r2, tracksDown );
					inout.GetTapNonEmptyTracks( r2, tracksDown );
					iTapsLeft -= tracksDown.size();
					if( iTapsLeft == 0 )
						break;	// we found the ending row for this hold
					else if( iTapsLeft < 0 )
					{
						addHold = false;
						break;
					}
				}

				if (!addHold)
				{
					continue;
				}

				// If the steps end in a tap, convert that tap
				// to a hold that lasts for at least one beat.
				if( r2 == r+1 )
					r2 = r+BeatToNoteRow(1);

				inout.AddHoldNote( t, r, r2, TAP_ORIGINAL_HOLD_HEAD );
				iTrackAddedThisRow++;
			}
		}
	}
	inout.RevalidateATIs(vector<int>(), false);
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
			if( inout.GetTapNote(t, r).type == TapNoteType_Tap )	// there is a tap here
			{
				// Look to see if there is enough empty space on either side of the note
				// to turn this into a jump.
				int iRowWindowBegin = r - BeatToNoteRow(0.5f);
				int iRowWindowEnd = r + BeatToNoteRow(0.5f);

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
	inout.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::SnapToNearestNoteType( NoteData &inout, NoteType nt1, NoteType nt2, int iStartIndex, int iEndIndex )
{
	// nt2 is optional and should be NoteType_Invalid if it is not used

	float fSnapInterval1 = NoteTypeToBeat( nt1 );
	float fSnapInterval2 = 10000; // nothing will ever snap to this.  That's what we want!
	if( nt2 != NoteType_Invalid )
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
			if( tnNew.type == TapNoteType_Empty )
				continue;

			inout.SetTapNote(c, iOldIndex, TAP_EMPTY);

			if( tnNew.type == TapNoteType_Tap && inout.IsHoldNoteAtRow(c, iNewIndex) )
				continue; // HoldNotes override TapNotes

			if( tnNew.type == TapNoteType_HoldHead )
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
	inout.RevalidateATIs(vector<int>(), false);
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
		for( int t=1; t<inout.GetNumTracks(); t++ )
		{
			NoteData::iterator iter = inout.FindTapNote( t, r );
			if( iter == inout.end(t) )
				continue;
			inout.SetTapNote( 0, r, iter->second );
			inout.RemoveTapNote( t, iter );
		}
}

void NoteDataUtil::CollapseLeft( NoteData &inout )
{
	FOREACH_NONEMPTY_ROW_ALL_TRACKS( inout, r )
	{
		int iNumTracksFilled = 0;
		for( int t=0; t<inout.GetNumTracks(); t++ )
		{
			if( inout.GetTapNote(t,r).type != TapNoteType_Empty )
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

void NoteDataUtil::SwapUpDown(NoteData& inout, StepsType st)
{
	int TakeFrom[MAX_NOTE_TRACKS];
	GetTrackMapping(st, NoteDataUtil::swap_up_down, inout.GetNumTracks(), TakeFrom);
	NoteData tempND;
	tempND.LoadTransformed(inout, inout.GetNumTracks(), TakeFrom);
	inout.CopyAll(tempND);
	inout.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::ArbitraryRemap(NoteData& inout, int* mapping)
{
	NoteData tempND;
	tempND.LoadTransformed(inout, inout.GetNumTracks(), mapping);
	inout.CopyAll(tempND);
	inout.RevalidateATIs(vector<int>(), false);
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
	{ StepsType_dance_double, { T,T,T,T,f,f,f,f } },
	{ StepsType_dance_double, { f,T,T,T,T,f,f,f } },
	{ StepsType_dance_double, { f,f,f,T,T,T,T,f } },
	{ StepsType_dance_double, { f,f,f,f,T,T,T,T } },
	{ StepsType_pump_double, { T,T,T,T,T,f,f,f,f,f } },
	{ StepsType_pump_double, { f,f,T,T,T,T,T,T,f,f } },
	{ StepsType_pump_double, { f,f,f,f,f,T,T,T,T,T } },
};
#undef T
#undef f

void NoteDataUtil::RemoveStretch( NoteData &inout, StepsType st, int iStartIndex, int iEndIndex )
{
	vector<const ValidRow*> vpValidRowsToCheck;
	for( unsigned i=0; i<ARRAYLEN(g_ValidRows); i++ )
	{
		if( g_ValidRows[i].st == st )
			vpValidRowsToCheck.push_back( &g_ValidRows[i] );
	}

	// bail early if there's nothing to validate against
	if( vpValidRowsToCheck.empty() )
		return;

	// each row must pass at least one valid mask
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, r, iStartIndex, iEndIndex )
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
	inout.RevalidateATIs(vector<int>(), false);
}

bool NoteDataUtil::RowPassesValidMask( NoteData &inout, int row, const bool bValidMask[] )
{
	for( int t=0; t<inout.GetNumTracks(); t++ )
	{
		if( !bValidMask[t] && inout.GetTapNote(t,row).type != TapNoteType_Empty )
			return false;
	}

	return true;
}

void NoteDataUtil::ConvertAdditionsToRegular( NoteData &inout )
{
	for( int t=0; t<inout.GetNumTracks(); t++ )
		FOREACH_NONEMPTY_ROW_IN_TRACK( inout, t, r )
			if( inout.GetTapNote(t,r).source == TapNoteSource_Addition )
			{
				TapNote tn = inout.GetTapNote(t,r);
				tn.source = TapNoteSource_Original;
				inout.SetTapNote( t, r, tn );
			}
	inout.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::TransformNoteData(NoteData &nd, TimingData const& timing_data, const AttackArray &aa, StepsType st, Song* pSong)
{
	FOREACH_CONST( Attack, aa, a )
	{
		PlayerOptions po;
		po.FromString( a->sModifiers );
		if( po.ContainsTransformOrTurn() )
		{
			float fStartBeat, fEndBeat;
			a->GetAttackBeats( pSong, fStartBeat, fEndBeat );

			NoteDataUtil::TransformNoteData(nd, timing_data, po, st, BeatToNoteRow(fStartBeat), BeatToNoteRow(fEndBeat) );
		}
	}
}

void NoteDataUtil::TransformNoteData( NoteData &nd, TimingData const& timing_data, const PlayerOptions &po, StepsType st, int iStartIndex, int iEndIndex )
{
	// Apply remove transforms before others so that we don't go removing
	// notes we just inserted.  Apply TRANSFORM_NOROLLS before TRANSFORM_NOHOLDS,
	// since NOROLLS creates holds.
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_LITTLE] )		NoteDataUtil::Little( nd, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_NOROLLS] )	NoteDataUtil::ChangeRollsToHolds( nd, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_NOHOLDS] )	NoteDataUtil::RemoveHoldNotes( nd, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_NOMINES] )	NoteDataUtil::RemoveMines( nd, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_NOJUMPS] )	NoteDataUtil::RemoveJumps( nd, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_NOLIFTS] )	NoteDataUtil::RemoveLifts( nd, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_NOFAKES] )	NoteDataUtil::RemoveFakes( nd, timing_data, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_NOHANDS] )	NoteDataUtil::RemoveHands( nd, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_NOQUADS] )	NoteDataUtil::RemoveQuads( nd, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_NOSTRETCH] )	NoteDataUtil::RemoveStretch( nd, st, iStartIndex, iEndIndex );

	// Apply inserts.
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_BIG] )		NoteDataUtil::Big( nd, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_QUICK] )		NoteDataUtil::Quick( nd, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_BMRIZE] )		NoteDataUtil::BMRize( nd, iStartIndex, iEndIndex );

	// Skippy will still add taps to places that the other 
	// AddIntelligentTaps above won't.
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_SKIPPY] )		NoteDataUtil::Skippy( nd, iStartIndex, iEndIndex );

	// These aren't affects by the above inserts very much.
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_MINES] )		NoteDataUtil::AddMines( nd, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_ECHO] )		NoteDataUtil::Echo( nd, iStartIndex, iEndIndex );

	// Jump-adding transforms aren't much affected by additional taps.
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_WIDE] )		NoteDataUtil::Wide( nd, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_STOMP] )		NoteDataUtil::Stomp( nd, st, iStartIndex, iEndIndex );

	// Transforms that add holds go last.  If they went first, most tap-adding 
	// transforms wouldn't do anything because tap-adding transforms skip areas 
	// where there's a hold.
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_PLANTED] )	NoteDataUtil::Planted( nd, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_FLOORED] )	NoteDataUtil::Floored( nd, iStartIndex, iEndIndex );
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_TWISTER] )	NoteDataUtil::Twister( nd, iStartIndex, iEndIndex );

	// Do this here to turn any added holds into rolls
	if( po.m_bTransforms[PlayerOptions::TRANSFORM_HOLDROLLS] )	NoteDataUtil::ChangeHoldsToRolls( nd, iStartIndex, iEndIndex );

	// Apply turns and shuffles last so that they affect inserts.
	if( po.m_bTurns[PlayerOptions::TURN_MIRROR] )			NoteDataUtil::Turn( nd, st, NoteDataUtil::mirror, iStartIndex, iEndIndex );
	if( po.m_bTurns[PlayerOptions::TURN_BACKWARDS] )	NoteDataUtil::Turn( nd, st, NoteDataUtil::backwards, iStartIndex, iEndIndex );
	if( po.m_bTurns[PlayerOptions::TURN_LEFT] )			NoteDataUtil::Turn( nd, st, NoteDataUtil::left, iStartIndex, iEndIndex );
	if( po.m_bTurns[PlayerOptions::TURN_RIGHT] )			NoteDataUtil::Turn( nd, st, NoteDataUtil::right, iStartIndex, iEndIndex );
	if( po.m_bTurns[PlayerOptions::TURN_SHUFFLE] )			NoteDataUtil::Turn( nd, st, NoteDataUtil::shuffle, iStartIndex, iEndIndex );
	if( po.m_bTurns[PlayerOptions::TURN_SOFT_SHUFFLE] )			NoteDataUtil::Turn( nd, st, NoteDataUtil::soft_shuffle, iStartIndex, iEndIndex );
	if( po.m_bTurns[PlayerOptions::TURN_SUPER_SHUFFLE] )		NoteDataUtil::Turn( nd, st, NoteDataUtil::super_shuffle, iStartIndex, iEndIndex );

	nd.RevalidateATIs(vector<int>(), false);
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
		float fBeat = pSong->m_SongTiming.GetBeatFromElapsedTime( sec );
		int iBeat = (int)fBeat;
		int iTrack = iBeat % nd.GetNumTracks();	// deterministically calculates track
		TapNote tn(
			TapNoteType_Attack,
			TapNoteSubType_Invalid,
			TapNoteSource_Original, 
			szAttacks[RandomInt(ARRAYLEN(szAttacks))],
			15.0f, 
			-1 );
		nd.SetTapNote( iTrack, BeatToNoteRow(fBeat), tn );
	}
	nd.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::Scale( NoteData &nd, float fScale )
{
	ASSERT( fScale > 0 );
	
	NoteData ndOut;
	ndOut.SetNumTracks( nd.GetNumTracks() );
	
	for( int t=0; t<nd.GetNumTracks(); t++ )
	{
		for( NoteData::const_iterator iter = nd.begin(t); iter != nd.end(t); ++iter )
		{
			TapNote tn = iter->second;
			int iNewRow      = lrintf( fScale * iter->first );
			int iNewDuration = lrintf( fScale * (iter->first + tn.iDuration) );
			tn.iDuration = iNewDuration;
			ndOut.SetTapNote( t, iNewRow, tn );
		}
	}
	
	nd.swap( ndOut );
	nd.RevalidateATIs(vector<int>(), false);
}

/* XXX: move this to an appropriate place, same place as NoteRowToBeat perhaps? */
static inline int GetScaledRow( float fScale, int iStartIndex, int iEndIndex, int iRow )
{
	if( iRow < iStartIndex )
		return iRow;
	else if( iRow > iEndIndex )
		return iRow + lrintf( (iEndIndex - iStartIndex) * (fScale - 1) );
	else
		return lrintf( (iRow - iStartIndex) * fScale ) + iStartIndex;
}

void NoteDataUtil::ScaleRegion( NoteData &nd, float fScale, int iStartIndex, int iEndIndex )
{
	ASSERT( fScale > 0 );
	ASSERT( iStartIndex < iEndIndex );
	ASSERT( iStartIndex >= 0 );
	
	NoteData ndOut;
	ndOut.SetNumTracks( nd.GetNumTracks() );
	
	for( int t=0; t<nd.GetNumTracks(); t++ )
	{
		for( NoteData::const_iterator iter = nd.begin(t); iter != nd.end(t); ++iter )
		{
			TapNote tn = iter->second;
			int iNewRow      = GetScaledRow( fScale, iStartIndex, iEndIndex, iter->first );
			int iNewDuration = GetScaledRow( fScale, iStartIndex, iEndIndex, iter->first + tn.iDuration ) - iNewRow;
			tn.iDuration = iNewDuration;
			ndOut.SetTapNote( t, iNewRow, tn );
		}
	}
	
	nd.swap( ndOut );
	nd.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::InsertRows( NoteData &nd, int iStartIndex, int iRowsToAdd )
{
	ASSERT( iRowsToAdd >= 0 );

	NoteData temp;
	temp.SetNumTracks( nd.GetNumTracks() );
	temp.CopyRange( nd, iStartIndex, MAX_NOTE_ROW );
	nd.ClearRange( iStartIndex, MAX_NOTE_ROW );
	nd.CopyRange( temp, 0, MAX_NOTE_ROW, iStartIndex + iRowsToAdd );		
	nd.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::DeleteRows( NoteData &nd, int iStartIndex, int iRowsToDelete )
{
	ASSERT( iRowsToDelete >= 0 );

	NoteData temp;
	temp.SetNumTracks( nd.GetNumTracks() );
	temp.CopyRange( nd, iStartIndex + iRowsToDelete, MAX_NOTE_ROW );
	nd.ClearRange( iStartIndex, MAX_NOTE_ROW );
	nd.CopyRange( temp, 0, MAX_NOTE_ROW, iStartIndex );		
	nd.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::RemoveAllTapsOfType( NoteData& ndInOut, TapNoteType typeToRemove )
{
	/* Be very careful when deleting the tap notes. Erasing elements from maps using
	 * iterators invalidates only the iterator that is being erased. To that end,
	 * increment the iterator before deleting the elment of the map.
	 */
	for( int t=0; t<ndInOut.GetNumTracks(); t++ )
	{
		for( NoteData::iterator iter = ndInOut.begin(t); iter != ndInOut.end(t); )
		{
			if( iter->second.type == typeToRemove )
				ndInOut.RemoveTapNote( t, iter++ );
			else
				++iter;
		}
	}
	ndInOut.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::RemoveAllTapsExceptForType( NoteData& ndInOut, TapNoteType typeToKeep )
{
	/* Same as in RemoveAllTapsOfType(). */
	for( int t=0; t<ndInOut.GetNumTracks(); t++ )
	{
		for( NoteData::iterator iter = ndInOut.begin(t); iter != ndInOut.end(t); )
		{
			if( iter->second.type != typeToKeep )
				ndInOut.RemoveTapNote( t, iter++ );
			else
				++iter;
		}
	}
	ndInOut.RevalidateATIs(vector<int>(), false);
}

int NoteDataUtil::GetMaxNonEmptyTrack( const NoteData& in )
{
	for( int t=in.GetNumTracks()-1; t>=0; t-- )
		if( !in.IsTrackEmpty( t ) )
			return t;
	return -1;
}

bool NoteDataUtil::AnyTapsAndHoldsInTrackRange( const NoteData& in, int iTrack, int iStart, int iEnd )
{
	if( iStart >= iEnd )
		return false;

	// for each index we crossed since the last update:
	FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( in, iTrack, r, iStart, iEnd )
	{
		switch( in.GetTapNote( iTrack, r ).type )
		{
		case TapNoteType_Empty:
		case TapNoteType_Mine:
			continue;
		default:
			return true;
		}
	}

	if( in.IsHoldNoteAtRow( iTrack, iEnd ) )
		return true;

	return false;
}

/* Find the next row that either starts a TapNote, or ends a previous one. */
bool NoteDataUtil::GetNextEditorPosition( const NoteData& in, int &rowInOut )
{
	int iOriginalRow = rowInOut;
	bool bAnyHaveNextNote = in.GetNextTapNoteRowForAllTracks( rowInOut );

	int iClosestNextRow = rowInOut;
	if( !bAnyHaveNextNote )
		iClosestNextRow = MAX_NOTE_ROW;

	for( int t=0; t<in.GetNumTracks(); t++ )
	{
		int iHeadRow;
		if( !in.IsHoldHeadOrBodyAtRow(t, iOriginalRow, &iHeadRow) )
			continue;

		const TapNote &tn = in.GetTapNote( t, iHeadRow );
		int iEndRow = iHeadRow + tn.iDuration;
		if( iEndRow == iOriginalRow )
			continue;

		bAnyHaveNextNote = true;
		ASSERT( iEndRow < MAX_NOTE_ROW );
		iClosestNextRow = min( iClosestNextRow, iEndRow );
	}

	if( !bAnyHaveNextNote )
		return false;

	rowInOut = iClosestNextRow;
	return true;
}

bool NoteDataUtil::GetPrevEditorPosition( const NoteData& in, int &rowInOut )
{
	int iOriginalRow = rowInOut;
	bool bAnyHavePrevNote = in.GetPrevTapNoteRowForAllTracks( rowInOut );

	int iClosestPrevRow = rowInOut;
	for( int t=0; t<in.GetNumTracks(); t++ )
	{
		int iHeadRow = iOriginalRow;
		if( !in.GetPrevTapNoteRowForTrack(t, iHeadRow) )
			continue;

		const TapNote &tn = in.GetTapNote( t, iHeadRow );
		if( tn.type != TapNoteType_HoldHead )
			continue;

		int iEndRow = iHeadRow + tn.iDuration;
		if( iEndRow >= iOriginalRow )
			continue;

		bAnyHavePrevNote = true;
		ASSERT( iEndRow < MAX_NOTE_ROW );
		iClosestPrevRow = max( iClosestPrevRow, iEndRow );
	}

	if( !bAnyHavePrevNote )
		return false;

	rowInOut = iClosestPrevRow;
	return true;
}


unsigned int NoteDataUtil::GetTotalHoldTicks( NoteData* nd, const TimingData* td )
{
	unsigned int ret = 0;
	// Last row must be included. -- Matt
	int end = nd->GetLastRow()+1;
	vector<TimingSegment*> segments = td->GetTimingSegments( SEGMENT_TICKCOUNT );
	// We start with the LAST TimingSegment and work our way backwards.
	// This way we can continually update end instead of having to lookup when
	// the next segment starts.
	for(int i = segments.size() - 1; i >= 0; i--)
	{
		TickcountSegment *ts = (TickcountSegment*) segments[i];
		if( ts->GetTicks() > 0)
		{
			// Jump to each point where holds would tick and add the number of holds there to ret.
			for(int j = ts->GetRow(); j < end; j += ROWS_PER_BEAT / ts->GetTicks() )
				// 1 tick per row.
				if( nd->GetNumTracksHeldAtRow(j) > 0 )
					ret++;
		}
		end = ts->GetRow();
	}
	return ret;
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
