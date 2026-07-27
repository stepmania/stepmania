/** @brief Style - A data structure that holds the definition for one of a Game's styles. */

#ifndef STYLE_H
#define STYLE_H

#include "GameInput.h"
#include "NoteTypes.h"
#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"

#include <vector>


/** @brief Each style can have a maximum amount of columns to work with. */
const int MAX_COLS_PER_PLAYER = MAX_NOTE_TRACKS;
/** @brief Provide a default value for an invalid column. */
static const int Column_Invalid = -1;

class NoteData;
struct Game;
struct lua_State;

class Style
{
public:
	/** @brief Can this style be used for gameplay purposes? */
	bool m_bUsedForGameplay;
	/** @brief Can this style be used for making edits? */
	bool m_bUsedForEdit;
	/** @brief Can this style be used in a demonstration? */
	bool m_bUsedForDemonstration;
	/** @brief Can this style be used to explain how to play the game? */
	bool m_bUsedForHowToPlay;

	/**
	 * @brief The name of the style.
	 *
	 * Used by GameManager::GameAndStringToStyle to determine whether this is the style that matches the string. */
	const char *		m_szName;

	/**
	 * @brief Steps format used for each player.
	 *
	 * For example, "dance versus" reads the Steps with the tag "dance-single". */
	StepsType		m_StepsType;

	/** @brief Style format used for each player. */
	StyleType		m_StyleType;

	/**
	 * @brief The number of total tracks/columns this style expects.
	 *
	 * As an example, 4 is expected for ITG style versus, but 8 for ITG style double. */
	int			m_iColsPerPlayer;
	/** @brief Some general column infromation */
	struct ColumnInfo
	{
		int   track;		/**< Take note data from this track. */
		float fXOffset;		/**< This is the x position of the column relative to the player's center. */
		const char *pzName;	/**< The name of the column, or nullptr to use the button name mapped to it. */
	};

	/** @brief Map each players' colun to a track in the NoteData. */
	ColumnInfo		m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];

	/* This maps from game inputs to columns. More than one button may map to a
	 * single column. */
	enum { NO_MAPPING = -1, END_MAPPING = -2 };
	/** @brief Map each input to a column, or GameButton_Invalid. */
	int			m_iInputColumn[NUM_GameController][NUM_GameButton];
	int			m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
	/** @brief Does this style need to be zoomed out with two players due to too many columns? */
	// Design change:  Instead of having a flag in the style that toggles a
	// fixed zoom that is only applied to the columns, ScreenGameplay now
	// calculates a zoom factor to apply to the notefield and puts it in the
	// PlayerState. -Kyz
	//bool		m_bNeedsZoomOutWith2Players;
	/** @brief Can this style use the BeginnerHelper for assisting new people to the game? */
	bool		m_bCanUseBeginnerHelper;
	/**
	 * @brief Should difficulty selection be locked when using this style?
	 *
	 * This is primarily for Couple and Routine styles. */
	bool		m_bLockDifficulties;

	void StyleInputToGameInput( int iCol, PlayerNumber pn, std::vector<GameInput>& ret ) const;
	/**
	 * @brief Retrieve the column based on the game input.
	 * @param GameI the game input.
	 * @return the Column number of the style, or Column_Invalid if it's an invalid column.
	 * Examples of this include getting the upper left hand corner in a traditional four panel mode. */
	int GameInputToColumn( const GameInput &GameI ) const;
	RString ColToButtonName( int iCol ) const;

	bool GetUsesCenteredArrows() const;
	void GetTransformedNoteDataForStyle( PlayerNumber pn, const NoteData& original, NoteData& noteDataOut ) const;
	void GetMinAndMaxColX( PlayerNumber pn, float& fMixXOut, float& fMaxXOut ) const;
	float GetWidth(PlayerNumber pn) const;

	// Lua
	void PushSelf( lua_State *L );
};


#endif

/**
 * @file
 * @author Chris Danford (c) 2001-2002
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
