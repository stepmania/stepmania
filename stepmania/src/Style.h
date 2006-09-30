/* Style - A data structure that holds the definition for one of a Game's styles. */

#ifndef STYLE_H
#define STYLE_H

#include "GameInput.h"
#include "NoteTypes.h"
#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"


const int MAX_COLS_PER_PLAYER = MAX_NOTE_TRACKS;
static const int Column_INVALID = -1;

class NoteData;
class Game;
struct lua_State;

class Style
{
public:
	const Game*	m_pGame;			// Which Game is this Style used with?
	bool		m_bUsedForGameplay;		// Can be used only for gameplay?
	bool		m_bUsedForEdit;			// Can be used for editing?
	bool		m_bUsedForDemonstration;	// Can be used for demonstration?
	bool		m_bUsedForHowToPlay;		// Can be used for HowToPlay?
	
	/* The name of the style.  (This is currently unused.) */
	const char 	*m_szName;
	
	/* Steps format used for each player.  For example, "dance versus" reads
	 * the Steps with the tag "dance-single". */
	StepsType	m_StepsType;
									
	StyleType	m_StyleType;
	
	int		m_iColsPerPlayer;		// number of total tracks this style expects (e.g. 4 for versus, but 8 for double)
	struct ColumnInfo 
	{ 
		int		track;			// take note data from this track
		float		fXOffset;		// x position of the column relative to player center
		const char	*pzName;		// name of this column, or NULL to use the name of a button mapped to it
	};

	ColumnInfo	m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];	// maps each players' column to a track in the NoteData

	/* This maps from game inputs to columns.  More than one button may map to a
	 * single column. */
	enum { NO_MAPPING = -1, END_MAPPING = -2 };
	int		m_iInputColumn[NUM_GameController][NUM_GameButton]; // maps each input to a column, or GameButton_Invalid
	int		m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
	bool		m_bNeedsZoomOutWith2Players;
	bool		m_bCanUseBeginnerHelper;
	bool		m_bLockDifficulties;		// used in couple Styles

	GameInput StyleInputToGameInput( int iCol, PlayerNumber pn ) const;
	int GameInputToColumn( const GameInput &GameI ) const;
	RString ColToButtonName( int iCol ) const;

	void GetTransformedNoteDataForStyle( PlayerNumber pn, const NoteData& original, NoteData& noteDataOut ) const;

	void GetMinAndMaxColX( PlayerNumber pn, float& fMixXOut, float& fMaxXOut ) const;
	
	// Lua
	void PushSelf( lua_State *L );
};


#endif

/*
 * (c) 2001-2002 Chris Danford
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
