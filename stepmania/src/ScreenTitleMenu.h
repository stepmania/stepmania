/*
-----------------------------------------------------------------------------
 Class: ScreenTitleMenu

 Desc: The main title screen and menu.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Transition.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "RageSound.h"
#include "RageTimer.h"
#include "ScreenSelect.h"

#define MAX_MODE_CHOICES 30

class ScreenTitleMenu : public ScreenSelect
{
public:
	ScreenTitleMenu();
	virtual ~ScreenTitleMenu();

	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void Update( float fDelta );
	virtual void HandleScreenMessage( const ScreenMessage SM );

private:
	int GetSelectionIndex( PlayerNumber pn ) { return m_Choice; }
	void UpdateSelectableChoices();

	void GainFocus( int iChoiceIndex );
	void LoseFocus( int iChoiceIndex );
	void MoveCursor( bool up );

	unsigned		m_Choice;

	Sprite			m_sprLogo;
	BitmapText		m_textVersion;
	BitmapText		m_textSongs;
	BitmapText		m_textHelp;
	BitmapText		m_textChoice[MAX_MODE_CHOICES];

	RageSound		m_soundChange;
	RageSound		m_soundSelect;
	RageSound		m_soundInvalid;
	
	RageTimer		TimeToDemonstration;

	BGAnimation		m_CoinMode;
	BGAnimation		m_JointPremium;

	Transition		m_AttractOut;
};



