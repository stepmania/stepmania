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

	enum Choice 
	{
		CHOICE_GAME_START = 0,
		CHOICE_SELECT_GAME,
		/* At request, moved this into the options/operator menu -- Miryokuteki */
		//CHOICE_MAP_KEY_JOY,
		CHOICE_OPTIONS,
		CHOICE_EDIT,
		CHOICE_JUKEBOX,
		#ifdef DEBUG
		CHOICE_SANDBOX,
		#endif
		CHOICE_EXIT,
		NUM_CHOICES	// leave this at the end!
	};

	CString ChoiceToString( Choice c )
	{
		const CString s[NUM_CHOICES] = {
			"GAME START",
			"SWITCH GAME",
			/* At request, moved this into the options/operator menu -- Miryokuteki */
			//"CONFIG KEY/JOY",
			"OPTIONS",
			"EDIT/SYNC SONGS",
			"JUKEBOX",
			#ifdef DEBUG
			"SANDBOX",
			#endif
			"EXIT"
		};
		return s[c];
	}


private:
	void GainFocus( int iChoiceIndex );
	void LoseFocus( int iChoiceIndex );

	Choice			m_Choice;

	BitmapText		m_textHelp;
	BitmapText		m_textChoice[NUM_CHOICES];

	Sprite			m_sprJointPremiumMsg;

	RandomSample	m_soundAttract;
	RandomSample	m_soundChange;
	RandomSample	m_soundSelect;
	RandomSample	m_soundInvalid;
	
	RageTimer		TimeToDemonstration;
	RageTimer		TimeToJPScroll;
};



