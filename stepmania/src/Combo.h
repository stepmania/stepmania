//-----------------------------------------------------------------------------
// File: Combo.h
//
// Desc: Combo counter that displays while dancing.
//
// Copyright (c) 2001 Chris Danford.  All rights reserved.
//-----------------------------------------------------------------------------

#ifndef _COMBO_H_
#define _COMBO_H_


#include "Sprite.h"
#include "BitmapText.h"

class Combo
{
public:
	Combo();
	~Combo();

	void SetX( int iNewX );

	void Update( const FLOAT &fDeltaTime );
	void Draw();

	void SetCombo( int iNum );

private:

	BOOL   m_bVisible;

	Sprite m_sprCombo;
	BitmapText m_textNum;
};




#endif