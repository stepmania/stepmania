/* JukeboxMenu - UI on ScreenJukeboxMenu. */

#ifndef JUKEBOX_MENU_H
#define JUKEBOX_MENU_H

#include "ActorFrame.h"
#include "Banner.h"
#include "GameConstantsAndTypes.h"
#include "RandomSample.h"
#include "BitmapText.h"

class Style;

class JukeboxMenu: public ActorFrame 
{
public:
	JukeboxMenu();
	~JukeboxMenu();
	virtual void DrawPrimitives();

	bool CanGoUp();
	bool CanGoDown();
	bool CanGoLeft();
	bool CanGoRight();

	void Up();
	void Down();
	void Left();
	void Right();

	enum Row 
	{ 
		ROW_STYLE, 
		ROW_GROUP, 
		ROW_DIFFICULTY,
		ROW_MODIFIERS,
		NUM_ROWS
	} m_SelectedRow;
	CString RowToString( Row r )
	{
		const CString s[NUM_ROWS] = 
		{
			"Style",
			"Group",
			"Difficulty",
			"Modifiers",
		};
		return s[r];
	}


	const Style*	GetSelectedStyle()				{ return m_Styles[m_iSelection[ROW_STYLE]]; }
	CString			GetSelectedGroup()				{ return m_sGroups[m_iSelection[ROW_GROUP]]; }
	CString			GetSelectedDifficultyString()	{ return m_sDifficulties[m_iSelection[ROW_DIFFICULTY]]; }
	Difficulty		GetSelectedDifficulty()			{ return StringToDifficulty( GetSelectedDifficultyString() ); }
	bool			GetSelectedModifiers()			{ return m_iSelection[ROW_MODIFIERS] != 0; }

private:
	Sprite	m_sprArrows[2];

	int			m_iSelection[NUM_ROWS];
	BitmapText	m_textLabel[NUM_ROWS];
	BitmapText	m_textValue[NUM_ROWS];

	vector<const Style*>	m_Styles;
	CStringArray			m_sGroups;
	vector<string>			m_sDifficulties;
	CStringArray			m_sModifiers;

	void OnRowValueChanged( Row row );
	void ChangeToRow( Row newRow );

	RandomSample	m_soundChangeRow;
	RandomSample	m_soundChangeValue;
};

#endif

/*
 * (c) 2003 Chris Danford
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
