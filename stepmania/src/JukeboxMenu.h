#ifndef JukeboxMenu_H
#define JukeboxMenu_H
/*
-----------------------------------------------------------------------------
 Class: JukeboxMenu

 Desc: UI on ScreenJukeboxMenu.
 
 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "Banner.h"
#include "GameConstantsAndTypes.h"
#include "RandomSample.h"
#include "Style.h"
#include "BitmapText.h"


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


	Style		GetSelectedStyle()				{ return m_Styles[m_iSelection[ROW_STYLE]]; }
	CString		GetSelectedGroup()				{ return m_sGroups[m_iSelection[ROW_GROUP]]; }
	CString		GetSelectedDifficultyString()	{ return m_sDifficulties[m_iSelection[ROW_DIFFICULTY]]; }
	Difficulty	GetSelectedDifficulty()			{ return StringToDifficulty( GetSelectedDifficultyString() ); }
	bool		GetSelectedModifiers()			{ return m_iSelection[ROW_MODIFIERS] != 0; }

private:
	Sprite	m_sprArrows[2];

	int			m_iSelection[NUM_ROWS];
	BitmapText	m_textLabel[NUM_ROWS];
	BitmapText	m_textValue[NUM_ROWS];

	vector<Style>		m_Styles;
	CStringArray		m_sGroups;
	vector<string>		m_sDifficulties;
	CStringArray		m_sModifiers;

	void OnRowValueChanged( Row row );
	void ChangeToRow( Row newRow );

	RandomSample	m_soundChangeRow;
	RandomSample	m_soundChangeValue;
};

#endif
