#ifndef ModeSelectorMaxType1_H
#define ModeSelectorMaxType1_H
/*
-----------------------------------------------------------------------------
 Class: ModeSelectorMaxType1

 Desc: Abstract class for a widget that selects a ModeChoice

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "ModeSelector.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "RageSound.h"


#define MAX_MODE_CHOICES 30


class ModeSelectorMaxType1 : public ModeSelector
{
public:
	ModeSelectorMaxType1();
	virtual ~ModeSelectorMaxType1() {};

	virtual void Init( const vector<ModeChoice>& choices, CString sClassName, CString sThemeElementPrefix );

	virtual void MenuLeft( PlayerNumber pn );
	virtual void MenuRight( PlayerNumber pn );
	virtual void MenuUp( PlayerNumber pn ) {};
	virtual void MenuDown( PlayerNumber pn ) {};
	virtual void MenuStart( PlayerNumber pn );
	virtual void MenuBack( PlayerNumber pn );
	virtual void TweenOffScreen();
	virtual void TweenOnScreen();

	virtual void GetSelectedModeChoice( PlayerNumber pn, ModeChoice* pModeChoiceOut );
	virtual void UpdateSelectableChoices();

protected:
	void BeforeChange();
	void AfterChange();

	vector<ModeChoice>	m_aModeChoices;
	int		m_iSelection;

	Sprite		m_sprIcon[MAX_MODE_CHOICES];
	// Artists don't make graphics for every single Game, so
	// have a text representation if textures are missing.
	BitmapText	m_textIcon[MAX_MODE_CHOICES];
	Sprite		m_sprPreview[MAX_MODE_CHOICES];
	Sprite		m_sprInfo[MAX_MODE_CHOICES];
	Sprite		m_sprExplanation;
	Sprite		m_sprJointPremium;
	
	RageSound m_soundChange;
	RageSound m_soundSelect;
};

#endif
