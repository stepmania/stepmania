#ifndef NOTES_H
#define NOTES_H
/*
-----------------------------------------------------------------------------
 Class: Notes

 Desc: Holds note information for a Song.  A Song may have one or more Notess.
	A Notes can be played by one or more Styles.  See StyleDef.h for more info on
	Styles.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"
#include "Grade.h"
class NoteData;

enum NotesDisplayType
{
	NOTES_DISPLAY_EASY,
	NOTES_DISPLAY_MEDIUM,
	NOTES_DISPLAY_HARD,
	NOTES_DISPLAY_S_HARD,
	NOTES_DISPLAY_CHALLENGE,
	NOTES_DISPLAY_BATTLE
};

struct Notes
{
public:
	Notes();
	~Notes();

	// Loading
	bool LoadFromNotesFile( const CString &sPath );

	/* We can have one or both of these; if we have both, they're always identical.
	 * Call Compress() to force us to only have notes_comp; otherwise, creation of 
	 * these is transparent. */
	mutable NoteData *notes;
	mutable CString *notes_comp;
	void Decompress() const;

public:
	void Compress() const;

	NotesType		m_NotesType;
	CString			m_sDescription;		// This text is displayed next to thte number of feet when a song is selected
	bool			m_bAutoGen;			// Was created by autogen?
	Difficulty		m_Difficulty;		// difficulty classification
	int				m_iMeter;			// difficulty rating from 1-10
	float			m_fRadarValues[NUM_RADAR_VALUES];	// between 0.0-1.2 starting from 12-o'clock rotating clockwise

	void			GetNoteData( NoteData* pNoteDataOut ) const;
	void			SetNoteData( NoteData* pNewNoteData );
	void			SetSMNoteData( const CString &out );
	CString 		GetSMNoteData() const;

	NotesDisplayType GetNotesDisplayType() const;
	RageColor		GetColor() const;
	
	// Statistics
	Grade m_TopGrade;
	int m_iTopScore;
	int m_iMaxCombo;
	int m_iNumTimesPlayed;

	static Difficulty DifficultyFromDescriptionAndMeter( CString sDescription, int iMeter );

	void TidyUpData();

protected:

};

bool CompareNotesPointersByDifficulty(const Notes *pNotes1, const Notes *pNotes2);
void SortNotesArrayByDifficulty( CArray<Notes*,Notes*> &arrayNotess );

#endif
