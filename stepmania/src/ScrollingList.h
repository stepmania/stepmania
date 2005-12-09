/* ScrollingList - An array of scrolling graphics. */

#ifndef SCROLLINGLIST_H
#define SCROLLINGLIST_H

#include "ActorFrame.h"
#include "Sprite.h"


class ScrollingList : public ActorFrame
{
public:
	ScrollingList();
	~ScrollingList();

	void Load( const vector<CString>& asGraphicPaths );
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
	
	int		m_iBouncingState;
	int		m_iBounceDir;
	int		m_iBounceWait;
	float	m_iBounceSize;
	

	int		m_iBannerPrefs;
	int		m_iSpriteType;
	int		m_iSelection;
	float	m_fSelectionLag;
	int		m_iSpacing;
	int		m_iNumVisible;
	float	m_fNextTween;
	Sprite	m_sprBannerMask;
	Sprite	m_RippleCSprite;
	Sprite	m_RippleSprite;

	vector<Sprite*>	m_apSprites;	// stores the list of elements (left to right)
	vector<Sprite*>	m_apCSprites;	// stores the list of elements (left to right)
};

#endif

/*
 * (c) 2001-2003 "Frieza"
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
