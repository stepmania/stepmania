/*
-----------------------------------------------------------------------------
 Class: ScreenTitleMenu

 Desc: The main title screen and menu.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenLogo.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "TransitionFade.h"
#include "RandomSample.h"
#include "BGAnimation.h"
#include "RageTimer.h"


class ScreenTitleMenu : public ScreenLogo
{
public:
	ScreenTitleMenu();
	virtual ~ScreenTitleMenu();

	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void Update( float fDelta );
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
		#ifdef _DEBUG
		CHOICE_SANDBOX,
		#endif
		CHOICE_EXIT,
		NUM_TITLE_MENU_CHOICES	// leave this at the end!
	};

private:
	void GainFocus( int iChoiceIndex );
	void LoseFocus( int iChoiceIndex );

	TitleMenuChoice m_TitleMenuChoice;

	BitmapText			m_textHelp;
	BitmapText			m_textChoice[NUM_TITLE_MENU_CHOICES];

	RandomSample		m_soundAttract;
	RandomSample		m_soundChange;
	RandomSample		m_soundSelect;
	RandomSample		m_soundInvalid;

	RageTimer			TimeToDemonstration;
};



