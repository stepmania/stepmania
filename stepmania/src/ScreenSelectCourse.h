/*
-----------------------------------------------------------------------------
 Class: ScreenSelectCourse

 Desc: The screen in PLAY_MODE_ONI where you choose a Course.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "RandomStream.h"
#include "GameConstantsAndTypes.h"
#include "MusicWheel.h"
#include "CourseInfoFrame.h"
#include "CourseContentsFrame.h"
#include "MenuElements.h"


class ScreenSelectCourse : public Screen
{
public:
	ScreenSelectCourse();
	virtual ~ScreenSelectCourse();

	virtual void DrawPrimitives();

	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	void TweenOnScreen();
	void TweenOffScreen();

	void MenuLeft( const PlayerNumber p, const InputEventType type );
	void MenuRight( const PlayerNumber p, const InputEventType type );
	void MenuStart( const PlayerNumber p );
	void MenuBack( const PlayerNumber p );

protected:
	void AfterCourseChange();

	MenuElements		m_Menu;

	CourseInfoFrame		m_CourseInfoFrame;
	CourseContentsFrame	m_CourseContentsFrame;
	MusicWheel			m_MusicWheel;

	bool				m_bMadeChoice;
	bool				m_bGoToOptions;
	BitmapText			m_textHoldForOptions;

	RandomSample		m_soundSelect;
	RandomSample		m_soundChangeNotes;
};


