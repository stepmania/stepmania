/********************************
ScreenSelectMode.h
Desc: The "Style Select Screen" for Ez2dancer
Copyright (c):
Andrew Livy
*********************************/
#ifndef __SCREEN_SELECT_MODE_H
#define __SCREEN_SELECT_MODE_H
/* Includes */

#include "ScreenSelect.h"
#include "Background.h"
#include "Screen.h"
#include "Sprite.h"
#include "Quad.h"
#include "MenuElements.h"
#include "ScrollingList.h"
#include "GameConstantsAndTypes.h"
#include "ModeChoice.h"
#include "BitmapText.h"
#include "RandomSample.h"
#include "BGAnimationLayer.h"
#include "Character.h"

/* Class Definition */

#define MAX_ELEMS 30

class ScreenSelectMode : public ScreenSelect
{
public:
	ScreenSelectMode( CString sName ); // Constructor
	virtual ~ScreenSelectMode(); // Destructor
	virtual void MenuLeft( PlayerNumber pn );
	virtual void MenuRight( PlayerNumber pn );
	virtual void MenuUp(PlayerNumber pn );
	virtual void MenuDown(PlayerNumber pn);
	virtual void MenuStart( PlayerNumber pn );
	virtual void Update( float fDelta );
	virtual void DrawPrimitives();
	virtual void HandleScreenMessage( const ScreenMessage SM );
protected:
	virtual int GetSelectionIndex( PlayerNumber pn );
	virtual void UpdateSelectableChoices();
	void SetCharacters();
	void ChangeBGA();
	int m_iNumChoices;
	int m_iSelectableChoices[MAX_ELEMS];

	RageSound			m_soundModeChange;
	RageSound			m_soundConfirm;
	RageSound			m_soundStart;
	CStringArray arrayLocations;
	ScrollingList m_ScrollingList;
	Sprite m_ChoiceListFrame;
	Sprite m_ChoiceListHighlight;
	Sprite m_sprJoinMessage[NUM_PLAYERS];
	Sprite m_sprJoinFrame[NUM_PLAYERS];
	Sprite m_CurChar[NUM_PLAYERS];
	int m_iCurrentChar[NUM_PLAYERS];
	Sprite m_Guide;
	vector<BGAnimation*>	m_Backgrounds;
	bool m_bSelected;
	bool m_b2DAvailable;
	bool m_bCharsAvailable;
	
//	private:
//	vector<Character*>		m_Characters;
};

#endif

