#ifndef SCROLLINGLIST_H
#define SCROLLINGLIST_H
/*
-----------------------------------------------------------------------------
 Class: ScrollingList

 Desc: Creates an array of graphics which can scroll left and right.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Andrew Livy
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "Sprite.h"
#include "CroppedSprite.h"


class ScrollingList : public ActorFrame
{
public:
	ScrollingList();
	~ScrollingList();

	void Load( const CStringArray& asGraphicPaths );
	void Unload();	// delete all items.  Called automatically on Load()

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	void Replace(CString sGraphicPath, int ElementNumber);

	void SetSelection( int iIndex );	
	int GetSelection();
	void SetNumberVisible( int iNumVisibleElements );
	void SetSpacing( int iSpacingInPixels );
	void UseSpriteType(int NewSpriteType);
	void StartBouncing();
	void StopBouncing();
	void Left();
	void Right();

protected:
	
	int						m_iBouncingState;
	int						m_iBounceDir;
	int						m_iBounceWait;
	float					m_iBounceSize;
	

	int						m_iBannerPrefs;
	int						m_iSpriteType;
	int						m_iSelection;
	float					m_fSelectionLag;
	int						m_iSpacing;
	int						m_iNumVisible;
	float					m_fNextTween;
	Sprite				m_sprBannerMask;
	CroppedSprite	m_RippleCSprite;
	Sprite m_RippleSprite;

	vector<Sprite*>	m_apSprites;	// stores the list of elements (left to right)
	vector<CroppedSprite*>	m_apCSprites;	// stores the list of elements (left to right)
};

#endif
