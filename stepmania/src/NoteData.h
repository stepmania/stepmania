#pragma once
/*
-----------------------------------------------------------------------------
 File: NoteData.h

 Desc: Holds data about the notes that the player is supposed to hit.  NoteData
	is organized by:
	track - corresponds to different columns of notes on the screen
	index - corresponds to subdivisions of beats

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"

// '1' = tap note
// '2' = hold note begin
// '3' = hold note end  ('1' can also end a HoldNote) ('3' without a matching '2' is ignored
// ... for future expansion


enum TapNoteScore { 
	TNS_NONE, 
	TNS_MISS,
	TNS_BOO,
	TNS_GOOD,
	TNS_GREAT,
	TNS_PERFECT, 
};

inline int TapNoteScoreToDancePoints( TapNoteScore tns )
{
	switch( tns )
	{
	case TNS_PERFECT:	return +2;
	case TNS_GREAT:		return +1;
	case TNS_GOOD:		return +0;
	case TNS_BOO:		return -4;
	case TNS_MISS:		return +8;
	default:			return 0;
	}
}

//enum TapNoteTiming { 
//	TNT_NONE, 
//	TNT_EARLY, 
//	TNT_LATE 
//};


struct HoldNote
{
	int m_iTrack;	
	int m_iStartIndex;
	int m_iEndIndex;
};

enum HoldNoteScore 
{ 
	HNS_NONE,	// this HoldNote has not been scored yet
	HNS_OK,		// the HoldNote has passed and was successfully held all the way through
	HNS_NG		// the HoldNote has passed and they missed it
};


inline int HoldNoteScoreToDancePoints( HoldNoteScore hns )
{
	switch( hns )
	{
	case HNS_OK:	return +6;
	case HNS_NG:	return +0;
	default:	ASSERT(0);	return 0;
	}
}


inline int   BeatToNoteRow( float fBeatNum )			{ return int( fBeatNum * ELEMENTS_PER_BEAT + 0.5f); };	// round
inline int   BeatToNoteIndexNotRounded( float fBeatNum ){ return (int)( fBeatNum * ELEMENTS_PER_BEAT ); };
inline float NoteRowToBeat( float fNoteIndex )		{ return fNoteIndex / (float)ELEMENTS_PER_BEAT;	 };
inline float NoteRowToBeat( int iNoteIndex )			{ return NoteRowToBeat( (float)iNoteIndex );	 };




class NoteData
{
public:
	NoteData();
	~NoteData();

	// used for loading
	void SetFromMeasureStrings( CStringArray &arrayMeasureStrings );	// for loading from a .notes file
	void SetFromDWIInfo();	// for loading from a .dwi file
	void SetFromBMSInfo();	// for loading from a .bms file
	void ReadFromCacheFile( FILE* file );
	static void SkipOverDataInCacheFile( FILE* file );

	// used for saving
	void GetMeasureStrings( CStringArray &arrayMeasureStrings );		// for saving to a .notes file
	void WriteToCacheFile( FILE* file );

	int			m_iNumTracks;
	TapNote		m_TapNotes[MAX_NOTE_TRACKS][MAX_TAP_NOTE_ROWS];
	HoldNote	m_HoldNotes[MAX_HOLD_NOTE_ELEMENTS];
	int			m_iNumHoldNotes;

	void ClearRange( int iNoteIndexBegin, int iNoteIndexEnd );
	void ClearAll() { ClearRange( 0, MAX_TAP_NOTE_ROWS ); };
	void CopyRange( NoteData* pFrom, int iNoteIndexBegin, int iNoteIndexEnd );
	void CopyAll( NoteData* pFrom ) { m_iNumTracks = pFrom->m_iNumTracks; CopyRange( pFrom, 0, MAX_TAP_NOTE_ROWS ); };

	inline bool IsRowEmpty( int index )
	{
		for( int t=0; t<m_iNumTracks; t++ )
			if( m_TapNotes[t][index] != '0' )
				return false;
		return true;
	}

	// used in edit/record
	void AddHoldNote( HoldNote newNote );	// add note hold note merging overlapping HoldNotes and destroying TapNotes underneath
	void RemoveHoldNote( int index );

	// statistics
	float GetLastBeat();	// return the beat number of the last note
	int GetLastRow();
	int GetNumTapNotes( const float fStartBeat = 0, const float fEndBeat = MAX_BEATS );
	int GetNumDoubles( const float fStartBeat = 0, const float fEndBeat = MAX_BEATS );
	int GetNumHoldNotes( const float fStartBeat = 0, const float fEndBeat = MAX_BEATS );
	
	int GetPossibleDancePoints();

	// radar values - return between 0.0 and 1.2
	float GetRadarValue( RadarCatrgory rv, float fSongSeconds )
	{
		switch( rv )
		{
		case RADAR_STREAM:	return GetStreamRadarValue( fSongSeconds );		break;
		case RADAR_VOLTAGE:	return GetVoltageRadarValue( fSongSeconds );	break;
		case RADAR_AIR:		return GetAirRadarValue( fSongSeconds );		break;
		case RADAR_CHAOS:	return GetChaosRadarValue( fSongSeconds );		break;
		case RADAR_FREEZE:	return GetFreezeRadarValue( fSongSeconds );		break;
		default:	ASSERT(0);  return 0;
		}
	};
	float GetStreamRadarValue( float fSongSeconds );
	float GetVoltageRadarValue( float fSongSeconds );
	float GetAirRadarValue( float fSongSeconds );
	float GetChaosRadarValue( float fSongSeconds );
	float GetFreezeRadarValue( float fSongSeconds );

	// Transformations
	void LoadTransformed( NoteData* pOriginal, int iNewNumTracks, int iNewToOriginalTrack[] );

	void CropToLeftSide();
	void CropToRightSide();
	void RemoveHoldNotes();
	void Turn( PlayerOptions::TurnType tt );
	void MakeLittle();

	void SnapToNearestNoteType( NoteType nt1, NoteType nt2, float fBeginBeat, float fEndBeat );

	// Convert between HoldNote representation and '2' and '3' markers in TapNotes
	void Convert2sAnd3sToHoldNotes();
	void ConvertHoldNotesTo2sAnd3s();

};
