/*
-----------------------------------------------------------------------------
 Class: ScreenTitleMenu

 Desc: The main title screen and menu.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "TransitionFade.h"
#include "RandomSample.h"
#include "RandomStream.h"
#include "BackgroundAnimation.h"



class ScreenTitleMenu : public Screen
{
public:
	ScreenTitleMenu();
	virtual ~ScreenTitleMenu();

	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	enum TitleMenuChoice {
		CHOICE_GAME_START = 0,
		CHOICE_SELECT_GAME,
		CHOICE_MAP_INSTRUMENTS,
		CHOICE_INPUT_OPTIONS,
		CHOICE_MACHINE_OPTIONS,
		CHOICE_GRAPHIC_OPTIONS,
		CHOICE_APPEARANCE_OPTIONS,
		CHOICE_EDIT,
		CHOICE_EXIT,
		NUM_TITLE_MENU_CHOICES	// leave this at the end!
	};

private:
	void GainFocus( int iChoiceIndex );
	void LoseFocus( int iChoiceIndex );
	void MenuUp( PlayerNumber pn );
	void MenuDown( PlayerNumber pn );
	void MenuBack( PlayerNumber pn );
	void MenuStart( PlayerNumber pn );

	TitleMenuChoice m_TitleMenuChoice;

	BackgroundAnimation	m_Background;
	Sprite				m_sprLogo;
	BitmapText			m_textHelp;
	BitmapText			m_textVersion;
	BitmapText			m_textSongs;
	BitmapText			m_textChoice[NUM_TITLE_MENU_CHOICES];

	TransitionFade		m_Fade;

	RandomStream		m_soundAttract;
	RandomSample		m_soundChange;
	RandomSample		m_soundSelect;
	RandomSample		m_soundInvalid;
};



