#ifndef ScreenSelect_H
#define ScreenSelect_H
/*
-----------------------------------------------------------------------------
 Class: ScreenSelect

 Desc: Base class for Style, Difficulty, and Mode selection screens.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "MenuElements.h"
#include "BGAnimation.h"
#include "ModeChoice.h"

// Derived classes must send this when done
const ScreenMessage SM_AllDoneChoosing = (ScreenMessage)(SM_User+123);	// unique

#define MAX_CHOICES 30

class ScreenSelect : public Screen
{
public:
	ScreenSelect( CString sClassName );
	virtual ~ScreenSelect();

	virtual void Update( float fDelta );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual void MenuBack( PlayerNumber pn );


protected:
	virtual int GetSelectionIndex( PlayerNumber pn ) = 0;
	virtual void UpdateSelectableChoices() = 0;		// derived screens must handle this
	
	CString m_sClassName;

	MenuElements m_Menu;
	BGAnimation m_BGAnimations[MAX_CHOICES];

	vector<ModeChoice>	m_aModeChoices;		// derived classes should look here for what choices are available
};


#endif
