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

//#include "GameConstantsAndTypes.h"
#include "PlayerOptions.h"
#include "NoteTypes.h"

// '1' = tap note
// '2' = hold note begin
// '3' = hold note end  ('1' can also end a HoldNote) ('3' without a matching '2' is ignored
// ... for future expansion



class NoteData
{
public:
	NoteData();
	void Init();
	~NoteData();

	int			m_iNumTracks;
	TapNote		m_TapNotes[MAX_NOTE_TRACKS][MAX_TAP_NOTE_ROWS];
	HoldNote	m_HoldNotes[MAX_HOLD_NOTES];
	int			m_iNumHoldNotes;

	void LoadFromSMNoteDataString( CString sSMNoteData );
	CString GetSMNoteDataString();

	void ClearRange( int iNoteIndexBegin, int iNoteIndexEnd );
	void ClearAll() { ClearRange( 0, MAX_TAP_NOTE_ROWS ); };
	void CopyRange( NoteData* pFrom, int iFromIndexBegin, int iFromIndexEnd, int iToIndexBegin = -1 );
	void CopyAll( NoteData* pFrom ) { m_iNumTracks = pFrom->m_iNumTracks; m_iNumHoldNotes = 0; CopyRange( pFrom, 0, MAX_TAP_NOTE_ROWS ); };

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
	bool IsThereANoteAtRow( int iRow );
	float GetFirstBeat();	// return the beat number of the first note
	int GetFirstRow();
	float GetLastBeat();	// return the beat number of the last note
	int GetLastRow();
	int GetNumTapNotes( const float fStartBeat = 0, const float fEndBeat = MAX_BEATS );
	int GetNumDoubles( const float fStartBeat = 0, const float fEndBeat = MAX_BEATS );
	int GetNumHoldNotes( const float fStartBeat = 0, const float fEndBeat = MAX_BEATS );
	
	int GetPossibleDancePoints();

	// radar values - return between 0.0 and 1.2
	float GetRadarValue( RadarCategory rv, float fSongSeconds )
	{
		switch( rv )
		{
		case RADAR_STREAM:	return GetStreamRadarValue( fSongSeconds );		break;
		case RADAR_VOLTAGE:	return GetVoltageRadarValue( fSongSeconds );	break;
		case RADAR_AIR:		return GetAirRadarValue( fSongSeconds );		break;
		case RADAR_FREEZE:	return GetFreezeRadarValue( fSongSeconds );		break;
		case RADAR_CHAOS:	return GetChaosRadarValue( fSongSeconds );		break;
		default:	ASSERT(0);  return 0;
		}
	};
	float GetStreamRadarValue( float fSongSeconds );
	float GetVoltageRadarValue( float fSongSeconds );
	float GetAirRadarValue( float fSongSeconds );
	float GetFreezeRadarValue( float fSongSeconds );
	float GetChaosRadarValue( float fSongSeconds );

	// Transformations
	void LoadTransformed( NoteData* pOriginal, int iNewNumTracks, const int iOriginalTrackToTakeFrom[] );	// -1 for iOriginalTracksToTakeFrom means no track

	void CropToLeftSide();
	void CropToRightSide();
	void RemoveHoldNotes();
	void Turn( PlayerOptions::TurnType tt );
	void MakeLittle();

	void SnapToNearestNoteType( NoteType nt1, NoteType nt2, float fBeginBeat, float fEndBeat );

	NoteType GetSmallestNoteTypeForMeasure( int iMeasureIndex );

	// Convert between HoldNote representation and '2' and '3' markers in TapNotes
	void Convert2sAnd3sToHoldNotes();
	void ConvertHoldNotesTo2sAnd3s();

	void Convert4sToHoldNotes();
	void ConvertHoldNotesTo4s();
};

